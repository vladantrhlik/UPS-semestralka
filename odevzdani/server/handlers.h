#include "structs.h"

#ifndef HANDLERS
#define HANDLERS

/**
 * Handles list message (for debug)
 */
int list_handler(Server *s, Player *p);

/**
 * Handles login message from client
 * Expects LOGIN|<nick>\n
 */
int login_handler(Server *s, Player *p);

/**
 * Handles create message from client
 * Expects CREATE|<game_name>\n
 */
int create_handler(Server *s, Player *p);

/**
 * Handles join message from client
 * Expects JOIN|<game-index>\n
 */
int join_handler(Server *s, Player *p);

/**
 * Handles turn message from client
 * Expects TURN|x|y\n
 */
int turn_handler(Server *s, Player *p);

/**
 * Handles leave message from client
 */
int leave_handler(Server *s, Player *p);

/**
 * Handles load message from client
 */
int load_handler(Server *s, Player *p);

/**
 * Handles sync message from client
 */
int sync_handler(Server *s, Player *p);

/**
 * Handles reconnect message from client
 */
int reconnect_handler(Server *s, Player *p);

#endif
