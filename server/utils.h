#include "structs.h"

#ifndef UTILS
#define UTILS

int send_msg(Player *p, MsgType type, char *msg);

int invalid_msg(Server *s, Player *p);

Player *find_connected_player(Server *s, int fd);

/**
 * Check if name is valid game name / nickname
 * @return 1 - valid, 0 - invalid
 */
int is_name_valid(char* name);

PState transition(PState state, PEvent en);

#endif
