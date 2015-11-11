#include <getopt.h>
#include <errno.h>
#include <ctype.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <sys/syscall.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include "../util.h" //x
#include "../volume.h"
#include "../dir.h" //a
#include "mkfs.h"
#include "../sqliteio.h"

static void print_usage(int ret)
{
  fprintf(stderr, "Usage: mkfs.mfs <options>\n");
  fprintf(stderr, "options:\n");
  fprintf(stderr, "--auto                    create a loopback file itself\n");
  fprintf(stderr, "\t-n|--name NAME          set a volume name\n");
  fprintf(stderr, "\t-s|--size SIZE          total number of bytes in the filesystem\n");
  fprintf(stderr, "\t-c|--clustersize SIZE   size of a data cluster\n");

  exit(ret);
}

static void make_data_cluster(struct mfs_volume* volume, u128 volume_size, u128 cluster_size)
{
  pu8_t buf = (pu8_t) malloc(cluster_size);
  memset(buf, 0, cluster_size);

  for (u32_t i = 1; i <= (volume_size / cluster_size); i++) 
  {
    write_volume(volume, buf, sizeof(u8_t), cluster_size);
  }

  free(buf);
}

//static void initialize_sb(struct mfs_sb_info *sb, ps8_t volume_name, u128 volume_size)
//{
//
//	struct mfs_sb_info_meta meta;
//	struct mfs_sb_info_reserved reserved;
//	struct mfs_sb_info_eos eos;
//
//	u128 total_sectors = COUNT_TOTAL_SECTOR(volume_size);
//	s8_t random_number = 0;
//
//	strcpy(meta->volume_label, volume_name);
//	srand((unsigned)time(NULL));
//
//	for (u32_t i = 0; i < sizeof(meta->volume_serial_number); i++) {
//		random_number = rand() % 36;
//		// [0x30 ~ 0x39] ASCII Number : 0~9(10)    [0x41 ~ 0x5A] ASCII Number : A~Z(26)
//		if(random_number < 10) {
//			meta->volume_serial_number[i] = random_number + 0x30;
//		} else {
//			meta->volume_serial_number[i] = (random_number - 10) + 0x41;
//		}
//	}
//
//	meta->bytes_per_sector 						= BYTES_PER_SECTOR;
//	meta->sectors_per_cluster 					= SECTORS_PER_CLUSTER;
//	meta->copies_of_fat 							= 2;
//	meta->sectors_of_fat 							= (u16_t) SECTORS_OF_FAT(COUNT_TOTAL_CLUSTER(total_sectors));
//	meta->reserved_bytes							= 0;
//	meta->version_major 							= VERSION_MAJOR;
//	meta->version_minor 							= VERSION_MINOR;
//	meta->version_build 							= VERSION_BUILD;
//	meta->fat_sector								= SECTORS_OF_SUPER_BLK;
//	meta->data_cluster_sector 					= SECTORS_OF_SUPER_BLK + (meta->sectors_of_fat * meta->copies_of_fat);
//	meta->fat_index_size 							= mfs_fat_index_size(COUNT_TOTAL_CLUSTER(total_sectors));
//	meta->total_sectors_high						= 0;
//	meta->total_sectors_low 						= total_sectors;
//	meta->flag_total_sectors_high 				= 0;
//	meta->total_data_cluster_sectors_high			= 0;
//	meta->total_data_cluster_sectors_low 			= COUNT_TOTAL_CLUSTER(total_sectors);
//	meta->flag_total_data_cluster_sectors_high	= 0;
//
//	memset(reserved->reserved_bytes2, '\0', RESERVED_BYTES2_SIZE);
//
//	eos->end_of_sb								= 0;
//}

