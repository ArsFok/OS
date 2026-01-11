CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread -D_DEFAULT_SOURCE
TARGET = threads_program
SOURCES = main.c common.c writer.c reader.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = common.h writer.h reader.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)