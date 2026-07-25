#ifndef GAME_H
#define GAME_H

#include <stdlib.h>
#include <time.h>

#define MAP_HEIGHT 48
#define MAP_WIDTH 64
#define TILE_SIZE 10
//32x24 tiles, 20 px in size

void game_initialize(void);
void game_update(void);
void game_shutdown(void);
void game_reset(void);

typedef enum
{
	OBJECT_NOTHING = 0, OBJECT_APPLE, OBJECT_SNAKE
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
	INPUT_TYPE_NOTHING = 0, INPUT_TYPE_LEFT, INPUT_TYPE_RIGHT, INPUT_TYPE_UP, INPUT_TYPE_DOWN, INPUT_TYPE_START, INPUT_TYPE_COUNT
}input_type;

typedef enum
{
	MODE_TITLE = 0, MODE_GAME, MODE_GAMEOVER
}mode;

typedef enum
{
	SPEED_SLOW = 1, SPEED_MEDIUM, SPEED_FAST
}speed;

#endif