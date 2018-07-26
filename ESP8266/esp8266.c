#include "usbctl.h"
#include "esp8266.h"

#define Conn(x, y) x##y
#define ToChar(x) #@ x
#define ToString(x) #x

#define AT #AT
#define AT_RST #AT_RST
#define AT_CWMODE(mode) ToString(AT + CWMODE = ##mode##\r\n)
#define SERVERPORT 8080

// esp8266 配置为热点模式
// 设备开启多连接 服务器模式
// 热点名称 “ESP8266TEST” 密码“12345678”
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
