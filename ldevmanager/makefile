CC = gcc

all : run clean
run : lpctl.o l_main.o usbctl.o localProtocol.o esp8266.o bluetooth.o routetable.o lora.o wrap.o
	$(CC) -o run lpctl.o l_main.o usbctl.o localProtocol.o esp8266.o bluetooth.o routetable.o lora.o wrap.o  -lpthread


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

localProtocol.o : localProtocol.c
	$(CC) -c localProtocol.c

lpctl.o : lpctl.c
	$(CC) -c lpctl.c

routetable.o : routetable.c
	$(CC) -c routetable.c

wrap.o:	../wanmanager/wrap.c
	$(CC) -c ../wanmanager/wrap.c

.PHONY:clean
clean :
	rm *.o