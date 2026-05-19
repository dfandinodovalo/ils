#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <ncursesw/ncurses.h>

enum {
    COLOR_BG = 1,
    COLOR_HEADER,
    COLOR_SELECTED,
    COLOR_FILE,
    COLOR_FOLDER,
    COLOR_HIDDEN,
    COLOR_SYMLINK,
    COLOR_FOOTER,
    COLOR_SEARCH,
    COLOR_SCROLLBAR,
};

#define MARGIN_X 2

void init_ncurses(void);
void end_ncurses(void);
void compute_layout(int *header_h, int *list_h, int *footer_h);
void print_wstring(int y, int x, const char *str);

#endif
