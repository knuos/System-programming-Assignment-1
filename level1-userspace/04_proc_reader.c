/*
 * 04_proc_reader.c — /proc를 통해 커널이 보는 프로세스 정보 읽기
 *
 * 26F 시스템프로그래밍특론 / Week 3 실습 · Level 1-D
 *
 * [학습 목표]
 *   - /proc은 디스크가 아니라 커널이 만들어 내는 가상 파일시스템이다.
 *   - /proc/<pid>/status 의 각 필드가 task_struct의 어떤 멤버에서 오는지 대응시킨다.
 *
 * [빌드]  make 04_proc_reader
 * [실행]  ./04_proc_reader          (자기 자신)
 *         ./04_proc_reader 1        (PID 1 = init/systemd)
 *         ./04_proc_reader $$       (현재 셸)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LINE_MAX_LEN 512

/* status 파일에서 뽑아 볼 필드들 */
static const char *keys[] = {
	"Name:", "State:", "Tgid:", "Pid:", "PPid:",
	"Threads:", "VmSize:", "VmRSS:", "voluntary_ctxt_switches:",
	NULL
};

static int starts_with_key(const char *line)
{
	int i;

	for (i = 0; keys[i]; i++) {
		if (strncmp(line, keys[i], strlen(keys[i])) == 0)
			return 1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	char path[64];
	char line[LINE_MAX_LEN];
	FILE *fp;
	pid_t target;

	target = (argc > 1) ? (pid_t)atoi(argv[1]) : getpid();

	/* ------------------------------------------------------------------
	 * TODO 1: snprintf()로 "/proc/<target>/status" 경로를 만드시오.
	 * ------------------------------------------------------------------ */
	snprintf(path, sizeof(path), "/proc/%d/status", (int)target);

	fp = fopen(path, "r");
	if (!fp) {
		perror(path);
		return EXIT_FAILURE;
	}

	printf("===== %s =====\n", path);
	while (fgets(line, sizeof(line), fp)) {
		if (starts_with_key(line))
			fputs(line, stdout);
	}
	fclose(fp);

	/* ------------------------------------------------------------------
	 * TODO 2: 같은 방식으로 아래 항목들을 추가로 출력하시오.
	 *   (a) /proc/<pid>/cmdline  — 인자들이 '\0'으로 구분되어 있음에 주의
	 *   (b) /proc/<pid>/stat     — 세 번째 필드가 상태 문자(R/S/D/Z/T)
	 *   (c) /proc/<pid>/task/    — 디렉터리 개수 = 이 프로세스의 스레드 수
	 *                              (opendir/readdir 사용)
	 * ------------------------------------------------------------------ */

	/* ------------------------------------------------------------------
	 * TODO 3 (심화): 인자를 주지 않으면 /proc 전체를 순회하여
	 *   숫자 이름의 디렉터리(= 프로세스)를 찾아
	 *   "PID  PPID  STATE  NAME" 형식의 표를 출력하시오.
	 *   → Level 2에서 커널 모듈로 만들 것과 같은 출력을 유저 공간에서 먼저 만들어 본다.
	 * ------------------------------------------------------------------ */

	return EXIT_SUCCESS;
}

/*
 * [확인 질문]
 *   Q1. State 필드에 나타나는 R / S / D / Z / T 는 각각 무슨 뜻인가?
 *       커널 상수(TASK_RUNNING, TASK_INTERRUPTIBLE, TASK_UNINTERRUPTIBLE …)와 대응시키시오.
 *   Q2. Tgid와 Pid가 다른 경우는 언제인가? (힌트: 스레드)
 *   Q3. /proc/<pid>/status 를 만들어 내는 커널 코드는 어디에 있는가?
 *       → Elixir에서 fs/proc/array.c 의 proc_pid_status 를 찾아 경로와 줄 번호를 적으시오.
 */
