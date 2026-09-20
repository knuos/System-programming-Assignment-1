# Week 3 실습 — Process Management

**26F 시스템프로그래밍특론 (Advanced System Programming)**
경북대학교 전자공학부 대학원 · 담당: 이경운 (kwlee87@knu.ac.kr)

---

## 1. 실습 개요

2주차에서 배운 커널 소스 탐색 능력을 이용해, **프로세스가 커널 안에서 어떻게 표현되고
생성되고 소멸하는지**를 코드로 직접 확인한다.

실습은 세 단계로 구성된다. 앞 단계를 마쳐야 다음 단계가 이해된다.

| 단계 | 위치 | 내용 | 배점 |
|---|---|---|---|
| **Level 1** | `level1-userspace/` | `fork()` / `exec()` / `wait()` 로 프로세스를 만들고 관찰 | 40 |
| **Level 2** | `level2-kernel-module/` | 커널 모듈에서 `task_struct`를 직접 읽고 순회 | 40 |
| **Level 3** | `level3-kthread/` | 커널 스레드 생성과 종료 | 20 |
| 심화 | 각 파일 하단 `[심화 과제]` | 선택 — 최대 +10 가산점 | +10 |

---

## 2. 실습 환경

가상 머신 위의 **Ubuntu 22.04 LTS 또는 24.04 LTS**를 권장한다.
커널 모듈을 적재하다 시스템이 멈출 수 있으므로 **호스트 OS에서 직접 실행하지 말 것.**

| 방법 | 비고 |
|---|---|
| VirtualBox / VMware + Ubuntu | 가장 안전. 스냅샷을 찍어 두면 복구가 쉽다 |
| WSL2 (Windows) | 편리하지만 커스텀 커널 헤더 설치가 필요할 수 있다 |
| 실습실 서버 | 공지 예정 |

### 준비 명령

```bash
sudo apt update
sudo apt install -y build-essential linux-headers-$(uname -r) \
                    gcc make gdb strace ltrace psmisc

# 커널 버전 확인 — 보고서 첫 줄에 이 값을 적을 것
uname -r
```

`linux-headers-$(uname -r)` 설치가 실패하면 실행 중인 커널과 저장소의 헤더 버전이
어긋난 것이다. `sudo apt install linux-generic` 후 재부팅하면 대개 해결된다.

---

## 3. 디렉터리 구조

```
lab02-process-management/
├── README.md                     ← 지금 읽고 있는 파일 (과제 명세)
├── level1-userspace/
│   ├── Makefile
│   ├── 01_fork_basic.c           fork()의 이중 반환
│   ├── 02_fork_exec_wait.c       fork + exec + wait 조합
│   ├── 03_zombie_orphan.c        좀비 · 고아 프로세스 관찰
│   └── 04_proc_reader.c          /proc로 커널이 보는 정보 읽기
├── level2-kernel-module/
│   ├── Makefile
│   ├── procinfo_current.c        current 매크로
│   ├── procinfo_list.c           for_each_process 순회
│   └── procinfo_tree.c           parent / children / sibling 트리
└── level3-kthread/
    ├── Makefile
    └── kthread_demo.c            kthread_run / kthread_stop
```

각 `.c` 파일에는 `TODO` 주석이 들어 있다. **TODO를 채우는 것이 과제다.**
파일 맨 아래의 `[확인 질문]`에 대한 답은 보고서에 작성한다.

---

## 4. 진행 순서

### Level 1 — 유저 공간 (40점)

```bash
cd level1-userspace
make                    # 네 개 모두 빌드
./01_fork_basic
./02_fork_exec_wait /bin/ls -l /
./03_zombie_orphan zombie     # 다른 터미널에서 ps로 STAT=Z 확인
./03_zombie_orphan orphan     # PPID가 1로 바뀌는지 확인
./04_proc_reader 1
```

관찰용 명령:

```bash
ps -eo pid,ppid,stat,comm | head -n 20
cat /proc/$$/status | head -n 12
pstree -p $$
strace -f -e trace=clone,execve,wait4 ./01_fork_basic
```

### Level 2 — 커널 모듈 (40점)

```bash
cd level2-kernel-module
make
sudo insmod procinfo_current.ko
sudo dmesg | tail -n 20
sudo rmmod procinfo_current

sudo insmod procinfo_list.ko limit=30
sudo dmesg | tail -n 40
sudo rmmod procinfo_list

sudo insmod procinfo_tree.ko target=1
sudo dmesg | tail -n 40
sudo rmmod procinfo_tree
```

> **주의**
> - 모듈을 고쳤으면 반드시 `rmmod` → `make` → `insmod` 순서로 다시 적재한다.
> - `rmmod`가 걸려 응답이 없으면 강제 재부팅하지 말고 먼저 `sudo dmesg | tail`을 확인한다.
> - `dmesg`에 아무것도 안 보이면 `sudo dmesg -n 8` 로 콘솔 로그 레벨을 올린다.

### Level 3 — 커널 스레드 (20점)

