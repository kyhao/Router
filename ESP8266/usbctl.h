#ifndef USBCTL_H
#define USBCTL_H

int open_port(int comport);
int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop);

#endif