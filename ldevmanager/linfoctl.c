// 2018/8/15
// 本地消息接收控制
// UPDATE: 本不应该使用write操作 为了时间效率暂时不分出模块
#include <stdio.h>
#include <unistd.h> //
#include <fcntl.h>  //
#include "linfoctl.h"
#include "routetable.h"
#include "../tools/localProtocol.h"

// 测试阶段使用宏定义 注意后期修改为 由配置文件读取
#define ROUTER_ID 0x0101

// 协议类型定义
#define TYPE_REGIST 0x01
#define TYPE_REGIST_ACK 0x02
#define TYPE_TRANSEFER 0x03
#define TYPE_TRANSEFER_ACK 0x04
#define TYPE_CONTROL 0x05
#define TYPE_CONTROL_ACK 0x06

// 协议版本
#define LPROROCOL_VER1 0x01

#define TX_MAX_LEN 128

static int router_id;
static int fd_out;
static char tx_buf[TX_MAX_LEN];
static Packet packet_ret;

// 初始化变量
int local_infoctl_init()
{
    router_id = ROUTER_ID;
    routetable_init(); // 路由表初始化
    return 0;
}

static int type_transefer(Packet *packet, int fd, int ndev)
{
    // 数据传输 若产生错误 忽略消息
    // 1.检测目的id与本机id
    // TODO: 2.匹配路由表与源id
    // TODO: 3.数据提取与发送
    // 4.成功 返回ACK
    int len;
    if (packet->header.did == router_id)
    {
        if (match_route(packet->header.sid, NULL) == 0)
        {
            packet_ret.header.seq = packet->header.seq;
            packet_ret.header.ver = packet->header.ver;
            packet_ret.header.sid = router_id;
            packet_ret.header.did = packet->header.sid;
            packet_ret.header.type = TYPE_TRANSEFER_ACK;
            packet_ret.header.datalen = 0x00;
            lprotocol_package(&packet_ret, tx_buf, &len, LPROROCOL_VER1);
            write(ndev, tx_buf, len);
            return 0;
        }
    }
    else
    {
    }
    return 0x0103;
}

static int type_control_ack(Packet *packet, int ndev)
{
    return 0;
}

// 注册类数据处理
// @param
// @packet 已处理好的数据包
// @ndev 设备
// @return 返状态值
static int type_regist(Packet *packet, int ndev)
{
    int len;
    // 检查id位
    if (packet->header.sid == 0x0000 && packet->header.did == 0x0000)
    {
        // 初次注册
        // 1.分配设备id
        // 2.id与ndev路由表匹配
        int id = getdevid();
        if (id = -1)
            return 0x0103;
        join_route(id, ndev); //
        // UPDATE: 当前直接采取将协议丢出
        packet_ret.header.seq = packet->header.seq;
        packet_ret.header.ver = packet->header.ver;
        packet_ret.header.sid = router_id;
        packet_ret.header.did = id;
        packet_ret.header.type = TYPE_REGIST_ACK;
        packet_ret.header.datalen = 0x00;
        lprotocol_package(&packet_ret, tx_buf, &len, LPROROCOL_VER1);
        write(ndev, tx_buf, len);
    }
    else
    {
        // 非初次注册
        // 1.识别目的地址是否为本机 不是本机 忽略消息
        // 2.若目的地址为本机，查询源地址是否包含在路由表内
        // 3.若路由表包含 修改其状态
        // UPDATE: 当前直接采取将协议丢出
        if (packet->header.did == router_id)
        {
            if (match_route(packet->header.sid, NULL) == 0)
            {
                packet_ret.header.seq = packet->header.seq;
                packet_ret.header.ver = packet->header.ver;
                packet_ret.header.sid = router_id;
                packet_ret.header.did = packet->header.sid;
                packet_ret.header.type = TYPE_REGIST_ACK;
                packet_ret.header.datalen = 0x00;
                lprotocol_package(&packet_ret, tx_buf, &len, LPROROCOL_VER1);
                write(ndev, tx_buf, len);
            }
        }
        else
        {
            return 0x0102;
        }
    }
}

// 本地消息控制函数(接收)
// @param
// @lprotocol 输入 协议消息字符串
// @fd 通信用文件描述符 出口
// @ndev 设备端口号 设备文件描述符 进口
// @return 错误值 0无错误
int local_infoctl(char *lprotocol, int fd, int ndev)
{
    int ret, sid, did;
    Packet packet;
    fd_out = fd;
    ret = lprotocol_decode(lprotocol, &packet); // 数据包解析

    if (ret == 0)
    {
        // DEBUG: printf
        printf("header_seq: %d\n", packet.header.seq);
        printf("sid: %d did : %d\n", packet.header.sid, packet.header.did);
        printf("header_type: %d\n", packet.header.type);
        printf("router_id: %d\n", router_id);

        switch (packet.header.type)
        {
        case TYPE_REGIST:
            type_regist(&packet, ndev);
            break;
        case TYPE_TRANSEFER:
            type_transefer(&packet, fd, ndev);
            break;
        case TYPE_CONTROL_ACK:
            type_control_ack(&packet, ndev);
            break;
        default:
            return 0x1010;
        }
    }
    else
    {
        //DEBUG: printf
        printf("decode_error\n");
    }
    return 0;
}
