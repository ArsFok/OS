#ifndef COMMON_H
#define COMMON_H

// Ключи для IPC
#define SHM_KEY 0x5678      // Ключ для разделяемой памяти
#define SEM_KEY 0x5679      // Ключ для семафора
#define SHM_SIZE 256        // Размер разделяемой памяти

// Сообщения для отладки
#define MSG_SENDER_STARTED "Отправитель запущен. PID: %d\n"
#define MSG_RECEIVER_STARTED "Получатель запущен. PID: %d\n"
#define MSG_SENDING "Отправитель: Отправка сообщения #%d\n"
#define MSG_RECEIVED "Получатель: Получено сообщение #%d\n"

// Структура для семафора System V (для Linux)
#if defined(__linux__)
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};
#endif

// Структура для данных в разделяемой памяти
typedef struct {
    char message[SHM_SIZE - sizeof(int)];
    int message_number;
} shared_data_t;

#endif // COMMON_H