#!/bin/sh
# Shell wrapper for ils - enables "cd on exit" (press 'c' inside ils)
#
# Setup: add to your .bashrc / .zshrc:
#   ILS_BIN="/path/to/ils"       # path to the ils binary
#   source /path/to/ils.sh
#
# Then use 'ils' normally. Press 'c' to quit and cd into the browsed directory.
# Press 'q' to quit without changing directory.

ils() {
    _ils_bin="${ILS_BIN:-ils}"
    "$_ils_bin" "$@"
    _ils_lastdir="${TMPDIR:-/tmp}/ils_lastdir"
    if [ -f "$_ils_lastdir" ]; then
        _ils_dir="$(cat "$_ils_lastdir")"
        rm -f "$_ils_lastdir"
        if [ -d "$_ils_dir" ] && [ "$_ils_dir" != "$(pwd)" ]; then
            cd "$_ils_dir" || return
            echo "cd: $_ils_dir"
        fi
    fi
}