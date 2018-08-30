#include <stdio.h>
#include "routetable.h"

int main(){

    // 测试初始化函数
    route_id_init();

    // 测试id分配函数
    int id[100], i;
    for(i = 0; i < 100; i++){
        id[i] = id_Alloca();
        printf("%d ", id);
    }
    printf("\n");


    return 0;
}