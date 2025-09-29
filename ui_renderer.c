#include "ui_renderer.h"
#include "screen_manager.h"
#include <ncursesw/ncurses.h>
#include <time.h>
#include <string.h>

void print_header(const char *path) {
    attron(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
    print_wstring(0, MARGIN_X, "File Explorer");
    attroff(A_BOLD);

    move(1, MARGIN_X);
    attron(COLOR_PAIR(COLOR_HEADER));
    int maxwidth = COLS - 2*MARGIN_X;
    char ruta[PATH_MAX+40];
    snprintf(ruta, sizeof(ruta), "[Path: %s]", path);
    ruta[maxwidth-1] = 0;
    print_wstring(1, MARGIN_X, ruta);
    attroff(COLOR_PAIR(COLOR_HEADER));
}

void print_list(struct file_item *items, int total_elements, int current_index, 
                int list_start_y, int max_rows) {
    int start = 0, end = total_elements;
    if (total_elements > max_rows) {
        start = current_index - max_rows/2;
        if (start < 0) start = 0;
        end = start + max_rows;
        if (end > total_elements) { end = total_elements; start = end - max_rows; }
    }

    // Barra de scroll a la izquierda
    for (int row = 0; row < max_rows; ++row) {
        int barra_y = list_start_y + row;
        int idx = start + row;
        chtype barra_char = ' ';
        if (idx == current_index) {
            attron(COLOR_PAIR(COLOR_ITEM_SEL));
            barra_char = ACS_CKBOARD; // Bloque para el seleccionado
        } else {
            attron(COLOR_PAIR(COLOR_HEADER));
            barra_char = ACS_VLINE;   // Línea para scroll
        }
        mvaddch(barra_y, MARGIN_X-1, barra_char);
        attroff(COLOR_PAIR(COLOR_ITEM_SEL));
        attroff(COLOR_PAIR(COLOR_HEADER));
    }

    for (int i = start; i < end; ++i) {
        int y = i-start+list_start_y;
        int is_sel = (i == current_index);
        struct file_item* f = &items[i];

        // Fondo del seleccionado
        if (is_sel)
            attron(COLOR_PAIR(COLOR_ITEM_SEL));
        else if (f->is_directory)
            attron(COLOR_PAIR(COLOR_FOLDER));
        else if (f->is_hidden)
            attron(COLOR_PAIR(COLOR_HIDDEN));
        else
            attron(COLOR_PAIR(COLOR_FILE));

        // --- Nombre del archivo (izquierda) ---
        char name_str[256 + 4];
        snprintf(name_str, sizeof(name_str), " %-30s", f->name);

        int is_executable = (f->stat_info.st_mode & S_IXUSR) ||
                            (f->stat_info.st_mode & S_IXGRP) ||
                            (f->stat_info.st_mode & S_IXOTH);

        if (is_executable)
            attron(A_BOLD);

        print_wstring(y, MARGIN_X, name_str);

        if (is_executable)
            attroff(A_BOLD);

        // --- Info (derecha) ---
        char size_str[20];
        if (f->is_directory)
            strcpy(size_str, "<DIR>");
        else {
            if (f->stat_info.st_size < 1024)
                snprintf(size_str, sizeof(size_str), "%ld B", f->stat_info.st_size);
            else if (f->stat_info.st_size < 1024*1024)
                snprintf(size_str, sizeof(size_str), "%.1f KB", (double)f->stat_info.st_size / 1024);
            else
                snprintf(size_str, sizeof(size_str), "%.2f MB", (double)f->stat_info.st_size / (1024 * 1024));
        }

        char timebuf[20];
        struct tm *mt = localtime(&f->stat_info.st_mtime);
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", mt);

        char info_right[128];
        snprintf(
            info_right, sizeof(info_right),
            "%-11s %-16s %-16s %12s %12s",
            f->permissions, f->owner, f->group, size_str, timebuf
        );

        int info_width = (int)strlen(info_right);
        int x_right = COLS - info_width - MARGIN_X;
        if (x_right <= MARGIN_X + (int)strlen(name_str) + 2)
            x_right = MARGIN_X + (int)strlen(name_str) + 2;

        print_wstring(y, x_right, info_right);

        attroff(COLOR_PAIR(COLOR_ITEM_SEL));
        attroff(COLOR_PAIR(COLOR_FOLDER));
        attroff(COLOR_PAIR(COLOR_FILE));
        attroff(COLOR_PAIR(COLOR_HIDDEN));
    }
}

void print_footer(int total_elements, bool show_hidden) {
    int y = LINES - ((int)(LINES * 0.10));
    attron(COLOR_PAIR(COLOR_FOOTER));
    mvhline(y, 0, ' ', COLS);
    print_wstring(y, MARGIN_X, "[Up/Down] Navegar   [Enter] Abrir   [h] Mostrar ocultos   [q] Salir");
    char footerline[128];
    snprintf(footerline, sizeof(footerline), " %d elemento%s%s", total_elements, 
             total_elements==1?"":"s", show_hidden? " | ocultos visibles":"");
    print_wstring(y+1, MARGIN_X, footerline);
    attroff(COLOR_PAIR(COLOR_FOOTER));
}