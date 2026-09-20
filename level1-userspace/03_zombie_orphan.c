/*
 * 03_zombie_orphan.c — 좀비(zombie)와 고아(orphan) 프로세스 관찰하기
 *
 * 26F 시스템프로그래밍특론 / Week 3 실습 · Level 1-C
 *
 * [학습 목표]
 *   - 좀비: 자식은 끝났는데 부모가 wait()하지 않아 task_struct가 남아 있는 상태(Z)
 *   - 고아: 부모가 먼저 죽어 init/systemd(PID 1)에게 입양된 프로세스
 *
 * [빌드]  make 03_zombie_orphan
 * [실행]  ./03_zombie_orphan zombie
 *         ./03_zombie_orphan orphan
 *
 * [관찰]  다른 터미널에서
 *           ps -eo pid,ppid,stat,comm | grep 03_zombie
 *           cat /proc/<pid>/status | head -n 8
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* ------------------------------------------------------------------
 * 좀비 만들기
 *   자식은 즉시 종료, 부모는 wait() 없이 오래 살아 있는다.
 *   → ps에서 자식의 STAT가 Z (defunct)로 보인다.
 * ------------------------------------------------------------------ */
static void make_zombie(void)
{
	pid_t pid;

	/* TODO 1: fork() 호출 */
	pid = -1;

	if (pid == 0) {
		printf("[child ] pid=%d — 바로 종료합니다\n", getpid());
		_exit(0);
	}

	printf("[parent] pid=%d, child=%d\n", getpid(), pid);
	printf("         → 다른 터미널에서 확인:  ps -o pid,ppid,stat,comm -p %d\n", pid);

	/* TODO 2: wait()를 호출하지 말고 30초 동안 sleep 하시오.
	 *         그 사이에 ps로 자식의 STAT가 Z 인지 확인할 것. */

	/* TODO 3: 30초가 지난 뒤 wait()를 호출하시오.
	 *         wait() 직후 다시 ps로 확인하면 좀비가 사라져 있다.
	 *         → 이것이 "부모가 자식을 회수(reap)한다"는 의미다. */
}

/* ------------------------------------------------------------------
 * 고아 만들기
 *   부모가 먼저 종료하고 자식은 계속 살아 있는다.
 *   → 자식의 PPID가 1(또는 systemd --user의 PID)로 바뀐다.
 * ------------------------------------------------------------------ */
static void make_orphan(void)
{
	pid_t pid;

	/* TODO 4: fork() 호출 */
	pid = -1;

	if (pid == 0) {
		printf("[child ] pid=%d ppid=%d — 부모가 아직 살아 있음\n",
		       getpid(), getppid());

		/* TODO 5: 5초 sleep 한 뒤 다시 getppid()를 출력하시오.
		 *         값이 어떻게 바뀌었는가? */

		_exit(0);
	}

	printf("[parent] pid=%d — 자식을 남겨두고 즉시 종료합니다\n", getpid());
	/* 부모는 곧바로 리턴하여 종료 → 자식은 고아가 된다 */
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s {zombie|orphan}\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (strcmp(argv[1], "zombie") == 0)
		make_zombie();
	else if (strcmp(argv[1], "orphan") == 0)
		make_orphan();
	else {
		fprintf(stderr, "unknown mode: %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
 * [확인 질문]
 *   Q1. 좀비 프로세스는 어떤 자원을 여전히 점유하고 있는가?
 *       (힌트: 주소 공간은 이미 해제되었다. 그렇다면 남아 있는 것은?)
 *   Q2. 좀비가 대량으로 쌓이면 시스템에 어떤 문제가 생기는가?
 *       → /proc/sys/kernel/pid_max 값을 확인해 볼 것.
 *   Q3. 고아 프로세스를 입양하는 프로세스는 무엇인가? PPID 값을 적으시오.
 *   Q4. 커널에서 자식을 재부모화(reparenting)하는 코드는 어디에 있는가?
 *       → Elixir에서 kernel/exit.c 의 forget_original_parent / reparent_leader 참고
 */
