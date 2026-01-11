#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "common.h"

// Глобальные переменные для обработки сигналов
volatile sig_atomic_t running = 1;

// Обработчик сигнала SIGINT (Ctrl+C)
void signal_handler(int signum) {
    (void)signum;
    running = 0;
    printf("\nПолучен сигнал завершения. Завершаю работу...\n");
}

int main(void) {
    int shmid = -1;
    int semid = -1;
    shared_data_t *shm_ptr = NULL;
    
    // Установка обработчика сигналов
    signal(SIGINT, signal_handler);
    
    printf("=== ПРОГРАММА-ПОЛУЧАТЕЛЬ ===\n\n");
    
    // 1. ПОДКЛЮЧЕНИЕ К СУЩЕСТВУЮЩЕЙ РАЗДЕЛЯЕМОЙ ПАМЯТИ
    printf("1. Подключение к разделяемой памяти...\n");
    
    // Ждем, пока отправитель создаст сегмент
    int attempts = 0;
    const int max_attempts = 30; // 30 попыток по 1 секунде = 30 секунд
    
    while (attempts < max_attempts) {
        shmid = shmget(SHM_KEY, sizeof(shared_data_t), 0666);
        if (shmid != -1) {
            break;
        }
        
        if (attempts == 0) {
            printf("   Ожидание создания сегмента памяти отправителем...\n");
        }
        
        sleep(1);
        attempts++;
    }
    
    if (shmid == -1) {
        fprintf(stderr, "Не удалось подключиться к разделяемой памяти.\n");
        fprintf(stderr, "Убедитесь, что программа-отправитель запущена.\n");
        return EXIT_FAILURE;
    }
    
    printf("   Успешно. ID сегмента: %d (ожидание: %d сек)\n", shmid, attempts);
    
    // 2. ПОДКЛЮЧЕНИЕ К РАЗДЕЛЯЕМОЙ ПАМЯТИ
    shm_ptr = (shared_data_t *)shmat(shmid, NULL, 0);
    if (shm_ptr == (shared_data_t *)-1) {
        perror("Ошибка shmat");
        return EXIT_FAILURE;
    }
    printf("   Успешно. Адрес: %p\n", (void*)shm_ptr);
    
    // 3. ПОДКЛЮЧЕНИЕ К СУЩЕСТВУЮЩЕМУ СЕМАФОРУ
    printf("2. Подключение к семафору...\n");
    
    attempts = 0;
    while (attempts < max_attempts) {
        semid = semget(SEM_KEY, 1, 0666);
        if (semid != -1) {
            break;
        }
        sleep(1);
        attempts++;
    }
    
    if (semid == -1) {
        fprintf(stderr, "Не удалось подключиться к семафору.\n");
        shmdt(shm_ptr);
        return EXIT_FAILURE;
    }
    printf("   Успешно. ID семафора: %d\n", semid);
    
    // Структура для операций с семафором
    struct sembuf sem_lock = {0, -1, 0};   // Блокировка (P-операция)
    struct sembuf sem_unlock = {0, 1, 0};  // Разблокировка (V-операция)
    
    printf("\n3. Получатель готов к работе.\n");
    printf(MSG_RECEIVER_STARTED, getpid());
    printf("   Для завершения нажмите Ctrl+C\n\n");
    
    int last_message_number = 0;
    
    // ОСНОВНОЙ ЦИКЛ РАБОТЫ
    while (running) {
        // БЛОКИРОВКА СЕМАФОРА (захват ресурса)
        if (semop(semid, &sem_lock, 1) == -1) {
            perror("Ошибка semop (lock)");
            break;
        }
        
        // ЧТЕНИЕ ДАННЫХ ИЗ РАЗДЕЛЯЕМОЙ ПАМЯТИ
        char received_message[SHM_SIZE];
        int received_number = shm_ptr->message_number;
        
        strncpy(received_message, shm_ptr->message, SHM_SIZE - 1);
        received_message[SHM_SIZE - 1] = '\0';
        
        // РАЗБЛОКИРОВКА СЕМАФОРА (освобождение ресурса)
        if (semop(semid, &sem_unlock, 1) == -1) {
            perror("Ошибка semop (unlock)");
            break;
        }
        
        // Если получено новое сообщение
        if (received_number > last_message_number) {
            last_message_number = received_number;
            
            // Получаем текущее время получателя
            time_t current_time = time(NULL);
            struct tm *time_info = localtime(&current_time);
            char time_str[64];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);
            
            // Выводим информацию
            printf("═══════════════════════════════════════════════\n");
            printf("ПОЛУЧЕНО НОВОЕ СООБЩЕНИЕ #%d\n", received_number);
            printf("═══════════════════════════════════════════════\n");
            printf("Время получения: %s\n", time_str);
            printf("PID получателя:  %d\n", getpid());
            printf("───────────────────────────────────────────────\n");
            printf("Содержимое сообщения:\n");
            printf("   %s\n", received_message);
            printf("═══════════════════════════════════════════════\n\n");
            
            fflush(stdout);
        }
        
        // Короткая пауза перед следующей проверкой
        if (running) {
            usleep(100000); // 100 мс
        }
    }
    
    // ОТСОЕДИНЕНИЕ ОТ РАЗДЕЛЯЕМОЙ ПАМЯТИ
    printf("\n4. Завершение работы...\n");
    if (shmdt(shm_ptr) == -1) {
        perror("Ошибка shmdt");
    }
    
    printf("Программа-получатель завершена.\n");
    return EXIT_SUCCESS;
}