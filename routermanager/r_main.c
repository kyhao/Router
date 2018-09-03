// 2018/9/3
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>

// 驱动模块
#include <driver/esp8266.h>
#include <driver/bluetooth.h>
#include <driver/lora.h>
//#include <driver/nb.h>
//#include <driver/module4g.h>

int main(int argc, char **argv)
{
    printf("r_main_argc:->%d\n", argc);

    fd_set fdset;
    int ret;
    char rx_buf[RX_SIZE]; // 接收缓存大小

    while (1)
    {
        FD_ZERO(&fdset);
        FD_SET(pfd_in, &fdset);
        ret = select(pfd_in + 1, &fdset, NULL, NULL, NULL);
        if (ret > 0)
        {
            // 监测是否有来自硬件端协议传输
            if (FD_ISSET(pfd_in, &fdset))
            {
                ret = read(pfd_in, rx_buf, 100);

                // FIXME: DEBUG:
                int j;
                printf("GET_FORM_LDEV: ");
                for (j = 0; j < ret; j++)
                {
                    printf("\033[33m%02X \033[0m", rx_buf[j]);
                }
                printf("\n");
                write(pfd_out, "AAAAAA", 5);
            }
        }
    }
    return 0;
}
