#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include "usbctl.h"
#include "esp8266.h"

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
            ret = read_port(fd, RX_buf, 100);
            if (ret == -1)
                printf("read error");
            printf("\n%s\n", RX_buf);
        }
        close(fd);
    }
    return 0;
}