
# make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-

mkfilepath := $(abspath $(lastword $(MAKEFILE_LIST)))
# $(info mkfilepath=$(mkfilepath))
mkfile_rdir := $(dir $(mkfilepath))
$(info mkfile_rdir=$(mkfile_rdir))
# to /home/albter/codespace
$(eval rdir := $(abspath $(mkfile_rdir)/../..))
#$(error "根目录2 "$(rdir))

ifneq  ($(KERNELRELEASE),)

obj-m:=msdrv1.o msdrv2.o
$(info "编译2nd")

else

#KDIR := /lib/modules/$(shell uname -r)/build
KDIR :=$(rdir)/linux-4.11
DST :=$(rdir)/busybox-1.33.1/_install/app/
PWD:=$(shell pwd)

all:
	$(shell if [ ! -e $(DST) ];then mkdir -p $(DST); fi)
	$(info "编译1st")
	make -C $(KDIR) M=$(PWD) modules
	arm-linux-gnueabihf-gcc -static -o test test.c -lpthread
	cp *.ko test $(DST)
clean:
	rm -rf *.ko *.o *.symvers *.mod.c *.mod.o *.mod *.order .[!.]* test $(DST)/*
endif
