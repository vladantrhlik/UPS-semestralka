#include "structs.h"

#ifndef GAME
#define GAME

Game *game_create(int width, int height, char *name, Player *p0);

int game_set(Game *g, int player, int x, int y);

void game_free(Game *g);

void game_print(Game *g);

#endif
