#include "UI.h"

Task tasks[MAX_TASKS];
int task_count = 0;

// SIGUSR1 시그널 핸들러 함수: 이벤트 감지 시 다음 프로세스를 실행
void sigusr1_handler(int signum, siginfo_t *info, void *context)
{
    // 어떤 이벤트 리스너 프로세스에서 시그널을 보냈는지 확인
    for (int i = 0; i < task_count; i++)
    {
        if (tasks[i].pid == info->si_pid)
        {
            // 이벤트를 발생시킨 태스크를 찾음
            pid_t id = fork();
            if (id == -1)
            {
                perror("fork");
                return;
            }
            if (id == 0)
            {
                // 자식 프로세스에서 다음 작업 실행
                if (strcmp(tasks[i].next_process, "LogWrite") == 0)
                {
                    // 파일 이벤트 기록
                    execl(LOG_WRITER, LOG_WRITER, tasks[i].target, tasks[i].event, NULL);
                }
                else if (strcmp(tasks[i].next_process, "Execute") == 0)
                {
                    // 지정된 프로그램 실행
                    execl(EXECUTOR, EXECUTOR, tasks[i].next_target, tasks[i].parameter, NULL);
                }
                else if (strcmp(tasks[i].next_process, "Change_Attr") == 0)
                {
                    // 파일 권한 변경
                    execl(CHG_ATTR, CHG_ATTR, tasks[i].next_target, tasks[i].parameter, NULL);
                }
                else if (strcmp(tasks[i].next_process, "Terminate") == 0)
                {
                    // 프로세스 종료
                    execl(PROC_KILLER, PROC_KILLER, tasks[i].next_variableType, tasks[i].next_target, NULL);
                }
                perror("execl failed");
                exit(EXIT_FAILURE);
            }
            return;
        }
    }
}

// SIGUSR1 시그널 핸들러 등록 함수
void setup_signal_handler()
{
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = sigusr1_handler;
    if (sigaction(SIGUSR1, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

// 태스크 배열 초기화
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

// 상단 제목 표시
void display_title()
{
    int row, col;
    getmaxyx(stdscr, row, col);
    const char *title = "Task Manager";
    mvprintw(0, (col - (int)strlen(title)) / 2, "%s", title);
    mvhline(1, 0, '-', col);
    refresh();
}

// 입력 창 표시 및 문자열 입력 받기
void display_input_window(const char *prompt, char *output)
{
    int row, col;
    getmaxyx(stdscr, row, col);

    // 하단에 작은 윈도우를 만들어 입력 받음
    WINDOW *input_win = newwin(3, col - 4, row - 4, 2);
    box(input_win, 0, 0);
    mvwprintw(input_win, 1, 1, "%s", prompt);
    wrefresh(input_win);

    echo();
    wgetnstr(input_win, output, 50);
    noecho();

    delwin(input_win);
    refresh();
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
            wattron(task_win, A_REVERSE);
        }
        mvwprintw(task_win, i + 2, 1, "%-20s %-10s %-5s %-5d", tasks[i].name, tasks[i].type,
                  tasks[i].active ? "ON" : "OFF", tasks[i].pid);
        if (i == highlight)
        {
            wattroff(task_win, A_REVERSE);
        }
    }

    wrefresh(task_win);
}

// 태스크 타입 선택 메뉴 (file, process, time, status)
void select_task_type(WINDOW *task_win, char *selected_type)
{
    const char *types[] = {"file", "process", "time", "system"};
    int highlight = 0;
    int choice = -1;
    int c;

    // 화살표 키로 이동, 엔터로 선택
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
                highlight--;
            break;
        case KEY_DOWN:
            if (highlight < 3)
                highlight++;
            break;
        case 10: // 엔터키
            strcpy(selected_type, types[highlight]);
            choice = highlight;
            break;
        }
    }
}

// 파일 이벤트 선택 메뉴
void select_file_event(WINDOW *task_win, char *event)
{
    const char *events[] = {"IN_CREATE", "IN_DELETE", "IN_ATTRIB", "IN_MODIFY", "IN_MOVED_TO",
                            "IN_MOVED_FROM", "IN_OPEN", "IN_CLOSE"};
    int highlight = 0;
    int choice = -1;
    int c;

    while (choice == -1)
    {
        werase(task_win);
        box(task_win, 0, 0);
        mvwprintw(task_win, 1, 1, "Select File Event:");

        for (int i = 0; i < 8; i++)
        {
            if (i == highlight)
                wattron(task_win, A_REVERSE);
            mvwprintw(task_win, i + 2, 1, "%s", events[i]);
            if (i == highlight)
                wattroff(task_win, A_REVERSE);
        }

        wrefresh(task_win);
        c = wgetch(task_win);
        switch (c)
        {
        case KEY_UP:
            if (highlight > 0)
                highlight--;
            break;
        case KEY_DOWN:
            if (highlight < 7)
                highlight++;
            break;
        case 10: // 엔터
            strcpy(event, events[highlight]);
            choice = highlight;
            break;
        }
    }
}

