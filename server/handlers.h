#include "structs.h"

#ifndef HANDLERS
#define HANDLERS

int list_handler(Server *s, Player *p);

/**
 * Handles LOGIN|<nick>\n
 *
 * returns: OK, ERR1 (invalid nickname), ERR5 (already in game)
 */
int login_handler(Server *s, Player *p);

/**
 * Handles CREATE|<game_name>\n
 *
 */
int create_handler(Server *s, Player *p);

/**
 * Handles JOIN|<game-index>\n
 */
int join_handler(Server *s, Player *p);

/**
 * Handles TURN|x|y\n
 */
int turn_handler(Server *s, Player *p);

int leave_handler(Server *s, Player *p);

#endif
