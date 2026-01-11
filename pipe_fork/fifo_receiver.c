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

    // Проверяем, существует ли FIFO
    if (access(FIFO_NAME, F_OK) == -1) {
        printf("FIFO не существует. Запустите сначала отправителя.\n");
        exit(EXIT_FAILURE);
    }

    // Открываем FIFO для чтения
    fd = open(FIFO_NAME, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    printf("Получатель PID: %d\n", getpid());
    printf("Ожидание данных... (10 секунд)\n");

    // Ждем 10 секунд
    sleep(10);

    // Читаем из FIFO
    if (read(fd, buffer, BUFFER_SIZE) == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    close(fd);

    // Получаем текущее время получателя
    current_time = time(NULL);
    time_info = localtime(&current_time);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);

    printf("Время получателя: %s\n", time_str);
    printf("Получено: %s\n", buffer);

    // Удаляем FIFO
    unlink(FIFO_NAME);

    return EXIT_SUCCESS;
}