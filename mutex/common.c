#include "common.h"
#include <string.h>

char shared_buffer[BUFFER_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int running = 1;

void init_shared_buffer(void) {
    memset(shared_buffer, 0, BUFFER_SIZE);
    strcpy(shared_buffer, "Initial value");
}

void cleanup_resources(void) {
    // Дополнительная очистка ресурсов при необходимости
    pthread_mutex_destroy(&mutex);
}