#include "archive.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <pwd.h> 
#include <grp.h> 

// Добавление файла в архив
int add_to_archive(const char *archive_name, const char *filename) {
    struct stat file_info;
    
    // Получаем информацию о файле
    if (stat(filename, &file_info) < 0) {
        perror("Не удалось получить информацию о файле");
        return 1;
    }
    
    // Проверяем, является ли файл обычным файлом
    if (!S_ISREG(file_info.st_mode)) {
        fprintf(stderr, "Файл '%s' не является обычным файлом\n", filename);
        return 1;
    }
    
    // Открываем архив для добавления
    int archive_fd = open(archive_name, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (archive_fd < 0) {
        perror("Не удалось открыть архив");
        return 1;
    }
    
    // Создаем заголовок файла
    struct file_header hdr;
    memset(&hdr, 0, sizeof(hdr));
    
    // Копируем имя файла
    strncpy(hdr.name, filename, MAX_NAME_LEN - 1);
    hdr.name[MAX_NAME_LEN - 1] = '\0';
    
    // Заполняем метаданные
    hdr.size = file_info.st_size;
    hdr.mode = file_info.st_mode;
    hdr.uid = file_info.st_uid;
    hdr.gid = file_info.st_gid;
    hdr.atime = file_info.st_atim;
    hdr.mtime = file_info.st_mtim;
    
    // Записываем заголовок в архив
    if (write_file_header(archive_fd, &hdr) < 0) {
        perror("Не удалось записать заголовок файла");
        close(archive_fd);
        return 1;
    }
    
    // Открываем исходный файл для чтения
    int file_fd = open(filename, O_RDONLY);
    if (file_fd < 0) {
        perror("Не удалось открыть исходный файл");
        close(archive_fd);
        return 1;
    }
    
    // Копируем данные файла в архив
    if (copy_data(file_fd, archive_fd, file_info.st_size) < 0) {
        perror("Не удалось скопировать данные файла");
        close(file_fd);
        close(archive_fd);
        return 1;
    }
    
    // Закрываем файлы
    close(file_fd);
    close(archive_fd);
    
    printf("Файл '%s' успешно добавлен в архив '%s'\n", filename, archive_name);
    return 0;
}

// Извлечение файла из архива
int extract_from_archive(const char *archive_name, const char *filename) {
    // Открываем архив для чтения и записи
    int archive_fd = open(archive_name, O_RDWR);
    if (archive_fd < 0) {
        perror("Не удалось открыть архив");
        return 1;
    }
    
    struct file_header file_hdr;
    off_t data_pos;
    
    // Ищем файл в архиве
    int found = find_file_in_archive(archive_fd, filename, &file_hdr, &data_pos);
    if (found <= 0) {
        if (found == 0) {
            printf("Файл '%s' не найден в архиве\n", filename);
        }
        close(archive_fd);
        return 1;
    }
    
    // Создаем выходной файл
    int out_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, file_hdr.mode);
    if (out_fd < 0) {
        perror("Не удалось создать выходной файл");
        close(archive_fd);
        return 1;
    }
    
    // Копируем данные из архива в файл
    char buffer[BUFFER_SIZE];
    off_t bytes_remaining = file_hdr.size;
    off_t read_pos = data_pos;
    
    while (bytes_remaining > 0) {
        size_t to_read = (bytes_remaining > BUFFER_SIZE) ? BUFFER_SIZE : bytes_remaining;
        ssize_t bytes_read = pread(archive_fd, buffer, to_read, read_pos);
        
        if (bytes_read <= 0) {
            perror("Ошибка чтения из архива");
            close(out_fd);
            close(archive_fd);
            return 1;
        }
        
        if (write(out_fd, buffer, bytes_read) != bytes_read) {
            perror("Ошибка записи в файл");
            close(out_fd);
            close(archive_fd);
            return 1;
        }
        
        read_pos += bytes_read;
        bytes_remaining -= bytes_read;
    }
    
    close(out_fd);
    
    // Восстанавливаем метаданные
    restore_metadata(filename, &file_hdr);
    
    // Создаем временный файл для нового архива
    char temp_name[] = "/tmp/archiver_XXXXXX";
    int temp_fd = mkstemp(temp_name);
    if (temp_fd < 0) {
        perror("Не удалось создать временный файл");
        close(archive_fd);
        return 1;
    }
    
    // Копируем все файлы кроме извлеченного в новый архив
    struct stat archive_info;
    fstat(archive_fd, &archive_info);
    
    off_t current_pos = 0;
    struct file_header current_hdr;
    
    while (current_pos < archive_info.st_size) {
        if (read_file_header(archive_fd, current_pos, &current_hdr) < 0) {
            break;
        }
        
        off_t current_data_pos = current_pos + sizeof(struct file_header);
        
        // Пропускаем извлеченный файл
        if (strcmp(current_hdr.name, filename) != 0) {
            // Записываем заголовок
            if (write_file_header(temp_fd, &current_hdr) < 0) {
                perror("Ошибка записи заголовка во временный файл");
                close(temp_fd);
                close(archive_fd);
                unlink(temp_name);
                return 1;
            }
            
            // Копируем данные
            off_t data_remaining = current_hdr.size;
            off_t data_read_pos = current_data_pos;
            
            while (data_remaining > 0) {
                size_t to_read = (data_remaining > BUFFER_SIZE) ? BUFFER_SIZE : data_remaining;
                ssize_t bytes_read = pread(archive_fd, buffer, to_read, data_read_pos);
                
                if (bytes_read <= 0) {
                    perror("Ошибка чтения данных файла");
                    close(temp_fd);
                    close(archive_fd);
                    unlink(temp_name);
                    return 1;
                }
                
                if (write(temp_fd, buffer, bytes_read) != bytes_read) {
                    perror("Ошибка записи данных во временный файл");
                    close(temp_fd);
                    close(archive_fd);
                    unlink(temp_name);
                    return 1;
                }
                
                data_read_pos += bytes_read;
                data_remaining -= bytes_read;
            }
        }
        
        current_pos = current_data_pos + current_hdr.size;
    }
    
    // Закрываем файлы
    close(archive_fd);
    close(temp_fd);
    
    // Заменяем старый архив новым
    if (rename(temp_name, archive_name) < 0) {
        perror("Не удалось переименовать временный файл");
        unlink(temp_name);
        return 1;
    }
    
    printf("Файл '%s' успешно извлечен из архива и удален из него\n", filename);
    return 0;
}

