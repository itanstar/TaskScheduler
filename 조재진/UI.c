#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define FILE_EVENT_LISTENER "./FileEventListener"
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
	
	char variableType[20];
	char target[20];
	char event[20];

	char next_variableType[20];
	char next_process[20];
	char next_target[20];
	char parameter[20];

    int active;
    pid_t pid; // 태스크 실행 시 fork된 프로세스의 PID
} Task;

Task tasks[MAX_TASKS];
int task_count = 0;

void init_tasks()
{
    for (int i = 0; i < MAX_TASKS; i++)
    {
        tasks[i].active = 0;
        tasks[i].pid = 0;
        strcpy(tasks[i].name, "");
        strcpy(tasks[i].type, "");
    }
}

//시그널 이벤트 처리 부분
void sigusr1_handler(int signum, siginfo_t *info, void *context){
	for(int i = 0; i < task_count; i++){
		if(tasks[i].pid == info->si_pid){
			printf("Event Captuered\n");
			pid_t id = fork();
			if(id == -1){
				perror("fork");
				exit(-1);
			}
			if(id == 0){
				//다음 이벤트가 로그 작성일 때 -> 감지 파일과 감지 이벤트 전달
				if(strcmp(tasks[i].next_process, "LogWrite") == 0){
					execl(LOG_WRITER, LOG_WRITER, 
							tasks[i].target, tasks[i].event, NULL);
					perror("execl failed");
					exit(EXIT_FAILURE);
				}
				//다음 이벤트가 실행일 때 -> 실행할 파일과 매개변수 전달
				if(strcmp(tasks[i].next_process, "Execute") == 0){
					execl(EXECUTOR, EXECUTOR, 
							tasks[i].next_target, tasks[i].parameter, NULL);
					perror("execl failed");
					exit(EXIT_FAILURE);
				}
				//다음 이벤트가 권한 변경일 때 -> 대상 파일과 매개변수(rwxrwxrwx) 전달
				//대상 파일이 "." 일 경우 현재 폴더의 모든 파일의 권한 변경
				if(strcmp(tasks[i].next_process, "Change_Attr") == 0){
					execl(CHG_ATTR, CHG_ATTR, 
							tasks[i].next_target, tasks[i].parameter, NULL);
					perror("execl failed");
					exit(EXIT_FAILURE);
				}
				if(strcmp(tasks[i].next_process, "Terminate") == 0){
					execl(PROC_KILLER, PROC_KILLER, 
							tasks[i].next_variableType, tasks[i].next_target, NULL);
					perror("execl failed");
					exit(EXIT_FAILURE);
				}
			}
			return;
		}
	}
}

// 상단 제목 출력
void display_title()
{
    int row, col;
    getmaxyx(stdscr, row, col); // 터미널 크기 가져오기

    const char *title = "Task Manager";
    mvprintw(0, (col - strlen(title)) / 2, "%s", title); // 중앙에 제목 출력
    mvhline(1, 0, '-', col);                             // 제목 아래에 구분선 출력
    refresh();
}

// 입력 창 표시
void display_input_window(const char *prompt, char *output)
{
    int row, col;
    getmaxyx(stdscr, row, col); // 터미널 크기 가져오기

    // 입력 창 생성
    WINDOW *input_win = newwin(3, col - 4, row - 4, 2); // 하단에 위치
    box(input_win, 0, 0);

    mvwprintw(input_win, 1, 1, "%s", prompt); // 프롬프트 메시지 표시
    wrefresh(input_win);

    echo();
    wgetstr(input_win, output); // 입력 받기
    noecho();

    delwin(input_win); // 입력 창 삭제
    refresh();         // 메인 화면 새로고침
}

// 태스크 목록 표시
void list_tasks(WINDOW *task_win, int highlight)
{
    werase(task_win);
    box(task_win, 0, 0);

    mvwprintw(task_win, 1, 1, "%-20s %-10s %-5s %-5s", "Name", "Type", "Active", "PID");
    for (int i = 0; i < task_count; i++)
    {
        if (i == highlight)
        {
            wattron(task_win, A_REVERSE); // 강조 표시
        }
        mvwprintw(task_win, i + 2, 1, "%-20s %-10s %-5s %-5d", tasks[i].name, tasks[i].type,
                  tasks[i].active ? "ON" : "OFF", tasks[i].pid);
        if (i == highlight)
        {
            wattroff(task_win, A_REVERSE); // 강조 해제
        }
    }

    wrefresh(task_win);
}

