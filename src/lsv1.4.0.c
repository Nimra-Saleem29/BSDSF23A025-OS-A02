/* src/lsv1.4.0.c
 * Feature-5 — Alphabetical Sort (v1.4.0)
 * Sort directory entries alphabetically using qsort(), affects all display modes.
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
#include <errno.h>
#include <limits.h>

/* ---------- Comparator for qsort ---------- */
int cmpstr(const void *a, const void *b) {
    const char * const *sa = a;
    const char * const *sb = b;
    return strcmp(*sa, *sb);
}

/* ---------- Helpers ---------- */
int get_term_width(void) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) return 80;
    return ws.ws_col ? ws.ws_col : 80;
}

/* Read directory names into dynamic array */
char **read_dir_names(const char *dirpath, int *count) {
    DIR *dir = opendir(dirpath);
    if (!dir) { perror("opendir"); *count = 0; return NULL; }
    int cap = 64, n = 0;
    char **arr = malloc(sizeof(char*) * cap);
    if (!arr) { closedir(dir); *count = 0; return NULL; }
    struct dirent *e;
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

/* free names */
void free_names(char **names, int n) {
    if (!names) return;
    for (int i = 0; i < n; ++i) free(names[i]);
    free(names);
}

/* ---------- Display: Down-then-across ---------- */
void print_columns_down_across(char **names, int n) {
    if (n <= 0) return;
    int maxlen = 0;
    for (int i = 0; i < n; ++i)
        if ((int)strlen(names[i]) > maxlen) maxlen = strlen(names[i]);
    int colw = maxlen + 2;
    int termw = get_term_width();
    int ncols = termw / colw;
    if (ncols < 1) ncols = 1;
    int nrows = (n + ncols - 1) / ncols;
    for (int r = 0; r < nrows; ++r) {
        for (int c = 0; c < ncols; ++c) {
            int idx = r + c * nrows;
            if (idx >= n) continue;
            if (c == ncols - 1) printf("%s", names[idx]);
            else printf("%-*s", colw, names[idx]);
        }
        printf("\n");
    }
}

/* ---------- Display: Horizontal (-x) ---------- */
void print_horizontal(char **names, int n) {
    if (n <= 0) return;
    int maxlen = 0;
    for (int i = 0; i < n; ++i)
        if ((int)strlen(names[i]) > maxlen) maxlen = strlen(names[i]);
    int colw = maxlen + 2;
    int termw = get_term_width();
    int cur = 0;
    for (int i = 0; i < n; ++i) {
        if (cur + colw > termw) { printf("\n"); cur = 0; }
        printf("%-*s", colw, names[i]);
        cur += colw;
    }
    printf("\n");
}

/* ---------- Display: Long listing (-l) ---------- */
void print_permissions(mode_t mode) {
    char perms[11];
    perms[0] = S_ISDIR(mode) ? 'd' : S_ISLNK(mode) ? 'l' : '-';
    perms[1] = (mode & S_IRUSR) ? 'r' : '-';
    perms[2] = (mode & S_IWUSR) ? 'w' : '-';
    perms[3] = (mode & S_IXUSR) ? 'x' : '-';
    perms[4] = (mode & S_IRGRP) ? 'r' : '-';
    perms[5] = (mode & S_IWGRP) ? 'w' : '-';
    perms[6] = (mode & S_IXGRP) ? 'x' : '-';
    perms[7] = (mode & S_IROTH) ? 'r' : '-';
    perms[8] = (mode & S_IWOTH) ? 'w' : '-';
    perms[9] = (mode & S_IXOTH) ? 'x' : '-';
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
        struct tm *tm_info = localtime(&st.st_mtime);
        strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm_info);
        printf(" %2ld %-8s %-8s %8lld %s %s\n",
               (long)st.st_nlink,
               pw ? pw->pw_name : "?",
               gr ? gr->gr_name : "?",
               (long long)st.st_size,
               timebuf,
               e->d_name);
    }
    closedir(dir);
}

/* ---------- Main ---------- */
typedef enum { MODE_DEFAULT = 0, MODE_LONG = 1, MODE_HORIZONTAL = 2 } display_mode_t;

int main(int argc, char *argv[]) {
    display_mode_t mode = MODE_DEFAULT;
    const char *path = ".";
    int opt;
    while ((opt = getopt(argc, argv, "lx")) != -1) {
        switch (opt) {
            case 'l': mode = MODE_LONG; break;
            case 'x': if (mode != MODE_LONG) mode = MODE_HORIZONTAL; break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [directory]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }
    if (optind < argc) path = argv[optind];

    int n = 0;
    char **names = read_dir_names(path, &n);
    if (!names) return EXIT_FAILURE;

    /* Feature-5: sort alphabetically before displaying */
    qsort(names, n, sizeof(char*), cmpstr);

    if (mode == MODE_LONG)
        print_long_listing(path);
    else if (mode == MODE_HORIZONTAL)
        print_horizontal(names, n);
    else
        print_columns_down_across(names, n);

    free_names(names, n);
    return EXIT_SUCCESS;
}
