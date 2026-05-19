<h1 align="center">ils</h1>
<p align="center"><b>A fast, interactive file explorer for the terminal</b></p>
<p align="center">Navigate directories, preview files, filter instantly — all from your terminal. Written in C with ncurses. Zero runtime dependencies.</p>

<p align="center">
  <img src="https://github.com/dfandinodovalo/ils/blob/main/docs/ils.gif?raw=true" alt="ils demo" width="720">
</p>

## Why ils?

Most terminal file managers are either too heavy or too minimal. **ils** sits in the sweet spot:

- **Fast** — pure C, compiles in under a second, starts instantly
- **Lightweight** — single binary, only depends on ncursesw (already on most systems)
- **Intuitive** — arrow keys, vim bindings, or WASD — pick your style
- **cd on exit** — press `c` and your shell lands in the directory you were browsing

## Features

| Feature | Description |
|---|---|
| 🔍 **Incremental search** | Press `/` to filter files in real-time as you type |
| 📄 **File preview** | Press `p` to preview file contents with scroll support |
| 📊 **Detailed info** | Toggle permissions, size, and dates with `i` |
| 👻 **Hidden files** | Toggle visibility with `h` |
| 🔗 **Symlink aware** | Displays symlink targets inline |
| 🎨 **Color-coded** | Folders, files, hidden items, and symlinks each have distinct colors |
| 📂 **cd on exit** | Quit and land in the directory you were browsing |

## Installation

### Build from source

Requires `gcc` and `ncursesw`:

```bash
# Install ncursesw
sudo apt-get install libncursesw5-dev   # Debian / Ubuntu
sudo pacman -S ncurses                  # Arch Linux

# Build
git clone https://github.com/dfandinodovalo/ils.git
cd ils
make
```

### Run

```bash
./ils                  # Browse current directory
./ils /path/to/dir     # Browse a specific directory
```

> **Tip:** Copy the binary to somewhere in your `$PATH` (e.g. `sudo cp ils /usr/local/bin/`) to use it from anywhere.

## Keyboard Controls

| Key | Action |
|---|---|
| `↑` `k` `w` | Move up |
| `↓` `j` `s` | Move down |
| `←` `a` | Go to parent directory |
| `→` `Enter` `l` `d` | Enter directory |
| `PgUp` / `PgDn` | Page up / down |
| `Home` `g` / `End` `G` | Jump to first / last |
| `h` | Toggle hidden files |
| `i` | Toggle detailed info columns |
| `p` | Preview file (scroll with arrows, close with `Esc`/`q`) |
| `/` | Incremental search (Esc to clear, Enter to keep) |
| `c` | Quit and cd into current directory |
| `q` `x` | Quit |

## Shell Integration (cd on exit)

Press `c` inside ils to quit and have your shell change to the directory you were browsing.

### Setup

Add to your `.bashrc` or `.zshrc`:

```bash
ILS_BIN="/path/to/ils"
source /path/to/ils.sh
```

Then use `ils` as usual. Press `c` to quit + cd, or `q` to quit normally.

### How it works

When you press `c`, ils writes the current path to a temp file. The shell wrapper reads it after ils exits and runs `cd`. This requires a shell function (not an alias) because only functions can change the parent shell's working directory.

## Project Structure

```
main.c              → Entry point, event loop, input handling
file_utils.c/h      → Directory reading, navigation, filtering
screen_manager.c/h  → ncurses init/cleanup, color pairs, layout
ui_renderer.c/h     → Header, file list, scrollbar, footer rendering
ils.sh              → Shell wrapper for cd-on-exit
```

## Contributing

Contributions are welcome! Feel free to open issues or submit pull requests.

## License

MIT