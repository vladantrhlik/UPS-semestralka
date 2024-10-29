#include "utils.h"
#include "structs.h"
#include <stdlib.h>

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
