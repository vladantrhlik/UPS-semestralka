#include "structs.h"
#include <netinet/in.h>

#ifndef SERVER
#define SERVER

int server_create(Server *s, char *config_file);

int server_handle(Server *s, int (*f)(Server *s, SEvent type, int fd, char *msg));

#endif
