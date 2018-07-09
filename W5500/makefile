W5500_Path = /home/pi/W5500/Ethernet

run : $(W5500_Path)/wizchip_conf.c $(W5500_Path)/socket.c $(W5500_Path)/W5500/w5500.c W5500_spi_config.c
	gcc W5500_spi_config.c $(W5500_Path)/wizchip_conf.c $(W5500_Path)/socket.c $(W5500_Path)/W5500/w5500.c -o run -I $(W5500_Path)