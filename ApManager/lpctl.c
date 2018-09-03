// 2018/8/15
// 本地消息接收处理器
// UPDATE: 本不应该使用write操作 为了时间效率暂时不分出模块
// 当前模块不具备完全的多线程能力
// 当前模块对协议的序号处理欠缺
#include <stdio.h>
#include <unistd.h> //
#include <time.h>
#include <fcntl.h> //
#include "lpctl.h"
#include "modules/localProtocol.h"
#include "modules/routetable.h"

// 测试阶段使用宏定义 注意后期修改为 由配置文件读取
#define ROUTER_ID 0xCCCC

// 协议类型定义
#define TYPE_REGIST 0x01
#define TYPE_REGIST_ACK 0x02
#define TYPE_TRANSEFER 0x03
#define TYPE_TRANSEFER_ACK 0x04
#define TYPE_CONTROL 0x05
#define TYPE_CONTROL_ACK 0x06
#define TYPE_SYNCTIME 0x07
#define TYPE_SYNCTIME_ACK 0x08

// 协议版本
#define LPROROCOL_VER1 0x01

#define TX_MAX_LEN 128

static int router_id;
static int fd_out;

void packet1_show(int pos, char *buf, char *title)
{
    // DEBUG: 输出获取的数据包
    int j;
    printf("%s: ", title);
    for (j = 0; j <= pos; j++)
    {
        printf("\033[33m%02X \033[0m", buf[j]);
    }
}

// 初始化
int lpctl_init()
{
    router_id = ROUTER_ID;
    route_id_init();
    return 0;
}

// 传输数据类型函数
// @param
// @packet 输入结构体
// @fd 通信用fd
// @ndev 设备编号
int type_transefer(Packet *packet, int dfd)
{
    // 数据传输 若产生错误 忽略消息
    // 1.检测目的id与本机id
    // 2.匹配路由表与源id
    // TODO: 3.数据提取与发送
    // 4.成功 返回ACK
    int len;
    char tx_buf[TX_MAX_LEN];
    Packet packet_ret;
    if (packet->header.did == router_id && route_march(packet->header.sid, NULL) == 0)
    {
        packet_ret.header.seq = packet->header.seq;
        packet_ret.header.ver = packet->header.ver;
        packet_ret.header.sid = router_id;
        packet_ret.header.did = packet->header.sid;
        packet_ret.header.type = TYPE_TRANSEFER_ACK;
        packet_ret.header.datalen = 0x00;
        lprotocol_package(&packet_ret, tx_buf, &len, LPROROCOL_VER1);
        write(packet->dev, tx_buf, len);
        return 0;
    }

    return 0x0203;
}

int type_control_ack(Packet *packet, int dfd)
{

    return 0;
}

// 注册类数据处理
// @param
// @packet 已处理好的数据包
// @ndev 设备
// @return 返状态值
int type_regist(Packet *packet, int dfd)
{
    int len;
    char tx_buf[TX_MAX_LEN];
    Packet packet_ret;
    // 检查id位
    if (packet->header.sid == 0x0000 && packet->header.did == 0x0000)
    {
        // 初次注册
        // 1.分配设备id
        // 2.id与ndev路由表匹配
        // UPDATE: 当前直接采取将协议丢出
        int id = id_Alloca();
        route_join(id, packet->header.type);
        packet_ret.header.seq = packet->header.seq;
        packet_ret.header.ver = packet->header.ver;
        packet_ret.header.sid = router_id;
        packet_ret.header.did = id;
        packet_ret.header.type = TYPE_REGIST_ACK;
        packet_ret.header.datalen = 0x00;
        lprotocol_package(&packet_ret, tx_buf, &len, LPROROCOL_VER1);
        printf("len_reg: %d\n", len);
        packet1_show(len, tx_buf, "_output_ : ");
        write(packet->dev, tx_buf, len);
    }
    else
    {
        // 非初次注册
        // 1.识别目的地址是否为本机 不是本机 忽略消息
        // 2.若目的地址为本机，查询源地址是否包含在路由表内
        // 3.若路由表包含 修改其状态
        // UPDATE: 当前直接采取将协议丢出
        Packet packet_ret;
        if (packet->header.did == router_id)
        {
            if (route_march(packet->header.sid, NULL) == 0)
            {
                packet_ret.header.seq = packet->header.seq;
                packet_ret.header.ver = packet->header.ver;
                packet_ret.header.sid = router_id;
                packet_ret.header.did = packet->header.sid;
                packet_ret.header.type = TYPE_REGIST_ACK;
                packet_ret.header.datalen = 0x00;
                lprotocol_package(&packet_ret, tx_buf, &len, LPROROCOL_VER1);
                write(packet->dev, tx_buf, len);
            }
        }
        else
        {
            return 0x0202;
        }
    }
    return 0;
}

int type_synctime(Packet *packet, int dfd)
{
    int len;
    char tx_buf[TX_MAX_LEN];
    time_t cur_time = time(NULL);
    struct tm *t;
    t = localtime(&cur_time);
    Packet packet_ret;
    if (packet->header.did == router_id && route_march(packet->header.sid, NULL) == 0)
    {
        packet_ret.header.seq = packet->header.seq;
        packet_ret.header.ver = packet->header.ver;
        packet_ret.header.sid = router_id;
        packet_ret.header.did = packet->header.sid;
        packet_ret.header.type = TYPE_SYNCTIME_ACK;
        packet_ret.header.datalen = 0x13;
        strftime(packet_ret.data, 13, "%y%m%d%H%M%S%w", t);
        packet_ret.data[10] = '1';
        packet_ret.data[11] = '0';
        packet_ret.data[12] = '5';
        packet_ret.data[13] = '0'; // 周期时间需太高‘0’
        packet_ret.data[14] = '0';
        packet_ret.data[15] = '0';
        packet_ret.data[16] = 0x0E + '0';
        packet_ret.data[17] = 0x10 + '0';
        packet_ret.data[18] = 0x3C;
        printf("data: %s\n", packet_ret.data);
        lprotocol_package(&packet_ret, tx_buf, &len, LPROROCOL_VER1);
        packet1_show(len, tx_buf, "_output_package : ");
        write(packet->dev, tx_buf, len);
        return 0;
    }

    return 0x0203;
}

// 数据包解析控制控制函数(接收)
// @param
// @packet
int lpctl(char *in_buf, int dfd)
{
    Packet packet;
    int ret;
    ret = lprotocol_decode(in_buf, &packet);
    if (ret != 0)
    {
        write(dfd, "AT+RST\r\n", 8); // 测试用 注意删除
        return ret;
    }
    packet.dev = dfd;
    switch (packet.header.type)
    {
    case TYPE_REGIST:
        ret = type_regist(&packet, dfd);
        break;
    case TYPE_TRANSEFER:
        ret = type_transefer(&packet, dfd);
        break;
    case TYPE_CONTROL_ACK:
        ret = type_control_ack(&packet, dfd);
        break;
    case TYPE_SYNCTIME:
        ret = type_synctime(&packet, dfd);
        break;
    default:
        return 0x0204;
        break;
    }
    return ret;
}
