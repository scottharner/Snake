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
#include "pcmsys.h"

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

static int snake_sprite_id;
static int apple_sprite_id;
static int border_top_sprite_id;
static int border_left_sprite_id;
static int border_corner_sprite_id;
static short pickup_sound_id;
static short lose_sound_id;

// Saturn implementation of platform initialization
void platform_initialize()
{
    jo_core_init(JO_COLOR_Black);
    load_drv(ADX_MASTER_2304);
    snake_sprite_id = jo_sprite_add_tga("TEX", "SNAKE.TGA", JO_COLOR_Transparent);
    apple_sprite_id = jo_sprite_add_tga("TEX", "APPLE.TGA", JO_COLOR_Transparent);
    border_top_sprite_id = jo_sprite_add_tga("TEX", "BORDERT.TGA", JO_COLOR_Transparent);
    border_left_sprite_id = jo_sprite_add_tga("TEX", "BORDERL.TGA", JO_COLOR_Transparent);
    border_corner_sprite_id = jo_sprite_add_tga("TEX", "BORDERC.TGA", JO_COLOR_Transparent);
    pickup_sound_id = load_8bit_pcm((Sint8 *)"PICKUP.PCM", 15360); // using ponetone due to issues with jo engine audio
    lose_sound_id = load_8bit_pcm((Sint8 *)"LOSE.PCM", 15360);
}

// plays the requested sound effect
void platform_play_sound(sound_type current_sound_type)
{
    if (current_sound_type == SOUND_PICKUP)
    {
        pcm_play(pickup_sound_id, PCM_PROTECTED, 6);
    }
    else if (current_sound_type == SOUND_LOSE)
    {
        pcm_play(lose_sound_id, PCM_PROTECTED, 6);
    }
}

// display a game over screen
void platform_draw_game_over_screen(int score, bool did_mode_change, loss_type current_loss_type)
{
    if (did_mode_change)
        jo_clear_screen();

    jo_printf_with_color(5, 5, JO_COLOR_INDEX_White, "Game Over");
    jo_printf_with_color(5, 7, JO_COLOR_INDEX_White, "Score: %d", score);
    jo_printf_with_color(5, 9, JO_COLOR_INDEX_White, "Reason: %s", current_loss_type == LOSS_TYPE_SELF ? "Self Collision" : "Wall Collision");
}

// display a win screen
void platform_draw_win_screen(int score, bool did_mode_change)
{
    if (did_mode_change)
        jo_clear_screen();

    jo_printf_with_color(5, 5, JO_COLOR_INDEX_White, "You Win!");
    jo_printf_with_color(5, 7, JO_COLOR_INDEX_White, "Score: %d", score);
}

// calculate the color to display for a menu option
static int get_option_color(speed selected_speed, speed option_speed)
{
    int selected_color = JO_COLOR_INDEX_Yellow;
    int default_color = JO_COLOR_INDEX_White;
    return selected_speed == option_speed ? selected_color : default_color;
}

// display a title screen
void platform_draw_title_screen(speed game_speed, bool did_mode_change)
{
    if (did_mode_change)
        jo_clear_screen();

    jo_printf_with_color(0, 1, JO_COLOR_INDEX_White, "SNAKE");

    jo_printf_with_color(0, 5, get_option_color(game_speed, SPEED_SLOW), "Slow");
    jo_printf_with_color(0, 7, get_option_color(game_speed, SPEED_MEDIUM), "Medium");
    jo_printf_with_color(0, 9, get_option_color(game_speed, SPEED_FAST), "Fast");
}

// platform specific setting of random generator seed
void platform_set_random_seed(unsigned int seed)
{
    jo_random_seed = seed;
}

// platform specific random number generation
int platform_get_random(int max)
{
    if (max <= 0) return 0;
    return (jo_random(max)-1); // jo_random requires passing parm so we cant use modulo
}

// platform specific memory allocation
void * platform_memory_allocate(unsigned int size)
{
    return jo_malloc(size);
}

// platform specific memory free
void platform_memory_free(void *pointer)
{
    jo_free(pointer);
}

// steps to prepare to exit the game
void platform_shutdown()
{
    // we dont have any shutdown steps to perform on this platform
}

// track all current and previous input states so we can check on input presses
static void update_input_states(bool current_input_states[INPUT_TYPE_COUNT])
{
    game_save_previous_inputstates();

    // read current state
    current_input_states[INPUT_TYPE_UP] = jo_is_pad1_key_pressed(JO_KEY_UP);
    current_input_states[INPUT_TYPE_DOWN] = jo_is_pad1_key_pressed(JO_KEY_DOWN);
    current_input_states[INPUT_TYPE_LEFT] = jo_is_pad1_key_pressed(JO_KEY_LEFT);
    current_input_states[INPUT_TYPE_RIGHT] = jo_is_pad1_key_pressed(JO_KEY_RIGHT);
    current_input_states[INPUT_TYPE_START] = jo_is_pad1_key_pressed(JO_KEY_START);    
}