```bash
cd level3-kthread
make
sudo insmod kthread_demo.ko interval=2
ps -eo pid,ppid,stat,comm | grep knu_worker
sudo dmesg -w                 # Ctrl+C로 중단
sudo rmmod kthread_demo
```

---

## 5. 제출물

`<학번>_<이름>_week3.zip` 하나로 압축해 **LMS**에 제출한다.

```
<학번>_<이름>_week3/
├── level1-userspace/     *.c  (TODO를 채운 것) + Makefile
├── level2-kernel-module/ *.c  + Makefile
├── level3-kthread/       *.c  + Makefile
├── report.pdf            보고서
└── screenshots/          실행 결과 캡처 (dmesg, ps 출력 등)
```

`.o`, `.ko`, `.mod.c`, `.cmd` 등 빌드 산출물은 제출 전에 `make clean`으로 지운다.

### 보고서 (report.pdf) 구성 — 5~8쪽

1. **실습 환경** — 배포판, `uname -r` 출력, 가상화 도구
2. **Level별 실행 결과** — 캡처 이미지와 두세 줄의 설명
   (터미널 텍스트를 그대로 붙여 넣지 말고, 무엇을 보여 주는 화면인지 설명할 것)
3. **확인 질문 답변** — 각 소스 파일 하단의 `[확인 질문]` 전부
4. **커널 소스 근거** — 아래 표를 채울 것. 반드시 **Elixir v7.2.2 링크**를 함께 적는다.

   | 항목 | 파일 : 줄 | Elixir 링크 |
   |---|---|---|
   | `struct task_struct` 정의 | | |
   | `__state` 필드 선언 | | |
   | `for_each_process` 정의 | | |
   | `kernel_clone` 정의 | | |
   | `copy_process` 정의 | | |
   | `kthread_should_stop` 정의 | | |

5. **고찰** — 막혔던 부분과 해결 과정. *잘 된 것보다 실패한 과정이 더 큰 점수를 받는다.*

---

## 6. 평가 기준

| 항목 | 배점 | 설명 |
|---|---|---|
| 코드 동작 | 50 | TODO가 채워져 있고 빌드·실행이 되는가 |
| 관찰 결과의 정확성 | 20 | 캡처가 주장하는 내용과 일치하는가 |
| 확인 질문 답변 | 20 | 근거를 소스에서 가져왔는가 |
| 보고서 완성도 | 10 | 구성, 가독성 |
| 심화 과제 | +10 | 선택 |

**감점 사항**

- 빌드 실패 상태로 제출 (−10)
- 빌드 산출물(`.ko`, `.o`) 포함 (−5)
- 다른 사람의 코드를 그대로 제출 / 생성형 AI 출력물을 이해 없이 붙여 넣기 → **0점**
  (참고 자체는 허용한다. 다만 보고서에 출처를 명시하고, 코드의 모든 줄을 설명할 수 있어야 한다)

---

## 7. 자주 막히는 지점

| 증상 | 원인과 해결 |
|---|---|
| `make: *** /lib/modules/.../build: No such file` | 커널 헤더 미설치. `sudo apt install linux-headers-$(uname -r)` |
| `insmod: ERROR: could not insert module: Invalid module format` | 빌드에 쓴 헤더와 실행 커널 버전 불일치. `uname -r` 확인 후 재빌드 |
| `insmod`는 되는데 `dmesg`에 아무것도 없다 | `pr_fmt`을 `#include` **뒤에** 정의했거나, `pr_info` 대신 주석만 남아 있음 |
| `rmmod`가 멈춘다 | `kthread_stop()`이 스레드의 리턴을 기다리는 중. `worker_fn`이 잠들어 깨어나지 못하는지 확인 |
| `task->state` 컴파일 오류 | 최신 커널은 `task->__state`. 교재(2.6 기준)와 다르다 |
| `do_fork` 를 찾을 수 없다 | `kernel_clone`으로 이름이 바뀌었다 (`kernel/fork.c:2694`) |
| 모듈 적재 후 시스템이 멈춤 | VM 스냅샷으로 복구. 커널 코드에서 busy loop / NULL 역참조를 확인 |

---

## 8. 참고 자료

- Robert Love, *Linux Kernel Development*, 3rd ed. — **Chapter 3. Process Management**
- Elixir Cross Referencer — <https://elixir.bootlin.com/linux/v7.2.2/source>
  - `struct task_struct` → `include/linux/sched.h` : 826
  - `kernel_clone` → `kernel/fork.c` : 2694
  - `copy_process` → `kernel/fork.c` : 1994
  - `for_each_process` → `include/linux/sched/signal.h` : 640
- Linux Kernel Labs — <https://linux-kernel-labs.github.io/refs/heads/master/so2/lec1-intro.html>
- `man 2 fork`, `man 2 execve`, `man 2 wait`, `man 5 proc`

---

## 9. 제출 기한

다음 주 수업 시작 전까지 (구체적 일시는 LMS 공지 참조).
질문은 **LMS 게시판**에 남기면 다른 수강생에게도 도움이 된다.
