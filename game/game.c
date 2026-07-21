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

// display game over message
static void game_over()
{
	printf("\ngame over. score = %d\n",score);
	exit(0);
}

// finds a random location to place the next apple
static void generate_new_apple()
{
	int randy, randx;

	do{
		randy = (int)(rand()%24);
		randx = (int)(rand()%32);
	}while (objMap[randy][randx] != nothing); //while we generate a spot that's taken, keep going;
	objMap[randy][randx] = apple;
}

/*
*  This function recursively moves the snake. It goes all the way to the last node of the snake, changes its coordinates,
*  and then changes its coordinates, all the way up.
*/
static snake_node* move_body(snake_node* player, int tempx, int tempy)
{
	if (player->next == NULL) //we've reached the end of the snake's body
	{
		objMap[player->y][player->x] = nothing; //reset the object map at this position
	}
	else //we need to keep traversing
	{
		player->next = move_body(player->next,player->x,player->y);
	}
	player->x = tempx;
	player->y = tempy;
	return player;

}

static void move(snake_node* player)
{
    //this contains the array of flags which tell which button has been pressed. It must be cleared before every input.
    // todo - move to allegro platform
    direction inputDirection = platform_get_input_direction();
    if (inputDirection == left)	 player->dir = left;
    if (inputDirection == right) player->dir = right;
    if (inputDirection == down) player->dir = down;
    if (inputDirection == up) player->dir = up;

    int tempx = player->x, tempy = player->y;

    if (player->dir == left)
    {

        if (player->x > 0)
        {
            tempx = player->x - 1;
        }
        else
        { //allow for wrap around
            game_over();
        }

    } 
    else if (player->dir == right)
    {

        if (player->x < MAP_WIDTH - 1)
        {
                tempx = player->x + 1;
        }
        else
        {
            game_over();
        }
    } 
    else if (player->dir == up)
    {
        if (player->y > 0)
        {
            tempy = player->y - 1;
        }
        else
        {
            game_over();
        }

    } 
    else if (player->dir == down)
    {

        if (player->y < MAP_HEIGHT - 1)
    	{
        	tempy = player->y + 1;
        }
        else{
            game_over();
        }
    } 
    else 
    {

        exit(0);
    }

    if (objMap[tempy][tempx] == apple) //the snake has run into an apple and another node is created
    {
	    score++;
        snake_node* temp = player;
        while (temp->next != NULL) //the snake is essentially a linked list and we're traversing it
        {
            temp = temp->next;
        }
        int newNodex = temp->x; //when we reach the final node, we store its location with two variables
        int newNodey = temp->y;
        player = move_body(player,tempx,tempy);
        snake_node* newNode = malloc(sizeof(snake_node)); //after we move the whole body we make a new node line( screen, 130, 130, 150, 150, makecol( 255, 0, 0));
        newNode->x = newNodex;
        newNode->y = newNodey; //we set this new node's location variables
        newNode->dir = temp->dir;
        newNode->next = NULL;
        temp->next = newNode; //we set the temp node (the previous tail)'s next node to our new tail!
        generate_new_apple();
    }
    else if (objMap[tempy][tempx] == snake)
    {
    	game_over();
    }
    else
    {
 	    player = move_body(player,tempx,tempy);
    }

    objMap[tempy][tempx] = snake; //update the object map to the new snake head position
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
    
    while (platform_is_running())
    {
        move(player);
        // draw();

        // //update ticks
        // ticks++;

        // //slow the game down
        // resting=0;

        platform_adjust_speed(gamespeed);
    }

    platform_shutdown();
}