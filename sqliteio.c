#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "dir.h"

struct mfs_volume* open_sqlite_volume(char* device_name)
{
	if (device_name == NULL) 
	{
		printf("ERROR: Set the correct device name\n");
		exit(1);
	}

	struct mfs_volume* volume = NULL;

	volume = open_volume(device_name, "r+b");

	if (volume == NULL) 
	{
		printf("ERROR: %s doesn't exist\n", device_name);
		exit(1);
	}

	return volume;
}

void write_sqlite_file(struct mfs_volume* volume, char* route, char* file_name, char* buff, int len, u64_t offset)
{
	struct mfs_dirent dentry;

	int n_write = 0;
	int n_total = 0;

	__mfs_create(volume, route, file_name);

	u128 root_cluster_number = get_cluster_number(volume, route);
	get_dentry(volume, root_cluster_number, file_name, &dentry);

	while (len > 0)
	{
		n_write = write_file(volume, &dentry, buff, len, offset);

		offset += n_write;
		buff += n_write;
		len -= n_write;

		if(n_write <= 0) break;
	}

	alloc_new_entry(volume, root_cluster_number, file_name, &dentry);
}

int read_sqlite_file(struct mfs_volume* volume, char* route, char* file_name, char* buff, int len, u64_t offset)
{
	if(buff == NULL)
	{
		return -1;
	}

	int n_read = 0;

	u128 cluster_number = get_cluster_number(volume, route);
	struct mfs_dirent dentry;

	get_dentry(volume, cluster_number, file_name, &dentry);

	n_read = read_file(volume, &dentry, buff, len, offset);

	return n_read;
}