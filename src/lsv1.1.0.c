/* lsv1.1.0.c
 * Simple ls utility with -l long listing implementation (Feature 2).
 *
 * Uses: lstat(), getpwuid(), getgrgid(), ctime(), getopt().
 *
 * Compile with the project's Makefile (gcc -std=gnu11).
 */

#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <limits.h>
#include <libgen.h>

typedef struct {
    char *name;         /* file name (not full path) */
    char *fullpath;     /* full path: dir + "/" + name */
    struct stat st;     /* lstat result */
    char *link_target;  /* if symlink, readlink target */
} FileInfo;

/* Convert mode to permission string like "drwxr-sr-t" */
void mode_to_str(mode_t mode, char str[11]) {
    str[0] = S_ISDIR(mode)  ? 'd' :
             S_ISLNK(mode)  ? 'l' :
             S_ISCHR(mode)  ? 'c' :
             S_ISBLK(mode)  ? 'b' :
             S_ISFIFO(mode) ? 'p' :
             S_ISSOCK(mode) ? 's' : '-';

    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    if (mode & S_ISUID) {
        str[3] = (mode & S_IXUSR) ? 's' : 'S';
    } else {
        str[3] = (mode & S_IXUSR) ? 'x' : '-';
    }

    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    if (mode & S_ISGID) {
        str[6] = (mode & S_IXGRP) ? 's' : 'S';
    } else {
        str[6] = (mode & S_IXGRP) ? 'x' : '-';
    }

    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    if (mode & S_ISVTX) {
        str[9] = (mode & S_IXOTH) ? 't' : 'T';
    } else {
        str[9] = (mode & S_IXOTH) ? 'x' : '-';
    }

    str[10] = '\0';
}

/* Format modification time using ctime() string pieces.
   If older/newer than ~6 months, show year, otherwise show HH:MM.
   Returns a statically allocated string (thread-unsafe but fine here). */
void format_mtime(time_t mtime, char *out, size_t outlen) {
    time_t now = time(NULL);
    const long SIX_MONTHS = 15552000L; /* approx 6 months in seconds */

    char *ct = ctime(&mtime); /* example: "Wed Jun 30 21:49:08 1993\n" */
    if (!ct) {
        snprintf(out, outlen, "??? ?? ?????");
        return;
    }

    /* safe substrings: month at ct+4 (3 chars), day at ct+8 (2), time at ct+11 (5), year at ct+20 (4) */
    char month[4] = {0}, day[3] = {0}, timestr[6] = {0}, year[6] = {0};
    if ((int)strlen(ct) >= 24) {
        memcpy(month, ct + 4, 3);
        month[3] = '\0';
        memcpy(day, ct + 8, 2);
        day[2] = '\0';
        memcpy(timestr, ct + 11, 5);
        timestr[5] = '\0';
        memcpy(year, ct + 20, 4);
        year[4] = '\0';
    } else {
        snprintf(month, sizeof(month), "???");
        snprintf(day, sizeof(day), "??");
        snprintf(timestr, sizeof(timestr), "??:??");
        snprintf(year, sizeof(year), "????");
    }

    if ( llabs((long long)(now - mtime)) > SIX_MONTHS ) {
        /* older/newer: "Mmm dd  YYYY"  (two spaces before year for single-digit day) */
        /* ensure single-space alignment similar to ls: day may be space-padded */
        if (day[0] == ' ') {
            /* already space-padded */
            snprintf(out, outlen, "%s %s  %s", month, day, year);
        } else {
            snprintf(out, outlen, "%s %2s  %s", month, day, year);
        }
    } else {
        /* recent: "Mmm dd HH:MM" */
        snprintf(out, outlen, "%s %2s %s", month, day, timestr);
    }
}

