#--------- Makefile ---------#

obj-m	:= spinlock_module.o

spinlock_module-y	:= spinlock_module-base.o linked_list_impl.o calclock.o

KERNEL_DIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

default:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
