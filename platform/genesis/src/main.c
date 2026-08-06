#include "genesis.h"
#include "resources.h"
#include "../../platform.h"

/*
*   Snake: Sega Genesis implementation of snake using sgdk framework.
*   Last Update: Aug 5, 2026
*   Author: Scott Harner
*/

#define MAP_HEIGHT 28
#define MAP_WIDTH 40
#define MAP_TILE_SIZE 8
#define SFX_LOSE 64
#define SFX_PICKUP 65

static int frame_counter;
static u16 tile_index = TILE_USER_INDEX;
static int old_score = -1;
static int *previous_object_map;//second copy of object make to track previous vs current values
static u16 APPLE_TILE_INDEX;
static u16 SNAKE_TILE_INDEX;
static u16 BORDERC_TILE_INDEX;
static u16 BORDERL_TILE_INDEX;
static u16 BORDERT_TILE_INDEX;


// Genesis implementation of platform initialization
void platform_initialize()
{
    VDP_init();
    JOY_init();
    SPR_init();
    
    VDP_loadTileSet(apple.tileset, tile_index, DMA);
    APPLE_TILE_INDEX = tile_index;
    tile_index += apple.tileset->numTile;

    VDP_loadTileSet(snake.tileset, tile_index, DMA);
    SNAKE_TILE_INDEX = tile_index;
    tile_index += snake.tileset->numTile;
    
    VDP_loadTileSet(borderc.tileset, tile_index, DMA);
    BORDERC_TILE_INDEX = tile_index;
    tile_index += borderc.tileset->numTile;
    
    VDP_loadTileSet(borderl.tileset, tile_index, DMA);
    BORDERL_TILE_INDEX = tile_index;
    tile_index += borderl.tileset->numTile;
    
    VDP_loadTileSet(bordert.tileset, tile_index, DMA);
    BORDERT_TILE_INDEX = tile_index;
    tile_index += bordert.tileset->numTile;
    
    PAL_setPalette(PAL1, apple.palette->data, DMA); // setup foreground palette including a final index for yellow text
    previous_object_map = platform_memory_allocate(MAP_HEIGHT * MAP_WIDTH * sizeof(int));

    VDP_setTextPlane(BG_B); // draw text behind tiles

    XGM_setPCM(SFX_LOSE, sfx_lose, sizeof(sfx_lose));
    XGM_setPCM(SFX_PICKUP, sfx_pickup, sizeof(sfx_pickup));
}

// plays the requested sound effect
void platform_play_sound(sound_type current_sound_type)
{
    if (current_sound_type == SOUND_PICKUP)
    {
        XGM_startPlayPCM(SFX_PICKUP,1,SOUND_PCM_CH2);
    }
    else if (current_sound_type == SOUND_LOSE)
    {
        XGM_startPlayPCM(SFX_LOSE,1,SOUND_PCM_CH2);
    }
}

static void clear_screen()
{
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);
}

// display a game over screen
void platform_draw_game_over_screen(int score, bool did_mode_change, loss_type current_loss_type)
{
    if (did_mode_change)
        clear_screen();

    VDP_setTextPalette(PAL0);
    VDP_drawText("Game Over", 5, 5);

    char format_string[32];
    char score_string[8];
    intToStr(score, score_string, 6);
    strcpy(format_string, "Score: ");
    strcat(format_string, score_string);
    VDP_drawText(format_string, 5, 7);

    char reason_string[15];
    strcpy(reason_string, current_loss_type == LOSS_TYPE_SELF ? "Self Collision" : "Wall Collision");
    strcpy(format_string, "Reason: ");
    strcat(format_string, reason_string);
    VDP_drawText(format_string, 5, 9);
}

// display a win screen
void platform_draw_win_screen(int score, bool did_mode_change)
{
    if (did_mode_change)
        clear_screen();

    VDP_setTextPalette(PAL0);
    VDP_drawText("You Win!", 5, 5);

    char format_string[32];
    char score_string[8];
    intToStr(score, score_string, 6);
    strcpy(format_string, "Score: ");
    strcat(format_string, score_string);
    VDP_drawText(format_string, 5, 7);
}

