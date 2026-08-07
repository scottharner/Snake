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

//timer variables
//it seems like framerate and some other variables my have no useful purpose
//since our goal is primarily porting we will leave that be for now
volatile int counter;
volatile int ticks;
volatile int framerate;
volatile int resting, rested;

BITMAP *buffer;//This will be our temporary bitmap for double buffering 

static bool did_sound_install = false;
static SAMPLE *pickup_sample;
static bool did_pickup_load = false;
static SAMPLE *lose_sample;
static bool did_lose_load = false;

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
    set_gfx_mode( GFX_GDI, MAX_MAP_WIDTH * MAX_TILE_SIZE, MAX_MAP_HEIGHT * MAX_TILE_SIZE, 0, 0);
    buffer = create_bitmap( MAX_MAP_WIDTH * MAX_TILE_SIZE, MAX_MAP_HEIGHT * MAX_TILE_SIZE);
    install_timer();
    int sound_load_result = install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL);
    if (sound_load_result == 0)
    {
        did_sound_install = true;
        pickup_sample = load_sample("../assets/sfx/pickup.wav");
        lose_sample = load_sample("../assets/sfx/lose.wav");
        did_pickup_load = (bool)pickup_sample;
        did_lose_load = (bool)lose_sample;
    }


    //lock interrupt variables

    LOCK_VARIABLE(counter);
    LOCK_VARIABLE(framerate);
    LOCK_VARIABLE(ticks);
    LOCK_VARIABLE(resting);
    LOCK_VARIABLE(rested);
    LOCK_FUNCTION(timer1);
    
    install_int(timer1, 1000);
}

// perform platform specific actions when the game resets
void platform_reset(game_config *config)
{
    // no actions to perform on this platform
    (void)config; // avoid compiler warnings
}

// plays the requested sound effect
void platform_play_sound(sound_type current_sound_type)
{
    if (did_sound_install)
    {
        if (current_sound_type == SOUND_PICKUP && did_pickup_load)
        {
            play_sample(pickup_sample, 255, 128, 1000, 0);
        }
        else if (current_sound_type == SOUND_LOSE && did_lose_load)
        {
            play_sample(lose_sample, 255, 128, 1000, 0);
        }
    }
}

// display a game over screen
void platform_draw_game_over_screen(int score, bool did_mode_change, loss_type current_loss_type)
{
    (void)did_mode_change; // avoid compiler warning
    clear_to_color(buffer, makecol(0, 0, 0));
    
    textout_ex(buffer, font, "Game Over", 50, 50, makecol(255,255,255), -1);
    textprintf_ex(buffer, font, 50, 70, makecol(255,255,255), -1, "Score: %d", score);
    textprintf_ex(buffer, font, 50, 90, makecol(255,255,255), -1, "Reason: %s", current_loss_type == LOSS_TYPE_SELF ? "Self Collision" : "Wall Collision");

    blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}

// display a win screen
void platform_draw_win_screen(int score, bool did_mode_change)
{
    (void)did_mode_change; // avoid compiler warning
    clear_to_color(buffer, makecol(0, 0, 0));
    
    textout_ex(buffer, font, "You Win!", 50, 50, makecol(255,255,255), -1);
    textprintf_ex(buffer, font, 50, 70, makecol(255,255,255), -1, "Score: %d", score);

    blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}

// calculate the color to display for a menu option
static int get_option_color(speed selected_speed, speed option_speed)
{
    int selected_color = makecol(255,255,0);
    int default_color = makecol(255,255,255);
    return selected_speed == option_speed ? selected_color : default_color;
}

