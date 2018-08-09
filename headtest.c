// test file
#include <stdio.h>
#include <stdlib.h>
#include "localProtocol.h"
#include "mac.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

char input[30] = {0xFF, 0x01, 0x41, 0x01, 0x03, 0x61, 0x62, 0x63};
char output[30];

Packet pin_buf;
Packet *pout_buf;

int main(int argc, char **argv)
{
    int ret = -1;
    char mac[32];
    printf("%d\n", argc);
    pin_buf.header.datalen = 0;
    pin_buf.header.ver = 1;
    pin_buf.header.type = 1;
    pin_buf.header.sid[0] = 255;
    get_mac(mac);
    int i, len;
    for (i = 0; i < 12; i++)
    {
        pin_buf.header.did[i] = 0xFF;
    }
    for (i = 0; i < 0; i++)
    {
        pin_buf.data[i] = ('a' + i);
    }

    pout_buf = (Packet *)malloc(sizeof(Packet));
    printf("start:%x, %x\n", *input, 0xFF);
    ret = lprotocol_decode(input, pout_buf);
    ret = lprotocol_package(&pin_buf, output, &len, 1);
 
    if (ret == 0)
    {
        printf("decode:");
        printf("ret:%d\n", ret);
        printf("header:%d,%d,%d,%d\ndata:%s\n", pout_buf->header.ver, pout_buf->header.sid[0], pout_buf->header.type, pout_buf->header.datalen, pout_buf->data);
        printf("Package:\n");
        for (i = 0; i < len; i++)
        {
            printf("0x%2x,", output[i]);
        }
        printf("\n");
    }
}