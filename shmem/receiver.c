#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <wchar.h>
#include <locale.h>

#define SHM_KEY 0x7A7A7A7A
#define SHM_SIZE 512

// Функция для подсчета видимых символов в строке
int visible_length(const char *str) {
    int len = 0;
    while (*str) {
        // Для кириллицы в UTF-8 пропускаем дополнительные байты
        if ((*str & 0xC0) != 0x80) {
            len++;
        }
        str++;
    }
    return len;
}

// Функция для вывода строки с учетом ширины
void print_padded(const char *str, int width) {
    int visible_len = visible_length(str);
    int padding = width - visible_len;
    
    if (padding > 0) {
        printf("%s", str);
        for (int i = 0; i < padding; i++) {
            printf(" ");
        }
    } else {
        // Если строка слишком длинная, обрезаем
        int printed = 0;
        const char *p = str;
        while (*p && printed < width) {
            putchar(*p);
            // Пропускаем дополнительные байты UTF-8
            if ((*p & 0xC0) != 0x80) {
                printed++;
            }
            p++;
        }
        // Добиваем пробелами если нужно
        for (int i = printed; i < width; i++) {
            printf(" ");
        }
    }
}

void signal_handler(int sig) {
    printf("\nПолучатель (PID: %d) завершает работу по сигналу %d...\n", 
           getpid(), sig);
    exit(EXIT_SUCCESS);
}

int main(void) {
    // Устанавливаем локаль для корректной работы с кириллицей
    setlocale(LC_ALL, "ru_RU.UTF-8");
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("Программа-получатель запущена (PID: %d)\n", getpid());
    printf("Ожидание данных от отправителя...\n");
    printf("Для завершения нажмите Ctrl+C\n\n");
    
    int shmid;
    int attempts = 0;
    const int max_attempts = 10;
    
    while (attempts < max_attempts) {
        shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
        if (shmid != -1) {
            break;
        }
        
        if (errno == ENOENT) {
            printf("Ожидание... (%d/%d)\r", attempts + 1, max_attempts);
            fflush(stdout);
            sleep(1);
            attempts++;
        } else {
            perror("Ошибка получения разделяемой памяти");
            exit(EXIT_FAILURE);
        }
    }
    
    if (shmid == -1) {
        fprintf(stderr, "\nНе удалось подключиться к разделяемой памяти\n");
        fprintf(stderr, "Убедитесь, что программа-отправитель запущена\n");
        exit(EXIT_FAILURE);
    }
    
    printf("\nПодключение успешно!\n");
    
    char *shm_ptr = (char *)shmat(shmid, NULL, 0);
    if (shm_ptr == (char *)-1) {
        perror("Ошибка подключения к разделяемой памяти");
        exit(EXIT_FAILURE);
    }
    
    // Главный цикл
    while (1) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char receiver_time[64];
        strftime(receiver_time, sizeof(receiver_time), 
                "%Y-%m-%d %H:%M:%S", tm_info);
        
        // Выводим с правильным форматированием
        printf("┌─────────────────────────────────────────────────────┐\n");
        
        // Время получателя
        printf("│ Время получателя: ");
        print_padded(receiver_time, 34);
        printf("│\n");
        
        // PID получателя
        char pid_str[32];
        snprintf(pid_str, sizeof(pid_str), "%d", getpid());
        printf("│ PID получателя:   ");
        print_padded(pid_str, 34);
        printf("│\n");
        
        // Разделитель
        printf("├─────────────────────────────────────────────────────┤\n");
        
        // Полученное сообщение
        printf("│ Получено сообщение:                                 │\n");
        printf("│ ");
        print_padded(shm_ptr, 51);
        printf(" │\n");
        
        printf("└─────────────────────────────────────────────────────┘\n\n");
        
        sleep(1);
    }
    
    shmdt(shm_ptr);
    return EXIT_SUCCESS;
}