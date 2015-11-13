#include <linux/string.h>
#include <linux/kernel.h>
#include "fat.h"
#include "entry.h"
#include "file.h"

#ifdef __KERNEL__
#define printf printk
#endif

BOOL get_dir_path (ps16_t file_path, ps16_t dir_path) {
  ps16_t last_seperator = strrchr(file_path, L'/');

  printf("get_dir_path last_seperator : %s\n", last_seperator);

  if (last_seperator == NULL)
    return FALSE;

  strncpy(dir_path, file_path, last_seperator - file_path);
  return TRUE;
}

BOOL get_file_name (ps16_t file_path, ps16_t file_name) {
  ps16_t last_seperator = strrchr(file_path, L'/');

  printf("get_file_name last_seperator : %s\n", last_seperator);

  if (last_seperator == NULL)
    return FALSE;

  strcpy(file_name, last_seperator + 1);
  return TRUE;
}

/*
 함수명  : search_fileDirectoryEntryInDirCluster
 하는일  : 디렉토리 클러스터 안에 파일 디렉토리 엔트리를 찾는다.
 디렉토리가 가지고 있는 모든 클러스터를 순환하여
 파일 이름과 같은 엔트리를 찾는다.
 인자    : fVolume : 루프백이미지/파일 볼륨의 포인터
 nDirClusterNumber : 디렉토리 클러스터 번호
 pFileName : 파일이름의 문자열 포인터
 pSearchedDirEntry : 검색된 디렉토리 엔트리의 포인터
 리턴 값 : BOOL
 */
BOOL get_dentry (struct mfs_volume* volume, u128 dir_cluster_number, ps16_t file_name, struct mfs_dirent* searched_dir_entry) {
  u8_t cluster[CLUSTER_SIZE] = { 0, };
  const u32_t entry_per_data_cluster = CLUSTER_SIZE / sizeof(struct mfs_dirent);
  s16_t composited_file_name[128] = { 0, };
  BOOL has_long_file_name_next_entry = FALSE;
  u128 read_position = 0;
  u128 current_cluster_number = dir_cluster_number;
  u32_t current_entry_number = 0;
  struct mfs_dirent* current_dir_entry = NULL;
  u128 end_cluster = get_end_cluster(volume);

  printf("get_dentry: %s\n", file_name);

//  if (strlen(file_name) == 0) {
//    printf("file name is too short\n");
//    return FALSE;
//  }

  // 디렉토리의 모든 클러스터를 검사한다.
  while (current_cluster_number != end_cluster) {
    //printf("current_cluster_number: %d\n", current_cluster_number);

    read_position = read_cluster(volume, current_cluster_number);

#ifdef __KERNEL__
    seek_volume(volume, read_position);
#else
    seek_volume(volume, read_position, SEEK_SET);
#endif
    read_volume(volume, cluster, sizeof(u8_t), CLUSTER_SIZE);

    current_dir_entry = get_first_entry(cluster, &current_entry_number,
					has_long_file_name_next_entry);

    while (current_entry_number != entry_per_data_cluster) {
      //printf("current entry number after : %d\n", current_entry_number);

      if (is_normal_file(current_dir_entry->attribute) == TRUE) {
	// 얻은 엔트리가 LongFileName인지 여부 검사
	if (is_long_file_name(current_dir_entry->attribute) == TRUE) {
	  // LongFileName일 경우 LongFileName을 조합한다.
	  composite_long_file_name(volume, current_cluster_number,
				   current_entry_number, composited_file_name);
	}
	else {
	  // 일반 FileName일 경우 복사
	  strcpy(composited_file_name, current_dir_entry->name);
	}

	// Name 비교
	if (!strcmp(file_name, composited_file_name)) {
	  memcpy(searched_dir_entry, current_dir_entry,
		 sizeof(struct mfs_dirent));

	  return TRUE;
	}
      }

      // 다음 엔트리를 얻는다.
      current_dir_entry = get_next_entry(cluster, &current_entry_number,
					 &has_long_file_name_next_entry);
    }

    current_cluster_number = read_fat_index(volume, current_cluster_number);
    //printf("read fat index : %d\n", current_cluster_number);
  }

  printf("file not exist\n");

  return FALSE;
}

/*
 함수명  : search_routeCluster
 하는일  : 경로 끝의 클러스터 수치를 구한다.
 pRoute를 strtok '/' 로 자르고 NULL이 나올때까지 디렉토리를
 참조해 들어간다.
 ex) 한번 자르고 NULL이 아니라면 자른 값이 현재 디렉토리안에 엔트리가 있는지
 여부를 검사, 존재한다면 그곳으로 또 잘라서 참조해 들어간다.
 인자    : fVolume : 루프백이미지/파일 볼륨의 포인터
 pRoute  : 경로의 문자열 포인터
 리턴 값 : BOOL
 */

