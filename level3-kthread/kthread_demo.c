// SPDX-License-Identifier: GPL-2.0
/*
 * kthread_demo.c — 커널 스레드 만들기
 *
 * 26F 시스템프로그래밍특론 / Week 3 실습 · Level 3
 *
 * [학습 목표]
 *   - 커널 스레드는 주소 공간(mm)이 없고 user space로 문맥 전환하지 않는다.
 *   - 그럼에도 스케줄 가능하고 선점 가능한, 정상적인 task_struct다.
 *   - kthread_run / kthread_should_stop / kthread_stop 의 협력 종료 패턴을 익힌다.
 *
 * [빌드]  make
 * [적재]  sudo insmod kthread_demo.ko interval=2
 * [확인]  ps -eo pid,ppid,stat,comm | grep knu_worker
 *         sudo dmesg -w            (Ctrl+C로 중단)
 * [제거]  sudo rmmod kthread_demo
 *
 * [참고할 커널 소스 — Elixir v7.2.2]
 *   kthread_run, kthread_should_stop : include/linux/kthread.h
 *   kthread 구현                      : kernel/kthread.c
 */

/* pr_fmt은 반드시 헤더 include 앞에 정의해야 적용된다 (dmesg 접두어) */
#define pr_fmt(fmt) "kthdemo: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/err.h>

static int interval = 2;          /* 초 단위 출력 주기 */
module_param(interval, int, 0444);
MODULE_PARM_DESC(interval, "seconds between messages (default 2)");

static struct task_struct *worker;

/*
 * 커널 스레드의 본체.
 * kthread_stop()이 호출될 때까지 루프를 돌다가 스스로 리턴한다.
 */
static int worker_fn(void *data)
{
	unsigned long tick = 0;

	pr_info("worker started: pid=%d comm=%s mm=%p\n",
		current->pid, current->comm, current->mm);

	/* ------------------------------------------------------------------
	 * TODO 1: kthread_should_stop()이 true를 리턴할 때까지 도는 루프를 작성하시오.
	 *
	 *   while (!kthread_should_stop()) {
	 *           pr_info("tick %lu (pid=%d)\n", tick++, current->pid);
	 *
	 *           /​* 잠들 때는 busy-wait 하지 말 것! *​/
	 *           ssleep(interval);          // 또는 msleep_interruptible(interval * 1000)
	 *   }
	 *
	 *   ※ 커널 스레드 안에서 while(1); 같은 busy loop을 돌리면
	 *      해당 CPU가 통째로 묶여 시스템이 멈춘 것처럼 보인다. 반드시 잠들 것.
	 * ------------------------------------------------------------------ */
	(void)tick;

	pr_info("worker stopping\n");
	return 0;
}

static int __init kthdemo_init(void)
{
	/* ------------------------------------------------------------------
	 * TODO 2: kthread_run()으로 커널 스레드를 생성하시오.
	 *
	 *   worker = kthread_run(worker_fn, NULL, "knu_worker");
	 *   if (IS_ERR(worker)) {
	 *           pr_err("failed to create kthread\n");
	 *           return PTR_ERR(worker);
	 *   }
	 *
	 *   kthread_run = kthread_create() + wake_up_process() 이다.
	 *   Elixir에서 include/linux/kthread.h 를 열어 정의를 확인할 것.
	 * ------------------------------------------------------------------ */

	pr_info("module loaded\n");
	return 0;
}

static void __exit kthdemo_exit(void)
{
	/* ------------------------------------------------------------------
	 * TODO 3: kthread_stop(worker)로 스레드를 정지시키시오.
	 *
	 *   kthread_stop()은 should_stop 플래그를 세우고,
	 *   worker_fn이 리턴할 때까지 "블록되어 기다린다".
	 *   따라서 worker_fn이 영원히 잠들어 있으면 rmmod가 멈춘다 — 주의!
	 * ------------------------------------------------------------------ */

	pr_info("module unloaded\n");
}

module_init(kthdemo_init);
module_exit(kthdemo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<학번> <이름>");
MODULE_DESCRIPTION("Week 3 Lab - create and stop a kernel thread");
MODULE_VERSION("0.1");

/*
 * [심화 과제]
 *   A. 커널 스레드를 N개 생성하도록 확장하시오 (모듈 파라미터 nr_workers).
 *      이름은 "knu_worker/%d" 형식으로 붙일 것.
 *   B. 각 워커가 for_each_process로 현재 프로세스 수를 세어 주기적으로 출력하게 하시오.
 *   C. 두 워커가 공유 카운터를 증가시키도록 만든 뒤, 락 없이 돌렸을 때와
 *      spinlock을 걸었을 때의 결과를 비교하시오. (13주차 동기화의 예고편)
 *
 * [확인 질문]
 *   Q1. worker_fn에서 출력한 current->mm 의 값은 무엇이었는가?
 *       일반 프로세스에서는 어떤 값이 나오는가? 왜 다른가?
 *   Q2. ps 출력에서 이 스레드의 PPID는 무엇인가? (힌트: kthreadd, PID 2)
 *   Q3. kthread_stop()을 호출하지 않고 rmmod하면 무슨 일이 벌어지는가?
 *       (실제로 시도하지 말고, 왜 위험한지 논리적으로 설명하시오)
 */
