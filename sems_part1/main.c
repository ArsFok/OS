#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 64

// Глобальные переменные
char shared_buffer[BUFFER_SIZE];
sem_t buffer_sem;  // Семафор для синхронизации доступа к буферу
int running = 1;

// Функция писателя
void *writer_func(void *arg) {
    (void)arg;
    int counter = 0;

    while (running) {
        // Формируем строку с номером записи
        snprintf(shared_buffer, BUFFER_SIZE, "Write #%d", counter);
        counter++;

        // Разблокируем семафор - данные готовы для чтения
        if (sem_post(&buffer_sem) != 0) {
            perror("sem_post");
            break;
        }

        // Ждем 1 секунду перед следующей записью
        sleep(1);
    }

    return NULL;
}

// Функция читателя
void *reader_func(void *arg) {
    (void)arg;
    char local_buf[BUFFER_SIZE];

    while (running) {
        // Ждем, пока писатель подготовит данные (блокируемся на семафоре)
        if (sem_wait(&buffer_sem) != 0) {
            perror("sem_wait");
            break;
        }

        // Копируем данные из общего буфера
        strncpy(local_buf, shared_buffer, BUFFER_SIZE - 1);
        local_buf[BUFFER_SIZE - 1] = '\0';

        // Выводим информацию
        printf("Reader tid=%lu: buffer = \"%s\"\n",
               (unsigned long)pthread_self(), local_buf);
        fflush(stdout);

        // Небольшая задержка для демонстрации
        usleep(500000);
    }

    return NULL;
}

int main(void) {
    pthread_t writer, reader;

    // Инициализация общего буфера
    memset(shared_buffer, 0, BUFFER_SIZE);
    
    // Инициализация семафора с начальным значением 0
    // (буфер пуст, читатель должен ждать)
    if (sem_init(&buffer_sem, 0, 0) != 0) {
        perror("sem_init");
        return EXIT_FAILURE;
    }

    // Создание потока писателя
    if (pthread_create(&writer, NULL, writer_func, NULL) != 0) {
        perror("pthread_create writer");
        sem_destroy(&buffer_sem);
        return EXIT_FAILURE;
    }

    // Создание потока читателя
    if (pthread_create(&reader, NULL, reader_func, NULL) != 0) {
        perror("pthread_create reader");
        running = 0;
        pthread_join(writer, NULL);
        sem_destroy(&buffer_sem);
        return EXIT_FAILURE;
    }

    printf("Программа будет работать 10 секунд...\n");
    printf("Писатель записывает данные каждую секунду\n");
    printf("Читатель читает данные при их наличии\n\n");

    // Работа программы в течение 10 секунд
    sleep(10);

    // Сигнал о завершении
    running = 0;

    // Отправляем дополнительные сигналы на семафор,
    // чтобы разблокировать потоки, если они ждут
    sem_post(&buffer_sem);
    
    // Ожидание завершения потоков
    pthread_join(writer, NULL);
    pthread_join(reader, NULL);

    // Уничтожение семафора
    sem_destroy(&buffer_sem);

    printf("\nПрограмма завершена успешно\n");
    return EXIT_SUCCESS;
}