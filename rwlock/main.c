#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_READERS 10
#define BUFFER_SIZE 64
#define RUN_TIME 15  // Время работы программы в секундах

// Глобальные переменные
char shared_buffer[BUFFER_SIZE];
pthread_rwlock_t rwlock;  // Блокировка чтения-записи
int running = 1;          // Флаг работы программы
int write_counter = 0;    // Счетчик записей

// Функция писателя
void *writer_func(void *arg) {
    (void)arg;  // Неиспользуемый параметр
    
    printf("Писатель запущен (tid: %lu)\n", (unsigned long)pthread_self());
    
    while (running) {
        // ЗАХВАТ блокировки на ЗАПИСЬ (эксклюзивный доступ)
        pthread_rwlock_wrlock(&rwlock);
        
        // Формируем строку для записи
        write_counter++;
        snprintf(shared_buffer, BUFFER_SIZE, 
                "Запись #%d от писателя", write_counter);
        
        printf("\033[1;31m[ПИСАТЕЛЬ]\033[0m Произвел запись #%d\n", write_counter);
        
        // ОСВОБОЖДЕНИЕ блокировки
        pthread_rwlock_unlock(&rwlock);
        
        // Пауза между записями (1-2 секунды)
        sleep(1 + (rand() % 2));
    }
    
    printf("Писатель завершен\n");
    return NULL;
}

// Функция читателя
void *reader_func(void *arg) {
    int reader_id = *(int *)arg;
    char local_buf[BUFFER_SIZE];
    
    printf("Читатель %d запущен (tid: %lu)\n", 
           reader_id, (unsigned long)pthread_self());
    
    while (running) {
        // ЗАХВАТ блокировки на ЧТЕНИЕ (могут читать несколько)
        pthread_rwlock_rdlock(&rwlock);
        
        // Копируем данные из общего буфера
        strncpy(local_buf, shared_buffer, BUFFER_SIZE - 1);
        local_buf[BUFFER_SIZE - 1] = '\0';
        
        // ОСВОБОЖДЕНИЕ блокировки
        pthread_rwlock_unlock(&rwlock);
        
        // Вывод информации
        printf("\033[1;32m[ЧИТАТЕЛЬ %d]\033[0m tid=%lu: %s\n",
               reader_id, (unsigned long)pthread_self(), local_buf);
        
        // Случайная пауза между чтениями (0.5-1.5 секунды)
        usleep(500000 + (rand() % 1000000));
    }
    
    printf("Читатель %d завершен\n", reader_id);
    return NULL;
}

int main(void) {
    pthread_t writer;
    pthread_t readers[NUM_READERS];
    int reader_ids[NUM_READERS];
    
    // Инициализация случайных чисел
    srand(time(NULL));
    
    printf("=== ЛАБОРАТОРНАЯ РАБОТА №10 ===\n");
    printf("Блокировки чтения-записи (rwlock)\n");
    printf("Читателей: %d, Писателей: 1\n", NUM_READERS);
    printf("Время работы: %d секунд\n\n", RUN_TIME);
    
    // Инициализация общего буфера
    memset(shared_buffer, 0, BUFFER_SIZE);
    strcpy(shared_buffer, "Начальное значение");
    
    // Инициализация rwlock
    if (pthread_rwlock_init(&rwlock, NULL) != 0) {
        perror("Ошибка инициализации rwlock");
        return EXIT_FAILURE;
    }
    
    // Создание потока писателя
    if (pthread_create(&writer, NULL, writer_func, NULL) != 0) {
        perror("Ошибка создания писателя");
        pthread_rwlock_destroy(&rwlock);
        return EXIT_FAILURE;
    }
    
    // Создание потоков читателей
    for (int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i + 1;
        if (pthread_create(&readers[i], NULL, reader_func, &reader_ids[i]) != 0) {
            perror("Ошибка создания читателя");
            running = 0;
            
            // Завершаем уже созданные потоки
            for (int j = 0; j < i; j++) {
                pthread_join(readers[j], NULL);
            }
            pthread_join(writer, NULL);
            pthread_rwlock_destroy(&rwlock);
            
            return EXIT_FAILURE;
        }
    }
    
    // Ждем указанное время
    printf("\nПрограмма работает %d секунд...\n\n", RUN_TIME);
    sleep(RUN_TIME);
    
    // Сигнал о завершении
    running = 0;
    printf("\nЗавершаем работу...\n");
    
    // Ожидание завершения всех потоков
    pthread_join(writer, NULL);
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    
    // Уничтожение rwlock
    pthread_rwlock_destroy(&rwlock);
    
    // Статистика
    printf("\n=== СТАТИСТИКА ===\n");
    printf("Всего записей произведено: %d\n", write_counter);
    printf("Каждый читатель выполнил примерно %d операций чтения\n", 
           RUN_TIME * 2);  // Примерная оценка
    printf("Программа завершена успешно!\n");
    
    return EXIT_SUCCESS;
}