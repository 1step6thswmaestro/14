#include "../util.h"
#include "../super.h"
#include <math.h>

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_BUILD 16
#define VERSION_PATCH 0

#define COUNT_TOTAL_CLUSTER(total_sector_size) 	total_sector_size / SECTORS_PER_CLUSTER
#define COUNT_TOTAL_SECTOR(volume_size) 		volume_size / BYTES_PER_SECTOR
#define SECTORS_OF_SUPER_BLK 					(sizeof(struct mfs_sb_info) / BYTES_PER_SECTOR)
#define SECTORS_OF_FAT(total_cluster_size)		(total_cluster_size * mfs_fat_index_size(total_cluster_size)) / BYTES_PER_SECTOR

inline u16_t mfs_fat_index_size(u128 total_cluster_size)
{
	if (total_cluster_size < 256)
	{
		return 1;
	}
	else if (total_cluster_size < 256 * 256)
	{
		return 2;
	}
	else if (total_cluster_size < 256 * 256 * 256 * 256)
	{
		return 4;
	}
	else if (total_cluster_size < 256 * 256 * 256 * 256 * 256 * 256 * 256 * 256)
	{
		return 8;
	}
	else if (total_cluster_size < 256 * 256 * 256 * 256 * 256 * 256 * 256 * 256 * 256 * 256 * 256 * 256 * 256 * 256 * 256 * 256)
	{
		return 16;
	}
	else
	{
		printf("total cluster size is too big.\n");
		return 0;
	}
}