// display a title screen
void platform_draw_title_screen(speed game_speed, bool did_mode_change)
{
    (void)did_mode_change; // avoid compiler warning
    clear_to_color(buffer, makecol(0, 0, 0));
    textout_ex(buffer, font, "SNAKE", 50, 10, makecol(255,255,255), -1);
    
    textout_ex(buffer, font, "Slow", 50, 50, get_option_color(game_speed, SPEED_SLOW), -1);
    textout_ex(buffer, font, "Medium", 50, 70, get_option_color(game_speed, SPEED_MEDIUM), -1);
    textout_ex(buffer, font, "Fast", 50, 90, get_option_color(game_speed, SPEED_FAST), -1);

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

// platform specific memory free
void platform_memory_free(void *pointer)
{
    free(pointer);
}

// steps to prepare to exit the game
void platform_shutdown()
{
    if (did_pickup_load) destroy_sample(pickup_sample);
    if (did_pickup_load) destroy_sample(lose_sample);
}

// check if the game is still running
static bool is_running()
{
    return !key[KEY_ESC];
}

// track all current and previous input states so we can check on input presses
static void update_input_states(bool current_input_states[INPUT_TYPE_COUNT])
{
    game_save_previous_inputstates();

    // read current state
    current_input_states[INPUT_TYPE_UP] = key[KEY_UP];
    current_input_states[INPUT_TYPE_DOWN] = key[KEY_DOWN];
    current_input_states[INPUT_TYPE_LEFT] = key[KEY_LEFT];
    current_input_states[INPUT_TYPE_RIGHT] = key[KEY_RIGHT];
    current_input_states[INPUT_TYPE_START] = key[KEY_ENTER];    
}

// retrieve the input type from the user
input_type platform_get_input_type(mode game_mode, bool current_input_states[INPUT_TYPE_COUNT])
{
    input_type current_input = INPUT_TYPE_NOTHING;
    clear_keybuf();

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
            else if (key[KEY_LEFT]) current_input = INPUT_TYPE_LEFT;
            else if (key[KEY_RIGHT]) current_input = INPUT_TYPE_RIGHT;
            else if (key[KEY_DOWN]) current_input = INPUT_TYPE_DOWN;
            else if (key[KEY_UP]) current_input = INPUT_TYPE_UP;

            break;
    }

    return current_input;
}

void platform_update_platform_state()
{
    //update ticks
    ticks++;
}

void platform_draw_game_screen(int *object_map, int score, bool did_mode_change, game_config *config)
{
    (void)did_mode_change; // avoid compiler warning
    int i,j;
    acquire_screen();	//O(N^2) runtime for this, 24^2 is pretty big.. so we may change this
                        //but for now, we draw every tile every frame!
    for (i = 0; i < config->map_height; i++)
    {
        for (j = 0; j < config->map_width; j++)
        {
            int c; //color to draw
            int tile_index = i * config->map_width + j;
            if (object_map[tile_index] == OBJECT_NOTHING){
                c = makecol(0,0,0);
            }else if (object_map[tile_index] == OBJECT_APPLE){
                c = makecol(255,0,0);
            }
            else if (object_map[tile_index] == OBJECT_SNAKE){
                c = makecol(0,255,0);
            }

            rectfill ( buffer, config->tile_size*j, config->tile_size*i, config->tile_size*(j+1),config->tile_size*(i+1), c);
        }
    }

    //draw the score
    char score_text[10];
    sprintf(score_text,"score: %d",score);
    textout_ex(buffer, font, score_text, config->tile_size*(config->map_width)*3/4, config->tile_size, makecol(255,255,255), makecol(0,0,0));

    //draw an outline of the game map
    rect( buffer, 0, 0, config->tile_size*config->map_width-1, config->tile_size*config->map_height-1, makecol( 0, 0, 255));

    draw_sprite( screen, buffer, 0, 0); //draw buffer image on screen every time we draw.
    release_screen();
}

static void wait_for_next_frame()
{
    // 16ms is supposed to be about 60fps but 8ms seems to match closest to what we see on other platforms
    rest(8); 
}

int main(void)
{
    game_initialize(MAX_MAP_HEIGHT, MAX_MAP_WIDTH, MAX_TILE_SIZE);
    
    while (is_running())
    {
        game_update();
        wait_for_next_frame();
    }

    game_shutdown();
    return 0;
}
END_OF_MAIN();