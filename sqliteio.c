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

void write_sqlite_file(struct mfs_volume* volume, char* route, char* file_name, char* buff, u32_t offset, u32_t total_size)
{
	struct mfs_dirent dentry;

	__mfs_create(volume, route, file_name);

	u128 root_cluster_number = get_cluster_number(volume, route);
	get_dentry(volume, root_cluster_number, file_name, &dentry);

	while (offset < total_size)
	{
		offset += write_file(volume, &dentry, &buff[offset], total_size, offset);
	}

	alloc_new_entry(volume, root_cluster_number, file_name, &dentry);
}

int read_sqlite_file(struct mfs_volume* volume, char* route, char* file_name, char* buff)
{
	int n_read = 0;
	u128 cluster_number;
	struct mfs_dirent dentry;
	int i;

	int offset = 0;

	cluster_number = get_cluster_number(volume, route);
	get_dentry(volume, cluster_number, file_name, &dentry);

	int len = dentry.size;

	buff = malloc(len);
	if (buff == NULL)
	{
		return -1;
	}

	while (offset < len)
	{
		n_read = read_file(volume, &dentry , &buff[offset], len, offset);
		offset += n_read;
	}

	for (i = 0; i < len; ++i)
	{
		printf("%02X ", buff[i] & 0xff);
		if ((i+1)%32 == 0)
		{
			printf("\n");
		}
	}

	free(buff);

	return len;
}
