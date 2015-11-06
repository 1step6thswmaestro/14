#include "util.h"
#include "volume.h"

void write_sqlite_file(struct mfs_volume*, char* route, char*, char*, u32_t, u32_t);
void read_sqlite_file(struct mfs_volume*, char* route, char*);
struct mfs_volume* open_sqlite_volume(char*);