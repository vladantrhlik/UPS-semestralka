#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_turn_valid(Game *g, int x, int y) {
	// y axis
	if (y < 0 || y > g->height * 2 + 1 - 1) return 0;
	// x axis
	if (x < 0 || x > g->width + y % 2 - 1) return 0;
	// already set
	if (g->sticks[y][x] != -1) return 0;
	return 1;
}

int count_square(Game *g, int x, int y) {
	if (!g || x < 0 || x >= g->width || y < 0 || y>= g->height) return 0;

	int top_y = y * 2;
	int left_x = x;
	int sum = (g->sticks[top_y][left_x] > -1) + (g->sticks[top_y+1][left_x] > -1) +
			  (g->sticks[top_y+1][left_x+1] > -1) + (g->sticks[top_y+2][left_x] > -1);
	return sum;
}

Game *game_create(int width, int height, char *name, Player *p0) {
	if (width < 1 || height < 1) return NULL;

	Game *g = malloc(sizeof(Game));
	if (!g) return NULL;

	g->width = width;
	g->height = height;

	g->p0 = p0;
	g->p1 = NULL;
	g->on_turn = 0;

	strcpy(g->name, name);

	// create sticks data
	// cols
	int row_count = height * 2 + 1;
	g->sticks = malloc(sizeof(int*) * row_count);
	if (!g->sticks) {
		free(g);
		return NULL;
	}
	// rows
	for (int i = 0; i < row_count; i++) {
		int size = width + i % 2;
		g->sticks[i] = malloc(sizeof(int) * size);
		if (!g->sticks[i]) {
			i--;
			while (i >= 0) {
				free(g->sticks[i]);
				g--;
				return NULL;
			}
		}
		memset(g->sticks[i], -1, size * sizeof(int));
	}

	return g;
}

PEvent game_potential_turn(Game *g, int player, int x, int y) {
	if (!g || player < 0 || player > 1 || !is_turn_valid(g, x, y)) return EV_NULL;
	// TODO figure out based on value of square
	return EV_BAD_TURN;
}

int game_set(Game *g, int player, int x, int y) {
	// validate game and player
	if (!g || player < 0 || player > 1 || !is_turn_valid(g, x, y)) return 1;

	g->sticks[y][x] = player;
	return 0;
}

void game_free(Game *g) {
	if (!g) return;

	// free sticks
	for (int y = 0; y < g->height * 2 + 1; y++) {
		free(g->sticks[y]);
	}
	free(g->sticks);
	free(g);
}

void game_print(Game *g) {
	if (!g) return;

	for (int y = 0; y < g->height * 2 + 1; y++) {
		for (int x = 0; x < g->width + y % 2; x++) {
			int sq = count_square(g, x, y/2);
			int empty = g->sticks[y][x] == -1;

			if (y % 2 == 0) printf(" %s", empty ? " " : "-");
			// last cell without number
			else if (x < g->width) printf("%s%d", empty ? " " : "|", sq);
			else printf("%s", empty ? " " : "|");
		}
		printf("\n");
	}
}
