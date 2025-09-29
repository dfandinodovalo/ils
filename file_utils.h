#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdbool.h>
#include <pwd.h>    // Para getpwuid
#include <grp.h>    // Para getgrgid

struct file_item {
    char name[256];
    struct stat stat_info;
    int is_directory;
    int is_hidden;
    char permissions[12];  // Para almacenar "drwxrwxrwx"
    char owner[32];        // Nombre del propietario
    char group[32];        // Nombre del grupo
};

// Obtiene la lista de archivos en el directorio especificado
void get_files(const char* path, struct file_item **items, int *total_elements, bool show_hidden);

// Limpia la memoria de los items
void clean_items(struct file_item **items);

// Navega al directorio seleccionado
void go_to_directory(int idx, struct file_item *items, int total_elements, 
                    char *path, int *current_index, 
                    struct file_item **items_ptr, int *total_elements_ptr, bool show_hidden);

// Obtiene los permisos formateados
void format_permissions(mode_t mode, char *perms);

// Obtiene el nombre del propietario
void get_owner_name(uid_t uid, char *owner, size_t size);

// Obtiene el nombre del grupo
void get_group_name(gid_t gid, char *group, size_t size);

// Función para comparar items (para qsort)
int compare_items(const void *a, const void *b);

#endif // FILE_UTILS_H