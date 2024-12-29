#include <sys/select.h>
#include <netinet/in.h>

#ifndef STRUCTS
#define STRUCTS

#define BUFFER_SIZE 128
#define MIN_NAME_LEN 3
#define MAX_NAME_LEN 12
#define MAX_INVALID_MSG 10

#define DELIM "|"
#define END_DELIM "\n"
#define DELIMETERS "|\n"

/**
 * Player states
 */
#define STATE_COUNT 6
typedef enum {
	ST_CONNECTED,
	ST_DISCONNECTED,
	ST_LOGGED,
	ST_WAITING,
	ST_ON_TURN,
	ST_NO_TURN,
} PState;

/**
 * Player events
 */
#define EVENT_COUNT 10
typedef enum {
	EV_LOGIN,
	EV_CREATE,
	EV_JOIN,
	EV_GOOD_TURN, // turn leads to acquired square
	EV_BAD_TURN,  // turn doesn't lead to acquired square
	EV_ON_TURN,
	EV_NO_TURN,
	EV_GAME_END,
	EV_NULL,	  // invalid
	EV_LEAVE
} PEvent;

/**
 * Server events / message types for handle_msg()
 */
typedef enum {
	CONNECT,
	MSG,
	DISCONNECT
} SEvent;

/**
 * Message types
 */
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
	PONG,		// response to ping
} MsgType;


typedef struct Gaym Game;

/**
 * Player structure
 */
typedef struct {
	int fd; 	// file descriptor
	char name[MAX_NAME_LEN + 1];
	int index;
	int invalid_msg_count;
	PState state;
	Game *game;
} Player;

/**
 * Game structure
 */
typedef struct Gaym {
	Player *p0, *p1;   // players who are playing the game
	int active_players;
	char name[MAX_NAME_LEN + 1];
	int on_turn;	   // who is on turn (0 / 1)
	int finished;	   // game finished?
	int width, height; // number of squares
	int **sticks;	   // stick / connections data
	int **squares;	   // square data
} Game;

/**
 * Server structure
 */
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
