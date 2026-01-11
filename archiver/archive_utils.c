#include "archive.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <unistd.h> 
#include <utime.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>

// Вывод справки
void print_help(void) {
    printf("Использование:\n");
    printf("  ./archiver архив -i файл      Добавить файл в архив\n");
    printf("  ./archiver архив -e файл      Извлечь файл из архива и удалить из него\n");
    printf("  ./archiver архив -s           Показать содержимое архива\n");
    printf("  ./archiver -h                 Показать справку\n");
}

// Восстановление метаданных файла
int restore_metadata(const char *filename, struct file_header *hdr) {
    struct timespec times[2] = {hdr->atime, hdr->mtime};
    
    // Восстанавливаем время доступа и модификации
    if (utimensat(AT_FDCWD, filename, times, 0) < 0) {
        perror("Не удалось восстановить временные метки");
        // Продолжаем выполнение, это не критическая ошибка
    }
    
    // Восстанавливаем права доступа
    if (chmod(filename, hdr->mode) < 0) {
        perror("Не удалось восстановить права доступа");
        // Продолжаем выполнение
    }
    
    // Восстанавливаем владельца и группу
    if (chown(filename, hdr->uid, hdr->gid) < 0) {
        // Если нет прав, просто пропускаем
        if (errno != EPERM) {
            perror("Не удалось восстановить владельца/группу");
        }
    }
    
    return 0;
}

// Поиск файла в архиве
int find_file_in_archive(int archive_fd, const char *filename, 
                         struct file_header *found_hdr, off_t *data_position) {
    struct stat archive_info;
    
    if (fstat(archive_fd, &archive_info) < 0) {
        perror("Не удалось получить информацию об архиве");
        return -1;
    }
    
    off_t current_pos = 0;
    struct file_header current_hdr;
    
    while (current_pos < archive_info.st_size) {
        // Читаем заголовок файла
        if (read_file_header(archive_fd, current_pos, &current_hdr) < 0) {
            break;
        }
        
        // Проверяем совпадение имени
        if (strcmp(current_hdr.name, filename) == 0) {
            if (found_hdr) *found_hdr = current_hdr;
            if (data_position) *data_position = current_pos + sizeof(struct file_header);
            return 1; // Найден
        }
        
        // Переходим к следующему файлу
        current_pos += sizeof(struct file_header) + current_hdr.size;
    }
    
    return 0; // Не найден
}