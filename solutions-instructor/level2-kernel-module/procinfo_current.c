// SPDX-License-Identifier: GPL-2.0
/*
 * procinfo_current.c — [강사용 정답] current 매크로
 * 26F 시스템프로그래밍특론 / Week 3 실습 · Level 2-A
 *
 * 참고: Linux v7.2.2 기준으로 작성. 배포 전 실습 환경(uname -r)에서
 *       한 번 빌드·적재해 확인할 것.
 */

#define pr_fmt(fmt) "procinfo: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/cred.h>

static int __init procinfo_init(void)
{
	pr_info("module loaded\n");

	/* TODO 1 */
	pr_info("current: comm=%s pid=%d tgid=%d state=%u prio=%d\n",
		current->comm,
		current->pid,
		current->tgid,
		current->__state,          /* 교재 3판의 task->state */
		current->prio);

	/* TODO 2 — real_parent는 RCU 보호 포인터이므로 읽기 구간 안에서 접근 */
	rcu_read_lock();
	pr_info("parent : comm=%s pid=%d\n",
		rcu_dereference(current->real_parent)->comm,
		rcu_dereference(current->real_parent)->pid);
	rcu_read_unlock();

	/* TODO 3 — 일반 프로세스는 mm != NULL, 커널 스레드는 mm == NULL */
	pr_info("mm     = %p  (%s)\n",
		current->mm,
		current->mm ? "user process" : "kernel thread");

	return 0;
}

static void __exit procinfo_exit(void)
{
	/* TODO 4 */
	pr_info("unload by: comm=%s pid=%d\n", current->comm, current->pid);
	pr_info("module unloaded\n");
}

module_init(procinfo_init);
module_exit(procinfo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kyungwoon Lee");
MODULE_DESCRIPTION("Week 3 Lab (solution) - inspect the current task");
MODULE_VERSION("1.0");

/*
 * [모범 답안]
 *  A1. "insmod". 모듈의 init 함수는 insmod(8)가 finit_module() 시스템 콜을
 *      호출한 문맥에서 실행되므로, current는 insmod 프로세스를 가리킨다.
 *      exit 함수에서는 같은 이유로 "rmmod"가 나온다.
 *  A2. "module license 'unspecified' taints kernel" 경고가 뜨고 커널이
 *      tainted로 표시된다. 또한 EXPORT_SYMBOL_GPL로만 공개된 심볼을 참조하면
 *      링크 단계에서 "Unknown symbol" 오류가 난다.
 *  A3. 커널에는 libc가 없다. printk()/pr_info()는 커널 내부 링 버퍼에 기록하고
 *      로그 레벨을 지정할 수 있으며, 인터럽트 문맥에서도 안전하게 호출된다.
 */
