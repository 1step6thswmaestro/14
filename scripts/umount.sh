#!/bin/bash

MFS_MOD="mfs_mod"
MFS_MOD_KO=$MFS_MOD".ko"

MOUNT_POINT=$1

is_mounted() {
	if mount | grep $MOUNT_POINT > /dev/null; then
	    return 0
	else
		return 1
	fi
}

if ! is_mounted ; then
	echo "$MOUNT_POINT is not mounted"
    exit
fi

sudo umount $MOUNT_POINT
sudo rmmod $MFS_MOD
