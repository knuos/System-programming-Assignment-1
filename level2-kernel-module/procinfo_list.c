// SPDX-License-Identifier: GPL-2.0
/*
 * procinfo_list.c — task list를 순회하여 전체 프로세스 출력하기
 *
 * 26F 시스템프로그래밍특론 / Week 3 실습 · Level 2-B
 *
 * [학습 목표]
 *   - 커널이 모든 프로세스를 "원형 이중 연결 리스트(task list)"로 관리함을 확인한다.
 *   - for_each_process 매크로가 결국 list_for_each_entry_rcu 라는 것을 소스로 확인한다.
 *   - ps 명령이 보여주는 것과 같은 정보를 커널 쪽에서 직접 만들어 본다.
 *
 * [빌드]  make
 * [적재]  sudo insmod procinfo_list.ko
 * [확인]  sudo dmesg | tail -n 60
 * [비교]  ps -eo pid,ppid,stat,comm | head -n 20
 * [제거]  sudo rmmod procinfo_list
 *
 * [참고할 커널 소스 — Elixir v7.2.2]
 *   for_each_process   : include/linux/sched/signal.h  (line 640)
 *   struct task_struct : include/linux/sched.h         (line 826)
 */

/* pr_fmt은 반드시 헤더 include 앞에 정의해야 적용된다 (dmesg 접두어) */
#define pr_fmt(fmt) "proclist: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>   /* for_each_process */
#include <linux/rcupdate.h>

/* 모듈 파라미터: 출력할 최대 개수 (dmesg가 넘치지 않도록)
 *   sudo insmod procinfo_list.ko limit=30
 */
static int limit = 20;
module_param(limit, int, 0444);
MODULE_PARM_DESC(limit, "number of tasks to print (default 20)");

/* 상태 비트를 사람이 읽을 수 있는 문자로 바꾼다 */
static char state_char(unsigned int state)
{
	/* TODO 1: 아래 매핑을 완성하시오.
	 *   0                        → 'R'  (TASK_RUNNING)
	 *   TASK_INTERRUPTIBLE       → 'S'
	 *   TASK_UNINTERRUPTIBLE     → 'D'
	 *   TASK_STOPPED             → 'T'
	 *   그 외                     → '?'
	 *
	 *   ※ 좀비/종료 상태는 __state가 아니라 task->exit_state 에 들어 있다.
	 *      (EXIT_ZOMBIE, EXIT_DEAD) — 심화 과제에서 함께 처리해 볼 것.
	 */
	if (state == 0)
		return 'R';

	return '?';
}

static int __init proclist_init(void)
{
	struct task_struct *task;
	int count = 0;

	pr_info("=== task list (limit=%d) ===\n", limit);
	pr_info("%-8s %-8s %-5s %s\n", "PID", "PPID", "STATE", "COMM");

	/* ------------------------------------------------------------------
	 * TODO 2: 리스트 순회 중 태스크가 사라지지 않도록 RCU 읽기 구간을 연다.
	 *         rcu_read_lock();
	 * ------------------------------------------------------------------ */

	/* ------------------------------------------------------------------
	 * TODO 3: for_each_process(task) 루프를 작성하고,
	 *         각 태스크마다 아래 형식으로 출력하시오.
	 *
	 *         pr_info("%-8d %-8d %-5c %s\n",
	 *                 task->pid,
	 *                 task->real_parent->pid,
	 *                 state_char(task->__state),
	 *                 task->comm);
	 *
	 *         count가 limit에 도달하면 break 할 것.
	 * ------------------------------------------------------------------ */
	(void)task;
	(void)count;

	/* TODO 4: rcu_read_unlock(); */

	pr_info("=== printed %d tasks ===\n", count);
	return 0;
}

static void __exit proclist_exit(void)
{
	pr_info("module unloaded\n");
}

module_init(proclist_init);
module_exit(proclist_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<학번> <이름>");
MODULE_DESCRIPTION("Week 3 Lab - walk the kernel task list");
MODULE_VERSION("0.1");

/*
 * [심화 과제]
 *   A. TASK_RUNNING 상태인 태스크의 수만 세어 출력하시오.
 *   B. task->mm == NULL 인 태스크(= 커널 스레드)를 [대괄호]로 감싸 출력하시오.
 *      ps aux 출력에서 [kworker/0:1] 처럼 보이는 것과 같은 표기다.
 *   C. 이름(comm)에 특정 문자열이 포함된 태스크만 출력하도록
 *      문자열 모듈 파라미터 filter 를 추가하시오.
 *
 * [확인 질문]
 *   Q1. for_each_process 는 실제로 어떤 매크로로 전개되는가?
 *       → Elixir에서 include/linux/sched/signal.h:640 을 열어 확인하고 적으시오.
 *   Q2. 이 매크로는 프로세스(스레드 그룹 리더)만 순회한다. 한 프로세스의 모든
 *       스레드까지 보려면 어떤 매크로가 필요한가? (힌트: for_each_thread)
 *   Q3. 순회 중 rcu_read_lock()이 필요한 이유를 설명하시오.
 *       락 없이 순회하면 어떤 문제가 생기는가?
 */