// 태스크 형식 선택 메뉴
void select_task_type(WINDOW *task_win, char *selected_type)
{
    const char *types[] = {"file", "process", "time", "status"};
    int highlight = 0;
    int choice = -1;
    int c;

    while (choice == -1)
    {
        werase(task_win);
        box(task_win, 0, 0);
        mvwprintw(task_win, 1, 1, "Select Task Type:");

        for (int i = 0; i < 4; i++)
        {
            if (i == highlight)
            {
                wattron(task_win, A_REVERSE);
            }
            mvwprintw(task_win, i + 2, 1, "%s", types[i]);
            if (i == highlight)
            {
                wattroff(task_win, A_REVERSE);
            }
        }

        wrefresh(task_win);
        c = wgetch(task_win);

        switch (c)
        {
        case KEY_UP:
            if (highlight > 0)
            {
                highlight--;
            }
            break;
        case KEY_DOWN:
            if (highlight < 3)
            {
                highlight++;
            }
            break;
        case 10: // Enter key
            strcpy(selected_type, types[highlight]);
            choice = highlight;
            break;
        }
    }
}

void get_file_event(WINDOW *task_win){
	char filePath[20];
	char event[20];

	display_input_window("Enter filePath: ", filePath);
    display_input_window("Enter event(IN_OOOOO): ", event);

    strcpy(tasks[task_count].target, filePath);
    strcpy(tasks[task_count].event, event);
}

void get_process_event(WINDOW *task_win){
	char variableType[20];
	char target[20];
	char event[20];

	display_input_window("Enter variableType(PID / NAME): ", variableType);
	if(strcmp(variableType, "PID") == 0)
		display_input_window("Enter PID: ", target);
	else
		display_input_window("Enter NAME: ", target);
	
    display_input_window("Enter event(terminate / create): ", event);

	strcpy(tasks[task_count].variableType, variableType);
    strcpy(tasks[task_count].target, target);
    strcpy(tasks[task_count].event, event);
}

void get_timer_event(WINDOW *task_win){
	char variableType[20];
	char time[20];

	display_input_window("Enter variableType(TIMER / ALARM): ", variableType);
	if(strcmp(variableType, "TIMER") == 0)
		display_input_window("Enter time(s): ", time);
	else
    	display_input_window("Enter time(HH:MM): ", time);

	strcpy(tasks[task_count].variableType, variableType);
    strcpy(tasks[task_count].target, time);
}

void get_next_process(WINDOW *task_win){
	char next_variableType[20];
	char next_process[20];
	char next_target[20];
	char parameter[20];
	
	display_input_window("Enter next process: ", next_process);
	strcpy(tasks[task_count].next_process, next_process);

	if((strcmp(next_process, "Execute") == 0) ||
			(strcmp(next_process, "Change_Attr") == 0)){
		//대상 파일과 파라미터 입력 받음
		display_input_window("Enter next filePath: ", next_target);
		display_input_window("Enter parameter(s): ", parameter);

		strcpy(tasks[task_count].next_target, next_target);
		strcpy(tasks[task_count].parameter, parameter);
	}
	else if(strcmp(next_process, "Terminate") == 0){
		display_input_window("Enter variableType(PID / NAME): ", next_variableType);
		if(strcmp(next_variableType, "PID") == 0)
			display_input_window("Enter PID: ", next_target);
		else
			display_input_window("Enter NAME: ", next_target);

		strcpy(tasks[task_count].next_variableType, next_variableType);
		strcpy(tasks[task_count].next_target, next_target);
	}
}

void run_event_listener(int task_num){
	tasks[task_num].pid = fork();
	if(tasks[task_num].pid == -1){
		perror("fork");
		exit(-1);
	}

	if(tasks[task_num].pid == 0){
		//이벤트 리스너(감지 파일, 감지 이벤트) 실행
		if(strcmp(tasks[task_num].type, "file") == 0){
			execl(FILE_EVENT_LISTENER, FILE_EVENT_LISTENER, 
				tasks[task_num].target, tasks[task_num].event, NULL);
			perror("execl failed");
			exit(EXIT_FAILURE);
		}
		else if(strcmp(tasks[task_num].type, "process") == 0){
			execl(SCAN_PROC, SCAN_PROC, 
			tasks[task_num].variableType, tasks[task_num].target, tasks[task_num].event, NULL);
			perror("execl failed");
			exit(EXIT_FAILURE);
		}
		else if(strcmp(tasks[task_num].type, "time") == 0){
			execl(TIMER, TIMER, 
				tasks[task_num].variableType, tasks[task_num].target, NULL);
			perror("execl failed");
			exit(EXIT_FAILURE);
		}
	}
}

