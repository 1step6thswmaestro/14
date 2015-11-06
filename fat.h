#include "volume.h"
#include "super.h"

BOOL write_in_fat_index(struct mfs_volume*, u32_t, u64_t);

u32_t read_fat_index(struct mfs_volume*, u128);

u32_t find_empty_fat_index(struct mfs_volume*);

