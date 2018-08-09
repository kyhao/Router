#include <stdio.h>
#include "ldevmanager.h"

#define LORA 1
#define BLUETOOTH 2
#define WIFI 3
#define NB_IOT 4
#define MODEL_4G 5

static int devtype;

int ldev_gdata(int devid, uint8_t *outbuf, int *len)
{
    devtype = ldev_gtype(devid);
    switch (devtype)
    {
    case WIFI:

        break;
    case LORA:

        break;
    case BLUETOOTH:

        break;
    default:
        return 3;
    }
}