// 태스크 생성
void create_task(WINDOW *task_win)
{
    if (task_count >= MAX_TASKS)
    {
        mvwprintw(task_win, 1, 1, "Task limit reached!");
        wrefresh(task_win);
        sleep(1);
        return;
    }

    char name[50];
    char type[20];

    display_input_window("Enter task name: ", name);
    select_task_type(task_win, type);

	strcpy(tasks[task_count].name, name);
	strcpy(tasks[task_count].type, type);

	if(strcmp(type, "file") == 0){
		get_file_event(task_win);
	}
	else if(strcmp(type, "process") == 0){
		get_process_event(task_win);
	}
	else if(strcmp(type, "time") == 0){
		get_timer_event(task_win);
	}

	
	get_next_process(task_win);
	

    tasks[task_count].active = 0;
    tasks[task_count].pid = 0;

	run_event_listener(task_count);

    task_count++;
}

// 태스크 삭제
void delete_task(WINDOW *task_win, int index)
{
    if (index < 0 || index >= task_count)
    {
        return;
    }

    if (tasks[index].active && tasks[index].pid > 0)
    {
        kill(tasks[index].pid, SIGKILL);
        waitpid(tasks[index].pid, NULL, 0);
    }

    for (int i = index; i < task_count - 1; i++)
    {
        tasks[i] = tasks[i + 1];
    }

    task_count--;
}

// 태스크 ON/OFF 토글
void toggle_task(int index)
{
    if (index < 0 || index >= task_count)
    {
        return;
    }

    if (tasks[index].active)
    {
        // 이미 ON 상태 -> OFF로 전환
        if (tasks[index].pid > 0)
        {
            kill(tasks[index].pid, SIGKILL);
            waitpid(tasks[index].pid, NULL, 0);
        }
        tasks[index].active = 0;
        tasks[index].pid = 0;
    }
    else
    {
        // OFF 상태 -> ON으로 전환
        pid_t pid = fork();
        if (pid == 0)
        {
            // 자식 프로세스에서 작업 실행
            printf("Task %s of type %s is running.\n", tasks[index].name, tasks[index].type);
            sleep(5);
            exit(0);
        }
        else if (pid > 0)
        {
            // 부모 프로세스: 자식 PID 저장
            tasks[index].active = 1;
            tasks[index].pid = pid;
        }
    }
}

int main()
{
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sigusr1_handler;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    initscr();
    clear();
    noecho();
    cbreak();

    display_title(); // 프로그램 실행 시 상단 제목 출력

    int startx = 0, starty = 2; // 작업 창 시작 위치 (제목 아래로 이동)
    int width = 50, height = 15;
    int highlight = 0; // 현재 강조된 태스크 인덱스
    int c;

    init_tasks();

    WINDOW *task_win = newwin(height, width, starty, startx);
    keypad(task_win, TRUE);

    while (1)
    {
        list_tasks(task_win, highlight);

        mvwprintw(task_win, height - 2, 1, "Arrow keys: Move | 'a': Add | 'd': Delete | ' ': Toggle | 'q': Quit");
        wrefresh(task_win);

        c = wgetch(task_win);
        switch (c)
        {
        case KEY_UP:
            if (highlight > 0)
            {
                highlight--;
            }
            break;
        case KEY_DOWN:
            if (highlight < task_count - 1)
            {
                highlight++;
            }
            break;
        case ' ': // 스페이스바로 토글
            toggle_task(highlight);
            break;
        case 'a': // 태스크 추가
            create_task(task_win);
            break;
        case 'd': // 태스크 삭제
            delete_task(task_win, highlight);
            if (highlight >= task_count && highlight > 0)
            {
                highlight--;
            }
            break;
        case 'q': // 종료
            endwin();
            return 0;
        }
    }

    endwin();
    return 0;
}