// 프로세스 이벤트 선택 메뉴 (create/terminate)
void select_process_event(WINDOW *task_win, char *event)
{
    const char *events[] = {"create", "terminate"};
    int highlight = 0;
    int choice = -1;
    int c;

    while (choice == -1)
    {
        werase(task_win);
        box(task_win, 0, 0);
        mvwprintw(task_win, 1, 1, "Select Process Event:");

        for (int i = 0; i < 2; i++)
        {
            if (i == highlight)
                wattron(task_win, A_REVERSE);
            mvwprintw(task_win, i + 2, 1, "%s", events[i]);
            if (i == highlight)
                wattroff(task_win, A_REVERSE);
        }

        wrefresh(task_win);
        c = wgetch(task_win);
        switch (c)
        {
        case KEY_UP:
            if (highlight > 0)
                highlight--;
            break;
        case KEY_DOWN:
            if (highlight < 1)
                highlight++;
            break;
        case 10:
            strcpy(event, events[highlight]);
            choice = highlight;
            break;
        }
    }
}

// 변수 타입 선택 메뉴 (PID/NAME)
void select_variable_type(WINDOW *task_win, char *variableType)
{
    const char *types[] = {"PID", "NAME"};
    int highlight = 0;
    int choice = -1;
    int c;

    while (choice == -1)
    {
        werase(task_win);
        box(task_win, 0, 0);
        mvwprintw(task_win, 1, 1, "Select Variable Type:");

        for (int i = 0; i < 2; i++)
        {
            if (i == highlight)
                wattron(task_win, A_REVERSE);
            mvwprintw(task_win, i + 2, 1, "%s", types[i]);
            if (i == highlight)
                wattroff(task_win, A_REVERSE);
        }

        wrefresh(task_win);
        c = wgetch(task_win);
        switch (c)
        {
        case KEY_UP:
            if (highlight > 0)
                highlight--;
            break;
        case KEY_DOWN:
            if (highlight < 1)
                highlight++;
            break;
        case 10:
            strcpy(variableType, types[highlight]);
            choice = highlight;
            break;
        }
    }
}

// 타이머 타입 선택 (TIMER/ALARM)
void select_timer_type(WINDOW *task_win, char *variableType)
{
    const char *types[] = {"TIMER", "ALARM"};
    int highlight = 0;
    int choice = -1;
    int c;

    while (choice == -1)
    {
        werase(task_win);
        box(task_win, 0, 0);
        mvwprintw(task_win, 1, 1, "Select Timer Type:");

        for (int i = 0; i < 2; i++)
        {
            if (i == highlight)
                wattron(task_win, A_REVERSE);
            mvwprintw(task_win, i + 2, 1, "%s", types[i]);
            if (i == highlight)
                wattroff(task_win, A_REVERSE);
        }

        wrefresh(task_win);
        c = wgetch(task_win);
        switch (c)
        {
        case KEY_UP:
            if (highlight > 0)
                highlight--;
            break;
        case KEY_DOWN:
            if (highlight < 1)
                highlight++;
            break;
        case 10:
            strcpy(variableType, types[highlight]);
            choice = highlight;
            break;
        }
    }
}

// 다음 프로세스(LogWrite, Execute, Change_Attr, Terminate) 선택 메뉴
void select_next_process(WINDOW *task_win, char *next_process)
{
    const char *types[] = {"LogWrite", "Execute", "Change_Attr", "Terminate"};
    int highlight = 0;
    int choice = -1;
    int c;

    while (choice == -1)
    {
        werase(task_win);
        box(task_win, 0, 0);
        mvwprintw(task_win, 1, 1, "Select Next Process:");

        for (int i = 0; i < 4; i++)
        {
            if (i == highlight)
                wattron(task_win, A_REVERSE);
            mvwprintw(task_win, i + 2, 1, "%s", types[i]);
            if (i == highlight)
                wattroff(task_win, A_REVERSE);
        }

        wrefresh(task_win);
        c = wgetch(task_win);
        switch (c)
        {
        case KEY_UP:
            if (highlight > 0)
                highlight--;
            break;
        case KEY_DOWN:
            if (highlight < 3)
                highlight++;
            break;
        case 10:
            strcpy(next_process, types[highlight]);
            choice = highlight;
            break;
        }
    }
}

