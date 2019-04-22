# 开发版本v1.2（验证版）2018/7/25 ：
[![Badge](https://img.shields.io/badge/link-996.icu-%23FF4D5B.svg?style=flat-square)](https://996.icu/#/zh_CN)
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg?style=flat-square)](https://github.com/996icu/996.ICU/blob/master/LICENSE)
[![Slack](https://img.shields.io/badge/slack-996icu-green.svg?style=flat-square)](https://join.slack.com/t/996icu/shared_invite/enQtNTg4MjA3MzA1MzgxLWQyYzM5M2IyZmIyMTVjMzU5NTE5MGI5Y2Y2YjgwMmJiMWMxMWMzNGU3NDJmOTdhNmRlYjJlNjk5ZWZhNWIwZGM)
[![HitCount](http://hits.dwyl.io/996icu/996.ICU.svg)](http://hits.dwyl.io/996icu/996.ICU)
（8/15）暂停更新 至 测试版

本开发为路由器端的硬件软件开发。不涉及接入端设备。

## 硬件部分

## 1.硬件系统平台

- 主控芯片 ALLWinner A64 a63
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

- ESP8266-01S `UART` `AP模式`
- SX1278 `SPI`
- NRF52832 `UART`
- ~~MAX485 `UART` `临时添加`~~
- ~~W5500  `SPI`~~

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

### 配置为AP AT命令：(注意结尾 '\r' '\n')

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

## 6.模块监控模式

对com端口循环连接测试，测试通过则下一步。

采用epoll/select方式监测文件描述符。

## 7.控制功能

采用前端页面控制

远程控制必须拥有：WideBand，WIFI，4G模块 之一

本地控制必须拥有：WideBand，WIFI 之一

单向数据传输必须包含：WideBand，WIFI，4G模块，NB-IOT模块 之一

## 8.本地网络通讯协议

| 字节  | 意义       | 内容 |
| ----- | :--------- | ---- |
| 0     | 起始位     | 0xFF |
| 1     | 协议版本号 | 当前采用0x01     |
| 2     | 协议序列号| 由接入端生成|
| 3-4 | 源id       | 共12字节     |
| 5-6 | 目标id     | 共12字节     |
| 7    | 协议类型   | 注册：0x01 （需配合目的id进行操作）确认注册：0x02 心跳0x03 确认心跳0x04 数据请求0x05 确认请求0x06 错误：0xFF |
| 8    | 数据段长度 | 0-255     |
| 9-   |   数据段   | 最大225 |

