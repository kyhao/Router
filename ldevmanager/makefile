CC = gcc

all : run clean
run : l_main.o usbctl.o esp8266.o bluetooth.o
	$(CC) -o run l_main.o usbctl.o esp8266.o bluetooth.o

r_main.o : l_main.c
	$(CC) -c l_main.c

usbctl.o : usbctl.c
	$(CC) -c usbctl.c

esp8266.o: esp8266.c
	$(CC) -c esp8266.c

bluetooth.o: bluetooth.c
	$(CC) -c bluetooth.c

.PHONY:clean
clean :
	rm *.o