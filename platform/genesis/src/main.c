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
#define HALF_MAP_WIDTH 20
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
static u16 GAMEOVER_TILE_INDEX;
static u16 CREDITS_TILE_INDEX;
static u16 YOUWIN_TILE_INDEX;


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
    
    VDP_loadTileSet(gameover.tileset, tile_index, DMA);
    GAMEOVER_TILE_INDEX = tile_index;
    tile_index += gameover.tileset->numTile;
    
    VDP_loadTileSet(credits.tileset, tile_index, DMA);
    CREDITS_TILE_INDEX = tile_index;
    tile_index += credits.tileset->numTile;
    
    VDP_loadTileSet(youwin.tileset, tile_index, DMA);
    YOUWIN_TILE_INDEX = tile_index;
    tile_index += youwin.tileset->numTile;
    
    PAL_setPalette(PAL1, apple.palette->data, DMA); // setup foreground palette including a final index for yellow text
    PAL_setPalette(PAL2, gameover.palette->data, DMA); // minor titles palette
    previous_object_map = platform_memory_allocate(MAP_HEIGHT * MAP_WIDTH * sizeof(int));

    VDP_setTextPlane(BG_B); // draw text behind tiles

    XGM2_setFMVolume(45);
    XGM2_setPSGVolume(45);
}

void draw_tile(int x, int y, u16 current_tile_index, VDPPlane plane, bool flip_vertical, bool flip_horizontal)
{
    VDP_setTileMapXY(plane, TILE_ATTR_FULL(PAL1, FALSE, flip_vertical, flip_horizontal, current_tile_index), x, y);
}


// plays the requested sound effect
void platform_play_sound(sound_type current_sound_type)
{
    if (current_sound_type == SOUND_PICKUP)
    {
        XGM2_playPCM(sfx_pickup, sizeof(sfx_pickup), SOUND_PCM_CH2);
    }
    else if (current_sound_type == SOUND_LOSE)
    {
        XGM2_playPCM(sfx_lose, sizeof(sfx_lose), SOUND_PCM_CH2);
    }
}

static void clear_screen()
{
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);
}

static void print_centered_text(int y, const char *string)
{
    int length = strlen(string);
    int x = HALF_MAP_WIDTH - (length / 2);

    if (x < 0)
    {
        x = 0;
    }

    VDP_drawText(string, x, y);
}

// display a game over screen
void platform_draw_game_over_screen(int score, bool did_mode_change, loss_type current_loss_type)
{
    if (did_mode_change)
    {
        XGM2_stop();
        clear_screen();
    }

    VDP_setTextPalette(PAL0);
    
    VDP_drawImageEx(BG_A, &gameover, TILE_ATTR_FULL(PAL2, FALSE, FALSE, FALSE, GAMEOVER_TILE_INDEX), 0, 5, FALSE, TRUE);

    char format_string[32];
    char score_string[8];
    intToStr(score, score_string, 0);
    strcpy(format_string, "Score: ");
    strcat(format_string, score_string);
    print_centered_text(11, format_string);

    char reason_string[15];
    strcpy(reason_string, current_loss_type == LOSS_TYPE_SELF ? "Self Collision" : "Wall Collision");
    strcpy(format_string, "Reason: ");
    strcat(format_string, reason_string);
    print_centered_text(13, format_string);
}

// display a win screen
void platform_draw_win_screen(int score, bool did_mode_change)
{
    if (did_mode_change)
    {
        XGM2_stop();
        clear_screen();
    }

    VDP_setTextPalette(PAL0);

    VDP_drawImageEx(BG_A, &youwin, TILE_ATTR_FULL(PAL2, FALSE, FALSE, FALSE, YOUWIN_TILE_INDEX), 0, 5, FALSE, TRUE);

    char format_string[32];
    char score_string[8];
    intToStr(score, score_string, 0);
    strcpy(format_string, "Score: ");
    strcat(format_string, score_string);
    print_centered_text(11, format_string);
}

// display a credits screen
void platform_draw_credits_screen(bool did_mode_change)
{
    if (did_mode_change)
    {
        XGM2_stop(); // stop game music
        clear_screen();
    }

    VDP_drawImageEx(BG_A, &credits, TILE_ATTR_FULL(PAL2, FALSE, FALSE, FALSE, CREDITS_TILE_INDEX), 0, 0, FALSE, TRUE);

    print_centered_text(7, "Game Engineer - Stephen Bryant");
    print_centered_text(9, "Port Engineer - Scott Harner");
    print_centered_text(11, "QA Tester - Evan Harner");
    print_centered_text(13, "Music Composer - Safety Stoat Studios");
    print_centered_text(15, "Sound Effects Designer - Kronbits");
    print_centered_text(26, "Powered By - SGDK");
}

