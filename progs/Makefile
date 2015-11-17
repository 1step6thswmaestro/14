CC = gcc

SRCS_MKFS = ../util.c ../volume.c mkfs.c
SRCS_FUSE = ../util.c ../volume.c ../fat.c ../entry.c ../file.c ../dir.c fuse.c
SRCS_MDBFS = ../util.c ../volume.c ../fat.c ../entry.c ../file.c ../dir.c ../sqlite3.c mdbfs.c shell.c

CFLAGS_MKFS = -g -O1 -Wall -fno-strict-aliasing -std=gnu99 
CFLAGS_FUSE = -Wall `pkg-config fuse --cflags --libs` -lpthread
CFLAGS_MDBFS = -g -O1 -w -fno-strict-aliasing -std=gnu99
CFLAGS_SQLITE = -lpthread -ldl

TARGET_MKFS = mkfs.mfs
TARGET_FUSE = mfs-fuse
TARGET_MDBFS = mdbfs

all: mkfs.mfs mfs-fuse mdbfs

clean:
	rm -rf $(TARGET_MKFS) $(TARGET_FUSE) $(TARGET_MDBFS)

mkfs.mfs:
	$(CC) $(CFLAGS_MKFS) $(SRCS_MKFS) -o $(TARGET_MKFS)
	
mfs-fuse:
	$(CC) $(SRCS_FUSE) $(CFLAGS_FUSE) -o $(TARGET_FUSE)

mdbfs:
	$(CC) $(CFLAGS_MDBFS) $(SRCS_MDBFS) -o $(TARGET_MDBFS) $(CFLAGS_SQLITE)
