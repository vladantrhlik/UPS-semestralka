#include "structs.h"

#ifndef GAME
#define GAME

int is_turn_valid(Game *g, int x, int y);

int is_game_finished(Game *g);

/**
* @param width int -- number of squares in X axis
* @param height int -- number of squares in Y axis
*/
Game *game_create(int width, int height, char *name, Player *p0);

int *game_set(Game *g, int player, int x, int y);

void game_free(Game *g);

PEvent game_potential_turn(Game *g, int player, int x, int y);

void game_print(Game *g);

#endif
