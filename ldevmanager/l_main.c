// Local Net Device Manager
// Made 2018/8/14
// 未说明的长度默认为字节
// TODO: 设备的插入与中途退出识别检测（即插即用）
// TODO: LoRa设备的接入策略
// TODO: 监测写数据
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>

#include "usbctl.h"
#include "esp8266.h"
#include "bluetooth.h"
// #include "mac.h"

#define DEBUG // 是否开启Debug模式 定义Debug则开启 调试模式

#define PIPE_NUM 2     // 读写管道总个数
#define RX_SIZE 33     // 接收缓存大小（33为最适大小）调大一次接收空间大，但容易浪费。根据测试：在保持数据传输时，一般处于32字节接收窗口
#define PACKET_LEN 255 // 协议包长度（字节）

#define PACKAGE_START 0xFF // 协议包其实字节位
#define PACKAGE_END_0 0x0D // 协议包终止字节位0
#define PACKAGE_END_1 0x0A // 协议包终止字节位1

// 数据接收状态模式标识位
#define FIND_START 0
#define FIND_DATA 1
#define FIND_END 2

#define MAX_FD(x, y) (x > y ? x : y)

// 本地网设备管理 主函数
// @param
// @argc 传入函数的参数个数
// @argv 传入函数参数变量数组 变量为管道通信文件描述符
// @@argv[1] 代表通信硬件模块管理(本程序)与路由业务程序 管道 读
// @@argv[2] 代表通信硬件模块管理(本程序)与路由业务程序 管道 写 与读管道一起实现全双工
// @@argv[3]
// @@argv[4]
int main(int argc, char **argv)
{
    int pfd_out, pfd_in;
    printf("ldev_argc:->%d\n", argc);
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

    // struct timeval timeout;                       // 超时配置
    fd_set fdset;                       // select 数组
    int fd_wifi, fd_bt, fd_lora, fd_nb; // wifi,蓝牙,lora,nb,设备描述符
    int ret;                            // 错误参数保存/临时变量 使用必须注意
    char RX_buf[RX_SIZE];               // 接收缓存

    int i; // 用于函数内部for循环，使用前注意初始化。减少内存申请释放次数

    // esp8266 WIFI设备选择
    fd_wifi = esp8266_open();
    if (-1 == fd_wifi)
    {
        printf("->> No esp8266 devcie\n");
    }
    else
    {
        esp8266_config(fd_wifi); // 成功打开esp8266后配置
    }
    // bluetooth设备选择
    fd_bt = bluetooth_open();
    if (-1 == fd_bt)
    {
        printf("->> No bluetooth devcie\n");
        // return -1;
    }

    // 若设备无法检测与选择则退出
    if (-1 == fd_wifi && -1 == fd_bt)
        return -1;

    char wifi_buf[PACKET_LEN], bt_buf[PACKET_LEN]; // 存放完整协议包
    int wifi_rdsta, bt_rdsta;                      // 设备读取数据状态
    int wifi_rd_pos, bt_rd_pos;                    // 数据协议接收游标
    int fd_max;
    wifi_rd_pos = 0;
    bt_rd_pos = 0;
    wifi_rdsta = FIND_START;
    bt_rdsta = FIND_START;

    int wifi_cal[] = {0, 0};
    int bt_cal[] = {0, 0};
    // 设备正常 通信监听开始
    printf("InitFinished:Start->>>>>%d %d \n", fd_wifi, fd_bt);
    while (1)
    {
        FD_ZERO(&fdset);
        FD_SET(0, &fdset);       // 标准输入监听
        FD_SET(fd_wifi, &fdset); // 将WiFi设备加入监听
        FD_SET(fd_bt, &fdset);   // 将BT设备加入监听
        FD_SET(pfd_in, &fdset);  // 将管道通信 入口 加入监听
        fd_max = MAX_FD(fd_bt, fd_wifi);
        fd_max = MAX_FD(fd_max, pfd_in);
        ret = select(fd_max + 1, &fdset, NULL, NULL, NULL);
        if (ret > 0)
        {
            // wifi设备读响应
            if (FD_ISSET(fd_wifi, &fdset))
            {
                // 获取esp8266数据
                // 串口通讯存在分包问题，故进行协议提取与组包操作，测试：10000个包错误200。
                ret = read(fd_wifi, RX_buf, RX_SIZE);
                for (i = 0; i < ret; i++)
                {
                    switch (wifi_rdsta)
                    {
                    case FIND_START:
                        if (RX_buf[i] == PACKAGE_START) // 判断起始位，发现起始位 切换模式
                        {
                            wifi_rdsta = FIND_DATA;
                            wifi_buf[wifi_rd_pos] = PACKAGE_START;
                            wifi_rd_pos++;
                        }
                        break;
                    case FIND_DATA:
                        if (RX_buf[i] == PACKAGE_END_0) // 发现第一个停止位，切换模式
                        {
                            wifi_rdsta = FIND_END;
                        }
                        wifi_buf[wifi_rd_pos] = RX_buf[i];
                        wifi_rd_pos++;
                        break;
                    case FIND_END:
                        if (RX_buf[i] == PACKAGE_END_1) // 判断第二个停止位，若第二停止位判断成功，则协议结束。重置接收数据缓存区游标
                        {
                            wifi_rdsta = FIND_START;
                            wifi_buf[wifi_rd_pos] = RX_buf[i];

                            // DEBUG:输出获取的数据包
                            int j;
                            printf("LDEV_WIFI: ");
                            for (j = 0; j <= wifi_rd_pos; j++)
                            {
                                printf("\033[33m%02X \033[0m", wifi_buf[j]);
                            }
                            printf("number: %d\n", wifi_cal[0]++);

                            // TODO: 数据协议包的传输与解析
                            write(pfd_out, wifi_buf, wifi_rd_pos + 1);
                            wifi_rd_pos = 0;
                        }
                        else // 若不为结束位 则返回数据接收模式
                        {
                            wifi_rdsta = FIND_DATA;
                            wifi_buf[wifi_rd_pos] = RX_buf[i];
                            wifi_rd_pos++;
                        }
                        break;
                    default:
                        wifi_rdsta = FIND_START;
                    }
                }
            }
            // BT设备读响应
            if (FD_ISSET(fd_bt, &fdset))
            {
                // 获取BT数据
                // 串口通讯存在分包问题，故进行协议提取与组包操作，测试：TODO:。
                ret = read(fd_bt, RX_buf, RX_SIZE);
                for (i = 0; i < ret; i++)
                {
                    switch (bt_rdsta)
                    {
                    case FIND_START:
                        if (RX_buf[i] == PACKAGE_START) // 判断起始位，发现起始为切换模式
                        {
                            bt_rdsta = FIND_DATA;
                            bt_buf[bt_rd_pos] = PACKAGE_START;
                            bt_rd_pos++;
                        }
                        break;
                    case FIND_DATA:
                        if (RX_buf[i] == PACKAGE_END_0) // 发现第一个停止位，切换模式
                        {
                            bt_rdsta = FIND_END;
                        }
                        bt_buf[bt_rd_pos] = RX_buf[i];
                        bt_rd_pos++;
                        break;
                    case FIND_END:
                        if (RX_buf[i] == PACKAGE_END_1) // 判断第二个停止位，若第二停止位判断成功，则协议结束。重置接收数据缓存区游标
                        {
                            bt_rdsta = FIND_START;
                            bt_buf[bt_rd_pos] = RX_buf[i];

                            // DEBUG:输出获取的数据包
                            int j;
                            printf("LDEV_BLUETOOTH: ");
                            for (j = 0; j <= bt_rd_pos; j++)
                            {
                                printf("\033[33m%02X \033[0m", bt_buf[j]);
                            }
                            printf("number: %d\n", bt_cal[0]++);

                            // TODO: 数据协议包的传输与解析
                            write(pfd_out, bt_buf, bt_rd_pos + 1);
                            bt_rd_pos = 0;
                        }
                        else // 若不为结束位 则返回数据接收模式
                        {
                            bt_rdsta = FIND_DATA;
                            bt_buf[bt_rd_pos] = RX_buf[i];
                            bt_rd_pos++;
                        }
                        break;
                    default:
                        bt_rdsta = FIND_START;
                    }
                }
            }
            // 管理程序响应
            if (FD_ISSET(pfd_in, &fdset))
            {
                // TODO:
                ret = read(pfd_in, RX_buf, RX_SIZE);

                // DEBUG:输出获取的数据包
                int j;
                printf("\033[32mGet_DATA_FROM_MANAGER\033[0m : ");
                for (j = 0; j <= ret; j++)
                {
                    printf("\033[33m%02X \033[0m", bt_buf[j]);
                }
                printf("\n");
            }
#ifdef DEBUG
            if (FD_ISSET(0, &fdset))
            {
                // 检测标准输入 选择发送的 模式
                char buf[128];
                fgets(buf, sizeof(buf), stdin);
                int len = strlen(buf);
                buf[len - 1] = '\0';
                printf("\033[32m%s\033[0m is pressed !\n", buf);
                fflush(stdin); //清空输入缓存区
            }
#endif
        }
    }
    close(fd_wifi);
    close(fd_bt);
    return 0;
}
