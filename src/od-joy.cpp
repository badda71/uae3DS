 /* 
  * UAE - The Un*x Amiga Emulator
  * 
  * Joystick emulation for Linux and BSD. They share too much code to
  * split this file.
  * 
  * Copyright 1997 Bernd Schmidt
  * Copyright 1998 Krister Walfridsson
  */

#include "sysconfig.h"
#include "sysdeps.h"

#include "config.h"
#include "uae.h"
#include "options.h"
#include "memory.h"
#include "custom.h"
#include "joystick.h"
#include "SDL.h"

#define SDL_NumJoysticks(x) 0

#define DEADZONE_J0 500
#define DEADZONE_J1 500

extern int emulated_mouse, mainMenu_usejoy;
int nr_joysticks;

SDL_Joystick *uae4all_joy0, *uae4all_joy1;


#ifndef DREAMCAST
struct joy_range dzone0, dzone1;
#endif

void read_joystick(int nr, unsigned int *dir, int *button)
{
    int left = 0, right = 0, top = 0, bot = 0;

    *dir = 0;
    *button = 0;
    nr = (~nr)&0x1;

	extern int emulated_left, emulated_right, emulated_top, emulated_bot, emulated_button1, emulated_button2, emulated_mouse_button1, emulated_mouse_button2;
    if (nr)
    {
		left|=emulated_left;
		right|=emulated_right;
		top|=emulated_top;
		bot|=emulated_bot;
		*button |= emulated_button1;

		// map b-btn to joy up
		top|=emulated_button2;
    }
    
	if (left) top = !top;
	if (right) bot = !bot;
		*dir = bot | (right << 1) | (top << 8) | (left << 9);
}

void init_joystick(void)
{
    int i;
    nr_joysticks = SDL_NumJoysticks ();
    if (nr_joysticks > 0)
	uae4all_joy0 = SDL_JoystickOpen (0);
    if (nr_joysticks > 1)
	uae4all_joy1 = SDL_JoystickOpen (1);
    else
	uae4all_joy1 = NULL;

#ifndef DREAMCAST
    dzone0.minx = -DEADZONE_J0;
    dzone0.maxx = DEADZONE_J0;
    dzone0.miny = -DEADZONE_J0;
    dzone0.maxy = DEADZONE_J0;
    dzone1.minx = -DEADZONE_J1;
    dzone1.maxx = DEADZONE_J1;
    dzone1.miny = -DEADZONE_J1;
    dzone1.maxy = DEADZONE_J1;
#endif
}

void close_joystick(void)
{
    if (nr_joysticks > 0)
	SDL_JoystickClose (uae4all_joy0);
    if (nr_joysticks > 1)
	SDL_JoystickClose (uae4all_joy1);
}
