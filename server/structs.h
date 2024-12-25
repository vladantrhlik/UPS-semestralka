#include <sys/select.h>
#include <netinet/in.h>

#ifndef STRUCTS
#define STRUCTS

#define BUFFER_SIZE 128
#define MIN_NAME_LEN 3
#define MAX_NAME_LEN 12

#define DELIM "|"
#define END_DELIM "\n"
#define DELIMETERS "|\n"

// player states
#define STATE_COUNT 6
typedef enum {
	ST_CONNECTED,
	ST_DISCONNECTED,
	ST_LOGGED,
	ST_WAITING,
	ST_ON_TURN,
	ST_NO_TURN,
} PState;

// player events
#define EVENT_COUNT 10
typedef enum {
	EV_LOGIN,
	EV_CREATE,
	EV_JOIN,
	EV_GOOD_TURN,
	EV_BAD_TURN,
	EV_ON_TURN,
	EV_NO_TURN,
	EV_GAME_END,
	EV_NULL,
	EV_LEAVE
} PEvent;

// server events
typedef enum {
	CONNECT,
	MSG,
	DISCONNECT
} SEvent;

typedef enum {
	OK = 1,
	ERR,
	OP_JOIN,
	OP_DISCONNECT,
	WIN,
	LOSE,
	TURN,
	ON_TURN,	// player on turn
	OP_TURN,	// oponent on turn
	ACQ,		// square acquired
	OP_ACQ,		// oponent acq square
	PONG,
} MsgType;


typedef struct Gaym Game;

typedef struct {
	int fd;
	char name[MAX_NAME_LEN + 1];
	int index;
	PState state;
	Game *game;
} Player;

typedef struct Gaym {
	Player *p0, *p1;
	int active_players;
	char name[MAX_NAME_LEN + 1];
	int on_turn;
	int finished;
	int width, height; // number of squares
	int **sticks;
	int **squares;
} Game;

typedef struct {
	int server_socket;
	int len_addr;
	struct sockaddr_in my_addr, peer_addr;
	fd_set client_socks, tests;
	/* players */
	Player **players;
	int player_count;
	int players_size;
	int logged_players;
	int max_players;
	/* games */
	Game **games;
	int game_count;
	int games_size;
	int max_games;
} Server;

#endif
