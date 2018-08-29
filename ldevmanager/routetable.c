// 2018/8/17
// 包含ID管理函数和路由表管理函数
// 当前模块不具备多线程能力
#include <stdio.h>
#include <time.h>
#include <semaphore.h> // Posix标准信号量 pthread库
#include "routetable.h"

#define ID_BYTE_BEGING 0x01 // ID池起始位置
#define ID_BYTE_END 0xFF    // ID池结束位置

#define IDPOOL_SIZE 2048 // 最大id分配数 UPDATE: 可通过配置文件修改
#define ID_FREE '0'      // 当前ID未使用
#define ID_USED '1'      // 当前ID已使用
#define ID_OUTTIME '2'   // 当前ID超时状态未知

// 路由表和ID池本地库路径
#define ID_POOLFILE_PATH "./idpool.db"      // ID池管理文件
#define ID_ROUTEFILE_PATH "./routetable.db" // 路由表路径

static char *ID_POOL;           // ID池
static int id_cur;              // ID池游标
static Routetable *Route_table; // 路由表数据结构
static sem_t mutex_id;          // id池互斥锁
static sem_t mutex_route;       // route互斥锁

// 时间获取函数
time_t get_time()
{
    time_t t;
    time(&t);
    return t;
}

// 路由表和ID统一初始化
void route_id_init(void)
{
    int ret, size;
    // 配置id及路由表的大小
    size = IDPOOL_SIZE;
    ID_POOL = (char *)malloc(sizeof(char) * (size + 1));
    Route_table = (Routetable *)malloc(sizeof(Routetable) * (size));
    // 初始化无名信号量互斥锁
    ret = sem_init(&mutex_id, 0, 1);    // id信号量
    ret = sem_init(&mutex_route, 0, 1); // route信号量
    // 路由表和ID初始化
    idpool_init();
    route_init();
}

// 读取id池文件，将信息读入内存
// @param
// @return 返回0
int idpool_init(void)
{
    // id池初始化
    int i, ret;
    FILE *ifp = NULL; // id文件指针
    // ID文件打开读取
    ifp = fopen(ID_POOLFILE_PATH, "r"); // 打开失败 创建文件
    if (ifp == NULL)
    {
        // 初始化id池文件
        ifp = fopen(ID_POOLFILE_PATH, "w+");
        for (i = 0; i < IDPOOL_SIZE; i++)
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
        ID_POOL[IDPOOL_SIZE] = '\0';
        fprintf(ifp, "%d:%s", ID_BYTE_BEGING, ID_POOL);
        id_cur = ID_BYTE_BEGING;
    }
    else
    {
        // 读取id池文件至内存
        fscanf(ifp, "%d:%s", &id_cur, ID_POOL);
    }
    fclose(ifp);
    return 0;
}

// ID分配函数
// 根据id池分配id，采用互斥锁
// @param
// @return 正确返回id号 错误返回-1
int id_Alloca(void)
{
    sem_wait(&mutex_id);
    int i;
    for (i = id_cur + 1; i <= IDPOOL_SIZE; i++)
    {
        if (i == IDPOOL_SIZE)
            i = ID_BYTE_BEGING;
        if (ID_POOL[i] == ID_FREE)
        {
            ID_POOL[i] = ID_USED;
            id_cur = i;
            break;
        }
        if (i == id_cur)
        {
            i = -1;
            break;
        }
    }
    sem_post(&mutex_id);
    return i;
}

// ID释放函数
// @param
// @id 输入需释放的id
// @return 正确返回0
void id_Release(int id)
{
    if (id >= ID_BYTE_END || id < ID_BYTE_BEGING)
        return 0x0301;
    if ((id & 0x00FF) == 0x000D) // 去除0x0D的影响
    {
        ID_POOL[id] = ID_USED;
    }
    else
    {
        ID_POOL[id] = ID_FREE;
    }
    return 0;
}

// id池文件更新
// 将内存中id池写入文件当中
// @param
// @return 0正常
int id_Update(void)
{
    sem_wait(&mutex_id);
    int i, ret;
    FILE *ifp = NULL; // id文件指针
    // ID文件打开读取
    ifp = fopen(ID_POOLFILE_PATH, "r"); // 打开失败则创建文件
    if (ifp == NULL)
    {
        // 初始化id池文件
        ifp = fopen(ID_POOLFILE_PATH, "w+");
        for (i = 0; i < IDPOOL_SIZE; i++)
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
        ID_POOL[IDPOOL_SIZE] = '\0';
        fprintf(ifp, "%d:%s", ID_BYTE_BEGING, ID_POOL);
        id_cur = ID_BYTE_BEGING;
    }
    else
    {
        // 内存中id池数据写入文件
        fprintf(ifp, "%d:%s", ID_BYTE_BEGING, ID_POOL);
    }
    fclose(ifp);
    sem_post(&mutex_id);
    return 0;
}

// 路由表初始化函数
// @param
// @return 错误返回 0正常
int route_init(void)
{
    // routetable初始化
    int ret;
    FILE *rfp = NULL; // 路由表文件指针
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

// 路由表管理
// 将新的设备id加入表
// @param
// @id 设备id
// @dtype 设备类型/接口编号
// @return 错误返回
int route_join(int id, int dtype)
{
    // 判断id是否越界
    if (id >= IDPOOL_SIZE || id < ID_BYTE_BEGING)
        return 0x0302;

    sem_wait(&mutex_route);
    // FIXME: 应该使用动态数据结构 为了节省开发时间 采用当前模式
    Route_table[id].id = id;
    Route_table[id].dtype = dtype;
    Route_table[id].stat = DEV_STA_ACTIVE;
    Route_table[id].reg_time = get_time();
    Route_table[id].last_time = get_time();
    sem_post(&mutex_route);
    return 0;
}

// 根据id获取，对应id设备/接口编号
// @param
// @id 需查询的设备id
// @dtype 返回对应设备id的接口编号
// @return 错误返回
int route_march(int id, int *dtype)
{
    if (id >= IDPOOL_SIZE || id < ID_BYTE_BEGING || dtype == NULL)
        return 0x0302;

    if (Route_table[id].id == 0)
    {
        *dtype = DEV_TYPE_NONE;
        return 0x0303;
    }
    *dtype = Route_table[id].dtype;
    return 0;
}

// 删除表
// @param
// @id 需释放的设备id
// @return 返回错误
int route_release(int id)
{
    if (id >= IDPOOL_SIZE || id < ID_BYTE_BEGING)
        return 0x0302;
    Route_table[id].id = 0;
    Route_table[id].dtype = DEV_TYPE_NONE;
    Route_table[id].stat = DEV_STA_NONE;
    Route_table[id].reg_time = 0;
    Route_table[id].last_time = 0;
    return 0;
}

int route_modify(int id, int sta, int dtype);
int route_maintain(void);
int route_update(void);