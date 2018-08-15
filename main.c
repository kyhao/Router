#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// 主函数
int main(int argc, char **argv)
{
    // 检查输入变量
    printf("Argument %d is %s.\n", argc, argv[0]);

    int i, pid, ret;
    int pfd_lm[2];
    int pfd_ml[2];
    ret = pipe(pfd_lm);
    ret = pipe(pfd_ml);

    char lm_0[2] = {(char)pfd_ml[0], '\0'};
    char lm_1[2] = {(char)pfd_lm[1], '\0'};
    char ml_0[2] = {(char)pfd_lm[0], '\0'};
    char ml_1[2] = {(char)pfd_ml[1], '\0'};
    // 创建子进程1  内部网络硬件管理
    pid = fork();
    if (pid == -1)
        perror("Fork 1 <Error>\n");
    if (pid == 0)
    {
        // 子程序开始
        printf("Subprocess1: pid:%d :Lan-Server\n", getpid());
        // FIXME: 参数变量未完成，需先完成tools/c_string.c
        ret = execl("./ldevmanager/run", "run", lm_0, lm_1, NULL);
        return 0;
        // 子程序结束
    }

    // 创建子进程2  外部网络硬件管理
    pid = fork();
    if (pid == -1)
        perror("Fork 2 <Error>\n");
    if (pid == 0)
    {
        // 子程序开始
        printf("Subprocess2:pid:%d :Wan-Server\n", getpid());

        return 0;
        // 子程序结束
    }

    // 创建子进程3  设备管理/协议处理
    pid = fork();
    if (pid == -1)
        perror("Fork 3 <Error>\n");
    if (pid == 0)
    {
        // 子程序开始
        printf("Subprocess3:pid:%d :Mananger-Server\n", getpid());
        ret = execl("./routermanager/run", "run", ml_0, ml_1, NULL);
        printf("process 3 : ret = %d", ret);
        return 0;
        // 子程序结束
    }
    // // Debug
    // char rx_buf[100];
    // while (1)
    // {
    //     ret = read(pfd_lm[0], rx_buf, 100);
    //     int j;
    //     printf("MAIN: ");
    //     for (j = 0; j <= ret; j++)
    //     {
    //         printf("\033[29m%02X \033[0m", rx_buf[j]);
    //     }
    //     printf("\n");
    // }

    int stat;
    int spid;
    while ((spid = wait(&stat)) > -1)
    {
        printf("pid:%d :finished\n", spid);
    }
    printf("main:finish\n");
    return 0;
}