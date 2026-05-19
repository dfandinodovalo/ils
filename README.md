# ils - Interactive List Command

**ils** is a terminal file explorer written in C with ncurses. It allows interactive navigation of directory structures with keyboard controls, file previews, incremental search, and the ability to `cd` into the browsed directory on exit.

## Demo

<p align="center">
  <img src="https://github.com/dfandinodovalo/ils/blob/main/docs/ils.gif?raw=true" alt="ils demo">
</p>

## Features

- **Interactive navigation** — browse directories with arrow keys, WASD, or vim-style bindings
- **File preview** — press `p` to preview file contents with scroll support
- **Incremental search** — press `/` to filter files in real-time
- **Detailed info** — toggle file metadata columns (permissions, size, date) with `i`
- **Hidden files** — toggle visibility with `h`
- **CD on exit** — press `c` to quit and have your shell cd into the directory you were browsing
- **Symlink support** — symlinks are displayed with their targets
- **Color-coded entries** — folders, files, hidden items, and symlinks each have distinct colors

## Dependencies

It is necessary to install **ncursesw**:

```bash
# Debian/Ubuntu
apt-get install ncurses-dev

# Arch Linux
pacman -S ncurses
```

## Build & Run

```bash
make          # Compile
make run      # Compile and run
make clean    # Remove build artifacts

./ils                  # Browse current directory
./ils /path/to/dir     # Browse specific directory
```

## Keyboard Controls

| Key | Action |
|---|---|
| `↑` / `w` / `k` | Move up |
| `↓` / `s` / `j` | Move down |
| `←` / `a` | Parent directory |
| `→` / `Enter` / `l` / `d` | Enter directory |
| `PgUp` / `PgDn` | Page navigation |
| `Home` / `g` | Jump to first |
| `End` / `G` | Jump to last |
| `h` / `H` | Toggle hidden files |
| `i` | Toggle detailed info columns |
| `p` | Preview file content (Esc/p/q to close) |
| `/` | Incremental search/filter |
| `c` | Quit and cd into current directory |
| `q` / `x` | Quit |

## CD on Exit

Press `c` inside ils to quit and have your shell automatically change to the directory you were browsing. This requires sourcing the shell wrapper function.

### Setup

Add the following to your `.bashrc` or `.zshrc`:

```bash
ILS_BIN="/path/to/ils"          # path to the ils binary
source /path/to/ils.sh
```

### How it works

1. When you press `c`, the ils binary writes the current path to a temp file (`$TMPDIR/ils_lastdir`)
2. The shell wrapper function reads the temp file after ils exits
3. The wrapper runs `cd` into that directory and displays the new path

> **Note:** A plain alias (`alias ils=...`) will **not** work for cd-on-exit — a shell function is required because aliases cannot change the working directory of the parent shell. Make sure you are using `source ils.sh` instead of an alias.