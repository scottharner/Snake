#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include "../game/game.h"

void platform_initialize();
void platform_shutdown();
bool platform_is_running();
direction platform_get_input_direction();
void platform_adjust_speed(int);

#endif