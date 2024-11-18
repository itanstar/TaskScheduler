#include <ncurses.h>
#include <string.h>

#define NUM_OPTIONS 4

int main()
{
    const char *options[NUM_OPTIONS] = {"Option 1", "Option 2", "Option 3", "Exit"};
    int current_selection = 0;
    int ch;

    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE); // 방향키 입력 허용
    curs_set(0);          // 커서 숨김

    while (1)
    {
        // 화면 초기화
        clear();
        mvprintw(2, 10, "Select option you want to control:");

        for (int i = 0; i < NUM_OPTIONS; i++)
        {
            if (i == current_selection)
            {
                attron(A_REVERSE); // 선택된 항목 강조
                mvprintw(5 + i, 10, "%s", options[i]);
                attroff(A_REVERSE);
            }
            else
            {
                mvprintw(5 + i, 10, "%s", options[i]);
            }
        }

        // 사용자 입력 처리
        ch = getch();
        if (ch == '\t' || ch == KEY_DOWN || ch == 'j')
        { // Tab, ↓, j
            current_selection = (current_selection + 1) % NUM_OPTIONS;
        }
        else if (ch == KEY_UP || ch == 'k')
        { // ↑, k
            current_selection = (current_selection - 1 + NUM_OPTIONS) % NUM_OPTIONS;
        }
        else if (ch == '\n')
        { // Enter
            if (current_selection == NUM_OPTIONS - 1)
            { // "Exit" 선택 시 종료
                break;
            }
            else
            {
                mvprintw(10 + NUM_OPTIONS, 10, "You selected: %s", options[current_selection]);
                refresh();
                getch(); // 메시지를 확인할 시간 제공
            }
        }
    }

    // ncurses 종료
    endwin();
    return 0;
}