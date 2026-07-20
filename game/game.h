#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define MAP_HEIGHT 48
#define MAP_WIDTH 64
#define TILE_SIZE 10
//32x24 tiles, 20 px in size

void game_run(void);
void generate_new_apple();

extern int objMap[MAP_HEIGHT][MAP_WIDTH];//This will contain all the objects. will use it to keep track of collisions
extern int gamespeed; //this is a value between 1 and 3
extern int score;//score for the game

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
	left = 0, right, up, down
}direction;