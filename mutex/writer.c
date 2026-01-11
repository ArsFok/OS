#include "writer.h"
#include "common.h"
#include <stdio.h>
#include <unistd.h>

void* writer_func(void* arg) {
    (void)arg; // Неиспользуемый параметр
    int counter = 0;

    while (running) {
        pthread_mutex_lock(&mutex); // Блокировка мьютекса
        
        // Запись счетчика в общий буфер
        snprintf(shared_buffer, BUFFER_SIZE, "Counter: %d", counter);
        counter++;
        
        pthread_mutex_unlock(&mutex); // Разблокировка мьютекса
        
        usleep(500000); // Пауза 0.5 секунды
    }

    return NULL;
}