#include "file_utils.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int compare_items(const void *a, const void *b) {
    const struct file_item *ia = a, *ib = b;
    // Directories first
    if (ia->is_directory && !ib->is_directory) return -1;
    if (!ia->is_directory && ib->is_directory) return 1;
    // Hidden at the end
    if (!ia->is_hidden && ib->is_hidden) return -1;
    if (ia->is_hidden && !ib->is_hidden) return 1;
    // Alphabetical order
    return strcasecmp(ia->name, ib->name);
}

void clean_items(struct file_item **items) {
    if (*items) {
        free(*items);
        *items = NULL;
    }
}

void format_permissions(mode_t mode, char *perms) {
    const char *rwx = "rwxrwxrwx";
    perms[0] = S_ISDIR(mode) ? 'd' : '-';
    for (int i = 0; i < 9; i++) {
        perms[i+1] = (mode & (1 << (8-i))) ? rwx[i] : '-';
    }
    perms[10] = '\0';
}

void get_owner_name(uid_t uid, char *owner, size_t size) {
    struct passwd *pw = getpwuid(uid);
    if (pw) {
        strncpy(owner, pw->pw_name, size-1);
        owner[size-1] = '\0';
    } else {
        snprintf(owner, size, "%d", uid);
    }
}

void get_group_name(gid_t gid, char *group, size_t size) {
    struct group *gr = getgrgid(gid);
    if (gr) {
        strncpy(group, gr->gr_name, size-1);
        group[size-1] = '\0';
    } else {
        snprintf(group, size, "%d", gid);
    }
}

void get_files(const char* path, struct file_item **items, int *total_elements, bool show_hidden) {
    clean_items(items);
    *total_elements = 0;

    DIR *d;
    struct dirent *dir;
    d = opendir(path);
    if (!d) {
        perror("Error opening directory");
        return;
    }

    int count = 0;
    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;
        if (!show_hidden && dir->d_name[0] == '.') continue;
        count++;
    }

    *items = malloc(count * sizeof(struct file_item));
    if (!(*items)) {
        closedir(d);
        return;
    }
    rewinddir(d);

    int i = 0;
    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;
        int hidden = (dir->d_name[0] == '.');
        if (!show_hidden && hidden) continue;

        char fullpath[PATH_MAX];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, dir->d_name);

        strncpy((*items)[i].name, dir->d_name, sizeof((*items)[i].name)-1);
        (*items)[i].name[sizeof((*items)[i].name)-1] = '\0';

        if (stat(fullpath, &(*items)[i].stat_info) == 0) {
            (*items)[i].is_directory = S_ISDIR((*items)[i].stat_info.st_mode);
            (*items)[i].is_hidden = hidden;
            
            // Nuevos campos
            format_permissions((*items)[i].stat_info.st_mode, (*items)[i].permissions);
            get_owner_name((*items)[i].stat_info.st_uid, (*items)[i].owner, sizeof((*items)[i].owner));
            get_group_name((*items)[i].stat_info.st_gid, (*items)[i].group, sizeof((*items)[i].group));
            
            i++;
        }
    }

    *total_elements = i;
    qsort(*items, *total_elements, sizeof(struct file_item), compare_items);
    closedir(d);
}

void go_to_directory(int idx, struct file_item *items, int total_elements, 
                    char *path, int *current_index, 
                    struct file_item **items_ptr, int *total_elements_ptr, bool show_hidden) {
    if (idx < 0 || idx >= total_elements) return;
    if (items[idx].is_directory) {
        char newPath[PATH_MAX];
        strncpy(newPath, path, sizeof(newPath) - 1);
        newPath[sizeof(newPath) - 1] = '\0';
        size_t len = strlen(newPath);
        if (len < sizeof(newPath) - 2) {
            newPath[len] = '/';
            newPath[len + 1] = '\0';
            strncat(newPath, items[idx].name, sizeof(newPath) - len - 2);
        }
        if (chdir(newPath) == 0) {
            getcwd(path, PATH_MAX);
            *current_index = 0;
            get_files(path, items_ptr, total_elements_ptr, show_hidden);
        }
    }
}