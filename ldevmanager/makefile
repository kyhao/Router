#esp8266makefile
CC = gcc

all : run clean
run : r_main.o usbctl.o esp8266.o
	$(CC) -o run r_main.o usbctl.o esp8266.o

r_main.o : r_main.c
	$(CC) -c r_main.c

usbctl.o : usbctl.c
	$(CC) -c usbctl.c

esp8266.o: esp8266.c
	$(CC) -c esp8266.c

.PHONY:clean
clean :
	rm *.o