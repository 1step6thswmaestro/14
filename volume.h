#ifndef __KERNEL__
#include <stdio.h>
#else
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/file.h>
#include <linux/syscalls.h>
#include <linux/vmalloc.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>
#endif
#include "super.h"

#define CLUSTER_SIZE 		 (BYTES_PER_SECTOR * SECTORS_PER_CLUSTER)
#define BYTES_PER_SECTOR     512
#define SECTORS_PER_CLUSTER  1

struct mfs_volume {
	int fd;
	int offset;
#ifdef __KERNEL__
	struct file* filp;
#endif
	int perm;
	char buf[1024];
#ifndef __KERNEL__
	FILE* fp;
#endif
};

void read_sb(struct mfs_volume*, struct mfs_sb_info*);

u128 get_end_cluster(struct mfs_volume*);

u128 get_bad_cluster(struct mfs_volume*);

void print_volume_status(struct mfs_volume*);

#ifdef __KERNEL__
struct mfs_volume* open_volume(char*);
#else
struct mfs_volume* open_volume(const char*, const char*);
#endif

long write_volume(struct mfs_volume*, void*,long, long);

long read_volume(struct mfs_volume*, void*,long, long);

#ifdef __KERNEL__
void seek_volume(struct mfs_volume*, unsigned long);
#else
long seek_volume(struct mfs_volume*, unsigned long, int);
#endif

#ifndef __KERNEL__
int close_volume(struct mfs_volume*);

void free_volume(struct mfs_volume*);
#endif


