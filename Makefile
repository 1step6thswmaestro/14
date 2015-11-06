obj-m += mfs_mod.o 

mfs_mod-objs := util.o volume.o fat.o entry.o file.o dir.o namei.o super.o
#EXTRA_CFLAGS=-std=c99

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules 

clean:
	rm -rf *.ko *.mod *.cmd *.o *.mod.c *.order *.symvers  .tmp_versions




