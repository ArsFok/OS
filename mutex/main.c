#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "common.h"
#include "writer.h"
#include "reader.h"

int main(void) {
    pthread_t writer;
    pthread_t readers[NUM_READERS];
    int reader_ids[NUM_READERS];

    srand(time(NULL)); // Инициализация генератора случайных чисел
    
    // Инициализация общих ресурсов
    init_shared_buffer();

    // Создание пишущего потока
    if (pthread_create(&writer, NULL, writer_func, NULL) != 0) {
        perror("Failed to create writer thread");
        return EXIT_FAILURE;
    }

    // Создание читающих потоков
    for (int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i + 1;
        if (pthread_create(&readers[i], NULL, reader_func, &reader_ids[i]) != 0) {
            perror("Failed to create reader thread");
            return EXIT_FAILURE;
        }
    }

    // Работаем 5 секунд
    sleep(5);

    // Сигнал всем потокам о завершении
    running = 0;

    // Ожидание завершения всех потоков
    pthread_join(writer, NULL);
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }

    // Очистка ресурсов
    cleanup_resources();

    printf("Program completed successfully\n");
    return EXIT_SUCCESS;
}