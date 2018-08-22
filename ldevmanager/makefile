CC = gcc

all : run clean
#run : linfoctl.o l_main.o usbctl.o localProtocol.o esp8266.o bluetooth.o routetable.o
#	$(CC) -o run linfoctl.o l_main.o usbctl.o localProtocol.o esp8266.o bluetooth.o routetable.o

run : l_main.o usbctl.o localProtocol.o esp8266.o bluetooth.o routetable.o
	$(CC) -o run  l_main.o usbctl.o localProtocol.o esp8266.o bluetooth.o routetable.o

r_main.o : l_main.c
	$(CC) -c l_main.c

usbctl.o : driver/usbctl.c
	$(CC) -c driver/usbctl.c

esp8266.o: driver/esp8266.c
	$(CC) -c driver/esp8266.c

bluetooth.o: driver/bluetooth.c
	$(CC) -c driver/bluetooth.c

localProtocol.o : ../tools/localProtocol.c
	$(CC) -c ../tools/localProtocol.c

#linfoctl.o : linfoctl.c
#	$(CC) -c linfoctl.c

routetable.o : routetable.c
	$(CC) -c routetable.c

.PHONY:clean
clean :
	rm *.o