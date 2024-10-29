#include "structs.h"
#include <netinet/in.h>

#ifndef SERVER
#define SERVER

int server_create(Server *s, in_addr_t addr, int port);

int server_handle(Server *s, int (*f)(Server *s, MsgType type, int fd, char *msg));

#endif