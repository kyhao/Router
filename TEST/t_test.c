#include <stdio.h>

#define Conn(x, y) x##y
#define ToString(x) #x

#define MODE 1
#define R_N \r\n
#define _AT_CWMODE(mode)
#define AT_CWMODE(mode) ToString(AT + CWMODE = mode\r\n)
#define DEV(port) ToString(/dev/ttyUSB##port)

int main()
{
    int a = 10;
    printf("%s", DEV(a));
    printf("%s", AT_CWMODE(10));
    return 0;
}