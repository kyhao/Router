#ifndef USBCTL_H
#define USBCTL_H

//#include <stddef.h>
//
int open_port(int comport);
int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop);
int write_port(int fd, const void *buf, size_t count);
int read_port(int fd, void *buf, size_t count);

#endif