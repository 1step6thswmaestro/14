#ifdef __KERNEL__
#include <linux/version.h>
#endif

//#include "util.h"
#include "compiler.h"

struct mfs_dirent {
	u8_t attribute;
	u32_t extended_attribute;
	s16_t name[24];
	u32_t head_cluster_number;
	u64_t size;
	u8_t reserved[23];
}
PACKED;

enum mfs_dirent_attribute {
	// unused 					= 0x00,
	deleted_file   	= 0x00,
	normal_file    	= 0x01,
	deleted_dir 		= 0x02,
	normal_dir 			= 0x03,
	long_file_name  = 0x04
};

#ifdef __KERNEL__
struct printdir{
	struct file* filp;
	void* dirent;
	filldir_t filldir;
};
#endif

BOOL composite_long_file_name(struct mfs_volume*, u128, u32_t, ps16_t);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 12, 0)
void __mfs_readdir(const ps16_t, struct file *, struct dir_context *);
#else
void __mfs_readdir(struct mfs_volume*, const ps16_t, struct printdir*);
#endif

int __mfs_lookup(struct mfs_volume*,const ps16_t,const ps16_t);

BOOL is_normal_dir(u8_t);

BOOL is_deleted_dir(u8_t);

void set_normal_dir_attribute(struct mfs_dirent*);

BOOL __mfs_mkdir(struct mfs_volume*, ps16_t, ps16_t);

void print_dentry(struct mfs_dirent*);