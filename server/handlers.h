#include "structs.h"

#ifndef HANDLERS
#define HANDLERS

int list_handler(Server *s, Player *p);

/**
 * Handles LOGIN|...
 *
 * returns: OK, ERR1 (invalid nickname), ERR5 (already in game)
 */
int login_handler(Server *s, Player *p);

#endif
