#ifndef _LOCAL_PROTOCOL_H_
#define _LOCAL_PROTOCOL_H_

#include <stdint.h>

#ifndef MAX_DATALEN
#define MAX_DATALEN 128 // 协议携带的最大数据量（字节）
#endif

#ifndef ID_LENGTH
#define ID_LENGTH 12 // ID字段长度（字节）
#endif
// 协议头部数据结构
typedef struct _Header
{
    uint8_t ver;            // 协议版本
    uint8_t sid[ID_LENGTH]; // 源地址
    uint8_t did[ID_LENGTH]; // 目的地址
    uint8_t type;           // 类型
    uint8_t datalen;        // 数据段长度
} Header;

// 协议包数据结构
typedef struct _Packet
{
    Header header;          // 头部
    char data[MAX_DATALEN]; // 数据部分
} Packet;

int lprotocol_decode(uint8_t *inbuf, Packet *outbuf);
int lprotocol_package(Packet *inbuf, uint8_t *outbuf, int *len, int ver);
int lprotocol_decode_v1(uint8_t *inbuf, Packet *outbuf);
int lprotocol_package_v1(Packet *inbuf, uint8_t *outbuf);

#endif