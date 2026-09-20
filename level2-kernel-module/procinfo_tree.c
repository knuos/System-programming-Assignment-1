// SPDX-License-Identifier: GPL-2.0
/*
 * procinfo_tree.c — 프로세스 계보(family tree) 탐색하기
 *
 * 26F 시스템프로그래밍특론 / Week 3 실습 · Level 2-C
 *
 * [학습 목표]
 *   - 모든 프로세스가 init(PID 1)의 후손이라는 사실을 코드로 확인한다.
 *   - parent / children / sibling 포인터가 만드는 트리 구조를 이해한다.
 *   - list_for_each_entry로 커널의 연결 리스트를 다루는 법을 익힌다.
 *
 * [빌드]  make
 * [적재]  sudo insmod procinfo_tree.ko target=1
 *         sudo insmod procinfo_tree.ko target=$$      (현재 셸)
 * [확인]  sudo dmesg | tail -n 40
 * [비교]  pstree -p <pid>
 * [제거]  sudo rmmod procinfo_tree
 *
 * [참고할 커널 소스 — Elixir v7.2.2]
 *   task_struct의 real_parent / children / sibling : include/linux/sched.h
 *   pid_task, find_get_pid                          : kernel/pid.c
 */

/* pr_fmt은 반드시 헤더 include 앞에 정의해야 적용된다 (dmesg 접두어) */
#define pr_fmt(fmt) "proctree: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/pid.h>
#include <linux/list.h>

static int target = 1;   /* 기본값: init/systemd */
module_param(target, int, 0444);
MODULE_PARM_DESC(target, "PID whose family tree will be printed");

static int __init proctree_init(void)
{
	struct task_struct *task = NULL;
	struct task_struct *child;
	struct list_head *pos;
	int nr_children = 0;

	/* ------------------------------------------------------------------
	 * TODO 1: target PID에 해당하는 task_struct를 찾으시오.
	 *
	 *   방법 A) pid_task(find_get_pid(target), PIDTYPE_PID)
	 *   방법 B) for_each_process 로 순회하며 task->pid == target 인 것을 찾기
	 *
	 *   찾지 못하면 pr_err()로 알리고 -ESRCH 를 리턴할 것.
	 * ------------------------------------------------------------------ */
	if (!task) {
		pr_err("pid %d not found\n", target);
		return -ESRCH;
	}

	pr_info("=== family tree of pid %d ===\n", target);

	/* ------------------------------------------------------------------
	 * TODO 2: 자기 자신의 정보를 출력하시오.
	 *         pr_info("self  : pid=%d comm=%s\n", task->pid, task->comm);
	 * ------------------------------------------------------------------ */

	/* ------------------------------------------------------------------
	 * TODO 3: 부모를 출력하시오. (task->real_parent)
	 *         real_parent 와 parent 의 차이는 무엇인가? 주석으로 적을 것.
	 * ------------------------------------------------------------------ */

	/* ------------------------------------------------------------------
	 * TODO 4: 자식들을 순회하여 모두 출력하시오.
	 *
	 *   list_for_each(pos, &task->children) {
	 *           child = list_entry(pos, struct task_struct, sibling);
	 *           pr_info("child : pid=%d comm=%s\n", child->pid, child->comm);
	 *           nr_children++;
	 *   }
	 *
	 *   또는 더 간결하게:
	 *   list_for_each_entry(child, &task->children, sibling) { ... }
	 * ------------------------------------------------------------------ */
	(void)child;
	(void)pos;

	pr_info("=== %d children ===\n", nr_children);

	/* ------------------------------------------------------------------
	 * TODO 5 (심화): 현재 프로세스(current)에서 시작해 real_parent를 계속
	 *         따라 올라가며 PID 1에 도달할 때까지의 경로를 출력하시오.
	 *
	 *         예)  insmod(3421) → bash(2870) → sshd(2100) → systemd(1)
	 *
	 *         무한 루프에 빠지지 않도록 pid == 1 에서 반드시 멈출 것.
	 * ------------------------------------------------------------------ */

	return 0;
}

static void __exit proctree_exit(void)
{
	pr_info("module unloaded\n");
}

module_init(proctree_init);
module_exit(proctree_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<학번> <이름>");
MODULE_DESCRIPTION("Week 3 Lab - traverse the process family tree");
MODULE_VERSION("0.1");

/*
 * [확인 질문]
 *   Q1. children 리스트에 연결된 각 원소는 자식 task_struct의 어떤 멤버인가?
 *       (sibling 인 이유를 그림으로 설명하시오)
 *   Q2. real_parent 와 parent 가 달라지는 상황은 언제인가?
 *       (힌트: ptrace로 디버거가 붙었을 때)
 *   Q3. 출력 결과를 pstree -p 와 비교하시오. 차이가 있다면 왜인가?
 */
