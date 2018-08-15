// 创建于2018/8/7
// 本地协议的封装与解析方法
// 包含4个函数 两个主要函数用于 协议的分配与协议处理 使用时--》只需调用两个主函数即可《--
// 其余为协议版本处理函数 为单独版本函数
// Packet结构体保存 数据协议内容
// 协议字符串 与 可处理的结构体 相互转换
// 数据部分长度可调 更改头文件中 MAX_DATALEN
// TODO: 加密数据 
#include <stdio.h>
#include "localProtocol.h"

#define ARRAY_INT(buf, pos) (int)(*(buf + pos)) // 根据数组指针获取并转为int
#define ARRAY_CHA(buf, pos) (*(buf + pos))      // 根据数组指针获取

// 本地协议起始标识位
#define LPROTOCOL_START 0xFF
#define LPROTOCOL_END_0 0x0D
#define LPROTOCOL_END_1 0x0A

// 本地协议版本/起始标识位 字段 位置
#define LPROTOCOL_START_POS 0
#define LPROTOCOL_VER_POS 1

// 本地协议版本
#define LPROTOCOL_VER1 1 // 协议1字符串格式  |0xFF|<ver>|<id>|<type>|<datalen>|<data>...|
#define LPROTOCOL_VER2 2
#define LPROTOCOL_VER3 3

// 本地协议头部大小(字节)
#define LPROTOCOL_VER1_HEADER_SIZE 9

// 本地协议版本1 字段位置
#define LPROTOCOL_VER1_HEADER_START LPROTOCOL_START_POS
#define LPROTOCOL_VER1_HEADER_VER LPROTOCOL_VER_POS
#define LPROTOCOL_VER1_HEADER_SEQ 2     // 协议序号所在位置
#define LPROTOCOL_VER1_HEADER_SID 3     // 源地址首位置 LPROTOCOL_VER1_HEADER_VER+1
#define LPROTOCOL_VER1_HEADER_DID 5     // 目的地址首位置 LPROTOCOL_VER1_HEADER_VER+12
#define LPROTOCOL_VER1_HEADER_TYPE 7    // 协议类型所在位置
#define LPROTOCOL_VER1_HEADER_DATALEN 8 // 数据段长度所在位置

// 本地协议数据解析(decode)
// 将本地数据协议 提取 存入结构体
// @param
// @inbuf 输入本地协议字符串
// @outbuf 输出协议结构包
// @return 返回值 错误代码 0为正常
int lprotocol_decode(uint8_t *inbuf, Packet *outbuf)
{
    if (inbuf == NULL || outbuf == NULL)
    {
        return 1;
    }
    // 判断起始标识位
    if (inbuf[LPROTOCOL_START_POS] != LPROTOCOL_START)
    {
        return 2;
    }

    int ver, err;
    ver = ARRAY_INT(inbuf, LPROTOCOL_VER1_HEADER_VER); // 获取协议版本号
    switch (ver)
    {
    case LPROTOCOL_VER1:
        return lprotocol_decode_v1(inbuf, outbuf);
        break;
    default:
        return 3;
    }
    return 0;
}

// 协议数据封装(package)
// 将数据结构体内容 封装成 本地协议字符串
// @param
// @inbuf 输入 协议结构体
// @outbuf 输出 本地协议字符串
// @len 输出 协议字符串长度
// @ver 封装使用的协议版本
// @return 返回值 错误代码 0为正常
int lprotocol_package(Packet *inbuf, uint8_t *outbuf, int *len, int ver)
{
    if (inbuf == NULL || outbuf == NULL || len == NULL)
    {
        return 1;
    }
    switch (ver)
    {
    case LPROTOCOL_VER1:
        lprotocol_package_v1(inbuf, outbuf);
        *len = inbuf->header.datalen + LPROTOCOL_VER1_HEADER_SIZE + 2;
        break;
    default:
        return 3;
    }
    return 0;
}

