#include "types.h"
#include "game.h"
#include "../platform/platform.h"

/*
*   Snake: Platform agnostic game logic.
*   Last Update: Jul 19, 2026
*   Author: Scott Harner
*/

static struct game_config* config;
static int *object_map;//This will contain all the objects. will use it to keep track of collisions
static speed game_speed = SPEED_SLOW;
static int score;//score for the game
static struct snake_node* player;
static mode game_mode;
static mode previous_game_mode;
static int action_cycles;
static unsigned int frame_counter = 0;
static bool game_started = false;
static loss_type current_loss_type = LOSS_TYPE_SELF;
static int snake_length;

// track key changes for better title menu input handling
static bool current_input_states[INPUT_TYPE_COUNT];
static bool previous_input_states[INPUT_TYPE_COUNT];

// finds a random location to place the next apple
static void generate_new_apple()
{
	int rand_y, rand_x;

    do{
		rand_y = (int)(platform_get_random(config->map_height/2));
		rand_x = (int)(platform_get_random(config->map_width/2));
	}while (object_map[rand_y * config->map_width + rand_x] != OBJECT_NOTHING); //while we generate a spot that's taken, keep going;
	object_map[rand_y * config->map_width + rand_x] = OBJECT_APPLE;
}

/*
*  This function recursively moves the snake. It goes all the way to the last node of the snake, changes its coordinates,
*  and then changes its coordinates, all the way up.
*/
static snake_node* move_body(snake_node* current_node, int temp_x, int temp_y)
{
	if (current_node->next == NULL) //we've reached the end of the snake's body
	{
		object_map[current_node->y * config->map_width + current_node->x] = OBJECT_NOTHING; //reset the object map at this position
	}
	else //we need to keep traversing
	{
		current_node->next = move_body(current_node->next,current_node->x,current_node->y);
	}
	current_node->x = temp_x;
	current_node->y = temp_y;
    return current_node;

}

static void title_screen_read_input()
{
    //this contains the array of flags which tell which button has been pressed. It must be cleared before every input.
    input_type current_input = platform_get_input_type(MODE_TITLE, current_input_states);
    if (current_input == INPUT_TYPE_START)
    {
        game_mode = MODE_GAME;

        if (!game_started)
            platform_set_random_seed(frame_counter); // use frames to seed since some platforms dont have clock

        generate_new_apple();
    }
    else if (current_input == INPUT_TYPE_DOWN) 
    {
        switch (game_speed)
        {
            case SPEED_SLOW:
                game_speed = SPEED_MEDIUM;
                break;

            case SPEED_MEDIUM:
                game_speed = SPEED_FAST;
                break;

            default:
                game_speed = SPEED_SLOW;
                break;
        }
    }
    else if (current_input == INPUT_TYPE_UP) 
    {
        switch (game_speed)
        {
            case SPEED_SLOW:
                game_speed = SPEED_FAST;
                break;

            case SPEED_MEDIUM:
                game_speed = SPEED_SLOW;
                break;

            default:
                game_speed = SPEED_MEDIUM;
                break;
        }
    }
}

static void move()
{
    //this contains the array of flags which tell which button has been pressed. It must be cleared before every input.
    input_type current_input = platform_get_input_type(MODE_GAME, current_input_states);
    if (current_input == INPUT_TYPE_LEFT) player->dir = INPUT_TYPE_LEFT;
    if (current_input == INPUT_TYPE_RIGHT) player->dir = INPUT_TYPE_RIGHT;
    if (current_input == INPUT_TYPE_DOWN) player->dir = INPUT_TYPE_DOWN;
    if (current_input == INPUT_TYPE_UP) player->dir = INPUT_TYPE_UP;

    int temp_x = player->x, temp_y = player->y;

    if (player->dir == INPUT_TYPE_LEFT)
    {

        if (player->x > 0)
        {
            temp_x = player->x - 1;
        }
        else
        { //allow for wrap around
            game_mode = MODE_GAME_OVER;
            current_loss_type = LOSS_TYPE_WALL;
            platform_play_sound(SOUND_LOSE);
            return;
        }

    } 
    else if (player->dir == INPUT_TYPE_RIGHT)
    {

        if (player->x < config->map_width - 1)
        {
                temp_x = player->x + 1;
        }
        else
        {
            game_mode = MODE_GAME_OVER;
            current_loss_type = LOSS_TYPE_WALL;
            platform_play_sound(SOUND_LOSE);
            return;
        }
    } 
    else if (player->dir == INPUT_TYPE_UP)
    {
        if (player->y > 0)
        {
            temp_y = player->y - 1;
        }
        else
        {
            game_mode = MODE_GAME_OVER;
            current_loss_type = LOSS_TYPE_WALL;
            platform_play_sound(SOUND_LOSE);
            return;
        }

    } 
    else if (player->dir == INPUT_TYPE_DOWN)
    {

        if (player->y < config->map_height - 1)
    	{
        	temp_y = player->y + 1;
        }
        else{
            game_mode = MODE_GAME_OVER;
            current_loss_type = LOSS_TYPE_WALL;
            platform_play_sound(SOUND_LOSE);
            return;
        }
    } 
    else 
    {
        game_mode = MODE_GAME_OVER;
        current_loss_type = LOSS_TYPE_WALL;
        platform_play_sound(SOUND_LOSE);
        return;
    }

    if (object_map[temp_y * config->map_width + temp_x] == OBJECT_APPLE) //the snake has run into an apple and another node is created
    {
	    score++;
        platform_play_sound(SOUND_PICKUP);
        snake_node* temp = player;
        while (temp->next != NULL) //the snake is essentially a linked list and we're traversing it
        {
            temp = temp->next;
        }
        int new_node_x = temp->x; //when we reach the final node, we store its location with two variables
        int new_node_y = temp->y;
        player = move_body(player, temp_x,temp_y);
        snake_node* new_node = platform_memory_allocate(sizeof(snake_node)); //after we move the whole body we make a new node line( screen, 130, 130, 150, 150, makecol( 255, 0, 0));
        new_node->x = new_node_x;
        new_node->y = new_node_y; //we set this new node's location variables
        new_node->dir = temp->dir;
        new_node->next = NULL;
        temp->next = new_node; //we set the temp node (the previous tail)'s next node to our new tail!
        snake_length++;
        if (snake_length == (config->map_height * config->map_width))
            game_mode = MODE_WIN; // the player wins if the snake can no longer grow
        else
            generate_new_apple();
    }
    else if (object_map[temp_y * config->map_width + temp_x] == OBJECT_SNAKE)
    {
    	game_mode = MODE_GAME_OVER;
        current_loss_type = LOSS_TYPE_SELF;
        platform_play_sound(SOUND_LOSE);
        return;
    }
    else
    {
 	    player = move_body(player,temp_x,temp_y);
    }

    object_map[temp_y * config->map_width + temp_x] = OBJECT_SNAKE; //update the object map to the new snake head position
}

