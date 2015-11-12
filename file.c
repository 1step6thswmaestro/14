#ifdef __KERNEL__
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/version.h>
#endif

#include "fat.h"
#include "entry.h"
#include "namei.h"
#include "file.h"

#ifdef __KERNEL__
#define printf printk
#endif

//#ifdef __KERNEL__
//static atomic_t available = ATOMIC_INIT(1);
//#endif

#ifdef __KERNEL__
static loff_t mfs_lseek(struct file *filp, loff_t offset, int whence) {
  printk("\t\t\t\t\t\t\t\t\t\tMFS LSEEK\n");

  loff_t newpos;
  switch(whence) {
    case 0: /* SEEK_SET */
      newpos = offset;
      break;
    case 1: /* SEEK_CUR */
      newpos = filp->f_pos + offset;
      break;
    case 2: /* SEEK_END */
      //newpos = dev->size + offset;
      printf("SEEK_END err\n");
      break;
    default: /* can't happen */
      return -EINVAL;
  }
  if (newpos < 0) return -EINVAL;
  filp->f_pos = newpos;
  return newpos;
}

/**
	@brief	read operation

	filp의 offset에서부터 len만큼 읽어서 buf에 저장한다.
	@param
			struct file	*filp	read를 수행할 파일의 정보가 담긴 구조
			char __user	*buf	read동작을 한 결과를 저장할 버퍼,
								커널영역이 아니므로 __user 로 유저영역 메모리 공간임을 명시적으로 표기
			size_t		len		read를 수행 할 크기
			loff_t		*offset	filp에서 offset만큼 떨어진 곳에서 read를 수행한다.
								read후 read한 만큼 offset을 증가시킨다.
	@return	읽기에 성공한 byte수
*/
static ssize_t mfs_read(struct file* filp, char *buf, size_t len, loff_t *offset)
{
	printk("\t\t\t\t\t\t\t\t\t\tMFS READ\n");

	char* kernel_buf;
	int ret = 0;
	u128 cluster_number;
	struct mfs_dirent dentry;
	s16_t full_path[512] = {0,};
	s16_t route[128] = {0, };
	s16_t file_name[64] = {0, };
	void* volume = filp->f_path.dentry->d_sb->s_fs_info;

	printk("try to read offset: %d len: %d\n",(int)*offset,(int)len);

	kernel_buf = vmalloc(len);
	if(kernel_buf == NULL){
		return 0;
	}

	get_file_path_from_dentry(filp->f_path.dentry, full_path, 512);

	get_dir_path(full_path, route);
	get_file_name(full_path, file_name);

	cluster_number = get_cluster_number(volume, route);
	get_dentry(volume, cluster_number, file_name, &dentry);

	ret = read_file(volume, &dentry , kernel_buf , len, *offset);
	*offset += ret;

	copy_to_user(buf, kernel_buf, ret);
	vfree(kernel_buf);

	//printk("read %d byte now offset is %d\n", ret, (int)*offset);
	printk("kernel buf : %s\n", kernel_buf);
	return ret;
}

/**
	@brief	write operation

	filp의 offset에 len만큼 buf에서 읽어서 저장한다.
	@param
			struct file	*filp	write를 수행할 파일의 정보가 담긴 구조
			char __user	*buf	write를 할 데이터가 들어 있는 버퍼
								커널영역이 아니므로 __user 로 유저영역 메모리 공간임을 명시적으로 표기
			size_t		len		write를 수행 할 크기
			loff_t		*offset	filp에서 offset만큼 떨어진 곳에 write를 수행한다.
								write후 write한 만큼 offset을 증가시킨다.
	@return	쓰기에 성공한 byte수
*/
static ssize_t mfs_write(struct file* filp, const char *buf, size_t len, loff_t *offset)
{
	printk("\t\t\t\t\t\t\t\t\t\tMFS WRITE\n");

	char* kernel_buf;
	int ret = 0;
	u128 cluster_number;
	struct mfs_dirent dentry;
	s16_t full_path[512] = {0,};
	s16_t route[128] = {0, };
	s16_t file_name[64] = {0, };
	void* volume = filp->f_path.dentry->d_sb->s_fs_info;

	printk("try to write offset: %d len: %d\n", (int)*offset, (int)len);

	kernel_buf = vmalloc(len);
	if(kernel_buf == NULL){
		return 0;
	}

	get_file_path_from_dentry(filp->f_path.dentry, full_path, 512);

	printk("full_path: %s\n", full_path);

	get_file_name(full_path, file_name);
	get_dir_path(full_path, route);

	cluster_number = get_cluster_number(volume, route);
	get_dentry(volume, cluster_number, file_name, &dentry);

	copy_from_user(kernel_buf, buf, len);

	ret = write_file(volume, &dentry, kernel_buf, len, *offset);
	*offset +=ret;

	alloc_new_entry(volume, cluster_number, file_name, &dentry);
	vfree(kernel_buf);

	i_size_write(filp->f_path.dentry->d_inode, dentry.size);
	printk("write %d byte now offset is %d(size %d)\n", ret, (int)*offset, (int)i_size_read(filp->f_path.dentry->d_inode));
	return ret;
}


