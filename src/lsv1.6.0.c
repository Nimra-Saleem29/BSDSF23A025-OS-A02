/* src/lsv1.6.0.c
 * ls v1.6.0 - colorized output based on file type, with recursive -R option
 *
 * Minimally modified from v1.5.0: added do_ls() and -R handling.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>
#include <libgen.h>
#include <limits.h>
#include <errno.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/* ANSI color codes */
#define RESET   "\033[0m"
#define BLUE    "\033[0;34m"
#define GREEN   "\033[0;32m"
#define RED     "\033[0;31m"
#define MAGENTA "\033[0;35m"
#define REVERSE "\033[7m"

static int color_enabled = 0;

/* ---------------- helpers ---------------- */

int cmpstr_qsort(const void *a, const void *b) {
    const char * const *sa = (const char * const *)a;
    const char * const *sb = (const char * const *)b;
    return strcmp(*sa, *sb);
}

int get_term_width(void) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) return 80;
    return ws.ws_col ? ws.ws_col : 80;
}

int ends_with(const char *name, const char *suffix) {
    if (!name || !suffix) return 0;
    size_t nlen = strlen(name), slen = strlen(suffix);
    if (slen > nlen) return 0;
    return (strcmp(name + nlen - slen, suffix) == 0);
}

/* detect archive-like names */
int is_archive_name(const char *name) {
    if (!name) return 0;
    if (ends_with(name, ".tar") || ends_with(name, ".tgz") ||
        ends_with(name, ".tar.gz") || ends_with(name, ".gz") ||
        ends_with(name, ".zip") ) return 1;
    return 0;
}

/* read directory names (skip . and ..) */
char **read_dir_names(const char *dirpath, int *count) {
    DIR *dir = opendir(dirpath);
    if (!dir) { perror("opendir"); *count = 0; return NULL; }
    struct dirent *e;
    int cap = 64, n = 0;
    char **arr = malloc(sizeof(char*) * cap);
    if (!arr) { closedir(dir); *count = 0; return NULL; }
    while ((e = readdir(dir)) != NULL) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
        if (n >= cap) {
            cap *= 2;
            char **tmp = realloc(arr, sizeof(char*) * cap);
            if (!tmp) { perror("realloc"); for (int i=0;i<n;i++) free(arr[i]); free(arr); closedir(dir); *count=0; return NULL; }
            arr = tmp;
        }
        arr[n++] = strdup(e->d_name);
    }
    closedir(dir);
    *count = n;
    return arr;
}

void free_names(char **names, int n) {
    if (!names) return;
    for (int i = 0; i < n; ++i) free(names[i]);
    free(names);
}

/* ---------------- color decision ---------------- */

const char* choose_color_for(const char *fullpath, const char *name) {
    struct stat st;
    if (lstat(fullpath, &st) == -1) {
        return ""; /* fallback: no color */
    }

    if (S_ISLNK(st.st_mode)) return MAGENTA;
    if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode) || S_ISSOCK(st.st_mode) || S_ISFIFO(st.st_mode))
        return REVERSE;
    if (S_ISDIR(st.st_mode)) return BLUE;
    if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) return GREEN;
    if (is_archive_name(name)) return RED;
    return ""; /* no color */
}

void print_colored_padded(const char *dirpath, const char *name, int pad, int last) {
    char full[PATH_MAX];
    if (snprintf(full, sizeof(full), "%s/%s", dirpath, name) >= (int)sizeof(full)) {
        /* path too long: fallback to printing name */
        if (!color_enabled) {
            printf("%s", name);
        } else {
            printf("%s%s%s", "", name, RESET);
        }
        if (!last && pad > 0) printf("%*s", pad - (int)strlen(name), "");
        return;
    }

    const char *start = "";
    if (color_enabled) start = choose_color_for(full, name);

    if (color_enabled && start && start[0] != '\0') {
        printf("%s%s%s", start, name, RESET);
    } else {
        printf("%s", name);
    }

    if (!last && pad > 0) {
        int visible = (int)strlen(name); /* visible length */
        int to_pad = pad - visible;
        if (to_pad > 0) printf("%*s", to_pad, "");
    }
}

