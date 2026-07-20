#include "game.h"
#include "../platform/platform.h"

/*
*   Snake: Platform agnostic game logic.
*   Last Update: Jul 19, 2026
*   Author: Scott Harner
*/

static int objMap[MAP_HEIGHT][MAP_WIDTH];//This will contain all the objects. will use it to keep track of collisions
static int gamespeed; //this is a value between 1 and 3
static int score;//score for the game

// finds a random location to place the next apple
void generate_new_apple()
{
	int randy, randx;

	do{
		randy = (int)(rand()%24);
		randx = (int)(rand()%32);
	}while (objMap[randy][randx] != nothing); //while we generate a spot that's taken, keep going;
	objMap[randy][randx] = apple;
}

// platform agnostic primary game logic
void game_run(void)
{
    
    //basic info from user about game speed.
    printf("\nSet your game speed (1,2,3): \n");
    scanf("%d",&gamespeed);
    score = 0;
    
    platform_initialize();

    snake_node* player = malloc(sizeof(snake_node));
    player->dir =  left; //init direction
    player->x = MAP_WIDTH/2;
    player->y = MAP_HEIGHT/2;
    player->next = NULL;

    int i, j; //this is so we don't get errors because the object map references nothing.
    for (i = 0; i < MAP_HEIGHT; i++)
    {
        for (j = 0; j < MAP_WIDTH; j++)
        {
            objMap[i][j] = nothing;
        }
    }
    // we need to seed our rand() and generate our first random object
    //srand(time(NULL));
    srand(time(NULL));
    generate_new_apple();
    
    // while (platform_is_running())
    // {
    //     platform_begin_frame();

    //     game_update();

    //     game_render();

    //     platform_end_frame();
    // }

    platform_shutdown();
}