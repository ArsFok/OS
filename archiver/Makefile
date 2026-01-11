CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = archiver
OBJS = archiver.o archive_io.o archive_ops.o archive_utils.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

archiver.o: archiver.c archive.h
	$(CC) $(CFLAGS) -c archiver.c

archive_io.o: archive_io.c archive.h
	$(CC) $(CFLAGS) -c archive_io.c

archive_ops.o: archive_ops.c archive.h
	$(CC) $(CFLAGS) -c archive_ops.c

archive_utils.o: archive_utils.c archive.h
	$(CC) $(CFLAGS) -c archive_utils.c

clean:
	rm -f $(TARGET) $(OBJS)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(TARGET)

.PHONY: all clean install uninstall