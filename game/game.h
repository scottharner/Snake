#ifndef GAME_H
#define GAME_H

#include "../common/types.h"

#define MAX_MAP_HEIGHT 48
#define MAX_MAP_WIDTH 64
#define MAX_TILE_SIZE 10

#define TIMEOUT_SCREEN_MAX_CYCLES 300
#define CREDITS_SCREEN_MAX_CYCLES 600
#define SLOw_SPEED_MAX_CYCLES 3
#define MEDIUM_SPEED_MAX_CYCLES 2
#define FAST_SPEED_MAX_CYCLES 1

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

typedef struct game_config
{
	int map_height;
	int map_width;
	int tile_size;
}game_config;


typedef enum
{
	INPUT_TYPE_NOTHING = 0, INPUT_TYPE_LEFT, INPUT_TYPE_RIGHT, INPUT_TYPE_UP, INPUT_TYPE_DOWN, INPUT_TYPE_START, INPUT_TYPE_COUNT
}input_type;

typedef enum
{
	MODE_NONE = 0, MODE_TITLE, MODE_GAME, MODE_GAME_OVER, MODE_WIN, MODE_CREDITS
}mode;

typedef enum
{
	SPEED_SLOW = 1, SPEED_MEDIUM, SPEED_FAST
}speed;

typedef enum
{
	OPTION_START = 0, OPTION_SPEED, OPTION_CREDITS
}option;

typedef enum
{
	LOSS_TYPE_WALL = 0, LOSS_TYPE_SELF
}loss_type;

typedef enum
{
	SOUND_PICKUP = 0, SOUND_LOSE
}sound_type;

void game_initialize(int, int, int);
void game_update();
void game_shutdown();
void game_reset();
bool game_input_pressed(input_type);
void game_save_previous_inputstates();
void game_reset_input_states();
void game_get_speed_string(char *);

#endif