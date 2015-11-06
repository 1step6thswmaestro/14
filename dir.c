#ifdef __KERNEL__
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#endif

#include "fat.h"
#include "entry.h"
#include "file.h"
#include "namei.h"

#ifdef __KERNEL__
#define printf printk
#endif

#ifdef __KERNEL__
//#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 3)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0)
#define f_dentry	f_path.dentry
#endif
#endif

/*
   함수명  : composite_long_file_name
   하는일  : 디렉토리 클러스터안에 있는 지정된 엔트리부터 시작되는 긴 파일 이름을 조합한다.
   인자    : fVolume : 루프백이미지/볼륨의 파일 포인터
nDirClusterNumber : 디렉토리 클러스터 번호
nLongFileEntryNumber : 긴 파일 엔트리의 번호
pCompositeFileName: 파일이름의 문자열 포인터
리턴 값 : BOOL
 */

BOOL composite_long_file_name(struct mfs_volume* volume, u128 dir_cluster_number, u32_t long_file_entry_number, ps16_t composited_file_name)
{
	u8_t cluster[CLUSTER_SIZE] = {0, };
	const u32_t entry_per_data_cluster = CLUSTER_SIZE / sizeof(struct mfs_dirent);
	u128 read_position = 0;
	u128 current_cluster_number = dir_cluster_number;
	u32_t current_entry_number = long_file_entry_number;
	struct mfs_dirent*  dentry = (struct mfs_dirent *)&cluster[current_entry_number * sizeof(struct mfs_dirent)];
	struct mfs_LFN_entry* current_long_file_name_entry = NULL;
	u128 end_cluster = get_end_cluster(volume);
	u128 bad_cluster = get_bad_cluster(volume);

	// 조합할 엔트리가 LongFileName이 맞는지 검증한다.
	if(is_long_file_name(dentry->attribute) == FALSE)
	{
		return FALSE;
	}

	strcpy(composited_file_name, dentry->name);
	++current_entry_number;

	// 조합할 엔트리가 다음 클러스터까지 연속되어질 경우를 대비한다.
	while(current_cluster_number != end_cluster)
	{
		read_position = read_cluster(volume, current_cluster_number);

		#ifdef __KERNEL__
		seek_volume(volume, read_position);
		#else
		seek_volume(volume, read_position, SEEK_SET);
		#endif
		read_volume(volume, cluster, sizeof(u8_t), CLUSTER_SIZE);

		// 현재 클러스터를 순환하며 조합한다.
		while(current_entry_number < entry_per_data_cluster)
		{
			current_long_file_name_entry = (struct mfs_LFN_entry *)&cluster[current_entry_number * sizeof(struct mfs_LFN_entry)];

			strcat(composited_file_name, current_long_file_name_entry->name);

			if(current_long_file_name_entry->id == 0x40)
				return TRUE;

			++current_entry_number;
		}

		// 다음 클러스터까지 LongFileName이 이어진다.
		current_cluster_number = read_fat_index(volume, current_cluster_number);

		if(current_cluster_number == bad_cluster)
		{
			return FALSE;
		}
	}

	return FALSE;
}

#ifdef __KERNEL__

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)

