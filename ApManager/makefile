CC = gcc

all : run clean
run : lpctl.o l_main.o usbctl.o localProtocol.o esp8266.o bluetooth.o routetable.o lora.o
	$(CC) -o run lpctl.o l_main.o usbctl.o localProtocol.o esp8266.o bluetooth.o routetable.o lora.o  -lpthread -lsqlite3


r_main.o : l_main.c
	$(CC) -c l_main.c -lpthread

usbctl.o : driver/usbctl.c
	$(CC) -c driver/usbctl.c

esp8266.o: driver/esp8266.c
	$(CC) -c driver/esp8266.c

bluetooth.o: driver/bluetooth.c
	$(CC) -c driver/bluetooth.c

lora.o: driver/lora.c
	$(CC) -c driver/lora.c

localProtocol.o : modules/localProtocol.c
	$(CC) -c modules/localProtocol.c

lpctl.o : lpctl.c
	$(CC) -c lpctl.c

routetable.o : modules/routetable.c
	$(CC) -c modules/routetable.c -lsqlite3 -lpthread

.PHONY:clean
clean :
	rm *.o