CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread -D_DEFAULT_SOURCE
TARGET = task9_1
SOURCE = main.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)