static void initialize_sb(struct mfs_sb_info *sb, ps8_t volume_name, u128 volume_size)
{
  u128 total_sectors = COUNT_TOTAL_SECTOR(volume_size);
  s8_t random_number = 0;

  strcpy(sb->volume_label, volume_name);
  srand((unsigned)time(NULL));

  for (u32_t i = 0; i < sizeof(sb->volume_serial_number); i++)
  {
    random_number = rand() % 36;
    // [0x30 ~ 0x39] ASCII Number : 0~9(10)    [0x41 ~ 0x5A] ASCII Number : A~Z(26)

    if(random_number < 10) 
    {
      sb->volume_serial_number[i] = random_number + 0x30;
    }
    else 
    {
      sb->volume_serial_number[i] = (random_number - 10) + 0x41;
    }
  }

	sb->bytes_per_sector											= BYTES_PER_SECTOR;
	sb->sectors_per_cluster										= SECTORS_PER_CLUSTER;
	sb->copies_of_fat													= 2;
	sb->sectors_of_fat												= (u16_t) SECTORS_OF_FAT(COUNT_TOTAL_CLUSTER(total_sectors));
	sb->reserved_bytes												= 0;
	sb->version_major								 					= VERSION_MAJOR;
	sb->version_minor 												= VERSION_MINOR;
	sb->version_build 												= VERSION_BUILD;
	sb->fat_sector														= SECTORS_OF_SUPER_BLK;
	sb->data_cluster_sector 									= SECTORS_OF_SUPER_BLK + (sb->sectors_of_fat * sb->copies_of_fat);
	sb->fat_index_size 												= mfs_fat_index_size(COUNT_TOTAL_CLUSTER(total_sectors));
	sb->total_sectors_high										= 0;
	sb->total_sectors_low 										= total_sectors;
	sb->flag_total_sectors_high 							= 0;
	sb->total_data_cluster_sectors_high				= 0;
	sb->total_data_cluster_sectors_low 				= COUNT_TOTAL_CLUSTER(total_sectors);
	sb->flag_total_data_cluster_sectors_high	= 0;
	memset(sb->reserved_bytes2, '\0', 416);
	sb->end_of_sb															= 0;

}

static void make_fat_index(struct mfs_volume* volume, struct mfs_sb_info* sb, u32_t index, u64_t size)
{
  u8_t first_sector[BYTES_PER_SECTOR] = { 0, };
  u32_t start_position = 0;
  u32_t end_position = 0;
  u128 index_position = 0;

  if (volume == NULL) {
    printf("ERROR: Volume is empty!\n");
    exit(1);
  }

  seek_volume(volume, 0, SEEK_SET);
  read_volume(volume, first_sector, sizeof(u8_t), sizeof(first_sector));

  start_position = (sb->fat_sector * sb->bytes_per_sector);
  end_position = (sb->fat_sector + (sb->sectors_of_fat * sb->copies_of_fat))
      * sb->bytes_per_sector;

  // Volume에 적을 위치를 계산한다. FAT Begin Address + FAT Index Address(Index - 2는 Index 0,1은 쓰이지 않기 때문이다. 시작이 2부터이다.)
  index_position = start_position + (sb->fat_index_size * (index - 2));

  if (index_position > end_position) {
    printf("ERROR: Can't create a file allocation table index!\n");
    exit(1);
  }

  seek_volume(volume, index_position, SEEK_SET);
  write_volume(volume, &size, sizeof(u32_t), 1);
}

static void make_fat(struct mfs_volume* volume, struct mfs_sb_info* sb, u16_t index_size)
{
  u128 end_cluster = get_end_cluster(volume);
  make_fat_index(volume, sb, 2, end_cluster);
}

static struct mfs_volume* open_device(ps8_t device_name, u128* device_size)
{
  struct mfs_volume* new_volume = NULL;

  new_volume = open_volume(device_name, "r+b");

  if (new_volume == NULL) {
    printf("ERROR: %s doesn't exist\n", device_name);
    exit(1);
  }

  seek_volume(new_volume, 0L, SEEK_END);
  *device_size = ftell(new_volume->fp);

  return new_volume;
}

static struct mfs_volume* create_loopback(ps8_t volume_name)
{
  struct mfs_volume* new_volume = NULL;

  new_volume = open_volume(volume_name, "rb");
  if (new_volume != NULL) {
    printf("ERROR: %s is already exist!\n", volume_name);
    exit(1);
  }

  new_volume = open_volume(volume_name, "w+b");
  if (new_volume == NULL) {
    printf("ERROR: Failed on creating the new volume\n");
    exit(1);
  }

  return new_volume;
}

