#include "handlers.h"
#include "structs.h"
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
		printf("Invalid args\n");
		return 1;
	}
	// TODO: validate name
	// TODO: check if not already connected (state machine?)
	
	// try reconnecting players if it was ever connected before
	for (int i = 0; i < s->player_count; i++) {
		if (!strcmp(s->players[i]->name, name)) {
			printf("Reconnecting player\n");
			s->players[i]->state = LOGGED;
			s->players[i]->fd = p->fd;
			// TODO: remove players from list
			p->state = DISCONNECTED;
			return 0;
		}
	}
	
	strcpy(p->name, name);
	p->state = LOGGED;
	printf("Loggin new player '%s'\n", name);
	return 0;
}
