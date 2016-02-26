

all: Command.o smallsh.o
	gcc -o smallsh Command.o smallsh.o

smallsh.o:
	gcc -c smallsh.c

Command.o: Command.c Command.h 
	gcc -c Command.c


clean:
	rm -r smallsh Command.o smallsh.o