static void create_volume(struct mfs_volume* new_volume, ps8_t volume_name, u128 volume_size, u128 cluster_size)
{
  u8_t first_sector[BYTES_PER_SECTOR] = { 0, };

  if (volume_name == NULL) {
    volume_name = "";
  }

  if (new_volume == NULL) {
    printf("ERROR: Volume is empty!\n");
    exit(1);
  }

  make_data_cluster(new_volume, volume_size, cluster_size);

  struct mfs_sb_info* sb = (struct mfs_sb_info *) first_sector;
  initialize_sb(sb, volume_name, volume_size);

  seek_volume(new_volume, 0, SEEK_SET);
  write_volume(new_volume, sb, sizeof(u8_t), sizeof(struct mfs_sb_info));

  make_fat(new_volume, sb, sb->fat_index_size);

  // Read Dump SQLite
  FILE *fp;
  u32_t total_size;
  u32_t n_size;

  fp = fopen("./test.db", "r");

  fseek(fp, 0, SEEK_END);
  total_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char* buff = (char *) malloc(total_size);

  while (1) {
    n_size = fread(buff, sizeof(u8_t), total_size, fp);

    if (n_size <= 0) {
      break;
    }
  }

  create_dummy(new_volume, "/", "test.db", buff, total_size, 0);


  fp = fopen("./test1.db", "r");

  fseek(fp, 0, SEEK_END);
  total_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char* buff2 = (char *) malloc(total_size);

  while (1) {
    n_size = fread(buff2, sizeof(u8_t), total_size, fp);

    if (n_size <= 0) {
      break;
    }
  }

  create_dummy(new_volume, "/", "test1.db", buff2, total_size, 0);

  // SQLite Dummy Buffer Free
  free(buff);
  fclose(fp);

  printf("\n\n\n");
}

void create_dummy(struct mfs_volume* volume, char* route, char* file_name, char* buff, int len, u64_t offset)
{
  int n_write;

  create_sqlite_file(volume, route, file_name);

  while (len > 0) {
    n_write = write_sqlite_file(volume, route, file_name, buff, len, offset);

    buff += n_write;
    offset += n_write;
    len -= n_write;

    if (n_write <= 0)
      break;
  }
}

int mfs_format(int argc, char **argv)
{
  ps8_t volume_name = NULL;
  u128 volume_size = 0;
  u128 cluster_size = 0;
  struct mfs_volume* volume = NULL;
  static int auto_create_loopback_flag = 0;
  while (1) {
    int c;
    static const struct option long_options[] = { { "auto", no_argument,
	&auto_create_loopback_flag, 1 },
	{ "name", required_argument, NULL, 'n' }, { "size", required_argument,
	    NULL, 's' }, { "clustersize", required_argument, NULL, 'c' }, {
	    NULL, 0, NULL, 0 } };
    c = getopt_long(argc, argv, "n:s:c:", long_options, NULL);

    if (c < 0)
      break;

    switch (c) {
      case 0:
	break;
      case 'n':
	volume_name = strdup(optarg);
	printf("volume_name : \n");
	break;
      case 's':
	volume_size = parse_size(optarg);
	printf("volume_size : \n");
	break;
      case 'c':
	cluster_size = parse_size(optarg);
	printf("cluster size\n");
	break;
      case GETOPT_VAL_HELP:
      default:
	print_usage(c != GETOPT_VAL_HELP);
    }
  }
  if (auto_create_loopback_flag) {
    if (volume_name == NULL) {
      printf("ERROR: Set the volume name\n");
      return 0;
    }
    if (volume_size <= 0) {
      printf("ERROR: Set the correct volume size\n");
      return 0;
    }

    volume = create_loopback(volume_name);

  }
  else {
    char* device_name = argv[0];
    if (device_name == NULL) {
      printf("ERROR: Set the correct device name\n");
      return 0;
    }
    volume = open_device(device_name, &volume_size);
  }
  if (cluster_size <= 0) {
    printf("ERROR: Set the correct cluster size\n");
    return 0;
  }
  create_volume(volume, volume_name, volume_size, cluster_size);
  if (volume != NULL) {
    print_volume_status(volume);
    free_volume(volume);
  }

  return 1;
}
