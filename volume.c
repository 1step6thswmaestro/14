#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/file.h>
#include <linux/syscalls.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>
#else
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <stdio.h>
#include <fcntl.h>
#endif
#include "util.h"

#include "volume.h"

#ifdef __KERNEL__
struct mfs_volume* open_volume(char* name) {
	struct mfs_volume* flp = (struct mfs_volume*) vmalloc(sizeof(struct mfs_volume));
	flp->filp = filp_open(name, O_RDWR, S_IRUSR|S_IWUSR);

	if (IS_ERR(flp->filp)) {
		printk("open error\n");
		return NULL;
	}
	else {
		printk("file open success\n");
	}
	return flp;
}
#else
struct mfs_volume* open_volume(const char* name, const char* mode) {
	struct mfs_volume* flp = (struct mfs_volume*) malloc(sizeof(struct mfs_volume));

	FILE* fp = fopen(name, mode);
	if(fp == NULL) return NULL;
	flp->fp = fp;

	return flp;
}
#endif

long write_volume(struct mfs_volume* stream, void* ptr, long size, long items)
{
#ifdef __KERNEL__
	int ret = 0;
	mm_segment_t old_fs = get_fs();
	set_fs(KERNEL_DS);
	ret=vfs_write(stream->filp, ptr, size*items, &(stream->filp->f_pos));
	set_fs(old_fs);
	return ret;
#else

	long result = 0;

#ifdef THREAD_LOCK
	pthread_spin_lock(&spinlock);
#endif
	result = fwrite(ptr, size, items, stream->fp);
#ifdef THREAD_LOCK
	pthread_spin_unlock(&spinlock);
#endif

	return result;
#endif
}

long read_volume(struct mfs_volume* stream, void* ptr, long size, long items)
{
#ifdef __KERNEL__
	int ret = 0;
	mm_segment_t old_fs = get_fs();
	set_fs(KERNEL_DS);
	ret = vfs_read(stream->filp, ptr, size*items, &(stream->filp->f_pos));
	set_fs(old_fs);
	return ret;
#else
	long result = 0;
	result = fread(ptr, size, items, stream->fp);
	return result;
#endif
}

#ifdef __KERNEL__
void seek_volume(struct mfs_volume* stream, unsigned long pos)
{
	mm_segment_t old_fs = get_fs();
	set_fs(KERNEL_DS);
	stream->filp->f_pos = pos;
	set_fs(old_fs);
}
#else
long seek_volume(struct mfs_volume* stream, unsigned long pos, int whence)
{

	long result = 0;
	result = fseek(stream->fp, pos, whence);
	return result;
}
#endif

#ifndef __KERNEL__
int close_volume(struct mfs_volume* stream)
{
	return fclose(stream->fp);
}

void free_volume(struct mfs_volume* stream)
{
	if (stream->fp != NULL) {
		close_volume(stream);
	}
	free(stream);
}
#endif

void read_sb(struct mfs_volume* volume, struct mfs_sb_info* sb)
{
	#ifdef __KERNEL__
	seek_volume(volume, 0);
	#else
	seek_volume(volume, 0, SEEK_SET);
	#endif
	read_volume(volume, sb, sizeof(u8_t), sizeof(struct mfs_sb_info));
}

u128 get_end_cluster(struct mfs_volume* volume)
{

	struct mfs_sb_info sb;
	u16_t fat_index_size = 0;

	read_sb(volume, &sb);
	fat_index_size = sb.fat_index_size;

	switch(fat_index_size)
	{
		case 1:
			return 0xFF;
		case 2:
			return 0xFFFF;
		case 4:
			return 0xFFFFFFFF;
		case 8:
			return 0xFFFFFFFFFFFFFFFF;
		default:
#ifdef __KERNEL__
			printk("ERROR: Can't get end cluster. The fat index size is incorrect.");
			BUG_ON(1);
#else
			printf("ERROR: Can't get end cluster. The fat index size is incorrect.");
			exit(1);
#endif
			break;
	}
}

u128 get_bad_cluster(struct mfs_volume* volume)
{

	struct mfs_sb_info sb;
	u16_t fat_index_size = 0;

	read_sb(volume, &sb);
	fat_index_size = sb.fat_index_size;

	switch(fat_index_size)
	{
		case 1:
			return 0xFE;
		case 2:
			return 0xFFFE;
		case 4:
			return 0xFFFFFFFE;
		case 8:
			return 0xFFFFFFFFFFFFFFFE;
		default:
#ifdef __KERNEL__
			printk("ERROR: Can't get bad cluster. The fat index size is incorrect.");
			BUG_ON(1);
#else
			printf("ERROR: Can't get bad cluster. The fat index size is incorrect.");
			exit(1);
#endif
			break;
	}
}

#ifndef __KERNEL__
void print_volume_status(struct mfs_volume* volume)
{
	struct mfs_sb_info sb;
	memset(&sb, 0, sizeof(struct mfs_sb_info));

	read_sb(volume, &sb);

	printf("Volume label : %s\n", sb.volume_label);
	printf("Serial number : %s\n", sb.volume_serial_number);
	printf("Bytes per sector : %d\n", sb.bytes_per_sector);
	printf("Sectors per cluster : %d\n", sb.sectors_per_cluster);
	printf("Copies of FAT : %d\n", sb.copies_of_fat);
	printf("Sectors of FAT : %d\n", sb.sectors_of_fat);
	printf("Reserved : %d\n", sb.reserved_bytes);
	printf("MFS version : %d.%d.%d\n", sb.version_major, sb.version_minor, sb.version_build);
	printf("FAT sector position : %d\n", sb.fat_sector);
	printf("Data cluster position : %d\n", sb.data_cluster_sector);
	printf("FAT index size : %d\n", sb.fat_index_size);
	printf("Total sectors HIGH : %llu\n", sb.total_sectors_high);
	printf("Total sectors LOW : %llu\n", sb.total_sectors_low);
	printf("Is used total sectors HIGH : %d\n", sb.flag_total_sectors_high);
	printf("Total data cluster sectors HIGH : %llu\n", sb.total_data_cluster_sectors_high);
	printf("Total data cluster sectors LOW : %llu\n", sb.total_data_cluster_sectors_low);
	printf("Is used total data cluster sectors HIGH : %d\n", sb.flag_total_data_cluster_sectors_high);
	printf("Reserved : %s\n", sb.reserved_bytes2);
	printf("The end : %d\n", sb.end_of_sb);
}
#endif
