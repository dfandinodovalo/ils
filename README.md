# ils - Interactive List Command

"ils" is a command-line utility designed to provide iterative exploration of directory contents within the user's working environment. It allows users to efficiently navigate through the folder structure and gain insights into files and subdirectories present in each directory.

## Features

- **Iterative Exploration:** Progressively navigate through directories, facilitating understanding of folder hierarchy.
- **Efficient Navigation:** Use arrow keys and 'WASD' shortcuts for seamless movement between elements and directories.
- **Streamlined Actions:** Options to return to parent directory, open directories, and exit the program efficiently.
- **Intuitive User Interface:** Text-based interface offers straightforward user experience for interacting with directories and files.



# Dependencies 
It is necessary to install __ncurses__:

- Install in Debian/Ubuntu Linux

```
  apt-get install ncurses-dev
```

- Install in Arch Linux distributions:
```
 pacman -S ncurses
```

### Other dependecies:

- dirent.h
- string.h
- sys/stat.h
- stdlib.h
- unistd.h
- time.h
- gcc

# Use

1. When you start the program, you will see a list of files and directories in the current directory. Files will be displayed in red, and directories in blue.

2. Use the following keys to navigate:

   - ⬆️ / w / W: Move the cursor upwards in the list.
   - ⬇️ / s / S: Move the cursor downwards in the list.
   - ⬅️ / a / A: Navigate to the parent directory.
   - ➡️ / d / D: Enter the selected directory.
   - To exit the program, press the 'x' or 'X' key. This will close the application and return you to the command line.


# Notes

- The highlighting option is enabled for the currently selected file or directory.
- At the bottom of the screen, the total number of elements in the current directory is displayed.
- (<span style="color:orange">It doesn't work properly</span>) If you want to explore another directory, you can provide the path as an argument when running the program.

```
./ils /path/to/another/directory
```


# Execution

Using the Makefile gives you the option to compile and execute, or just to compile the program. You can compile "ils" using the provided [Makefile](https://github.com/dfandinodovalo/ils/blob/main/Makefile).

## Compile and run:

```
make run
```

## Compile:

```
make
```

Or, you can manually compile it using gcc:

```
gcc ils.c -o ils -lncurses
```

## Execute:


```
./ils
```
<br>
<br>

### Example:

<p align="center">
  <img src="https://github.com/dfandinodovalo/ils/blob/main/docs/image_1.png?raw=true" alt="ils running in /home/user/Desktop">
  <img src="https://github.com/dfandinodovalo/ils/blob/main/docs/image_2.png?raw=true" alt="ils running in /home/user/Desktop/src">
</p>