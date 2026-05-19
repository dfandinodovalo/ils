#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdbool.h>

typedef struct {
    char name[256];
    struct stat stat_info;
    bool is_directory;
    bool is_hidden;
    bool is_symlink;
    char permissions[12];
    char owner[32];
    char group[32];
    char link_target[256];
} file_item;

typedef struct {
    char path[PATH_MAX];
    file_item *items;
    int total_items;
    int *visible;
    int visible_count;
    int cursor;
    bool show_hidden;
    bool show_info;
    bool filter_active;
    char filter[256];
    int filter_len;
    int header_h;
    int list_h;
    int footer_h;
    int list_y;
    bool running;
    bool cd_on_exit;
    bool preview_active;
    char **preview_lines;
    int preview_line_count;
    int preview_scroll;
    char preview_filename[256];
    bool preview_binary;
} app_state;

void load_directory(app_state *state);
void rebuild_visible(app_state *state);
void navigate_parent(app_state *state);
void enter_selected(app_state *state);
void toggle_hidden(app_state *state);
void cleanup_state(app_state *state);
void format_permissions(mode_t mode, bool is_symlink, char *perms);
void format_size(off_t size, bool is_dir, char *buf, size_t buflen);
void load_preview(app_state *state);
void free_preview(app_state *state);

#endif