static int mfs_open(struct inode *inode, struct file *filp) {
  printk("\t\t\t\t\t\t\t\t\t\tMFS OPEN\n");

  s16_t full_path[512] = {0,};
  s16_t route[128] = {0,};
  s16_t file_name[64] = {0,};
  void* volume = filp->f_path.dentry->d_sb->s_fs_info;

  get_file_path_from_dentry(filp->f_path.dentry, full_path, 512);

  get_dir_path(full_path, route);
  get_file_name(full_path, file_name);

  if (__mfs_lookup(volume, route, file_name) != FILE_DENTRY) return -1;
  printf("found file: %s\n", file_name);
  return 0;
}

static int mfs_fsync(struct file *filp, loff_t start, loff_t end, int datasync) {
  printk("\t\t\t\t\t\t\t\t\t\tMFS FSYNC\n");

  int ret = 0;
//  struct file *host_file;
//  struct inode *inode = file_inode(filp);
//  //struct inode *coda_inode = file_inode(coda_file);
//  struct coda_file_info *cfi;
//
//  if (!(S_ISREG(inode->i_mode) || S_ISDIR(inode->i_mode) || S_ISLNK(inode->i_mode)))
//    return -EINVAL;
//
//  ret = filemap_write_and_wait_range(inode->i_mapping, start, end);
//  if (ret) return ret;
//  mutex_lock(&inode->i_mutex);
//
//  cfi = CODA_FTOC(coda_file);
//  BUG_ON(!cfi || cfi->cfi_magic != CODA_MAGIC);
//  host_file = cfi->cfi_container;
//
//  ret = vfs_fsync(host_file, datasync);
//  if (!ret && !datasync)
//    ret = venus_fsync(inode->i_sb, coda_i2f(inode));
//  mutex_unlock(&inode->i_mutex);

  return ret;
}


static int mfs_release(struct inode *inode, struct file *filp) {
  printk("\t\t\t\t\t\t\t\t\t\tMFS CLOSE\n");
  return 0;
}

struct file_operations mfs_file_operations = {
	.llseek		= mfs_lseek,
	.read           = mfs_read,//<파일에 대하여 read연산을 수행 했을 때 호출될 함수
	.write          = mfs_write,//<파일에 대하여 write연산을 수행 했을 때 호출될 함수
	.open		= mfs_open,
	.release	= mfs_release,
	.fsync		= mfs_fsync,
};
#endif

int read_file(struct mfs_volume* volume, struct mfs_dirent* dentry, char* buf,
	   	unsigned int len, unsigned int offset)
{

	//printk("read_file enter\n");

	unsigned long file_size = dentry->size;
	unsigned long pos = 0;
	unsigned int valid_len = len;
	unsigned int read_byte = 0;
	unsigned int local_len = 0;
	unsigned int cluster_offset = 0;

	#ifdef __x86_64__

	__uint128_t current_cluster = dentry->head_cluster_number;
	__uint128_t current_read_pos = read_cluster(volume, current_cluster);

	#else

	int64_t current_cluster = dentry->head_cluster_number;
	int64_t current_read_pos = read_cluster(volume, current_cluster);

	#endif

	//printk("file size : %d\n", file_size);
	//printk("current cluster before : %d\n", current_cluster);
	//printk("read_file : offset %d len %d filesize %d\n", offset, len, file_size);
	if(offset > file_size){
		return 0;
	}

	if ((offset + len) > file_size) {
		len = file_size - offset;
	}

	//goto valid positon
	while (offset >= (pos + CLUSTER_SIZE)) {
		current_cluster = read_fat_index(volume, current_cluster);
		pos = pos + CLUSTER_SIZE;
	}

	cluster_offset = offset % CLUSTER_SIZE;
	if(len + cluster_offset > CLUSTER_SIZE){
		local_len = CLUSTER_SIZE - cluster_offset;
	} else{
		local_len = len;
	}

	printf("read_file - current_cluster %d\n", (int) current_cluster);
	//printk("read_file : goto valid position:%d\n",(int)current_cluster);
	//printk("read_file : valid len%d\n",local_len);
	//read operation
	current_read_pos = read_cluster(volume, current_cluster) + cluster_offset;

	//printk("read_file1: read pos:%u vlen %d llen%u  readb%u\n", offset, valid_len, local_len, read_byte);

#ifdef __KERNEL__
	seek_volume(volume, current_read_pos);
#else
	seek_volume(volume, current_read_pos, SEEK_SET);
#endif
	read_volume(volume, buf, sizeof(u8_t), local_len);

	printf("file.c: read_file buf = %s\n", buf);
	return local_len;
}