// calculate the color to display for a menu option
static int get_option_color(speed selected_speed, speed option_speed)
{
    int selected_color = RGB24_TO_VDPCOLOR(0xFFFF00);
    int default_color = RGB24_TO_VDPCOLOR(0xFFFFFF);
    return selected_speed == option_speed ? selected_color : default_color;
}

// display a title screen
void platform_draw_title_screen(speed game_speed, bool did_mode_change)
{
    if (did_mode_change)
        clear_screen();

    int white_color = RGB24_TO_VDPCOLOR(0xFFFFFF);
    
    VDP_setTextPalette(PAL0);
    VDP_drawText("SNAKE", 0, 1);

    if (get_option_color(game_speed, SPEED_SLOW) == white_color)
        VDP_setTextPalette(PAL0);
    else
        VDP_setTextPalette(PAL1);

    VDP_drawText("Slow", 0, 5);

    if (get_option_color(game_speed, SPEED_MEDIUM) == white_color)
        VDP_setTextPalette(PAL0);
    else
        VDP_setTextPalette(PAL1);

    VDP_drawText("Medium", 0, 7);

    if (get_option_color(game_speed, SPEED_FAST) == white_color)
        VDP_setTextPalette(PAL0);
    else
        VDP_setTextPalette(PAL1);

    VDP_drawText("Fast", 0, 9);
}

// platform specific setting of random generator seed
void platform_set_random_seed(unsigned int seed)
{
    setRandomSeed(seed);
}

// platform specific random number generation
int platform_get_random(int max)
{
    return random() % max;
}

// platform specific memory allocation
void * platform_memory_allocate(unsigned int size)
{
    return MEM_alloc(size);
}

// platform specific memory free
void platform_memory_free(void *pointer)
{
    MEM_free(pointer);
}

// steps to prepare to exit the game
void platform_shutdown()
{
    if (previous_object_map != NULL)
    {
        platform_memory_free(previous_object_map);
        previous_object_map = NULL;
    }
}

// perform platform specific actions when the game resets
void platform_reset(game_config *config)
{
    int i, j; //this is so we don't get errors because the object map references nothing.
    for (i = 0; i < config->map_height; i++)
    {
        for (j = 0; j < config->map_width; j++)
        {
            previous_object_map[i * config->map_width + j] = OBJECT_NOTHING;
        }
    }
}

// track all current and previous input states so we can check on input presses
static void update_input_states(bool current_input_states[INPUT_TYPE_COUNT], u16 joy_read_value)
{
    game_save_previous_inputstates();

    // read current state
    current_input_states[INPUT_TYPE_UP] = (joy_read_value & BUTTON_UP);
    current_input_states[INPUT_TYPE_DOWN] = (joy_read_value & BUTTON_DOWN);
    current_input_states[INPUT_TYPE_LEFT] = (joy_read_value & BUTTON_LEFT);
    current_input_states[INPUT_TYPE_RIGHT] = (joy_read_value & BUTTON_RIGHT);
    current_input_states[INPUT_TYPE_START] = (joy_read_value & BUTTON_START);
}

