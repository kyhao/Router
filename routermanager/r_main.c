// 2018/8/15
// 消息控制
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
// #include "localProtocol.h"
#include "linfoctl.h"

#define PIPE_NUM 2  // 管道数量
#define RX_SIZE 128 // 接受缓存大小

int main(int argc, char **argv)
{
    int pfd_out, pfd_in;
    printf("r_main_argc:->%d\n", argc);
    if (argc != PIPE_NUM + 1)
    {
        // TODO: 错误日志处理
        // 无输出管道处理
    }
    else
    {
        // 奇淫技巧 减少参数的计算量 采用二进制传输 忽略字符特性
        pfd_in = (int)argv[1][0];
        pfd_out = (int)argv[2][0];
        if (pfd_in > 1000 || pfd_out > 1000) // 对fd判断，若大于监控数量则错误
        {
            pfd_out = -1;
            pfd_in = -1;
        }
    }

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
