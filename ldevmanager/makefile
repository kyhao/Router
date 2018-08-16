CC = gcc

all : run clean
run : linfoctl.o l_main.o usbctl.o localProtocol.o esp8266.o bluetooth.o
	$(CC) -o run linfoctl.o l_main.o usbctl.o localProtocol.o esp8266.o bluetooth.o 

r_main.o : l_main.c
	$(CC) -c l_main.c

usbctl.o : usbctl.c
	$(CC) -c usbctl.c

esp8266.o: esp8266.c
	$(CC) -c esp8266.c

bluetooth.o: bluetooth.c
	$(CC) -c bluetooth.c

localProtocol.o : ../tools/localProtocol.c
	$(CC) -c ../tools/localProtocol.c

linfoctl.o : linfoctl.c
	$(CC) -c linfoctl.c

.PHONY:clean
clean :
	rm *.o