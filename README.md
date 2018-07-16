# 开发版本v1.1（验证版）2018/7/16 ：

## 1.系统环境

- 系统：raspbian-stretch(Linux)
- 内核版本：Linux raspberrypi 4.14.50-v7+ #1122 SMP Tue Jun 19 12:26:26 BST 2018 armv7l GNU/Linux</br>`系统内核适配GPIO可在 /sys/class/gpio 下查看`
- Python版本：Pyhton3.6(CPython)
- GCC版本：gcc version 6.3.0 20170516 (Raspbian 6.3.0-18+rpi1+deb9u1)

### 依赖库

- wiringpi
- wiringPiSPI

## 2.硬件平台

主控：

- Raspberry Pi 3 Model B+

通信模块`数量8`：

- W5500 * 2  `SPI`
- AX88772 (暂定)  `USB`
- BC95-B5 `UART`
- ME909s-821(mirco-PCIE)  `mirco-PCIE to USB`
- ESP-01S `UART`
- SX1278 `SPI`
- NRF52832 `UART`
- MAX485 `UART`

中间件：

- ~~STC15W408~~ `UART <---> SPI`
- CH340  `UART <---> USB`

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