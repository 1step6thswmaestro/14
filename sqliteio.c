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
    printf("mdbfsVolume ERROR: %s doesn't exist\n", name);
    exit(1);
  }
  printf("mdbfsVolume SUC: %s is opened\n", name);
  return new;
}

char *mdbfsPath (char *filename) {
  int len = strlen(filename);
  char fullpath[] = "/";
  strcat(fullpath, filename);
  printf("fullpath : %s\n", fullpath);
  return fullpath;
}

int mdbfsOpen(struct mfs_volume* volume, char *filename) {
  int cn = -1;
  printf("[mdbfsOpen]\n");

  switch (mdbfsLookup(volume, filename)) {
    case FILE_DENTRY:
      printf("found file: %s\n", filename);
      int n_read = 0;
      cn = (int) get_cluster_number(volume, mdbfsPath(filename));
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

int mdbfsRead(struct mfs_volume* volume, int cn, char* filename, char* buff, int len, u64_t offset) {
  printf("[mdbfsRead]: read %dB in %d\n", len, cn);
  struct mfs_dirent dentry;
  get_dentry(volume, cn, filename, &dentry);
  return read_file(volume, &dentry, buff, len, offset);
}

int mdbfsWrite(struct mfs_volume* volume, char* filename, int cn, char* buff, int len, u64_t offset) {
  if (buff == NULL) {
    return -1;
  }
  printf("mfs sqlite: write %dB in %d\n", len, cn);

  int n_write = 0;
  u128 cluster_number = (u128) cn;
  struct mfs_dirent dentry;

  get_dentry(volume, cluster_number, filename, &dentry);
  n_write = write_file(volume, &dentry, buff, len, offset);
  alloc_new_entry(volume, cluster_number, filename, &dentry);
  return n_write;
}

int mdbfsStat(struct mfs_volume* volume, char *filename, struct stat *stat) {
  struct mfs_dirent dentry;
  u128 cluster_number = get_cluster_number(volume, "/");
  get_dentry(volume, cluster_number, filename, &dentry);

//  struct inode *inode = dentry->d_inode;
//  stat->st_mode =

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
