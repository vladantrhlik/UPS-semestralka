#include "structs.h"

#ifndef GAME
#define GAME

/**
 * Decide whether turn on [x, y] is valid in given game
 */
int is_turn_valid(Game *g, int x, int y);

/**
 * Are all squares acquired?
 */
int is_game_finished(Game *g);

/**
* @param width int -- number of squares in X axis
* @param height int -- number of squares in Y axis
*/
Game *game_create(int width, int height, char *name, Player *p0);

/**
 * Make a turn in game
 */
int *game_set(Game *g, int player, int x, int y);

/**
 * Deallocate game
 */
void game_free(Game *g);

/**
 * Get info about if turn is good (creates square) or not
 */
PEvent game_potential_turn(Game *g, int player, int x, int y);

/**
 * Print game info
 */
void game_print(Game *g);

#endif
