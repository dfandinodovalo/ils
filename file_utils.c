#define _GNU_SOURCE
#include "file_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

static int compare_items(const void *a, const void *b) {
    const file_item *ia = a, *ib = b;
    if (ia->is_directory != ib->is_directory)
        return ib->is_directory - ia->is_directory;
    if (ia->is_hidden != ib->is_hidden)
        return ia->is_hidden - ib->is_hidden;
    return strcasecmp(ia->name, ib->name);
}

static bool matches_filter(const char *name, const char *filter) {
    if (!filter[0]) return true;
    return strcasestr(name, filter) != NULL;
}

void rebuild_visible(app_state *state) {
    free(state->visible);
    state->visible = NULL;

    if (state->total_items == 0) {
        state->visible_count = 0;
        state->cursor = 0;
        return;
    }

    state->visible = malloc(state->total_items * sizeof(int));
    state->visible_count = 0;

    for (int i = 0; i < state->total_items; i++) {
        if (matches_filter(state->items[i].name, state->filter))
            state->visible[state->visible_count++] = i;
    }

    if (state->cursor >= state->visible_count)
        state->cursor = state->visible_count > 0 ? state->visible_count - 1 : 0;
}

void load_directory(app_state *state) {
    free(state->items);
    state->items = NULL;
    state->total_items = 0;

    DIR *d = opendir(state->path);
    if (!d) return;

    int capacity = 64;
    state->items = malloc(capacity * sizeof(file_item));
    int count = 0;

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        bool hidden = entry->d_name[0] == '.';
        if (!state->show_hidden && hidden)
            continue;

        if (count >= capacity) {
            capacity *= 2;
            state->items = realloc(state->items, capacity * sizeof(file_item));
        }

        file_item *item = &state->items[count];
        memset(item, 0, sizeof(file_item));
        snprintf(item->name, sizeof(item->name), "%s", entry->d_name);

        char fullpath[PATH_MAX + 256];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", state->path, entry->d_name);

        struct stat lstat_info;
        if (lstat(fullpath, &lstat_info) != 0)
            continue;

        item->is_symlink = S_ISLNK(lstat_info.st_mode);

        if (stat(fullpath, &item->stat_info) != 0)
            item->stat_info = lstat_info;

        item->is_directory = S_ISDIR(item->stat_info.st_mode);
        item->is_hidden = hidden;

        format_permissions(item->stat_info.st_mode, item->is_symlink, item->permissions);

        struct passwd *pw = getpwuid(item->stat_info.st_uid);
        if (pw)
            strncpy(item->owner, pw->pw_name, sizeof(item->owner) - 1);
        else
            snprintf(item->owner, sizeof(item->owner), "%d", item->stat_info.st_uid);

        struct group *gr = getgrgid(item->stat_info.st_gid);
        if (gr)
            strncpy(item->group, gr->gr_name, sizeof(item->group) - 1);
        else
            snprintf(item->group, sizeof(item->group), "%d", item->stat_info.st_gid);

        if (item->is_symlink) {
            ssize_t len = readlink(fullpath, item->link_target, sizeof(item->link_target) - 1);
            if (len > 0) item->link_target[len] = '\0';
        }

        count++;
    }

    closedir(d);
    state->total_items = count;
    qsort(state->items, count, sizeof(file_item), compare_items);

    state->filter_len = 0;
    state->filter[0] = '\0';
    state->filter_active = false;
    rebuild_visible(state);
}

void navigate_parent(app_state *state) {
    if (chdir("..") == 0 && getcwd(state->path, sizeof(state->path))) {
        state->cursor = 0;
        load_directory(state);
    }
}

void enter_selected(app_state *state) {
    if (state->visible_count == 0) return;

    file_item *item = &state->items[state->visible[state->cursor]];
    if (!item->is_directory) return;

    char newpath[PATH_MAX + 256];
    snprintf(newpath, sizeof(newpath), "%s/%s", state->path, item->name);

    if (chdir(newpath) == 0 && getcwd(state->path, sizeof(state->path))) {
        state->cursor = 0;
        load_directory(state);
    }
}

void toggle_hidden(app_state *state) {
    state->show_hidden = !state->show_hidden;
    state->cursor = 0;
    load_directory(state);
}

void cleanup_state(app_state *state) {
    free(state->items);
    free(state->visible);
    state->items = NULL;
    state->visible = NULL;
}

void format_permissions(mode_t mode, bool is_symlink, char *perms) {
    perms[0] = is_symlink ? 'l' : S_ISDIR(mode) ? 'd' : '-';
    const char *rwx = "rwxrwxrwx";
    for (int i = 0; i < 9; i++)
        perms[i + 1] = (mode & (1 << (8 - i))) ? rwx[i] : '-';
    perms[10] = '\0';
}

void format_size(off_t size, bool is_dir, char *buf, size_t buflen) {
    if (is_dir)
        snprintf(buf, buflen, "<DIR>");
    else if (size < 1024)
        snprintf(buf, buflen, "%ld B", (long)size);
    else if (size < 1024 * 1024)
        snprintf(buf, buflen, "%.1f KB", (double)size / 1024);
    else if (size < 1024L * 1024 * 1024)
        snprintf(buf, buflen, "%.1f MB", (double)size / (1024 * 1024));
    else
        snprintf(buf, buflen, "%.1f GB", (double)size / (1024L * 1024 * 1024));
}
