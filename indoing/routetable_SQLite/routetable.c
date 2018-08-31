// 路由表和ID池管理模块
// Made 2018/8/17
// Last Modify 2018/8/30 加入多线程/进程支持（线程安全）
// 包含ID管理函数和路由表管理函数
// 模块具备多线程/进程调用能力
// 采用POSIX标准信号量 -lpthread
// 采用SQLite 需要有SQLite支持 -lsqlite3
// 可优化项：可以一次执行多条SQL语句，减少调用开销

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

// *** SQLite 语句组 *** //
#define SQL_CREATEROUTER "create table if not exists route_table(" \
                         "id INTEGER PRIMARY KEY NOT NULL,"        \
                         "dtype INTEGER NOT NULL,"                 \
                         "stat INTEGER NOT NULL,"                  \
                         "reg_time DATE NOT NULL,"                 \
                         "last_time DATE NOT NULL"                 \
                         ");"
#define SQL_INSERT_FORMAT "INSERT INTO route_table (id,dtype,stat,reg_time,last_time) " \
                          "VALUES (%d, %d, %d, %d, %d );"
#define SQL_SELECT_FORMAT "SELECT * FROM route_table " \
                          "WHERE id == %d;"
#define SQL_UPDATE_FORMAT "UPDATE route_table "             \
                          "SET stat = %d, last_time = %ld " \
                          "WHERE id == %d;"
#define SQL_MAINTAIN_FORMAT "UPDATE route_table "           \
                            "SET stat = 2 "                 \
                            "WHERE %ld - last_time > 7200;" \
                            "UPDATE route_table "           \
                            "SET stat = 3 "                 \
                            "WHERE %ld - last_time > 172800;" // 两小时超时时间,两天释放时间
#define SQL_DELETE_FORMAT "DELETE FROM route_table " \
                          "WHERE id == %d;"
// *** SQLite 语句组结尾 *** //

#define get_time() time(NULL) // 获取当前时间距离1970-01-01的秒数

static char *ID_POOL;       // ID池
static int id_cur;          // ID池游标
static sqlite3 *rdb = NULL; // route表
static sem_t mutex_id;      // id池互斥锁
static char *c_errmesg;     // 公用SQLite错误收集变量
static int idpool_mode;     // id表存储模式 0表示手动更新 1表示自动更新

// 路由表和ID统一初始化
void route_id_init(void)
{
    int ret, size;
    // 配置id及路由表的大小
    size = IDPOOL_SIZE;
    ID_POOL = (char *)malloc(sizeof(char) * (size + 1)); // 无需释放常驻内存
    id_cur = ID_BYTE_BEGING;
    // 初始化无名信号量互斥锁
    ret = sem_init(&mutex_id, 0, 1); // id信号量
    // 路由表和ID初始化
    idpool_init();
    route_init();
}

// 读取id池文件，将信息读入内存
void idpool_init(void)
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
    idpool_mode = 1; // 自动更新idpool文件
}

// 重置id池
void idpool_rest(void)
{
    int i;
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
    id_cur = ID_BYTE_BEGING;
    idpool_Save();
}

// ID分配函数
// 根据id池分配id，采用互斥锁
// @param
// @return 返回值：正确返回id号，错误返回-1
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
    if (idpool_mode)
        idpool_Save();
    return i;
}

// ID释放函数
// @param
// @id 输入需释放的id
void id_Release(int id)
{
    if (id >= ID_BYTE_END || id < ID_BYTE_BEGING)
        return;
    if ((id & 0x00FF) == 0x000D) // 去除0x0D的影响
    {
        ID_POOL[id] = ID_USED;
    }
    else
    {
        ID_POOL[id] = ID_FREE;
    }
    if (idpool_mode)
        idpool_Save();
}

// id池文件更新
// 将内存中id池写入文件当中
void idpool_Save(void)
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
        fprintf(ifp, "%d:%s", id_cur, ID_POOL);
    }
    fclose(ifp);
    sem_post(&mutex_id);
}

