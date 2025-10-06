# REPORT – OS Programming Assignment 02
## Feature 1: Initial Setup and Build

**Student Information**
- Name: Nimra Saleem  
- Roll No: BSDSF23A025  
- Course: Operating Systems (Programming Assignment 02)

---

### Tasks Completed

1. Created GitHub repository named `BSDSF23A025-OS-A02`.
2. Cloned instructor’s repository (`arifpucit/OS-Codes`) and copied starter file `lsv1.0.0.c` into `src/`.
3. Created project directories: `src/`, `obj/`, `bin/`, `man/`.
4. Wrote a `Makefile` to compile the project using `make` and verified binary `bin/ls`.
5. Successfully built and executed the starter code.
6. Committed and pushed Feature-1 setup to GitHub main branch.

---

### Verification

- **Build command:** `make`
- **Binary location:** `bin/ls`
- **Run command:** `./bin/ls`
- **Output:** Starter ls version running successfully.

---

### Learning Outcome

I learned how to:
- Structure a Linux C project with `src`, `obj`, and `bin`.
- Use `Makefile` for compilation automation.
- Manage repositories using Git and GitHub.

---
## 🧠 Report Questions

---

### • What is the crucial difference between the stat() and lstat() system calls? 
In the context of the ls command, when is it more appropriate to use lstat()?

The `stat()` system call follows symbolic links — if the path points to a symlink, it returns the 
metadata of the **target file** that the link points to.  

The `lstat()` system call does **not** follow symbolic links — it returns the metadata of the **link itself**, 
including its type (`'l'`), size of the link path, and other properties.  

In the context of the `ls` command, `lstat()` is more appropriate because `ls -l` must display 
information **about the link itself** (e.g., showing `lrwxrwxrwx` and `-> target`), 
not about the file to which it points.

---

### • The st_mode field in struct stat is an integer that contains both the file type (e.g., regular file, directory) and the permission bits. 
Explain how you can use bitwise operators (like &) and predefined macros (like S_IFDIR or S_IRUSR) to extract this information.

The `st_mode` field in the `struct stat` structure encodes both the **file type** and the **permission bits**.  
Each bit in this integer corresponds to a specific property of the file.

---

