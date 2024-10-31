#include "handlers.h"
#include "structs.h"
#include "game.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

char *states[STATE_COUNT] = {
	[ST_CONNECTED] = "connected",
	[ST_LOGGED] = "logged",
	[ST_DISCONNECTED] = "disconnected",
	[ST_WAITING] = "waiting",
	[ST_NO_TURN] = "not on turn",
	[ST_ON_TURN] = "on turn"
};

int list_handler(Server *s, Player *p) {
	printf("\nPlayers:\n");
	for (int i = 0; i < s->player_count; i++) {
		printf("[%d]: %s (%d) - %s\n", i, s->players[i]->name, s->players[i]->fd, states[s->players[i]->state]);
	}
	printf("\nGames:\n");
	for (int i = 0; i < s->game_count; i++) {
		Game *g = s->games[i];
		if (g->p1 && g->p0) {
			printf("[%d] %s: %s (%d) X %s (%d)\n", i, g->name, g->p0->name, g->p0->fd, g->p1->name, g->p1->fd);
		} else {
			printf("[%d] %s: %s (%d) is waiting\n", i, g->name, g->p0->name, g->p0->fd);
		}
		game_print(g);
	}
	send_msg(p, OK, NULL);
	return 0;
}

int login_handler(Server *s, Player *p) { char *name = strtok(NULL, END_DELIM);
	if (!name) {
		printf("No args\n");
		send_msg(p, ERR, "1");
		return 1;
	}

	// validate nickname (only alphanumeric chars) + _; length
	int len = strlen(name);
	if (len > MAX_NAME_LEN) {
		send_msg(p, ERR, "1");
		printf("Invalid nickname\n");
		return 1;
	}
	for (int i = 0; i < len; i++) {
		if (!isalnum(name[i]) && name[i] != '_') {
			send_msg(p, ERR, "1");
			printf("Invalid nickname\n");
			return 1;
		}
	}

	// TODO: validate if its legal to login (state machine?)
	PState next = transition(p->state, EV_LOGIN);
	if (!next) {
		printf("Invalid state\n");
		send_msg(p, ERR, "1");
		return 1;
	}
	//p->state = next;
	
	// try reconnecting players if it was ever connected before
	for (int i = 0; i < s->player_count; i++) {
		if (!strcmp(s->players[i]->name, name)) {
			if (s->players[i]->state != ST_DISCONNECTED) {
				printf("Already connected, rejecting\n");
				send_msg(p, ERR, "5");
				return 1;
			} else {
				printf("Reconnecting player\n");
				s->players[i]->state = ST_LOGGED;
				s->players[i]->fd = p->fd;

				// remove player from list (now its players[i])
				remove_player(s, p);
				send_msg(s->players[i], OK, NULL);
				return 0;
			}
		}
	}
	
	// login new player
	strcpy(p->name, name);
	p->state = ST_LOGGED;
	printf("Loggin new player '%s'\n", name);
	p->state = next;
	send_msg(p, OK, NULL);
	return 0;
}

int create_handler(Server *s, Player *p) {
	char *name = strtok(NULL, END_DELIM);
	if (!name) {
		printf("Invalid args\n");
		send_msg(p, ERR, "1");
		return 1;
	}

	// validate name
	int len = strlen(name);
	if (len > MAX_NAME_LEN) {
		printf("Name too long\n");
		send_msg(p, ERR, "1");
		return 1;
	}
	for (int i = 0; i < len; i++) {
		if (!isalnum(name[i])) {
	  		printf("Invalid name\n");
			send_msg(p, ERR, "1");
	  		return 1;
		}
	}

	// check if game with same name already exists
	for (int i = 0; i < s->game_count; i++) {
		Game *g = s->games[i];
		if (!strcmp(g->name, name)) {
			printf("Game already exists\n");
			send_msg(p, ERR, "1");
			return 1;
		}
	}

	// check if valid state
	PState next = transition(p->state, EV_CREATE);
	if (!next) {
		printf("Invalid state\n");
		send_msg(p, ERR, "1");
		return 1;
	}
	p->state = next;

	// create new game
	if (s->game_count + 1 > s->games_size) {
		// add space for 10 games
		Game **new_games = realloc(s->games, sizeof(Game*) * (s->game_count + 10));
		if (!new_games) {
			printf("Malloc err\n");
			send_msg(p, ERR, "1");
			return -1;
		}
		s->games = new_games;
	}
	// malloc new game
	
	Game *g = game_create(3, 3, name, p);
	if (!g) {
		printf("Malloc err\n");
		send_msg(p, ERR, "1");
		return -1;
	}
	p->game = g;
	// add game to games
	s->games[s->game_count] = g;
	s->game_count++;
	printf("Game '%s' created.\n", g->name);
	send_msg(p, OK, NULL);

	return 0;
}

int join_handler(Server *s, Player *p) {
	char *name = strtok(NULL, END_DELIM);
	if (!name) {
		printf("No args\n");
		send_msg(p, ERR, "1");
		return 1;
	}

	// find game with given name
	Game *g = NULL;
	for (int i = 0; i < s->game_count; i++) {
		Game *tmp = s->games[i];
		if (!strcmp(tmp->name, name)) {
			g = tmp;
			break;
		}
	}
	if (!g) {
		printf("Game not found\n");
		send_msg(p, ERR, "1");
		return 1;
	}

	int rejoin = 0;

	// check if game is waiting for more players
	if (g->p1 != NULL && g->p0 != NULL) {
		if (g->p1 == p || g->p0 == p) {
			rejoin = 1;
		} else {
			printf("Game is full\n");
			send_msg(p, ERR, "1");
			return 1;
		}
	}

	// check if same player isn't joining
	if ((g->p0 == p && !g->p1) ||
		(g->p1 == p && !g->p0)) {
		printf("Can't join game you're already in\n");
		send_msg(p, ERR, "1");
		return 1;
	}

	// check player state
	PState next = transition(p->state, EV_JOIN);
	if (!next) {
		printf("Invalid state\n");
		send_msg(p, ERR, "1");
		return 1;
	}
	p->state = next;
	p->game = g;

	// update players of the game
	if (!rejoin) {
		if (g->p0) {
			g->p0->state = transition(g->p0->state, EV_JOIN);
			send_msg(g->p0, OP_JOIN, NULL);
			g->p1 = p;
		} else if (g->p1) {
			g->p1->state = transition(g->p1->state, EV_JOIN);
			send_msg(g->p1, OP_JOIN, NULL);
			g->p0 = p;
		}
	} else {
		// notify oponent about rejoin
		if (g->p0 == p) send_msg(g->p1, OP_JOIN, NULL);
		if (g->p1 == p) send_msg(g->p0, OP_JOIN, NULL);
	}

	if (rejoin) printf("Rejoin successfull\n");
	else printf("Join successfull\n");
	send_msg(p, OK, NULL);

	return 0;
}
