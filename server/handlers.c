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

int login_handler(Server *s, Player *p) { 
	char *name = strtok(NULL, END_DELIM);
	if (!name) {
		printf("No args\n");
		send_msg(p, ERR, "1");
		return 1;
	}

	// validate nickname
	if (!is_name_valid(name)) {
		send_msg(p, ERR, "1");
		printf("Invalid nickname\n");
		return 0;
	}

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
				return 0;
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

	if (s->logged_players + 1 > s->max_players) {
		printf("Too many players (%d / %d)\n", s->player_count, s->max_players);
		send_msg(p, ERR, "6");
		return 0;
	}
	
	// login new player
	strcpy(p->name, name);
	p->state = ST_LOGGED;
	printf("Loggin new player '%s'\n", name);
	p->state = next;
	send_msg(p, OK, NULL);
	s->logged_players++;
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
	if (!is_name_valid(name)) {
		printf("Invalid name\n");
		send_msg(p, ERR, "1");
		return 0;
	}

	// check if game with same name already exists
	for (int i = 0; i < s->game_count; i++) {
		Game *g = s->games[i];
		if (!strcmp(g->name, name)) {
			printf("Game already exists\n");
			send_msg(p, ERR, "1");
			return 0;
		}
	}

	// check if valid state
	PState next = transition(p->state, EV_CREATE);
	if (!next) {
		printf("Invalid state\n");
		send_msg(p, ERR, "1");
		return 1;
	}

	// check max game limit
	if (s->game_count + 1 > s->max_games) {
		printf("Max game limit exceeded\n");
		send_msg(p, ERR, "6");
		return 0;
	}

	p->state = next;

	// create new game
	if (s->game_count + 1 > s->games_size) {
		// add space for 10 games
		Game **new_games = realloc(s->games, sizeof(Game*) * (s->game_count + 10));
		if (!new_games) {
			printf("Malloc err\n");
			send_msg(p, ERR, "1");
			return 0;
		}
		s->games = new_games;
	}
	// malloc new game
	
	//Game *g = game_create(3, 3, name, p);
	Game *g = game_create(1, 1, name, p);
	if (!g) {
		printf("Malloc err\n");
		send_msg(p, ERR, "1");
		return 0;
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
	static char name_buff[16], op_name_buff[16];
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
	if (!g || g->finished) {
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
			return 0;
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
	p->game = g;
	send_msg(p, OK, NULL);

	// update players of the game
	sprintf(name_buff, "|%s", p->name);
	sprintf(op_name_buff, "|%s", (g->p0 && g->p0 != p) ? g->p0->name : g->p1->name);

	if (!rejoin) {
		// update state
		p->state = next;
		printf("Join successfull\n");
		if (g->p0) {
			g->p0->state = transition(g->p0->state, EV_JOIN);
			send_msg(g->p0, OP_JOIN, name_buff);
			send_msg(g->p0, ON_TURN, NULL);
			g->p1 = p;
			send_msg(g->p1, OP_JOIN, op_name_buff);
			send_msg(g->p1, OP_TURN, NULL);
		} else if (g->p1) {
			g->p1->state = transition(g->p1->state, EV_JOIN);
			send_msg(g->p1, OP_JOIN, name_buff);
			send_msg(g->p1, ON_TURN, NULL);
			g->p0 = p;
			send_msg(g->p1, OP_JOIN, op_name_buff);
			send_msg(g->p0, OP_TURN, NULL);
		}
	} else {
		// update state
		Player *op = g->p0 == p ? g->p1 : g->p0;
		p->state = op->state == ST_ON_TURN ? ST_NO_TURN : ST_ON_TURN;
		// notify oponent about rejoin
		printf("Rejoin successfull\n");
		if (g->p0 == p) {
			send_msg(g->p1, OP_JOIN, name_buff);
			send_msg(g->p0, OP_JOIN, op_name_buff);
			send_msg(g->p0, p->state == ST_ON_TURN ? ON_TURN : OP_TURN, NULL);
		}
		if (g->p1 == p) {
			send_msg(g->p0, OP_JOIN, name_buff);
			send_msg(g->p1, OP_JOIN, op_name_buff);
			send_msg(g->p1, p->state == ST_ON_TURN ? ON_TURN : OP_TURN, NULL);
		}
	}

	return 0;
}

int turn_handler(Server *s, Player *p) {
	static char buff[32];
	// load args
	char *x_str = strtok(NULL, DELIM);
	char *y_str = strtok(NULL, END_DELIM);
	if (!x_str || !y_str) {
		printf("Invalid args\n");
		send_msg(p, ERR, "1");
		return 1;
	}

	// check if integers
	int nan = 0;
	for (int i = 0; i < strlen(x_str); i++) if (!isdigit(x_str[i])) nan = 1;
	for (int i = 0; i < strlen(y_str); i++) if (!isdigit(y_str[i])) nan = 1;
	if (nan) {
		printf("Not a numbers\n");
		send_msg(p, ERR, "1");
		return 1;
	}

	int x = atoi(x_str);
	int y = atoi(y_str);

	// check player state
	PState valid = transition(p->state, EV_GOOD_TURN);
	if (!valid) {
		printf("Invalid state\n");
		send_msg(p, ERR, "1");
		return 1;
	}

	Game *g = p->game;
	Player *op = g->p0 == p ? g->p1 : g->p0;
	int player = g->p0 == p ? 0 : 1;

	printf("%s turn: %d, %d\n", p->name, x, y);

	// check move
	if (!is_turn_valid(g, x, y)) {
		printf("Invalid move\n");
		send_msg(p, ERR, "1");
		return 0;
	}

	// figure out if good or bad turn -- closed square?
	PEvent ev = game_potential_turn(g, player, x, y);
	if (ev == EV_NULL) {
		printf("Something fucked up, null potential state\n");
		return 1;
	}
	PState next = transition(p->state, ev);
	if (!next) {
		printf("Something fucked up, null state\n");
		return 1;
	}

	p->state = next;
	int *squares = game_set(g, player, x, y);

	// send info to oponent
	sprintf(buff, "|%d|%d", x, y);
	send_msg(op, TURN, buff);

	send_msg(p, OK, NULL);

	if (ev == EV_BAD_TURN) {
		// switch turns if bad turn
		send_msg(p, OP_TURN, NULL);
		if (transition(op->state, EV_ON_TURN)) {
			op->state = ST_ON_TURN;
			send_msg(op, ON_TURN, NULL);
		}
	} else {
		send_msg(p, ON_TURN, NULL);
		//send_msg(op, OP_TURN, NULL);
		if (transition(op->state, EV_NO_TURN)) send_msg(op, OP_TURN, NULL);
		// send info about acquired squares
		switch (squares[0]) {
			case 0:
				printf("Square acq, but not found in squares[]\n");
				return 1;
			case 1:
				sprintf(buff, "|%d|%d", squares[1], squares[2]);
				break;
			case 2:
				sprintf(buff, "|%d|%d|%d|%d", squares[1], squares[2], squares[3], squares[4]);
				break;
		}
		send_msg(p, ACQ, buff);
		send_msg(op, OP_ACQ, buff);

		// check if win
		int winner = is_game_finished(g);
		if (winner > -1) {
			send_msg(g->p0, winner == 0 ? WIN : LOSE, 0);
			send_msg(g->p1, winner == 1 ? WIN : LOSE, 0);
			// reset states
			PState next = transition(p->state, EV_GAME_END);
			if (!next) {
				printf("Something fucked up, null state\n");
				return 1;
			}
			p->state = next;
			next = transition(op->state, EV_GAME_END);
			if (!next) {
				printf("Something fucked up, null state\n");
				return 1;
			}
			op->state = next;
			g->finished = 1;
		}
	}
	
	return 0;
}

int leave_handler(Server *s, Player *p) {
	// validate state
	PState next = transition(p->state, EV_LEAVE);
	Game *g = p->game;
	if (!next) {
		printf("Invalid state\n");
		send_msg(p, ERR, NULL);
		return 1;
	} 

	p->state = next;

	if (g->finished) {
		printf("trying to remove game\n");
		if (g->p0 == p) g->p0 = NULL;
		else g->p1 = NULL;

		p->game = NULL;

		if (!g->p0 && !g->p1) {
			remove_game(s, g);
		}
		send_msg(p, OK, NULL);
	} else {
		// remove game if not oponent
		Player *op = p->game->p0 == p ? p->game->p1 : p->game->p0;
		if (op) {
			// inform oponent about leaving
			send_msg(op, OP_DISCONNECT, NULL);
		} else {
			remove_game(s, p->game);
		}
		send_msg(p, OK, NULL);
	}

	return 0;
}

int load_handler(Server *s, Player *p) {
	// calculate max message length
	int max_len = strlen("OK") + s->game_count * (MAX_NAME_LEN + 1) + 2;
	char *buff = malloc(sizeof(char) * max_len);

	memset(buff, 0, sizeof(char) * max_len);
	for (int i = 0; i < s->game_count; i++) {
		Game *g = s->games[i];
		if (g->finished) continue;
		strcat(buff, DELIM);
		strcat(buff, g->name);
	}

	send_msg(p, OK, buff);
	free(buff);

	return 0;
}

int sync_handler(Server *s, Player *p) {
	if (!p->game) {
		send_msg(p, ERR, "4");
		return 0;
	}
	Game *g = p->game;
	// calculate data length
	int square_len = (g->width) * (g->height);
	int delim_len = 6;
	int size_len = 4;
	int sticks_len = (g->width + 1) * 2 * (g->height + 1); // approx
	int len = square_len + delim_len + size_len + sticks_len;

	char *msgbuff = malloc(sizeof(char) * len);
	if (!msgbuff) {
		send_msg(p, ERR, "1");
		return 0;
	}
	memset(msgbuff, 0, sizeof(char) * len);
	char *buff = malloc(sizeof(char) * 32);
	if (!buff) {
		send_msg(p, ERR, "1");
		free(msgbuff);
		return 0;
	}
	memset(buff, 0, sizeof(char) * 32);

	// dimensions
	sprintf(buff, "|%d|%d|", g->width, g->height);
	strcat(msgbuff, buff);
	// stick data
	for (int y = 0; y < (g->height) * 2 + 1; y++) {
		for (int x = 0; x < (g->width) + y % 2; x++) {
			int val = g->sticks[y][x] + 1;
			sprintf(buff, "%d", val);
			strcat(msgbuff, buff);
		}
	}
	strcat(msgbuff, "|");
	// square data
	for (int y = 0; y < g->height; y++) {
		for (int x = 0; x < g->width; x++) {
			int val = g->squares[y][x] + 1;
			sprintf(buff, "%d", val);
			strcat(msgbuff, buff);
		}
	}

	send_msg(p, OK, msgbuff);
	return 0;
}