#### 🔹 File Type:
The file type bits can be extracted using predefined macros such as:  
```c
S_ISDIR(st_mode)   // checks if directory
S_ISREG(st_mode)   // checks if regular file
S_ISLNK(st_mode)   // checks if symbolic link
These macros internally perform a bitwise AND with the mask S_IFMT and compare it
with constants like S_IFDIR or S_IFREG.

Example:

if ((st_mode & S_IFMT) == S_IFDIR)
    printf("This is a directory");

🔹 Permission Bits:

The lower bits of st_mode represent the read, write, and execute permissions for the owner, group, and others.

Example:

(st_mode & S_IRUSR) ? 'r' : '-';   // owner read
(st_mode & S_IWUSR) ? 'w' : '-';   // owner write
(st_mode & S_IXUSR) ? 'x' : '-';   // owner execute


Special bits like S_ISUID, S_ISGID, and S_ISVTX control set-user-ID, set-group-ID,
and sticky bit behavior, and they modify the x position into s, S, t, or T.

By combining these macros with bitwise operators (&), a program can accurately determine the file’s
type and permissions to display them in the ls -l format.


🧩 Feature-3: ls-v1.2.0 – Column Display (Down Then Across)
Q1. Explain the general logic for printing items in a "down then across" columnar format. Why is a simple single loop through the list of filenames insufficient for this task?

Answer:
In a “down-then-across” format, the filenames are arranged vertically first, then move horizontally to the next column.
A simple single for loop prints all items in a single row or column, but it cannot automatically format the output into multiple aligned columns.
To achieve proper columnar display, we must know:

The terminal width (in characters)

The longest filename length

The total number of filenames

The program performs the following steps:

Store all filenames in an array using readdir().

Find the longest filename length to decide how wide each column should be.

Get terminal width using ioctl() to calculate how many columns fit per row.

Compute rows using:

𝑛
𝑟
𝑜
𝑤
𝑠
=
𝑡
𝑜
𝑡
𝑎
𝑙
_
𝑓
𝑖
𝑙
𝑒
𝑠
+
𝑛
𝑐
𝑜
𝑙
𝑠
−
1
𝑛
𝑐
𝑜
𝑙
𝑠
nrows=
ncols
total_files+ncols−1
	​


Print row by row:
For each row r, print:

names[r], names[r + nrows], names[r + 2*nrows], ...


until all columns are printed.

This approach ensures files are displayed neatly in a grid-like structure, filling each column from top to bottom, then moving across — exactly like the real Linux ls.

Q2. What is the purpose of the ioctl() system call in this context? What would be the limitations of your program if you only used a fixed-width fallback (e.g., 80 columns) instead of detecting the terminal size?

Answer:
The ioctl() system call is used to query the terminal for its current window size using the TIOCGWINSZ request.
It provides the number of columns (character width) and rows of the terminal, allowing the program to adjust its output layout dynamically.

Purpose:

Automatically detects the terminal width at runtime.

Ensures that filenames are arranged in the maximum possible columns without wrapping or truncation.

Makes the output look clean and professional regardless of terminal resizing.

Example:

struct winsize ws;
if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
    term_width = 80; // fallback
else
    term_width = ws.ws_col;


If only a fixed width (e.g., 80 columns) was used:

On smaller terminals, text may wrap to the next line and misalign.

On larger terminals, there would be excessive empty space.

The output would not adapt to the user’s screen size.

Therefore, ioctl() enables responsive, dynamic output — a key feature for modern command-line utilities.


## Feature-4 — ls-v1.3.0: Horizontal Column (-x)

### Q1. Compare the implementation complexity of the "down then across" (vertical) printing logic versus the "across" (horizontal) printing logic. Which one requires more pre-calculation and why?

**Answer:**  
The **"down then across"** layout requires more pre-calculation than the **horizontal (across)** layout. For the down-then-across method we must:
1. Read and store *all* filenames.
2. Determine the longest name to compute a fixed column width.
3. Obtain the terminal width and compute how many columns (`ncols`) fit.
4. Compute the required number of rows (`nrows = ceil(total_files / ncols)`).
5. Use the formula `index = r + c * nrows` to select entries for each printed row.

This pre-calculation is necessary because the mapping from linear array index to printed position is not sequential; we must compute row/column counts before printing.

The **horizontal** layout (row-major, `-x`) is simpler: after computing the column width and terminal width, we iterate the file list sequentially, printing each name left-to-right, keeping track of the current horizontal position and wrapping to a new line when the next entry would overflow. This requires much less precomputation — only the column width and terminal width are needed — and the algorithm is a simple single pass.

Therefore, **down-then-across requires more pre-calculation** because it needs rows/columns determination and index mapping before printing.

---

### Q2. Describe the strategy you used in your code to manage the different display modes (-l, -x, default).

**Answer:**  
I used a small enumeration `display_mode_t` with values `MODE_DEFAULT`, `MODE_LONG`, and `MODE_HORIZONTAL`. The `getopt()` loop recognizes options `-l` and `-x`:

- If `-l` is present, the mode is set to `MODE_LONG` (long-listing) — this takes precedence.
- If `-x` is present and `-l` is not set, the mode is set to `MODE_HORIZONTAL`.
- If neither is present, the mode remains `MODE_DEFAULT` (down-then-across).

After argument parsing, the program `lstat()`s the target path:
- If mode is `MODE_LONG` and the path is a directory, call `print_long_listing()`; if it is a single file, call `print_long_single()`.
- If mode is `MODE_HORIZONTAL`, call `print_horizontal_path()` (which handles files vs directories).
- If mode is `MODE_DEFAULT`, call `print_simple()` (down-then-across).

This strategy centralizes mode selection in one place, keeps precedence simple (long-listing highest), and makes adding future modes straightforward.



## 🧩 Feature-5: ls-v1.4.0 — Alphabetical Sort

### 🔹 Question 1:
**Why is it necessary to read all directory entries into memory before you can sort them? What are the potential drawbacks of this approach for directories containing millions of files?**

### 🧠 Answer:
To perform sorting, the program must be able to **compare any two filenames freely** — meaning random access is needed.  
This is only possible when all filenames are **stored in memory (for example, in a `char**` array)**.  

If we tried to sort files one by one while reading them directly from the filesystem stream, it would be impossible because:
- Directory streams (`readdir()`) return entries sequentially.
- Sorting requires re-ordering based on comparisons between *all* elements.

Therefore, the program first:
1. Reads all directory entries into a dynamically allocated array of strings.  
2. Then calls the C standard library function `qsort()` to reorder the array alphabetically.  
3. Finally, the sorted array is passed to the display functions (default, `-l`, and `-x`).

#### ⚠️ Drawbacks:
While this approach works perfectly for small and medium directories, it can cause:
- **High memory usage** — each filename takes heap memory, and thousands or millions of files will consume significant RAM.
- **Performance issues** — reading, storing, and sorting millions of entries is computationally expensive (O(n log n)).
- **Potential crash** — if available memory is exhausted due to too many directory entries.

---

### 🔹 Question 2:
**Explain the purpose and signature of the comparison function required by `qsort()`. How does it work, and why must it take `const void*` arguments?**

### 🧠 Answer:
The `qsort()` function in C is a **generic sorting utility**, which can sort any kind of array — integers, floats, strings, or custom structures.  
To achieve this flexibility, it doesn’t know the type of the array elements.  
That’s why it requires a **custom comparison function** with the signature:

```c
int cmpstr(const void *a, const void *b);
The parameters are typed as const void* so that qsort() can pass pointers to any data type.
Inside the comparator, these pointers are cast back to the correct type before comparison.
For strings, the comparator looks like this:

