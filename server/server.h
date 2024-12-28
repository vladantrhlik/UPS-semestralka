#include "structs.h"
#include <netinet/in.h>

#ifndef SERVER
#define SERVER

int server_create(Server *s, char *config_file);

int server_handle(Server *s);

int handle_msg(Server *s, SEvent type, int fd, char *msg);

int remove_player(Server *s, Player *p);

int remove_game(Server *s, Game *g);

#endif
