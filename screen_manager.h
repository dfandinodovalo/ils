#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <ncursesw/ncurses.h>

// Definiciones de colores
#define COLOR_BG             1
#define COLOR_HEADER         2
#define COLOR_ITEM_SEL       3
#define COLOR_FILE           4
#define COLOR_FOLDER         5
#define COLOR_HIDDEN         6
#define COLOR_ITEM_FOCUS     7
#define COLOR_FOOTER         8

// Márgenes y constantes de visualización
#define MARGIN_X             3

// Inicializa ncurses y los colores
void init_ncurses();

// Obtiene las dimensiones actuales del terminal
void get_terminal_rows(int *header_height, int *list_height, int *footer_height);

// Finaliza ncurses
void end_ncurses();

// Función auxiliar para imprimir cadenas UTF-8
void print_wstring(int y, int x, const char* utf8str);

#endif // SCREEN_MANAGER_H