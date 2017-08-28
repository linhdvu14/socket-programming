ftserver.o: ftserver.c ftserver-utils.h
	gcc -c ftserver.c

ftserver-utils.o: ftserver-utils.c ftserver-utils.h
	gcc -c ftserver-utils.c

ftserver: ftserver.o ftserver-utils.o
	gcc -g ftserver.o ftserver-utils.o -o ftserver

clean: 
	rm -f *.o

all: ftserver clean