#include "structs.h"

#ifndef UTILS
#define UTILS

int remove_player(Server *s, Player *p);

PState transition(PState state, PEvent en);

#endif
