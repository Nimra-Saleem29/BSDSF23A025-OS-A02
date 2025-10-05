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