// 路由表初始化函数
// @param
// @return 返回值：0:正常; 0x0304:数据库打开错误; 0x0305:SQL语句执行出错
int route_init(void)
{
    int ret;
    sqlite3_config(SQLITE_CONFIG_MULTITHREAD); // 开启多线程支持
    ret = sqlite3_open(ROUTEDB_PATH, &rdb);
    if (ret)
    {
        // fprintf函数格式化输出错误信息到指定的stderr文件流中
        // fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(rdb)); //sqlite3_errmsg(rdb)用以获得数据库打开错误码的英文描述。
        sqlite3_close(rdb);
        return 0x0304;
    }
    ret = sqlite3_exec(rdb, SQL_CREATEROUTER, NULL, NULL, &c_errmesg);
    if (ret != SQLITE_OK)
    {
        // 记录日志
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
// @id 需加入的设备id
// @dtype 设备id设备类型/接口编号
// @return 返回值：0:正常; 0x0305:SQL语句执行出错
int route_join(int id, int dtype)
{
    // id越界控制
    if (id >= IDPOOL_SIZE || id < ID_BYTE_BEGING)
        return 0x0302;
    int ret;
    char sql[100];
    sprintf(sql, SQL_INSERT_FORMAT, id, dtype, DEV_STA_ACTIVE, get_time(), get_time());
    ret = sqlite3_exec(rdb, sql, NULL, NULL, &c_errmesg);
    if (ret != SQLITE_OK)
    {
        // 需记录错误日志
        ret = 0x0305;
    }
    else
    {
        ret = 0;
    }
    sqlite3_free(c_errmesg);
    return ret;
}

// 根据id获取，对应id设备/接口编号
// @param
// @id 需查询的设备id
// @route 传入指针或NULL，返回对应设备id信息， 若为NULL只做查询匹配
// @return 返回值： 0:正常; 0x0302:id越界; 0x0305:SQL语句执行出错; 0x0307:无匹配数据
int route_march(int id, Routetable *route)
{
    // id越界控制
    if (id >= IDPOOL_SIZE || id < ID_BYTE_BEGING)
        return 0x0302;
    char **pResult;
    char *errmsg;
    int nRow, nCol, ret;
    char sql[100];
    sprintf(sql, SQL_SELECT_FORMAT, id);
    ret = sqlite3_get_table(rdb, sql, &pResult, &nRow, &nCol, &errmsg);
    if (ret != SQLITE_OK)
    {
        // 记录错误日志
        ret = 0x0305;
    }
    else
    {
        if (nRow == 0)
        {
            ret = 0x0307;
        }
        else
        {
            ret = 0;
            if (route != NULL)
            {
                route->id = atoi(pResult[5]);
                route->dtype = atoi(pResult[6]);
                route->stat = atoi(pResult[7]);
                route->reg_time = atoi(pResult[8]);
                route->last_time = atoi(pResult[9]);
            }
        }
    }
    sqlite3_free(errmsg);
    sqlite3_free_table(pResult);
    return ret;
}

// 删除表
// @param
// @id 需释放的设备id
void route_release(int id)
{
    if (id >= IDPOOL_SIZE || id < ID_BYTE_BEGING)
        return;
    char sql[100];
    sprintf(sql, SQL_DELETE_FORMAT, id);
    sqlite3_exec(rdb, sql, NULL, NULL, &c_errmesg);
    sqlite3_free(c_errmesg);
    id_Release(id);
}

// 表的状态和设备接口更新(可更新，优化性能)
// @param
// @id 需更新信息的设备id
// @sta 设备id需修改的设备状态
// @return 返回值：0:正常; 0x0302:id越界; 0x0305:SQL语句执行出错
int route_update(int id, int sta)
{
    // id越界控制
    if (id >= IDPOOL_SIZE || id < ID_BYTE_BEGING)
        return 0x0302;
    int ret;
    ret = route_march(id, NULL);
    if (ret != 0)
        return ret;
    char sql[100];
    sprintf(sql, SQL_UPDATE_FORMAT, sta, get_time(), id);
    ret = sqlite3_exec(rdb, sql, NULL, NULL, &c_errmesg);
    if (ret != SQLITE_OK)
    {
        // 需记录错误日志
        ret = 0x0305;
    }
    else
    {
        ret = 0;
    }
    sqlite3_free(c_errmesg);
    return 0;
}

// 表的维护
// 查询超时记录
// @param
// @return 返回值：0:正常; 0x0305:SQL语句执行出错
int route_maintain(void)
{
    int ret;
    char sql[200];
    sprintf(sql, SQL_MAINTAIN_FORMAT, get_time(), get_time());
    ret = sqlite3_exec(rdb, sql, NULL, NULL, &c_errmesg);
    if (ret != SQLITE_OK)
    {
        // 需记录错误日志
        ret = 0x0305;
    }
    else
    {
        ret = 0;
    }
    sqlite3_free(c_errmesg);
    return ret;
}

// idpool/route资源回收函数
void route_recover(void)
{
    char *sql = "SELECT id FROM route_table "
                "WHERE dtype == 3;";
    char **pResult;
    char *errmsg;
    int nRow, nCol, ret, i;
    ret = sqlite3_get_table(rdb, sql, &pResult, &nRow, &nCol, &errmsg);
    if (ret == SQLITE_OK && nRow != 0)
    {
        for (i = 0; i < nRow; i++)
        {
            route_release(atoi(pResult[i + 1]));
        }
    }
    sqlite3_free(errmsg);
    sqlite3_free_table(pResult);
}