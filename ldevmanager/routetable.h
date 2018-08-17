#ifndef _ROUTETABLE_H_
#define _ROUTETABLE_H_

// 路由表中设备类型
#define DEV_TYPE_WIFI 0x01
#define DEV_TYPE_BLUETOOTH 0x02
#define DEV_TYPE_LORA 0x03
#define DEV_TYPE_NB 0x04

// 路由表中状态字段
#define DEV_STA_ACTIVE 0x01
#define DEV_STA_OUTTIME 0x02
#define DEV_STA_MAYBEDEAD 0x03

// 路由表数据结构
typedef struct _Routetable
{
    int id;
    int type;
    int stat;
    int last_time;
} Routetable;

int routetable_init(void);

// ID管理
int getdevid(void);

// 路由表管理
int join_route(int id, int com);
int match_route(int id, int *com);
int delete_route(int id);
int modify_route(int id, int sta);
void check_route(void);

#endif