# 🧮 Custom `ls` Utility (Operating Systems Assignment)

### 📘 Instructor: **Sir Muhammad Arif Butt**  
### 👩‍💻 Student: **Nimra Saleem — BSDSF23A025**

---

## 🔰 Overview

This project is a simplified, educational re-implementation of the **Linux `ls` command**, developed step-by-step across multiple feature versions (**v1.0.0 → v1.5.0**).  
Each version introduces new **Operating Systems** concepts such as:

- File metadata and permissions  
- System calls (`opendir`, `readdir`, `stat`)  
- Command-line argument parsing  
- Dynamic memory allocation  
- Terminal formatting using ANSI escape codes  

---

## ⚙️ Build Instructions

To compile and run the project in **Kali Linux**, follow these commands:

```bash
make clean
make

# Run executables
./bin/ls           # Default view (Down-then-Across)
./bin/ls -l        # Long Listing Format
./bin/ls -x        # Horizontal Display

## 🧩 Feature Timeline

Below is the development timeline of each feature version, showing the gradual enhancement of the custom `ls` utility.

| **Version** | **Feature Name** | **Key Concepts** | **Description** |
|--------------|------------------|------------------|------------------|
| **v1.0.0** | Basic Directory Listing | `opendir()`, `readdir()` | Displays files and directories in the current working directory (default mode). |
| **v1.1.0** | Long Listing Format (`-l`) | `stat()`, File Metadata | Displays file permissions, ownership, size, and modification time. |
| **v1.2.0** | Sort and Filter | String Manipulation | Sorts entries alphabetically and filters hidden files (`.` prefix). |
| **v1.3.0** | Horizontal Listing (`-x`) | Output Formatting | Displays directory entries in a horizontal, across-then-down layout. |
| **v1.4.0** | Command-Line Arguments | `argc`, `argv` Handling | Enables parsing of multiple options like `-l`, `-x`, and directory names. |
| **v1.5.0** | Colorized Output | ANSI Escape Codes, Permissions | Adds color formatting based on file type (directory, executable, etc.). |
| **v1.6.0** | Recursive Listing (`-R`) | Directory Traversal, Recursion | Lists files and directories recursively, showing subdirectory contents. |
