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

int nr_joysticks;

SDL_Joystick *uae4all_joy0, *uae4all_joy1;


#ifndef DREAMCAST
struct joy_range
{
    int minx, maxx, miny, maxy;
} range0, range1;
#endif

void read_joystick(int nr, unsigned int *dir, int *button)
{
#if !defined(MAX_AUTOEVENTS) && !defined(AUTOEVENTS)
    int x_axis, y_axis;
    int left = 0, right = 0, top = 0, bot = 0;
    int len, i, num;
    SDL_Joystick *joy = nr == 0 ? uae4all_joy0 : uae4all_joy1;

    *dir = 0;
    *button = 0;

    nr = (~nr)&0x1;

    SDL_JoystickUpdate ();
#ifndef DREAMCAST
    struct joy_range *r = nr == 0 ? &range0 : &range1;
    x_axis = SDL_JoystickGetAxis (joy, 0);
    y_axis = SDL_JoystickGetAxis (joy, 1);

    if (x_axis < r->minx) r->minx = x_axis;
    if (y_axis < r->miny) r->miny = y_axis;
    if (x_axis > r->maxx) r->maxx = x_axis;
    if (y_axis > r->maxy) r->maxy = y_axis;
    
    if (x_axis < (r->minx + (r->maxx - r->minx)/3))
    	left = 1;
    else if (x_axis > (r->minx + 2*(r->maxx - r->minx)/3))
    	right = 1;

    if (y_axis < (r->miny + (r->maxy - r->miny)/3))
    	top = 1;
    else if (y_axis > (r->miny + 2*(r->maxy - r->miny)/3))
    	bot = 1;

    num = SDL_JoystickNumButtons (joy);
    if (num > 16)
	num = 16;
    for (i = 0; i < num; i++)
	*button |= (SDL_JoystickGetButton (joy, i) & 1) << i;
#ifdef EMULATED_JOYSTICK
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
#endif
#else
    int hat=/*15^*/(SDL_JoystickGetHat(joy,0));
    if (hat & SDL_HAT_LEFT)
	    left = 1;
    else if (hat & SDL_HAT_RIGHT)
	    right = 1;
    if (hat & SDL_HAT_UP)
	    top = 1;
    else if (hat & SDL_HAT_DOWN)
	    bot = 1;
    if (vkbd_button2==(SDLKey)0)
    	top |= SDL_JoystickGetButton(joy,6) & 1;
    *button = SDL_JoystickGetButton(joy,2) & 1;
#endif
    
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
#if defined(EMULATED_JOYSTICK) || defined(DREAMCAST)
#ifndef DREAMCAST
	else if (emulated_button2)
#else
	else if (SDL_JoystickGetButton(joy,6)&1)
#endif
		vkbd_move=VKBD_BUTTON2;
#ifndef DREAMCAST
	else if (emulated_mouse_button1)
#else
 	else if (SDL_JoystickGetButton(joy,5)&1)
#endif
		vkbd_move=VKBD_BUTTON3;
#ifndef DREAMCAST
	else if (emulated_mouse_button2)
#else
	else if (SDL_JoystickGetButton(joy,1)&1)
#endif
		vkbd_move=VKBD_BUTTON4;
#endif
    }
    else
    {
    	if (left) top = !top;
    	if (right) bot = !bot;
    	*dir = bot | (right << 1) | (top << 8) | (left << 9);
    }
#endif
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
    range0.minx = INT_MAX;
    range0.maxx = INT_MIN;
    range0.miny = INT_MAX;
    range0.maxy = INT_MIN;
    range1.minx = INT_MAX;
    range1.maxx = INT_MIN;
    range1.miny = INT_MAX;
    range1.maxy = INT_MIN;
#endif
}

void close_joystick(void)
{
    if (nr_joysticks > 0)
	SDL_JoystickClose (uae4all_joy0);
    if (nr_joysticks > 1)
	SDL_JoystickClose (uae4all_joy1);
}
