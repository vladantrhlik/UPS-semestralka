#include "utils.h"
#include "structs.h"
#include <stdlib.h>


PState transitions[STATE_COUNT][EVENT_COUNT] = {
	[ST_CONNECTED][EV_LOGIN] = ST_LOGGED,
	[ST_LOGGED][EV_CREATE] = ST_WAITING,
	[ST_LOGGED][EV_JOIN] = ST_NO_TURN,
	[ST_WAITING][EV_JOIN] = ST_ON_TURN,
};

PState transition(PState state, PEvent ev) {
	return transitions[state][ev];
}

int remove_player(Server *s, Player *p) {
	if (!s || !p) return -1;

	// get index in s->players
	int index = p->index;
	free(p);
	// move last player to index
	s->players[index] = s->players[s->player_count - 1];
	s->players[s->player_count - 1] = NULL;
	s->player_count--;

	return 0;
}

