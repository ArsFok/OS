#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#define NUM_READERS 10
#define BUFFER_SIZE 64
#define RUN_TIME 15      // Время работы в секундах
#define MAX_WRITES 50    // Максимальное количество записей

// Глобальные переменные для синхронизации
char shared_buffer[BUFFER_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t data_ready = PTHREAD_COND_INITIALIZER;  // Условная переменная
pthread_cond_t data_processed = PTHREAD_COND_INITIALIZER;

// Флаги и счетчики
volatile int running = 1;           // Флаг работы программы
volatile int write_counter = 0;     // Счетчик записей
volatile int read_counter = 0;      // Счетчик прочитавших текущие данные
volatile int data_available = 0;    // Флаг наличия новых данных
volatile int active_readers = 0;    // Количество активных читателей

// Статистика
int reads_per_reader[NUM_READERS];
int writer_waits = 0;

// Функция писателя
void *writer_func(void *arg) {
    (void)arg;
    
    printf("Писатель запущен (tid: %lu)\n", (unsigned long)pthread_self());
    
    while (running && write_counter < MAX_WRITES) {
        // Захватываем мьютекс перед проверкой условия
        pthread_mutex_lock(&mutex);
        
        // Ждем, пока все читатели обработают предыдущие данные
        while (active_readers > 0 && running) {
            writer_waits++;
            printf("\033[1;33m[ПИСАТЕЛЬ]\033[0m Ждет завершения %d читателей...\n", 
                   active_readers);
            
            // Ожидаем сигнала, что данные обработаны
            struct timespec timeout;
            clock_gettime(CLOCK_REALTIME, &timeout);
            timeout.tv_sec += 2;  // Таймаут 2 секунды
            
            int rc = pthread_cond_timedwait(&data_processed, &mutex, &timeout);
            if (rc == ETIMEDOUT) {
                printf("\033[1;33m[ПИСАТЕЛЬ]\033[0m Таймаут ожидания!\n");
            }
        }
        
        if (!running) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        
        // ПОДГОТОВКА НОВЫХ ДАННЫХ
        write_counter++;
        snprintf(shared_buffer, BUFFER_SIZE,
                "Запись #%d | Писатель: %lu | Время: %ld",
                write_counter, 
                (unsigned long)pthread_self(),
                time(NULL));
        
        // Сбрасываем счетчик читателей для новых данных
        read_counter = 0;
        data_available = 1;
        
        printf("\n\033[1;31m═══════════════════════════════════════════\033[0m\n");
        printf("\033[1;31m[ПИСАТЕЛЬ]\033[0m Произвел запись #%d\n", write_counter);
        printf("\033[1;31m═══════════════════════════════════════════\033[0m\n\n");
        
        // Сигнализируем всем читателям, что данные готовы
        pthread_cond_broadcast(&data_ready);
        
        // Освобождаем мьютекс
        pthread_mutex_unlock(&mutex);
        
        // Короткая пауза между записями
        usleep(500000 + (rand() % 500000));  // 0.5-1.0 секунды
    }
    
    printf("Писатель завершен. Всего записей: %d\n", write_counter);
    return NULL;
}

// Функция читателя
void *reader_func(void *arg) {
    int reader_id = *(int *)arg;
    char local_buf[BUFFER_SIZE];
    int my_reads = 0;
    int last_write_seen = 0;
    
    printf("Читатель %d запущен (tid: %lu)\n", 
           reader_id, (unsigned long)pthread_self());
    
    while (running) {
        pthread_mutex_lock(&mutex);
        
        // Регистрируем себя как активного читателя
        active_readers++;
        
        // Ждем, пока появятся новые данные
        while (!data_available && running) {
            printf("\033[1;36m[ЧИТАТЕЛЬ %d]\033[0m Ждет новых данных...\n", reader_id);
            pthread_cond_wait(&data_ready, &mutex);
        }
        
        if (!running) {
            active_readers--;
            pthread_mutex_unlock(&mutex);
            break;
        }
        
        // ЧТЕНИЕ ДАННЫХ
        strncpy(local_buf, shared_buffer, BUFFER_SIZE - 1);
        local_buf[BUFFER_SIZE - 1] = '\0';
        
        // Увеличиваем счетчик прочитавших
        read_counter++;
        my_reads++;
        reads_per_reader[reader_id - 1] = my_reads;
        last_write_seen = write_counter;
        
        // Выводим информацию
        printf("\033[1;32m[ЧИТАТЕЛЬ %d]\033[0m tid=%lu | %s | Читателей прочитало: %d/%d\n",
               reader_id,
               (unsigned long)pthread_self(),
               local_buf,
               read_counter,
               NUM_READERS);
        
        // Если все читатели прочитали данные, сигнализируем писателю
        if (read_counter >= NUM_READERS) {
            data_available = 0;  // Данные обработаны
            printf("\033[1;35m[СИГНАЛ]\033[0m Все читатели обработали запись #%d\n", 
                   write_counter);
            pthread_cond_signal(&data_processed);
        }
        
        // Уменьшаем счетчик активных читателей
        active_readers--;
        
        pthread_mutex_unlock(&mutex);
        
        // Имитация обработки данных
        usleep(200000 + (rand() % 300000));  // 0.2-0.5 секунды
        
        // Короткая пауза перед следующим чтением
        usleep(100000 + (rand() % 200000));  // 0.1-0.3 секунды
    }
    
    printf("Читатель %d завершен. Прочитал %d записей. Последняя: #%d\n", 
           reader_id, my_reads, last_write_seen);
    return NULL;
}

// Функция мониторинга (дополнительный поток)
void *monitor_func(void *arg) {
    (void)arg;
    
    printf("\033[1;34m[МОНИТОР]\033[0m Запущен\n");
    
    while (running) {
        sleep(2);  // Проверяем каждые 2 секунды
        
        pthread_mutex_lock(&mutex);
        
        printf("\033[1;34m[МОНИТОР]\033[0m Статус:\n");
        printf("  Записей: %d\n", write_counter);
        printf("  Активных читателей: %d\n", active_readers);
        printf("  Обработано текущую запись: %d/%d\n", 
               read_counter, NUM_READERS);
        printf("  Данные доступны: %s\n", data_available ? "ДА" : "НЕТ");
        printf("  Писатель ждал: %d раз(а)\n", writer_waits);
        
        pthread_mutex_unlock(&mutex);
    }
    
    printf("\033[1;34m[МОНИТОР]\033[0m Завершен\n");
    return NULL;
}

int main(void) {
    pthread_t writer;
    pthread_t readers[NUM_READERS];
    pthread_t monitor;
    int reader_ids[NUM_READERS];
    
    // Инициализация
    srand(time(NULL));
    memset(reads_per_reader, 0, sizeof(reads_per_reader));
    memset(shared_buffer, 0, BUFFER_SIZE);
    strcpy(shared_buffer, "Начальное значение");
    
    printf("\033[1;35m========================================\033[0m\n");
    printf("\033[1;35m   ЛАБОРАТОРНАЯ РАБОТА №11             \033[0m\n");
    printf("\033[1;35m   Условные переменные (condvar)       \033[0m\n");
    printf("\033[1;35m========================================\033[0m\n\n");
    
    printf("Конфигурация:\n");
    printf("  Читателей: %d\n", NUM_READERS);
    printf("  Писателей: 1\n");
    printf("  Время работы: %d секунд\n", RUN_TIME);
    printf("  Максимальное число записей: %d\n\n", MAX_WRITES);
    
    // Создание потока мониторинга
    pthread_create(&monitor, NULL, monitor_func, NULL);
    usleep(100000);  // Даем монитору запуститься
    
    // Создание потока писателя
    if (pthread_create(&writer, NULL, writer_func, NULL) != 0) {
        perror("Ошибка создания писателя");
        return EXIT_FAILURE;
    }
    
    // Создание потоков читателей
    for (int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i + 1;
        if (pthread_create(&readers[i], NULL, reader_func, &reader_ids[i]) != 0) {
            perror("Ошибка создания читателя");
            running = 0;
            
            // Ожидаем уже созданные потоки
            for (int j = 0; j < i; j++) {
                pthread_join(readers[j], NULL);
            }
            pthread_join(writer, NULL);
            pthread_join(monitor, NULL);
            
            return EXIT_FAILURE;
        }
        usleep(50000);  // Небольшая задержка между созданием читателей
    }
    
    printf("\n\033[1;32mВсе потоки созданы. Программа работает...\033[0m\n\n");
    
    // Ожидаем указанное время
    sleep(RUN_TIME);
    
    // Сигнал о завершении
    printf("\n\033[1;33mЗавершаем работу...\033[0m\n");
    running = 0;
    
    // Разбудим все ожидающие потоки
    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&data_ready);
    pthread_cond_broadcast(&data_processed);
    pthread_mutex_unlock(&mutex);
    
    // Ожидание завершения всех потоков
    pthread_join(writer, NULL);
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    pthread_join(monitor, NULL);
    
    // Вывод итоговой статистики
    printf("\n\033[1;35m========================================\033[0m\n");
    printf("\033[1;35m           ИТОГОВАЯ СТАТИСТИКА          \033[0m\n");
    printf("\033[1;35m========================================\033[0m\n\n");
    
    printf("Общая статистика:\n");
    printf("  Всего произведено записей: %d\n", write_counter);
    printf("  Писатель ждал читателей: %d раз(а)\n", writer_waits);
    
    int total_reads = 0;
    printf("\nСтатистика по читателям:\n");
    for (int i = 0; i < NUM_READERS; i++) {
        printf("  Читатель %2d: %3d операций чтения\n", 
               i + 1, reads_per_reader[i]);
        total_reads += reads_per_reader[i];
    }
    
    printf("\nИтоги:\n");
    printf("  Всего операций чтения: %d\n", total_reads);
    printf("  Среднее на читателя: %.1f\n", (float)total_reads / NUM_READERS);
    
    if (write_counter > 0) {
        printf("  Отношение чтений/записей: %.1f:1\n", 
               (float)total_reads / write_counter);
    }
    
    printf("\n\033[1;32mПрограмма завершена успешно!\033[0m\n");
    
    // Очистка ресурсов
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&data_ready);
    pthread_cond_destroy(&data_processed);
    
    return EXIT_SUCCESS;
}