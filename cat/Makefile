.PHONY: all clean 

all: mycat

main.o: main.c
	gcc -c main.c -Wall -Wextra -o main.o

mycat: main.o
	gcc main.o -Wall -Wextra -o mycat
	rm *.o

clean: 
	rm -f *.o mycat
