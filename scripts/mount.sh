#!/bin/bash

DIR_SEPERATOR="/"
PARENT_DIR="../"
MFS_MOD="mfs_mod"
MFS_MOD_KO=$MFS_MOD".ko"

MFS_DISK_IMAGE=$1
MOUNT_POINT=$2

if [ ! -f "$MFS_DISK_IMAGE" ]
then
	echo "Can't found MFS disk image $MFS_DISK_IMAGE"
	exit
fi

if [ ! -d "$MOUNT_POINT" ]
then
	echo "Can't found mount point $MOUNT_POINT"
	exit
fi

is_loaded_module() {
    if lsmod | grep "$1" &> /dev/null ; then
    	return 0
	else
		return 1
	fi
}

if is_loaded_module $MFS_MOD; then
	echo "$MFS_MOD is already loaded."
	sudo rmmod $MFS_MOD
	
	if is_loaded_module $MFS_MOD; then
		echo "Can't rmmod $MFS_MOD"
		exit
	fi
fi

sudo insmod $PARENT_DIR$MFS_MOD_KO

if ! is_loaded_module $MFS_MOD; then
	echo "Can't insmod $MFS_MOD"
	exit
fi

if mount | grep $MOUNT_POINT > /dev/null; then
    echo "$MOUNT_POINT is already mounted"
    exit
fi

sudo mount -t mfs mfs -o $MFS_DISK_IMAGE $MOUNT_POINT

echo "$MFS_DISK_IMAGE is mounted on $MOUNT_POINT!"

