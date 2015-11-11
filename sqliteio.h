#include "util.h"

struct mfs_volume* mdbfsVolume(char* name);
int mdbfsOpen(struct mfs_volume* volume, char *name);
int mdbfsRead(struct mfs_volume* volume, int cn, char* filename, char* buff, int len, u64_t offset);
int mdbfsWrite(struct mfs_volume* volume, char* filename, int cn, char* buff, int len, u64_t offset);

struct mfs_volume* open_sqlite_volume(char*);

void open_sqlite_file  (struct mfs_volume*, char*, char*);
int  lookup_sqlite_file(struct mfs_volume*, char*, char*);
void create_sqlite_file(struct mfs_volume*, char*, char*);

int  write_sqlite_file (struct mfs_volume*, char*, char*, char*, int, u64_t);
int  read_sqlite_file  (struct mfs_volume*, char*, char*, char*, int, u64_t);
