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
    int reg_time;
    int last_time;
} Routetable;

int route_id_init(void);

// ID管理
int idpool_init(void);
int id_Alloca(void);
int id_Release(int id);

// 路由表管理
int route_init(void);
int route_join(int id, int type);
int route_march(int id, int type);
int route_release(int id);
int route_modify(int id, int sta);
void route_maintain(void);

#endif