# TOS Mini Shell

TOS는 운영체제 수업용 미니 쉘 프로젝트입니다. 현재 단계의 목표는 완성된 커널 시뮬레이터가 아니라, 이후 운영체제 코어 시뮬레이터를 붙이기 위한 앞단 인터페이스를 만드는 것입니다.

지금 구현된 기능은 사용자 입력을 받아 파싱하고, 내장 명령어는 쉘 내부에서 처리하며, 일반 외부 명령어는 `fork()` + `execvp()` + `waitpid()` 흐름으로 실행하는 기본 쉘입니다. 추가로 시뮬레이션용 PCB와 Ready Queue를 만들기 위한 `create`, `ps` 명령이 포함되어 있습니다.

## 현재 폴더 구조

```text
OS/
├── main.c
├── Makefile
├── README.md
├── common/
│   ├── queue.c
│   ├── queue.h
│   ├── util.c
│   └── util.h
├── user/
│   └── shell/
│       ├── builtins.c
│       ├── builtins.h
│       ├── executor.c
│       ├── executor.h
│       ├── parser.c
│       ├── parser.h
│       ├── shell.c
│       └── shell.h
├── core/
│   ├── kernel/
│   │   ├── kernel.c
│   │   └── kernel.h
│   ├── process/
│   │   ├── process.c
│   │   └── process.h
│   ├── scheduler/
│   │   ├── scheduler.c
│   │   └── scheduler.h
│   ├── memory/
│   ├── filesystem/
│   └── io/
├── runtime/
├── tests/
└── docs/
    └── ARCHITECTURE.md
```

## 빌드

```bash
cd <project-root>
make
```

빌드 결과 실행 파일 이름은 `TOS`입니다.

```text
TOS
```

정리하려면 다음을 실행합니다.

```bash
make clean
```

## 실행

```bash
./TOS
```

프롬프트는 현재 다음처럼 표시됩니다.

```text
mini-shell>
```

## 현재 구현된 명령

- `cd <dir>`
  - 쉘 프로세스의 현재 작업 디렉터리를 변경합니다.
- `pwd`
  - 현재 작업 디렉터리를 출력합니다.
- `help`
  - 사용 가능한 내장 명령어를 출력합니다.
- `exit`
  - 쉘을 종료합니다.
- `create <name> <total_time>`
  - I/O가 없는 시뮬레이션용 프로세스 PCB를 생성합니다.
  - 생성된 PCB는 `READY` 상태가 되고 Ready Queue에 들어갑니다.
- `create <name> <total_time> io <io_at> <io_wait>`
  - 한 번의 I/O 이벤트를 가진 시뮬레이션용 프로세스 PCB를 생성합니다.
  - `io_at`은 `total_time`보다 작아야 합니다.
- `ps`
  - 현재 Running 프로세스, Ready Queue, Waiting Queue, Terminated 프로세스 목록을 먼저 출력합니다.
  - 그 다음 PCB 목록을 표 형태로 출력합니다.

내장 명령어가 아닌 입력은 외부 명령어로 간주하고 `fork()` + `execvp()` + `waitpid()`로 실행합니다.

## 실행 예시

```text
mini-shell> create P1 10
Created P1 (logical PID=1, state=READY, total=10)
  I/O: disabled
mini-shell> create P4 10 io 2 4
Created P4 (logical PID=2, state=READY, total=10)
  I/O: enabled at time 2, wait 4
mini-shell> ps
Running: (none)
Ready Queue: P1 P4
Waiting Queue: (empty)
Terminated: (empty)

PID  Name     State       Arrival Total   Remain  Wait    IO  IO@   IOW
1    P1       READY       0       10      10      0       N   -1    -1
2    P4       READY       0       10      10      0       Y   2     4
mini-shell> pwd
/home/user/OS
mini-shell> exit
```

## 현재 구현 상태

- 쉘 입력 루프는 `user/shell/shell.c`에 있습니다.
- 입력 파싱은 `user/shell/parser.c`에서 공백 기준으로 처리합니다.
- 내장 명령어 dispatch는 `user/shell/builtins.c`에서 처리합니다.
- 외부 명령어 실행은 `user/shell/executor.c`에서 처리합니다.
- PCB와 프로세스 테이블은 `core/process/`에 있습니다.
- Ready Queue와 Waiting Queue를 포함한 스케줄러 기본 구조는 `core/scheduler/`에 있습니다.
- 공용 정수 큐와 유틸리티 함수는 `common/`에 있습니다.


현재 `core/scheduler`에는 기본 알고리즘 값과 큐 구조가 있지만, 아직 실제 FCFS나 RR 실행 로직이 구현된 상태는 아닙니다. `ps`에서도 스케줄링 알고리즘은 사용자가 나중에 `set_sched`로 설정했을 때만 표시되도록 설계되어 있습니다.

## 확장 방향

다음 단계에서는 다음 순서로 확장하는 것이 자연스럽습니다.

1. `set_sched` 명령 추가
2. `run` 명령 추가
3. FCFS 실행 루프 구현
4. I/O 대기와 Waiting Queue 처리
5. RR과 time quantum 처리
6. `core/memory`에 파일 기반 메모리 시뮬레이션 추가
