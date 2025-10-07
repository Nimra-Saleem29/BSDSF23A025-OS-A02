# 🧾 REPORT – Operating Systems Programming Assignment 02  

## 👩‍💻 Student Information  
| Name | Roll No | Course |
|------|----------|--------|
| **Nimra Saleem** | **BSDSF23A025** | **Operating Systems (Programming Assignment 02)** |

---

## 🧩 Feature 1 — Initial Setup and Build  

### ✅ Tasks Completed  
- Created GitHub repository named **`BSDSF23A025-OS-A02`**  
- Cloned instructor’s repository **`arifpucit/OS-Codes`** and copied starter file **`lsv1.0.0.c`** into `src/`  
- Created project directories: `src/`, `obj/`, `bin/`, `man/`  
- Wrote a **Makefile** to compile the project using `make` and verified binary `bin/ls`  
- Successfully built and executed the starter code  
- Committed and pushed **Feature-1 setup** to GitHub main branch  

### 🔍 Verification  
| Step | Command | Result |
|------|----------|--------|
| **Build** | `make` | Project compiled successfully |
| **Binary Location** | `bin/ls` | Binary generated |
| **Run** | `./bin/ls` | Starter `ls` version running successfully |

### 🧠 Learning Outcome  
- Learned to structure a Linux C project with `src`, `obj`, and `bin`  
- Used **Makefile** for compilation automation  
- Managed repositories using **Git and GitHub**

---

## 🧠 Report Questions  

### 🧩 Feature 2 — `stat()` vs `lstat()` & `st_mode`  

#### 📝 Q1: Difference between `stat()` and `lstat()`  
| Function | Description |
|-----------|-------------|
| **`stat()`** | Follows symbolic links, returns metadata of the **target file** |
| **`lstat()`** | Does **not** follow symbolic links, returns metadata of the **link itself** |

📌 In `ls -l`, use `lstat()` to display link info (`lrwxrwxrwx → target`) instead of the target file.

#### 📝 Q2: Using `st_mode` to extract file type and permissions  

**File Type Macros**
```c
S_ISDIR(st_mode);  // Directory  
S_ISREG(st_mode);  // Regular file  
S_ISLNK(st_mode);  // Symbolic link  

Permission Bits

(st_mode & S_IRUSR) ? 'r' : '-';  // Owner read  
(st_mode & S_IWUSR) ? 'w' : '-';  // Owner write  
(st_mode & S_IXUSR) ? 'x' : '-';  // Owner execute  


Combining these macros allows ls -l to display file types and permissions accurately.

🧩 Feature 3 — ls-v1.2.0: Column Display (Down Then Across)
🧠 Q1: Logic for “down then across”

Print filenames vertically first, then horizontally.

A single loop cannot handle this.

Steps:

Store all filenames with readdir()

Find the longest filename → column width

Use ioctl() to get terminal width → determine number of columns

Compute rows:

nrows = (total_files + ncols - 1) / ncols;


Print row by row:

names[r], names[r + nrows], names[r + 2*nrows], ...

🧠 Q2: Purpose of ioctl()

Used to retrieve terminal size dynamically:

struct winsize ws;
ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);


Benefit: Adjusts output to fit terminal width.
If fixed width (e.g., 80 columns) is used:

Small terminals → text wraps

Large terminals → wasted space

🧩 Feature 4 — ls-v1.3.0: Horizontal Column (-x)
🧮 Q1: Complexity Comparison
Mode	Complexity	Reason
Down-then-across	More complex	Requires pre-calculating rows/columns and index mapping
Horizontal (-x)	Simpler	Prints sequentially and wraps when terminal width reached
⚙️ Q2: Display Modes Management
typedef enum { MODE_DEFAULT, MODE_LONG, MODE_HORIZONTAL } display_mode_t;


Mode Handling:

if (mode == MODE_LONG)
    print_long_listing();
else if (mode == MODE_HORIZONTAL)
    print_horizontal();
else
    print_default();

🧩 Feature 5 — ls-v1.4.0: Alphabetical Sort
🧠 Q1: Why read all entries before sorting? Drawbacks?

Sorting requires random access to compare filenames.

readdir() is sequential → must store in memory first.

Steps:

Read entries → store in array

Sort using qsort()

Display sorted array

Drawbacks:

High memory usage for large directories

Slower performance: O(n log n)

Possible memory exhaustion

🧠 Q2: qsort() Comparison Function
int cmpstr(const void *a, const void *b) {
    const char * const *sa = (const char * const *)a;
    const char * const *sb = (const char * const *)b;
    return strcmp(*sa, *sb);
}


✅ const void* allows generic sorting of any data type.

🧩 Feature 6 — ls-v1.5.0: Colorized Output Based on File Type
🎨 Q1: ANSI Escape Codes for Color

Format → \033[<attribute>;<foreground>;<background>m

Example (Green Text):

printf("\033[0;32mThis is green!\033[0m\n");

Code	Meaning
0	Normal style
32	Green
\033[0m	Reset color
⚙️ Q2: Executable Permission Bits
Permission	Constant	Description
Owner	S_IXUSR	Executable by owner
Group	S_IXGRP	Executable by group
Others	S_IXOTH	Executable by anyone

Check & Color Executables:

if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
    printf("\033[1;32m%s\033[0m\n", filename);

🧩 Feature 7 — ls-v1.6.0: Recursive Listing (-R)
🔁 Q1: Base Case in Recursion

The base case prevents infinite recursion:

if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
    continue;


Directories that cannot be opened (permission denied) also act as base cases.

🧠 Q2: Check if a File is Executable
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

✨ End of Report
Author: Nimra Saleem — BSDSF23A025
Course: Operating Systems


