# 26F 시스템프로그래밍특론 / Week 3 실습 · Level 2 (kernel module)
#
#   make                 세 모듈을 모두 빌드
#   sudo insmod procinfo_current.ko
#   sudo dmesg | tail -n 20
#   sudo rmmod procinfo_current
#   make clean
#
# 준비물:
#   sudo apt install build-essential linux-headers-$(uname -r)

obj-m += procinfo_current.o
obj-m += procinfo_list.o
obj-m += procinfo_tree.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

.PHONY: all clean load unload log

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

# 편의 타겟: make load MOD=procinfo_list
load:
	sudo insmod $(MOD).ko

unload:
	sudo rmmod $(MOD)

log:
	sudo dmesg | tail -n 40
