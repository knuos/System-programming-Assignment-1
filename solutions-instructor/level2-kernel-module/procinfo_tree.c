// SPDX-License-Identifier: GPL-2.0
/*
 * procinfo_tree.c — [강사용 정답] 프로세스 계보 탐색
 * 26F 시스템프로그래밍특론 / Week 3 실습 · Level 2-C
 */

#define pr_fmt(fmt) "proctree: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/pid.h>
#include <linux/list.h>
#include <linux/rcupdate.h>

static int target = 1;
module_param(target, int, 0444);
MODULE_PARM_DESC(target, "PID whose family tree will be printed");

static int __init proctree_init(void)
{
	struct task_struct *task, *child, *p;
	struct pid *pid_struct;
	int nr_children = 0;
	int depth = 0;

	/* TODO 1 — 방법 A */
	rcu_read_lock();
	pid_struct = find_get_pid(target);
	task = pid_struct ? pid_task(pid_struct, PIDTYPE_PID) : NULL;

	if (!task) {
		rcu_read_unlock();
		if (pid_struct)
			put_pid(pid_struct);
		pr_err("pid %d not found\n", target);
		return -ESRCH;
	}

	pr_info("=== family tree of pid %d ===\n", target);

	/* TODO 2 */
	pr_info("self   : pid=%-7d comm=%s\n", task->pid, task->comm);

	/*
	 * TODO 3
	 * real_parent — 실제로 이 태스크를 fork한 부모.
	 * parent      — 종료 시그널(SIGCHLD)을 받고 wait()로 회수할 부모.
	 * 평소에는 둘이 같지만, ptrace로 디버거가 붙으면 parent만 디버거로 바뀐다.
	 */
	pr_info("parent : pid=%-7d comm=%s\n",
		rcu_dereference(task->real_parent)->pid,
		rcu_dereference(task->real_parent)->comm);

	/* TODO 4 — children 리스트에는 자식의 sibling 멤버가 매달려 있다 */
	list_for_each_entry(child, &task->children, sibling) {
		pr_info("child  : pid=%-7d comm=%s\n", child->pid, child->comm);
		nr_children++;
	}

	pr_info("=== %d children ===\n", nr_children);

	/* TODO 5 (심화) — current에서 PID 1까지 거슬러 올라간다 */
	pr_info("--- ancestry of current ---\n");
	p = current;
	while (p && depth++ < 32) {
		pr_info("  [%d] pid=%-7d comm=%s\n", depth, p->pid, p->comm);
		if (p->pid == 1)
			break;
		p = rcu_dereference(p->real_parent);
	}

	rcu_read_unlock();
	put_pid(pid_struct);
	return 0;
}

static void __exit proctree_exit(void)
{
	pr_info("module unloaded\n");
}

module_init(proctree_init);
module_exit(proctree_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kyungwoon Lee");
MODULE_DESCRIPTION("Week 3 Lab (solution) - traverse the process family tree");
MODULE_VERSION("1.0");

/*
 * [모범 답안]
 *  A1. 부모의 children은 리스트 "헤드"이고, 그 리스트에 실제로 매달리는 노드는
 *      각 자식의 sibling 멤버다. 한 태스크는 자기 자식들의 헤드(children)와
 *      형제들 사이에서의 자기 자리(sibling)를 동시에 가져야 하므로 멤버가
 *      두 개 필요하다.
 *
 *        parent.children ⇄ childA.sibling ⇄ childB.sibling ⇄ (다시 parent.children)
 *
 *  A2. ptrace로 디버거(gdb, strace)가 붙었을 때. 이때 parent는 디버거를 가리키고
 *      real_parent는 원래 부모를 유지한다. 디버거가 떨어지면 parent가 복구된다.
 *  A3. pstree는 스레드까지 포함해 표시하지만(-p 옵션), 이 모듈은 children 리스트
 *      만 보므로 스레드 그룹 리더의 자식 프로세스만 보인다. 또 모듈이 도는
 *      순간과 pstree를 실행한 순간 사이에 프로세스가 생성·종료되면 차이가 난다.
 */