int __mfs_create(struct mfs_volume* volume, ps16_t route, ps16_t file_name)
{

	printk("__mfs_creat %s\n", file_name);

	u32_t searched_cluster_number = 0;
	u128 end_cluster = get_end_cluster(volume);

	struct mfs_dirent new_dentry;
	memset(&new_dentry, 0x00, sizeof(struct mfs_dirent));

	searched_cluster_number = get_cluster_number(volume, route);

	// 삽입할 디렉토리 엔트리를 초기화한다.
	set_normal_file_attribute(&new_dentry);
	strcpy(new_dentry.name, file_name);
	new_dentry.head_cluster_number = find_empty_fat_index(volume);
	new_dentry.size = 0;
	write_in_fat_index(volume, new_dentry.head_cluster_number, end_cluster);

	if(alloc_new_dirent(volume, searched_cluster_number, &new_dentry, file_name) == FALSE)
		return FALSE;
	return TRUE;

}

int write_file(struct mfs_volume* volume, struct mfs_dirent* dentry, char* buf,
		unsigned int len, unsigned int offset)
{
	unsigned int end_cluster = get_end_cluster(volume);
	unsigned int file_size = dentry->size;
	unsigned int pos = 0;
	unsigned int valid_len = len;
	unsigned int cluster_offset;
	// 클러스터 하나가 차지하는 Bytes를 구한다.

	#ifdef __x86_64__

	__uint128_t current_cluster = dentry->head_cluster_number;
	__uint128_t current_write_pos = read_cluster(volume, current_cluster);

	#else

	int64_t current_cluster = dentry->head_cluster_number;
	int64_t current_write_pos = read_cluster(volume, current_cluster);

	#endif

	//printk("write_file : offset %d len %d file_size %d\n", offset, len, file_size);

	//no data to write
	if(len == 0){
		return 0;
	}

	//valid write length setting (per call -> write one cluster)
	cluster_offset = offset % CLUSTER_SIZE;

	if(len<CLUSTER_SIZE){
		valid_len = len;
		if(cluster_offset != 0){
			if(cluster_offset + valid_len > CLUSTER_SIZE){
				valid_len =valid_len - (CLUSTER_SIZE - cluster_offset + valid_len);
			}
		}
	}
	else{
		valid_len = CLUSTER_SIZE;
	}

	//goto valid positon
	while(offset >= pos + CLUSTER_SIZE){

		#ifdef __x86_64__
		__uint128_t next_cluster = read_fat_index(volume, current_cluster);
		#else
		int64_t next_cluster = read_fat_index(volume, current_cluster);
		#endif

		if (next_cluster == end_cluster){
			next_cluster = find_empty_fat_index(volume);

			write_in_fat_index(volume, current_cluster, next_cluster);
			write_in_fat_index(volume, next_cluster, end_cluster);

			printk("new cluster add\n");

			current_cluster = next_cluster;
			pos = pos + CLUSTER_SIZE;
			break;
		}

		current_cluster = next_cluster;
		pos = pos + CLUSTER_SIZE;
	}

	printf("file.c: write_file in cluster %d\n",(int) current_cluster);
	//printk("write_file : valid len%d\n",valid_len);
	//write operation

	current_write_pos = read_cluster(volume,current_cluster) + cluster_offset;

	//printk("write_file : valid len%d\n", valid_len);
	//printk("write_file1: write pos:%u vlen %d \n", offset, valid_len);

#ifdef __KERNEL__
	seek_volume(volume, current_write_pos);
#else
	seek_volume(volume, current_write_pos, SEEK_SET);
#endif
	write_volume(volume, buf, sizeof(u8_t), valid_len);

	dentry->size = dentry->size + valid_len;

	return valid_len;
}


/*
   함수명  : IsNormalFile
   하는일  : 속성이 보통 파일 속성인지 검사한다.
   인자    : u8 nAttribute : 1바이트의 속성 값
   리턴 값 : BOOL
 */

BOOL is_normal_file(u8_t attribute)
{
	if((attribute & 0x000000FF) == normal_file)
		return TRUE;
	return FALSE;
}

/*
   함수명  : IsLongFileName
   하는일  : 속성이 긴 파일 이름 속성인지 검사한다.
   인자    : u8 nAttribute : 1바이트의 속성 값
   리턴 값 : BOOL
 */

BOOL is_long_file_name(u8_t attribute)
{
	if((attribute & 0x000000FF) == long_file_name)
		return TRUE;
	return FALSE;
}

/*
   함수명  : IsDeletedFile
   하는일  : 속성이 삭제된 파일 속성인지 검사한다.
   인자    : u8 nAttribute : 1바이트의 속성 값
   리턴 값 : BOOL
 */

BOOL is_deleted_file(u8_t attribute)
{
	if((attribute & 0x000000FF) == deleted_file)
		return TRUE;
	return FALSE;
}
/*
   함수명  : setNormalFileAttribute
   하는일  : 디렉토리 엔트리에 보통 파일 속성을 부여한다.
   인자    : pDirectoryEntry : 디렉토리 엔트리의 포인터
   리턴 값 : void
 */

void set_normal_file_attribute(struct mfs_dirent* dentry)
{
	dentry->attribute &= 0xffffff00;
	dentry->attribute += normal_file;
}
