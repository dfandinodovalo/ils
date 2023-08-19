#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>

void init_ncurses();
void get_input();
void get_terminal_rows();

void get_files(char* path);
void check_file_stats(const char* path, const char* folder, int index);
void go_to_directory(int index);

void print_file(const char* file, bool highlight);
void print_folder(const char* folder, bool highlight);
void print_footer();
void print_element(struct stat fileStat, const char* folder, bool value);

const int FILE_COLOR = 1;
const int FOLDER_COLOR = 2;
const int HIGHLIGHT_FILE_COLOR = 3;
const int HIGHLIGHT_FOLDER_COLOR = 4;

int total_elements = 0;
int max_rows = 0;

int current_index = 0;
char* path = ".";


int main(int argc, char* argv[]) {

  if (argc == 2) {
    path = argv[1];
  }    

  init_ncurses();
  get_terminal_rows();

  chdir(path);
  get_files(path);
  refresh();
  get_input();

  endwin();
  return 0;
}


void init_ncurses() {
  initscr();
  start_color();
  use_default_colors();
  init_pair(FILE_COLOR, COLOR_RED, -1); 
  init_pair(FOLDER_COLOR, COLOR_BLUE, -1);
  init_pair(HIGHLIGHT_FILE_COLOR, COLOR_RED, COLOR_WHITE);
  init_pair(HIGHLIGHT_FOLDER_COLOR, COLOR_BLUE, COLOR_WHITE);

  clear();
  curs_set(0);
  cbreak();
  keypad(stdscr, TRUE);
  noecho();
}


void get_input() {
  struct timespec sleep_time;
  sleep_time.tv_sec = 1;
  int ch;

  while (1) {
    ch = getch();
    if (ch != ERR) {
      switch (ch) {
        case KEY_LEFT:
        case 'a':
        case 'A':
          chdir("..");
          getcwd(path, PATH_MAX); 
          current_index = 0;
          break;
        case KEY_UP:
        case 'w':
        case 'W':
          current_index--;
          current_index = current_index % total_elements;
          if (current_index < 0)
            current_index = total_elements - current_index - 2;
          break;
        case KEY_DOWN:
        case 's':
        case 'S':
          current_index++;
          current_index = current_index % total_elements;
          break;
        case KEY_RIGHT:
        case 'd':
        case 'D':
          go_to_directory(current_index);
          break;
        case 'x':
        case 'X':
          endwin();
          exit(0);
          break;
        default:
          break;
      }
      clear();
      get_files(path);
    }
    nanosleep(&sleep_time, NULL);
  }
}


void get_terminal_rows() {
  max_rows = getmaxy(stdscr) - 6;
}


void get_files(char* path) {
  total_elements = 0;
  DIR *d;
  struct dirent *dir;
  d = opendir(path);
  int i = 0;

  char current_path[PATH_MAX];
  if (getcwd(current_path, sizeof(current_path)) != NULL) {
    printw(" -----", current_path);
    printw(" %s ", current_path);
    printw(" -----\n\n", current_path);
  }

  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
        continue;
      check_file_stats(path, dir->d_name, i++);
    }
    closedir(d);
    print_footer();
  } else {
    perror("Error opening the directory");
  }
}


void check_file_stats(const char* path, const char* folder, int index) {
    char fullpath[PATH_MAX];
    struct stat fileStat;

    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, folder);

    if (stat(fullpath, &fileStat) == 0) {
      bool value = (current_index == index);
      int index_range = current_index - max_rows;

      if ((index_range - index) < max_rows) {
        if (current_index + 5 < max_rows && index < max_rows) {
          print_element(fileStat, folder, value);
        } else if ( index > (current_index + 5 - max_rows) && index < (current_index + 6)){
          print_element(fileStat, folder, value);
        }
      }
      total_elements++;
    } else {
      perror("Error while obtaining file information");
    }
}


void go_to_directory(int index) {
  DIR *d;
  struct dirent *dir;
  d = opendir(path);
  int i = 0;

  if (d) {
      while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) {
            continue;
        }
        if (i == index) {
          char newPath[PATH_MAX];
          snprintf(newPath, sizeof(newPath), "%s/%s", path, dir->d_name);

          if (chdir(newPath) == 0) {
            getcwd(path, PATH_MAX);
            current_index = 0;
            get_files(path);
          } else {
            perror("Error changing to the new directory");
          }
          break;
        }
        i++;
      }
      closedir(d);
  } else {
      perror("Error opening the directory");
  }
}


void print_element(struct stat fileStat, const char* folder, bool value) {
   if (S_ISREG(fileStat.st_mode)) {
      print_file(folder, value);
    } else if (S_ISDIR(fileStat.st_mode)) {
      print_folder(folder, value);
    } else {
      printw("Unknown type: %s\n", folder);
    }
}


void print_file(const char* file, bool highlight) {
  if (highlight) {
    attron(COLOR_PAIR(HIGHLIGHT_FILE_COLOR) | A_REVERSE);
    printw("   %s\n", file);
    attroff(COLOR_PAIR(HIGHLIGHT_FILE_COLOR) | A_REVERSE); 
    return;
  }

  attron(COLOR_PAIR(FILE_COLOR));
  printw("   %s\n", file);
  attroff(COLOR_PAIR(FILE_COLOR));
}

void print_folder(const char* folder, bool highlight) {
  if (!highlight) {
    attron(COLOR_PAIR(FOLDER_COLOR));
    printw("   %s\n", folder);
    attroff(COLOR_PAIR(FOLDER_COLOR));
    return;
  }

  attron(COLOR_PAIR(HIGHLIGHT_FOLDER_COLOR) | A_REVERSE);
  printw(" > %s\n", folder);
  attroff(COLOR_PAIR(HIGHLIGHT_FOLDER_COLOR) | A_REVERSE);
}

void print_footer() {
    get_terminal_rows();
    mvprintw(max_rows + 3, 0, "-------------------");
    mvprintw(max_rows + 4, 0, " Total elements: %d | [X] Exit ", total_elements);
}



