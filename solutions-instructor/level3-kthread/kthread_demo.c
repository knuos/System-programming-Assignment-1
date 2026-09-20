// SPDX-License-Identifier: GPL-2.0
/*
 * kthread_demo.c — [강사용 정답] 커널 스레드 생성과 종료
 * 26F 시스템프로그래밍특론 / Week 3 실습 · Level 3
 */

#define pr_fmt(fmt) "kthdemo: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/err.h>

static int interval = 2;
module_param(interval, int, 0444);
MODULE_PARM_DESC(interval, "seconds between messages (default 2)");

/* 심화 A: 워커 개수 */
static int nr_workers = 1;
module_param(nr_workers, int, 0444);
MODULE_PARM_DESC(nr_workers, "number of kernel threads (default 1)");

static struct task_struct **workers;
static int nr_created;

static int worker_fn(void *data)
{
	unsigned long tick = 0;
	long id = (long)data;

	pr_info("worker %ld started: pid=%d comm=%s mm=%p\n",
		id, current->pid, current->comm, current->mm);

	/* TODO 1 */
	while (!kthread_should_stop()) {
		struct task_struct *p;
		int nr_tasks = 0;

		/* 심화 B: 현재 프로세스 수를 세어 함께 출력 */
		rcu_read_lock();
		for_each_process(p)
			nr_tasks++;
		rcu_read_unlock();

		pr_info("worker %ld tick %lu (pid=%d, %d processes)\n",
			id, tick++, current->pid, nr_tasks);

		/*
		 * busy-wait 금지. ssleep()은 TASK_UNINTERRUPTIBLE로 잠들므로
		 * kthread_stop()의 wake_up_process()에도 즉시 깨지 않는다.
		 * 응답성이 필요하면 msleep_interruptible()을 쓴다.
		 */
		msleep_interruptible(interval * 1000);
	}

	pr_info("worker %ld stopping\n", id);
	return 0;
}

static int __init kthdemo_init(void)
{
	long i;

	if (nr_workers < 1)
		nr_workers = 1;

	workers = kcalloc(nr_workers, sizeof(*workers), GFP_KERNEL);
	if (!workers)
		return -ENOMEM;

	/* TODO 2 */
	for (i = 0; i < nr_workers; i++) {
		workers[i] = kthread_run(worker_fn, (void *)i,
					 "knu_worker/%ld", i);
		if (IS_ERR(workers[i])) {
			int err = PTR_ERR(workers[i]);

			pr_err("failed to create kthread %ld: %d\n", i, err);
			workers[i] = NULL;

			/* 이미 만든 것들은 정리하고 실패 반환 */
			while (--i >= 0)
				kthread_stop(workers[i]);
			kfree(workers);
			workers = NULL;
			return err;
		}
		nr_created++;
	}

	pr_info("module loaded (%d workers, interval=%ds)\n",
		nr_created, interval);
	return 0;
}

static void __exit kthdemo_exit(void)
{
	int i;

	/* TODO 3 */
	for (i = 0; i < nr_created; i++)
		if (workers[i])
			kthread_stop(workers[i]);   /* 리턴할 때까지 블록된다 */

	kfree(workers);
	pr_info("module unloaded\n");
}

module_init(kthdemo_init);
module_exit(kthdemo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kyungwoon Lee");
MODULE_DESCRIPTION("Week 3 Lab (solution) - create and stop kernel threads");
MODULE_VERSION("1.0");

/*
 * [모범 답안]
 *  A1. current->mm 은 NULL이다. 커널 스레드는 user space로 문맥 전환하지
 *      않으므로 자기 주소 공간이 필요 없다. 스케줄될 때는 직전 태스크의
 *      페이지 테이블을 그대로 빌려 쓰며, 그 포인터가 active_mm이다.
 *      일반 프로세스에서는 mm == active_mm != NULL 이다.
 *  A2. PPID는 2 — kthreadd. 모든 커널 스레드는 kthreadd가 생성하므로
 *      kthreadd(PID 2)의 자식이 된다. kthreadd 자신의 부모는 PID 0(swapper)다.
 *  A3. 모듈 코드가 메모리에서 해제된 뒤에도 worker_fn이 계속 실행되므로
 *      다음 명령어를 fetch하는 순간 존재하지 않는 주소를 실행하게 되어
 *      oops/panic이 발생한다. 모듈이 만든 모든 커널 스레드는 exit 함수에서
 *      반드시 회수해야 한다.
 *
 * [심화 C 관련 메모]
 *  두 워커가 락 없이 공유 카운터를 증가시키면 read-modify-write가 겹쳐
 *  갱신 손실이 발생한다. spinlock_t 또는 atomic_t로 해결한다.
 *  → 13주차 Kernel Synchronization에서 다룬다.
 */
