#include "screen_manager.h"
#include <locale.h>
#include <wchar.h>
#include <string.h>

extern int mvaddwstr(int, int, const wchar_t *);

void init_ncurses() {
    setlocale(LC_ALL, "");
    initscr();
    start_color();
    use_default_colors();

    init_pair(COLOR_BG,        COLOR_WHITE,   COLOR_BLACK);
    init_pair(COLOR_HEADER,    COLOR_CYAN,    COLOR_BLACK);
    // init_pair(COLOR_ITEM_SEL,  COLOR_BLACK,   COLOR_CYAN);
    init_pair(COLOR_ITEM_FOCUS,COLOR_WHITE,   COLOR_BLUE);
    init_pair(COLOR_FILE,      COLOR_GREEN,   -1);
    init_pair(COLOR_FOLDER,    COLOR_YELLOW,  -1);
    init_pair(COLOR_HIDDEN,    COLOR_MAGENTA, -1);
    init_pair(COLOR_FOOTER,    COLOR_CYAN,    COLOR_BLACK);

// Reemplaza la definición de COLOR_ITEM_SEL por un gris oscuro si está disponible
init_pair(COLOR_ITEM_SEL, COLOR_WHITE, COLOR_BLACK); // O usa COLOR_BLACK, COLOR_BLUE, COLOR_MAGENTA, según disponibilidad

    curs_set(0); 
    cbreak(); 
    keypad(stdscr, TRUE); 
    noecho();
    bkgd(COLOR_PAIR(COLOR_BG));
}

void get_terminal_rows(int *header_height, int *list_height, int *footer_height) {
    int total_lines = LINES;
    *header_height = (int)(total_lines * 0.10); // 10% para cabecera
    *footer_height = (int)(total_lines * 0.20); // 20% para pie
    
    if(*header_height < 2) *header_height = 2;
    if(*footer_height < 2) *footer_height = 2;
    
    *list_height = total_lines - *header_height - *footer_height;
    if(*list_height < 1) *list_height = 1;
}

void end_ncurses() {
    endwin();
}

void print_wstring(int y, int x, const char* utf8str) {
    wchar_t wstr[512];
    mbstowcs(wstr, utf8str, sizeof(wstr)/sizeof(wchar_t)-1);
    wstr[sizeof(wstr)/sizeof(wchar_t)-1] = L'\0';
    mvaddwstr(y, x, wstr);
}