void select_system_resource(WINDOW *task_win, char *resource)
{
    const char *resources[] = {"CPU", "RAM", "DISK"};
    int highlight = 0;
    int choice = -1;
    int c;

    while (choice == -1)
    {
        werase(task_win);
        box(task_win, 0, 0);
        mvwprintw(task_win, 1, 1, "Select Resource:");

        for (int i = 0; i < 3; i++)
        {
            if (i == highlight)
                wattron(task_win, A_REVERSE);
            mvwprintw(task_win, i + 2, 1, "%s", resources[i]);
            if (i == highlight)
                wattroff(task_win, A_REVERSE);
        }

        wrefresh(task_win);
        c = wgetch(task_win);

        switch (c)
        {
        case KEY_UP:
            if (highlight > 0)
                highlight--;
            break;
        case KEY_DOWN:
            if (highlight < 2)
                highlight++;
            break;
        case 10: // Enter key
            strcpy(resource, resources[highlight]);
            choice = highlight;
            break;
        }
    }
}

void select_system_operation(WINDOW *task_win, char *operation)
{
    const char *operations[] = {"MAX", "MIN"};
    int highlight = 0;
    int choice = -1;
    int c;

    while (choice == -1)
    {
        werase(task_win);
        box(task_win, 0, 0);
        mvwprintw(task_win, 1, 1, "Select Operation:");

        for (int i = 0; i < 2; i++)
        {
            if (i == highlight)
                wattron(task_win, A_REVERSE);
            mvwprintw(task_win, i + 2, 1, "%s", operations[i]);
            if (i == highlight)
                wattroff(task_win, A_REVERSE);
        }

        wrefresh(task_win);
        c = wgetch(task_win);

        switch (c)
        {
        case KEY_UP:
            if (highlight > 0)
                highlight--;
            break;
        case KEY_DOWN:
            if (highlight < 1)
                highlight++;
            break;
        case 10: // Enter key
            strcpy(operation, operations[highlight]);
            choice = highlight;
            break;
        }
    }
}
// 파일 이벤트 태스크 설정
void get_file_event(WINDOW *task_win)
{
    char filePath[20];
    display_input_window("Enter filePath: ", filePath);
    char event[20];
    select_file_event(task_win, event);

    strcpy(tasks[task_count].target, filePath);
    strcpy(tasks[task_count].event, event);
}

// 프로세스 이벤트 태스크 설정
void get_process_event(WINDOW *task_win)
{
    char variableType[20];
    select_variable_type(task_win, variableType);

    char target[20];
    if (strcmp(variableType, "PID") == 0)
        display_input_window("Enter PID: ", target);
    else
        display_input_window("Enter Process Name: ", target);

    char event[20];
    select_process_event(task_win, event);

    strcpy(tasks[task_count].variableType, variableType);
    strcpy(tasks[task_count].target, target);
    strcpy(tasks[task_count].event, event);
}

// 타이머 이벤트 태스크 설정
void get_timer_event(WINDOW *task_win)
{
    char variableType[20];
    select_timer_type(task_win, variableType);

    char timeStr[20];
    if (strcmp(variableType, "TIMER") == 0)
        display_input_window("Enter time(s): ", timeStr);
    else
        display_input_window("Enter time(HH:MM): ", timeStr);

    strcpy(tasks[task_count].variableType, variableType);
    strcpy(tasks[task_count].target, timeStr);
}

// system 타입 태스크 설정
void get_system_event(WINDOW *task_win)
{
    char resource[50];
    char operation[50];
    char value_str[50];

    // 리소스 선택 (CPU, RAM, DISK)
    select_system_resource(task_win, resource);

    // 작업 선택 (MAX, MIN)
    select_system_operation(task_win, operation);

    // 값 입력 (0 ~ 100)
    display_input_window("Enter Value (0~100): ", value_str);

    // 값 검증
    int value = atoi(value_str);
    if (value < 0 || value > 100)
    {
        mvwprintw(task_win, 1, 1, "Invalid Value! Must be 0~100.");
        wrefresh(task_win);
        sleep(1);
        return;
    }

    // 태스크 설정
    strcpy(tasks[task_count].target, resource);
    strcpy(tasks[task_count].event, operation);
    strcpy(tasks[task_count].parameter, value_str);
}
// 다음 프로세스 설정 (LogWrite, Execute, Change_Attr, Terminate 선택 후 필요시 파라미터 입력)
void get_next_process(WINDOW *task_win)
{
    char next_process[20];
    select_next_process(task_win, next_process);
    strcpy(tasks[task_count].next_process, next_process);

    if ((strcmp(next_process, "Execute") == 0) ||
        (strcmp(next_process, "Change_Attr") == 0))
    {
        char next_target[20];
        char parameter[20];
        display_input_window("Enter next filePath: ", next_target);
        display_input_window("Enter parameter(s): ", parameter);

        strcpy(tasks[task_count].next_target, next_target);
        strcpy(tasks[task_count].parameter, parameter);
    }
    else if (strcmp(next_process, "Terminate") == 0)
    {
        char next_variableType[20];
        select_variable_type(task_win, next_variableType);

        char next_target[20];
        if (strcmp(next_variableType, "PID") == 0)
            display_input_window("Enter PID: ", next_target);
        else
            display_input_window("Enter Process Name: ", next_target);

        strcpy(tasks[task_count].next_variableType, next_variableType);
        strcpy(tasks[task_count].next_target, next_target);
    }
    // LogWrite는 추가 인자 필요 없음
}

