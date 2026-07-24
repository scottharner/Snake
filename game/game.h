#ifndef GAME_H
#define GAME_H

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define MAP_HEIGHT 48
#define MAP_WIDTH 64
#define TILE_SIZE 10
//32x24 tiles, 20 px in size

void game_initialize(void);
void game_update(void);
void game_shutdown(void);

typedef enum
{
	nothing = 0, apple, snake
}object;

typedef struct snake_node
{
	int dir;
	int x;
	int y;
	struct snake_node *next;
}snake_node;

typedef enum
{
	none = 0, left, right, up, down
}direction;

#endif