#include "server.h"
#include "structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utils.h"
#include "handlers.h"
#include "game.h"

#define BUFF_LEN 256

int server_create(Server *s, char *config_file) {
	// load config file
	int port = 10000, max_players = -1, max_games = -1;
	char *ip;
	char buffer[BUFF_LEN];
	FILE *conf = fopen(config_file, "r");
	while (fgets(buffer, BUFF_LEN, conf) != NULL) {
		char *key = strtok(buffer, " ");
		char *value = strtok(NULL, "\n");
		if (!strcmp(key, "port")) {
			port = atoi(value);
		} else if (!strcmp(key, "ip")) {
			ip = value;
		} else if (!strcmp(key, "maxPlayers")) {
			max_players = atoi(value);
		} else if (!strcmp(key, "maxGames")) {
			max_games = atoi(value);
		}
	}

	printf("ip: %s\nport: %d\nmaxPlayers: %d\nmaxGames: %d\n", ip, port, max_players, max_games);

	s->server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (s->server_socket < 0) {
		printf("Socket create - ERR\n");
		return -1;
	}

	memset(&s->my_addr, 0, sizeof(struct sockaddr_in));
	s->my_addr.sin_family = AF_INET;
	s->my_addr.sin_port = htons(port);
	s->my_addr.sin_addr.s_addr = inet_addr(ip);
	s->max_games = max_games;
	s->max_players = max_players;
	s->logged_players = 0;

	int return_value = bind(s->server_socket, (struct sockaddr *) &s->my_addr, \
		sizeof(struct sockaddr_in));

	if (return_value == 0) 
		printf("Bind - OK\n");
	else {
		printf("Bind - ERR\n");
		return -1;
	}

	return_value = listen(s->server_socket, 5);
	if (return_value == 0){
		printf("Listen - OK\n");
	} else {
		printf("Listen - ER\n");
	}

	// setup fd set
	FD_ZERO( &s->client_socks );
	FD_SET( s->server_socket, &s->client_socks );

	return 0;
}

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

int server_handle(Server *s) {
	static char buffer[BUFFER_SIZE];
	int msg_len;

	s->tests = s->client_socks;
	int return_value = select( FD_SETSIZE, &s->tests, ( fd_set *)0, ( fd_set *)0, ( struct timeval *)0 );
	if (return_value < 0) {
		printf("Select - ERR\n");
		return -1;
	}

	for(int fd = 3; fd < FD_SETSIZE; fd++ ){
		if( FD_ISSET( fd, &s->tests ) ){
			// server socket -> new connection
			if (fd == s->server_socket){
				int client_socket = accept(s->server_socket, (struct sockaddr *) &s->peer_addr, (socklen_t*) &s->len_addr);
				if (client_socket >= 0) {
					FD_SET( client_socket, &s->client_socks );
					handle_msg(s, CONNECT, client_socket, NULL);
				} else {
					printf("Accept - ERR\n");
				}
			}
			// client socket -> receive data
			else {
				ioctl( fd, FIONREAD, &msg_len );
				if (msg_len > BUFFER_SIZE) {
					printf("Message is too long.\n");
					Player *p = find_connected_player(s, fd);
					if (invalid_msg(s, p)) handle_msg(s, DISCONNECT, fd, NULL);
					return -1;
				}
				// read message
				if (msg_len > 0){
					memset(buffer, '\0', 8);
					recv(fd, &buffer, msg_len, 0);
					handle_msg(s, MSG, fd, buffer);
				}
				else {
					close(fd);
					FD_CLR( fd, &s->client_socks );
					handle_msg(s, DISCONNECT, fd, buffer);
				}
			}
		}
	}

	return 0;
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

