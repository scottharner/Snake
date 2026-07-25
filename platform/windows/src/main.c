#include <allegro.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "../../platform.h"

/*
*   Snake: Windows implementation of snake using allegro framework.
*   Last Update: Jul 19, 2026
*   Author: Scott Harner
*/

// track key changes for better title menu input handling
static bool current_keystates[INPUT_TYPE_COUNT];
static bool previous_keystates[INPUT_TYPE_COUNT];

//timer variables
//it seems like framerate and some other variables my have no useful purpose
//since our goal is primarily porting we will leave that be for now
volatile int counter;
volatile int ticks;
volatile int framerate;
volatile int resting, rested;

BITMAP *buffer;//This will be our temporary bitmap for double buffering 

//do something while resting (?)
static void rest1(void)
{
    resting++;
}

//calculate framerate every second
static void timer1(void)
{
    counter++;
    framerate = ticks;
    ticks=0;
    rested=resting;
}

//Windows implementation of platform initialization
void platform_initialize()
{
    allegro_init();
    install_keyboard();

    set_color_depth(16); //graphics
    set_gfx_mode( GFX_GDI, MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE, 0, 0);
    buffer = create_bitmap( MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE);

    install_timer();

    //lock interrupt variables

    LOCK_VARIABLE(counter);
    LOCK_VARIABLE(framerate);
    LOCK_VARIABLE(ticks);
    LOCK_VARIABLE(resting);
    LOCK_VARIABLE(rested);
    LOCK_FUNCTION(timer1);
    LOCK_FUNCTION(rest1);

    install_int(timer1, 1000);
}

// display a game over screen
void platform_draw_game_over_screen(int score)
{
    clear_to_color(buffer, makecol(0, 0, 0));
    
    textout_ex(buffer, font, "Game Over", 50, 50, makecol(255,255,255), -1);
    textprintf_ex(buffer, font, 50, 70, makecol(255,255,255), -1, "Score: %d", score);

    blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}

// calculate the color to display for a menu option
static int getOptionColor(speed selectedSpeed, speed optionSpeed)
{
    int selectedColor = makecol(255,255,0);
    int defaultColor = makecol(255,255,255);
    return selectedSpeed == optionSpeed ? selectedColor : defaultColor;
}

// display a title screen
void platform_draw_title_screen(speed gameSpeed)
{
    clear_to_color(buffer, makecol(0, 0, 0));
    textout_ex(buffer, font, "SNAKE", 50, 10, makecol(255,255,255), -1);
    
    textout_ex(buffer, font, "Slow", 50, 50, getOptionColor(gameSpeed, SPEED_SLOW), -1);
    textout_ex(buffer, font, "Medium", 50, 70, getOptionColor(gameSpeed, SPEED_MEDIUM), -1);
    textout_ex(buffer, font, "Fast", 50, 90, getOptionColor(gameSpeed, SPEED_FAST), -1);

    blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}

// platform specific setting of random generator seed
void platform_set_random_seed(unsigned int seed)
{
    srand(seed);
}

// platform specific random number generation
int platform_get_random(int max)
{
    if (max <= 0) return 0;
    return rand() % max;
}

// platform specific memory allocation
void * platform_memory_allocate(unsigned int size)
{
    return malloc(size);
}

// steps to prepare to exit the game
void platform_shutdown()
{

}

// end the game immediately
void platform_quit()
{
    exit(0);
}

// any logic for making certain that graphics finish rendering at an appropriate speed
void platform_adjust_speed(speed gameSpeed, mode gameMode)
{
    //slow the game down
    //the rest callback formula is hacked together. This number is the one which determines how slow the game is. The
    //higher the number the longer we wait. So higher gameSpeed means a lower wait.
    switch(gameMode)
    {
        case MODE_TITLE:
            resting=0;

            rest_callback(100-SPEED_SLOW * 30, rest1);
            break;

        case MODE_GAMEOVER:
            rest(3000);
            break;

        default:
            resting=0;

            rest_callback(100-gameSpeed * 30, rest1);

            break;
    }
}