// calculate the color to display for a menu option
static int get_option_color(option selected_option, option display_option)
{
    int selected_color = RGB24_TO_VDPCOLOR(0xFFFF00);
    int default_color = RGB24_TO_VDPCOLOR(0xFFFFFF);
    return selected_option == display_option ? selected_color : default_color;
}

void platform_copy_string(char * buffer, char * source)
{
    strcpy(buffer, source);
}

// display a title screen
void platform_draw_title_screen(bool did_mode_change, option title_option)
{
    if (did_mode_change)
    {
        clear_screen();
        XGM2_play(bgm_title);
    }

    char speed_string[6];
    game_get_speed_string(speed_string);
    int white_color = RGB24_TO_VDPCOLOR(0xFFFFFF);
    
    VDP_setTextPalette(PAL0);
    VDP_drawText("SNAKE", 0, 1);

    if (get_option_color(title_option, OPTION_START) == white_color)
        VDP_setTextPalette(PAL0);
    else
        VDP_setTextPalette(PAL1);

    VDP_drawText("Start Game", 0, 5);

    if (get_option_color(title_option, OPTION_SPEED) == white_color)
        VDP_setTextPalette(PAL0);
    else
        VDP_setTextPalette(PAL1);

    char format_string[20];
    sprintf(format_string, "Speed: < %-6s >", speed_string);
    VDP_drawText(format_string, 0, 7);

    if (get_option_color(title_option, OPTION_CREDITS) == white_color)
        VDP_setTextPalette(PAL0);
    else
        VDP_setTextPalette(PAL1);

    VDP_drawText("Credits", 0, 9);

    // draw credits white
    VDP_setTextPalette(PAL0);
    VDP_drawText("Game (C) 2010 Stephen Bryant", 0, 24);
    VDP_drawText("Port (C) 2026 Scott Harner", 0, 26);
}

// platform specific setting of random generator seed
void platform_set_random_seed(unsigned int seed)
{
    setRandomSeed(seed);
}

// platform specific random number generation
int platform_get_random(int max)
{
    if (max <= 0) return 0;
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
        update_input_states(current_input_states, joy_read_value);
        switch(game_mode)
        {
            case MODE_GAME:
                if (game_input_pressed(INPUT_TYPE_START)) current_input = INPUT_TYPE_START;
                else if (joy_read_value & BUTTON_LEFT) current_input = INPUT_TYPE_LEFT;
                else if (joy_read_value & BUTTON_RIGHT) current_input = INPUT_TYPE_RIGHT;
                else if (joy_read_value & BUTTON_DOWN) current_input = INPUT_TYPE_DOWN;
                else if (joy_read_value & BUTTON_UP) current_input = INPUT_TYPE_UP;

                break;

            default:
                if (game_input_pressed(INPUT_TYPE_START)) current_input = INPUT_TYPE_START;    
                else if (game_input_pressed(INPUT_TYPE_DOWN)) current_input = INPUT_TYPE_DOWN;
                else if (game_input_pressed(INPUT_TYPE_UP)) current_input = INPUT_TYPE_UP;
                else if (game_input_pressed(INPUT_TYPE_LEFT)) current_input = INPUT_TYPE_LEFT;
                else if (game_input_pressed(INPUT_TYPE_RIGHT)) current_input = INPUT_TYPE_RIGHT;

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

static void draw_score(int score)
{
    char hud_string[32];
    char score_string[8];
    intToStr(score, score_string, 0);
    strcpy(hud_string, "Score: ");
    strcat(hud_string, score_string);
    VDP_clearTextArea(MAP_WIDTH-11, 1, 9, 1);
    VDP_drawText(hud_string, MAP_WIDTH-11, 1);
    old_score = score;
}

static void draw_high_score(int high_score)
{
    char hud_string[32];
    char score_string[8];
    intToStr(high_score, score_string, 0);
    strcpy(hud_string, "High Score: ");
    strcat(hud_string, score_string);
    VDP_clearTextArea(3, 1, 9, 1);
    VDP_drawText(hud_string, 3, 1);
}

void platform_draw_game_screen(int *object_map, int score, bool did_mode_change, game_config *config, int high_score)
{
    if (did_mode_change)
    {
        XGM2_stop();
        XGM2_play(bgm_game);
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

    if (score != old_score || did_mode_change)
    {
        draw_score(score);
        draw_high_score(high_score);
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