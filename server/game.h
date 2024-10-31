#include "structs.h"

#ifndef GAME
#define GAME

int is_turn_valid(Game *g, int x, int y);

Game *game_create(int width, int height, char *name, Player *p0);

int game_set(Game *g, int player, int x, int y);

void game_free(Game *g);

PEvent game_potential_turn(Game *g, int player, int x, int y);

void game_print(Game *g);

#endif
