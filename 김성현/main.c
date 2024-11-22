#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_TASKS 10

typedef struct Task
{
    char name[50];
    char type[20]; // file, process, time, status
    int active;
} Task;

Task tasks[MAX_TASKS];
int task_count = 0;

void init_tasks()
{
    for (int i = 0; i < MAX_TASKS; i++)
    {
        tasks[i].active = 0;
        strcpy(tasks[i].name, "");
        strcpy(tasks[i].type, "");
    }
}

void draw_menu(WINDOW *menu_win, int highlight)
{
    char *choices[] = {"list", "create", "delete", "exit"};
    int n_choices = sizeof(choices) / sizeof(char *);

    box(menu_win, 0, 0);
    for (int i = 0; i < n_choices; ++i)
    {
        if (highlight == i + 1)
        {
            wattron(menu_win, A_REVERSE);
            mvwprintw(menu_win, i + 1, 1, "%s", choices[i]);
            wattroff(menu_win, A_REVERSE);
        }
        else
            mvwprintw(menu_win, i + 1, 1, "%s", choices[i]);
    }
    wrefresh(menu_win);
}

void list_tasks(WINDOW *task_win)
{
    werase(task_win);
    box(task_win, 0, 0);

    mvwprintw(task_win, 1, 1, "%-20s %-10s %-5s", "Name", "Type", "Active");
    for (int i = 0; i < task_count; i++)
    {
        mvwprintw(task_win, i + 2, 1, "%-20s %-10s %-5s", tasks[i].name, tasks[i].type, tasks[i].active ? "ON" : "OFF");
    }

    wrefresh(task_win);
}

void create_task(WINDOW *task_win)
{
    if (task_count >= MAX_TASKS)
    {
        mvwprintw(task_win, 1, 1, "Task limit reached!");
        wrefresh(task_win);
        return;
    }

    echo();
    char name[50];
    char type[20];

    mvwprintw(task_win, 1, 1, "Enter task name: ");
    wrefresh(task_win);
    wgetstr(task_win, name);

    mvwprintw(task_win, 2, 1, "Enter task type (file/process/time/status): ");
    wrefresh(task_win);
    wgetstr(task_win, type);

    noecho();

    strcpy(tasks[task_count].name, name);
    strcpy(tasks[task_count].type, type);
    tasks[task_count].active = 0;
    task_count++;
}

void delete_task(WINDOW *task_win)
{
    echo();
    char name[50];

    mvwprintw(task_win, 1, 1, "Enter task name to delete: ");
    wrefresh(task_win);
    wgetstr(task_win, name);

    noecho();

    for (int i = 0; i < task_count; i++)
    {
        if (strcmp(tasks[i].name, name) == 0)
        {
            for (int j = i; j < task_count - 1; j++)
            {
                tasks[j] = tasks[j + 1];
            }
            task_count--;
            break;
        }
    }
}

void toggle_task(WINDOW *task_win)
{
    echo();
    char name[50];

    mvwprintw(task_win, 1, 1, "Enter task name to toggle: ");
    wrefresh(task_win);
    wgetstr(task_win, name);

    noecho();

    for (int i = 0; i < task_count; i++)
    {
        if (strcmp(tasks[i].name, name) == 0)
        {
            tasks[i].active = !tasks[i].active;
            break;
        }
    }
}

int main()
{
    initscr();
    clear();
    noecho();
    cbreak();

    int startx = 0, starty = 0;
    int width = 30, height = 10;
    int highlight = 1;
    int choice = 0;
    int c;

    init_tasks();

    WINDOW *menu_win = newwin(height, width, starty, startx);
    keypad(menu_win, TRUE);

    WINDOW *task_win = newwin(height, 50, starty + height, startx);

    while (1)
    {
        draw_menu(menu_win, highlight);
        c = wgetch(menu_win);

        switch (c)
        {
        case KEY_UP:
            if (highlight == 1)
                highlight = 4;
            else
                highlight--;
            break;
        case KEY_DOWN:
            if (highlight == 4)
                highlight = 1;
            else
                highlight++;
            break;
        case 10:
            choice = highlight;
            break;
        }

        if (choice == 1)
        {
            list_tasks(task_win);
        }
        else if (choice == 2)
        {
            create_task(task_win);
        }
        else if (choice == 3)
        {
            delete_task(task_win);
        }
        else if (choice == 4)
        {
            break;
        }

        choice = 0;
    }

    endwin();
    return 0;
}
