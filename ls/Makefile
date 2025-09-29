.PHONY: all clean 

all: myls

ls.o: ls.c
	gcc -g -c ls.c -Wall -Wextra -o ls.o

myls: ls.o
	gcc ls.o -Wall -Wextra -o myls
	rm *.o

clean: 
	rm -f *.o myls
