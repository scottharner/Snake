#include <allegro.h>
#include "../../platform.h"

/*
*   Snake: Windows implementation of snake using allegro framework.
*   Last Update: Jul 19, 2026
*   Author: Scott Harner
*/

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
void platform_adjust_speed(int gamespeed)
{
    //slow the game down
    resting=0;

    //this formula is hacked together. This number is the one which determines how slow the game is. The
    //higher the number the longer we wait. So higher gamespeed means a lower wait.
    rest_callback(100-gamespeed * 30, rest1);
}

// check if the game is still running
static bool is_running()
{
    return !key[KEY_ESC];
}

// retrieve the input direction from the user
direction platform_get_input_direction()
{
    direction inputDirection = none;
    clear_keybuf();

    if (key[KEY_LEFT]) inputDirection = left;
    else if (key[KEY_RIGHT]) inputDirection = right;
    else if (key[KEY_DOWN]) inputDirection = down;
    else if (key[KEY_UP]) inputDirection = up;

    return inputDirection;
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
            if (objMap[i][j] == nothing){
                c = makecol(0,0,0);
            }else if (objMap[i][j] == apple){
                c = makecol(255,0,0);
            }
            else if (objMap[i][j] == snake){
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