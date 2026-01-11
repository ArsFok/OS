CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L
LDFLAGS = 

# Цвета для вывода
GREEN = \033[0;32m
YELLOW = \033[0;33m
RED = \033[0;31m
NC = \033[0m # No Color

all: sender receiver

sender: sender.c
	@echo "$(GREEN)Компиляция отправителя...$(NC)"
	$(CC) $(CFLAGS) -o sender sender.c $(LDFLAGS)
	@echo "$(GREEN)Готово: sender$(NC)"

receiver: receiver.c
	@echo "$(GREEN)Компиляция получателя...$(NC)"
	$(CC) $(CFLAGS) -o receiver receiver.c $(LDFLAGS)
	@echo "$(GREEN)Готово: receiver$(NC)"

run-sender: sender
	@echo "$(YELLOW)Запуск отправителя...$(NC)"
	@echo "$(YELLOW)Используйте Ctrl+C для завершения$(NC)"
	./sender

run-receiver: receiver
	@echo "$(YELLOW)Запуск получателя...$(NC)"
	@echo "$(YELLOW)Используйте Ctrl+C для завершения$(NC)"
	./receiver

clean:
	@echo "$(RED)Очистка...$(NC)"
	rm -f sender receiver
	rm -f /tmp/lab7_sender.pid
	@echo "$(RED)Очистка разделяемой памяти...$(NC)"
	@-ipcrm -M 0x7A7A7A7A 2>/dev/null || true

help:
	@echo "Доступные команды:"
	@echo "  make all          - Скомпилировать все программы"
	@echo "  make sender       - Скомпилировать отправитель"
	@echo "  make receiver     - Скомпилировать получателя"
	@echo "  make run-sender   - Запустить отправитель"
	@echo "  make run-receiver - Запустить получателя"
	@echo "  make clean        - Удалить скомпилированные файлы и временные файлы"
	@echo "  make help         - Показать эту справку"
	@echo ""
	@echo "Порядок работы:"
	@echo "  1. make all"
	@echo "  2. В первом терминале: make run-sender"
	@echo "  3. Во втором терминале: make run-receiver (можно запускать несколько раз)"
	@echo "  4. Для завершения: Ctrl+C в терминале с отправителем"

.PHONY: all clean help run-sender run-receiver