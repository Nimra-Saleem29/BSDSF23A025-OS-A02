/*
 * ls-v1.3.0 — adds horizontal (-x) display
 * Supports:
 *   - Default: "down then across" column layout
 *   - -l      : long listing with metadata
 *   - -x      : horizontal "across then down" layout
 */

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

/* ------------------ Helpers ------------------ */

int cmpstr(const void *a, const void *b) {
    const char *s1 = *(const char **)a;
    const char *s2 = *(const char **)b;
    return strcmp(s1, s2);
}

/* Read all filenames from directory */
char **read_dir_names(const char *dirpath, int *count) {
    DIR *dir = opendir(dirpath);
    if (!dir) {
        perror("opendir");
        return NULL;
    }
    struct dirent *entry;
    int cap = 50, n = 0;
    char **names = malloc(cap * sizeof(char*));
    if (!names) return NULL;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        if (n >= cap) {
            cap *= 2;
            names = realloc(names, cap * sizeof(char*));
        }
        names[n++] = strdup(entry->d_name);
    }
    closedir(dir);
    *count = n;
    return names;
}

/* ------------------ Terminal width ------------------ */
int get_term_width(void) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
        return 80;
    return ws.ws_col ? ws.ws_col : 80;
}

/* ------------------ Down-then-across layout ------------------ */
void print_columns_down_across(char **names, int n) {
    if (n <= 0) return;
    int maxlen = 0;
    for (int i = 0; i < n; ++i)
        if ((int)strlen(names[i]) > maxlen) maxlen = strlen(names[i]);
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
            if (c == ncols - 1)
                printf("%s", names[idx]);
            else
                printf("%-*s", colw, names[idx]);
        }
        printf("\n");
    }
}

/* ------------------ Horizontal layout (-x) ------------------ */
void print_horizontal(char **names, int n) {
    if (n <= 0) return;
    int maxlen = 0;
    for (int i = 0; i < n; i++)
        if ((int)strlen(names[i]) > maxlen) maxlen = strlen(names[i]);
    int colw = maxlen + 2;
    int termw = get_term_width();
    int cur = 0;
    for (int i = 0; i < n; i++) {
        if (cur + colw > termw) {
            printf("\n");
            cur = 0;
        }
        printf("%-*s", colw, names[i]);
        cur += colw;
    }
    printf("\n");
}

/* ------------------ Long listing (-l) ------------------ */
void print_permissions(mode_t mode) {
    char perms[11];
    perms[0] = S_ISDIR(mode) ? 'd' :
               S_ISLNK(mode) ? 'l' : '-';
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
    if (!dir) {
        perror("opendir");
        return;
    }
    struct dirent *entry;
    struct stat st;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        char full[PATH_MAX];
        snprintf(full, sizeof(full), "%s/%s", dirpath, entry->d_name);
        if (lstat(full, &st) == -1) {
            perror("lstat");
            continue;
        }
        print_permissions(st.st_mode);
        struct passwd *pw = getpwuid(st.st_uid);
        struct group  *gr = getgrgid(st.st_gid);
        char *time_str = ctime(&st.st_mtime);
        time_str[strlen(time_str) - 1] = '\0';
        printf(" %3ld %-8s %-8s %8ld %s ",
               (long)st.st_nlink,
               pw ? pw->pw_name : "?", 
               gr ? gr->gr_name : "?",
               (long)st.st_size,
               time_str);

        char *copy = strdup(entry->d_name);
        printf("%s\n", basename(copy));
        free(copy);
    }
    closedir(dir);
}

/* ------------------ Main ------------------ */
enum DisplayMode { MODE_DEFAULT, MODE_LONG, MODE_HORIZONTAL };

int main(int argc, char *argv[]) {
    const char *dirpath = ".";
    enum DisplayMode mode = MODE_DEFAULT;

    int opt;
    while ((opt = getopt(argc, argv, "lx")) != -1) {
        switch (opt) {
            case 'l':
                mode = MODE_LONG;
                break;
            case 'x':
                mode = MODE_HORIZONTAL;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [directory]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind < argc)
        dirpath = argv[optind];

    int n = 0;
    char **names = read_dir_names(dirpath, &n);
    if (!names) return 1;
    qsort(names, n, sizeof(char*), cmpstr);

    if (mode == MODE_LONG)
        print_long_listing(dirpath);
    else if (mode == MODE_HORIZONTAL)
        print_horizontal(names, n);
    else
        print_columns_down_across(names, n);

    for (int i = 0; i < n; i++) free(names[i]);
    free(names);
    return 0;
}
