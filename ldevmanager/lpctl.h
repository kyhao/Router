#ifndef _LINFOCTL_H_
#define _LINFOCTL_H_

typedef struct _ARG
{
    char *buf;
    int dfd;
}ARG_M;

int
lpctl_init();
int lpctl(char *in_buf, int dfd);
void lp_ctl(void *arg);

#endif