u32_t get_cluster_number (struct mfs_volume* volume, ps16_t path) {
  u8_t current_cluster[CLUSTER_SIZE] = { 0, };
  ps16_t seperated_path = strtok(path, "/");
  const u32_t entry_per_data_cluster = CLUSTER_SIZE / sizeof(struct mfs_dirent);
  struct mfs_dirent* current_dir_entry = NULL;
  s16_t composited_file_name[128] = { 0, };
  u128 current_cluster_number = 2;	// Root Directory의 클러스터 값
  u32_t current_entry_number = 0;
  BOOL is_dir_changed = FALSE;
  BOOL has_long_file_name_next_entry = FALSE;
  u128 end_cluster = get_end_cluster(volume);
  u128 read_position = read_cluster(volume, current_cluster_number);

#ifdef __KERNEL__
  seek_volume(volume, read_position);
#else
  seek_volume(volume, read_position, SEEK_SET);
#endif
  read_volume(volume, current_cluster, sizeof(u8_t), CLUSTER_SIZE);

  while (seperated_path != NULL) {
    is_dir_changed = FALSE;

    // 클러스터의 첫 엔트리를 얻는다.
    current_dir_entry = get_first_entry(current_cluster, &current_entry_number,
					has_long_file_name_next_entry);

    // 클러스터의 모든 엔트리를 검사한다.
    while (current_entry_number != entry_per_data_cluster) {

      // 얻은 엔트리가 폴더일 경우
      if (is_normal_dir(current_dir_entry->attribute) == TRUE) {
	// 얻은 엔트리가 LongFileName인지 여부 검사
	if (is_long_file_name(current_dir_entry->attribute) == TRUE) {
	  // LongFileName일 경우 LongFileName을 조합한다.
	  composite_long_file_name(volume, current_cluster_number,
				   current_entry_number, composited_file_name);
	}
	else {
	  // 일반 FileName일 경우 복사
	  strcpy(composited_file_name, current_dir_entry->name);
	}

	// Name 비교
	if (!strcmp(seperated_path, composited_file_name)) {
	  // 일치한다면 다음 route 경로 명을 얻는다.
	  seperated_path = strtok(NULL, "/");

	  current_cluster_number = current_dir_entry->head_cluster_number;
	  is_dir_changed = TRUE;
	  break;
	}
      }

      // 다음 엔트리를 얻는다.
      current_dir_entry = get_next_entry(current_cluster, &current_entry_number,
					 &has_long_file_name_next_entry);
    }

    // 현재 탐색 디렉토리가 변경되지 않았다면 현재 디렉토리의 다음 클러스터를 얻는다.
    if (is_dir_changed == FALSE) {
      current_cluster_number = read_fat_index(volume, current_cluster_number);

      // route 경로가 잘못되었다.
      if (current_cluster_number == end_cluster) {
	printf("wrong route\n");
	return 0;
      }
    }

    // 지정된 번호의 클러스터를 읽는다.
    read_position = read_cluster(volume, current_cluster_number);

#ifdef __KERNEL__
    seek_volume(volume, read_position);
#else
    seek_volume(volume, read_position, SEEK_SET);
#endif
    read_volume(volume, current_cluster, sizeof(u8_t), CLUSTER_SIZE);
  }

  return current_cluster_number;
}

/*
 함수명  : writeDirEntryInDirCluster
 하는일  : 디렉토리 클러스터안에 디렉토리 엔트리를 추가한다.
 디렉토리의 모든 클러스터를 검사하여, 빈 엔트리를 찾고 그 위치에 디렉토리 엔트리를 추가한다.
 인자    : fVolume :  루프백이미지/파일 볼륨의 포인터
 nDirClusterNumber : 디렉토리 클러스터 번호
 pDirectoryEntry : 디렉토리 엔트리의 포인터
 pFileName : 파일 이름의 문자열 포인터
 리턴 값 : BOOL
 */
