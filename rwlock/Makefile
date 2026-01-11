CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread -D_DEFAULT_SOURCE
TARGET = lab10_rwlock
SOURCE = main.c

# Цвета для вывода
RED=\033[0;31m
GREEN=\033[0;32m
YELLOW=\033[1;33m
NC=\033[0m # No Color

.PHONY: all clean run debug

all: $(TARGET)

$(TARGET): $(SOURCE)
	@echo "${YELLOW}Компиляция лабораторной работы №10...${NC}"
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)
	@echo "${GREEN}Готово! Исполняемый файл: $(TARGET)${NC}"

run: $(TARGET)
	@echo "${YELLOW}Запуск программы...${NC}"
	@echo "${YELLOW}================================${NC}"
	./$(TARGET)

debug: $(SOURCE)
	@echo "${YELLOW}Компиляция с отладочной информацией...${NC}"
	$(CC) $(CFLAGS) -g -o $(TARGET)_debug $(SOURCE)
	@echo "${GREEN}Готово! Для отладки: gdb $(TARGET)_debug${NC}"

clean:
	@echo "${YELLOW}Очистка...${NC}"
	rm -f $(TARGET) $(TARGET)_debug
	@echo "${GREEN}Готово!${NC}"

help:
	@echo "${YELLOW}Доступные команды:${NC}"
	@echo "  make all    - скомпилировать программу"
	@echo "  make run    - скомпилировать и запустить"
	@echo "  make debug  - скомпилировать с отладочной информацией"
	@echo "  make clean  - удалить скомпилированные файлы"
	@echo "  make help   - показать эту справку"