void __mfs_readdir(const ps16_t route, struct file *file, struct dir_context *ctx)
{
	u8_t cluster[CLUSTER_SIZE] = {0, };
	const u32_t entry_per_data_cluster = CLUSTER_SIZE / sizeof(struct mfs_dirent);
	s16_t composited_file_name[128] = {0, };
	BOOL has_long_file_name_entry = FALSE;
	u128 current_cluster_number = 0;
	u32_t current_entry_number = 0;
	u128 read_position = 0;
	struct mfs_dirent* current_dirent = NULL;
	static s16_t path[1024]={0};
	struct dentry *de = file->f_dentry;
	struct mfs_volume* volume = de->d_sb->s_fs_info;
	u128 end_cluster = get_end_cluster(volume);

	strncpy(path, route, 1024);
	current_cluster_number = get_cluster_number(volume, path);

	if(current_cluster_number == 0)
	{
		return ;
	}

	printk("__mfs_readdir\n");

	// 볼륨이 무효하다.
	if(volume == NULL)
		return ;

	read_position = read_cluster(volume, current_cluster_number);

#ifdef __KERNEL__
	seek_volume(volume, read_position);
#else
	seek_volume(volume, read_position, SEEK_SET);
#endif
	read_volume(volume, cluster, sizeof(u8_t), CLUSTER_SIZE);

	printk("current cluster number before : %d\n", current_cluster_number);

	while(current_cluster_number != end_cluster)
	{
		// 클러스터의 첫 엔트리를 얻는다.
		current_dirent = get_first_entry(cluster, &current_entry_number, has_long_file_name_entry);

		printk("current dentry : %x\n", current_dirent);

		// 클러스터의 모든 엔트리를 검사한다.
		while(current_entry_number != entry_per_data_cluster)
		{

			printk("current cluster number after : %d\n", current_cluster_number);

			// if(current_dirent->size != 0) {

				// printk("current_dentry size is NOT 0\n");

				// 얻은 엔트리가 LongFileName인지 여부 검사
				if(is_long_file_name(current_dirent->attribute) == TRUE) {
					// LongFileName일 경우 LongFileName을 조합한다.
					composite_long_file_name(volume, current_cluster_number, current_entry_number, composited_file_name);
				} else {
					// 일반 FileName일 경우 복사
					strcpy(composited_file_name, current_dirent->name);
				}

				if(is_normal_dir(current_dirent->attribute) == TRUE) {
					static char buf[1024]="";
					int len;
					buf[0]='\0';

					strcat(buf,path);
					len=strlen(path);

					if(len > 0){
						if(path[len-1] != '/') strcat(buf,"/");
					}

					strcat(buf, composited_file_name);
					if(!dir_emit(ctx, composited_file_name, strlen(composited_file_name), 2, DT_DIR)) {
						printk("WARNING %s %d", __FILE__, __LINE__);
						return ;
					}

					ctx->pos++;
					strcat(buf, "*<DIR>\n");
					printk(buf);
				} else if(is_normal_file(current_dirent->attribute) == TRUE) {
					static char buf[1024]="";
					buf[0]='\0';

					if(!dir_emit(ctx, composited_file_name, strlen(composited_file_name), 2, DT_REG)) {
						printk("WARNING %s %d", __FILE__, __LINE__);
						return ;
					}
					ctx->pos++;
					strcat(buf, composited_file_name);
					strcat(buf, "*<FILE>\n");
					printk(buf);
				}
			// }

			// 다음 엔트리를 얻는다.
			current_dirent = get_next_entry(cluster, &current_entry_number, &has_long_file_name_entry);
		}

		current_cluster_number = read_fat_index(volume, current_cluster_number);

		// 지정된 번호의 클러스터를 읽는다.
		read_position = read_cluster(volume, current_cluster_number);

#ifdef __KERNEL__
		seek_volume(volume, read_position);
#else
		seek_volume(volume, read_position, SEEK_SET);
#endif
		read_volume(volume, cluster, sizeof(u8_t), CLUSTER_SIZE);
	}

	return ;
}



#else

/*
   함수명  : show_dir_k
   하는일  : 지정한 경로의 디렉토리를 보여준다.
   그 디렉토리의 클러스터 수치를 구하고 클러스터의 모든 엔트리를 읽으며
   출력해준다.
   인자    : fVolume : 루프백이미지/볼륨의 파일 포인터
pRoute : 유니코드 경로의 문자열 포인터
리턴 값 :
 */

