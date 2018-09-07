#include <stdio.h>
#include <string.h>
#include "configp.h"

#define MAXCount 100

CFILE *conf_open(const char *path)
{
    return fopen(path, "w+");
}

int conf_get(CFILE *confd, const char *key, char *value)
{
    if(value == NULL)
    fgets(,MAXCount, confd);
}

int conf_update(CFILE *confd, const char *key, const char *value)
{
}

int conf_add(CFILE *confd, const char *key, const char *value)
{
}

int conf_delete(CFILE *confd, const char *key)
{
}

int conf_close(CFILE *confd)
{
}