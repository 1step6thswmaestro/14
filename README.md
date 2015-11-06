#Maestro File System for Linux
MFS(Maestro File System)는 SW Maestro 과정의 파일시스템 개발 프로젝트입니다. 본 레포지터리는 MFS를 리눅스에서 사용할 수 있도록 한 구현체입니다. 리눅스 커널모듈(mfs_mod.ko)과 mkfs.mfs, 그리고 mfs-fuse로 구성되어 있습니다.

##Download

$ git clone http://bitbucket.org/maestrofs/mfs-linux.git

##Build

###mfs_mod.ko
$ cd mfs-linux  
$ make

###mkfs.mfs
$ cd mfs-linux/progs  
$ make mkfs.mfs

###mfs-fuse
$ cd mfs-linux/progs  
$ make mfs-fuse

##Test

###Build kernel module, move into rootfs for virtual machine
$ cd mfs-linux/scripts  
$ ./make.sh ../../linux ../../rootfs.img /mnt/mfs tmp/mfs/

###Automatic insmod, and mount
$ cd mfs-linux/scripts  
$ ./mount.sh ../../mfs\_hdd /mnt/mfs\_hdd 

###Automatic rmmod, and umount
$ cd mfs-linux/scripts  
$ ./umount.sh /mnt/mfs\_hdd

##Usage

###mfs_mod.ko
$ cd mfs-linux  
$ make  
\# insmod mfs_mod.ko

###mkfs.mfs
$ dd if=/dev/zero of=./new\_mfs\_hdd bs=1M count=4  
$ cd mfs-linux/progs  
$ make mkfs.mfs  
$ ./mkfs.mfs ../../new\_mfs\_hdd

###mfs-fuse
$ cd mfs-linux/progs  
$ make mfs-fuse  
$ ./mfs-fuse ../../mfs\_hdd /mnt/mfs\_hdd


