#include "structs.h"
#include <netinet/in.h>

#ifndef SERVER
#define SERVER

/**
 * Create a new server
 *
 * @param s struct to save server to
 * @param config_file configuration file with max number 
 * 					  of players and games, ip, port
 */
int server_create(Server *s, char *config_file);

/**
 * Basic function for handling server, connections and disconnections from clients
 */
int server_handle(Server *s);

int server_ping(Server *s);

/**
 * Handle message from client
 *
 * @param type type of message (basic message, new client, disconnect)
 */
int handle_msg(Server *s, SEvent type, int fd, char *msg);

/**
 * Remove player from server
 *
 * @param p player to remove
 */
int remove_player(Server *s, Player *p);

/**
 * Remve game from server
 *
 * @param g game to remove
 */
int remove_game(Server *s, Game *g);

#endif
