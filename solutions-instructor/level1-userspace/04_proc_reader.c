/*
 * 04_proc_reader.c — [강사용 정답] /proc 읽기
 * 26F 시스템프로그래밍특론 / Week 3 실습 · Level 1-D
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>

#define LINE_LEN 512

static const char *keys[] = {
	"Name:", "State:", "Tgid:", "Pid:", "PPid:",
	"Threads:", "VmSize:", "VmRSS:", "voluntary_ctxt_switches:",
	NULL
};

static int wanted(const char *line)
{
	int i;

	for (i = 0; keys[i]; i++)
		if (strncmp(line, keys[i], strlen(keys[i])) == 0)
			return 1;
	return 0;
}

/* /proc/<pid>/status 에서 한 필드를 뽑아 out에 담는다 */
static int read_field(pid_t pid, const char *key, char *out, size_t outsz)
{
	char path[64], line[LINE_LEN];
	FILE *fp;
	int found = 0;

	snprintf(path, sizeof(path), "/proc/%d/status", (int)pid);
	fp = fopen(path, "r");
	if (!fp)
		return 0;

	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, key, strlen(key)) == 0) {
			const char *v = line + strlen(key);

			while (*v == ' ' || *v == '\t')
				v++;
			snprintf(out, outsz, "%s", v);
			out[strcspn(out, "\n")] = '\0';
			found = 1;
			break;
		}
	}
	fclose(fp);
	return found;
}

/* TODO 2 (a),(b),(c) */
static void dump_extra(pid_t pid)
{
	char path[64], buf[LINE_LEN];
	FILE *fp;
	DIR *d;
	struct dirent *e;
	size_t n, i;
	int nthreads = 0;

	/* (a) cmdline — 인자들이 '\0'으로 구분되어 있다 */
	snprintf(path, sizeof(path), "/proc/%d/cmdline", (int)pid);
	fp = fopen(path, "r");
	if (fp) {
		n = fread(buf, 1, sizeof(buf) - 1, fp);
		fclose(fp);
		buf[n] = '\0';
		printf("Cmdline:\t");
		for (i = 0; i < n; i++)
			putchar(buf[i] ? buf[i] : ' ');
		putchar('\n');
	}

	/* (b) stat — 세 번째 필드가 상태 문자.
	 *     comm이 괄호로 감싸여 있고 공백을 포함할 수 있으므로
	 *     마지막 ')' 를 기준으로 파싱한다. */
	snprintf(path, sizeof(path), "/proc/%d/stat", (int)pid);
	fp = fopen(path, "r");
	if (fp) {
		if (fgets(buf, sizeof(buf), fp)) {
			char *rp = strrchr(buf, ')');

			if (rp && *(rp + 1))
				printf("StatChar:\t%c\n", *(rp + 2));
		}
		fclose(fp);
	}

	/* (c) task/ 디렉터리 개수 = 스레드 수 */
	snprintf(path, sizeof(path), "/proc/%d/task", (int)pid);
	d = opendir(path);
	if (d) {
		while ((e = readdir(d))) {
			if (isdigit((unsigned char)e->d_name[0]))
				nthreads++;
		}
		closedir(d);
		printf("TaskDirs:\t%d\n", nthreads);
	}
}

/* TODO 3 (심화): /proc 전체 순회 */
static void list_all(void)
{
	DIR *d;
	struct dirent *e;
	char name[64], state[64], ppid[64];

	d = opendir("/proc");
	if (!d) {
		perror("/proc");
		return;
	}

	printf("%-8s %-8s %-6s %s\n", "PID", "PPID", "STATE", "NAME");
	while ((e = readdir(d))) {
		pid_t pid;

		if (!isdigit((unsigned char)e->d_name[0]))
			continue;

		pid = (pid_t)atoi(e->d_name);
		if (!read_field(pid, "Name:", name, sizeof(name)))
			continue;                      /* 그 사이에 종료됨 */
		read_field(pid, "State:", state, sizeof(state));
		read_field(pid, "PPid:", ppid, sizeof(ppid));

		printf("%-8d %-8s %-6.1s %s\n", (int)pid, ppid, state, name);
	}
	closedir(d);
}

int main(int argc, char *argv[])
{
	char path[64], line[LINE_LEN];
	FILE *fp;
	pid_t target;

	if (argc == 1) {
		list_all();
		return EXIT_SUCCESS;
	}

	target = (pid_t)atoi(argv[1]);
	snprintf(path, sizeof(path), "/proc/%d/status", (int)target);

	fp = fopen(path, "r");
	if (!fp) {
		perror(path);
		return EXIT_FAILURE;
	}

	printf("===== %s =====\n", path);
	while (fgets(line, sizeof(line), fp))
		if (wanted(line))
			fputs(line, stdout);
	fclose(fp);

	dump_extra(target);
	return EXIT_SUCCESS;
}

/*
 * [모범 답안]
 *  A1. R = TASK_RUNNING(실행 중이거나 런큐 대기), S = TASK_INTERRUPTIBLE
 *      (시그널로 깨울 수 있는 대기), D = TASK_UNINTERRUPTIBLE(주로 디스크 I/O
 *      대기, 시그널로 깨지 않음), Z = EXIT_ZOMBIE, T = TASK_STOPPED.
 *      주의: Z와 X는 __state가 아니라 task->exit_state 에 들어 있다.
 *  A2. 멀티스레드 프로세스의 스레드에서 다르다. tgid는 프로세스(스레드 그룹)의
 *      대표 ID = getpid()가 돌려주는 값, pid는 개별 스레드 ID = gettid().
 *      단일 스레드 프로세스에서는 두 값이 같다.
 *  A3. fs/proc/array.c 의 proc_pid_status()
 *      https://elixir.bootlin.com/linux/v7.2.2/A/ident/proc_pid_status
 */
