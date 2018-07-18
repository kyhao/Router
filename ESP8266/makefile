CC = gcc

all : run clean
run : r_main.o usbctl.o
	$(CC) -o run r_main.o usbctl.o

r_main.o : r_main.c
	$(CC) -c r_main.c

usbctl.o : usbctl.c usbctl.h
	$(CC) -c usbctl.c

.PHONY:clean
clean :
	rm *.o