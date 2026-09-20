/*
 * 02_fork_exec_wait.c — fork() + exec() + wait()로 외부 명령 실행하기
 *
 * 26F 시스템프로그래밍특론 / Week 3 실습 · Level 1-B
 *
 * [학습 목표]
 *   - UNIX가 "프로세스 생성(fork)"과 "프로그램 적재(exec)"를 왜 분리했는지 이해한다.
 *   - exec() 성공 시 원래 코드가 사라진다는 사실(= 리턴하지 않는다)을 확인한다.
 *
 * [빌드]  make 02_fork_exec_wait
 * [실행]  ./02_fork_exec_wait /bin/ls -l /
 *         ./02_fork_exec_wait /bin/sleep 3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	pid_t pid, waited;
	int status;

	if (argc < 2) {
		fprintf(stderr, "usage: %s <program> [args...]\n", argv[0]);
		return EXIT_FAILURE;
	}

	printf("[parent] pid=%d, about to fork\n", getpid());

	/* TODO 1: fork() 호출 및 오류 처리 */
	pid = -1;

	if (pid == 0) {
		/* ---------------- 자식: 새 프로그램으로 교체 ---------------- */
		printf("[child ] pid=%d, exec \"%s\"\n", getpid(), argv[1]);

		/* TODO 2: execvp()로 argv[1]을 실행하시오.
		 *         인자 배열은 &argv[1]을 그대로 넘기면 된다.
		 *         (argv는 NULL로 끝나므로 execvp의 규약을 만족한다) */

		/* TODO 3: 아래 줄이 실행된다면 그것은 exec가 "실패"했다는 뜻이다.
		 *         perror("execvp") 후 _exit(127)로 종료하시오.
		 *         왜 exit()가 아니라 _exit()를 쓰는지 생각해 볼 것. */
		return 127;
	}

	/* ---------------- 부모: 자식을 기다린다 ---------------- */
	/* TODO 4: waitpid(pid, &status, 0)으로 자식을 기다리시오. */
	waited = -1;
	status = 0;

	if (waited < 0) {
		perror("waitpid");
		return EXIT_FAILURE;
	}

	/* TODO 5: 아래 매크로로 종료 원인을 구분해 출력하시오.
	 *   WIFEXITED(status)   — 정상 종료했는가?   WEXITSTATUS(status)  종료 코드
	 *   WIFSIGNALED(status) — 시그널로 죽었는가? WTERMSIG(status)     시그널 번호
	 *
	 *   시그널 종료를 확인하려면 다른 터미널에서
	 *     ./02_fork_exec_wait /bin/sleep 30
	 *   를 실행한 뒤 kill -9 <자식 pid> 를 보내 보시오.
	 */
	printf("[parent] child %d finished (raw status=0x%x)\n", waited, status);

	return EXIT_SUCCESS;
}

/*
 * [확인 질문]
 *   Q1. exec()가 성공하면 왜 그 아래 코드가 실행되지 않는가?
 *       (프로세스의 주소 공간에 무슨 일이 일어나는가?)
 *   Q2. fork()와 exec()가 하나의 시스템 콜로 합쳐져 있다면 잃게 되는 것은?
 *       (힌트: 자식에서 exec 전에 리다이렉션/권한 변경을 할 수 있다)
 *   Q3. Copy-on-Write 덕분에 fork() 직후 곧바로 exec()해도 손해가 크지 않은 이유는?
 */
