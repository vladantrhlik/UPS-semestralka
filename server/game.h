#include "structs.h"

#ifndef GAME
#define GAME

/**
 * Decide whether turn on [x, y] is valid in given game
 *
 * @param g game to make turn in
 * @param x X position of turn
 * @param y Y position of turn
 */
int is_turn_valid(Game *g, int x, int y);

/**
 * Decide whether game is finished -- are all squares acquired?
 *
 * @param g game to check
 */
int is_game_finished(Game *g);

/**
 * Create a new game (create by one player)
 *
 * @param width number of squares in X axis
 * @param height number of squares in Y axis
 * @param name name of the game
 * @param p0 player who started the game
*/
Game *game_create(int width, int height, char *name, Player *p0);

/**
 * Make a turn in game
 *
 * @param g game to make turn in
 * @param player player who made a turn
 * @param x X position of turn
 * @param y Y position of turn
 */
int *game_set(Game *g, int player, int x, int y);

/**
 * Deallocate game
 *
 * @param g game to free
 */
void game_free(Game *g);

/**
 * Get info about if turn is good (creates square) or not
 *
 * @param g game
 * @param player player who made the turn
 * @param x X position of turn
 * @param y Y position of turn
 */
PEvent game_potential_turn(Game *g, int player, int x, int y);

/**
 * Print game info
 *
 * @param g game
 */
void game_print(Game *g);

#endif