// 解析本地协议 (版本1)
// @param
// @inbuf 输入 本地协议字符串
// @outbuf 输出 协议结构包
// @return 返回值 错误代码 0为正常
int lprotocol_decode_v1(uint8_t *inbuf, Packet *outbuf)
{
    int cur, end, i;
    outbuf->header.ver = ARRAY_INT(inbuf, LPROTOCOL_VER1_HEADER_VER);         // 从字符串取出版本
    outbuf->header.seq = ARRAY_INT(inbuf, LPROTOCOL_VER1_HEADER_SEQ);         // 从字符串取出序号
    outbuf->header.type = ARRAY_INT(inbuf, LPROTOCOL_VER1_HEADER_TYPE);       // 从字符串取出类型
    outbuf->header.datalen = ARRAY_INT(inbuf, LPROTOCOL_VER1_HEADER_DATALEN); // 从字符串取出数据长度
    // 循环取出源地址
    cur = LPROTOCOL_VER1_HEADER_SID; // 设置游标起始位置
    for (i = 0; i < ID_LENGTH; cur++, i++)
    {
        ARRAY_CHA(outbuf->header.sid, i) = ARRAY_CHA(inbuf, cur);
    }
    // 循环取出目的地址
    cur = LPROTOCOL_VER1_HEADER_DID; // 设置游标起始位置
    for (i = 0; i < ID_LENGTH; cur++, i++)
    {
        ARRAY_CHA(outbuf->header.did, i) = ARRAY_CHA(inbuf, cur);
    }

    // 循环将数据取出并导入结构体data
    cur = LPROTOCOL_VER1_HEADER_SIZE;   // 设置游标起始位置
    end = cur + outbuf->header.datalen; // 结束位置
    for (i = 0; cur < end; cur++, i++)
    {
        ARRAY_CHA(outbuf->data, i) = ARRAY_CHA(inbuf, cur);
    }
    // 检查结尾标识字节
    if (ARRAY_CHA(inbuf, cur) != LPROTOCOL_END_0 || ARRAY_CHA(inbuf, cur + 1) != LPROTOCOL_END_1)
    {
        return -1;
    }
    return 0;
}

// 封装本地协议 (版本1)
// @param
// @inbuf 输入 协议结构包
// @outbuf 输出 本地协议字符串
// @return 返回值 错误代码 0为正常
int lprotocol_package_v1(Packet *inbuf, uint8_t *outbuf)
{
    int cur, end, i;
    // 头部数据封装
    ARRAY_CHA(outbuf, LPROTOCOL_VER1_HEADER_START) = LPROTOCOL_START;                  // 封装数据起始标志位
    ARRAY_CHA(outbuf, LPROTOCOL_VER1_HEADER_VER) = (uint8_t)LPROTOCOL_VER1;            // 封装协议版本
    ARRAY_CHA(outbuf, LPROTOCOL_VER1_HEADER_SEQ) = (uint8_t)inbuf->header.seq;         // 封装协议序号
    ARRAY_CHA(outbuf, LPROTOCOL_VER1_HEADER_TYPE) = (uint8_t)inbuf->header.type;       // 封装协议类型
    ARRAY_CHA(outbuf, LPROTOCOL_VER1_HEADER_DATALEN) = (uint8_t)inbuf->header.datalen; // 封装数据长度
    // 循环封装源地址
    cur = LPROTOCOL_VER1_HEADER_SID; // 设置游标起始位置
    for (i = 0; i < ID_LENGTH; cur++, i++)
    {
        ARRAY_CHA(outbuf, cur) = ARRAY_CHA(inbuf->header.sid, i);
    }
    // 循环封装目的地址
    cur = LPROTOCOL_VER1_HEADER_DID; // 设置游标起始位置
    for (i = 0; i < ID_LENGTH; cur++, i++)
    {
        ARRAY_CHA(outbuf, cur) = ARRAY_CHA(inbuf->header.did, i);
    }

    // 数据部分封装
    cur = LPROTOCOL_VER1_HEADER_SIZE;  //设置游标起始位置
    end = cur + inbuf->header.datalen; // 结束位置
    for (i = 0; cur < end; cur++, i++)
    {
        ARRAY_CHA(outbuf, cur) = ARRAY_CHA(inbuf->data, i);
    }
    // 添加结尾标识字节
    ARRAY_CHA(outbuf, cur++) = LPROTOCOL_END_0;
    ARRAY_CHA(outbuf, cur) = LPROTOCOL_END_1;

    return 0;
}