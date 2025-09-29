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

void print_permissions(mode_t mode) {
    printf(
        "-%c%c%c%c%c%c%c%c%c",

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
    long total_blocks = 0;

    if(!(dirp = opendir(dirname))){
        fprintf(stderr, "Error opening the mkdir:.%s\n", dirname);
        exit(EXIT_FAILURE);
    }
    if(long_format){
        while((dp = readdir(dirp)) != NULL){
            if(!show_hidden && dp->d_name[0] == '.') continue;
            
            snprintf(fullpath, PATH_MAX, "%s/%s", dirname, dp->d_name);
            if(stat(fullpath, &stbuf) < 0){
                continue;
            }
            total_blocks += stbuf.st_blocks;
        }
        printf("total %ld\n", total_blocks / 2);
        rewinddir(dirp);
    }

    while((dp = readdir(dirp)) != NULL){
        if(!show_hidden && dp->d_name[0] == '.') continue;

        snprintf(fullpath, PATH_MAX, "%s/%s", dirname, dp->d_name);
        if(stat(dp->d_name, &stbuf) < 0){
            fprintf(stderr, "Error getting file statistics.\n");
            continue;
        }
        if(long_format){
            print_permissions(stbuf.st_mode);
            printf(" %lu ", (unsigned long)stbuf.st_nlink);

            pwent = getpwuid(stbuf.st_uid);
            gent = getgrgid(stbuf.st_gid);
            printf("%s %s ", pwent->pw_name, gent->gr_name);
            printf(" %5ld ", (long)stbuf.st_size);

            mtime = stbuf.st_mtim.tv_sec;
            struct tm *timeinfo = localtime(&mtime);
            char timestring[30];
            strftime(timestring, sizeof(timestring), "%b %d %H:%M", timeinfo);
            printf(" %s", timestring);
            switch (stbuf.st_mode & S_IFMT) {
                case S_IFDIR:
                    printf(" \033[34m%s\033[0m\n", dp->d_name);
                    break;
                case S_IFLNK:
                    printf(" \033[35m%s\033[0m\n", dp->d_name);
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
                    printf("\033[34m%s\033[0m", dp->d_name);
                    break;
                case S_IFLNK:
                    printf("\033[35m%s\033[0m", dp->d_name);
                    break;
                case S_IFSOCK:
                    printf("\033[31m%s\033[0m", dp->d_name);
                    break;
                default:
                    if (is_executable(dp->d_name, stbuf.st_mode)) {
                        printf("\033[32m%s\033[0m ", dp->d_name);
                    } else {
                        printf("%s ", dp->d_name);
                    }
                    break;
            }
        }
    }
    closedir(dirp);
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