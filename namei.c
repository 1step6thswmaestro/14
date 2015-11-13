#include <linux/fs.h>
#include <linux/version.h>
#include "fat.h"
#include "entry.h"
#include "file.h"
#include "namei.h"

extern struct inode_operations mfs_file_inode_ops;
extern struct inode_operations mfs_dir_inode_ops;

/**
	@brief	inode를 구조체 할당 함수

	inode를 얻고 적당한 파라메터에 따라 값으로 초기화 해 주는 함수이다.
	@param
			struct super_block	*sb		MFS파일시스템의 superblock
			int					mode	파일 혹은 디렉토리 여부 등의 속성정보
			dev_t				dev		현재 미사용
	@return	할당에 성공한 inode구조체 포인터
*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0)
struct inode *mfs_get_inode(struct super_block *sb, umode_t mode, dev_t dev)
#else
struct inode *mfs_get_inode(struct super_block *sb, int mode, dev_t dev)
#endif
{
	struct inode * inode = new_inode(sb);

	if (inode) {
		inode->i_mode = mode|=0777;
		inode->i_size = 0;

		#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
		inode->i_uid.val = 0;//file의 소유uid는 root(0)
		inode->i_gid.val = 0;//File의 소유gid는 root(0)
		#else
		inode->i_uid = 0;
		inode->i_gid = 0;
		#endif
		inode->i_blocks = 0;//새로 만드는 inode이므로 사용 블럭은 0
		inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;//현재시간 기준으로

		switch (mode & S_IFMT) {
			default:
				init_special_inode(inode, mode, dev);
				break;
			case S_IFREG:
				inode->i_op = &mfs_file_inode_ops;
				inode->i_fop =  &mfs_file_operations;
				break;
			case S_IFDIR:
				inode->i_op = &mfs_dir_inode_ops;
				inode->i_fop = &mfs_dir_operations;
				inc_nlink(inode);
				break;
			case S_IFLNK:
				printk("namei.c: no support symbolic link");
				break;
		}
	}

	return inode;

}

/**
	@brief	디렉토리에 dentry에 해당하는 파일이 존재하는지 여부를 확인

	@param
			struct	inode		*dir	존재여부를 확인할 디렉토리의 inode
			struct	dentry		*dentry	디렉토리에서 존재여부를 확인할 파일의 dentry
			struct	nameidata	*nd		미사용
	@return	NULL
*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0)
static struct dentry *mfs_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags)
#else
static struct dentry *mfs_lookup(struct inode *dir, struct dentry *dentry, struct nameidata *nd)
#endif
{
	printk("\t\t\t\t\t\t\t\t\t\tMFS LOOKUP\n");

	struct inode* inode = NULL;
	char path[512] = {0};
	int res = 0;
	void* volume = dir->i_sb->s_fs_info;

	printk("namei.c: mfs_lookup %s\n", dentry->d_name.name);
	get_dir_path_from_dentry(dentry, path, 512);

	if ((res = __mfs_lookup(volume, path, (ps16_t) dentry->d_name.name))) {

		if(res == FILE_DENTRY){
			printk("namei.c: file %s\n", res, dentry->d_name.name);
			inode = mfs_get_inode(dir->i_sb, S_IFREG, 0);
		}
		else if(res == DIR_DENTRY){
			printk("namei.c: dir %s\n", res, dentry->d_name.name);
			inode = mfs_get_inode(dir->i_sb, S_IFDIR, 0);
		}

		inode->i_ino = 2;
	} else {
		printk("namei.c: not exist %s\n",dentry->d_name.name);
	}

	d_add(dentry, inode);
	return NULL;
}

/**
	@brief	새로운 노드 생성

	미사용
	@param
			struct inode	*dir
			struct dentry	*dentry
			int				mode
			dev_t			dev
	@return
*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0)
static int mfs_mknod(struct inode *dir, struct dentry *dentry, umode_t mode, dev_t dev)
#else
static int mfs_mknod(struct inode *dir, struct dentry *dentry, int mode, dev_t dev)
#endif
{
	printk("\t\t\t\t\t\t\t\t\t\tMFS MKNOD\n");

	struct inode * inode = mfs_get_inode(dir->i_sb, mode, dev);
	int error = -ENOSPC;

	if (inode) {
		if (dir->i_mode & S_ISGID) {
			inode->i_gid = dir->i_gid;
			if (S_ISDIR(mode))
				inode->i_mode |= S_ISGID;
		}

		d_instantiate(dentry, inode);
		dget(dentry);   /* Extra count - pin the dentry in core */
		error = 0;
		dir->i_mtime = dir->i_ctime = CURRENT_TIME;

		/* real filesystems would normally use i_size_write function */
		dir->i_size += 0x20;  /* bogus small size for each dir entry */
	}
	return error;
}

