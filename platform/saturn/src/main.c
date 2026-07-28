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

#define OBJECT_ZINDEX 450
#define BORDER_ZINDEX 500
#define JO_GRID_WIDTH (JO_TV_WIDTH / 8)
#define JO_GRID_HEIGHT (JO_TV_HEIGHT / 8)
#define MAP_HEIGHT 30
#define MAP_WIDTH 40
#define TILE_SIZE 8

static int green_sprite_id;
static int red_sprite_id;
static bool show_debug_info;

// Saturn implementation of platform initialization
void platform_initialize()
{
    jo_core_init(JO_COLOR_Black);
    green_sprite_id = jo_sprite_add_tga("TEX", "GREEN.TGA", JO_COLOR_Transparent);
    red_sprite_id = jo_sprite_add_tga("TEX", "RED.TGA", JO_COLOR_Transparent);
    show_debug_info = false;
}

// display a game over screen
// returns flag indicating whether time is up for displaying game over
void platform_draw_game_over_screen(int score, bool didModeChange)
{
    if (didModeChange)
        jo_clear_screen();

    jo_printf_with_color(5, 5, JO_COLOR_INDEX_White, "Game Over");
    jo_printf_with_color(5, 7, JO_COLOR_INDEX_White, "Score: %d", score);
}

// calculate the color to display for a menu option
static int getOptionColor(speed selectedSpeed, speed optionSpeed)
{
    int selectedColor = JO_COLOR_INDEX_Yellow;
    int defaultColor = JO_COLOR_INDEX_White;
    return selectedSpeed == optionSpeed ? selectedColor : defaultColor;
}

// display a title screen
void platform_draw_title_screen(speed gameSpeed, bool didModeChange)
{
    if (didModeChange)
        jo_clear_screen();

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
    // we dont have any shutdown steps to perform on this platform
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
                if (jo_is_pad1_key_pressed(JO_KEY_LEFT)) currentInput = INPUT_TYPE_LEFT;
                else if (jo_is_pad1_key_pressed(JO_KEY_RIGHT)) currentInput = INPUT_TYPE_RIGHT;
                else if (jo_is_pad1_key_pressed(JO_KEY_DOWN)) currentInput = INPUT_TYPE_DOWN;
                else if (jo_is_pad1_key_pressed(JO_KEY_UP)) currentInput = INPUT_TYPE_UP;
                else if (jo_is_pad1_key_pressed(JO_KEY_START)) currentInput = INPUT_TYPE_START;

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

void draw_tile(int x, int y, int width, int height, int sprite_id)
{
    // jo engine uses a center origin coordinate system so we have to recalculate from the provided upper left coords
    int jo_x = x - JO_TV_WIDTH_2;
    int jo_y = y - JO_TV_HEIGHT_2;
    jo_x += (width/2);
    jo_y += (height/2);

    jo_sprite_draw3D2(sprite_id, x, y, OBJECT_ZINDEX);

    if (show_debug_info)
    {
        jo_printf_with_color(0, 0, JO_COLOR_INDEX_White, "tile x: %d", x);
        jo_printf_with_color(0, 1, JO_COLOR_INDEX_White, "tile y: %d", y);
    }
}

// static void draw_border()
// {
//     //draw an outline of the game map (single pixel blue border)
//     int left_edge = -JO_TV_WIDTH_2;
//     int right_edge = JO_TV_WIDTH_2 - 1;
//     int top_edge = JO_TV_WIDTH_2;
//     int bottom_edge = JO_TV_HEIGHT_2 - 1;
    
//     // top
//     jo_sprite_change_sprite_scale_xy((float)JO_TV_WIDTH, (float)1);
//     jo_sprite_draw3D(tile_sprite_id, 0, top_edge, BORDER_ZINDEX);
    
//     // bottom
//     jo_sprite_change_sprite_scale_xy((float)JO_TV_WIDTH, (float)1);
//     jo_sprite_draw3D(tile_sprite_id, 0, bottom_edge, BORDER_ZINDEX);
    
//     // left
//     jo_sprite_change_sprite_scale_xy((float)1, (float)JO_TV_HEIGHT);
//     jo_sprite_draw3D(tile_sprite_id, left_edge, 0, BORDER_ZINDEX);
    
//     // right
//     jo_sprite_change_sprite_scale_xy((float)1, (float)JO_TV_HEIGHT);
//     jo_sprite_draw3D(tile_sprite_id, right_edge, 0, BORDER_ZINDEX);
    
//     jo_sprite_change_sprite_scale(1.0f);
// }

void platform_draw_game_screen(int objMap[MAX_MAP_HEIGHT][MAX_MAP_WIDTH], int score, bool didModeChange, game_config *config)
{
    if (didModeChange)
        jo_clear_screen();

    int i,j;
    //O(N^2) runtime for this, 24^2 is pretty big.. so we may change this
    //but for now, we draw every tile every frame!
    for (i = 0; i < config->map_height; i++)
    {
        for (j = 0; j < config->map_width; j++)
        {
            int sprite_id;
            if (objMap[i][j] == OBJECT_APPLE)
            {
                sprite_id = red_sprite_id;
            }
            else if (objMap[i][j] == OBJECT_SNAKE)
            {
                sprite_id = green_sprite_id;
            }
            else
            {
                continue;
            }

            draw_tile(config->tile_size*j, config->tile_size*i, config->tile_size, config->tile_size, sprite_id);
        }
    }

    //draw the score
    jo_printf_with_color(JO_GRID_WIDTH-10, 0, JO_COLOR_INDEX_White, "score: %d", score);

    // todo - fix border
    //draw_border();
}

void jo_main(void)
{
    game_initialize(MAP_HEIGHT, MAP_WIDTH, TILE_SIZE);
    jo_core_add_callback(game_update);
    jo_core_run();
}

/*
** END OF FILE
*/