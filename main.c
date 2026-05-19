#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "screen_manager.h"
#include "file_utils.h"
#include "ui_renderer.h"

int main(int argc, char *argv[]) {
    app_state state = {0};
    state.running = true;
    state.show_info = true;

    if (argc == 2) {
        if (chdir(argv[1]) != 0) {
            fprintf(stderr, "ils: cannot access '%s'\n", argv[1]);
            return 1;
        }
    }
    if (!getcwd(state.path, sizeof(state.path))) {
        fprintf(stderr, "ils: cannot get current directory\n");
        return 1;
    }

    init_ncurses();
    compute_layout(&state.header_h, &state.list_h, &state.footer_h);
    state.list_y = state.header_h;

    load_directory(&state);

    while (state.running) {
        render(&state);
        int ch = getch();

        if (state.preview_active) {
            int preview_h = LINES - 6;
            if (preview_h < 3) preview_h = 3;
            switch (ch) {
                case 27: case 'p': case 'q':
                    free_preview(&state);
                    state.preview_active = false;
                    break;
                case KEY_UP: case 'k': case 'w':
                    if (state.preview_scroll > 0)
                        state.preview_scroll--;
                    break;
                case KEY_DOWN: case 'j': case 's':
                    if (state.preview_scroll < state.preview_line_count - preview_h)
                        state.preview_scroll++;
                    if (state.preview_scroll < 0) state.preview_scroll = 0;
                    break;
                case KEY_PPAGE:
                    state.preview_scroll -= preview_h;
                    if (state.preview_scroll < 0) state.preview_scroll = 0;
                    break;
                case KEY_NPAGE:
                    state.preview_scroll += preview_h;
                    if (state.preview_scroll > state.preview_line_count - preview_h)
                        state.preview_scroll = state.preview_line_count - preview_h;
                    if (state.preview_scroll < 0) state.preview_scroll = 0;
                    break;
                case KEY_HOME: case 'g':
                    state.preview_scroll = 0;
                    break;
                case KEY_END: case 'G':
                    state.preview_scroll = state.preview_line_count - preview_h;
                    if (state.preview_scroll < 0) state.preview_scroll = 0;
                    break;
                case KEY_RESIZE:
                    compute_layout(&state.header_h, &state.list_h, &state.footer_h);
                    state.list_y = state.header_h;
                    break;
            }
            continue;
        }

        if (state.filter_active) {
            switch (ch) {
                case 27:
                    state.filter_active = false;
                    state.filter_len = 0;
                    state.filter[0] = '\0';
                    state.cursor = 0;
                    rebuild_visible(&state);
                    break;
                case '\n':
                    state.filter_active = false;
                    break;
                case KEY_BACKSPACE: case 127: case 8:
                    if (state.filter_len > 0) {
                        state.filter[--state.filter_len] = '\0';
                        state.cursor = 0;
                        rebuild_visible(&state);
                    } else {
                        state.filter_active = false;
                    }
                    break;
                case KEY_UP:
                    if (state.visible_count > 0)
                        state.cursor = (state.cursor - 1 + state.visible_count) % state.visible_count;
                    break;
                case KEY_DOWN:
                    if (state.visible_count > 0)
                        state.cursor = (state.cursor + 1) % state.visible_count;
                    break;
                case KEY_RESIZE:
                    compute_layout(&state.header_h, &state.list_h, &state.footer_h);
                    state.list_y = state.header_h;
                    break;
                default:
                    if (ch >= 32 && ch < 127 && state.filter_len < (int)sizeof(state.filter) - 1) {
                        state.filter[state.filter_len++] = (char)ch;
                        state.filter[state.filter_len] = '\0';
                        state.cursor = 0;
                        rebuild_visible(&state);
                    }
                    break;
            }
            continue;
        }

        switch (ch) {
            case KEY_UP: case 'k': case 'w':
                if (state.visible_count > 0)
                    state.cursor = (state.cursor - 1 + state.visible_count) % state.visible_count;
                break;
            case KEY_DOWN: case 'j': case 's':
                if (state.visible_count > 0)
                    state.cursor = (state.cursor + 1) % state.visible_count;
                break;
            case KEY_LEFT: case 'a':
                navigate_parent(&state);
                break;
            case KEY_RIGHT: case '\n': case 'l': case 'd':
                enter_selected(&state);
                break;
            case KEY_PPAGE:
                if (state.visible_count > 0) {
                    state.cursor -= state.list_h;
                    if (state.cursor < 0) state.cursor = 0;
                }
                break;
            case KEY_NPAGE:
                if (state.visible_count > 0) {
                    state.cursor += state.list_h;
                    if (state.cursor >= state.visible_count)
                        state.cursor = state.visible_count - 1;
                }
                break;
            case KEY_HOME: case 'g':
                state.cursor = 0;
                break;
            case KEY_END: case 'G':
                if (state.visible_count > 0)
                    state.cursor = state.visible_count - 1;
                break;
            case 'h': case 'H':
                toggle_hidden(&state);
                break;
            case 'i':
                state.show_info = !state.show_info;
                break;
            case '/':
                state.filter_active = true;
                state.filter_len = 0;
                state.filter[0] = '\0';
                break;
            case 'p':
                if (state.visible_count > 0 &&
                    !state.items[state.visible[state.cursor]].is_directory)
                    load_preview(&state);
                break;
            case 'c':
                state.cd_on_exit = true;
                state.running = false;
                break;
            case 'q': case 'Q': case 'x': case 'X':
                state.running = false;
                break;
            case KEY_RESIZE:
                compute_layout(&state.header_h, &state.list_h, &state.footer_h);
                state.list_y = state.header_h;
                break;
        }
    }

    cleanup_state(&state);
    end_ncurses();

    if (state.cd_on_exit) {
        const char *tmpdir = getenv("TMPDIR");
        if (!tmpdir) tmpdir = "/tmp";
        char lastdir_path[PATH_MAX];
        snprintf(lastdir_path, sizeof(lastdir_path), "%s/ils_lastdir", tmpdir);
        FILE *f = fopen(lastdir_path, "w");
        if (f) {
            fprintf(f, "%s", state.path);
            fclose(f);
        }
    }

    return 0;
}
