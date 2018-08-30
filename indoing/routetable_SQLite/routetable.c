// 2018/8/17
// 包含ID管理函数和路由表管理函数
// 当前模块不具备多线程能力
// 采用SQLite 需要有SQLite支持
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sqlite3.h>
#include <semaphore.h> // Posix标准信号量 pthread库
#include "routetable.h"

#define IDPOOL_SIZE 1024 // 最大id分配数 UPDATE: 可通过配置文件修改
#define ID_FREE '0'      // 当前ID未使用
#define ID_USED '1'      // 当前ID已使用
#define ID_OUTTIME '2'   // 当前ID超时状态未知

#define ID_BYTE_BEGING 0x01     // ID池起始位置
#define ID_BYTE_END IDPOOL_SIZE // ID池结束位置

// 路由表和ID池本地库路径
#define IDPOOLDB_PATH "./idpool.db"    // ID池管理文件
#define ROUTEDB_PATH "./routetable.db" // 路由表路径

#define get_time() time(NULL) // 获取当前时间距离1970-01-01的秒数

static char *ID_POOL;       // ID池
static int id_cur;          // ID池游标
static sqlite3 *rdb = NULL; // route表
static sem_t mutex_id;      // id池互斥锁
static sem_t mutex_route;   // route互斥锁
static char *c_errmesg;     // 公用SQLite错误收集变量

// 路由表和ID统一初始化
void route_id_init(void)
{
    int ret, size;
    // 配置id及路由表的大小
    size = IDPOOL_SIZE;
    ID_POOL = (char *)malloc(sizeof(char) * (size + 1)); // 无需释放常驻内存
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
    ifp = fopen(IDPOOLDB_PATH, "r"); // 打开失败 创建文件
    if (ifp == NULL)
    {
        // 初始化id池文件
        ifp = fopen(IDPOOLDB_PATH, "w+");
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
int id_Release(int id)
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
    ifp = fopen(IDPOOLDB_PATH, "w"); // 打开失败则创建文件
    if (ifp == NULL)
    {
        // 初始化id池文件
        ifp = fopen(IDPOOLDB_PATH, "w+");
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
    int ret;
    ret = sqlite3_open(ROUTEDB_PATH, &rdb);
    if (ret)
    {
        // fprintf函数格式化输出错误信息到指定的stderr文件流中
        // fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(rdb)); //sqlite3_errmsg(rdb)用以获得数据库打开错误码的英文描述。
        sqlite3_close(rdb);
        return 0x0304;
    }
    char *sql = "create table if not exists route_table(\
    id INTEGER PRIMARY KEY NOT NULL,\
    dtype INTEGER NOT NULL,\
    stat INTEGER NOT NULL,\
    reg_time DATE NOT NULL,\
    last_time DATE NOT NULL\
    );";
    ret = sqlite3_exec(rdb, sql, NULL, NULL, &c_errmesg);
    if (ret != SQLITE_OK)
    {
        sqlite3_free(c_errmesg);
        return 0x0305;
    }
    else
    {
        sqlite3_free(c_errmesg);
        return 0;
    }
}

// 路由表管理
// 将新的设备id加入表
// @param
// @id 设备id
// @dtype 设备类型/接口编号
// @return 错误返回
int route_join(int id, int dtype)
{
    // id越界控制
    if (id >= IDPOOL_SIZE || id < ID_BYTE_BEGING)
        return 0x0302;
    char *errmsg;
    int ret;
    sem_wait(&mutex_route);
    char *sql = "";
    ret = sqlite3_exec(rdb, sql, NULL, NULL, &c_errmesg);
    ret = (ret != SQLITE_OK) ? 0x0305 : 0;
    sqlite3_free(c_errmesg);
    sem_post(&mutex_route);
    return ret;
}

// 根据id获取，对应id设备/接口编号
// @param
// @id 需查询的设备id
// @dtype 返回对应设备id的接口编号
// @return 错误返回
int route_march(int id, int *dtype)
{
    // id越界控制
    if (id >= IDPOOL_SIZE || id < ID_BYTE_BEGING)
        return 0x0302;

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

    return 0;
}

// 修改表的状态和设备接口
// @param
// @id 设备id
// @sta 需修改的设备状态
// @dtype 需修改的设备接口
int route_modify(int id, int sta, int dtype)
{
    // id越界控制
    if (id >= IDPOOL_SIZE || id < ID_BYTE_BEGING)
        return 0x0302;

    return 0;
}

// 表的维护
int route_maintain(void)
{
    return 0;
}

// 表更新
int route_update(void)
{

    return 0;
}