/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static void pabort(const char *s)
{
    perror(s);
    abort();
}

static const char *device[] = {"/dev/spidev0.0", "/dev/spidev0.1"};
static uint8_t mode = SPI_MODE_0;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay = 0;

int spi_write_read(int fd, void *tx, void *rx, int len)
{
    int ret;
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = len,
        .delay_usecs = delay,
        .speed_hz = speed,
        .bits_per_word = bits,
    };
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr); // 传输一次
    if (ret < 1)
        pabort("can't send/read spi message");
    return ret;
}

int spi_config(int fd)
{
    int ret;
    // spi mode
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
        pabort("can't set spi mode");
    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1)
        pabort("can't get spi mode");
    // bits per word
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't set bits per word");
    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't get bits per word");
    // max speed hz
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't set max speed hz");
    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't get max speed hz");
    printf("spi mode: %d\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d Hz (%d KHz)\n", speed, speed / 1000);
    printf("SPI INIT SUCCESS\n");
    return ret;
}

int open_set_dev(int num){
    
}

int main(int argc, char *argv[])
{
    int ret = 0;
    int fd;

    fd = open(device[0], O_RDWR);
    if (fd < 0)
        pabort("can't open device");

    ret = spi_config(fd);

    uint8_t tx[] = {
        0x01,
        0x80,
    };
    uint8_t rx[ARRAY_SIZE(tx)] = {
        0,
    };

    ret = spi_write_read(fd, tx, 0, ARRAY_SIZE(tx));
    ret = spi_write_read(fd, 0, rx, 10);
    close(fd);
    return ret;
}

// spi控制主要就是一个结构体和一个ioctl函数。
// 配置结构体后，调用ioctl函数控制