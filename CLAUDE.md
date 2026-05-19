# ILS - Interactive List Command

Terminal file explorer written in C with ncurses. Allows interactive navigation of directory structures with keyboard controls.

## Build & Run

```bash
# Dependencies (ncursesw required)
apt-get install ncurses-dev    # Debian/Ubuntu
pacman -S ncurses              # Arch

# Build
make                           # Compile only
make run                       # Compile and run
make clean                     # Remove build artifacts

# Run
./ils                          # Current directory
./ils /path/to/dir             # Specific directory
```

## Architecture

```
main.c              → Entry point, event loop, input handling
file_utils.c/h      → Directory reading, navigation, filtering, file metadata
screen_manager.c/h  → ncurses init/cleanup, color pairs, layout, UTF-8 printing
ui_renderer.c/h     → Header, file list, scrollbar, footer rendering
```

Layered design: main.c orchestrates via `app_state`, file_utils handles data, ui_renderer handles display, screen_manager abstracts ncurses.

## Key Data Structures

- `file_item` (file_utils.h) — filename, stat info, directory/hidden/symlink flags, permissions, owner, group, link target.
- `app_state` (file_utils.h) — centralized application state: path, items, visible indices, cursor, filter, layout dimensions, flags.

## Keyboard Controls

- `↑/↓/w/s/k/j` — Navigate list
- `←/a` — Parent directory
- `→/Enter/l/d` — Enter directory
- `PgUp/PgDn` — Page navigation
- `Home/g` / `End/G` — Jump to first/last
- `h/H` — Toggle hidden files
- `i` — Toggle detailed info columns
- `/` — Incremental search/filter (Esc to clear, Enter to keep)
- `c` — Quit and cd into current directory (requires shell wrapper)
- `q/x` — Exit

## CD on Exit

Press `c` to quit ils and have your shell cd into the directory you were browsing. Requires sourcing the shell wrapper:

```bash
# Add to .bashrc / .zshrc:
source /path/to/ils.sh
```

How it works: `c` writes the current path to `$TMPDIR/ils_lastdir`, then the shell function reads it and runs `cd`.

## Rendering

- Uses `erase()` + `refresh()` for flicker-free differential updates
- Fixed layout: 3-line header, 2-line footer, rest for file list
- Adaptive info columns based on terminal width
- Proportional scrollbar on right edge when content overflows
- Color scheme: yellow=folders, green=files, magenta=hidden, cyan=symlinks, black-on-cyan=selected

## Build Details

- Compiler: gcc with `-Wall -Wextra -O2`
- Links against: `ncursesw`
- Objects go to `build/`, executable is `ils` in project root
- Makefile includes header dependency tracking

## Style

- No test suite
- No CI/CD