void __mfs_readdir(struct mfs_volume* volume, const ps16_t route, struct printdir *printdir)
{
	u8_t cluster[CLUSTER_SIZE] = {0, };
	const u32_t entry_per_data_cluster = CLUSTER_SIZE / sizeof(struct mfs_dirent);
	s16_t composited_file_name[128] = {0, };
	BOOL has_long_file_name_next_entry = FALSE;
	u128 current_cluster_number = 0;
	u32_t current_entry_number = 0;
	u128 read_position = 0;
	struct mfs_dirent* current_dirent = NULL;
	static s16_t path[1024]={0};
	struct file* filp = printdir->filp;
	void* dirent = printdir->dirent;
	filldir_t filldir = printdir->filldir;
	u128 end_cluster = get_end_cluster(volume);

	strncpy(path, route, 1024);
	current_cluster_number = get_cluster_number(volume, path);

	if(current_cluster_number == 0)
	{
		return ;
	}

	// 볼륨이 무효하다.
	if(volume == NULL)
		return ;

	read_position = read_cluster(volume, current_cluster_number);

	seek_volume(volume, read_position);
	read_volume(volume, cluster, sizeof(u8_t), CLUSTER_SIZE);

	while(current_cluster_number != end_cluster)
	{
		// 클러스터의 첫 엔트리를 얻는다.
		current_dirent = get_first_entry(cluster, &current_entry_number, has_long_file_name_next_entry);

		// 클러스터의 모든 엔트리를 검사한다.
		while(current_entry_number != entry_per_data_cluster)
		{
			// if(current_dirent->size != 0)
			// {
				// 얻은 엔트리가 LongFileName인지 여부 검사
				if(is_long_file_name(current_dirent->attribute) == TRUE)
				{
					// LongFileName일 경우 LongFileName을 조합한다.
					composite_long_file_name(volume, current_cluster_number, current_entry_number, composited_file_name);
				}
				else
				{
					// 일반 FileName일 경우 복사
					strcpy(composited_file_name, current_dirent->name);
				}

				if(is_normal_dir(current_dirent->attribute) == TRUE)
				{
					static char buf[1024] = "";
					int len;
					buf[0] = '\0';
					strcat(buf, path);
					len = strlen(path);
					if(len > 0){
						if(path[len-1] != '/') strcat(buf, "/");
					}
					strcat(buf, composited_file_name);
					if(filldir(dirent, composited_file_name, strlen(composited_file_name), filp->f_pos++, 2, DT_DIR)){
						printk("WARNING %s %d", __FILE__,__LINE__);
						return ;
					}
					strcat(buf, "*<DIR>\n");
					printk(buf);
				}

				else if(is_normal_file(current_dirent->attribute) == TRUE)
				{
					static char buf[1024]="";
					buf[0]='\0';
					printk("garig %p %p %p %s\n", filp, dirent, filldir, composited_file_name);
					if(filldir(dirent, composited_file_name, strlen(composited_file_name), filp->f_pos++, 2, DT_REG)){
						printk("WARNING %s %d", __FILE__, __LINE__);
						return ;
					}
					strcat(buf, composited_file_name);
					strcat(buf, "*<FILE>\n");
					printk(buf);
				}
			// }

			// 다음 엔트리를 얻는다.
			current_dirent = get_next_entry(cluster, &current_entry_number, &has_long_file_name_next_entry);
		}

		current_cluster_number = read_fat_index(volume, current_cluster_number);

		// 지정된 번호의 클러스터를 읽는다.
		read_position = read_cluster(volume, current_cluster_number);

		seek_volume(volume, read_position);
		read_volume(volume, cluster, sizeof(u8_t), CLUSTER_SIZE);
	}

	return ;
}

#endif
#endif

#ifdef __KERNEL__
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)

static int mfs_readdir(struct file *file, struct dir_context *ctx)
{
	printk("\t\t\t\t\t\t\t\t\t\tMFS READDIR\n");

	char buf[512] = {0};
	struct dentry *de = file->f_dentry;

	get_file_path_from_dentry(de, buf, 512);
	printk("\t\t\t\t\t\t\t\t\t\tMFS READDIR: %s\n", buf);

	if(ctx->pos > 0) {
		printk("\t\t\t\t\t\t\t\t\t\tMFS READDIR: OUT 1 %d\n", ctx->pos);
		return 1;
	}

	if(!dir_emit_dots(file, ctx)) {
		printk("\t\t\t\t\t\t\t\t\t\tMFS READDIR: OUT 0\n");
		return 0;
	}
	
	printk("\t\t\t\t\t\t\t\t\t\tMFS READDIR: RUN READ\n");


	__mfs_readdir(buf, file, ctx);

	return 1;
}

#else

