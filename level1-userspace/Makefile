# 26F 시스템프로그래밍특론 / Week 3 실습 · Level 1 (user space)
#
#   make            모든 예제 빌드
#   make 01_fork_basic
#   make clean

CC      := gcc
CFLAGS  := -Wall -Wextra -g -std=gnu11
TARGETS := 01_fork_basic 02_fork_exec_wait 03_zombie_orphan 04_proc_reader

.PHONY: all clean

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
