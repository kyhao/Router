// 路由表测试文件
#include <stdio.h>
#include "routetable.h"

#define ID_NUM 100

int main()
{
    
    // 测试初始化函数
    route_id_init();

    // 测试id分配函数
    int id[ID_NUM], i;
    for (i = 0; i < ID_NUM; i++)
    {
        id[i] = id_Alloca();
        printf("%d ", id[i]);
    }
    printf("\n");
    id_Release(300); // 测试id释放函数
    idpool_Update(); // 测试id池更新函数
    idpool_rest();   // 测试id池重置函数
    // id函数族测试完毕;单线程情况下没有问题


    return 0;
}