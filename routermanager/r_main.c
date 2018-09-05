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
    fd_set fdset;

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
                
            }
        }
    }
    return 0;
}
