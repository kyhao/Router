// ESP8266设备控制文件
// 由于USB串口配置为阻塞模式，我们采用select方式监听超时
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>
#include "usbctl.h"
#include "esp8266.h"

#define ToString(x) #x
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define AT #AT
#define AT_RST #AT_RST
#define AT_CWMODE(mode) ToString(AT + CWMODE = ##mode##\r\n)
#define SERVERPORT 8080

// 选择使用的的协议
#define USE_PROTOCOL USE_UDP
#define USE_UDP 0 // 标识使用 0 UDP协议 1 TCP协议
#define USE_TCP 1

// 遍历com端口，寻找esp8266设备并打开
// 采用 select 方式检测超时
// 先发送 AT 指令测试，测试是否启动，再发送 AT+RST 指令进行重启操作，重启成功，完成配对
// @param
// @return 找到并返回文件描述符，若未找到，返回-1
int esp8266_open()
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
        // 开始测试 esp8266设备 AT指令
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
                    p = strstr(buf, "OK");
                    if (p != NULL)
                    {
                        // 重启设备
                        write_port(fd, "AT+RST\r\n", 8);
                        sleep(1);
                        ret = read(fd, buf, 33);
                        // 检查 AT+RST 指令返回值
                        p = strstr(buf, "OK");
                        if (p == NULL)
                        {
                            close(fd);
                            continue;
                        }
                        while (read(fd, buf, 33))
                        {
                            // 检查 AT+RST 指令(重启)是否执行成功
                            p = strstr(buf, "ready");
                            if (p != NULL)
                            {
                                // 完成检测 返回文件描述符
                                tcflush(fd, TCIFLUSH); // 刷新输入缓冲区
                                return fd;
                            }
                        }
                    }
                }
            }
        }
        FD_CLR(fd, &fdset);
        close(fd);
    }
    return -1;
}

// esp8266 配置为热点模式
// 设备开启多连接 服务器模式
// 原版本为 TCP 服务器，当前更换为UDP服务器， 丢包问题由上层解决 2018/8/14
// 热点名称 “ESP8266TEST” 密码“12345678”
// @param
// @fd 文件描述符
int esp8266_config(int fd)
{
    //char TX_buf[8] = {'A', 'T', '+', 'R', 'S', 'T', '\r', '\n'};
    char RST[] = "AT+RST\r\n";
    char CWMODE[] = "AT+CWMODE=2\r\n";
    char CWSAP[] = "AT+CWSAP=\"ESP8266TEST\",\"12345678\",1,3\r\n";
#if USE_PROTOCOL == USE_UDP
    char CIPSTART[] = "AT+CIPSTART=\"UDP\",\"192.168.4.255\",8080,9090,0\r\n";
    char CIPMODE[] = "AT+CIPMODE=1\r\n";
    char CIPSEND[] = "AT+CIPSEND\r\n";
#else
    char CIPMUX[] = "AT+CIPMUX=1\r\n";
    char CIPSERVER[] = "AT+CIPSERVER=1,8080\r\n";
#endif

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
#if USE_PROTOCOL == USE_UDP
    write_port(fd, CIPSTART, strlen(CIPSTART));
    sleep(1);
    write_port(fd, CIPMODE, strlen(CIPMODE));
    sleep(1);
    write_port(fd, CIPSEND, strlen(CIPSEND));
    sleep(1);
#else
    write_port(fd, CIPMUX, strlen(CIPMUX));
    sleep(1);
    write_port(fd, CIPSERVER, strlen(CIPSERVER));
    sleep(1);
#endif
    tcflush(fd, TCIOFLUSH); // 刷新输入输出缓冲区
    return 0;
}
