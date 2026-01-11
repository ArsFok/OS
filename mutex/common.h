#ifndef COMMON_H
#define COMMON_H

#include <pthread.h>

#define NUM_READERS 10
#define BUFFER_SIZE 64

extern char shared_buffer[BUFFER_SIZE];
extern pthread_mutex_t mutex;
extern int running;

void init_shared_buffer(void);
void cleanup_resources(void);

#endif // COMMON_H