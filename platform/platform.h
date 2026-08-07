#ifndef PLATFORM_H
#define PLATFORM_H

#include "../common/types.h"
#include "../game/game.h"

void platform_initialize();
void platform_shutdown();
input_type platform_get_input_type(mode, bool[INPUT_TYPE_COUNT]);
void * platform_memory_allocate(unsigned int);
void platform_memory_free(void *);
int platform_get_random(int);
void platform_set_random_seed(unsigned int);
void platform_update_platform_state();
void platform_draw_game_screen(int *, int, bool, game_config *);
void platform_draw_game_over_screen(int, bool, loss_type);
void platform_draw_win_screen(int, bool);
void platform_draw_title_screen(speed, bool);
void platform_play_sound(sound_type);
void platform_reset(game_config *);

#endif