#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include "../game/game.h"

void platform_initialize();
void platform_shutdown();
input_type platform_get_input_type(mode, bool[INPUT_TYPE_COUNT]);
void * platform_memory_allocate(unsigned int);
int platform_get_random(int);
void platform_set_random_seed(unsigned int);
void platform_adjust_speed(speed, mode);
void platform_update_platform_state();
void platform_draw_game_screen(int[MAP_HEIGHT][MAP_WIDTH], int, bool);
bool platform_draw_game_over_screen(int, bool);
void platform_draw_title_screen(speed, bool);

#endif