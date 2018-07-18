#include <stdio.h>

#define MODE 1
#define R_N "\r\n"
#define _AT_CWMODE(mode) mode##R_N
#define AT_CWMODE(mode) _AT_CWMODE(mode)

int main()
{
    printf("%d", AT_CWMODE(MODE));
    return 0;
}