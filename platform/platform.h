#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include "../game/game.h"

void platform_initialize();
void platform_shutdown();
bool platform_is_running();
direction platform_get_input_direction();
void platform_adjust_speed(int);
void platform_update_game_state();
void platform_draw_game_screen(int[MAP_HEIGHT][MAP_WIDTH], int);

#endif