BOOL alloc_new_dirent (struct mfs_volume* volume, u128 dir_cluster_number, struct mfs_dirent* dirent, ps16_t file_name) {

  u8_t cluster[CLUSTER_SIZE] = { 0, };
  const u32_t entry_per_data_cluster = CLUSTER_SIZE / sizeof(struct mfs_dirent);
  BOOL has_long_file_name_next_entry = FALSE;
  u128 read_position = 0;
  u128 wirte_position = 0;
  u128 current_cluster_number = dir_cluster_number;
  u128 before_cluster_number = 0;
  u32_t current_entry_number = 0;
  struct mfs_dirent* current_dirent = NULL;
  u128 end_cluster = get_end_cluster(volume);

  // 디렉토리의 모든 클러스터를 검사한다.
  while (current_cluster_number != end_cluster) {
    read_position = read_cluster(volume, current_cluster_number);

#ifdef __KERNEL__
    seek_volume(volume, read_position);
#else
    seek_volume(volume, read_position, SEEK_SET);
#endif
    read_volume(volume, cluster, sizeof(u8_t), CLUSTER_SIZE);

    current_dirent = get_first_entry(cluster, &current_entry_number, has_long_file_name_next_entry);

    while (current_entry_number != entry_per_data_cluster) {
      //if (current_dirent->size == 0) {
      //if (is_deleted_file(current_dirent->attribute) || is_deleted_dir(current_dirent->attribute)) {
      if (is_empty_entry(current_dirent)) {
	wirte_position = read_position + (current_entry_number * sizeof(struct mfs_dirent));

#ifdef __KERNEL__
	seek_volume(volume, wirte_position);
#else
	seek_volume(volume, wirte_position, SEEK_SET);
#endif

	write_volume(volume, dirent, sizeof(struct mfs_dirent), 1);

	printf("alloc_new_dirent %d %d\n", current_cluster_number,
	       wirte_position);

	return TRUE;
      }

      // 다음 엔트리를 얻는다.
      current_dirent = get_next_entry(cluster, &current_entry_number,
				      &has_long_file_name_next_entry);
    }

    before_cluster_number = current_cluster_number;
    current_cluster_number = read_fat_index(volume, current_cluster_number);
  }

  // 디렉토리 클러스터에 빈 공간이 없다, 클러스터 추가
  current_cluster_number = find_empty_fat_index(volume);

  write_in_fat_index(volume, before_cluster_number, current_cluster_number);
  write_in_fat_index(volume, current_cluster_number, end_cluster);

  wirte_position = read_cluster(volume, current_cluster_number);

#ifdef __KERNEL__
  seek_volume(volume, wirte_position);
#else
  seek_volume(volume, wirte_position, SEEK_SET);
#endif
  write_volume(volume, dirent, sizeof(struct mfs_dirent), 1);

  printf("alloc_new_dirent %d %d\n", current_cluster_number, wirte_position);

  return TRUE;
}

/*
 함수명  : getFirstEntry
 하는일  : 클러스터의 처음 엔트리를 얻는다.
 이전 클러스터에서 LongFileName Entry가 계속되고 있는지 여부를 bLongFileNameContinueToNextCluster로
 알아낸다.
 nCurEntryNumber에는 현재 Entry의 수치를 넣어준다.
 인자    : pCluster : 클러스터의 포인터
 nCurEntryNumber : 현재 엔트리의 번호
 bLongFileNameContinueToNextCluster : LongFileName이 다음 클러스터까지 연속되어졌는지 여부
 리턴 값 : struct mfs_dirent*
 */

struct mfs_dirent* get_first_entry (pu8_t cluster, u32_t* current_entry_number, BOOL has_long_file_name_next_entry) {
  struct mfs_LFN_entry* current_long_file_name_entry = NULL;
  (*current_entry_number) = 0;

  if (has_long_file_name_next_entry == TRUE) {
    do {
      ++(*current_entry_number);

    }
    while (current_long_file_name_entry->id != 0x40);
  }

  return (struct mfs_dirent *) &cluster[(*current_entry_number)
      * sizeof(struct mfs_dirent)];
}

/*
 함수명  : getNextEntry
 하는일  : 클러스터의 다음 엔트리를 얻는다.
 nCurEntryNumber를 토대로 다음 엔트리를 구한다.
 LongFileName일시 LongFileName이 끝날때까지 Cluster를 검색한다.
 인자    : pCluster : 클러스터의 포인터
 nCurEntryNumber : 현재 엔트리의 번호
 bLongFileNameContinueToNextCluster : LongFileName이 다음 클러스터까지 연속되어졌는지 여부
 리턴 값 : struct mfs_dirent*
 */

struct mfs_dirent* get_next_entry (pu8_t cluster, u32_t* current_entry_number, BOOL* has_long_file_name_next_entry) {
  const u32_t entry_per_data_cluster = CLUSTER_SIZE / sizeof(struct mfs_dirent);
  struct mfs_dirent* current_dir_entry =
      (struct mfs_dirent *) &cluster[(*current_entry_number)
	  * sizeof(struct mfs_dirent)];
  struct mfs_LFN_entry* current_long_file_name_entry = NULL;

  if (is_long_file_name(current_dir_entry->attribute) == TRUE) {
    do {
      ++(*current_entry_number);

      // LongFileName이 다음 클러스터로 넘어갈 경우를 대비
      if ((*current_entry_number) >= entry_per_data_cluster) {
	(*has_long_file_name_next_entry) = TRUE;

	return NULL;
      }

      current_long_file_name_entry =
	  (struct mfs_LFN_entry *) &cluster[(*current_entry_number)
	      * sizeof(struct mfs_LFN_entry)];

    }
    while (current_long_file_name_entry->id != 0x40);
  }

