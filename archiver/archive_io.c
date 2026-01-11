#include "archive.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// Чтение заголовка файла из указанной позиции
int read_file_header(int fd, off_t position, struct file_header *hdr) {
    if (pread(fd, hdr, sizeof(struct file_header), position) != sizeof(struct file_header)) {
        return -1;
    }
    return 0;
}

// Запись заголовка файла
int write_file_header(int fd, struct file_header *hdr) {
    if (write(fd, hdr, sizeof(struct file_header)) != sizeof(struct file_header)) {
        return -1;
    }
    return 0;
}

// Копирование данных из одного файла в другой
int copy_data(int src_fd, int dst_fd, off_t size) {
    char buffer[BUFFER_SIZE];
    off_t total_copied = 0;
    ssize_t bytes_read, bytes_written;
    
    while (total_copied < size) {
        size_t to_read = (size - total_copied > BUFFER_SIZE) ? BUFFER_SIZE : size - total_copied;
        
        bytes_read = read(src_fd, buffer, to_read);
        if (bytes_read <= 0) {
            if (bytes_read < 0) {
                perror("Ошибка чтения");
            }
            return -1;
        }
        
        bytes_written = write(dst_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            perror("Ошибка записи");
            return -1;
        }
        
        total_copied += bytes_read;
    }
    
    return 0;
}