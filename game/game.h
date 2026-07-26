#ifndef GAME_H
#define GAME_H

#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAP_HEIGHT 24
#define MAP_WIDTH 32
#define TILE_SIZE 8

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
	MODE_NONE = 0, MODE_TITLE, MODE_GAME, MODE_GAMEOVER
}mode;

typedef enum
{
	SPEED_SLOW = 1, SPEED_MEDIUM, SPEED_FAST
}speed;

void game_initialize();
void game_update();
void game_shutdown();
void game_reset();
bool game_input_pressed(input_type);
void game_save_previous_inputstates();
void game_reset_input_states();

#endif