# 🤖 Task Manager

## 📄 소개

Task Manager는 CPU, RAM, Disk 사용량 등 시스템 자원을 모니터링하고, 특정 이벤트에 따라 자동으로 작업을 실행하는 터미널 기반 애플리케이션입니다. `ncurses` 라이브러리를 이용한 직관적인 인터페이스를 제공하며, `libnotify`를 통해 데스크탑 알림도 지원합니다.

## 🎯 주요 기능

- **파일 이벤트 모니터링**: 파일 생성, 삭제, 수정 등의 이벤트 감지
- **프로세스 이벤트 모니터링**: 프로세스 생성 및 종료 감지
- **시스템 자원 모니터링**: CPU, RAM, Disk 사용량 실시간 감시
- **타이머 이벤트**: 지정된 시간에 작업 실행
- **작업 관리**: 태스크 추가, 삭제, 활성화/비활성화
- **알림 및 로그**: 이벤트 발생 시 알림 및 로그 기록

## 📂 프로젝트 구조


## ⚒️ 빌드 및 실행

### 📋 필수 사항

- **GCC**: C 컴파일러
- **Make**: 빌드 도구
- **ncurses**: 터미널 UI 라이브러리
- **libnotify**: 데스크탑 알림 라이브러리

#### Ubuntu/Debian 설치 방법

```bash
sudo apt update
sudo apt install build-essential libncurses5-dev libncursesw5-dev libnotify-dev
```
🛠️ 빌드하기
	1.	소스 디렉토리로 이동:

cd src


	2.	프로젝트 빌드:

make

	•	모든 실행 파일이 build/ 디렉토리에 생성됩니다.

🚀 실행하기

빌드가 완료된 후 메인 애플리케이션을 실행합니다.

./build/UI

📋 사용법

Task Manager 인터페이스
	•	화살표 키: 태스크 목록 이동
	•	‘a’: 태스크 추가
	•	‘d’: 선택된 태스크 삭제
	•	스페이스바: 선택된 태스크 활성화/비활성화
	•	‘q’: 애플리케이션 종료

태스크 추가
	1.	‘a’ 키 누르기
	2.	태스크 이름 입력
	3.	태스크 타입 선택:
	•	file: 파일 이벤트 모니터링
	•	process: 프로세스 이벤트 모니터링
	•	time: 타이머 이벤트 설정
	•	system: 시스템 자원 모니터링
	4.	타입에 따른 세부 정보 입력
	5.	다음 프로세스 선택 및 설정

🧹 클린 빌드

빌드된 파일과 로그를 삭제하려면 다음 명령어를 실행합니다.

make clean

🛡️ Git 설정

빌드 아티팩트가 Git에 포함되지 않도록 .gitignore 파일을 설정했습니다.

# 객체 파일 무시
*.o

# 빌드 디렉토리 무시
build/

# 실행 파일 무시
src/UI
src/CHG_ATTR
src/cpuAlarm
src/diskAlarm
src/Executor
src/ExecutorTest
src/FileEventListener
src/logWriter
src/proc_killer
src/scan_proc
src/SignalHandler
src/SystemEventListener
src/Test
src/timer

# 로그 파일 무시
log.txt
*.log

# 임시 파일 무시
*~

.gitignore 적용 방법
	1.	.gitignore 파일 생성 또는 수정:

cd /path/to/your/project
touch .gitignore


	2.	위 내용을 .gitignore에 추가
	3.	이미 추적 중인 파일 무시하기:

git rm --cached src/UI src/CHG_ATTR src/cpuAlarm src/diskAlarm src/Executor src/ExecutorTest src/FileEventListener src/logWriter src/proc_killer src/scan_proc src/SignalHandler src/SystemEventListener src/Test src/timer log.txt


	4.	변경 사항 커밋:

git add .gitignore
git commit -m "빌드 아티팩트 및 실행 파일 무시 설정"



🔧 Makefile 개요

Makefile은 프로젝트의 빌드 과정을 자동화합니다.

주요 타겟
	•	all: 모든 실행 파일 빌드
	•	clean: 빌드 파일 및 로그 삭제

빌드 규칙 예시

# UI 빌드 (ncurses 필요)
build/UI: UI.c UI.h | build
	$(CC) $(CFLAGS) -o $@ UI.c -lncurses

# cpuAlarm 빌드 (libnotify 필요)
build/cpuAlarm: cpuAlarm.c | build
	$(CC) -o $@ cpuAlarm.c `pkg-config --cflags --libs libnotify`

# 기타 타겟들도 유사하게 설정

사용 예
	•	전체 빌드:

make


	•	특정 타겟 빌드:

make UI


	•	클린 빌드:

make clean
```

