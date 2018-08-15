CC = gcc

all : run clean
run : r_main.o localProtocol.o
	$(CC) -o run r_main.o localProtocol.o

r_main.o : r_main.c
	$(CC) -c r_main.c

localProtocol.o : localProtocol.c
	$(CC) -c localProtocol.c

.PHONY:clean
clean :
	rm *.o