/**
	@brief	새로운 디렉토리 생성

	inode로 들어오는 dir에 dentry->d_name.dname에 해당하는 디렉토리를 생성해준다.
	@param
			struct inode	*dir	디렉토리를 생성할 디렉토리의 inode
			struct dentry	*dentry	생성할 디렉토리의 이름정보등이 담겨있는 dentry
			int				mode	현재 미사용
	@return	0
*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0)
static int mfs_mkdir(struct inode * dir, struct dentry * dentry, umode_t mode)
#else
static int mfs_mkdir(struct inode * dir, struct dentry * dentry, int mode)
#endif
{
	printk("\t\t\t\t\t\t\t\t\t\tMFS MKDIR\n");
	char buf[512];
	printk("namei.c: mfs_mkdir %s\n", dentry->d_name.name);

	get_dir_path_from_dentry(dentry, buf, 512);
	__mfs_mkdir(dentry->d_sb->s_fs_info, buf, (ps16_t) dentry->d_name.name);

	return 0;
}


void mfs_print_inode(struct inode *inode) {
  if (inode) {
    printk(" = mode(%d) size(%d)", inode->i_mode, i_size_read(inode));

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
    printk("uid(%d) gid(%d)\n", inode->i_uid.val, inode->i_gid.val);
#else
    printk("uid(%d), gid(%d)\n", inode->i_uid, inode->i_gid);
#endif
  }
}


/**
	@brief	새로운 파일 생성

	inode로 들어오는 dir에 dentry->d_name.dname에 해당하는 파일을 생성해준다.
	@param
			struct inode		*dir	디렉토리를 생성할 디렉토리의 inode
			struct dentry		*dentry	생성할 파일의 이름정보등이 담겨있는 dentry
			int					mode	현재 미사용
			struct nameidata	*nd		현재 미사용
	@return 0
*/

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0)
static int mfs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0)
static int mfs_create(struct inode *dir, struct dentry *dentry, umode_t mode, struct nameidata *nd)
#else
static int mfs_create(struct inode *dir, struct dentry *dentry, int mode, struct nameidata *nd)
#endif
{
  printk("\t\t\t\t\t\t\t\t\t\tMFS CREATE\n");

//	return mfs_mknod(dir, dentry, mode | S_IFREG, 0);

  char path[512];
  struct inode* inode = NULL;
  void* volume = dir->i_sb->s_fs_info;

  get_dir_path_from_dentry(dentry, path, 512);

  if (__mfs_create(volume, path, (ps16_t) dentry->d_name.name) == FALSE) {
    printk("namei.c: creation fail\n");
    return 1;
  }

  struct mfs_dirent dentry_mfs;
  u128 cluster_number = get_cluster_number(volume, path);
  //printk("cluster number : %d\n", cluster_number);
  get_dentry(volume, cluster_number, (ps16_t) dentry->d_name.name, &dentry_mfs);

  inode = mfs_get_inode(dir->i_sb, S_IFREG, 0);

  if (inode) {
    inode->i_ino = 2;
    i_size_write(inode, dentry_mfs.size);
    dentry->d_inode = inode;

    //#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0)
    d_instantiate(dentry, inode);
    //dget(dentry);
    //#endif
  }

  printk("%s", (ps16_t) dentry->d_name.name);
  mfs_print_inode(dentry->d_inode);
  return 0;
}


