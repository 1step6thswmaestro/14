#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/slab.h>
#include <linux/version.h>
#include "fat.h"
#include "file.h"
#include "namei.h"
#include "entry.h"

#define MFSFS_MAGIC     0x73616d70 /* "SAMP" */

/**
	@brief	mfs가 언마운트 되었을 경우 호출됨 슈퍼블럭 객체 해지

	언마운시 매개변수로 전달되는 슈퍼블럭 객체에 대한 정보를 해지하기 위함.
	@param
			struct super_block	*sb	해제할 슈퍼블럭의 주소
	@return	none
*/
static void mfs_put_super(struct super_block *sb)
{
	printk("MFS : umount\n");
	return;
}

//슈블럭에 관련된 오퍼레이션들
struct super_operations mfs_super_ops = {
	.statfs         = simple_statfs,//< FS의 정보를 보기 위함 함수. 커널에 기본 구현되있는 함수사용
	.drop_inode     = generic_delete_inode, /* Not needed, is the default */
	.put_super      = mfs_put_super//<슈퍼블럭 해지용
};

/**
	@brief	슈퍼블럭의 생성시 슈퍼블럭의 데이터를 채워주는 함수.

	슈퍼블럭
*/
static int mfs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct inode *inode;
	struct mfs_volume* volume = NULL;

	printk("MFS : fill_super enter\n");

	sb->s_maxbytes = MAX_LFS_FILESIZE; /* NB: may be too large for mem */
	sb->s_blocksize = PAGE_CACHE_SIZE;
	sb->s_blocksize_bits = PAGE_CACHE_SHIFT;
	sb->s_magic = MFSFS_MAGIC;
	sb->s_op = &mfs_super_ops;
	sb->s_time_gran = 1; /* 1 nanosecond time granularity */

	printk("parse option:%s\n", (char*)data);

	if (!data) {
		return -ENOMEM;
	}

	volume = open_volume(data);

	if (volume == NULL) {
		printk("ERROR: volume is not found\n");
		return -ENOMEM;
	}

	sb->s_fs_info = volume;
	inode = mfs_get_inode(sb, S_IFDIR | 0755, 0);

	if (!inode) {
		return -ENOMEM;
	}

	#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
	sb->s_root = d_make_root(inode);
	#else
	sb->s_root = d_alloc_root(inode);
	#endif

	if (!sb->s_root) {
		iput(inode);
		return -ENOMEM;
	}

	printk("MFS : fill_super leave\n");
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)

struct dentry *mfs_get_sb(struct file_system_type *fs_type, int flags, const char *dev_name, void *data)
{
	printk("MFS : mount enter\n");
	return mount_nodev(fs_type, flags, data, mfs_fill_super);
}

#else

/**
	@brief	mfs가 마운트 되었을 경우 호출되는 함수.

	슈퍼블럭을 만들어 준다.
	함수 내에서 호출되는 get_sb_nodev() 함수는 슈퍼블럭 노드를 반환 해 주는데
	이때 get_sb_nodev함수내에서 get_sb_nodev의 4번째 인자(함수포인터)가 호출되면서
	슈퍼블럭 구조체의 내부를 채우게 된다.
	@param
			struct file_system_type *fs_type	마운트 할 파일시스템의 정보 구조체 포인터
			int						flag		마운트 관련 flags
			const char				*dev_name
			void					*data
	@return	superblock의 inode
*/
int mfs_get_sb(struct file_system_type *fs_type, int flags, const char *dev_name,void *data,
		struct vfsmount *mnt)
{
	printk("MFS : get_sb enter\n");
	return get_sb_nodev(fs_type,flags,data,mfs_fill_super,mnt);
}

#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)

static struct file_system_type mfs_type = {///<커널에 등록될 fs의 정보가 담길 구조체
	.owner	= THIS_MODULE,
	.name	= "mfs", //< fs의 이름
	.mount	= mfs_get_sb, //< 마운트시 수행될 함수
	.kill_sb= kill_anon_super //< 언마운트시 수행될 함수, vfs에 구현되어 있음
};

#else

static struct file_system_type mfs_type = {///<커널에 등록될 fs의 정보가 담길 구조체
	.owner	= THIS_MODULE,
	.name	= "mfs", //< fs의 이름
	.get_sb	= mfs_get_sb, //< 마운트시 수행될 함수
	.kill_sb= kill_anon_super //< 언마운트시 수행될 함수, vfs에 구현되어 있음
};

#endif

/**
	@brief	MFS모듈을 커널에 탑재했을 때 수행되는 함수.

	MFS모듈이 커널에 탑재될 경우 모듈로더에 의해 호출되는 함수
	커널에 구현되어있는 register_filesystem함수를 호출하여 커널에 fs를 등록
	@return	ignore
**/
static int __init init_mfs(void)
{
	printk("MFS : module  loading ok\n");
	register_filesystem(&mfs_type);

	printk("MFS : fs loading ok\n");
	return 0;
}

/**
	@brief	MFS모듈을 커널에서 내렸을 때 수행되는 함수.

	MFS모듈이 커널에서 내려갈 경우 호출됨.
	커널에 구현되어있는 unregister_filesystem함수를 호출하여 커널에 등록되있던
	fs관련 정보를 삭제
**/
static void __exit exit_mfs(void)
{
	unregister_filesystem(&mfs_type);
	printk("MFS : module unloading ok\n");
}

module_init(init_mfs);
module_exit(exit_mfs);

MODULE_LICENSE("Dual BSD/GPL");





