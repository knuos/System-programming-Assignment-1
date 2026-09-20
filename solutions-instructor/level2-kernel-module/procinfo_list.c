// SPDX-License-Identifier: GPL-2.0
/*
 * procinfo_list.c — [강사용 정답] for_each_process 순회
 * 26F 시스템프로그래밍특론 / Week 3 실습 · Level 2-B
 */

#define pr_fmt(fmt) "proclist: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/rcupdate.h>

static int limit = 20;
module_param(limit, int, 0444);
MODULE_PARM_DESC(limit, "number of tasks to print (default 20)");

/* 심화 C: comm에 포함될 문자열 필터 */
static char *filter = "";
module_param(filter, charp, 0444);
MODULE_PARM_DESC(filter, "print only tasks whose comm contains this string");

/* TODO 1 */
static char state_char(struct task_struct *t)
{
	/* 좀비/사망 상태는 __state가 아니라 exit_state에 들어 있다 */
	if (t->exit_state & EXIT_ZOMBIE)
		return 'Z';
	if (t->exit_state & EXIT_DEAD)
		return 'X';

	/*
	 * __state는 비트마스크다. switch로 값을 비교하면 커널 버전에 따라
	 * 상수가 합성 비트(예: TASK_STOPPED = TASK_WAKEKILL | __TASK_STOPPED)로
	 * 정의되어 있어 어긋날 수 있으므로 비트 검사로 처리한다.
	 */
	if (t->__state == TASK_RUNNING)
		return 'R';
	if (t->__state & TASK_INTERRUPTIBLE)
		return 'S';
	if (t->__state & TASK_UNINTERRUPTIBLE)
		return 'D';
	if (t->__state & __TASK_STOPPED)
		return 'T';

	return '?';
}

static int __init proclist_init(void)
{
	struct task_struct *task;
	int count = 0;
	int nr_running = 0;      /* 심화 A */
	char name[TASK_COMM_LEN + 4];

	pr_info("=== task list (limit=%d, filter=\"%s\") ===\n", limit, filter);
	pr_info("%-8s %-8s %-5s %s\n", "PID", "PPID", "STATE", "COMM");

	/* TODO 2 */
	rcu_read_lock();

	/* TODO 3 */
	for_each_process(task) {
		if (task->__state == TASK_RUNNING)
			nr_running++;

		if (filter[0] && !strstr(task->comm, filter))
			continue;

		/* 심화 B: 커널 스레드는 [대괄호]로 표시 (ps aux와 동일한 관례) */
		if (!task->mm)
			snprintf(name, sizeof(name), "[%s]", task->comm);
		else
			snprintf(name, sizeof(name), "%s", task->comm);

		pr_info("%-8d %-8d %-5c %s\n",
			task->pid,
			rcu_dereference(task->real_parent)->pid,
			state_char(task),
			name);

		if (++count >= limit)
			break;
	}

	/* TODO 4 */
	rcu_read_unlock();

	pr_info("=== printed %d tasks, %d in TASK_RUNNING ===\n",
		count, nr_running);
	return 0;
}

static void __exit proclist_exit(void)
{
	pr_info("module unloaded\n");
}

module_init(proclist_init);
module_exit(proclist_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kyungwoon Lee");
MODULE_DESCRIPTION("Week 3 Lab (solution) - walk the kernel task list");
MODULE_VERSION("1.0");

/*
 * [모범 답안]
 *  A1. include/linux/sched/signal.h:640
 *        #define for_each_process(p) \
 *                for (p = &init_task ; (p = next_task(p)) != &init_task ; )
 *      init_task(PID 0, swapper)를 시작점이자 종료 조건으로 쓰는 원형 리스트
 *      순회다. next_task()는 p->tasks 를 따라간다.
 *      https://elixir.bootlin.com/linux/v7.2.2/A/ident/for_each_process
 *  A2. for_each_thread(p, t) — 한 스레드 그룹 안의 모든 스레드를 순회한다.
 *      두 개를 중첩하면 시스템의 모든 태스크를 볼 수 있다:
 *        for_each_process(p) for_each_thread(p, t) { ... }
 *  A3. 순회 도중 다른 CPU에서 태스크가 종료되어 task_struct가 해제되면
 *      해제된 메모리를 역참조하게 된다(use-after-free → oops).
 *      RCU 읽기 구간 안에서는 grace period가 끝날 때까지 실제 해제가
 *      미뤄지므로 포인터가 유효함이 보장된다.
 *      (엄밀히는 tasklist_lock을 잡는 방법도 있으나 읽기 경로에서는 RCU가 표준)
 */
