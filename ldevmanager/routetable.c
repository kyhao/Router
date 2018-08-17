// 2018/8/17
// 包含ID管理函数和路由表管理函数
#include <stdio.h>
#include <time.h>
#include "routetable.h"

#define ID_BYTE_BEGING 0x01 // ID池起始位
#define ID_BYTE_END 0xFF    // ID池结束

#define ID_NUM_MAX 2048 // 最大id分配数 UPDATE: 可通过配置文件修改
#define ID_FREE '0'     // 当前ID未使用
#define ID_USED '1'     // 当前ID已使用
#define ID_OUTTIME '2'  // 当前ID超时状态未知

// 路由表和ID池本地库路径
#define ID_POOLFILE_PATH "./idpool.db"      // ID池管理文件
#define ID_ROUTEFILE_PATH "./routetable.db" // 路由表路径

static char ID_POOL[ID_NUM_MAX + 1];       // ID池
static int id_cur;                         // ID池游标
static Routetable Route_table[ID_NUM_MAX]; // 路由表数据结构

time_t get_time()
{
    time_t t;
    time(&t);
    return t;
}

// 路由表/id初始化函数
// 从数据库配置文件
// @param
// @return 错误返回 0正常
int routetable_init()
{
    // 1.id管理初始化
    // 2.routetable初始化
    int i, ret;
    FILE *ifp = NULL; // id文件指针
    FILE *rfp = NULL; // 路由表文件指针
    // ID文件打开读取
    ifp = fopen(ID_POOLFILE_PATH, "r"); // 打开失败 创建文件
    if (ifp == NULL)
    {
        // LOG: 记录日志
        ifp = fopen(ID_POOLFILE_PATH, "w+");
        for (i = 0; i < ID_NUM_MAX; i++)
        {
            if ((i & 0x00FF) == 0x000D) // 去除0x0D的影响
            {
                ID_POOL[i] = ID_USED;
            }
            else
            {
                ID_POOL[i] = ID_FREE;
            }
        }
        ID_POOL[ID_NUM_MAX] = '\0';
        fprintf(ifp, "%d:%s", ID_BYTE_BEGING, ID_POOL);
        id_cur = ID_BYTE_BEGING;
    }
    else
    {
        fscanf(ifp, "%d:%s", &id_cur, ID_POOL);
    }
    fclose(ifp);

    // ROUTETABLE文件打开读取
    rfp = fopen(ID_ROUTEFILE_PATH, "r");
    if (rfp == NULL)
    {
        // LOG: 记录日志
        rfp = fopen(ID_ROUTEFILE_PATH, "w+");
        fprintf(rfp, "%d %d %d %d", 0, 0, 0, 0);
        Route_table[0].id = 0;
        Route_table[0].type = 0;
        Route_table[0].stat = 0;
        Route_table[0].last_time = 0;
    }
    else
    {
        ret = fscanf(rfp, "%d %d %d %d", &Route_table[0].id, &Route_table[0].type, &Route_table[0].stat, &Route_table[0].last_time);
        for (i = 1; ret == 4; i++)
        {
            ret = fscanf(rfp, "%d %d %d %d", &Route_table[i].id, &Route_table[i].type, &Route_table[i].stat, &Route_table[i].last_time);
        }
    }
    fclose(rfp);
    return 0;
}

// ID分配
// @param
// @return 正确返回id号 错误返回-1
int getdevid()
{
    int i;
    for (i = id_cur + 1; i <= ID_NUM_MAX; i++)
    {
        if (i == ID_NUM_MAX)
            i = ID_BYTE_BEGING;
        if (ID_POOL[i] == ID_FREE)
        {
            ID_POOL[i] = ID_USED;
            id_cur = i;
            return i;
        }
        if (i == id_cur)
            return -1;
    }
}

// ID释放
void releseid(int id)
{
    if (id >= ID_NUM_MAX)
        return;
    if ((id & 0x00FF) == 0x000D) // 去除0x0D的影响
    {
        ID_POOL[id] = ID_USED;
    }
    else
    {
        ID_POOL[id] = ID_FREE;
    }
}

// 路由表管理
// 加入表
int join_route(int id, int com)
{
    if (id >= ID_NUM_MAX)
        return -1;
    // FIXME: 应该使用动态数据结构 为了节省开发时间 采用当前模式
    Route_table[id].type = com;
    Route_table[id].stat = DEV_STA_ACTIVE;
    Route_table[id].last_time = get_time();
}

// 匹配表
int match_route(int id, int *com)
{
    if (id >= ID_NUM_MAX)
        return -1;
    if (Route_table[id].type == 0)
    {
        return -1;
    }
    if (com == NULL)
    {
        return 0;
    }
    *com = Route_table[id].type;
    return 0;
}

// 删除表
int delete_route(int id)
{
    if (id >= ID_NUM_MAX)
        return -1;
    Route_table[id].type = 0;
    Route_table[id].stat = 0;
    Route_table[id].last_time = 0;
}

// 修改表
//
int modify_route(int id, int sta)
{
    if (id >= ID_NUM_MAX)
        return -1;
    Route_table[id].stat = sta;
    Route_table[id].last_time = 0;
}

// 检查表
void check_route(void)
{

    // 检查超时
}