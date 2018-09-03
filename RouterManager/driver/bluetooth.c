// 2018/7/26
// 蓝牙检测文件
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>

#include "bluetooth.h"
#include "usbctl.h"

// 遍历com端口，寻找bluetooth设备并打开
// 采用 select 方式检测超时
// 先发送 AT 指令测试，测试是否启动，再发送 AT+RST 指令进行重启操作，重启成功，完成配对
// @param
// @return 找到并返回文件描述符，若未找到，返回-1
int bluetooth_open()
{
    int fd, ret;
    char *p;
    int comport = 0;
    char buf[33];

    // select 方式
    struct timeval timeout;
    fd_set fdset;
    FD_ZERO(&fdset);

    // 遍历com口
    for (; comport < 10; comport++)
    {
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        fd = open_port(comport);
        if (-1 == fd)
        {
            continue;
        }
        // 配置USB端口
        ret = set_opt(fd, 115200, 8, 'n', 1);

        FD_SET(fd, &fdset);
        // 开始测试 bluetooth设备 AT指令
        // 检测设备启动
        write_port(fd, "AT\r\n", 4);
        sleep(1);
        // 监听事件 1秒钟 超时跳过
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
                ret = read_port(fd, buf, 33);
                if (ret > 0)
                {
                    // 检查AT指令返回值
                    p = strstr(buf, "BLUETOOTH");
                    if (p != NULL)
                    {
                        return fd;
                    }
                }
            }
        }
        FD_CLR(fd, &fdset);
        close(fd);
    }
    return -1;
}

int getbtinfo()
{
}

int getbtsta()
{
}