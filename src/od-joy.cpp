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

#include "vkbd.h"

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
    int x_axis, y_axis;
    int left = 0, right = 0, top = 0, bot = 0;
    int len, i, num;
    SDL_Joystick *joy = nr == 0 ? uae4all_joy0 : uae4all_joy1;

    if (emulated_mouse)
	return;

    *dir = 0;
    *button = 0;

    nr = (~nr)&0x1;

    SDL_JoystickUpdate ();
    struct joy_range *r = nr == 0 ? &dzone0 : &dzone1;

    if (mainMenu_usejoy)
    {
	x_axis = SDL_JoystickGetAxis (joy, 0);
	y_axis = SDL_JoystickGetAxis (joy, 1);

	if (x_axis < r->minx)
	left = 1;
	else if (x_axis > r->maxx)
	right = 1;

	if (y_axis < r->miny)
	top = 1;
	else if (y_axis > r->maxy)
	bot = 1;

	num = SDL_JoystickNumButtons (joy);
	if (num > 16)
	num = 16;
	for (i = 0; i < num; i++)
	*button |= (SDL_JoystickGetButton (joy, i) & 1) << i;
     }
    extern int emulated_left, emulated_right, emulated_top, emulated_bot, emulated_button1, emulated_button2, emulated_mouse_button1, emulated_mouse_button2;
    if (nr)
    {
	left|=emulated_left;
	right|=emulated_right;
	top|=emulated_top;
	bot|=emulated_bot;
	*button |= emulated_button1;
	if ((vkbd_button2==(SDLKey)0)&&(!vkbd_mode))
		top|=emulated_button2;
    }
    
    if(vkbd_mode && nr)
    {
    	if (left)
		vkbd_move |= VKBD_LEFT;
	else
	{
		vkbd_move &= ~VKBD_LEFT;
		if (right)
			vkbd_move |= VKBD_RIGHT;
		else
			vkbd_move &= ~VKBD_RIGHT;
	}
	if (top)
		vkbd_move |= VKBD_UP;
	else
	{
		vkbd_move &= ~VKBD_UP;
		if (bot)
			vkbd_move |= VKBD_DOWN;
		else
			vkbd_move &= ~VKBD_DOWN;
	}
	if ((*button)&1)
	{
		vkbd_move=VKBD_BUTTON;
		*button=0;
	}
	else if (emulated_button2)
		vkbd_move=VKBD_BUTTON2;
	else if (emulated_mouse_button1)
		vkbd_move=VKBD_BUTTON3;
	else if (emulated_mouse_button2)
		vkbd_move=VKBD_BUTTON4;
    }
    else
    {
    	if (left) top = !top;
    	if (right) bot = !bot;
    	*dir = bot | (right << 1) | (top << 8) | (left << 9);
    }
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
