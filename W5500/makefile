CC = gcc
#
all : run clean
run : dhcp.c socket.c W5500_spi_config_wiringpi.c w5500.o wizchip_conf.c
	$(CC) -o run dhcp.c socket.c W5500_spi_config_wiringpi.c w5500.c wizchip_conf.c -l wiringPi


.PHONY:clean
clean :
	rm *.o