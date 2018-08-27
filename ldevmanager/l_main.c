// Local Net Device Manager
// Made 2018/7/21
// Modify 2018/8/21 取消管道方式
// Modify 2018/8/24 优化监听代码\提升所有设备兼容性
// 未说明的长度默认为字节
// TODO: 设备的插入与中途退出识别检测（即插即用）
// TODO: LoRa设备的接入策略采用通用串口总线
// TODO: 监测写数据
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>

// 设备驱动调用
#include "driver/usbctl.h"
#include "driver/esp8266.h"
#include "driver/bluetooth.h"
#include "driver/lora.h"

#include "lpctl.h"

#define DEBUG // 是否开启Debug模式 定义Debug则开启 调试模式

#define MAXDEVFD_NUM 8 // 最大可能存在的设备数
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

// 测试函数，显示
static int cal_num = 0;
void packet_show(int pos, char *buf, char *title)
{
    // DEBUG: 输出获取的数据包
    int j;
    printf("%s: ", title);
    for (j = 0; j <= pos; j++)
    {
        printf("\033[33m%02X \033[0m", buf[j]);
    }
    printf(" num:%d\n", cal_num);
    cal_num++;
}

// 调用设备驱动，初始化设备
// @param
// @fd_array 设备文件描述符数组
// @num 输出变量文件描述符数量
int dev_init(int *fd_array, int *num)
{
    int fd_wifi, fd_bt, fd_lora, fd_nb; // wifi,蓝牙,lora,nb,设备描述符
    int num_dev = 0;                    // 配置成功的设备数
    int pos = 0;
    // esp8266 WIFI设备选择
    fd_wifi = esp8266_open();
    if (-1 == fd_wifi)
    {
        printf("->> No esp8266 devcie\n");
    }
    else
    {
        esp8266_config(fd_wifi); // 成功打开esp8266后配置
        fd_array[pos] = fd_wifi;
        pos++;
        num_dev++;
    }
    // bluetooth设备选择
    fd_bt = bluetooth_open();
    if (-1 == fd_bt)
    {
        printf("->> No bluetooth devcie\n");
        // return -1;
    }
    else
    {
        fd_array[pos] = fd_bt;
        pos++;
        num_dev++;
    }
    // lora设备选择
    fd_lora = lora_open();
    if (-1 == fd_lora)
    {
        printf("->> No Lora devcie\n");
        // return -1;
    }
    else
    {
        fd_array[pos] = fd_lora;
        pos++;
        num_dev++;
    }
    // 若设备无法检测与选择则退出
    if (-1 == fd_wifi && -1 == fd_bt && -1 == fd_lora)
        return -1;
    *num = num_dev;
}

// 本地网设备管理 主函数
// @param
// @argc 传入函数的参数个数
// @argv 传入函数参数变量数组 无用
int main(int argc, char **argv)
{
    // struct timeval timeout;                             // 超时配置
    fd_set fdset;                                         // select 数组
    int ret, i, j;                                        // 错误参数保存/临时变量 使用前注意初始化。减少内存申请释放次数
    char RX_buf[RX_SIZE];                                 // 公用单次接收缓冲
    int dfd_array[MAXDEVFD_NUM];                          // devfd 设备文件描述符数组
    char dev_buf[MAXDEVFD_NUM][PACKET_LEN];               // 存放完整协议包
    int dev_rdsta[MAXDEVFD_NUM], dev_rdpos[MAXDEVFD_NUM]; // 设备读取数据状态 数据协议接收游标 需初始化
    int fd_max, fd_num;                                   // 最大文件描述符/文件描述符数

    ret = dev_init(dfd_array, &fd_num); // 设备初始化
    lpctl_init();                       // 协议控制初始化
    if (ret == -1)
        return -1;

    // 初始化设备读写状态变量
    for (i = 0; i < fd_num; i++)
    {
        dev_rdsta[i] = FIND_START;
        dev_rdpos[i] = 0;
    }
    fd_max = 0;
    // 设备正常 监听开始
    printf("InitFinished:Start->>>>>\ndev_num: %d\n", fd_num);
    while (1)
    {
        FD_ZERO(&fdset);
        FD_SET(0, &fdset); // 标准输入监听
        for (i = 0; i < fd_num; i++)
        {
            FD_SET(dfd_array[i], &fdset);
            fd_max = MAX_FD(dfd_array[i], fd_max);
        }
        ret = select(fd_max + 1, &fdset, NULL, NULL, NULL);
        if (ret > 0)
        {
            // 循环检查是否有设备触发事件
            for (i = 0; i < fd_num; i++)
            {
                if (FD_ISSET(dfd_array[i], &fdset))
                {
                    // 获取数据
                    // 串口通讯存在分包问题，故进行协议提取与组包操作，测试：10000个包错误200。
                    ret = read(dfd_array[i], RX_buf, RX_SIZE);
                    for (j = 0; j < ret; j++)
                    {
                        switch (dev_rdsta[i])
                        {
                        case FIND_START:
                            if (RX_buf[j] == PACKAGE_START) // 判断起始位，发现起始位 切换模式
                            {
                                dev_rdsta[i] = FIND_DATA;
                                dev_buf[i][dev_rdpos[i]] = PACKAGE_START;
                                dev_rdpos[i]++;
                            }
                            break;
                        case FIND_DATA:
                            if (RX_buf[j] == PACKAGE_END_0) // 发现第一个停止位，切换模式
                            {
                                dev_rdsta[i] = FIND_END;
                            }
                            dev_buf[i][dev_rdpos[i]] = RX_buf[j];
                            dev_rdpos[i]++;
                            break;
                        case FIND_END:
                            if (RX_buf[j] == PACKAGE_END_1) // 判断第二个停止位，若第二停止位判断成功，则协议结束。重置接收数据缓存区游标
                            {
                                dev_rdsta[i] = FIND_START;
                                dev_buf[i][dev_rdpos[i]] = RX_buf[j];
#ifdef DEBUG
                                packet_show(dev_rdpos[i], dev_buf[i], "_GET_: ");
#endif
                                // TODO: 数据协议包的传输与解析
                                // write(dfd_array[i], dev_buf[i], dev_rdpos[i] + 1);
                                dev_rdpos[i] = 0;
                                lpctl(dev_buf[i], dfd_array[i]);
                            }
                            else // 若不为结束位 则返回数据接收模式
                            {
                                dev_rdsta[i] = FIND_DATA;
                                dev_buf[i][dev_rdpos[i]] = RX_buf[j];
                                dev_rdpos[i]++;
                            }
                            break;
                        default:
                            dev_rdsta[i] = FIND_START;
                        }
                    }
                }
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

    // 关闭文件描述符 但未关闭通信设备
    for (i = 0; i < fd_num; i++)
    {
        close(dfd_array[i]);
    }
    return 0;
}