// retrieve the input type from the user
input_type platform_get_input_type(mode game_mode, bool current_input_states[INPUT_TYPE_COUNT])
{
    input_type current_input = INPUT_TYPE_NOTHING;
    u8 joy_type = JOY_getJoypadType(JOY_1);
    if (joy_type == JOY_TYPE_PAD3 || joy_type == JOY_TYPE_PAD6)
    {
        u16 joy_read_value = JOY_readJoypad(JOY_1);
        switch(game_mode)
        {
            case MODE_TITLE:
                update_input_states(current_input_states, joy_read_value);
                if (game_input_pressed(INPUT_TYPE_START)) current_input = INPUT_TYPE_START;    
                else if (game_input_pressed(INPUT_TYPE_DOWN)) current_input = INPUT_TYPE_DOWN;
                else if (game_input_pressed(INPUT_TYPE_UP)) current_input = INPUT_TYPE_UP;

                break;

            default:
                if (joy_read_value & BUTTON_LEFT) current_input = INPUT_TYPE_LEFT;
                else if (joy_read_value & BUTTON_RIGHT) current_input = INPUT_TYPE_RIGHT;
                else if (joy_read_value & BUTTON_DOWN) current_input = INPUT_TYPE_DOWN;
                else if (joy_read_value & BUTTON_UP) current_input = INPUT_TYPE_UP;
                else if (joy_read_value & BUTTON_START) current_input = INPUT_TYPE_START;

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

void draw_tile(int x, int y, u16 current_tile_index, VDPPlane plane, bool flip_vertical, bool flip_horizontal)
{
    VDP_setTileMapXY(plane, TILE_ATTR_FULL(PAL1, FALSE, flip_vertical, flip_horizontal, current_tile_index), x, y);
}

void clear_tile(int x, int y, VDPPlane plane)
{
    VDP_setTileMapXY(plane, 0, x, y);
}

static void draw_border(game_config *config)
{
    for (int i = 1; i < config->map_width-1; i++)
    {
        // top
        draw_tile(i, 0, BORDERT_TILE_INDEX, BG_B, false, false);
    }

    for (int i = 1; i < config->map_width-1; i++)
    {
        // bottom
        draw_tile(i, config->map_height-1, BORDERT_TILE_INDEX, BG_B, true, false);
    }

    for (int i = 1; i < config->map_height-1; i++)
    {
        // left
        draw_tile(0, i, BORDERL_TILE_INDEX, BG_B, false, false);
    }

    for (int i = 1; i < config->map_height-1; i++)
    {
        // right
        draw_tile(config->map_width-1, i, BORDERL_TILE_INDEX, BG_B, false, true);
    }

    // upper left
    draw_tile(0, 0, BORDERC_TILE_INDEX, BG_B, false, false);
    
    // upper right
    draw_tile(config->map_width-1, 0, BORDERC_TILE_INDEX, BG_B, false, true);

    // lower left
    draw_tile(0, config->map_height-1, BORDERC_TILE_INDEX, BG_B, true, false);

    // lower right
    draw_tile(config->map_width-1, config->map_height-1, BORDERC_TILE_INDEX, BG_B, true, true);
}

void platform_draw_game_screen(int *object_map, int score, bool did_mode_change, game_config *config)
{
    if (did_mode_change)
    {
        VDP_setTextPalette(PAL0);
        clear_screen();
        draw_border(config);
    }

    int i,j;
    for (i = 0; i < config->map_height; i++)
    {
        int row = i * config->map_width;
        for (j = 0; j < config->map_width; j++)
        {
            int map_index = row + j;
            // only draw tiles when they change
            if (object_map[map_index] != previous_object_map[map_index])
            {
                previous_object_map[map_index] = object_map[map_index];
                if (object_map[map_index] == OBJECT_APPLE)
                {
                    draw_tile(j, i, APPLE_TILE_INDEX, BG_A, false, false);
                }
                else if (object_map[map_index] == OBJECT_SNAKE)
                {
                    draw_tile(j, i, SNAKE_TILE_INDEX, BG_A, false, false);
                }
                else
                {
                    clear_tile(j, i, BG_A);
                }
            }
        }
    }

    //draw the score
    if (score != old_score)
    {
        char hud_string[32];
        char score_string[8];
        intToStr(score, score_string, 6);
        strcpy(hud_string, "Score: ");
        strcat(hud_string, score_string);
        VDP_clearTextArea(MAP_WIDTH-14, 1, 12, 1);
        VDP_drawText(hud_string, MAP_WIDTH-14, 1);
        old_score = score;
    }
}

int main(bool hardReset)
{
    game_initialize(MAP_HEIGHT, MAP_WIDTH, MAP_TILE_SIZE);
    
    while(TRUE)
    {
        frame_counter++;
        if (frame_counter >= 60)
        {
            // check on gamepad availability about once per second to see if it changed
            frame_counter = 0;
            JOY_reset();
        }

        game_update();

        // always call this method at the end of the frame
        SYS_doVBlankProcess();
    }

    game_shutdown();
    return 0;
}

/*
** END OF FILE
*/