# 26F 시스템프로그래밍특론 / Week 3 실습 · Level 3 (kernel thread)
#
#   make
#   sudo insmod kthread_demo.ko interval=2
#   ps -eo pid,ppid,stat,comm | grep knu_worker
#   sudo dmesg -w
#   sudo rmmod kthread_demo
#   make clean

obj-m += kthread_demo.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

.PHONY: all clean

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
