// test file
#include <stdio.h>
#include <stdlib.h>
#include "localProtocol.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

char input[30] = {0xFF, 0x01, 0x41, 0x01, 0x03, 0x61, 0x62, 0x63};
char output[30];

Packet pin_buf;
Packet *pout_buf;

int main(int argc, char **argv)
{
    int ret = -1;
    printf("%d\n", argc);
    pin_buf.header.datalen = 10;
    pin_buf.header.ver = 1;
    pin_buf.header.type = 1;
    pin_buf.header.id = 255;
    int i, len;
    for (i = 0; i < 10; i++)
    {
        pin_buf.data[i] = ('a' + i);
    }
    if (argc == 1)
    {
        pout_buf = (Packet *)malloc(sizeof(Packet));
        printf("start:%x, %x\n", *input, 0xFF);
        ret = lprotocol_decode(input, pout_buf);
        ret = lprotocol_package(&pin_buf, output, &len, 1);
    }
    else
    {
        argv[1];
    }
    if (ret == 0)
    {
        printf("decode:");
        printf("ret:%d\n", ret);
        printf("header:%d,%d,%d,%d\ndata:%s\n", pout_buf->header.ver, pout_buf->header.id, pout_buf->header.type, pout_buf->header.datalen, pout_buf->data);
        printf("Package:\n");
        for (i = 0; i < len; i++)
        {
            printf("0x%2x,", output[i]);
        }
        printf("\n");
    }
}