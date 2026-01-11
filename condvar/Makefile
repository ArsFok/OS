CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200112L
TARGET = lab11_condvar
SOURCE = main.c

# Цвета для вывода
RED=\033[0;31m
GREEN=\033[0;32m
YELLOW=\033[1;33m
BLUE=\033[0;34m
PURPLE=\033[0;35m
CYAN=\033[0;36m
NC=\033[0m # No Color

.PHONY: all clean run debug test help

all: $(TARGET)

$(TARGET): $(SOURCE)
	@echo "${PURPLE}Компиляция лабораторной работы №11...${NC}"
	@echo "${CYAN}(Условные переменные - condvar)${NC}"
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)
	@echo "${GREEN}✓ Готово! Исполняемый файл: ${YELLOW}$(TARGET)${NC}"

run: $(TARGET)
	@echo "${PURPLE}Запуск программы...${NC}"
	@echo "${PURPLE}================================${NC}"
	@./$(TARGET)

debug: $(SOURCE)
	@echo "${YELLOW}Компиляция с отладочной информацией...${NC}"
	$(CC) $(CFLAGS) -g -o $(TARGET)_debug $(SOURCE)
	@echo "${GREEN}✓ Готово! Для отладки: ${CYAN}gdb $(TARGET)_debug${NC}"

test: $(TARGET)
	@echo "${YELLOW}Запуск тестового прогона (5 секунд)...${NC}"
	@timeout 5 ./$(TARGET) || true
	@echo "${GREEN}✓ Тест завершен${NC}"

clean:
	@echo "${YELLOW}Очистка...${NC}"
	rm -f $(TARGET) $(TARGET)_debug *.o
	@echo "${GREEN}✓ Готово!${NC}"

valgrind: $(TARGET)
	@echo "${YELLOW}Запуск с Valgrind (проверка утечек памяти)...${NC}"
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

help:
	@echo "${PURPLE}Доступные команды:${NC}"
	@echo "  ${CYAN}make all${NC}     - скомпилировать программу"
	@echo "  ${CYAN}make run${NC}     - скомпилировать и запустить"
	@echo "  ${CYAN}make debug${NC}   - скомпилировать с отладочной информацией"
	@echo "  ${CYAN}make test${NC}    - запустить короткий тест (5 сек)"
	@echo "  ${CYAN}make valgrind${NC}- проверить утечки памяти"
	@echo "  ${CYAN}make clean${NC}   - удалить скомпилированные файлы"
	@echo "  ${CYAN}make help${NC}    - показать эту справку"