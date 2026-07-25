#include "game.h"
#include "../platform/platform.h"

/*
*   Snake: Platform agnostic game logic.
*   Last Update: Jul 19, 2026
*   Author: Scott Harner
*/

static int objMap[MAP_HEIGHT][MAP_WIDTH];//This will contain all the objects. will use it to keep track of collisions
static speed gameSpeed = SPEED_SLOW;
static int score;//score for the game
static struct snake_node* player;
static mode gameMode;

// finds a random location to place the next apple
static void generate_new_apple()
{
	int randy, randx;

    do{
		randy = (int)(platform_get_random(MAP_HEIGHT/2));
		randx = (int)(platform_get_random(MAP_WIDTH/2));
	}while (objMap[randy][randx] != OBJECT_NOTHING); //while we generate a spot that's taken, keep going;
	objMap[randy][randx] = OBJECT_APPLE;
}

/*
*  This function recursively moves the snake. It goes all the way to the last node of the snake, changes its coordinates,
*  and then changes its coordinates, all the way up.
*/
static snake_node* move_body(snake_node* currentNode, int tempx, int tempy)
{
	if (currentNode->next == NULL) //we've reached the end of the snake's body
	{
		objMap[currentNode->y][currentNode->x] = OBJECT_NOTHING; //reset the object map at this position
	}
	else //we need to keep traversing
	{
		currentNode->next = move_body(currentNode->next,currentNode->x,currentNode->y);
	}
	currentNode->x = tempx;
	currentNode->y = tempy;
    return currentNode;

}

static void title_screen_read_input()
{
    //this contains the array of flags which tell which button has been pressed. It must be cleared before every input.
    input_type inputType = platform_get_input_type(MODE_TITLE);
    if (inputType == INPUT_TYPE_START)
    {
        gameMode = MODE_GAME;
    }
    else if (inputType == INPUT_TYPE_DOWN) 
    {
        switch (gameSpeed)
        {
            case SPEED_SLOW:
                gameSpeed = SPEED_MEDIUM;
                break;

            case SPEED_MEDIUM:
                gameSpeed = SPEED_FAST;
                break;

            default:
                gameSpeed = SPEED_SLOW;
                break;
        }
    }
    else if (inputType == INPUT_TYPE_UP) 
    {
        switch (gameSpeed)
        {
            case SPEED_SLOW:
                gameSpeed = SPEED_FAST;
                break;

            case SPEED_MEDIUM:
                gameSpeed = SPEED_SLOW;
                break;

            default:
                gameSpeed = SPEED_MEDIUM;
                break;
        }
    }
}

static void move()
{
    //this contains the array of flags which tell which button has been pressed. It must be cleared before every input.
    input_type inputType = platform_get_input_type(MODE_GAME);
    if (inputType == INPUT_TYPE_LEFT)	 player->dir = INPUT_TYPE_LEFT;
    if (inputType == INPUT_TYPE_RIGHT) player->dir = INPUT_TYPE_RIGHT;
    if (inputType == INPUT_TYPE_DOWN) player->dir = INPUT_TYPE_DOWN;
    if (inputType == INPUT_TYPE_UP) player->dir = INPUT_TYPE_UP;

    int tempx = player->x, tempy = player->y;

    if (player->dir == INPUT_TYPE_LEFT)
    {

        if (player->x > 0)
        {
            tempx = player->x - 1;
        }
        else
        { //allow for wrap around
            gameMode = MODE_GAMEOVER;
            return;
        }

    } 
    else if (player->dir == INPUT_TYPE_RIGHT)
    {

        if (player->x < MAP_WIDTH - 1)
        {
                tempx = player->x + 1;
        }
        else
        {
            gameMode = MODE_GAMEOVER;
            return;
        }
    } 
    else if (player->dir == INPUT_TYPE_UP)
    {
        if (player->y > 0)
        {
            tempy = player->y - 1;
        }
        else
        {
            gameMode = MODE_GAMEOVER;
            return;
        }

    } 
    else if (player->dir == INPUT_TYPE_DOWN)
    {

        if (player->y < MAP_HEIGHT - 1)
    	{
        	tempy = player->y + 1;
        }
        else{
            gameMode = MODE_GAMEOVER;
            return;
        }
    } 
    else 
    {
        platform_quit();
    }

    if (objMap[tempy][tempx] == OBJECT_APPLE) //the snake has run into an apple and another node is created
    {
	    score++;
        snake_node* temp = player;
        while (temp->next != NULL) //the snake is essentially a linked list and we're traversing it
        {
            temp = temp->next;
        }
        int newNodex = temp->x; //when we reach the final node, we store its location with two variables
        int newNodey = temp->y;
        player = move_body(player, tempx,tempy);
        snake_node* newNode = platform_memory_allocate(sizeof(snake_node)); //after we move the whole body we make a new node line( screen, 130, 130, 150, 150, makecol( 255, 0, 0));
        newNode->x = newNodex;
        newNode->y = newNodey; //we set this new node's location variables
        newNode->dir = temp->dir;
        newNode->next = NULL;
        temp->next = newNode; //we set the temp node (the previous tail)'s next node to our new tail!
        generate_new_apple();
    }
    else if (objMap[tempy][tempx] == OBJECT_SNAKE)
    {
    	gameMode = MODE_GAMEOVER;
        return;
    }
    else
    {
 	    player = move_body(player,tempx,tempy);
    }

    objMap[tempy][tempx] = OBJECT_SNAKE; //update the object map to the new snake head position
}

void game_update(void)
{
    switch (gameMode)   
    {
        case MODE_TITLE:
            title_screen_read_input();
            platform_draw_title_screen(gameSpeed);
            platform_adjust_speed(gameSpeed, MODE_TITLE);
            break;
        
        case MODE_GAMEOVER:
            platform_draw_game_over_screen(score);
            platform_adjust_speed(gameSpeed, MODE_GAMEOVER);
            game_reset(); // reset the game after showing game over
            break;

        default:
            move();
            platform_draw_game_screen(objMap, score);
            platform_update_platform_state();
            platform_adjust_speed(gameSpeed, MODE_GAME);
            break;
    }

}

// platform agnostic primary game logic
void game_initialize()
{
    
    platform_initialize();
    player = platform_memory_allocate(sizeof(snake_node));

    game_reset();
}

void game_reset()
{
    gameMode = MODE_TITLE;
    score = 0;
    
    player->dir =  INPUT_TYPE_LEFT; //init direction
    player->x = MAP_WIDTH/2;
    player->y = MAP_HEIGHT/2;
    player->next = NULL;

    int i, j; //this is so we don't get errors because the object map references nothing.
    for (i = 0; i < MAP_HEIGHT; i++)
    {
        for (j = 0; j < MAP_WIDTH; j++)
        {
            objMap[i][j] = OBJECT_NOTHING;
        }
    }
    // we need to seed our rand() and generate our first random object
    //srand(time(NULL));
    platform_set_random_seed(time(NULL));
    generate_new_apple();
    platform_reset();
}

void game_shutdown(void)
{
    platform_shutdown();
}