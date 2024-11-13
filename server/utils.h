#include "structs.h"

#ifndef UTILS
#define UTILS

int remove_player(Server *s, Player *p);

int remove_game(Server *s, Game *g);

int send_msg(Player *p, MsgType type, char *msg);

/**
 * Check if name is valid game name / nickname
 * @return 1 - valid, 0 - invalid
 */
int is_name_valid(char* name);

PState transition(PState state, PEvent en);

#endif
