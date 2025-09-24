.PHONY: all clean 

all: mycat mygrep

cat.o: cat.c grep.c
	gcc -g -c cat.c -Wall -Wextra -o cat.o

grep.o: grep.c
	gcc -g -c grep.c -Wall -Wextra -o grep.o

mycat: cat.o
	gcc cat.o -Wall -Wextra -o mycat
	rm *.o

mygrep: grep.o
	gcc grep.o -Wall -Wextra -o mygrep
	rm *.o

clean: 
	rm -f *.o mycat mygrep
