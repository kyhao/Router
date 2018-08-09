// 通用USB串口控制 2018/7/17
// 阻塞方式实现
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "usbctl.h"

// 等待时间和最小接收字符
#define VMIN_GET 1
#define VTIME_GET 0

#define READ_ONCE_SIZE 16 // 默认一次读取数据大小（字节）

static int ret;

// 打开端口/串口
// 阻塞方式
// @param
// @comport    想要打开的串口号 串口不超过9
// @return     返回 -1 为打开失败
int open_port(int comport)
{
    int fd;
    char dev[13] = "/dev/ttyUSB0";
    if (comport < 10)
    {
        dev[11] = (char)(48 + comport);
    }
    else
    {
        dev[11] = (char)(48 + comport / 10);
        dev[12] = (char)(48 + comport % 10);
    }

    fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY | O_RSYNC);
    if (-1 == fd)
    {
        perror("Can't Open Serial Port");
        return (-1);
    }

    // 恢复串口为阻塞状态
    if (fcntl(fd, F_SETFL, 0) < 0)
        printf("fcntl failed!\n");
    else
        printf("fcntl=%d\n", fcntl(fd, F_SETFL, 0));
    // 测试是否为终端设备
    if (isatty(STDIN_FILENO) == 0)
        printf("standard input is not a terminal device\n");
    else
        printf("isatty success!\n");
    printf("fd-open=%d\n", fd);
    return fd;
}

// 串口配置
// @param
// @fd      文件描述符
// @nSpeed  波特率选项< 2400 4800 9600 115200 460800 > 若不为上述选项，则自动设为9600
// @nBits   数据位数选项 < 7 8 >
// @nEvent  奇偶检验为选项 < o e n>
// @nStop   设置停止位 < 1 2 >
int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio, oldtio;
    // 保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息
    if (tcgetattr(fd, &oldtio) != 0)
    {
        perror("SetupSerial 1");
        printf("tcgetattr( fd,&oldtio) -> %d\n", tcgetattr(fd, &oldtio));
        return -1;
    }
    bzero(&newtio, sizeof(newtio)); // 类似memset
    // 设置字符大小
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;
    // 设置停止位
    switch (nBits)
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }
    // 设置奇偶校验位
    switch (nEvent)
    {
    case 'o':
    case 'O': // 奇数
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'e':
    case 'E': // 偶数
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'n':
    case 'N': // 无奇偶校验位
        newtio.c_cflag &= ~PARENB;
        break;
    default:
        break;
    }
    // 设置波特率
    switch (nSpeed)
    {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    case 460800:
        cfsetispeed(&newtio, B460800);
        cfsetospeed(&newtio, B460800);
        break;
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }
    // 设置停止位
    if (nStop == 1)
        newtio.c_cflag &= ~CSTOPB;
    else if (nStop == 2)
        newtio.c_cflag |= CSTOPB;
    // 设置等待时间和最小接收字符
    newtio.c_cc[VTIME] = VTIME_GET;
    newtio.c_cc[VMIN] = VMIN_GET;
    // 处理未接收字符
    tcflush(fd, TCIFLUSH);
    // 激活新配置
    if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
    {
        perror("com set error");
        return -1;
    }
    printf("set done!\n");
    return 0;
}

// 写串口
// @param
// @fd 文件描述符
// @buf 写缓存
// @len 写数据长度(字节)
int write_port(int fd, const void *buf, size_t len)
{
    ret = write(fd, buf, len);
    if (ret == -1)
        return ret;
    return 0;
}

// 读串口len长度数据
// 阻塞模式
// @param
// @fd 文件描述符
// @buf 读缓存
// @len 读数据长度(字节)
// @return 错误值
int read_port(int fd, void *buf, size_t len)
{
    ret = read(fd, buf, len);
    if (ret == -1)
        return ret;
    tcflush(fd, TCIFLUSH);
    return 0;
}

// 读串口全部数据
// 阻塞模式
// @param
// @fd 文件描述符
// @buf 读缓存
// @len 获取的数据长度(字节)
// @return 错误值
int read_port_all(int fd, void *buf, size_t *len)
{
    char tbuf[READ_ONCE_SIZE];
    // TODO: 完成获取全部字符
    ret = read(fd, tbuf, READ_ONCE_SIZE);
    while (ret == READ_ONCE_SIZE)
    {
    }
    return 0;
}