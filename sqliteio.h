#include "util.h"

struct mfs_volume* open_sqlite_volume(char*);

void open_sqlite_file  (struct mfs_volume*, char*, char*);
int  lookup_sqlite_file(struct mfs_volume*, char*, char*);
void create_sqlite_file(struct mfs_volume*, char*, char*);

int  write_sqlite_file (struct mfs_volume*, char*, char*, char*, int, u64_t);
int  read_sqlite_file  (struct mfs_volume*, char*, char*, char*, int, u64_t);