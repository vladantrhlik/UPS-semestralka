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

int is_game_finished(Game *g) {
	if (!g) return -1;
	int p0 = 0, p1 = 0;
	for (int y = 0; y < g->height; y++) {
		for (int x = 0; x < g->width; x++) {
			switch (g->squares[y][x]) {
				case -1:
					return -1;
					break;
				case 0:
					p0++;
					break;
				case 1:
					p1++;
					break;
			}
		}
	}

	return p0 > p1 ? 0 : 1;
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
	g->active_players = 1;

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
			}
			return NULL;
		}
		memset(g->sticks[i], -1, size * sizeof(int));
	}

	// squares
	g->squares = malloc(sizeof(int*) * g->height);
	if (!g) goto free_gs;
	for (int y = 0; y < height; y++) {
		g->squares[y] = malloc(sizeof(int) * g->width);
		if (!g->squares[y]) {
			y--;
			while (y >= 0) {
				free(g->squares[y]);
				y--;
			}
			goto free_gs;
		}
		memset(g->squares[y], -1, sizeof(int) * g->width);
	}

	return g;

free_gs:
	for (int i = 0; i < row_count; i++) {
		free(g->sticks[i]);
	}
	free(g->sticks);
	free(g);
	return NULL;
}

PEvent game_potential_turn(Game *g, int player, int x, int y) {
	if (!g || player < 0 || player > 1 || !is_turn_valid(g, x, y)) return EV_NULL;
	// get surrounding squares
	int good = 0;
	if (y % 2 == 0) {
		y = y / 2 - 1;
		if (count_square(g, x, y) == 3 || count_square(g, x, y + 1) == 3) good = 1;
	} else {
		y = (y - 1) / 2;
		if (count_square(g, x, y) == 3 || count_square(g, x - 1, y) == 3) good = 1;
	}
	return good ? EV_GOOD_TURN : EV_BAD_TURN;
}

int *game_set(Game *g, int player, int x, int y) {
	// array of acquired squares max(2), [0] = count
	static int squares[5];
	squares[0] = 0;
	// validate game and player
	if (!g || player < 0 || player > 1 || !is_turn_valid(g, x, y)) return NULL;

	g->sticks[y][x] = player;
	// check squares
	if (y % 2 == 0) {
		y = y / 2;
		// under
		if (count_square(g, x, y) == 4 && g->squares[y][x] == -1) {
			g->squares[y][x] = player;
			squares[0]++;
			squares[1] = x;
			squares[2] = y;
		}
		// up
		if (count_square(g, x, y-1) == 4 && g->squares[y-1][x] == -1) {
			g->squares[y-1][x] = player;
			squares[squares[0]*2 + 1] = x;
			squares[squares[0]*2 + 2] = y-1;
			squares[0]++;
		}
	} else {
		y = y / 2;
		// right
		if (count_square(g, x, y) == 4 && g->squares[y][x] == -1) {
			squares[0]++;
			squares[1] = x;
			squares[2] = y;
			g->squares[y][x] = player;
		}
		// left
		if (count_square(g, x-1, y) == 4 && g->squares[y][x-1] == -1) {
			g->squares[y][x-1] = player;
			squares[squares[0]*2 + 1] = x-1;
			squares[squares[0]*2 + 2] = y;
			squares[0]++;
		}
	}
	return squares;
}

void game_free(Game *g) {
	if (!g) return;

	// free sticks
	for (int y = 0; y < g->height * 2 + 1; y++) {
		free(g->sticks[y]);
	}
	// free squares
	for (int y = 0; y < g->height; y++) {
		free(g->squares[y]);
	}
	free(g->squares);
	free(g->sticks);
	free(g);
}

void game_print(Game *g) {
	if (!g) return;

	for (int y = 0; y < g->height * 2 + 1; y++) {
		for (int x = 0; x < g->width + y % 2; x++) {
			int empty = g->sticks[y][x] == -1;

			if (y % 2 == 0) printf(" %s", empty ? " " : "-");
			else if (x < g->width) {
				int sq = g->squares[y/2][x];
				if (sq == -1) {
					printf("%s×", empty ? " " : "|");
				} else {
					printf("%s%d", empty ? " " : "|", g->squares[y/2][x]);
				}
			}
			// last cell without number
			else printf("%s", empty ? " " : "|");
		}
		printf("\n");
	}
}
