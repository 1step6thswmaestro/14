#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "volume.h"
#include "dir.h"
#include "namei.h"
//#include "volume.h"
//#include "entry.h"
#include "sqliteio.h"

int mdbfsLookup(struct mfs_volume* volume, char *filename) {
  return __mfs_lookup(volume, "/", filename);
}

struct mfs_volume* mdbfsVolume(char* name) {
  struct mfs_volume* new = NULL;
  new = open_volume(name, "r+b");
  if (new == NULL) {
    printf("open_mdbfs ERROR: %s doesn't exist\n", name);
    exit(1);
  }
  printf("open_mdbfs SUC: %s is opened\n", name);
  return new;
}

int mdbfsOpen(struct mfs_volume* volume, char *filename) {
  int cn = -1;

  switch (mdbfsLookup(volume, filename)) {
    case FILE_DENTRY:
      printf("found file: %s\n", filename);
      int n_read = 0;

      u128 cluster_number = get_cluster_number(volume, "/");
      struct mfs_dirent dentry;
      get_dentry(volume, cluster_number, filename, &dentry);
      cn = (int) cluster_number;
//      char path[500] = "/";
//      strcat(path, filename);
//      cn = (int) get_cluster_number(volume, path);
//      struct mfs_dirent dentry;
//      get_dentry(volume, cn, filename, &dentry);
      print_dentry(&dentry);
      cn = (int) dentry.head_cluster_number;
      break;
    case DIR_DENTRY:
      printf("found dir: %s\n", filename);
      break;
    default:
      printf("not found >> create file: %s\n", filename);
      cn = find_empty_fat_index(volume);
      __mfs_create(volume, "/", filename);
      break;
  }

  printf("cls_number for %s : %d\n", filename, cn);
  return cn;
}

int mdbfsRead(u128 cluster, char* filename, char* buff, int len, u64_t offset) {
//  got = read_sqlite_file(volume, "/", file_name, pBuf, cnt, offset);

//  return read_sqlite_file(volume, "/", name);
  return 0;
}

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

void open_sqlite_file(struct mfs_volume* volume, char* route, char* file_name)
{
	printf("mfs sqlite: open\n");

  int dentry_type = lookup_sqlite_file(volume, route, file_name);

  switch (dentry_type) {
    case FILE_DENTRY:
      printf("found file: %s\n", file_name);
      break;
    case DIR_DENTRY:
      printf("found dir: %s\n", file_name);
      break;
    default:
      printf("not found: %s\n", file_name);
      create_sqlite_file(volume, route, file_name);
      break;
  }
}

int lookup_sqlite_file(struct mfs_volume* volume, char* route, char* file_name)
{
	printf("mfs sqlite: lookup\n");
	return __mfs_lookup(volume, route, file_name);
}

void create_sqlite_file(struct mfs_volume* volume, char* route, char* file_name)
{
	printf("mfs sqlite: create\n");
	__mfs_create(volume, route, file_name);
}

int write_sqlite_file(struct mfs_volume* volume, char* route, char* file_name, char* buff, int len, u64_t offset)
{
	if(buff == NULL)
	{
		return -1;
	}
	printf("mfs sqlite: write %dB in %s%s:%d\n", len, route, file_name, offset);

	int n_write = 0;

	u128 cluster_number = get_cluster_number(volume, route);
	struct mfs_dirent dentry;

	get_dentry(volume, cluster_number, file_name, &dentry);

	n_write = write_file(volume, &dentry, buff, len, offset);

	alloc_new_entry(volume, cluster_number, file_name, &dentry);

	return n_write;
}

int read_sqlite_file(struct mfs_volume* volume, char* route, char* file_name, char* buff, int len, u64_t offset)
{
	if(buff == NULL)
	{
		return -1;
	}
	printf("mfs sqlite: read %dB in %s%s:%d\n", len, route, file_name, offset);

	int n_read = 0;

	u128 cluster_number = get_cluster_number(volume, route);
	struct mfs_dirent dentry;

	get_dentry(volume, cluster_number, file_name, &dentry);

	n_read = read_file(volume, &dentry, buff, len, offset);

	return n_read;
}
