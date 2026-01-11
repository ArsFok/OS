#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#define FIFO_NAME "/tmp/lab6_fifo"
#define BUFFER_SIZE 256

int main(void) {
    int fd;
    char buffer[BUFFER_SIZE];
    time_t current_time;
    struct tm *time_info;
    char time_str[64];

    // Создаем FIFO, если он не существует
    if (mkfifo(FIFO_NAME, 0666) == -1) {
        if (errno != EEXIST) {
            perror("mkfifo");
            exit(EXIT_FAILURE);
        }
    }

    // Открываем FIFO для записи
    fd = open(FIFO_NAME, O_WRONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Получаем текущее время
    current_time = time(NULL);
    time_info = localtime(&current_time);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);

    // Формируем строку для отправки
    snprintf(buffer, BUFFER_SIZE, "Время: %s, PID: %d", time_str, getpid());

    printf("Отправитель PID: %d\n", getpid());
    printf("Отправляем: %s\n", buffer);

    // Записываем в FIFO
    if (write(fd, buffer, strlen(buffer) + 1) == -1) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    close(fd);

    return EXIT_SUCCESS;
}