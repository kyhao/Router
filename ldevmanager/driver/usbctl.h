#ifndef _USBCTL_H_
#define _USBCTL_H_

#include <stddef.h>

// 1.openport
// 2.set_opt
int open_port(int comport);
int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop);
int write_port(int fd, const void *buf, size_t len);
int read_port(int fd, void *buf, size_t len);

#endif