/**
	@brief	readdir operation

	디렉토리 filp를 읽어서 해당 디렉토리 내의 파일,디렉토리 정보를 filldir함수를 이용하여 dirent에 기록
	@param
			struct file	*filp	readdir을 수행한 디렉토리의 커널내 파일 포인터
			void		*dirent	파일,디렉토리 정보가 저장 될 포인터
			filldir_t	filldir	dirent에 파일,디렉토리 정보를 저장할 때 사용할 함수포인터
	@return	성공,정상종료시 1, 실패시 0
*/
static int mfs_readdir(struct file *filp, void* dirent, filldir_t filldir)
{
	printk("\t\t\t\t\t\t\t\t\t\tMFS READDIR\n");

	char buf[512]={0};
	struct dentry *de = filp->f_dentry;
	//struct dentry *new_dentry;

	struct printdir printdir;
	get_file_path_from_dentry(de,buf,512);
	printk("orig  %p %p %p \n",filp,dirent,filldir);
	printdir.filp=filp;
	printdir.dirent=dirent;
	printdir.filldir=filldir;

	printk( "rkfs: filp_operations.readdir called %s %d %p\n",de->d_name.name,(int)filp->f_pos,dirent);
	if(filp->f_pos > 0 )
		return 1;
	if(filldir(dirent, ".", 1, filp->f_pos++, de->d_inode->i_ino, DT_DIR)||
			(filldir(dirent, "..", 2, filp->f_pos++, de->d_parent->d_inode->i_ino, DT_DIR)))
		return 0;

	__mfs_readdir(de->d_sb->s_fs_info, buf, &printdir);


	return 1;
}

#endif

struct file_operations mfs_dir_operations = {
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)
	.iterate		=	mfs_readdir,
	#else
	.readdir		=	mfs_readdir,
	#endif
};
#endif


/*
   함수명  : is_normal_dir
   하는일  : 속성이 보통 폴더 속성인지 검사한다.
   인자    : nAttribute : 1바이트의 속성 값
   리턴 값 : BOOL
 */

BOOL is_normal_dir(u8_t attribute)
{
	if((attribute & 0x000000FF) == normal_dir)
		return TRUE;
	return FALSE;
}


/*
   함수명  : IsDeletedFolder
   하는일  : 속성이 삭제된 폴더 속성인지 검사한다.
   인자    : u8 nAttribute : 1바이트의 속성 값
   리턴 값 : BOOL
 */

BOOL is_deleted_dir(u8_t attribute)
{
	if((attribute & 0x000000FF) == deleted_dir)
		return TRUE;
	return FALSE;
}

/*
   함수명  : setNormalFolderAttribute
   하는일  : 디렉토리 엔트리에 보통 폴더 속성을 부여한다.
   인자    : pDirectoryEntry : 디렉토리 엔트리의 포인터
   리턴 값 : void
 */

void set_normal_dir_attribute(struct mfs_dirent* dentry)
{
	dentry->attribute &= 0xffffff00;
	dentry->attribute += normal_dir;
}

/*
   함수명  : create_directory
   하는일  : 디렉토리를 생성할 디렉토리의 클러스터 수치를 구한다.
   추가할 디렉토리 엔트리를 초기화한다.(Cluster 할당 등)
   찾은 디렉토리 클러스터에 디렉토리 엔트리를 추가한다.
   인자    : fVolume  : 루프백이미지/볼륨의 파일 포인터
pRoute   : 유니코드 경로의 문자열 포인터
pDirName : 디렉토리 이름의 문자열 포인터
리턴 값 : BOOL
 */

BOOL __mfs_mkdir(struct mfs_volume* volume, ps16_t path, ps16_t dir_name)
{

	u128 cluster_number = 0;
	u128 end_cluster = get_end_cluster(volume);

	struct mfs_dirent new_dentry ;
	memset(&new_dentry, 0x00, sizeof(struct mfs_dirent));

	cluster_number = get_cluster_number(volume, path);

	// 삽입할 디렉토리 엔트리를 초기화한다.
	set_normal_dir_attribute(&new_dentry);
	strcpy(new_dentry.name, dir_name);
	new_dentry.head_cluster_number = find_empty_fat_index(volume);
	new_dentry.size = 0;
	write_in_fat_index(volume, new_dentry.head_cluster_number, end_cluster);

	// 디렉토리 엔트리를 삽입한다.
	if(alloc_new_dirent(volume, cluster_number, &new_dentry, dir_name) == FALSE)
		return FALSE;
	return TRUE;
}

