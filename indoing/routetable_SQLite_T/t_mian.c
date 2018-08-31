// 路由表测试文件
#include <stdio.h>
#include "routetable.h"

#define ID_NUM 100

int main()
{
    int ret;

    route_id_init(); // 测试初始化函数 PASS

    int id[ID_NUM], i;
    for (i = 0; i < ID_NUM; i++)
    {
        id[i] = id_Alloca(); // 测试ID分配函数 PASS
        printf("%d ", id[i]);
    }
    printf("\n");
    id_Release(300); // 测试id释放函数 PASS
    idpool_Save();   // 测试id池更新函数 PASS
    idpool_rest();   // 测试id池重置函数 PASS

    // 测试路由表添加函数 PASS
    ret = route_join(id[0], DEV_TYPE_BLUETOOTH);
    printf("route_join  0 ret : %d \n", ret);

    ret = route_join(id[1], DEV_TYPE_BLUETOOTH);
    printf("route_join 1 ret : %d \n", ret);

    // 测试路由表匹配函数 PASS
    ret = route_march(id[1], NULL);
    printf("route_march 1 ret : %d \n", ret);

    ret = route_march(id[2], NULL);
    printf("route_march 2 ret : %d \n", ret);

    Routetable routetable;
    ret = route_march(id[1], &routetable);
    printf("route_march 1 NO NULL ret : %d \n", ret);
    printf("route_table : %ld \n", routetable.last_time);

    // 测试路由表释放函数 PASS
    route_release(id[0]);
    printf("route_release  0 ret : %d \n", ret);

    route_release(id[2]);
    printf("route_release  2 ret : %d \n", ret);

    // 更新表测试 PASS
    ret = route_update(id[1], DEV_STA_OUTTIME);
    printf("route_update 1 ret : %d \n", ret);
    route_march(id[1], &routetable);
    printf("route_table : %ld , %ld\n", routetable.last_time, routetable.reg_time);

    ret = route_update(id[2], DEV_STA_OUTTIME);
    printf("route_update 2 ret : %d \n", ret);
    route_march(id[2], &routetable);
    printf("route_table : %ld , %ld\n", routetable.last_time, routetable.reg_time);

    // 表维护测试
    ret = route_maintain();
    printf("route_maintain ret : %d \n", ret);

    return 0;
}