// Вывод информации об архиве
int show_archive_info(const char *archive_name) {
    int archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd < 0) {
        perror("Не удалось открыть архив");
        return 1;
    }
    
    struct stat archive_info;
    if (fstat(archive_fd, &archive_info) < 0) {
        perror("Не удалось получить информацию об архиве");
        close(archive_fd);
        return 1;
    }
    
    if (archive_info.st_size == 0) {
        printf("Архив '%s' пуст\n", archive_name);
        close(archive_fd);
        return 0;
    }
    
    printf("Содержимое архива '%s':\n", archive_name);
    printf("Общий размер архива: %ld байт\n\n", archive_info.st_size);
    
    printf("%-40s %-12s %-10s %-10s %-10s\n", 
           "Filename", "Size", "Owner", "Group", "Permissions");
    printf("%-40s %-12s %-10s %-10s %-10s\n", 
           "--------", "----", "-----", "-----", "----------");
    
    off_t current_pos = 0;
    struct file_header hdr;
    int file_count = 0;
    off_t total_size = 0;
    
    while (current_pos < archive_info.st_size) {
        if (read_file_header(archive_fd, current_pos, &hdr) < 0) {
            break;
        }
        
        // Получаем имена владельца и группы
        struct passwd *pwd = getpwuid(hdr.uid);
        struct group *grp = getgrgid(hdr.gid);
        
        const char *owner_name = pwd ? pwd->pw_name : "unknown";
        const char *group_name = grp ? grp->gr_name : "unknown";
        
        printf("%-40s %-12ld %-10s %-10s %04o\n", 
               hdr.name, 
               (long)hdr.size,
               owner_name,
               group_name,
               hdr.mode & 0777);
        
        file_count++;
        total_size += hdr.size + sizeof(struct file_header);
        current_pos += sizeof(struct file_header) + hdr.size;
    }
    
    printf("\nИтого: %d файлов, %ld байт данных + %ld байт заголовков\n", 
           file_count, 
           total_size - file_count * sizeof(struct file_header),
           file_count * sizeof(struct file_header));
    
    close(archive_fd);
    return 0;
}