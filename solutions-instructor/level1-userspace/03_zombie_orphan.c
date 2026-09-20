/*
 * 03_zombie_orphan.c — [강사용 정답] 좀비와 고아 프로세스
 * 26F 시스템프로그래밍특론 / Week 3 실습 · Level 1-C
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static void make_zombie(void)
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid < 0) {
		perror("fork");
		return;
	}

	if (pid == 0) {
		printf("[child ] pid=%d — 바로 종료합니다\n", getpid());
		_exit(0);
	}

	printf("[parent] pid=%d, child=%d\n", getpid(), pid);
	printf("         → 지금 확인:  ps -o pid,ppid,stat,comm -p %d\n", pid);
	printf("         → 30초 동안 wait() 하지 않습니다 (좀비 상태)\n");
	fflush(stdout);

	sleep(30);                       /* 이 사이에 자식은 STAT=Z */

	if (wait(&status) > 0)
		printf("[parent] 자식을 회수했습니다 (code=%d)\n",
		       WEXITSTATUS(status));

	printf("         → 다시 확인:  ps -o pid,ppid,stat,comm -p %d  (사라졌다)\n",
	       pid);
}

static void make_orphan(void)
{
	pid_t pid;

	pid = fork();
	if (pid < 0) {
		perror("fork");
		return;
	}

	if (pid == 0) {
		printf("[child ] pid=%d ppid=%d — 부모가 아직 살아 있음\n",
		       getpid(), getppid());
		fflush(stdout);

		sleep(5);

		printf("[child ] pid=%d ppid=%d — 부모가 죽은 뒤 (입양됨)\n",
		       getpid(), getppid());
		_exit(0);
	}

	printf("[parent] pid=%d — 자식을 남겨두고 즉시 종료합니다\n", getpid());
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
 * [모범 답안]
 *  A1. 주소 공간·열린 파일 등 대부분의 자원은 이미 해제되었다. 남아 있는 것은
 *      task_struct(축소된 형태)와 그것이 점유한 PID, 그리고 종료 상태 정보다.
 *      부모가 wait()로 그 값을 읽어 가야 비로소 해제할 수 있기 때문이다.
 *  A2. PID는 유한한 자원이다(/proc/sys/kernel/pid_max, 보통 4194304 또는 32768).
 *      좀비가 쌓이면 PID가 고갈되어 새 프로세스를 만들 수 없게 된다.
 *      task_struct가 차지하는 슬랩 메모리도 낭비된다.
 *  A3. PID 1 — 전통적으로 init, 최근 배포판에서는 systemd.
 *      (사용자 세션에서는 systemd --user 프로세스에게 입양될 수도 있다)
 *  A4. kernel/exit.c 의 forget_original_parent() / reparent_leader()
 *      https://elixir.bootlin.com/linux/v7.2.2/A/ident/forget_original_parent
 */
