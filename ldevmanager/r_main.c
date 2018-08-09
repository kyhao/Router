#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>

#include "usbctl.h"
#include "esp8266.h"

int main()
{
    int fd, ret;
    char RX_buf[100];

    fd = esp8266_open();
    if (-1 == fd)
    {
        printf("->> No esp8266 devcie\n");
        return -1;
    }

    // select 方式
    struct timeval timeout;
    fd_set fdset;

    esp8266_config(fd); // esp8266_配置

    while (1)
    {
        FD_ZERO(&fdset);
        FD_SET(fd, &fdset);
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        ret = select(fd + 1, &fdset, NULL, NULL, &timeout);
        switch (ret)
        {
        case -1:
            printf("->> Error occurs when select() \n");
            break;
        case 0:
            printf("->> Select() timeout \n");
            break;
        default:
            if (FD_ISSET(fd, &fdset))
            {
                ret = read(fd, RX_buf, 100);
                if (ret == -1)
                    printf("->> read error");
                printf("->> %d:%s", ret, RX_buf);
                bzero(&RX_buf, sizeof(RX_buf));
            }
        }
    }

    close(fd);
    return 0;
}