/* ---------------- display implementations ---------------- */

void print_columns_down_across(const char *dirpath, char **names, int n) {
    (void)dirpath;  // suppress unused parameter warning
    if (n <= 0) return;
    int maxlen = 0;
    for (int i = 0; i < n; ++i) {
        int L = (int)strlen(names[i]);
        if (L > maxlen) maxlen = L;
    }
    int spacing = 2;
    int colw = maxlen + spacing;
    int termw = get_term_width();
    int ncols = termw / colw;
    if (ncols < 1) ncols = 1;
    int nrows = (n + ncols - 1) / ncols;

    for (int r = 0; r < nrows; ++r) {
        for (int c = 0; c < ncols; ++c) {
            int idx = r + c * nrows;
            if (idx >= n) continue;
            int last = (c == ncols - 1);
            print_colored_padded(".", names[idx], colw, last);
        }
        printf("\n");
    }
}

void print_horizontal(const char *dirpath, char **names, int n) {
    (void)dirpath;  // suppress unused parameter warning
    if (n <= 0) return;
    int maxlen = 0;
    for (int i = 0; i < n; ++i)
        if ((int)strlen(names[i]) > maxlen) maxlen = strlen(names[i]);
    int colw = maxlen + 2;
    int termw = get_term_width();
    int cur = 0;
    for (int i = 0; i < n; ++i) {
        if (cur + colw > termw) {
            printf("\n");
            cur = 0;
        }
        print_colored_padded(".", names[i], colw, 0);
        cur += colw;
    }
    printf("\n");
}

/* Long listing helpers */
void print_permissions(mode_t mode) {
    char perms[11];
    perms[0] = S_ISDIR(mode) ? 'd' :
               S_ISLNK(mode) ? 'l' :
               S_ISCHR(mode) ? 'c' :
               S_ISBLK(mode) ? 'b' :
               S_ISSOCK(mode)? 's' :
               S_ISFIFO(mode)? 'p' : '-';
    perms[1] = (mode & S_IRUSR) ? 'r' : '-';
    perms[2] = (mode & S_IWUSR) ? 'w' : '-';
    perms[3] = (mode & S_IXUSR) ? ((mode & S_ISUID) ? 's' : 'x') : ((mode & S_ISUID) ? 'S' : '-');
    perms[4] = (mode & S_IRGRP) ? 'r' : '-';
    perms[5] = (mode & S_IWGRP) ? 'w' : '-';
    perms[6] = (mode & S_IXGRP) ? ((mode & S_ISGID) ? 's' : 'x') : ((mode & S_ISGID) ? 'S' : '-');
    perms[7] = (mode & S_IROTH) ? 'r' : '-';
    perms[8] = (mode & S_IWOTH) ? 'w' : '-';
    perms[9] = (mode & S_IXOTH) ? ((mode & S_ISVTX) ? 't' : 'x') : ((mode & S_ISVTX) ? 'T' : '-');
    perms[10] = '\0';
    printf("%s", perms);
}

