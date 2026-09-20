/*
 * 01_fork_basic.c — [강사용 정답] fork()로 자식 프로세스 만들기
 * 26F 시스템프로그래밍특론 / Week 3 실습 · Level 1-A
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
	int counter = 0;
	pid_t pid;
	int status;

	printf("[before fork] pid=%d ppid=%d counter=%d\n",
	       getpid(), getppid(), counter);
	/*
	 * stdout이 파이프나 파일로 리다이렉트되면 전 버퍼링(fully buffered)이
	 * 된다. 버퍼를 비우지 않고 fork()하면 버퍼 내용까지 복제되어 같은 줄이
	 * 두 번 출력된다. 직접 확인해 볼 것:  ./01_fork_basic | cat
	 */
	fflush(stdout);

	pid = fork();

	if (pid < 0) {
		perror("fork");
		return EXIT_FAILURE;
	} else if (pid == 0) {
		/* 자식 */
		counter += 100;
		printf("[child ] pid=%d ppid=%d counter=%d\n",
		       getpid(), getppid(), counter);
		exit(42);
	} else {
		/* 부모 */
		counter += 1;
		printf("[parent] pid=%d child=%d counter=%d\n",
		       getpid(), pid, counter);

		if (wait(&status) < 0) {
			perror("wait");
			return EXIT_FAILURE;
		}
		if (WIFEXITED(status))
			printf("[parent] child exited with code %d\n",
			       WEXITSTATUS(status));
		else if (WIFSIGNALED(status))
			printf("[parent] child killed by signal %d\n",
			       WTERMSIG(status));
	}

	return EXIT_SUCCESS;
}

/*
 * [모범 답안]
 *  A1. 부모는 1, 자식은 100. fork() 시점에 주소 공간이 논리적으로 복제되며
 *      (실제로는 Copy-on-Write) 이후 두 프로세스는 서로 다른 사본을 수정한다.
 *  A2. 자식이 먼저 끝나면 좀비(Z)로 남는다. 부모가 wait()로 종료 상태를
 *      회수해야 task_struct와 PID가 해제된다.
 *  A3. fork() → clone() 시스템 콜 → kernel_clone()  (kernel/fork.c:2694)
 *                                  → copy_process() (kernel/fork.c:1994)
 *      https://elixir.bootlin.com/linux/v7.2.2/source/kernel/fork.c#L2694
 */
