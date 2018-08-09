#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    int i, pid;
    // 检查输入变量
    for (i = 0; i < argc; i++)

        printf("Argument %d is %s.\n", i, argv[i]);

    // 创建子进程1  内部网络硬件管理
    pid = fork();
    if (pid == -1)
        perror("Fork 1 <Error>\n");
    if (pid == 0)
    {
        // 子程序开始
        printf("Subprocess1: pid:%d :Lan-Server\n", getpid());

        return 0; // 子程序结束
    }

    // 创建子进程2  外部网络硬件管理
    pid = fork();
    if (pid == -1)
        perror("Fork 2 <Error>\n");
    if (pid == 0)
    {
        // 子程序开始
        printf("Subprocess2:pid:%d :Wan-Server\n", getpid());

        return 0; // 子程序结束
    }

    // 创建子进程3  设备管理/协议处理
    pid = fork();
    if (pid == -1)
        perror("Fork 3 <Error>\n");
    if (pid == 0)
    {
        // 子程序开始
        printf("Subprocess3:pid:%d :Mananger-Server\n", getpid());
        
        return 0; // 子程序结束
    }
    int stat;
    int spid;
    while ((spid = wait(&stat)) > -1)
    {
        printf("pid:%d :finished\n", spid);
    }
    printf("main:finish\n");
    return 0;
}