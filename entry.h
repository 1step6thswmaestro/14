#include "dir.h"

#define CLUSTER_SIZE (BYTES_PER_SECTOR * SECTORS_PER_CLUSTER)
#define EMPTY_CLUSTER 0x0
#define UNUSED_CLUSTER 0x1

BOOL get_dir_path(ps16_t, ps16_t);

BOOL get_file_name(ps16_t, ps16_t);

BOOL get_dentry(struct mfs_volume*, u128,
						ps16_t, struct mfs_dirent*);

u32_t get_cluster_number(struct mfs_volume*, ps16_t);

BOOL alloc_new_dirent(struct mfs_volume*, u128, struct mfs_dirent*, ps16_t);

struct mfs_dirent* get_first_entry(pu8_t, u32_t*, BOOL);

struct mfs_dirent* get_next_entry(pu8_t, u32_t*, BOOL*);

BOOL alloc_new_entry(struct mfs_volume*, u128,
								ps16_t, struct mfs_dirent*);

u128 read_cluster(struct mfs_volume*, u128);

