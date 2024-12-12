# Makefile 예제

# 공통 옵션
CFLAGS = -Wall -std=c99 -D_XOPEN_SOURCE=700
LDFLAGS =
CC = gcc

# ncurses가 필요한 UI
UI_OBJS = UI.c
UI_TARGET = UI

# libnotify가 필요한 cpuAlarm, diskAlarm
CPU_ALARM = cpuAlarm
DISK_ALARM = diskAlarm
RAM_ALARM = ramAlarm

# 나머지 타겟들
TARGETS = CHG_ATTR Executor FileEventListener logWriter \
          proc_killer scan_proc SignalHandler SystemEventListener Test timer

# 모든 실행파일
ALL = $(UI_TARGET) $(TARGETS) $(CPU_ALARM) $(DISK_ALARM) $(RAM_ALARM)

.PHONY: all clean

all: $(ALL)

# UI 빌드 (ncurses 필요)
$(UI_TARGET): UI.c UI.h
	$(CC) $(CFLAGS) -o $@ UI.c -lncurses

# cpuAlarm, diskAlarm 빌드 (libnotify 필요)
$(CPU_ALARM): cpuAlarm.c
	$(CC) -o $@ cpuAlarm.c `pkg-config --cflags --libs libnotify`

$(DISK_ALARM): diskAlarm.c
	$(CC) -o $@ diskAlarm.c `pkg-config --cflags --libs libnotify`

$(RAM_ALARM): ramAlarm.c
	$(CC) -o $@ ramAlarm.c `pkg-config --cflags --libs libnotify`

# 나머지 타겟 빌드
CHG_ATTR: CHG_ATTR.c
	$(CC) $(CFLAGS) -o $@ CHG_ATTR.c

Executor: Executor.c
	$(CC) $(CFLAGS) -o $@ Executor.c

FileEventListener: FileEventListener.c
	$(CC) $(CFLAGS) -o $@ FileEventListener.c

logWriter: logWriter.c
	$(CC) $(CFLAGS) -o $@ logWriter.c

proc_killer: proc_killer.c
	$(CC) $(CFLAGS) -o $@ proc_killer.c

scan_proc: scan_proc.c
	$(CC) $(CFLAGS) -o $@ scan_proc.c

SignalHandler: SignalHandler.c
	$(CC) $(CFLAGS) -o $@ SignalHandler.c

SystemEventListener: SystemEventListener.c
	$(CC) $(CFLAGS) -o $@ SystemEventListener.c

Test: Test.c
	$(CC) $(CFLAGS) -o $@ Test.c

timer: timer.c
	$(CC) $(CFLAGS) -o $@ timer.c

clean:
	rm -f $(ALL) log.txt