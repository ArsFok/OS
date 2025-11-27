#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <limits.h>


int compare_names(const void *a, const void *b) {
    const struct dirent *da = *(const struct dirent **)a;
    const struct dirent *db = *(const struct dirent **)b;
    return strcasecmp(da->d_name, db->d_name);
}

void print_permissions(mode_t mode) {
    switch (mode & S_IFMT) {
        case S_IFDIR:
            printf("d");
            break;
        case S_IFLNK:
            printf("l");
            break;
        case S_IFSOCK:
            printf("s");
            break;
        case S_IFIFO:
            printf("p");
            break;
        case S_IFBLK:
            printf("b");
            break;
        case S_IFCHR:
            printf("c");
            break;
        default:
            printf("-");
            break;
    }

    printf(
        "%c%c%c%c%c%c%c%c%c",

        ((mode & S_IRUSR) ? 'r' : '-'),
        ((mode & S_IWUSR) ? 'w' : '-'),
        ((mode & S_IXUSR) ? 'x' : '-'),
        
        ((mode & S_IRGRP) ? 'r' : '-'),
        ((mode & S_IWGRP) ? 'w' : '-'),
        ((mode & S_IXGRP) ? 'x' : '-'),
        
        ((mode & S_IROTH) ? 'r' : '-'),
        ((mode & S_IWOTH) ? 'w' : '-'),
        ((mode & S_IXOTH) ? 'x' : '-')
    );
}

bool is_executable(const char* filename, mode_t mode) {
    if((mode & S_IFMT) != S_IFREG) return false;

    bool has_exec_bits = (mode & S_IXUSR) || (mode & S_IXGRP) || (mode & S_IXOTH);
    const char *dot = strrchr(filename, '.');
    bool has_exec_extension = false;
    if (dot) {
        has_exec_extension = (strcmp(dot, ".out") == 0 || strcmp(dot, ".exe") == 0);
    }
    
    return has_exec_bits || has_exec_extension;
}

