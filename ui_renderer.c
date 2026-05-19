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

        const char *help = "/ filter  p preview  i info  h hidden  c cd&quit  q quit";
        int help_x = COLS - (int)strlen(help) - MARGIN_X;
        if (help_x > MARGIN_X + (int)strlen(status) + 2) {
            attron(A_DIM);
            mvprintw(y, help_x, "%s", help);
            attroff(A_DIM);
        }

        attroff(COLOR_PAIR(COLOR_FOOTER));
    }
}

static void render_preview(app_state *state) {
    int pad = 2;
    int box_y = pad;
    int box_x = pad + 1;
    int box_h = LINES - pad * 2;
    int box_w = COLS - (pad + 1) * 2;
    if (box_h < 5 || box_w < 20) return;

    int inner_h = box_h - 2;
    int inner_w = box_w - 2;

    attron(COLOR_PAIR(COLOR_PREVIEW_BORDER));
    mvaddch(box_y, box_x, ACS_ULCORNER);
    mvaddch(box_y, box_x + box_w - 1, ACS_URCORNER);
    mvaddch(box_y + box_h - 1, box_x, ACS_LLCORNER);
    mvaddch(box_y + box_h - 1, box_x + box_w - 1, ACS_LRCORNER);
    for (int i = 1; i < box_w - 1; i++) {
        mvaddch(box_y, box_x + i, ACS_HLINE);
        mvaddch(box_y + box_h - 1, box_x + i, ACS_HLINE);
    }
    for (int i = 1; i < box_h - 1; i++) {
        mvaddch(box_y + i, box_x, ACS_VLINE);
        mvaddch(box_y + i, box_x + box_w - 1, ACS_VLINE);
        move(box_y + i, box_x + 1);
        for (int c = 0; c < inner_w; c++)
            addch(' ');
    }
    attroff(COLOR_PAIR(COLOR_PREVIEW_BORDER));

    char title[300];
    snprintf(title, sizeof(title), " %s ", state->preview_filename);
    int title_len = (int)strlen(title);
    if (title_len > box_w - 4) title_len = box_w - 4;
    attron(COLOR_PAIR(COLOR_PREVIEW_TITLE) | A_BOLD);
    mvprintw(box_y, box_x + 2, "%.*s", title_len, title);
    attroff(COLOR_PAIR(COLOR_PREVIEW_TITLE) | A_BOLD);

    if (state->preview_binary) {
        const char *msg = "Binary file - preview not available";
        attron(COLOR_PAIR(COLOR_PREVIEW_TEXT) | A_DIM);
        mvprintw(box_y + inner_h / 2 + 1, box_x + (box_w - (int)strlen(msg)) / 2, "%s", msg);
        attroff(COLOR_PAIR(COLOR_PREVIEW_TEXT) | A_DIM);
    } else if (state->preview_line_count == 0) {
        const char *msg = "Empty file";
        attron(COLOR_PAIR(COLOR_PREVIEW_TEXT) | A_DIM);
        mvprintw(box_y + inner_h / 2 + 1, box_x + (box_w - (int)strlen(msg)) / 2, "%s", msg);
        attroff(COLOR_PAIR(COLOR_PREVIEW_TEXT) | A_DIM);
    } else {
        int lineno_width = 0;
        int max_lineno = state->preview_scroll + inner_h;
        if (max_lineno > state->preview_line_count)
            max_lineno = state->preview_line_count;
        for (int n = max_lineno; n > 0; n /= 10)
            lineno_width++;
        if (lineno_width < 3) lineno_width = 3;

        int text_w = inner_w - lineno_width - 2;
        if (text_w < 1) text_w = 1;

        for (int i = 0; i < inner_h && state->preview_scroll + i < state->preview_line_count; i++) {
            int line_idx = state->preview_scroll + i;
            int y = box_y + 1 + i;

            attron(COLOR_PAIR(COLOR_PREVIEW_LINENO) | A_DIM);
            mvprintw(y, box_x + 1, "%*d ", lineno_width, line_idx + 1);
            attroff(COLOR_PAIR(COLOR_PREVIEW_LINENO) | A_DIM);

            attron(COLOR_PAIR(COLOR_PREVIEW_TEXT));
            char *line = state->preview_lines[line_idx];
            int col = 0;
            int screen_x = box_x + 1 + lineno_width + 1;
            for (int j = 0; line[j] && col < text_w; j++) {
                if (line[j] == '\t') {
                    int spaces = 4 - (col % 4);
                    for (int s = 0; s < spaces && col < text_w; s++, col++)
                        mvaddch(y, screen_x + col, ' ');
                } else {
                    mvaddch(y, screen_x + col, (unsigned char)line[j]);
                    col++;
                }
            }
            attroff(COLOR_PAIR(COLOR_PREVIEW_TEXT));
        }
    }

    char footer_info[128];
    if (state->preview_binary) {
        snprintf(footer_info, sizeof(footer_info), " binary ");
    } else {
        snprintf(footer_info, sizeof(footer_info), " %d lines | Esc/p close | arrows scroll ",
                 state->preview_line_count);
    }
    int fi_len = (int)strlen(footer_info);
    if (fi_len > box_w - 4) fi_len = box_w - 4;
    attron(COLOR_PAIR(COLOR_PREVIEW_BORDER) | A_DIM);
    mvprintw(box_y + box_h - 1, box_x + 2, "%.*s", fi_len, footer_info);
    attroff(COLOR_PAIR(COLOR_PREVIEW_BORDER) | A_DIM);
}

void render(app_state *state) {
    erase();
    render_header(state);
    render_list(state);
    render_footer(state);
    if (state->preview_active)
        render_preview(state);
    refresh();
}
