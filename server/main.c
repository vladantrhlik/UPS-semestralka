#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#include "server.h"
#include "structs.h"
#include "handlers.h"
#include "utils.h"

char *handled_msgs[] = {"LIST", "LOGIN", "CREATE", "JOIN", "TURN", "LEAVE", "LOAD", "SYNC"};
int (*handlers[])(Server *s, Player *p) = {list_handler, login_handler, create_handler, join_handler, turn_handler, leave_handler, load_handler, sync_handler};
int handler_count = sizeof(handled_msgs) / sizeof(char*);

int handle_msg(Server *s, SEvent type, int fd, char *msg) {
	Player *p;

	switch (type) {
		case CONNECT:
			printf("New client: %d\n", fd);
			// add new player to server
			if (s->player_count + 1 > s->players_size) {
				// add space for 10 players
				Player **new_players = realloc(s->players, sizeof(Player*) * (s->player_count + 10));
				if (!new_players) {
					printf("Malloc err\n");
					return -1;
				}
				s->players = new_players;
			}
			// malloc new player
			p = (Player*) malloc(sizeof(Player));
			p->fd = fd;
			p->state = ST_CONNECTED;
			p->index = s->player_count;
			p->invalid_msg_count = 0;
			if (!p) {
				printf("Malloc err\n");
				return -1;
			}
			// add player to players
			s->players[s->player_count] = p;
			s->player_count++;
		break;
		case MSG:
			p = find_connected_player(s, fd);
			if (!p) {
				printf("Player not found in connected\n");
				return -1;
			}
			printf("\nmsg from %d: %s", fd, msg);
			char *cmd = strtok(msg, DELIMETERS);
			if (!cmd) break;

			for (int i = 0; i < handler_count; i++) {
				if (!strcmp(cmd, handled_msgs[i])) {
					if (handlers[i](s, p)) {
						printf("Invalid message.\n");
						if (invalid_msg(s, p)) handle_msg(s, DISCONNECT, fd, NULL);
					}
					break;
				}
			}

			if (!strcmp(cmd, "PING")) {
				send_msg(p, PONG, NULL);
				break;
			}

		break;
		case DISCONNECT:
			p = find_connected_player(s, fd);
			if (!p) {
				printf("Player not found in connected\n");
				return -1;
			}

			if (strlen(p->name) == 0) {
				// remove from players if didn't even logged
				remove_player(s, p);
				break;
			} else {
				// mark as disconnected
				p->state = ST_DISCONNECTED;
				printf("Disconnected: %d\n", fd);
				s->logged_players--;
			}

			// remove game if player was waiting in the game
			Game *g = p->game;
			if (g) {
				// player was waiting in game alone -> delete game
				if ((p == g->p0 && (!g->p1 || (g->p1 && g->p1->state == ST_DISCONNECTED))) ||
					(p == g->p1 && (!g->p0 || (g->p0 && g->p0->state == ST_DISCONNECTED)))) {
					//(p == g->p1 && g->p0 && g->p0->state == ST_DISCONNECTED)) {
					printf("Removing game '%s'\n", g->name);
					if (g->p0) g->p0->game = NULL;
					if (g->p1) g->p1->game = NULL;
					remove_game(s, g);
				} else {
					// notify about op left
					if (g->p0 == p) send_msg(g->p1, OP_DISCONNECT, NULL);
					if (g->p1 == p) send_msg(g->p0, OP_DISCONNECT, NULL);
				}
			}
		break;
	}
	return 0;
}

int main(int argc, char *argv[]) {
	Server s;
	memset(&s, 0, sizeof(Server));

	server_create(&s, "config.txt");
	for (;;) server_handle(&s, &handle_msg);

	return 0;
}
