/*
 * 01_fork_basic.c — fork()로 자식 프로세스 만들기
 *
 * 26F 시스템프로그래밍특론 / Week 3 실습 · Level 1-A
 *
 * [학습 목표]
 *   - fork()가 한 번 호출되어 두 번 리턴한다는 의미를 눈으로 확인한다.
 *   - 부모와 자식이 각각 무엇을 보는지(pid, ppid, 변수 사본) 구분한다.
 *
 * [빌드]  make 01_fork_basic
 * [실행]  ./01_fork_basic
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
	int counter = 0;          /* fork() 이후 부모/자식이 각자 사본을 갖는다 */
	pid_t pid;

	printf("[before fork] pid=%d ppid=%d counter=%d\n",
	       getpid(), getppid(), counter);
	/*
	 * stdout이 파이프나 파일로 리다이렉트되면 전 버퍼링(fully buffered)이
	 * 된다. 버퍼를 비우지 않고 fork()하면 버퍼 내용까지 복제되어 같은 줄이
	 * 두 번 출력된다. 직접 확인해 볼 것:  ./01_fork_basic | cat
	 */
	fflush(stdout);

	/* ------------------------------------------------------------------
	 * TODO 1: fork()를 호출하고 반환값을 pid에 저장하시오.
	 *         fork()는 실패 시 -1, 자식에서는 0, 부모에서는 자식의 PID를
	 *         반환한다. 반드시 세 경우를 모두 처리할 것.
	 * ------------------------------------------------------------------ */
	pid = /* TODO 1 */ -1;

	if (pid < 0) {
		/* TODO 2: perror()로 오류를 출력하고 EXIT_FAILURE로 종료하시오. */
		return EXIT_FAILURE;
	} else if (pid == 0) {
		/* ---------------- 자식 프로세스 ---------------- */
		counter += 100;
		/* TODO 3: 자식의 pid, ppid, counter를 출력하시오.
		 *         부모의 pid와 자식의 ppid가 같은지 확인할 것. */

		/* 자식은 exit()로 명시적으로 종료한다. 종료 코드 42를 사용하시오. */
		/* TODO 4 */
	} else {
		/* ---------------- 부모 프로세스 ---------------- */
		counter += 1;
		/* TODO 5: 부모의 pid, 자식의 pid(=변수 pid), counter를 출력하시오. */

		/* TODO 6: wait()로 자식이 끝날 때까지 기다리고,
		 *         WIFEXITED / WEXITSTATUS로 종료 코드를 출력하시오. */
	}

	return EXIT_SUCCESS;
}

/*
 * [확인 질문 — 보고서에 답을 적을 것]
 *   Q1. fork() 직후 counter의 값은 부모와 자식에서 각각 얼마인가? 왜 그런가?
 *   Q2. 부모가 wait()를 호출하지 않으면 어떤 일이 벌어지는가? (03번 실습과 연결)
 *   Q3. 커널에서 fork()는 결국 어떤 함수로 이어지는가?
 *       → Elixir에서 kernel_clone / copy_process 를 찾아 경로와 줄 번호를 적을 것.
 */
