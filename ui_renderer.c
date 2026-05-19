#include "ui_renderer.h"
#include "screen_manager.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

static void render_header(app_state *state) {
    attron(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
    mvprintw(0, MARGIN_X, "ILS");
    attroff(A_BOLD);

    const char *home = getenv("HOME");
    char display_path[PATH_MAX];
    if (home && home[0] && strncmp(state->path, home, strlen(home)) == 0) {
        snprintf(display_path, sizeof(display_path), "~%s", state->path + strlen(home));
    } else {
        strncpy(display_path, state->path, sizeof(display_path) - 1);
        display_path[sizeof(display_path) - 1] = '\0';
    }

    int max_w = COLS - MARGIN_X * 2;
    if ((int)strlen(display_path) > max_w)
        display_path[max_w] = '\0';

    mvprintw(1, MARGIN_X, "%s", display_path);
    attroff(COLOR_PAIR(COLOR_HEADER));

    attron(COLOR_PAIR(COLOR_HEADER) | A_DIM);
    mvhline(2, 0, ACS_HLINE, COLS);
    attroff(COLOR_PAIR(COLOR_HEADER) | A_DIM);
}

static void render_scrollbar(app_state *state) {
    if (state->visible_count <= state->list_h || COLS < 40)
        return;

    int x = COLS - 1;
    int track_h = state->list_h;

    int thumb_h = (track_h * state->list_h) / state->visible_count;
    if (thumb_h < 1) thumb_h = 1;

    int thumb_pos = 0;
    if (state->visible_count > 1)
        thumb_pos = (state->cursor * (track_h - thumb_h)) / (state->visible_count - 1);

    for (int i = 0; i < track_h; i++) {
        int y = state->list_y + i;
        if (i >= thumb_pos && i < thumb_pos + thumb_h) {
            attron(COLOR_PAIR(COLOR_SCROLLBAR) | A_REVERSE);
            mvaddch(y, x, ' ');
            attroff(COLOR_PAIR(COLOR_SCROLLBAR) | A_REVERSE);
        } else {
            attron(COLOR_PAIR(COLOR_SCROLLBAR) | A_DIM);
            mvaddch(y, x, ACS_VLINE);
            attroff(COLOR_PAIR(COLOR_SCROLLBAR) | A_DIM);
        }
    }
}

static void render_list(app_state *state) {
    if (!state->items || !state->visible || state->visible_count == 0) {
        attron(COLOR_PAIR(COLOR_HIDDEN) | A_DIM);
        const char *msg = state->filter_len > 0 ? "No matches" : "Empty directory";
        mvprintw(state->list_y + state->list_h / 2,
                 (COLS - (int)strlen(msg)) / 2, "%s", msg);
        attroff(COLOR_PAIR(COLOR_HIDDEN) | A_DIM);
        return;
    }

    int start = 0;
    if (state->visible_count > state->list_h) {
        start = state->cursor - state->list_h / 2;
        if (start < 0) start = 0;
        if (start + state->list_h > state->visible_count)
            start = state->visible_count - state->list_h;
    }

    int rows = state->list_h;
    if (state->visible_count < rows)
        rows = state->visible_count;

    for (int row = 0; row < rows; row++) {
        int vi = start + row;
        int idx = state->visible[vi];
        file_item *f = &state->items[idx];
        int y = state->list_y + row;
        bool selected = (vi == state->cursor);

        int color = selected   ? COLOR_SELECTED :
                    f->is_symlink  ? COLOR_SYMLINK :
                    f->is_hidden   ? COLOR_HIDDEN :
                    f->is_directory ? COLOR_FOLDER : COLOR_FILE;

        attron(COLOR_PAIR(color));

        if (selected) {
            move(y, 0);
            for (int c = 0; c < COLS; c++)
                addch(' ');
            attron(COLOR_PAIR(color));
        }

        const char *prefix = f->is_directory ? "> " :
                             f->is_symlink   ? "@ " : "  ";

        char name_buf[300];
        snprintf(name_buf, sizeof(name_buf), "%s%s", prefix, f->name);

        bool is_exec = !f->is_directory &&
                       (f->stat_info.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH));

        if (is_exec) attron(A_BOLD);
        print_wstring(y, MARGIN_X, name_buf);
        if (is_exec) attroff(A_BOLD);

        char size_str[20];
        format_size(f->stat_info.st_size, f->is_directory, size_str, sizeof(size_str));

        char timebuf[20];
        struct tm *mt = localtime(&f->stat_info.st_mtime);
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", mt);

        char info[256];
        int avail = COLS - MARGIN_X * 2 - 4;

        if (state->show_info && avail >= 90)
            snprintf(info, sizeof(info), "%-10s  %-12s %-12s  %8s  %s",
                     f->permissions, f->owner, f->group, size_str, timebuf);
        else if (state->show_info && avail >= 60)
            snprintf(info, sizeof(info), "%-10s  %8s  %s",
                     f->permissions, size_str, timebuf);
        else if (avail >= 30)
            snprintf(info, sizeof(info), "%8s  %s", size_str, timebuf);
        else
            info[0] = '\0';

        if (info[0]) {
            int info_x = COLS - (int)strlen(info) - MARGIN_X;
            if (info_x < MARGIN_X + (int)strlen(name_buf) + 2)
                info_x = MARGIN_X + (int)strlen(name_buf) + 2;
            print_wstring(y, info_x, info);
        }

        attroff(COLOR_PAIR(color));
    }

    render_scrollbar(state);
}

static void render_footer(app_state *state) {
    int y = LINES - state->footer_h;

    attron(COLOR_PAIR(COLOR_HEADER) | A_DIM);
    mvhline(y, 0, ACS_HLINE, COLS);
    attroff(COLOR_PAIR(COLOR_HEADER) | A_DIM);

    y++;

    if (state->filter_active) {
        attron(COLOR_PAIR(COLOR_SEARCH) | A_BOLD);
        mvprintw(y, MARGIN_X, "/%s", state->filter);
        attroff(COLOR_PAIR(COLOR_SEARCH) | A_BOLD);

        attron(COLOR_PAIR(COLOR_FOOTER) | A_DIM);
        printw("  (%d/%d)", state->visible_count, state->total_items);
        attroff(COLOR_PAIR(COLOR_FOOTER) | A_DIM);
    } else {
        attron(COLOR_PAIR(COLOR_FOOTER));

        char pos[32] = "";
        if (state->visible_count > 0)
            snprintf(pos, sizeof(pos), "  %d/%d", state->cursor + 1, state->visible_count);

        char status[256];
        snprintf(status, sizeof(status), "%d item%s%s%s",
                 state->visible_count,
                 state->visible_count == 1 ? "" : "s",
                 state->show_hidden ? " | hidden visible" : "",
                 pos);

        mvprintw(y, MARGIN_X, "%s", status);

        const char *help = "/ filter  i info  h hidden  c cd&quit  q quit";
        int help_x = COLS - (int)strlen(help) - MARGIN_X;
        if (help_x > MARGIN_X + (int)strlen(status) + 2) {
            attron(A_DIM);
            mvprintw(y, help_x, "%s", help);
            attroff(A_DIM);
        }

        attroff(COLOR_PAIR(COLOR_FOOTER));
    }
}

void render(app_state *state) {
    erase();
    render_header(state);
    render_list(state);
    render_footer(state);
    refresh();
}
