/*
 * ls-v1.2.0 : Custom ls implementation
 * Features:
 *   v1.0.0 - Simple listing
 *   v1.1.0 - Long listing format (-l)
 *   v1.2.0 - Column display (down then across)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>   // For terminal width

/* ---------- Helper functions ---------- */

/* Compare function for sorting */
int cmpstr(const void *a, const void *b) {
    const char *sa = *(const char **)a;
    const char *sb = *(const char **)b;
    return strcmp(sa, sb);
}

/* Read all directory names into an array of strings */
char **read_dir_names(const char *path, int *count) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return NULL;
    }

    struct dirent *entry;
    int capacity = 50;
    int n = 0;
    char **names = malloc(capacity * sizeof(char *));
    if (!names) return NULL;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        if (n >= capacity) {
            capacity *= 2;
            names = realloc(names, capacity * sizeof(char *));
        }
        names[n++] = strdup(entry->d_name);
    }
    closedir(dir);
    *count = n;
    return names;
}

/* ---------- Feature-2: Long Listing (-l) ---------- */

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
    perms[3] = (mode & S_IXUSR) ? ((mode & S_ISUID) ? 's' : 'x') :
                                   ((mode & S_ISUID) ? 'S' : '-');
    perms[4] = (mode & S_IRGRP) ? 'r' : '-';
    perms[5] = (mode & S_IWGRP) ? 'w' : '-';
    perms[6] = (mode & S_IXGRP) ? ((mode & S_ISGID) ? 's' : 'x') :
                                   ((mode & S_ISGID) ? 'S' : '-');
    perms[7] = (mode & S_IROTH) ? 'r' : '-';
    perms[8] = (mode & S_IWOTH) ? 'w' : '-';
    perms[9] = (mode & S_IXOTH) ? ((mode & S_ISVTX) ? 't' : 'x') :
                                   ((mode & S_ISVTX) ? 'T' : '-');
    perms[10] = '\0';

    printf("%s", perms);
}

/* Long listing function */
void print_long_listing(const char *dirpath) {
    int n = 0;
    char **names = read_dir_names(dirpath, &n);
    if (!names) return;
    qsort(names, n, sizeof(char *), cmpstr);

    for (int i = 0; i < n; i++) {
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, names[i]);
        struct stat st;
        if (lstat(fullpath, &st) == -1) {
            perror("lstat");
            continue;
        }

        print_permissions(st.st_mode);
        printf(" %2lu", st.st_nlink);

        struct passwd *pw = getpwuid(st.st_uid);
        struct group  *gr = getgrgid(st.st_gid);
        printf(" %-8s %-8s", pw ? pw->pw_name : "?", gr ? gr->gr_name : "?");
        printf(" %8ld", (long)st.st_size);

        char timebuf[64];
        struct tm *tm_info = localtime(&st.st_mtime);
        strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm_info);
        printf(" %s", timebuf);

        if (S_ISLNK(st.st_mode)) {
            char link_target[512];
            ssize_t len = readlink(fullpath, link_target, sizeof(link_target) - 1);
            if (len != -1) {
                link_target[len] = '\0';
                printf(" %s -> %s\n", names[i], link_target);
            } else {
                printf(" %s\n", names[i]);
            }
        } else {
            printf(" %s\n", names[i]);
        }
    }

    for (int i = 0; i < n; i++) free(names[i]);
    free(names);
}

/* ---------- Feature-3: Column Display (Default) ---------- */

/* Get terminal width (fallback = 80 if ioctl fails) */
int get_term_width(void) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
        return 80;
    if (ws.ws_col == 0) return 80;
    return (int)ws.ws_col;
}

/* Print array of names in "down then across" layout */
void print_columns_down_across(char **names, int n) {
    if (n <= 0) return;

    int maxlen = 0;
    for (int i = 0; i < n; i++) {
        int len = (int)strlen(names[i]);
        if (len > maxlen) maxlen = len;
    }

    int spacing = 2;
    int colw = maxlen + spacing;
    int termw = get_term_width();
    int ncols = termw / colw;
    if (ncols < 1) ncols = 1;
    int nrows = (n + ncols - 1) / ncols;

    for (int r = 0; r < nrows; r++) {
        for (int c = 0; c < ncols; c++) {
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

/* Default simple print (Feature-3 replaces Feature-1 logic) */
void print_simple(const char *dirpath) {
    int n = 0;
    char **names = read_dir_names(dirpath, &n);
    if (!names) return;
    qsort(names, n, sizeof(char *), cmpstr);
    print_columns_down_across(names, n);
    for (int i = 0; i < n; i++) free(names[i]);
    free(names);
}

/* ---------- main ---------- */

int main(int argc, char *argv[]) {
    int opt;
    int long_format = 0;

    /* argument parsing (-l) */
    while ((opt = getopt(argc, argv, "l")) != -1) {
        switch (opt) {
            case 'l': long_format = 1; break;
            default:
                fprintf(stderr, "Usage: %s [-l] [directory]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    const char *dirpath = ".";
    if (optind < argc)
        dirpath = argv[optind];

    if (long_format)
        print_long_listing(dirpath);
    else
        print_simple(dirpath);

    return 0;
}