int cmpstr(const void *a, const void *b) {
    const char * const *sa = (const char * const *)a;
    const char * const *sb = (const char * const *)b;
    return strcmp(*sa, *sb);
}


:

📘 Report: Feature-6 (ls-v1.5.0 – Colorized Output Based on File Type)
❓ Question 1:

How do ANSI escape codes work to produce color in a standard Linux terminal? Show the specific code sequence for printing text in green.

💡 Answer:

ANSI escape codes are special sequences of characters that control how text appears in the terminal — including colors, boldness, and styles.
They begin with the escape character \033 (or \x1b), followed by [ and a set of numeric parameters, ending with the letter m.

General format:

\033[<attribute>;<foreground>;<background>m


attribute: Controls style (e.g., 0 = normal, 1 = bold)

foreground: Sets text color (30–37 = normal colors, 90–97 = bright colors)

background: Sets background color (40–47 = normal, 100–107 = bright)

Example: Printing green text

printf("\033[0;32mThis text is green!\033[0m\n");


Explanation:

\033[0;32m → Turns on green text (0 = normal style, 32 = green color)

\033[0m → Resets color to default after printing

❓ Question 2:

To color an executable file, you need to check its permission bits. Explain which bits in the st_mode field you need to check to determine if a file is executable by the owner, group, or others.

💡 Answer:

Every file in Linux has a set of permission bits stored in its metadata, accessible through the st_mode field (obtained using the stat() system call).
To check if a file is executable, we look at the execute bits for the owner, group, and others.

Permission Type	Constant	Binary Mask	Description
Owner Execute	S_IXUSR	0100	Executable by file owner
Group Execute	S_IXGRP	0010	Executable by group members
Others Execute	S_IXOTH	0001	Executable by anyone

Example Check in C:

if (st.st_mode & S_IXUSR || st.st_mode & S_IXGRP || st.st_mode & S_IXOTH)
    printf("\033[1;32m%s\033[0m\n", filename); // Print executable files in green


Explanation:

S_IXUSR, S_IXGRP, and S_IXOTH are macros that represent the execute permission bits.

If any of these bits are set, the file is executable.

The code above prints executable filenames in green using ANSI color codes.


## 🧩 Feature-7: ls-v1.6.0 — Recursive Listing (-R)

### **Question 1:**
In a recursive function, what is a "base case"?  
In the context of your recursive `ls`, what is the base case that stops the recursion from continuing forever?

### **Answer:**
A **base case** in recursion is the condition that stops further recursive calls.  
Without a base case, the function would call itself indefinitely, causing a stack overflow.  

In the context of the recursive `ls`:
- The base case occurs when the directory has **no subdirectories** or when it encounters `"."` or `".."`.  
- The program checks:
  ```c
  if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;
This ensures that the function does not recurse into the current or parent directories, preventing infinite recursion.

Additionally, directories that cannot be opened (e.g., permission denied) are gracefully skipped, serving as another natural base case.Q2: To color an executable file, you need to check its permission bits. Explain which bits in the st_mode field you need to check to determine if a file is executable by the owner, group, or others.

Answer:
To determine if a file is executable, we use the st_mode field of the stat structure.
The following bits indicate executable permissions:

S_IXUSR → executable by owner

S_IXGRP → executable by group

S_IXOTH → executable by others

If any of these bits are set, the file is executable by that category.

Example Code:

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


Explanation:
The st_mode field stores file type and permission bits.
By using bitwise AND (&) with these macros, we can check whether the executable permission is granted for the owner, group, or others.
