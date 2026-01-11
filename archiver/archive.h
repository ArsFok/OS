#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#define MAX_NAME_LEN 256
#define BUFFER_SIZE 4096

// Структура заголовка файла в архиве
struct file_header {
    char name[MAX_NAME_LEN];
    off_t size;
    mode_t mode;
    uid_t uid;
    gid_t gid;
    struct timespec atime;
    struct timespec mtime;
};

// Прототипы функций из archive_io.c
int read_file_header(int fd, off_t position, struct file_header *hdr);
int write_file_header(int fd, struct file_header *hdr);
int copy_data(int src_fd, int dst_fd, off_t size);

// Прототипы функций из archive_ops.c
int add_to_archive(const char *archive_name, const char *filename);
int extract_from_archive(const char *archive_name, const char *filename);
int show_archive_info(const char *archive_name);

// Прототипы функций из archive_utils.c
void print_help(void);
int restore_metadata(const char *filename, struct file_header *hdr);
int find_file_in_archive(int archive_fd, const char *filename, 
                         struct file_header *found_hdr, off_t *data_position);

#endif