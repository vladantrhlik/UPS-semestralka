#include "structs.h"

#ifndef UTILS
#define UTILS

int remove_player(Server *s, Player *p);

int remove_game(Server *s, Game *g);

PState transition(PState state, PEvent en);

#endif
