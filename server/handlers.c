#include "handlers.h"
#include "structs.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int list_handler(Server *s, Player *p) {
	printf("listing all players:\n");
	for (int i = 0; i < s->player_count; i++) {
		printf("[%d]: %s (%s)\n", s->players[i]->fd, s->players[i]->name, s->players[i]->state == DISCONNECTED ? "disconnected" : "connected");
	}
	return 0;
}

int login_handler(Server *s, Player *p) {
	char *name = strtok(NULL, END_DELIM);
	if (!name) {
		printf("No args\n");
		send(p->fd, "ERR1\n", strlen("ERR1\n"), 0);
		return 1;
	}

	// TODO: validate if its legal to login (state machine?)

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
	
	// try reconnecting players if it was ever connected before
	for (int i = 0; i < s->player_count; i++) {
		if (!strcmp(s->players[i]->name, name)) {
			if (s->players[i]->state != DISCONNECTED) {
				send(p->fd, "ERR5\n", strlen("ERR1\n"), 0);
				printf("Already connected, rejecting\n");
				return 1;
			} else {
				printf("Reconnecting player\n");
				s->players[i]->state = LOGGED;
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
	p->state = LOGGED;
	printf("Loggin new player '%s'\n", name);
	send(p->fd, "OK\n", strlen("OK\n"), 0);
	return 0;
}
