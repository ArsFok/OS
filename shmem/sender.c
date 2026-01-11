#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define SHM_KEY 0x7A7A7A7A  // Уникальный ключ
#define SHM_SIZE 512
#define LOCK_FILE "/tmp/lab7_sender.pid"

// Проверка существующего процесса отправителя
int is_sender_running(void) {
    FILE *fp = fopen(LOCK_FILE, "r");
    if (fp == NULL) {
        return 0;  // Файл не существует
    }
    
    pid_t pid;
    if (fscanf(fp, "%d", &pid) == 1) {
        // Проверяем, существует ли процесс с таким PID
        if (kill(pid, 0) == 0) {
            fclose(fp);
            return 1;  // Процесс работает
        }
    }
    
    fclose(fp);
    // Удаляем устаревший lock-файл
    unlink(LOCK_FILE);
    return 0;
}

// Создание lock-файла
void create_lock_file(void) {
    FILE *fp = fopen(LOCK_FILE, "w");
    if (fp == NULL) {
        perror("Ошибка создания lock-файла");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "%d\n", getpid());
    fclose(fp);
}

// Очистка ресурсов при завершении
void cleanup_resources(int shmid, char *shm_ptr) {
    if (shm_ptr != NULL && shm_ptr != (char *)-1) {
        shmdt(shm_ptr);
    }
    
    if (shmid >= 0) {
        shmctl(shmid, IPC_RMID, NULL);
    }
    
    unlink(LOCK_FILE);
}

// Обработчик сигналов для graceful shutdown
void signal_handler(int sig) {
    printf("\nПолучен сигнал %d. Завершение работы...\n", sig);
    exit(EXIT_SUCCESS);
}

int main(void) {
    // Устанавливаем обработчики сигналов
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Проверяем, не запущен ли уже отправитель
    if (is_sender_running()) {
        fprintf(stderr, "Ошибка: программа-отправитель уже запущена!\n");
        fprintf(stderr, "Проверьте процесс или удалите файл %s\n", LOCK_FILE);
        exit(EXIT_FAILURE);
    }
    
    // Создаем lock-файл
    create_lock_file();
    
    // Создаем/подключаемся к разделяемой памяти
    int shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0666);
    if (shmid == -1) {
        if (errno == EEXIST) {
            // Разделяемая память уже существует, пытаемся подключиться
            shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
            if (shmid == -1) {
                perror("Ошибка подключения к существующей разделяемой памяти");
                cleanup_resources(-1, NULL);
                exit(EXIT_FAILURE);
            }
        } else {
            perror("Ошибка создания разделяемой памяти");
            cleanup_resources(-1, NULL);
            exit(EXIT_FAILURE);
        }
    }
    
    // Подключаем разделяемую память
    char *shm_ptr = (char *)shmat(shmid, NULL, 0);
    if (shm_ptr == (char *)-1) {
        perror("Ошибка подключения к разделяемой памяти");
        cleanup_resources(shmid, NULL);
        exit(EXIT_FAILURE);
    }
    
    printf("Программа-отправитель запущена (PID: %d)\n", getpid());
    printf("Ключ разделяемой памяти: 0x%X\n", SHM_KEY);
    printf("Для завершения нажмите Ctrl+C\n\n");
    
    // Главный цикл передачи данных
    while (1) {
        // Получаем текущее время
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        
        // Форматируем время
        char time_buffer[64];
        strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);
        
        // Формируем строку для передачи
        char message[SHM_SIZE];
        snprintf(message, sizeof(message), 
                "Время: %s | PID отправителя: %d",
                time_buffer, getpid());
        
        // Копируем в разделяемую память
        strncpy(shm_ptr, message, SHM_SIZE - 1);
        shm_ptr[SHM_SIZE - 1] = '\0';  // Гарантируем null-terminated строку
        
        // Ждем 1 секунду
        sleep(1);
    }
    
    // Этот код никогда не выполнится из-за бесконечного цикла,
    // но нужен для правильной структуры программы
    cleanup_resources(shmid, shm_ptr);
    return EXIT_SUCCESS;
}