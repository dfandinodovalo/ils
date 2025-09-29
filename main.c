#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "screen_manager.h"
#include "file_utils.h"
#include "ui_renderer.h"

int main(int argc, char* argv[]) {
    char path[PATH_MAX] = ".";
    struct file_item *items = NULL;
    int total_elements = 0;
    int current_index = 0;
    bool show_hidden = false;
    int header_height, list_height, footer_height;
    int list_start_y;
    bool show_info = false;

    if (argc == 2) 
        strncpy(path, argv[1], sizeof(path)-1);

    init_ncurses();
    get_terminal_rows(&header_height, &list_height, &footer_height);
    list_start_y = header_height;
    
    chdir(path);
    getcwd(path, sizeof(path));
    get_files(path, &items, &total_elements, show_hidden);

    while (1) {
        clear();
        print_header(path);
        print_list(items, total_elements, current_index, list_start_y, list_height);
        print_footer(total_elements, show_hidden);
        refresh();

        // Manejo de entrada
        int ch = getch();
        switch (ch) {
            case KEY_UP: case 'k': case 'w':
                current_index = (current_index - 1 + total_elements) % total_elements;
                break;
            case KEY_DOWN: case 'j': case 's':
                current_index = (current_index + 1) % total_elements;
                break;
            case KEY_LEFT: case 'a':
                chdir("..");
                getcwd(path, sizeof(path));
                current_index = 0;
                get_files(path, &items, &total_elements, show_hidden);
                break;
            case KEY_RIGHT: case '\n': case 'l': case 'd':
                go_to_directory(current_index, items, total_elements, 
                                path, &current_index, &items, &total_elements, show_hidden);
                break;
            case 'h': case 'H':
                show_hidden = !show_hidden;
                current_index = 0;
                get_files(path, &items, &total_elements, show_hidden);
                break;
            case 'q': case 'Q': case 'x': case 'X':
                clean_items(&items);
                end_ncurses();
                exit(0);
                break;
            case 'i':
                show_info = !show_info;
                break;
            case KEY_RESIZE:
                get_terminal_rows(&header_height, &list_height, &footer_height);
                list_start_y = header_height;
                break;
            default:
                break;
        }
    }

    clean_items(&items);
    end_ncurses();
    return 0;
}