// check if the game is still running
static bool is_running()
{
    return !key[KEY_ESC];
}

// track all current and previous input states so we can check on input presses
static void update_key_states()
{
    // save previous state
    for (int i = 0; i < INPUT_TYPE_COUNT; i++)
        previous_keystates[i] = current_keystates[i];

    // read current state
    current_keystates[INPUT_TYPE_UP] = key[KEY_UP];
    current_keystates[INPUT_TYPE_DOWN] = key[KEY_DOWN];
    current_keystates[INPUT_TYPE_LEFT] = key[KEY_LEFT];
    current_keystates[INPUT_TYPE_RIGHT] = key[KEY_RIGHT];
    current_keystates[INPUT_TYPE_START] = key[KEY_ENTER];    
}

// check if input was newly pressed
static bool key_pressed(input_type candidateInput)
{
    return current_keystates[candidateInput] && !previous_keystates[candidateInput];
}


// retrieve the input type from the user
input_type platform_get_input_type(mode gameMode)
{
    input_type currentInput = INPUT_TYPE_NOTHING;
    clear_keybuf();

    switch(gameMode)
    {
        case MODE_TITLE:
            update_key_states();
            if (key_pressed(INPUT_TYPE_START)) currentInput = INPUT_TYPE_START;
            else if (key_pressed(INPUT_TYPE_DOWN)) currentInput = INPUT_TYPE_DOWN;
            else if (key_pressed(INPUT_TYPE_UP)) currentInput = INPUT_TYPE_UP;
            break;

        default:
            if (key[KEY_LEFT]) currentInput = INPUT_TYPE_LEFT;
            else if (key[KEY_RIGHT]) currentInput = INPUT_TYPE_RIGHT;
            else if (key[KEY_DOWN]) currentInput = INPUT_TYPE_DOWN;
            else if (key[KEY_UP]) currentInput = INPUT_TYPE_UP;
            else if (key[KEY_ENTER]) currentInput = INPUT_TYPE_START;

            break;
    }

    return currentInput;
}

// platform specific game reset logic
void platform_reset()
{
    for (int i = 0; i < INPUT_TYPE_COUNT; i++)
    {
        previous_keystates[i] = false;
        current_keystates[i] = false;
    }
}

void platform_update_platform_state()
{
    //update ticks
    ticks++;
}

void platform_draw_game_screen(int objMap[MAP_HEIGHT][MAP_WIDTH], int score)
{
    int i,j;
    acquire_screen();	//O(N^2) runtime for this, 24^2 is pretty big.. so we may change this
                        //but for now, we draw every tile every frame!
    for (i = 0; i < MAP_HEIGHT; i++)
    {
        for (j = 0; j < MAP_WIDTH; j++)
        {
            int c; //color to draw
            if (objMap[i][j] == OBJECT_NOTHING){
                c = makecol(0,0,0);
            }else if (objMap[i][j] == OBJECT_APPLE){
                c = makecol(255,0,0);
            }
            else if (objMap[i][j] == OBJECT_SNAKE){
                c = makecol(0,255,0);
            }

            rectfill ( buffer, TILE_SIZE*j, TILE_SIZE*i, TILE_SIZE*(j+1),TILE_SIZE*(i+1), c);
        }
    }

    //draw the score
    char scoretxt[10];
    sprintf(scoretxt,"score: %d",score);
    textout_ex(buffer, font, scoretxt, TILE_SIZE*(MAP_WIDTH)*3/4, TILE_SIZE, makecol(255,255,255), makecol(0,0,0));

    //draw an outline of the game map
    rect( buffer, 0, 0, TILE_SIZE*MAP_WIDTH-1, TILE_SIZE*MAP_HEIGHT-1, makecol( 0, 0, 255));

    draw_sprite( screen, buffer, 0, 0); //draw buffer image on screen every time we draw.
    release_screen();
}

int main(void)
{
    game_initialize();
    
    while (is_running())
    {
        game_update();
    }

    game_shutdown();
    return 0;
}
END_OF_MAIN();