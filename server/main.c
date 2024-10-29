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

Player *find_connected_player(Server *s, int fd) {
	for (int i = 0; i < s->player_count; i++) {
		if (s->players[i]->fd == fd && s->players[i]->state != DISCONNECTED) return s->players[i];
	}
	return NULL;
}

int handle_msg(Server *s, MsgType type, int fd, char *msg) {
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
			p->state = CONNECTED;
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
			printf("msg from %d: %s", fd, msg);
			char *cmd = strtok(msg, DELIMETERS);

			if (!strcmp(cmd, "LIST")) {
				list_handler(s, p);
			} else if (!strcmp(cmd, "LOGIN")) {
				login_handler(s, p);
			} 
		break;
		case DISCONNECT:
			p = find_connected_player(s, fd);
			if (!p) {
				printf("Player not found in connected\n");
				return -1;
			}
			p->state = DISCONNECTED;
			printf("Disconnected: %d\n", fd);
			// TODO: remove from list if no name
		break;
	}
	return 0;
}

int main (void){
	Server s;
	memset(&s, 0, sizeof(Server));

	server_create(&s, INADDR_ANY, 10000);
	for (;;) server_handle(&s, &handle_msg);

	return 0;
}