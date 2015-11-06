#include "util.h"

struct mfs_volume* open_sqlite_volume(char*);
void write_sqlite_file(struct mfs_volume*, char* route, char*, char*, u32_t, u32_t);
void read_sqlite_file(struct mfs_volume*, char* route, char*, char*);