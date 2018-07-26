# 开发版本v1.2（验证版）2018/7/25 ：

## 1.系统环境

- 系统：raspbian-stretch(Linux)
- 内核版本：Linux raspberrypi 4.14.50-v7+ #1122 SMP Tue Jun 19 12:26:26 BST 2018 armv7l GNU/Linux</br>`系统内核适配GPIO可在 /sys/class/gpio 下查看`
- Python版本：Pyhton3.6(CPython)
- GCC版本：gcc version 6.3.0 20170516 (Raspbian 6.3.0-18+rpi1+deb9u1)

### 依赖库

- spidev

## 2.硬件平台

主控：

- Raspberry Pi 3 Model B+

通信模块：

外网：

- RTL8211E `板载系统驱动`
- AP6212 `板载系统驱动`
- ME909s-821  `mirco-PCIE` `系统驱动`
- BC95-B5 `UART` `单向`
- ~~AX88772`USB` `网卡`~~  

内网：

- W5500  `SPI`
- ESP8266-01S `UART` `AP模式`
- SX1278 `SPI`
- NRF52832 `UART`
- MAX485 `UART` `临时添加`

中间件：

- CH340  `UART <---> USB` `系统驱动` `系统UART接口不足`
- mirco-PCIE转USB转接板 `mirco-PCIE <---> USB`
- ~~STC15W408 `UART <---> SPI`~~  

## 3.文件介绍

代码文件：  
t_xxx.xx文件为模块测试代码  
r_xxx.xx文件为正式代码  
h_xxx.xx文件为查询文件

## 4.W5500模块

文件夹结构：  
.  
+-- W5500  
|   +-- Ethernet  
|   +-- Internet  

W5500自带完整的协议栈，W5500文件夹为W5500驱动文件，分为Ethernet和Internet，Ethernet包含W5500驱动，socket控制，wizchip_conf通信配置。Internet包含DHCP和DNS控制。

## 5.ESP8226模块

配置为AP AT命令：(注意结尾 '\r' '\n')
AT+RST
AT+CWMODE=2 `配置为AP模式` 1,2,3 三个模式
AT+RST
AT+CWSAP=<ssid>,<pwd>,<chi>,<ecn>
指令只有在 AP模式开启后有效:
<ssid>字符串参数，接入点名称（路由器发送的无线信号的名字）

<pwd>字符串参数，密码最长 64 字节ASCII

<chl>通道号 
<ecn>
0 OPEN;开放不加密
1 WEP;加密
2 WPA_PSK 
3 WPA2_PSK 
4 WPA_WPA2_PSK

AT+RST

AT+CIPMUX=1 启用多路连接

AT+CIPSERVER=<mode>[,<port>] 配置为服务器

AT+CIPSEND=<id>,<length> 作为服务器向 编号为id的设备

## 模块监控模式

对com端口循环连接测试，测试通过则下一步。

采用epoll/select方式监测文件描述符。
