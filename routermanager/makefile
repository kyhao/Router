CC = gcc

all : run clean
run : r_main.o localProtocol.o linfoctl.o
	$(CC) -o run r_main.o localProtocol.o linfoctl.o

r_main.o : r_main.c
	$(CC) -c r_main.c

.PHONY:clean
clean :
	rm *.o