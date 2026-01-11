#include "reader.h"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void* reader_func(void* arg) {
    int id = *(int*)arg;
    char local_buf[BUFFER_SIZE];

    while (running) {
        pthread_mutex_lock(&mutex); // Блокировка мьютекса
        
        // Копирование данных из общего буфера в локальный
        strncpy(local_buf, shared_buffer, BUFFER_SIZE - 1);
        local_buf[BUFFER_SIZE - 1] = '\0'; // Гарантируем нуль-терминацию
        
        pthread_mutex_unlock(&mutex); // Разблокировка мьютекса
        
        // Вывод информации о читателе и содержимого буфера
        printf("Reader %d (TID: %lu): %s\n", 
               id, (unsigned long)pthread_self(), local_buf);
        
        // Случайная пауза между 0.3 и 0.5 секундами
        usleep(300000 + (rand() % 200000));
    }

    return NULL;
}