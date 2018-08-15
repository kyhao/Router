// 2018/8/15
// 本地消息接收控制
#include "linfoctl.h"
#include "localProtocol.h"

// 测试阶段使用宏定义 注意后期修改为 由配置文件读取
#define ROUTER_ID 0x0101

// 协议类型定义
#define TYPE_REGIST 0x01
#define TYPE_REGIST_ACK 0x02
#define TYPE_TRANSEFER 0x03
#define TYPE_TRANSEFER_ACK 0x04
#define TYPE_CONTROL 0x05
#define TYPE_CONTROL_ACK 0x06

static int router_id;

// 本地消息控制函数
int local_infoctl(char *lprotocol)
{
    int ret;
    Packet packet_in, packet_out;
    ret = lprotocol_decode(lprotocol, &packet); // 数据包解析

}