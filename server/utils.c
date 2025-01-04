#include "utils.h"
#include "structs.h"
#include "game.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

#include "structs.h"

PState transitions[STATE_COUNT][EVENT_COUNT] = {
	[ST_CONNECTED][EV_LOGIN] = ST_LOGGED,
	[ST_LOGGED][EV_CREATE] = ST_WAITING,
	[ST_LOGGED][EV_JOIN] = ST_NO_TURN,
	[ST_WAITING][EV_JOIN] = ST_ON_TURN,
	// next turn of turning player
	[ST_ON_TURN][EV_BAD_TURN] = ST_NO_TURN,
	[ST_ON_TURN][EV_GOOD_TURN] = ST_ON_TURN,
	// ending game
	[ST_ON_TURN][EV_GAME_END] = ST_NO_TURN,
	[ST_NO_TURN][EV_GAME_END] = ST_NO_TURN,
	// changing turns
	[ST_ON_TURN][EV_NO_TURN] = ST_NO_TURN,
	[ST_ON_TURN][EV_ON_TURN] = ST_ON_TURN,
	[ST_NO_TURN][EV_NO_TURN] = ST_NO_TURN,
	[ST_NO_TURN][EV_ON_TURN] = ST_ON_TURN,
	// leaving
	[ST_NO_TURN][EV_LEAVE] = ST_LOGGED,
	[ST_ON_TURN][EV_LEAVE] = ST_LOGGED,
	[ST_WAITING][EV_LEAVE] = ST_LOGGED,
};

PState transition(PState state, PEvent ev) {
	return transitions[state][ev];
}
Player *find_connected_player(Server *s, int fd) {
	for (int i = 0; i < s->player_count; i++) {
		if (s->players[i]->fd == fd && s->players[i]->state != ST_DISCONNECTED) return s->players[i];
	}
	return NULL;
}

int invalid_msg(Server *s, Player *p) {
	if (!p) return 1;
	p->invalid_msg_count++;
	if (p->invalid_msg_count >= MAX_INVALID_MSG) {
		printf("Disconnecting player %d\n", p->fd);
		int fd = p->fd;
		close(fd);
		FD_CLR( fd, &s->client_socks );
		return 1;
	}
	return 0;
}

char *typeMessages[] = {
	[OK] = "OK",
	[ERR] = "ERR",
	[OP_JOIN] = "OP_JOIN",
	[OP_DISCONNECT] = "OP_LEAVE",
	[WIN] = "WIN",
	[LOSE] = "LOSE",
	[TURN] = "TURN",
	[ON_TURN] = "ON_TURN",
	[OP_TURN] = "OP_TURN",
	[ACQ] = "ACQ",
	[OP_ACQ] = "OP_ACQ",
	[PONG] = "PONG",
	[PING] = "PING"
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

	if (type != PONG) printf("to %s: %s\n", p->name, buffer);
	send(p->fd, buffer, strlen(buffer), 0);
	return 0;
}

int is_name_valid(char* name) {
	int len = strlen(name);
	if (len > MAX_NAME_LEN || len < MIN_NAME_LEN) {
		return 0;
	}
	for (int i = 0; i < len; i++) {
		if (!isalnum(name[i]) && name[i] != '_') {
			return 0;
		}
	}
	return 1;
}