/*
   함수명  : exist_in_dir
   하는일  : 지정한 경로의 디렉토리를 보여준다.
   그 디렉토리의 클러스터 수치를 구하고 클러스터의 모든 엔트리를 읽으며
   출력해준다.
   인자    : fVolume : 루프백이미지/볼륨의 파일 포인터
pRoute : 유니코드 경로의 문자열 포인터
리턴 값 :
 */

int __mfs_lookup(struct mfs_volume* volume, const ps16_t route, const ps16_t file_name)
{
	u8_t cluster[CLUSTER_SIZE] = {0, };
	const u32_t entry_per_data_cluster = CLUSTER_SIZE / sizeof(struct mfs_dirent);
	s16_t composited_file_name[128] = {0, };
	BOOL has_long_file_name_next_entry = FALSE;
	u128 current_cluster_number = 0;
	u32_t current_entry_number = 0;
	u128 read_position = 0;
	struct mfs_dirent* current_dentry = NULL;
	static s16_t path[1024]={0};
	u128 end_cluster = get_end_cluster(volume);

	strncpy(path, route, 1024);
	current_cluster_number = get_cluster_number(volume, route);

//	if(current_cluster_number == 0)
//	{
//		return 0;
//	}

	// 볼륨이 무효하다.
	if(volume == NULL)
		return 0;

	read_position = read_cluster(volume, current_cluster_number);
#ifdef __KERNEL__
	seek_volume(volume, read_position);
#else
	seek_volume(volume, read_position, SEEK_SET);
#endif
	read_volume(volume, cluster, sizeof(u8_t), CLUSTER_SIZE);

	while(current_cluster_number != end_cluster)
	{
		// 클러스터의 첫 엔트리를 얻는다.
		current_dentry = get_first_entry(cluster, &current_entry_number, has_long_file_name_next_entry);

		// 클러스터의 모든 엔트리를 검사한다.
		while(current_entry_number != entry_per_data_cluster)
		{
			// 얻은 엔트리가 LongFileName인지 여부 검사
			if(is_long_file_name(current_dentry->attribute) == TRUE)
			{
				// LongFileName일 경우 LongFileName을 조합한다.
				composite_long_file_name(volume, current_cluster_number, current_entry_number, composited_file_name);
			}
			else
			{
				// 일반 FileName일 경우 복사
				strcpy(composited_file_name, current_dentry->name);
			}

			if(strcmp(composited_file_name, file_name) == 0){
				if(is_normal_dir(current_dentry->attribute) == TRUE){
					return DIR_DENTRY;
				}
				else if(is_normal_file(current_dentry->attribute) == TRUE){
					return FILE_DENTRY;
				}
			}

			// 다음 엔트리를 얻는다.
			current_dentry = get_next_entry(cluster, &current_entry_number, &has_long_file_name_next_entry);
		}

		current_cluster_number = read_fat_index(volume, current_cluster_number);

		// 지정된 번호의 클러스터를 읽는다.
		read_position = read_cluster(volume, current_cluster_number);

#ifdef __KERNEL__
		seek_volume(volume, read_position);
#else
		seek_volume(volume, read_position, SEEK_SET);
#endif
		read_volume(volume, cluster, sizeof(u8_t), CLUSTER_SIZE);
	}

	return 0;
}

void print_dentry(struct mfs_dirent* dentry)
{
	u8_t attribute;
	int i;

	printf("- Directory Entry -\n");
	printf("Attribute:           %d %X\n", dentry->attribute, dentry->attribute);
	printf("Extended Attribute:  %d %X\n", dentry->extended_attribute, dentry->extended_attribute);
	printf("name:                ");
	for(i=0; i<24; i++){
		printf("%c", dentry->name[i]);
	}
	printf("\n");
	for(i=0; i<24; i++){
		printf("%02X ", dentry->name[i]);
	}
	printf("\n");

	printf("head_cluster_number: %d %X\n", dentry->head_cluster_number, dentry->head_cluster_number);
	printf("size:                %d %X\n", dentry->size, dentry->size);

	printf("reserved:\n");
	for(i=0; i<23; i++){
		printf("%02X ", dentry->reserved[i]);
	}

	printf("\n\n\n");
}