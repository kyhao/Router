#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include "usbctl.h"

int main()
{
    int fd, ret;
    char TX_buf[8] = {'A', 'T', '+', 'R', 'S', 'T', '\r', '\n'};
    char RST[] = "AT+RST\r\n";
    char CWMODE[] = "AT+CWMODE=2\r\n";
    char CWSAP[] = "AT+CWSAP=\"ESP8266TEST\",\"12345678\",1,3\r\n";
    char CIPMUX[] = "AT+CIPMUX=1\r\n";
    char CIPSERVER[] = "AT+CIPSERVER=1,8080\r\n";

    char RX_buf[100];
    fd = open_port(0); // 打开USBCOM1
    if (fd != -1)
    {
        set_opt(fd, 115200, 8, 'n', 1);
        //read(fd, buf, 8);
        printf("%s", TX_buf);
        write(fd, RST, 8);
        sleep(1);
        tcflush(fd, TCIOFLUSH); // 刷新输入输出缓冲区
        write(fd, CWMODE, 13);
        tcflush(fd, TCIOFLUSH); // 刷新输入输出缓冲区
        sleep(1);
        write(fd, RST, 8);
        tcflush(fd, TCIOFLUSH); // 刷新输入输出缓冲区
        sleep(1);
        write(fd, CWSAP, 43);
        tcflush(fd, TCIOFLUSH); // 刷新输入输出缓冲区
        sleep(1);
        write(fd, RST, 8);
        tcflush(fd, TCIOFLUSH); // 刷新输入输出缓冲区
        sleep(1);
        write(fd, CIPMUX, 13);
        tcflush(fd, TCIOFLUSH); // 刷新输入输出缓冲区
        sleep(1);
        // write(fd, RST, 8);
        // tcflush(fd, TCIOFLUSH); // 刷新输入输出缓冲区
        // sleep(1);
        write(fd, CIPSERVER, 21);
        //tcflush(fd, TCIOFLUSH); // 刷新输入输出缓冲区
        sleep(1);
        //write(fd, TX_buf, 8);
        sleep(1);
        //sleep(3);
        ret = read(fd, RX_buf, 100);
        printf("\n%d\n", ret);

        while (1)
        {
            read(fd, RX_buf, 100);
            printf("\n%s\n", RX_buf);
            tcflush(fd, TCIOFLUSH); // 刷新输入输出缓冲区
        }

        if (ret == -1)
            printf("read error");
        printf("\n%s", RX_buf);
        tcflush(fd, TCIOFLUSH); // 刷新输入输出缓冲区
        close(fd);
    }
    return 0;
}