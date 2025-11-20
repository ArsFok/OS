.PHONY: all clean 

all: mychmod

chmod.o: chmod.c
	gcc -g -c chmod.c -Wall -Wextra -o chmod.o

mychmod: chmod.o
	gcc chmod.o -Wall -Wextra -o mychmod
	rm *.o

clean: 
	rm -f *.o mychmod
