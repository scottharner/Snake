#ifndef PLATFORM_H
#define PLATFORM_H

#include "../game/game.h"

void platform_initialize();
void platform_shutdown();
void platform_quit();
direction platform_get_input_direction();
void * platform_memory_allocate(unsigned int);
int platform_get_random(int);
void platform_set_random_seed(unsigned int);
void platform_adjust_speed(int);
void platform_update_platform_state();
void platform_draw_game_screen(int[MAP_HEIGHT][MAP_WIDTH], int);
void platform_draw_game_over_screen(int);

#endif