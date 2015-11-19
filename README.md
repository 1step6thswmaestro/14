#커널기반 데이터베이스 파일시스템(MDBFS)
MDBFS(Maestro DataBase File System)는 MFS(Maestro File System)와 SQLite를 기반으로한 데이터베이스를 내장한 파일시스템 개발 프로젝트입니다.
본 레포지터리는 MDBFS를 리눅스에서 사용할 수 있도록 한 구현체입니다.
MDBFS는 리눅스 커널모듈(mfs_mod.ko)과 mdbfs 그리고 mfs-fuse로 구성되어 있습니다.

##Team
###Mentee
- 장재혁(팀장)
- 이영인
- 조태상

###Mentor
- 김태하

##Download
$ git clone http://bitbucket.org/mdbfs/mdbfs.git


##Build
###mfs_mod.ko
$ cd mdbfs<br />
$ make

###mdbfs
$ cd mdbfs/progs<br />
$ make mdbfs

###mfs-fuse
$ cd mdbfs/progs<br />
$ make mfs-fuse

##MFS Test
###Build kernel module, and insert module
$ cd mdbfs;<br />
$ ./insmod_mfs.sh

###Make disk image(disk.img), and mfs format disk.img
$ cd mdbfs/progs;<br />
$ ./if_mdbfs.sh<br />
mdbfs> .format<br />
device_name, volume_name, size, cluster_size : disk.img -c 1024 -n VNAME<br />
...some prints...<br />
mdbfs> .q

###MFS formatted check
$ cd mdbfs/progs<br />
$ sudo apt-get install hexedit<br />
(if didn't install hexedit.)<br />
$ hexedit disk.img

###Mount disk image, and move to mounted folder
$ cd mdbfs/progs;<br />
$ ./mount.sh<br />
$ cd /mnt/mfs;

##SQLite test
###Create and insert
$ cd /mnt/mfs<br />
$ [MDBFS_FOLDER_PATH]/progs/mdbfs<br />
mdbfs> create table tbl1(one varchar(100), two int);<br />
mdbfs> insert into tbl1(one, two) values("aaa", 10);<br />
mdbfs> insert into tbl1(one, two) values("bbb", 1234);

###Show tables
mdbfs> .tables<br />
tbl1

###Show columns
mdbfs> select * from tbl1;<br />
aaa | 10<br />
bbb | 1234

###Save to file from in-memory
mdbfs> .save test1.db

###Quit
mdbfs> .q

###Saved check
$ ls<br />
test1.db
