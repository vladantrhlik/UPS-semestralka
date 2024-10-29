#include "handlers.h"
#include "structs.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int list_handler(Server *s, Player *p) {
	printf("\nPlayers:\n");
	for (int i = 0; i < s->player_count; i++) {
		printf("[%d]: %s (%s)\n", s->players[i]->fd, s->players[i]->name, s->players[i]->state == ST_DISCONNECTED ? "disconnected" : "connected");
	}
	printf("\nGames:\n");
	for (int i = 0; i < s->game_count; i++) {
		Game *g = s->games[i];
		if (g->p1 && g->p0) {
			printf("%s: %s (%d) X %s (%d)\n", g->name, g->p0->name, g->p0->fd, g->p1->name, g->p1->fd);
		} else {
			printf("%s: %s (%d) is waiting\n", g->name, g->p0->name, g->p0->fd);
		}
	}
	return 0;
}

int login_handler(Server *s, Player *p) { char *name = strtok(NULL, END_DELIM);
	if (!name) {
		printf("No args\n");
		send(p->fd, "ERR1\n", strlen("ERR1\n"), 0);
		return 1;
	}

	// validate nickname (only alphanumeric chars) + _; length
	int len = strlen(name);
	if (len > MAX_NAME_LEN) {
		send(p->fd, "ERR1\n", strlen("ERR1\n"), 0);
		printf("Invalid nickname\n");
		return 1;
	}
	for (int i = 0; i < len; i++) {
		if (!isalnum(name[i]) && name[i] != '_') {
			send(p->fd, "ERR1\n", strlen("ERR1\n"), 0);
			printf("Invalid nickname\n");
			return 1;
		}
	}

	// TODO: validate if its legal to login (state machine?)
	PState next = transition(p->state, EV_LOGIN);
	if (!next) {
		printf("Invalid state\n");
		send(p->fd, "ERR1\n", strlen("ERR1\n"), 0);
		return 1;
	}
	p->state = next;
	
	// try reconnecting players if it was ever connected before
	for (int i = 0; i < s->player_count; i++) {
		if (!strcmp(s->players[i]->name, name)) {
			if (s->players[i]->state != ST_DISCONNECTED) {
				send(p->fd, "ERR5\n", strlen("ERR1\n"), 0);
				printf("Already connected, rejecting\n");
				return 1;
			} else {
				printf("Reconnecting player\n");
				s->players[i]->state = ST_LOGGED;
				s->players[i]->fd = p->fd;

				// remove player from list (now its players[i])
				remove_player(s, p);

				send(p->fd, "OK\n", strlen("OK\n"), 0);
				return 0;
			}
		}
	}
	
	// login new player
	strcpy(p->name, name);
	p->state = ST_LOGGED;
	printf("Loggin new player '%s'\n", name);
	send(p->fd, "OK\n", strlen("OK\n"), 0);
	return 0;
}

int create_handler(Server *s, Player *p) {
	char *name = strtok(NULL, END_DELIM);
	if (!name) {
		printf("Invalid args\n");
		send(p->fd, "ERR1\n", strlen("ERR1\n"), 0);
		return 1;
	}

	printf("Creating game '%s'\n", name);

	// validate name
	int len = strlen(name);
	if (len > MAX_NAME_LEN) {
		printf("Name too long\n");
		return 1;
	}
	for (int i = 0; i < len; i++) {
		if (!isalnum(name[i])) {
	  		printf("Invalid name\n");
	  		return 1;
		}
	}

	// check if game with same name already exists
	for (int i = 0; i < s->game_count; i++) {
		Game *g = s->games[i];
		if (!strcmp(g->name, name)) {
			printf("Game already exists\n");
			return 1;
		}
	}

	// check if valid state
	PState next = transition(p->state, EV_CREATE);
	if (!next) {
		printf("Invalid state\n");
		send(p->fd, "ERR1\n", strlen("ERR1\n"), 0);
		return 1;
	}
	p->state = next;

	// create new game
	if (s->game_count + 1 > s->games_size) {
		// add space for 10 games
		Game **new_games = realloc(s->games, sizeof(Game*) * (s->game_count + 10));
		if (!new_games) {
			printf("Malloc err\n");
			return -1;
		}
		s->games = new_games;
	}
	// malloc new game
	Game *g = (Game*) malloc(sizeof(Game));
	if (!g) {
		printf("Malloc err\n");
		return -1;
	}
	printf("name: %s\n", name);
	strcpy(g->name, name);
	g->p0 = p;
	p->game = g;
	g->p1 = NULL;
	g->on_turn = 0;
	// add game to games
	s->games[s->game_count] = g;
	s->game_count++;
	printf("Game '%s' created.\n", g->name);
	send(p->fd, "OK\n", 3, 0);

	return 0;
}

int join_handler(Server *s, Player *p) {
	char *name = strtok(NULL, END_DELIM);
	if (!name) {
		printf("No args\n");
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
		return 1;
	}

	// check if game is waiting for more players
	if (g->p1 != NULL && g->p0 != NULL) {
		printf("Game is full\n");
		return 1;
	}

	// check player state
	PState next = transition(p->state, EV_JOIN);
	if (!next) {
		printf("Invalid state\n");
		send(p->fd, "ERR1\n", strlen("ERR1\n"), 0);
		return 1;
	}
	p->state = next;
	p->game = g;
	// update players of the game
	if (g->p0 != NULL) {
	if (g->p0) {
		g->p1 = p;
	}
	if (g->p1) {
		g->p1->state = transition(g->p1->state, EV_JOIN);
		g->p0 = p;
	}

	printf("Join successfull\n");

	return 0;
}