void list_files(const char* dirname, bool show_hidden, bool long_format){
    DIR* dirp;

    struct dirent *dp;
    struct stat stbuf;
    struct passwd *pwent;
    struct group *gent;
    time_t mtime;
    char fullpath[PATH_MAX];
    char link_target[PATH_MAX];
    long total_blocks = 0;
    ssize_t len;

    struct dirent **entries = NULL;
    int count = 0;
    int capacity = 100;

    size_t max_links = 0;
    size_t max_user = 0;
    size_t max_group = 0;
    size_t max_size = 0;

    if(!(dirp = opendir(dirname))){
        fprintf(stderr, "Error opening the directory: %s\n", dirname);
        exit(EXIT_FAILURE);
    }

    entries = malloc(capacity * sizeof(struct dirent *));
    if (!entries) {
        fprintf(stderr, "Memory allocation failed\n");
        closedir(dirp);
        exit(EXIT_FAILURE);
    }

    while((dp = readdir(dirp)) != NULL){
        if(!show_hidden && dp->d_name[0] == '.') continue;
        
        struct dirent *entry = malloc(sizeof(struct dirent));
        if (!entry) {
            fprintf(stderr, "Memory allocation failed\n");
            continue;
        }
        memcpy(entry, dp, sizeof(struct dirent));
        
        if (count >= capacity) {
            capacity *= 2;
            struct dirent **new_entries = realloc(entries, capacity * sizeof(struct dirent *));
            if (!new_entries) {
                fprintf(stderr, "Memory reallocation failed\n");
                free(entry);
                break;
            }
            entries = new_entries;
        }
        
        entries[count++] = entry;
    }
    closedir(dirp);

    qsort(entries, count, sizeof(struct dirent *), compare_names);

    if(long_format){
        for(int i = 0; i < count; i++){
            snprintf(fullpath, PATH_MAX, "%s/%s", dirname, entries[i]->d_name);
            if(lstat(fullpath, &stbuf) < 0){
                continue;
            }
            total_blocks += stbuf.st_blocks;

            char links_str[20];
            snprintf(links_str, sizeof(links_str), "%lu", (unsigned long)stbuf.st_nlink);
            size_t links_len = strlen(links_str);
            if (links_len > max_links) max_links = links_len;

            pwent = getpwuid(stbuf.st_uid);
            char uid_str[20];
            const char *user_name;
            if (pwent) {
                user_name = pwent->pw_name;
            } else {
                snprintf(uid_str, sizeof(uid_str), "%u", stbuf.st_uid);
                user_name = uid_str;
            }
            size_t user_len = strlen(user_name);
            if (user_len > max_user) max_user = user_len;

            gent = getgrgid(stbuf.st_gid);
            char gid_str[20];
            const char *group_name;
            if (gent) {
                group_name = gent->gr_name;
            } else {
                snprintf(gid_str, sizeof(gid_str), "%u", stbuf.st_gid);
                group_name = gid_str;
            }
            size_t group_len = strlen(group_name);
            if (group_len > max_group) max_group = group_len;

            char size_str[20];
            snprintf(size_str, sizeof(size_str), "%ld", (long)stbuf.st_size);
            size_t size_len = strlen(size_str);
            if (size_len > max_size) max_size = size_len;
        }
        printf("total %ld\n", total_blocks / 2);
    }

    for(int i = 0; i < count; i++){
        dp = entries[i];
        snprintf(fullpath, PATH_MAX, "%s/%s", dirname, dp->d_name);
        if(lstat(fullpath, &stbuf) < 0){
            fprintf(stderr, "Error getting file statistics: %s\n", fullpath);
            continue;
        }
        
        if(long_format){
            print_permissions(stbuf.st_mode);
            printf(" ");

            printf("%*lu ", (int)max_links, (unsigned long)stbuf.st_nlink);

            pwent = getpwuid(stbuf.st_uid);
            gent = getgrgid(stbuf.st_gid);
            
            char uid_str[20], gid_str[20];
            const char *user_name, *group_name;

            if (pwent) {
                user_name = pwent->pw_name;
            } else {
                snprintf(uid_str, sizeof(uid_str), "%u", stbuf.st_uid);
                user_name = uid_str;
            }

            if (gent) {
                group_name = gent->gr_name;
            } else {
                snprintf(gid_str, sizeof(gid_str), "%u", stbuf.st_gid);
                group_name = gid_str;
            }
            
            printf("%-*s %-*s", (int)max_user, user_name, (int)max_group, group_name);
            printf(" %*ld ", (int)max_size, (long)stbuf.st_size);

            mtime = stbuf.st_mtim.tv_sec;
            struct tm *timeinfo = localtime(&mtime);
            time_t current_time = time(NULL);

            char timestring[30];
            if(difftime(current_time, mtime) > 15778800){
                strftime(timestring, sizeof(timestring), "%b %d  %Y", timeinfo);
            }else{
                strftime(timestring, sizeof(timestring), "%b %d %H:%M", timeinfo);
            }
            printf(" %s", timestring);
            switch (stbuf.st_mode & S_IFMT) {
                case S_IFDIR:
                    printf(" \033[34m%s\033[0m\n", dp->d_name);
                    break;
                case S_IFLNK:
                    len = readlink(fullpath, link_target, sizeof(link_target) - 1);
                    if (len != -1) {
                        link_target[len] = '\0';
                        printf(" \033[36m%s\033[0m -> \033[34m%s\033[0m\n", dp->d_name, link_target);
                    } else {
                        printf(" \033[36m%s\033[0m\n", dp->d_name);
                    }
                    break;
                case S_IFSOCK:
                    printf(" \033[31m%s\033[0m\n", dp->d_name);
                    break;
                default:
                    if (is_executable(dp->d_name, stbuf.st_mode)) {
                        printf(" \033[32m%s\033[0m\n", dp->d_name);
                    } else {
                        printf(" %s\n", dp->d_name);
                    }
                    break;
            }
        }else{
            switch (stbuf.st_mode & S_IFMT) {
                case S_IFDIR:
                    printf("\033[34m%-2s\033[0m ", dp->d_name);
                    break;
                case S_IFLNK:
                    printf("\033[36m%-2s\033[0m ", dp->d_name);
                    break;
                case S_IFSOCK:
                    printf("\033[31m%-2s\033[0m ", dp->d_name);
                    break;
                default:
                    if (is_executable(dp->d_name, stbuf.st_mode)) {
                        printf("\033[32m%-2s\033[0m ", dp->d_name);
                    } else {
                        printf("%-2s ", dp->d_name);
                    }
                    break;
            }
        }
        free(entries[i]);
    }
    free(entries);
}

int main(int argc, char** argv){
    
    int opt;
    bool aflags = false;
    bool lflags = false;

    while((opt = getopt(argc, argv, "la")) != -1){
        switch(opt){
            case 'l':
                lflags = true;
                break;
            case 'a':
                aflags = true;
                break;
            default:
                fprintf(stderr, "Unknown option: -%c\n", optopt);
                exit(EXIT_FAILURE);
        }
    }
    const char* dir = ".";
    if(argc > optind){
        dir = argv[optind];
    }
    list_files(dir, aflags, lflags);
    if (!lflags) printf("\n");
    return 0;
}