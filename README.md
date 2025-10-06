# 🧮 Custom `ls` Utility (Operating Systems Assignment)

### 📘 Instructor: Sir Muhammad Arif Butt  
### 👩‍💻 Student: Nimra Saleem — BSDSF23A025  

---

## 🔰 Overview
This project is a simplified re-implementation of the Linux `ls` command, developed in incremental versions (**v1.0.0 → v1.5.0**).  
Each version demonstrates new **Operating Systems** concepts such as file metadata, system calls, command-line argument parsing, dynamic memory allocation, and terminal I/O formatting.

---

## ⚙️ Build Instructions

To compile and run the project in **Kali Linux**:

```bash
make clean
make
./bin/ls           # Default view (Down-then-Across)
./bin/ls -l        # Long Listing Format
./bin/ls -x        # Horizontal Display# BSDSF23A025-OS-A02

## 🧩 Feature Timeline

---

### 🟢 **Version 1.0.0 — Basic Directory Listing**
- Displays all filenames inside a directory using:
  - `opendir()`
  - `readdir()`
- Single-column output format.
- Introduced directory traversal and file reading basics.

---

### 🟡 **Version 1.1.0 — Long Listing Format (-l)**
- Added `-l` flag for detailed listing similar to real `ls -l`.
- Implemented:
  - `stat()` / `lstat()` → file metadata  
  - `getpwuid()` / `getgrgid()` → user & group names  
  - `ctime()` → modification time formatting  
- Prints permissions, owner, group, size, and modification time.

---

### 🟦 **Version 1.2.0 — Multi-Column Display (Down-Then-Across)**
- Files are displayed in multiple aligned columns.
- Used:
  - `ioctl(TIOCGWINSZ)` → detect terminal width dynamically  
  - Dynamic memory for file storage  
- Output automatically adjusts to terminal resizing.

---

### 🟩 **Version 1.3.0 — Horizontal Column Display (-x)**
- Added `-x` flag.
- Displays files **left-to-right (row-major)** instead of top-down.
- Handles wrapping when terminal width is exceeded.
- Demonstrated display mode control using `getopt()`.

---

### 🟧 **Version 1.4.0 — Alphabetical Sort**
- Default output (and all display modes) now **sorted alphabetically**.
- Used:
  - `qsort()` with a custom comparison function:

    ```c
    int cmpstr(const void *a, const void *b) {
        const char * const *sa = a;
        const char * const *sb = b;
        return strcmp(*sa, *sb);
    }
    ```

- Demonstrates sorting logic using **dynamic memory** and **arrays of pointers**.

---

### 🟥 **Version 1.5.0 — Colorized Output Based on File Type**
- Adds colorized output using **ANSI escape codes** for better visibility.

| File Type | Color | Detection Method |
|------------|--------|-----------------|
| 📘 Directory | Blue | `S_ISDIR()` |
| 💚 Executable | Green | `S_IXUSR`, `S_IXGRP`, `S_IXOTH` |
| 📦 Archives (.tar, .gz, .zip) | Red | `strstr()` extension check |
| 🩷 Symbolic Link | Magenta | `S_ISLNK()` |
| ⚙️ Special Files (device/socket/fifo) | Reverse Video | `S_ISCHR()`, etc. |

- Uses:
  - `lstat()` → detect symbolic links without following them  
  - `isatty()` → enable colors only for terminal output (not pipes)

- Example color macros:
  ```c
  #define RESET   "\033[0m"
  #define BLUE    "\033[0;34m"
  #define GREEN   "\033[0;32m"
  #define RED     "\033[0;31m"
  #define MAGENTA "\033[0;35m"
  #define REVERSE "\033[7m"
