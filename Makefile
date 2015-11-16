obj-m += mfs_mod.o 


mfs_mod-objs := util.o volume.o fat.o entry.o file.o dir.o namei.o super.o sqlite3.o
#EXTRA_CFLAGS=-std=c99

ccflags-y := -mhard-float -msse -mpreferred-stack-boundary=4 -w -I/usr/include -I/usr/include/x86_64-linux-gnu/
#CFLAGS_sqlite3.o = -lpthread -ldl
#LDFLAGS_sqlite3.o = -lpthread -ldl
ldflags-y += -L/usr/lib/x86_64-linux-gnu/ -lpthread -ldl
#LDFLAGS_sqlite3.o = -lpthread -ldl


KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules 

clean:
	rm -rf *.ko *.mod *.cmd *.o *.mod.c *.order *.symvers  .tmp_versions
