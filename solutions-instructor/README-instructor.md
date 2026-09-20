# 강사용 정답 및 진행 노트 — Week 3 Lab

> **이 폴더는 학생 배포본에 포함하지 마십시오.**
> 배포 시에는 `lab02-process-management/` 에서 `solutions-instructor/` 를 제외하고 압축합니다.

---

## 검증 상태

| 항목 | 상태 |
|---|---|
| Level 1 스켈레톤 (4개) | `gcc -Wall -Wextra` 경고 없이 빌드 확인 |
| Level 1 정답 (4개) | 빌드 + 실행 결과 확인 (fork/exec/wait, 좀비, 고아, /proc 순회) |
| Level 2 · 3 정답 | Linux v7.2.2 API 기준으로 작성. **실습 환경에서 한 번 빌드·적재 확인 필요** |

Level 2/3은 커널 헤더가 있는 실제 Ubuntu VM에서 다음으로 확인하십시오.

```bash
cd solutions-instructor/level2-kernel-module && make
sudo insmod procinfo_current.ko && sudo dmesg | tail -n 10 && sudo rmmod procinfo_current
sudo insmod procinfo_list.ko limit=15 filter=kworker && sudo dmesg | tail -n 20 && sudo rmmod procinfo_list
sudo insmod procinfo_tree.ko target=1 && sudo dmesg | tail -n 30 && sudo rmmod procinfo_tree

cd ../level3-kthread && make
sudo insmod kthread_demo.ko nr_workers=2 interval=3
ps -eo pid,ppid,stat,comm | grep knu_worker
sudo rmmod kthread_demo
```

---

## 커널 버전에 따라 손봐야 할 곳

| 심볼 | 주의 |
|---|---|
| `task->__state` | 5.14 이전 커널은 `task->state`. 교재(2.6)와도 다르다 |
| `kernel_clone()` | 5.10 이전은 `_do_fork()`, 그 이전은 `do_fork()` |
| `TASK_STOPPED` / `TASK_TRACED` | 버전에 따라 합성 비트라 `switch`로 비교하면 어긋난다 → 비트 검사 사용 |
| `pid_task()` | 반드시 `rcu_read_lock()` 안에서. `find_get_pid()`가 준 참조는 `put_pid()`로 반납 |
| `real_parent` | `__rcu` 주석이 붙어 있어 `rcu_dereference()`로 읽는 것이 정석 (sparse 경고 회피) |

---

## 90분 실습 진행 타임라인 (권장)

| 시간 | 내용 |
|---|---|
| 0~10분 | 슬라이드 Part 0 — task_struct 복습, `current` / `for_each_process` |
| 10~20분 | 환경 점검 — `uname -r`, 헤더 설치 확인, 스켈레톤 배포 |
| 20~45분 | Level 1 — 다 같이 `01`을 채우고, `03 zombie`는 함께 `ps`로 관찰 |
| 45~50분 | 휴식 |
| 50~75분 | Level 2 — `procinfo_current` 를 함께 완성 → `list`/`tree` 는 개별 진행 |
| 75~85분 | Level 3 — `kthread_demo` 시연, `rmmod` 멈춤 사례 설명 |
| 85~90분 | 제출물·평가 기준 안내, 질의응답 |

**실습 중 강조할 지점**

1. `01_fork_basic` 을 `| cat` 으로 파이프하면 `[before fork]` 가 두 번 출력된다
   → stdio 버퍼가 fork로 복제된다는 것을 보여 주는 가장 좋은 예. `fflush()`의 이유.
2. `insmod` 했을 때 `current->comm` 이 `insmod` 로 나오는 것 — "커널 코드는 누군가의
   문맥을 빌려 실행된다"는 개념을 여기서 못 박아 두면 5주차 시스템 콜이 쉬워진다.
3. `procinfo_list` 출력을 `ps -eo pid,ppid,stat,comm` 과 나란히 띄워 비교.
   ps가 `/proc`을 읽어 만드는 것과 같은 정보를 커널에서 직접 읽었다는 점.
4. 커널 스레드의 `mm == NULL` — 대괄호 표기(`[kworker/0:1]`)의 정체.

---

## 자주 나오는 학생 질문과 답

**Q. 왜 `printf` 대신 `pr_info` 인가요?**
커널에는 libc가 없다. `pr_info`는 커널 링 버퍼에 쓰고 로그 레벨을 갖는다.

**Q. `dmesg`에 아무것도 안 나와요.**
십중팔구 `pr_fmt`을 `#include` 뒤에 정의했거나 `pr_info` 호출을 주석 안에 남겨 뒀다.
`sudo dmesg -n 8`로 콘솔 로그 레벨을 올리게 하고 `dmesg | tail`로 확인시킨다.

**Q. `rmmod`가 안 끝나요.**
`kthread_stop()`이 워커의 리턴을 기다리는 중이다. `worker_fn`이 `ssleep()`로
`TASK_UNINTERRUPTIBLE`에 잠들면 최대 interval 만큼 지연된다.
`msleep_interruptible()`을 쓰면 즉시 깬다 — 두 방식의 차이를 비교해 보게 하면 좋다.

**Q. 모듈을 올렸더니 VM이 멈췄어요.**
스냅샷 복구. 대부분 `while(1)` busy loop이나 NULL 역참조다. `dmesg`에 남은
call trace를 함께 읽어 주면 좋은 학습 기회가 된다.

---

## 채점 시 눈여겨볼 것

- Level 2에서 `rcu_read_lock()` 을 넣었는가 (안 넣어도 대개 동작하므로 그냥 지나치기 쉽다)
- `task->real_parent` vs `task->parent` 를 구분해 답했는가
- 근거표의 줄 번호를 **v7.2.2 기준으로 직접 확인**했는가 (교재 값을 그대로 옮겨 적은 경우 감점)
- 고찰에 실패 과정이 구체적으로 적혀 있는가

---

## 다음 주차와의 연결

- 4주차 *Process 관찰 실습* — 이번에 만든 `procinfo_list`를 확장해
  `/proc/knu_procinfo` 항목을 만드는 과제로 이어갈 수 있다 (`proc_create`).
- 5주차 *Process Scheduling* — 이번에 출력해 본 `task->prio`, `__state`, `on_rq` 가
  스케줄러 논의의 출발점이 된다.