// 이벤트 리스너 프로세스 실행 함수
void run_event_listener(int task_num)
{
    tasks[task_num].pid = fork();
    if (tasks[task_num].pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (tasks[task_num].pid == 0)
    {
        // 자식 프로세스: 이벤트 리스너 실행
        if (strcmp(tasks[task_num].type, "file") == 0)
        {
            execl(FILE_EVENT_LISTENER, FILE_EVENT_LISTENER,
                  tasks[task_num].target, tasks[task_num].event, NULL);
        }
        else if (strcmp(tasks[task_num].type, "process") == 0)
        {
            execl(SCAN_PROC, SCAN_PROC,
                  tasks[task_num].variableType, tasks[task_num].target, tasks[task_num].event, NULL);
        }
        else if (strcmp(tasks[task_num].type, "time") == 0)
        {
            execl(TIMER, TIMER,
                  tasks[task_num].variableType, tasks[task_num].target, NULL);
        }
        else if (strcmp(tasks[task_num].type, "system") == 0)
        {
            // SystemEventListener 실행: resource operation value
            execl(SYSTEM_EVENT_LISTENER, SYSTEM_EVENT_LISTENER,
                  tasks[task_num].target,    // resource
                  tasks[task_num].event,     // operation (MAX/MIN)
                  tasks[task_num].parameter, // value
                  NULL);
        }
        perror("execl failed!");
        exit(EXIT_FAILURE);
    }
    else
    {
        // 부모 프로세스: PID 기록 후 active 상태로 변경
        tasks[task_num].active = 1;
    }
}

// 새로운 태스크 생성 함수
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
    display_input_window("Enter task name: ", name);

    char type[20];
    select_task_type(task_win, type);

    strcpy(tasks[task_count].name, name);
    strcpy(tasks[task_count].type, type);

    if (strcmp(type, "file") == 0)
    {
        get_file_event(task_win);
    }
    else if (strcmp(type, "process") == 0)
    {
        get_process_event(task_win);
    }
    else if (strcmp(type, "time") == 0)
    {
        get_timer_event(task_win);
    }
    else if (strcmp(type, "system") == 0)
    {
        get_system_event(task_win);
    }
    // status 타입에 대한 처리는 필요시 구현

    get_next_process(task_win);

    tasks[task_count].active = 0;
    tasks[task_count].pid = 0;

    // 태스크 생성 시 이벤트 리스너 자동 실행 (초기 상태 ON)
    run_event_listener(task_count);

    task_count++;
}

// 태스크 삭제 함수
void delete_task(WINDOW *task_win, int index)
{
    if (index < 0 || index >= task_count)
    {
        return;
    }

    // 활성화된 태스크면 먼저 종료
    if (tasks[index].active && tasks[index].pid > 0)
    {
        kill(tasks[index].pid, SIGKILL);
        waitpid(tasks[index].pid, NULL, 0);
    }

    // 태스크 배열에서 제거
    for (int i = index; i < task_count - 1; i++)
    {
        tasks[i] = tasks[i + 1];
    }

    task_count--;
}

// 태스크 토글 함수: ON/OFF 전환
void toggle_task(int index)
{
    if (index < 0 || index >= task_count)
    {
        return;
    }

    if (tasks[index].active)
    {
        // 현재 ON이면 OFF로: 프로세스 kill
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
        // 현재 OFF이면 ON으로: 이벤트 리스너 재실행
        run_event_listener(index);
    }
}

// 메인 함수
int main()
{
    setup_signal_handler();
    initscr();
    clear();
    noecho();
    cbreak();

    display_title();

    int startx = 0, starty = 2;
    int width = 50, height = 15;
    int highlight = 0;
    int c;

    init_tasks();

    WINDOW *task_win = newwin(height, width, starty, startx);
    keypad(task_win, TRUE);

    // 메인 루프: 키 입력 처리
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
                highlight--;
            break;
        case KEY_DOWN:
            if (highlight < task_count - 1)
                highlight++;
            break;
        case ' ':
            // 스페이스바로 태스크 토글
            toggle_task(highlight);
            break;
        case 'a':
            // 태스크 추가
            create_task(task_win);
            break;
        case 'd':
            // 태스크 삭제
            delete_task(task_win, highlight);
            if (highlight >= task_count && highlight > 0)
                highlight--;
            break;
        case 'q':
            // 종료
            endwin();
            return 0;
        }
    }

    endwin();
    return 0;
}