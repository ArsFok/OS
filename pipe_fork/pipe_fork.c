#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#define BUFFER_SIZE 256

int main(void) {
    int pipefd[2];
    pid_t pid;
    char buffer[BUFFER_SIZE];
    time_t parent_time, child_time;
    struct tm *time_info;
    char time_str[64];

    // Создание pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Создание дочернего процесса
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {  // Родительский процесс
        close(pipefd[0]);  // Закрываем чтение из pipe

        // Получаем текущее время родителя
        parent_time = time(NULL);
        time_info = localtime(&parent_time);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);

        printf("Родительское время: %s\n", time_str);
        printf("PID родителя: %d\n", getpid());

        // Формируем строку для передачи
        snprintf(buffer, BUFFER_SIZE, "Время: %s, PID: %d", time_str, getpid());

        // Записываем в pipe
        if (write(pipefd[1], buffer, strlen(buffer) + 1) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }

        close(pipefd[1]);  // Закрываем запись в pipe

        // Ждем завершения дочернего процесса
        wait(NULL);
    } else {  // Дочерний процесс
        close(pipefd[1]);  // Закрываем запись в pipe

        // Ждем 5 секунд
        sleep(5);

        // Читаем из pipe
        if (read(pipefd[0], buffer, BUFFER_SIZE) == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        close(pipefd[0]);  // Закрываем чтение из pipe

        // Получаем текущее время дочернего процесса
        child_time = time(NULL);
        time_info = localtime(&child_time);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);

        printf("Дочернее время: %s\n", time_str);
        printf("PID дочернего процесса: %d\n", getpid());
        printf("Получено: %s\n", buffer);
    }

    return EXIT_SUCCESS;
}