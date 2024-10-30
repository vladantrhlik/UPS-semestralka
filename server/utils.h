#include "structs.h"

#ifndef UTILS
#define UTILS

int remove_player(Server *s, Player *p);

int remove_game(Server *s, Game *g);

int send_msg(Player *p, MsgType type, char *msg);

PState transition(PState state, PEvent en);

#endif
