/*
** Jo Sega Saturn Engine
** Copyright (c) 2012-2017, Johannes Fetz (johannesfetz@gmail.com)
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the Johannes Fetz nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL Johannes Fetz BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <jo/jo.h>
#include "../../platform.h"

/*
*   Snake: Sega Saturn implementation of snake using joengine framework.
*   Last Update: Jul 22, 2026
*   Author: Scott Harner
*/

// Saturn implementation of platform initialization
void platform_initialize()
{
    jo_core_init(JO_COLOR_Black);
}

// display a game over screen
void platform_draw_game_over_screen(int score)
{
    // todo - implement this
    (void)score;
}

// calculate the color to display for a menu option
static int getOptionColor(speed selectedSpeed, speed optionSpeed)
{
    int selectedColor = JO_COLOR_INDEX_Yellow;
    int defaultColor = JO_COLOR_INDEX_White;
    return selectedSpeed == optionSpeed ? selectedColor : defaultColor;
}

// display a title screen
void platform_draw_title_screen(speed gameSpeed)
{
    jo_printf_with_color(0, 1, JO_COLOR_INDEX_White, "SNAKE");

    jo_printf_with_color(0, 5, getOptionColor(gameSpeed, SPEED_SLOW), "Slow");
    jo_printf_with_color(0, 7, getOptionColor(gameSpeed, SPEED_MEDIUM), "Medium");
    jo_printf_with_color(0, 9, getOptionColor(gameSpeed, SPEED_FAST), "Fast");
}

// platform specific setting of random generator seed
void platform_set_random_seed(unsigned int seed)
{
    jo_random_seed = seed;
}

// platform specific random number generation
int platform_get_random(int max)
{
    return jo_random(max);
}

// platform specific memory allocation
void * platform_memory_allocate(unsigned int size)
{
    return jo_malloc(size);
}

// steps to prepare to exit the game
void platform_shutdown()
{

}

// end the game immediately
void platform_quit()
{
    // todo - implement this
}

// any logic for making certain that graphics finish rendering at an appropriate speed
void platform_adjust_speed(speed gameSpeed, mode gameMode)
{
    // we dont need to adjust the speed for this platform
    (void)gameSpeed; // avoid compiler warnings
    (void)gameMode; // avoid compiler warnings
}

// track all current and previous input states so we can check on input presses
static void update_input_states(bool current_inputstates[INPUT_TYPE_COUNT])
{
    game_save_previous_inputstates();

    // read current state
    current_inputstates[INPUT_TYPE_UP] = jo_is_pad1_key_pressed(JO_KEY_UP);
    current_inputstates[INPUT_TYPE_DOWN] = jo_is_pad1_key_pressed(JO_KEY_DOWN);
    current_inputstates[INPUT_TYPE_LEFT] = jo_is_pad1_key_pressed(JO_KEY_LEFT);
    current_inputstates[INPUT_TYPE_RIGHT] = jo_is_pad1_key_pressed(JO_KEY_RIGHT);
    current_inputstates[INPUT_TYPE_START] = jo_is_pad1_key_pressed(JO_KEY_START);    
}

// retrieve the input type from the user
input_type platform_get_input_type(mode gameMode, bool current_inputstates[INPUT_TYPE_COUNT])
{
    input_type currentInput = INPUT_TYPE_NOTHING;
    if (jo_is_pad1_available())
    {
        switch(gameMode)
        {
            case MODE_TITLE:
                update_input_states(current_inputstates);
                if (game_input_pressed(INPUT_TYPE_START)) currentInput = INPUT_TYPE_START;    
                else if (game_input_pressed(INPUT_TYPE_DOWN)) currentInput = INPUT_TYPE_DOWN;
                else if (game_input_pressed(INPUT_TYPE_UP)) currentInput = INPUT_TYPE_UP;

                break;

            default:
                // if (key[KEY_LEFT]) currentInput = INPUT_TYPE_LEFT;
                // else if (key[KEY_RIGHT]) currentInput = INPUT_TYPE_RIGHT;
                // else if (key[KEY_DOWN]) currentInput = INPUT_TYPE_DOWN;
                // else if (key[KEY_UP]) currentInput = INPUT_TYPE_UP;
                // else if (key[KEY_ENTER]) currentInput = INPUT_TYPE_START;

                break;
        }
    }
    else
    {
        game_reset_input_states();
    }

    return currentInput;
}

void platform_update_platform_state()
{
    // we dont have any updates to make on this platform
}

void platform_draw_game_screen(int objMap[MAP_HEIGHT][MAP_WIDTH], int score)
{
    // todo - implement this
    (void)objMap; // avoid compiler warnings
    (void)score; // avoid compiler warnings
}

void jo_main(void)
{
    game_initialize();
    jo_core_add_callback(game_update);
    jo_core_run();
}

/*
** END OF FILE
*/