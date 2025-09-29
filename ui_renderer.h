#ifndef UI_RENDERER_H
#define UI_RENDERER_H

#include "file_utils.h"
#include <stdbool.h>

// Imprime la cabecera del explorador
void print_header(const char *path);

// Imprime la lista de archivos
void print_list(struct file_item *items, int total_elements, int current_index, 
                int list_start_y, int max_rows);

// Imprime el pie de página
void print_footer(int total_elements, bool show_hidden);

#endif // UI_RENDERER_H