// SPDX-License-Identifier: GPL-2.0
/*
 * procinfo_current.c — current 매크로로 "지금 이 코드를 실행 중인 태스크" 보기
 *
 * 26F 시스템프로그래밍특론 / Week 3 실습 · Level 2-A
 *
 * [학습 목표]
 *   - 커널 모듈의 뼈대(module_init / module_exit / MODULE_LICENSE)를 익힌다.
 *   - 모듈의 init 함수가 "insmod를 실행한 프로세스의 문맥(process context)"에서
 *     동작한다는 사실을 current로 직접 확인한다.
 *
 * [빌드]  make
 * [적재]  sudo insmod procinfo_current.ko
 * [확인]  sudo dmesg | tail -n 20
 * [제거]  sudo rmmod procinfo_current
 *
 * [참고할 커널 소스 — Elixir v7.2.2]
 *   struct task_struct : include/linux/sched.h  (line 826)
 *   current            : arch/x86/include/asm/current.h
 */

/* pr_fmt은 반드시 헤더 include 앞에 정의해야 적용된다 (dmesg 접두어) */
#define pr_fmt(fmt) "procinfo: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>          /* struct task_struct, current */
#include <linux/sched/signal.h>   /* for_each_process (다음 예제에서 사용) */
#include <linux/cred.h>           /* current_uid() */

static int __init procinfo_init(void)
{
	pr_info("module loaded\n");

	/* ------------------------------------------------------------------
	 * TODO 1: current를 이용해 아래 정보를 pr_info()로 출력하시오.
	 *
	 *   current->comm       (char[TASK_COMM_LEN])  프로세스 이름
	 *   current->pid        (pid_t)                스레드 ID
	 *   current->tgid       (pid_t)                프로세스(스레드 그룹) ID
	 *   current->__state    (unsigned int)         상태 비트
	 *   current->prio       (int)                  우선순위
	 *
	 *   예)
	 *   pr_info("current: comm=%s pid=%d tgid=%d state=%u prio=%d\n",
	 *           current->comm, current->pid, current->tgid,
	 *           current->__state, current->prio);
	 *
	 *   ※ 교재(3판)에는 task->state 로 나오지만 최신 커널은 __state 이다.
	 *      Elixir에서 include/linux/sched.h 를 열어 직접 확인할 것.
	 * ------------------------------------------------------------------ */

	/* ------------------------------------------------------------------
	 * TODO 2: current->real_parent 를 따라가 부모 프로세스의 comm과 pid도
	 *         함께 출력하시오. insmod를 실행한 셸이 보이는가?
	 * ------------------------------------------------------------------ */

	/* ------------------------------------------------------------------
	 * TODO 3 (심화): current->mm 이 NULL인지 확인하는 코드를 넣으시오.
	 *         일반 프로세스와 커널 스레드의 차이가 여기서 드러난다.
	 *         (Level 3에서 다시 확인한다)
	 * ------------------------------------------------------------------ */

	return 0;   /* 0을 리턴해야 모듈이 정상 적재된다 */
}

static void __exit procinfo_exit(void)
{
	/* TODO 4: rmmod를 실행한 프로세스의 comm/pid를 출력하시오.
	 *         init 때와 같은 프로세스인가? 왜 그런가/아닌가? */

	pr_info("module unloaded\n");
}

module_init(procinfo_init);
module_exit(procinfo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<학번> <이름>");
MODULE_DESCRIPTION("Week 3 Lab - inspect the current task");
MODULE_VERSION("0.1");

/*
 * [확인 질문]
 *   Q1. insmod로 적재했을 때 current->comm 은 무엇이었는가? 왜 그 값인가?
 *   Q2. MODULE_LICENSE("GPL")을 빼면 어떤 경고가 나오는가?
 *       (힌트: EXPORT_SYMBOL_GPL로 공개된 심볼을 쓸 수 없게 된다)
 *   Q3. 커널 모듈에서 printf() 대신 pr_info()/printk()를 쓰는 이유는?
 */
