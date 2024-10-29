#include <sys/select.h>
#include <netinet/in.h>

#ifndef STRUCTS
#define STRUCTS

#define BUFFER_SIZE 128
#define MAX_NAME_LEN 12

#define DELIM "|"
#define END_DELIM "\n"
#define DELIMETERS "|\n"

typedef enum {
	ST_CONNECTED,
	ST_DISCONNECTED,
	ST_LOGGED,
	ST_WAITING,
	ST_ON_TURN,
	ST_NO_TURN
} PState;

#define STATE_COUNT 6

typedef enum {
	EV_LOGIN,
	EV_CREATE,
	EV_JOIN,
	EV_TURN
} PEvent;

#define EVENT_COUNT 4

typedef enum {
	CONNECT,
	MSG,
	DISCONNECT
} MsgType;

typedef struct {
	int fd;
	char name[MAX_NAME_LEN + 1];
	int index;
	PState state;
} Player;

typedef struct {
	int server_socket;
	int len_addr;
	struct sockaddr_in my_addr, peer_addr;
	fd_set client_socks, tests;
	/* players */
	Player **players;
	int player_count;
	int players_size;
} Server;

#endif