/* Read names from directory (non-hidden), return array of strings */
char **read_dir_names(const char *dirpath, int *count_out) {
    DIR *d = opendir(dirpath);
    if (!d) {
        perror(dirpath);
        *count_out = 0;
        return NULL;
    }
    struct dirent *ent;
    int cap = 128, n = 0;
    char **arr = malloc(sizeof(char*) * cap);
    while ((ent = readdir(d)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;
        if (ent->d_name[0] == '.') continue; /* skip hidden for now */
        if (n >= cap) {
            cap *= 2;
            arr = realloc(arr, sizeof(char*) * cap);
        }
        arr[n++] = strdup(ent->d_name);
    }
    closedir(d);
    *count_out = n;
    return arr;
}

/* comparator for qsort */
int cmpstr(const void *a, const void *b) {
    const char * const *pa = a;
    const char * const *pb = b;
    return strcmp(*pa, *pb);
}

/* gather metadata for each name and return array of FileInfo */
FileInfo *gather_infos(const char *dirpath, char **names, int n) {
    FileInfo *arr = calloc(n, sizeof(FileInfo));
    for (int i = 0; i < n; ++i) {
        arr[i].name = strdup(names[i]);
        arr[i].fullpath = malloc(PATH_MAX);
        snprintf(arr[i].fullpath, PATH_MAX, "%s/%s", dirpath, names[i]);
        if (lstat(arr[i].fullpath, &arr[i].st) == -1) {
            /* print error but continue */
            perror(arr[i].fullpath);
            arr[i].st.st_mode = 0;
        }
        if (S_ISLNK(arr[i].st.st_mode)) {
            char buf[PATH_MAX];
            ssize_t len = readlink(arr[i].fullpath, buf, sizeof(buf)-1);
            if (len != -1) {
                buf[len] = '\0';
                arr[i].link_target = strdup(buf);
            }
        }
    }
    return arr;
}

/* compute column widths */
void compute_widths(FileInfo *arr, int n, int *w_links, int *w_owner, int *w_group, int *w_size) {
    *w_links = *w_owner = *w_group = *w_size = 0;
    for (int i = 0; i < n; ++i) {
        struct stat *st = &arr[i].st;
        /* links */
        int links_len = snprintf(NULL, 0, "%lu", (unsigned long) st->st_nlink);
        if (links_len > *w_links) *w_links = links_len;
        /* owner */
        struct passwd *pw = getpwuid(st->st_uid);
        int owner_len = pw ? (int)strlen(pw->pw_name) : snprintf(NULL, 0, "%u", st->st_uid);
        if (owner_len > *w_owner) *w_owner = owner_len;
        /* group */
        struct group *gr = getgrgid(st->st_gid);
        int group_len = gr ? (int)strlen(gr->gr_name) : snprintf(NULL, 0, "%u", st->st_gid);
        if (group_len > *w_group) *w_group = group_len;
        /* size */
        int size_len = snprintf(NULL, 0, "%lld", (long long)st->st_size);
        if (size_len > *w_size) *w_size = size_len;
    }
    /* make sure minimal widths */
    if (*w_links < 1) *w_links = 1;
    if (*w_owner < 1) *w_owner = 1;
    if (*w_group < 1) *w_group = 1;
    if (*w_size < 1) *w_size = 1;
}

/* print long listing */
void print_long_listing(FileInfo *arr, int n) {
    int w_links, w_owner, w_group, w_size;
    compute_widths(arr, n, &w_links, &w_owner, &w_group, &w_size);

    for (int i = 0; i < n; ++i) {
        struct stat *st = &arr[i].st;
        char perms[11];
        mode_to_str(st->st_mode, perms);
        struct passwd *pw = getpwuid(st->st_uid);
        struct group *gr = getgrgid(st->st_gid);
        char timestr[64];
        format_mtime(st->st_mtime, timestr, sizeof(timestr));

        printf("%s %*lu %-*s %-*s %*lld %s %s",
               perms,
               w_links, (unsigned long) st->st_nlink,
               w_owner, pw ? pw->pw_name : "(?)",
               w_group, gr ? gr->gr_name : "(?)",
               w_size, (long long) st->st_size,
               timestr,
               arr[i].name);

        if (S_ISLNK(st->st_mode) && arr[i].link_target) {
            printf(" -> %s", arr[i].link_target);
        }
        printf("\n");
    }
}

/* fallback simple listing: one file per line */
void print_simple(const char *dirpath) {
    int n = 0;
    char **names = read_dir_names(dirpath, &n);
    if (!names) return;
    qsort(names, n, sizeof(char*), cmpstr);
    for (int i = 0; i < n; ++i) {
        printf("%s\n", names[i]);
        free(names[i]);
    }
    free(names);
}

/* Helper: print long for a single file path (file or symlink) */
void print_long_single(const char *path) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        perror(path);
        return;
    }
    char *basec = strdup(path);
    char *name = basename(basec);
    FileInfo fi;
    memset(&fi, 0, sizeof(fi));
    fi.name = strdup(name);
    fi.fullpath = strdup(path);
    fi.st = st;
    if (S_ISLNK(st.st_mode)) {
        char buf[PATH_MAX];
        ssize_t len = readlink(path, buf, sizeof(buf)-1);
        if (len != -1) {
            buf[len] = '\0';
            fi.link_target = strdup(buf);
        }
    }
    FileInfo arr[1];
    arr[0] = fi;
    print_long_listing(arr, 1);
    free(fi.name);
    free(fi.fullpath);
    free(fi.link_target);
    free(basec);
}

/* free helper */
void free_infos(FileInfo *arr, int n) {
    for (int i = 0; i < n; ++i) {
        free(arr[i].name);
        free(arr[i].fullpath);
        free(arr[i].link_target);
    }
    free(arr);
}

int main(int argc, char **argv) {
    int opt;
    int long_flag = 0;
    while ((opt = getopt(argc, argv, "l")) != -1) {
        switch (opt) {
            case 'l': long_flag = 1; break;
            default:
                fprintf(stderr, "Usage: %s [-l] [path]\n", argv[0]);
                return 1;
        }
    }

    const char *path = (optind < argc) ? argv[optind] : ".";
    struct stat pst;
    if (lstat(path, &pst) == -1) {
        perror(path);
        return 1;
    }

    if (!long_flag) {
        /* fallback to simple listing */
        if (S_ISDIR(pst.st_mode)) {
            print_simple(path);
        } else {
            printf("%s\n", path);
        }
        return 0;
    }

    /* long listing */
    if (!S_ISDIR(pst.st_mode)) {
        /* single file */
        print_long_single(path);
        return 0;
    }

    int n = 0;
    char **names = read_dir_names(path, &n);
    if (!names) return 1;
    qsort(names, n, sizeof(char*), cmpstr);
    FileInfo *infos = gather_infos(path, names, n);
    print_long_listing(infos, n);

    /* cleanup */
    for (int i = 0; i < n; ++i) free(names[i]);
    free(names);
    free_infos(infos, n);
    return 0;
}
