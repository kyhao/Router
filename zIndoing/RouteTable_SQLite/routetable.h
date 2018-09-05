// ROUTETABLE_H
// 路由表和ID池管理模块
// 采用POSIX标准信号量 -lpthread
// 采用SQLite 需要有SQLite支持 -lsqlite3
// 下面为安装方法
// $tar xvfz sqlite-autoconf-***.tar.gz
// $cd sqlite-autoconf-***
// $./configure --prefix=/usr/local
// $make
// $make install

#include <time.h>

#ifndef _ROUTETABLE_H_
#define _ROUTETABLE_H_

// 路由表中设备类型
#define DEV_TYPE_NONE 0x00
#define DEV_TYPE_WIFI 0x01
#define DEV_TYPE_BLUETOOTH 0x02
#define DEV_TYPE_LORA 0x03
#define DEV_TYPE_NB 0x04

// 路由表中状态字段
#define DEV_STA_NONE 0x00
#define DEV_STA_ACTIVE 0x01
#define DEV_STA_OUTTIME 0x02
#define DEV_STA_MAYBEDEAD 0x03

#define get_time() time(NULL) // 获取当前时间距离1970-01-01的秒数

// 路由表数据结构
typedef struct _Routetable
{
    int id;
    int dtype;
    int stat;
    long reg_time;
    long last_time;
} Routetable;

// 模块初始化函数
void route_id_init(void);

// ID管理函数组
void idpool_init(void);
void idpool_rest(void);
int id_Alloca(void);
void id_Release(int id);
void idpool_Save(void);

// 路由表管理函数组
int route_init(void);
int route_join(int id, int type);
int route_march(int id, Routetable *type);
void route_release(int id);
int route_update(int id, int sta);
int route_maintain(void);

void route_recover(void);

#endif