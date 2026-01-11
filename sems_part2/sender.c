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

// Функция очистки ресурсов
void cleanup_resources(int shmid, void* shm_ptr, int semid) {
    // Отсоединяем разделяемую память
    if (shm_ptr != NULL && shm_ptr != (void*)-1) {
        if (shmdt(shm_ptr) == -1) {
            perror("shmdt");
        }
    }
    
    // Удаляем сегмент разделяемой памяти
    if (shmid != -1) {
        if (shmctl(shmid, IPC_RMID, NULL) == -1) {
            perror("shmctl IPC_RMID");
        }
    }
    
    // Удаляем семафор
    if (semid != -1) {
        if (semctl(semid, 0, IPC_RMID) == -1) {
            perror("semctl IPC_RMID");
        }
    }
}

int main(void) {
    int shmid = -1;
    int semid = -1;
    shared_data_t *shm_ptr = NULL;
    
    // Установка обработчика сигналов
    signal(SIGINT, signal_handler);
    
    printf("=== ПРОГРАММА-ОТПРАВИТЕЛЬ ===\n\n");
    
    // 1. СОЗДАНИЕ РАЗДЕЛЯЕМОЙ ПАМЯТИ
    printf("1. Создание сегмента разделяемой памяти...\n");
    shmid = shmget(SHM_KEY, sizeof(shared_data_t), IPC_CREAT | IPC_EXCL | 0666);
    
    // Если сегмент уже существует, удаляем и создаем заново
    if (shmid == -1 && errno == EEXIST) {
        printf("Сегмент уже существует. Удаляю и создаю заново...\n");
        int temp_shmid = shmget(SHM_KEY, sizeof(shared_data_t), 0666);
        if (temp_shmid != -1) {
            shmctl(temp_shmid, IPC_RMID, NULL);
        }
        shmid = shmget(SHM_KEY, sizeof(shared_data_t), IPC_CREAT | 0666);
    }
    
    if (shmid == -1) {
        perror("Ошибка shmget");
        return EXIT_FAILURE;
    }
    printf("   Успешно. ID сегмента: %d\n", shmid);
    
    // 2. ПОДКЛЮЧЕНИЕ К РАЗДЕЛЯЕМОЙ ПАМЯТИ
    printf("2. Подключение к разделяемой памяти...\n");
    shm_ptr = (shared_data_t *)shmat(shmid, NULL, 0);
    if (shm_ptr == (shared_data_t *)-1) {
        perror("Ошибка shmat");
        cleanup_resources(shmid, shm_ptr, semid);
        return EXIT_FAILURE;
    }
    printf("   Успешно. Адрес: %p\n", (void*)shm_ptr);
    
    // 3. СОЗДАНИЕ И ИНИЦИАЛИЗАЦИЯ СЕМАФОРА
    printf("3. Создание и инициализация семафора...\n");
    semid = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | 0666);
    
    // Если семафор уже существует, удаляем и создаем заново
    if (semid == -1 && errno == EEXIST) {
        printf("Семафор уже существует. Удаляю и создаю заново...\n");
        int temp_semid = semget(SEM_KEY, 1, 0666);
        if (temp_semid != -1) {
            semctl(temp_semid, 0, IPC_RMID);
        }
        semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    }
    
    if (semid == -1) {
        perror("Ошибка semget");
        cleanup_resources(shmid, shm_ptr, semid);
        return EXIT_FAILURE;
    }
    
    // Инициализация семафора значением 1 (ресурс свободен)
    union semun sem_arg;
    sem_arg.val = 1;
    if (semctl(semid, 0, SETVAL, sem_arg) == -1) {
        perror("Ошибка semctl SETVAL");
        cleanup_resources(shmid, shm_ptr, semid);
        return EXIT_FAILURE;
    }
    printf("   Успешно. ID семафора: %d\n", semid);
    
    // Структура для операций с семафором
    struct sembuf sem_lock = {0, -1, 0};   // Блокировка (P-операция)
    struct sembuf sem_unlock = {0, 1, 0};  // Разблокировка (V-операция)
    
    printf("\n4. Отправитель готов к работе.\n");
    printf(MSG_SENDER_STARTED, getpid());
    printf("   Для завершения нажмите Ctrl+C\n\n");
    
    int message_counter = 0;
    
    // ОСНОВНОЙ ЦИКЛ РАБОТЫ
    while (running) {
        message_counter++;
        
        // Получаем текущее время
        time_t current_time = time(NULL);
        struct tm *time_info = localtime(&current_time);
        char time_str[64];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);
        
        // Формируем сообщение
        char message[SHM_SIZE];
        snprintf(message, sizeof(message),
                "Сообщение #%d | Время отправки: %s | PID отправителя: %d",
                message_counter, time_str, getpid());
        
        // БЛОКИРОВКА СЕМАФОРА (захват ресурса)
        printf("Отправка сообщения #%d...\n", message_counter);
        if (semop(semid, &sem_lock, 1) == -1) {
            perror("Ошибка semop (lock)");
            break;
        }
        
        // ЗАПИСЬ ДАННЫХ В РАЗДЕЛЯЕМУЮ ПАМЯТЬ
        strncpy(shm_ptr->message, message, SHM_SIZE - 1);
        shm_ptr->message[SHM_SIZE - 1] = '\0';
        shm_ptr->message_number = message_counter;
        
        // РАЗБЛОКИРОВКА СЕМАФОРА (освобождение ресурса)
        if (semop(semid, &sem_unlock, 1) == -1) {
            perror("Ошибка semop (unlock)");
            break;
        }
        
        printf(MSG_SENDING, message_counter);
        printf("   Содержимое: %s\n\n", message);
        
        // Пауза 3 секунды перед следующей отправкой
        if (running) {
            sleep(3);
        }
    }
    
    // ОЧИСТКА РЕСУРСОВ
    printf("\n5. Завершение работы. Очистка ресурсов...\n");
    cleanup_resources(shmid, shm_ptr, semid);
    
    printf("Программа-отправитель завершена.\n");
    return EXIT_SUCCESS;
}