#include <allegro.h>
#include "../platform.h"

/*
*   Snake: Windows implementation of snake using allegro framework.
*   Last Update: Jul 19, 2026
*   Author: Scott Harner
*/

//timer variables
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

// Windows implementation of platform shutdown
void platform_shutdown()
{

}

// any logic for making certain that graphics finish rendering at an appropriate speed
void platform_adjust_speed(int gamespeed)
{
    //this formula is hacked together. This number is the one which determines how slow the game is. The
    //higher the number the longer we wait. So higher gamespeed means a lower wait.
    // todo - allegro
    rest_callback(100-gamespeed * 30, rest1);
}

// check if the game is still running
bool platform_is_running()
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

int main(void)
{
    game_run();
    return 0;
}
END_OF_MAIN();