void game_reset_input_states()
{
    for (int i = 0; i < INPUT_TYPE_COUNT; i++)
    {
        previous_input_states[i] = false;
        current_input_states[i] = false;
    }

}

// calculate the max movement cycles based upon the current speed
static int get_move_max_cycles()
{
    switch (game_speed)
    {
        case SPEED_FAST:
            return FAST_SPEED_MAX_CYCLES;
            break;
        case SPEED_MEDIUM:
            return MEDIUM_SPEED_MAX_CYCLES;
            break;
        default:
            return SLOw_SPEED_MAX_CYCLES;
            break;

    }
}

void game_update(void)
{
    action_cycles++;
    frame_counter++;
    bool did_mode_change = false;
    if (previous_game_mode != game_mode)
        did_mode_change = true;

    previous_game_mode = game_mode;

    switch (game_mode)   
    {
        case MODE_TITLE:
            title_screen_read_input();
            platform_draw_title_screen(game_speed, did_mode_change);
            action_cycles = 0; // always reset for title screen as were not timing anything
            break;
        
        case MODE_GAME_OVER:
            platform_draw_game_over_screen(score, did_mode_change, current_loss_type);
            if (action_cycles > GAME_OVER_MAX_CYCLES)
                game_reset(); // reset the game after showing game over
                
            break;

        case MODE_WIN:
            platform_draw_win_screen(score, did_mode_change);
            if (action_cycles > WIN_MAX_CYCLES)
                game_reset(); // reset the game after showing win screen
                
            break;

        default:
            if (action_cycles > get_move_max_cycles())    
            {
                move();
                action_cycles = 0;
            }
        
            platform_draw_game_screen(object_map, score, did_mode_change, config);
            platform_update_platform_state();
            break;
    }

}

// platform agnostic primary game logic
void game_initialize(int map_height, int map_width, int tile_size)
{
    platform_initialize();
    previous_game_mode = MODE_NONE;
    player = platform_memory_allocate(sizeof(snake_node));
    player->next = NULL;
    config = platform_memory_allocate(sizeof(game_config));
    config->map_height = map_height;
    config->map_width = map_width;
    config->tile_size = tile_size;
    object_map = platform_memory_allocate(config->map_height * config->map_width * sizeof(int));

    game_reset();
}

// check if input was newly pressed
bool game_input_pressed(input_type candidate_input)
{
    return current_input_states[candidate_input] && !previous_input_states[candidate_input];
}

void game_save_previous_inputstates()
{
    // save previous state
    for (int i = 0; i < INPUT_TYPE_COUNT; i++)
        previous_input_states[i] = current_input_states[i];
}

static void free_snake()
{
    snake_node *current = player->next;

    while (current != NULL)
    {
        snake_node *next = current->next;
        platform_memory_free(current);
        current = next;
    }

    player->next = NULL;
}

void game_reset()
{
    platform_reset(config);

    game_mode = MODE_TITLE;
    score = 0;
    action_cycles = 0;
    snake_length = 1;
    
     if (player->next != NULL)
         free_snake();

    player->dir =  INPUT_TYPE_LEFT; //init direction
    player->x = (config->map_width)/2;
    player->y = (config->map_height)/2;

    int i, j; //this is so we don't get errors because the object map references nothing.
    for (i = 0; i < config->map_height; i++)
    {
        for (j = 0; j < config->map_width; j++)
        {
            object_map[i * config->map_width + j] = OBJECT_NOTHING;
        }
    }

    game_reset_input_states();
}

void game_shutdown(void)
{
    free_snake();
    if (player != NULL)
    {
        platform_memory_free(player);
        player = NULL;
    }

    if (config != NULL)
    {
        platform_memory_free(config);
        config = NULL;
    }

    if (object_map != NULL)
    {
        platform_memory_free(object_map);
        object_map = NULL;
    }

    platform_shutdown();
}