#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

//////////////////////////////////////////////////////////////////////////////
// spi使用前的准备 返回spi的文件描述符 //
//////////////////////////////////////////////////////////////////////////////
int initSPI()
{
    int spiFd; //spi的文件描述符
    //初始化所用到的IO引脚

    //初始化SPI通道0，并设置为最大速度32000000
    spiFd=wiringPiSPISetup(0,500000);
    if(spiFd==-1)
    {
        printf("init spi failed!\n");
    }  
}

int init_GPIO() {
    pinMode(1, OUTPUT);
}

int main()
{
    char Data[1]={0xAA}; //定义读写的数据,在该数据被写入的时候，同时也从总线中读出数据
    int i=0;
    //初始化wiringPI的库函数
    if(wiringPiSetup()<0)
    {
        printf("init wiringPi error\n");
    }
    initSPI(); //spi的初始化

    wiringPiSPIDataRW(0,Data,1); //向总线中写入&个数据
    wiringPiSPIDataRW(0,Data,1); //向总线中写入&个数据
    printf("read spi data is:\n"); //读出总线的数据

    digitalWrite(1, HIGH);
    printf("%.2X\n",Data[0]);

 return 0;
}