// retrieve the input type from the user
input_type platform_get_input_type(mode game_mode, bool current_input_states[INPUT_TYPE_COUNT])
{
    input_type current_input = INPUT_TYPE_NOTHING;
    if (jo_is_pad1_available())
    {
        update_input_states(current_input_states);
        switch(game_mode)
        {
            case MODE_TITLE:
                if (game_input_pressed(INPUT_TYPE_START)) current_input = INPUT_TYPE_START;    
                else if (game_input_pressed(INPUT_TYPE_DOWN)) current_input = INPUT_TYPE_DOWN;
                else if (game_input_pressed(INPUT_TYPE_UP)) current_input = INPUT_TYPE_UP;

                break;

            default:
                if (game_input_pressed(INPUT_TYPE_START)) current_input = INPUT_TYPE_START;
                else if (jo_is_pad1_key_pressed(JO_KEY_LEFT)) current_input = INPUT_TYPE_LEFT;
                else if (jo_is_pad1_key_pressed(JO_KEY_RIGHT)) current_input = INPUT_TYPE_RIGHT;
                else if (jo_is_pad1_key_pressed(JO_KEY_DOWN)) current_input = INPUT_TYPE_DOWN;
                else if (jo_is_pad1_key_pressed(JO_KEY_UP)) current_input = INPUT_TYPE_UP;

                break;
        }
    }
    else
    {
        game_reset_input_states();
    }

    return current_input;
}

void platform_update_platform_state()
{
    // we dont have any updates to make on this platform
}

void draw_tile(int x, int y, int sprite_id, int z, int angle)
{
    if (angle == 0)
        jo_sprite_draw3D2(sprite_id, x, y, z);
    else
        jo_sprite_draw3D_and_rotate2(sprite_id, x, y, z, angle);

#if JO_DEBUG
    jo_printf_with_color(0, 0, JO_COLOR_INDEX_White, "tile x: %d", x);
    jo_printf_with_color(0, 1, JO_COLOR_INDEX_White, "tile y: %d", y);
#endif
}

// perform platform specific actions when the game resets
void platform_reset(game_config *config)
{
    // no actions to perform on this platform
    (void)config; // avoid compiler warnings
}

static void draw_border(game_config *config)
{
    for (int i = 1; i < config->map_width-1; i++)
    {
        // top
        draw_tile(i * config->tile_size, 0, border_top_sprite_id, BORDER_ZINDEX, 0);
    }

    jo_sprite_enable_vertical_flip();
    for (int i = 1; i < config->map_width-1; i++)
    {
        // bottom
        draw_tile(i * config->tile_size, (MAP_HEIGHT*config->tile_size)-config->tile_size-1, border_top_sprite_id, BORDER_ZINDEX, 0);
    }
    jo_sprite_disable_vertical_flip();

    for (int i = 1; i < config->map_height-1; i++)
    {
        // left
        draw_tile(0, i * config->tile_size, border_left_sprite_id, BORDER_ZINDEX, 0);
    }

    jo_sprite_enable_horizontal_flip();
    for (int i = 1; i < config->map_height-1; i++)
    {
        // right
        draw_tile((MAP_WIDTH*config->tile_size)-config->tile_size-1, i * config->tile_size, border_left_sprite_id, BORDER_ZINDEX, 0);
    }
    jo_sprite_disable_horizontal_flip();

    // upper left
    draw_tile(0, 0, border_corner_sprite_id, BORDER_ZINDEX, 0);
    
    // upper right
    jo_sprite_enable_horizontal_flip();
    draw_tile((MAP_WIDTH*config->tile_size)-config->tile_size-1, 0, border_corner_sprite_id, BORDER_ZINDEX, 0);
    jo_sprite_disable_horizontal_flip();

    // lower left
    jo_sprite_enable_vertical_flip();
    draw_tile(0, (MAP_HEIGHT*config->tile_size)-config->tile_size-1, border_corner_sprite_id, BORDER_ZINDEX, 0);

    // lower right
    jo_sprite_enable_horizontal_flip();
    draw_tile((MAP_WIDTH*config->tile_size)-config->tile_size-1, (MAP_HEIGHT*config->tile_size)-config->tile_size-1, border_corner_sprite_id, BORDER_ZINDEX, 0);
    jo_sprite_disable_horizontal_flip();
    jo_sprite_disable_vertical_flip();
}

void platform_draw_game_screen(int *object_map, int score, bool did_mode_change, game_config *config)
{
    if (did_mode_change)
        jo_clear_screen();

    int i,j;
    //O(N^2) runtime for this, 24^2 is pretty big.. so we may change this
    //but for now, we draw every tile every frame!
    for (i = 0; i < config->map_height; i++)
    {
        for (j = 0; j < config->map_width; j++)
        {
            int sprite_id;
            int tile_index = i * config->map_width + j;
            if (object_map[tile_index] == OBJECT_APPLE)
            {
                sprite_id = apple_sprite_id;
            }
            else if (object_map[tile_index] == OBJECT_SNAKE)
            {
                sprite_id = snake_sprite_id;
            }
            else
            {
                continue;
            }

            draw_tile(config->tile_size*j, config->tile_size*i, sprite_id, OBJECT_ZINDEX, 0);
        }
    }

    //draw the score
    jo_printf_with_color(JO_GRID_WIDTH-10, 1, JO_COLOR_INDEX_White, "Score: %d", score);

    draw_border(config);
}

void jo_main(void)
{
    game_initialize(MAP_HEIGHT, MAP_WIDTH, TILE_SIZE);
	jo_core_add_vblank_callback(sdrv_vblank_rq);
    jo_core_add_callback(game_update);
    jo_core_run();
}

/*
** END OF FILE
*/