#!/bin/bash

DIR_SEPERATOR="/"
PARENT_PATH="../"
MFS_MOD_KO="mfs_mod.ko"

KDIR=$1
ROOT_FS=$2
ROOT_FS_MOUNT_POINT=$3
MFS_MOD_KO_PATH=$4
CURRENT_PATH=`pwd -P`

if [ ! -d "$KDIR" ]
then
	echo "Can't found kernel directory $KDIR"
	exit
fi

if [ ! -f "$ROOT_FS" ]
then
	echo "Can't found root filesystem image $ROOT_FS"
	exit
fi

if [ ! -d "$ROOT_FS_MOUNT_POINT" ]
then
	echo "Can't found mount point $ROOT_FS_MOUNT_POINT"
	exit
fi

if [ -f "..$DIR_SEPERATOR$MFS_MOD_KO" ]
then
	rm "..$DIR_SEPERATOR$MFS_MOD_KO"
fi

echo "Build $MFS_MOD_KO for $KDIR"

KDIR=$CURRENT_PATH$DIR_SEPERATOR$KDIR
cd .. && make KDIR=$KDIR

if [ ! -f ".$DIR_SEPERATOR$MFS_MOD_KO" ]
then
	echo "Build $MFS_MOD_KO is Fail!"
	exit
fi

cd $CURRENT_PATH

echo "VM root filesystem image is : $ROOT_FS"

sudo mount -o loop $ROOT_FS $ROOT_FS_MOUNT_POINT

if [ -f "$ROOT_FS_MOUNT_POINT$DIR_SEPERATOR$MFS_MOD_KO_PATH$DIR_SEPERATOR$MFS_MOD_KO" ]
then
	echo "Found $MFS_MOD_KO in $ROOT_FS_MOUNT_POINT$DIR_SEPERATOR$MFS_MOD_KO_PATH"
	echo "Delete $ROOT_FS_MOUNT_POINT$DIR_SEPERATOR$MFS_MOD_KO_PATH$DIR_SEPERATOR$MFS_MOD_KO"
	sudo rm $ROOT_FS_MOUNT_POINT$DIR_SEPERATOR$MFS_MOD_KO_PATH$DIR_SEPERATOR$MFS_MOD_KO
fi

echo "Copy the new $MFS_MOD_KO to $ROOT_FS_MOUNT_POINT$DIR_SEPERATOR$MFS_MOD_KO_PATH"
sudo cp ..$DIR_SEPERATOR$MFS_MOD_KO $ROOT_FS_MOUNT_POINT$DIR_SEPERATOR$MFS_MOD_KO_PATH$DIR_SEPERATOR

sudo umount $ROOT_FS_MOUNT_POINT

echo "Success! Now you can use root filesystem $ROOT_FS"