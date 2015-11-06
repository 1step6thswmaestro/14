#include "util.h"

struct mfs_volume* open_sqlite_volume(char*);
void write_sqlite_file(struct mfs_volume*, char*, char*, char*, u32_t, u32_t);
int read_sqlite_file(struct mfs_volume*, char*, char*, char*, int, u64_t);