  ++(*current_entry_number);

  // EntryNumber가 클러스터안의 한계 Entry 수치를 넘었을 경우
  if ((*current_entry_number) >= entry_per_data_cluster) {
    has_long_file_name_next_entry = FALSE;

    return NULL;
  }

  current_dir_entry = (struct mfs_dirent *) &cluster[(*current_entry_number)
      * sizeof(struct mfs_dirent)];

  return current_dir_entry;
}

/*
 함수명  : write_fileDirectoryEntryInDirCluster
 하는일  : 디렉토리 클러스터 안에 파일 디렉토리 엔트리를 찾는다.
 디렉토리가 가지고 있는 모든 클러스터를 순환하여
 파일 이름과 같은 엔트리를 찾는다.
 인자    : fVolume : 루프백이미지/파일 볼륨의 포인터
 nDirClusterNumber : 디렉토리 클러스터 번호
 pFileName : 파일이름의 문자열 포인터
 pSearchedDirEntry : 검색된 디렉토리 엔트리의 포인터
 리턴 값 : BOOL
 */
BOOL alloc_new_entry (struct mfs_volume* volume, u128 dir_cluster_number, ps16_t file_name, struct mfs_dirent* searched_dentry) {
  u8_t cluster[CLUSTER_SIZE] = { 0, };
  const u32_t entry_per_data_cluster = CLUSTER_SIZE / sizeof(struct mfs_dirent);
  s16_t composited_file_name[128] = { 0, };
  BOOL has_long_file_name_next_entry = FALSE;
  u128 read_position = 0;
  u128 current_cluster_number = dir_cluster_number;
  u32_t current_entry_number = 0;
  struct mfs_dirent* current_dentry = NULL;
  u128 end_cluster = get_end_cluster(volume);

  // 디렉토리의 모든 클러스터를 검사한다.
  while (current_cluster_number != end_cluster) {

    read_position = read_cluster(volume, current_cluster_number);

#ifdef __KERNEL__
    seek_volume(volume, read_position);
#else
    seek_volume(volume, read_position, SEEK_SET);
#endif
    read_volume(volume, cluster, sizeof(u8_t), CLUSTER_SIZE);

    current_dentry = get_first_entry(cluster, &current_entry_number,
				     has_long_file_name_next_entry);
    printf("alloc_new_entry current_cluster_number: %d\n",
	   current_cluster_number);

    while (current_entry_number != entry_per_data_cluster) {

      if (is_normal_file(current_dentry->attribute) == TRUE) {

	// 얻은 엔트리가 LongFileName인지 여부 검사
	if (is_long_file_name(current_dentry->attribute) == TRUE) {
	  // LongFileName일 경우 LongFileName을 조합한다.
	  composite_long_file_name(volume, current_cluster_number,
				   current_entry_number, composited_file_name);
	}
	else {
	  // 일반 FileName일 경우 복사
	  strcpy(composited_file_name, current_dentry->name);
	}

	// Name 비교
	if (!strcmp(file_name, composited_file_name)) {
	  memcpy(current_dentry, searched_dentry, sizeof(struct mfs_dirent));
#ifdef __KERNEL__
	  seek_volume(volume, read_position);
#else
	  seek_volume(volume, read_position, SEEK_SET);
#endif
	  write_volume(volume, cluster, sizeof(u8_t), CLUSTER_SIZE);
	  printf("mfs inner dentry update\n");

	  return TRUE;
	}
      }

      // 다음 엔트리를 얻는다.
      current_dentry = get_next_entry(cluster, &current_entry_number,
				      &has_long_file_name_next_entry);
    }

    current_cluster_number = read_fat_index(volume, current_cluster_number);
  }

  return FALSE;
}

/*
 함수명  : getVolumeReadPos
 하는일  : Volume에서 nClusterNum 즉 클러스터 번호의 실제 위치를 읽는다.
 인자    : fVolume : 루프백 이미지/파일 볼륨의 포인터
 nClusterNum : 클러스터 번호
 리턴 값 : u32
 */

u128 read_cluster (struct mfs_volume* volume, u128 cluster_number) {
  struct mfs_sb_info sb;
  read_sb(volume, &sb);

  // Volume에서 읽을 위치를 계산한다. DataCluster Begin Address + 건내져온 클러스터 수치까지의 크기(nCurClusterNumber - 2는 Index 0,1은 쓰이지 않기 때문이다. 시작이 2부터이다.)
  return ((sb.data_cluster_sector * sb.bytes_per_sector)
      + ((cluster_number - 2) * CLUSTER_SIZE));
}

BOOL is_empty_entry(struct mfs_dirent* dentry) {
  if (!strlen(dentry->name))
    return TRUE;
  return FALSE;
}

