#ifndef __SUPER_H__
#define __SUPER_H__

#include "util.h"

#ifdef __KERNEL__

#include <linux/fs.h>
#include <linux/version.h>

#endif

//struct mfs_sb_info_meta {
//	s8_t volume_label[11];
//	s8_t volume_serial_number[8];
//	u16_t bytes_per_sector;
//	u8_t sectors_per_cluster;
//	u8_t copies_of_fat;
//	u16_t sectors_of_fat;
//	u32_t reserved_bytes;
//	u8_t version_major;
//	u8_t version_minor;
//	u16_t version_build;
//	u32_t fat_sector;
//	u32_t data_cluster_sector;
//	u16_t fat_index_size;
//	u64_t total_sectors_high;
//	u64_t total_sectors_low;
//	u8_t flag_total_sectors_high;
//	u64_t total_data_cluster_sectors_high;
//	u64_t total_data_cluster_sectors_low;
//	u8_t flag_total_data_cluster_sectors_high;
//};
//
//struct mfs_sb_info_eos {
//	u16_t end_of_sb;
//};
//
//#define RESERVED_BYTES2_SIZE 512 - sizeof(struct mfs_sb_info) - sizeof(struct mfs_sb_info_eos)
//
//struct mfs_sb_info_reserved {
//	u8_t reserved_bytes2[RESERVED_BYTES2_SIZE];
//};
//
//struct mfs_sb_info {
//	struct mfs_sb_info_meta meta;
//	struct mfs_sb_info_reserved reserved;
//	struct mfs_sb_info_eos eos;
//};

struct mfs_sb_info {
	s8_t volume_label[11];
	s8_t volume_serial_number[8];
	u16_t bytes_per_sector;
	u8_t sectors_per_cluster;
	u8_t copies_of_fat;
	u16_t sectors_of_fat;
	u32_t reserved_bytes;
	u8_t version_major;
	u8_t version_minor;
	u16_t version_build;
	u32_t fat_sector;
	u32_t data_cluster_sector;
	u16_t fat_index_size;
	u64_t total_sectors_high;
	u64_t total_sectors_low;
	u8_t flag_total_sectors_high;
	u64_t total_data_cluster_sectors_high;
	u64_t total_data_cluster_sectors_low;
	u8_t flag_total_data_cluster_sectors_high;
	u8_t reserved_bytes2[416];
	u16_t end_of_sb;
};

#ifdef __KERNEL__

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)

struct dentry *mfs_get_sb(struct file_system_type *fs_type, int flags, const char *dev_name,void *data);

#else

int mfs_get_sb(struct file_system_type *fs_type, int flags, const char *dev_name,void *data,
		struct vfsmount *mnt);

#endif
#endif
#endif
