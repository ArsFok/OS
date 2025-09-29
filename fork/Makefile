.PHONY: all clean 

all: myfork

fork.o: fork.c
	gcc -g -c fork.c -Wall -Wextra -o fork.o

myfork: fork.o
	gcc fork.o -Wall -Wextra -o myfork
	rm *.o

clean: 
	rm -f *.o myfork
