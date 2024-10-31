#include "utils.h"
#include "structs.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>


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
	if (index < s->player_count - 1) {
		// move last player to index
		s->players[index] = s->players[s->player_count - 1];
		s->players[s->player_count - 1] = NULL;
		s->player_count--;
		s->players[index]->index = index;
	} else {
		// remove last player
		s->players[index] = NULL;
		s->player_count--;
	}

	return 0;
}

int remove_game(Server *s, Game *g) {
	if (!s || !g) return -1;

	// get index in s->games
	int index = -1;
	for (int i = 0; i < s->game_count; i++) {
		Game *tmp = s->games[i];
		if (!strcmp(tmp->name, g->name)) {
	  		index = i;
	  		break;
		}
	}

	if (index == -1) {
		printf("Game not found when removing\n");
		return -1;
	}

	game_free(g);
	if (index < s->game_count - 1) {
		// move last player to index
		s->games[index] = s->games[s->game_count - 1];
		s->games[s->game_count - 1] = NULL;
		s->game_count--;
	} else {
		// remove last
		s->games[index] = NULL;
		s->game_count--;
	}

	return 0;

}

char *typeMessages[] = {
	[OK] = "OK",
	[ERR] = "ERR",
	[OP_JOIN] = "OP_JOIN",
	[OP_DISCONNECT] = "OP_LEAVE"
};

int send_msg(Player *p, MsgType type, char *msg) {
	static char buffer[128];
	if (!p || p->state == ST_DISCONNECTED) {
		printf("Error sending message '%s' to %s (%d)\n", msg, p->name, p->fd);
		return 1;
	}

	if (msg) {
		sprintf(buffer, "%s%s\n", typeMessages[type], msg);
	} else {
		sprintf(buffer, "%s\n", typeMessages[type]);
	}

	printf("to %s: %s\n", p->name, buffer);
	send(p->fd, buffer, strlen(buffer), 0);
	return 0;
}
