#ifndef _CONFIG_H_
#define _CONFIG_H_

#define CFILE FILE

CFILE *conf_open(const char *path);
int conf_get(CFILE *confd, const char *key, char *value);
int conf_update(CFILE *confd, const char *key, const char *value);
int conf_add(CFILE *confd, const char *key, const char *value);
int conf_delete(CFILE *confd, const char *key);
int conf_close(CFILE *confd);

#endif