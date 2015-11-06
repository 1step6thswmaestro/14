#ifdef __KERNEL__
#include <linux/fs.h>
#include <linux/version.h>
#endif

#define FILE_DENTRY		1
#define DIR_DENTRY		2

#ifdef __KERNEL__
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0)
struct inode *mfs_get_inode(struct super_block *, umode_t, dev_t);
#else
struct inode *mfs_get_inode(struct super_block *, int, dev_t);
#endif
#endif
