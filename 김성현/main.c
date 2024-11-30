#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_TASKS 10

typedef struct Task
{
    char name[50];
    char type[20]; // file, process, time, status
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
    tasks[task_count].active = 0;
    tasks[task_count].pid = 0;
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