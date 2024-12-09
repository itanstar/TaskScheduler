#ifndef UI_H
#define UI_H

// UI 사용자 정의 헤더파일로 따로 분리
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define FILE_EVENT_LISTENER "./FileEventListener"
#define SYSTEM_EVENT_LISTENER "./SystemEventListener"

#define SCAN_PROC "./scan_proc"
#define LOG_WRITER "./logWriter"
#define EXECUTOR "./Executor"
#define CHG_ATTR "./CHG_ATTR"
#define TIMER "./timer"
#define PROC_KILLER "./proc_killer"

#define MAX_TASKS 10

typedef struct Task
{
    char name[50];
    char type[20]; // file, process, time, status

    char variableType[20]; // PID, NAME, TIMER, ALARM
    char target[20];       // 파일 경로, PID 값, 프로세스명, 타이머 시간 등
    char event[20];        // file: IN_XXXX, process: create/terminate

    char next_variableType[20];
    char next_process[20]; // LogWrite, Execute, Change_Attr, Terminate
    char next_target[20];
    char parameter[20];

    int active;
    pid_t pid; // 태스크 실행 시 fork된 프로세스의 PID
} Task;

void init_tasks();
void display_title();
void display_input_window(const char *prompt, char *output);
void list_tasks(WINDOW *task_win, int highlight);
void select_task_type(WINDOW *task_win, char *selected_type);

void create_task(WINDOW *task_win);
void delete_task(WINDOW *task_win, int index);
void toggle_task(int index);
void run_event_listener(int task_num);

void select_system_resource(WINDOW *task_win, char *resource);
void select_system_operation(WINDOW *task_win, char *operation);
void get_system_event(WINDOW *task_win);

void get_file_event(WINDOW *task_win);
void get_process_event(WINDOW *task_win);
void get_timer_event(WINDOW *task_win);
void get_next_process(WINDOW *task_win);

void setup_signal_handler();

extern Task tasks[MAX_TASKS];
extern int task_count;

#endif