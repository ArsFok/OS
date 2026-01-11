CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_DEFAULT_SOURCE
TARGET_SENDER = sender
TARGET_RECEIVER = receiver
COMMON_HEADER = common.h

.PHONY: all clean run kill_ipc

all: $(TARGET_SENDER) $(TARGET_RECEIVER)

$(TARGET_SENDER): sender.c $(COMMON_HEADER)
	$(CC) $(CFLAGS) -o $(TARGET_SENDER) sender.c

$(TARGET_RECEIVER): receiver.c $(COMMON_HEADER)
	$(CC) $(CFLAGS) -o $(TARGET_RECEIVER) receiver.c

run: $(TARGET_SENDER) $(TARGET_RECEIVER)
	@echo "Запуск в разных терминалах:"
	@echo "1. В первом терминале: ./$(TARGET_RECEIVER)"
	@echo "2. Во втором терминале: ./$(TARGET_SENDER)"
	@echo ""
	@echo "Или последовательно:"
	@echo "./$(TARGET_RECEIVER) &"
	@sleep 2
	@./$(TARGET_SENDER)

clean:
	rm -f $(TARGET_SENDER) $(TARGET_RECEIVER)

kill_ipc:
	-ipcrm -M $(SHM_KEY) 2>/dev/null || true
	-ipcrm -S $(SEM_KEY) 2>/dev/null || true
	@echo "Ресурсы IPC удалены"

# Для очистки всех IPC объектов (осторожно!)
clean_all: clean kill_ipc
	@echo "Все файлы и IPC ресурсы удалены"