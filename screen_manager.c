#include "screen_manager.h"
#include <locale.h>
#include <stdlib.h>
#include <wchar.h>

extern int mvaddwstr(int, int, const wchar_t *);

void init_ncurses(void) {
    setlocale(LC_ALL, "");
    initscr();
    start_color();
    use_default_colors();

    init_pair(COLOR_BG,        COLOR_WHITE,   -1);
    init_pair(COLOR_HEADER,    COLOR_CYAN,    -1);
    init_pair(COLOR_SELECTED,  COLOR_BLACK,   COLOR_CYAN);
    init_pair(COLOR_FILE,      COLOR_GREEN,   -1);
    init_pair(COLOR_FOLDER,    COLOR_YELLOW,  -1);
    init_pair(COLOR_HIDDEN,    COLOR_MAGENTA, -1);
    init_pair(COLOR_SYMLINK,   COLOR_CYAN,    -1);
    init_pair(COLOR_FOOTER,    COLOR_WHITE,   -1);
    init_pair(COLOR_SEARCH,    COLOR_YELLOW,  -1);
    init_pair(COLOR_SCROLLBAR, COLOR_CYAN,    -1);

    curs_set(0);
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
}

void end_ncurses(void) {
    endwin();
}

void compute_layout(int *header_h, int *list_h, int *footer_h) {
    *header_h = 3;
    *footer_h = 2;
    *list_h = LINES - *header_h - *footer_h;
    if (*list_h < 1) *list_h = 1;
}

void print_wstring(int y, int x, const char *str) {
    wchar_t wstr[1024];
    size_t n = mbstowcs(wstr, str, sizeof(wstr) / sizeof(wchar_t) - 1);
    if (n == (size_t)-1) {
        mvprintw(y, x, "%s", str);
        return;
    }
    wstr[n] = L'\0';
    mvaddwstr(y, x, wstr);
}
