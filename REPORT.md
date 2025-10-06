🧾 REPORT – OS Programming Assignment 02
👩‍💻 Student Information

Name: Nimra Saleem

Roll No: BSDSF23A025

Course: Operating Systems (Programming Assignment 02)

🧩 Feature 1: Initial Setup and Build
Tasks Completed

Created GitHub repository named BSDSF23A025-OS-A02.

Cloned instructor’s repository (arifpucit/OS-Codes) and copied starter file lsv1.0.0.c into src/.

Created project directories: src/, obj/, bin/, man/.

Wrote a Makefile to compile the project using make and verified binary bin/ls.

Successfully built and executed the starter code.

Committed and pushed Feature-1 setup to GitHub main branch.

Verification

Build command: make

Binary location: bin/ls

Run command: ./bin/ls

Output: Starter ls version running successfully.

Learning Outcome

Learned to structure a Linux C project with src, obj, and bin.

Used Makefile for compilation automation.

Managed repositories using Git and GitHub.

🧠 Report Questions
Feature 2 — stat() vs lstat() & st_mode
Q1: Difference between stat() and lstat()

stat() follows symbolic links, returning metadata of the target file.

lstat() does not follow symbolic links, returning metadata of the link itself.

In ls -l:

Use lstat() to display link info (lrwxrwxrwx → target) instead of the target file.

Q2: Using st_mode to extract file type and permissions
File Type
S_ISDIR(st_mode);  // Directory
S_ISREG(st_mode);  // Regular file
S_ISLNK(st_mode);  // Symbolic link

Permission Bits
(st_mode & S_IRUSR) ? 'r' : '-';  // Owner read
(st_mode & S_IWUSR) ? 'w' : '-';  // Owner write
(st_mode & S_IXUSR) ? 'x' : '-';  // Owner execute


Combining these macros allows ls -l to display file types and permissions accurately.

🧩 Feature 3 — ls-v1.2.0: Column Display (Down Then Across)
Q1: Logic for “down then across”

Print filenames vertically first, then horizontally.

A single loop cannot handle this.

Steps:

Store all filenames with readdir().

Find the longest filename → column width.

Use ioctl() to get terminal width → determine columns.

Compute rows:

nrows = (total_files + ncols - 1) / ncols


Print row by row:

names[r], names[r + nrows], names[r + 2*nrows], ...

Q2: Purpose of ioctl()

Retrieves terminal size dynamically:

struct winsize ws;
ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);


Adjusts output to fit terminal width.

If fixed width (e.g., 80 columns) is used:

Small terminals → text wraps.

Large terminals → wasted space.

🧩 Feature 4 — ls-v1.3.0: Horizontal Column (-x)
Q1: Complexity comparison

Down-then-across → more complex:

Requires pre-calculating rows and columns.

Index mapping: r + c * nrows.

Horizontal (-x) → simpler:

Print sequentially across screen.

Wrap when reaching terminal width.

Q2: Display modes management

Used enumeration:

typedef enum { MODE_DEFAULT, MODE_LONG, MODE_HORIZONTAL } display_mode_t;


-l → MODE_LONG

-x → MODE_HORIZONTAL

Default → MODE_DEFAULT

After parsing:

if (mode == MODE_LONG) print_long_listing();
else if (mode == MODE_HORIZONTAL) print_horizontal();
else print_default();

🧩 Feature 5 — ls-v1.4.0: Alphabetical Sort
Q1: Why read all entries before sorting? Drawbacks?

Sorting requires random access to compare filenames.

readdir() is sequential → store in memory first.

Steps:

Read entries → store in array.

Sort using qsort().

Display sorted array.

Drawbacks:

High memory usage for millions of files.

Slower performance (O(n log n)).

Potential memory exhaustion.

Q2: qsort() comparison function
int cmpstr(const void *a, const void *b) {
    const char * const *sa = (const char * const *)a;
    const char * const *sb = (const char * const *)b;
    return strcmp(*sa, *sb);
}


const void* → allows generic sorting of any data type.

🧩 Feature 6 — ls-v1.5.0: Colorized Output Based on File Type
Q1: ANSI escape codes for color

Format: \033[<attribute>;<foreground>;<background>m

Example: Green text

printf("\033[0;32mThis is green!\033[0m\n");


0 → normal style

32 → green

\033[0m → reset color

Q2: Executable permission bits
Permission	Constant	Description
Owner	S_IXUSR	Executable by owner
Group	S_IXGRP	Executable by group
Others	S_IXOTH	Executable by anyone

Check and color executables:

if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
    printf("\033[1;32m%s\033[0m\n", filename);

🧩 Feature 7 — ls-v1.6.0: Recursive Listing (-R)
Q1: Base case in recursion

Condition that ends recursion → prevents infinite calls.

In ls -R:

if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
    continue;


Prevents recursion into current (.) and parent (..) directories.

Directories that cannot be opened (permissions denied) also act as base cases.

Q2: Check if a file is executable
#include <stdio.h>
#include <sys/stat.h>

int main() {
    struct stat fileStat;
    const char *filename = "example";

    if (stat(filename, &fileStat) == 0) {
        if (fileStat.st_mode & S_IXUSR)
            printf("Owner can execute\n");
        if (fileStat.st_mode & S_IXGRP)
            printf("Group can execute\n");
        if (fileStat.st_mode & S_IXOTH)
            printf("Others can execute\n");
    } else {
        perror("stat");
    }
    return 0;
}


st_mode stores permission bits.

Bitwise AND with S_IXUSR, S_IXGRP, S_IXOTH identifies executable files.
