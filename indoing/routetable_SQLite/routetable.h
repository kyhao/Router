// Last Modify 2018/8/30
// 路由表与ID管理程序
// 使用SQLite数据库 需要有SQLite支持,下面为安装方法
// $tar xvfz sqlite-autoconf-***.tar.gz
// $cd sqlite-autoconf-***
// $./configure --prefix=/usr/local
// $make
// $make install
// 
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

// 路由表数据结构
typedef struct _Routetable
{
    int id;
    int dtype;
    int stat;
    long reg_time;
    long last_time;
} Routetable;

void route_id_init(void);

// ID管理
int idpool_init(void);
int id_Alloca(void);
int id_Release(int id);
int id_Update(void);

// 路由表管理
int route_init(void);
int route_join(int id, int type);
int route_march(int id, int *type);
int route_release(int id);
int route_modify(int id, int sta, int dtype);
int route_maintain(void);
int route_update(void);

#endif