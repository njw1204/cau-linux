# SPDX-License-Identifier: GPL-2.0
#
# Makefile for SQRT List Module
# Compatible with Linux Kernel 5.4.214
#

obj-m	+= sqrtlist.o
sqrtlist-y	:= main.o tester.o calclock.o
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

run:
	$(MAKE)
	insmod sqrtlist.ko
	rmmod sqrtlist.ko
	$(MAKE) clean
	dmesg | tail -n 19