/**
	@brief 파일의 속성 조회

	파일의 크기나 퍼미션등의 속성을 조회한다.
	vfs에 기 구현되어있는 simple_getattr에 파일의 크기정보와 퍼미션을 설정
	@param
			struct vfsmount *mnt	마운트정보
			struct dentry	*dentry	속성을 조회할 파일의 이름정보등이 담겨있는 dentry
			struct kstat	*kstat	속성정보를 기록할 구조체
	@return	simple_getattr의 리턴값
*/
int mfs_getattr(struct vfsmount *mnt, struct dentry *dentry, struct kstat *kstat) {

	printk("\t\t\t\t\t\t\t\t\t\tMFS GETATTR\n");
	int ret=0;
	s16_t full_path[512] = {0,};
	s16_t route[128] = {0, };
	s16_t file_name[64] = {0, };
	int cluster_number;
	void* volume = dentry->d_sb->s_fs_info;
	struct mfs_dirent dentry_mfs;

	get_file_path_from_dentry(dentry, full_path, 512);

	printk("full_path: %s\n", full_path);

	get_file_name(full_path, file_name);
	get_dir_path(full_path, route);

	cluster_number = get_cluster_number(volume, route);
	get_dentry(volume, cluster_number, file_name, &dentry_mfs);

	printk("mfs_getattr %s", dentry->d_name.name);
	mfs_print_inode(dentry->d_inode);
	ret = simple_getattr(mnt, dentry, kstat);

	if (kstat->size==0) {
	  printk("kstat size is 0\n");
	  kstat->size = dentry_mfs.size;
	}
	//printk("mode - %d\n", kstat->mode);
	//kstat->mode|=0777;
	printk("namei.c: mfs_getattr cluster (%d) = %d\n", dentry_mfs.head_cluster_number, ret);
	return ret;
}

static int mfs_setattr(struct dentry *dentry, struct iattr *iattr) {
  printk("\t\t\t\t\t\t\t\t\t\tMFS SETATTR\n");
  int ret = 0;
  ret = simple_setattr(dentry, iattr);
  return ret;
}

static int mfs_unlink(struct inode *inode, struct dentry *dentry) {
  printk("\t\t\t\t\t\t\t\t\t\tMFS UNLINK\n");
  printk("%s\n", dentry->d_name.name);

  int ret = 0;
  void* volume = dentry->d_sb->s_fs_info;
  struct mfs_dirent dentry_mfs;
  s16_t full_path[512] = { 0, };
  s16_t route[128] = {0, };

  get_file_path_from_dentry(dentry, full_path, 512);

  printk("full_path: %s\n", full_path);

  get_file_name(full_path, dentry->d_name.name);
  get_dir_path(full_path, route);

  u128 cluster_number = get_cluster_number(volume, route);
  get_dentry(volume, cluster_number, dentry->d_name.name, &dentry_mfs);

  printk("delete bef %x\n", dentry_mfs.attribute);
  set_deleted_file_attribute(&dentry_mfs);
  printk("deleted %x\n", dentry_mfs.attribute);
  alloc_new_entry(volume, cluster_number, dentry->d_name.name, &dentry_mfs);

  ret = simple_unlink(inode, dentry);
  return ret;
}


struct inode_operations mfs_file_inode_ops = {
    .unlink		= mfs_unlink,
    .setattr		= mfs_setattr,
    .getattr		= mfs_getattr,
};


struct inode_operations mfs_dir_inode_ops = {
	.create         = mfs_create,
	.lookup         = mfs_lookup,
	.link		= simple_link,
	.unlink         = mfs_unlink,
	.mkdir          = mfs_mkdir,
	.rmdir          = simple_rmdir,
	.mknod          = mfs_mknod,
	.rename         = simple_rename,
	.setattr	= mfs_setattr,
	.getattr	= mfs_getattr,
};

