/*
 * 02_fork_exec_wait.c — [강사용 정답] fork + exec + wait
 * 26F 시스템프로그래밍특론 / Week 3 실습 · Level 1-B
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

	pid = fork();
	if (pid < 0) {
		perror("fork");
		return EXIT_FAILURE;
	}

	if (pid == 0) {
		printf("[child ] pid=%d, exec \"%s\"\n", getpid(), argv[1]);
		fflush(stdout);          /* exec 전에 버퍼를 비운다 */

		execvp(argv[1], &argv[1]);

		/* 여기 도달 = exec 실패 */
		perror("execvp");
		/*
		 * _exit()를 쓰는 이유: exit()는 atexit 핸들러를 돌리고 stdio
		 * 버퍼를 flush 한다. 자식은 부모의 버퍼 사본을 갖고 있으므로
		 * 같은 내용이 두 번 출력될 수 있다.
		 */
		_exit(127);
	}

	waited = waitpid(pid, &status, 0);
	if (waited < 0) {
		perror("waitpid");
		return EXIT_FAILURE;
	}

	if (WIFEXITED(status))
		printf("[parent] child %d exited, code=%d\n",
		       waited, WEXITSTATUS(status));
	else if (WIFSIGNALED(status))
		printf("[parent] child %d killed by signal %d\n",
		       waited, WTERMSIG(status));
	else
		printf("[parent] child %d finished (status=0x%x)\n",
		       waited, status);

	return EXIT_SUCCESS;
}

/*
 * [모범 답안]
 *  A1. exec()는 현재 프로세스의 주소 공간(텍스트·데이터·힙·스택)을 새 실행
 *      파일의 이미지로 통째로 교체한다. 호출한 코드 자체가 메모리에서
 *      사라지므로 돌아올 곳이 없다. PID와 열린 파일 디스크립터는 유지된다.
 *  A2. fork와 exec 사이의 "틈"을 잃는다. 셸은 그 틈에서 자식만의 리다이렉션
 *      (dup2), 시그널 핸들러 초기화, setuid/setgid, 작업 그룹 설정 등을 한다.
 *  A3. fork()는 주소 공간을 실제로 복사하지 않고 페이지 테이블만 복제하며
 *      모든 페이지를 read-only로 표시한다(Copy-on-Write). 곧바로 exec하면
 *      쓰기가 거의 발생하지 않아 실제 복사 비용이 들지 않는다.
 */
