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
#include "../platform.h"

/*
*   Snake: Sega Saturn implementation of snake using joengine framework.
*   Last Update: Jul 22, 2026
*   Author: Scott Harner
*/

// Saturn implementation of platform initialization
void platform_initialize()
{

}

// Saturn implementation of platform shutdown
void platform_shutdown()
{

}

// any logic for making certain that graphics finish rendering at an appropriate speed
void platform_adjust_speed(int gamespeed)
{

}

// check if the game is still running
bool platform_is_running()
{
    return true;
}

// retrieve the input direction from the user
direction platform_get_input_direction()
{
    direction inputDirection = none;

    return inputDirection;
}

void platform_update_game_state()
{

}

void platform_draw_game_screen(int objMap[MAP_HEIGHT][MAP_WIDTH], int score)
{

}

void run_game_loop()
{

}

void jo_main(void)
{
    jo_core_init(JO_COLOR_Black);
    jo_printf(10, 0, "* Tiny Sonic Demo *");
    jo_core_add_callback(run_game_loop);
    jo_core_run();
    // game_run();
}

/*
** END OF FILE
*/