void print_long_listing(const char *dirpath) {
    DIR *dir = opendir(dirpath);
    if (!dir) { perror("opendir"); return; }
    struct dirent *e;
    struct stat st;
    while ((e = readdir(dir)) != NULL) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
        char full[PATH_MAX];
        snprintf(full, sizeof(full), "%s/%s", dirpath, e->d_name);
        if (lstat(full, &st) == -1) { perror("lstat"); continue; }
        print_permissions(st.st_mode);
        struct passwd *pw = getpwuid(st.st_uid);
        struct group  *gr = getgrgid(st.st_gid);

        char timebuf[64];
        time_t now = time(NULL);
        const long SIX_MONTHS = 15552000L;
        struct tm *tm_info = localtime(&st.st_mtime);
        if (llabs((long long)(now - st.st_mtime)) > SIX_MONTHS)
            strftime(timebuf, sizeof(timebuf), "%b %e  %Y", tm_info);
        else
            strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm_info);

        printf(" %2ld %-8s %-8s %8lld %s ",
               (long)st.st_nlink,
               pw ? pw->pw_name : "?",
               gr ? gr->gr_name : "?",
               (long long)st.st_size,
               timebuf);

        const char *start = "";
        if (color_enabled) start = choose_color_for(full, e->d_name);

        if (color_enabled && start[0] != '\0') {
            printf("%s%s%s", start, e->d_name, RESET);
        } else {
            printf("%s", e->d_name);
        }

        if (S_ISLNK(st.st_mode)) {
            char link_target[PATH_MAX];
            ssize_t len = readlink(full, link_target, sizeof(link_target)-1);
            if (len != -1) {
                link_target[len] = '\0';
                printf(" -> %s", link_target);
            }
        }
        printf("\n");
    }
    closedir(dir);
}

/* ---------------- recursive do_ls ---------------- */

typedef enum { MODE_DEFAULT=0, MODE_LONG=1, MODE_HORIZONTAL=2 } display_mode_t;

void do_ls(const char *dirname, display_mode_t mode, int recursive_flag) {
    /* Print header like `ls -R` does */
    printf("%s:\n", dirname);

    /* Read and sort names */
    int n = 0;
    char **names = read_dir_names(dirname, &n);
    if (!names) {
        printf("\n"); /* keep spacing similar to ls output when unreadable */
        return;
    }
    qsort(names, n, sizeof(char*), cmpstr_qsort);

    /* Display based on mode */
    if (mode == MODE_LONG) {
        print_long_listing(dirname);
    } else if (mode == MODE_HORIZONTAL) {
        print_horizontal(dirname, names, n);
    } else {
        print_columns_down_across(dirname, names, n);
    }
    printf("\n"); /* blank line after listing (like ls -R) */

    /* If recursive, find subdirectories and recurse */
    if (recursive_flag) {
        for (int i = 0; i < n; ++i) {
            char full[PATH_MAX];
            if (snprintf(full, sizeof(full), "%s/%s", dirname, names[i]) >= (int)sizeof(full)) continue;
            struct stat st;
            if (lstat(full, &st) == -1) continue;
            if (S_ISDIR(st.st_mode)) {
                /* skip . and .. */
                if (strcmp(names[i], ".") == 0 || strcmp(names[i], "..") == 0) continue;
                /* Recurse */
                do_ls(full, mode, recursive_flag);
            }
        }
    }

    free_names(names, n);
}

/* ---------------- main & dispatch ---------------- */

int main(int argc, char *argv[]) {
    color_enabled = isatty(STDOUT_FILENO); /* only colorize when stdout is a terminal */

    display_mode_t mode = MODE_DEFAULT;
    const char *path = ".";
    int opt;
    int recursive_flag = 0;
    while ((opt = getopt(argc, argv, "lxR")) != -1) {
        switch (opt) {
            case 'l': mode = MODE_LONG; break;
            case 'x': if (mode != MODE_LONG) mode = MODE_HORIZONTAL; break;
            case 'R': recursive_flag = 1; break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [-R] [directory]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }
    if (optind < argc) path = argv[optind];

    /* If recursive_flag is set, use do_ls which handles recursion */
    if (recursive_flag) {
        do_ls(path, mode, recursive_flag);
        return EXIT_SUCCESS;
    }

    /* Non-recursive path: read names and dispatch as before */
    int n = 0;
    char **names = read_dir_names(path, &n);
    if (!names) return EXIT_FAILURE;

    qsort(names, n, sizeof(char*), cmpstr_qsort);

    if (mode == MODE_LONG) {
        print_long_listing(path);
    } else if (mode == MODE_HORIZONTAL) {
        print_horizontal(path, names, n);
    } else {
        print_columns_down_across(path, names, n);
    }

    free_names(names, n);
    return EXIT_SUCCESS;
}
