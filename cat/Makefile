.PHONY: all clean 

all: mycat

main.o: main.c
	gcc main.c -Wall -Wextra -o main.o

mycat: main.o
	gcc main.o -Wall -Wextra -o mycat

clean: 
	rm *.o mycat
