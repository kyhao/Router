#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include "usbctl.h"

// esp8266 配置
// @param
// @fd 文件描述符
//
int esp8266_config(int fd)
{
    char TX_buf[8] = {'A', 'T', '+', 'R', 'S', 'T', '\r', '\n'};
    char RST[] = "AT+RST\r\n";
    char CWMODE[] = "AT+CWMODE=2\r\n";
    char CWSAP[] = "AT+CWSAP=\"ESP8266TEST\",\"12345678\",1,3\r\n";
    char CIPMUX[] = "AT+CIPMUX=1\r\n";
    char CIPSERVER[] = "AT+CIPSERVER=1,8080\r\n";

    set_opt(fd, 115200, 8, 'n', 1);

    write_port(fd, RST, 8);
    sleep(1);
    write_port(fd, CWMODE, 13);
    sleep(1);
    write_port(fd, RST, 8);
    sleep(1);
    write_port(fd, CWSAP, 43);
    sleep(1);
    write_port(fd, RST, 8);
    sleep(1);
    write_port(fd, CIPMUX, 13);
    sleep(1);
    write_port(fd, CIPSERVER, 21);

    return 0;
}

int main()
{
    int fd, ret;
    char RX_buf[100];
    fd = open_port(0); // 打开USBCOM0
    if (fd != -1)
    {
        esp8266_config(fd);
        while (1)
        {
            bzero(&RX_buf, sizeof(RX_buf));
            ret = read(fd, RX_buf, 100);
            if (ret == -1)
                printf("read error");
            printf("\n%s\n", RX_buf);
            tcflush(fd, TCIFLUSH); // 刷新输入输出缓冲区
        }

        close(fd);
    }
    return 0;
}