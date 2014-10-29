#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "menu.h"
#include "sysconfig.h"
#include "sysdeps.h"
#include "uae.h"
#include "options.h"
#include "sound.h"
#include "savestate.h"


extern int emulating;

static char *text_str_title="----- UAE4ALL rc3 ------";
static char *text_str_load="Select Image Disk (X)";
static char *text_str_save="SaveStates (Y)";
static char *text_str_throttle="Throttle ";
static char *text_str_frameskip="Frameskip";
static char *text_str_autosave="Save Disks";
static char *text_str_eject="Eject DF1";
static char *text_str_vpos="Screen Pos";
static char *text_str_8="8";
static char *text_str_16="16";
static char *text_str_20="20";
static char *text_str_24="24";
static char *text_str_32="32";
static char *text_str_40="40";
static char *text_str_60="60";
static char *text_str_80="80";
static char *text_str_100="100";
static char *text_str_0="0";
static char *text_str_1="1";
static char *text_str_2="2";
static char *text_str_3="3";
static char *text_str_4="4";
static char *text_str_5="5";
static char *text_str_auto="auto";
static char *text_str_sound="Sound";
static char *text_str_on="on";
static char *text_str_off="off";
static char *text_str_separator="------------------------------";
static char *text_str_reset="Reset (R)";
static char *text_str_run="Run (L)";
#ifdef DREAMCAST
static char *text_str_exit="Exit - Reboot Dreamcast";
#else
static char *text_str_exit="Exit";
#endif

enum MainMenuEntry {
	MAIN_MENU_ENTRY_NONE = -1 /* pseudo-entry */,
	MAIN_MENU_ENTRY_LOAD,
	MAIN_MENU_ENTRY_SAVED_STATES,
	MAIN_MENU_ENTRY_THROTTLE,
	MAIN_MENU_ENTRY_FRAMESKIP,
	MAIN_MENU_ENTRY_SCREEN_POSITION,
	MAIN_MENU_ENTRY_SOUND,
	MAIN_MENU_ENTRY_SAVE_DISKS,
	MAIN_MENU_ENTRY_EJECT_DF1,
	MAIN_MENU_ENTRY_RESET_EMULATION,
	MAIN_MENU_ENTRY_RESET_AND_RUN,
	MAIN_MENU_ENTRY_EXIT_UAE,
	MAIN_MENU_ENTRY_COUNT, /* the number of entries to be shown */
	MAIN_MENU_ENTRY_RETURN_TO_EMULATION,
};

int mainMenu_vpos=1;
#if !defined(DEBUG_UAE4ALL) && !defined(PROFILER_UAE4ALL) && !defined(AUTO_RUN) && !defined(AUTO_FRAMERATE)
int mainMenu_throttle=3;
int mainMenu_frameskip=-1;
#else
#ifdef PROFILER_UAE4ALL
#ifndef AUTO_PROFILER
int mainMenu_frameskip=0;
#else
int mainMenu_frameskip=-1;
#endif
#ifndef AUTO_PROFILER_THROTTLE
int mainMenu_throttle=0;
#else
int mainMenu_throttle=3;
#endif
#else
#ifdef DEBUG_FRAMERATE
int mainMenu_frameskip=-1;
#ifndef AUTO_FRAMERATE_THROTTLE
int mainMenu_throttle=0;
#else
int mainMenu_throttle=3;
#endif
#else
int mainMenu_throttle=0;
int mainMenu_frameskip=0;
#endif
#endif
#endif


#if !defined(DEBUG_UAE4ALL) && !defined(PROFILER_UAE4ALL) && !defined(AUTO_RUN) && !defined(AUTO_FRAMERATE)
int mainMenu_sound=-1;
#else
int mainMenu_sound=0;
#endif
int mainMenu_autosave=-1;

static void draw_mainMenu(enum MainMenuEntry c)
{
	static int frame = 0;
	int flash = frame / 3;
	int row = 3, column = 0;

	text_draw_background();
	text_draw_window(40,20,260,216,text_str_title);

	if (c == MAIN_MENU_ENTRY_LOAD && flash)
		write_text_inv(6, row++, text_str_load);
	else
		write_text(6, row++, text_str_load);

	write_text(6, row++, text_str_separator);
	write_text(6, row++, text_str_separator);
	
	if (c == MAIN_MENU_ENTRY_SAVED_STATES && flash)
		write_text_inv(6, row++, text_str_save);
	else
		write_text(6, row++, text_str_save);

	write_text(6, row, text_str_separator);
	row += 2;

	write_text(6, row, text_str_throttle);
	column = 17;

	if ((mainMenu_throttle == 0) && (c != MAIN_MENU_ENTRY_THROTTLE || flash))
		write_text_inv(column, row, text_str_0);
	else
		write_text(column, row, text_str_0);
	column += strlen(text_str_0) + 1;
	if ((mainMenu_throttle == 1) && (c != MAIN_MENU_ENTRY_THROTTLE || flash))
		write_text_inv(column, row, text_str_20);
	else
		write_text(column, row, text_str_20);
	column += strlen(text_str_20) + 1;
	if ((mainMenu_throttle == 2) && (c != MAIN_MENU_ENTRY_THROTTLE || flash))
		write_text_inv(column, row, text_str_40);
	else
		write_text(column, row, text_str_40);
	column += strlen(text_str_40) + 1;
	if ((mainMenu_throttle == 3) && (c != MAIN_MENU_ENTRY_THROTTLE || flash))
		write_text_inv(column, row, text_str_60);
	else
		write_text(column, row, text_str_60);
	column += strlen(text_str_60) + 1;
	if ((mainMenu_throttle == 4) && (c != MAIN_MENU_ENTRY_THROTTLE || flash))
		write_text_inv(column, row, text_str_80);
	else
		write_text(column, row, text_str_80);
	column += strlen(text_str_80) + 1;
	if ((mainMenu_throttle == 5) && (c != MAIN_MENU_ENTRY_THROTTLE || flash))
		write_text_inv(column, row, text_str_100);
	else
		write_text(column, row, text_str_100);

	row += 2;

	write_text(6, row, text_str_frameskip);
	column = 17;

	if ((mainMenu_frameskip == 0) && (c != MAIN_MENU_ENTRY_FRAMESKIP || flash))
		write_text_inv(column, row, text_str_0);
	else
		write_text(column, row, text_str_0);
	column += strlen(text_str_0) + 1;
	if ((mainMenu_frameskip == 1) && (c != MAIN_MENU_ENTRY_FRAMESKIP || flash))
		write_text_inv(column, row, text_str_1);
	else
		write_text(column, row, text_str_1);
	column += strlen(text_str_1) + 1;
	if ((mainMenu_frameskip == 2) && (c != MAIN_MENU_ENTRY_FRAMESKIP || flash))
		write_text_inv(column, row, text_str_2);
	else
		write_text(column, row, text_str_2);
	column += strlen(text_str_2) + 1;
	if ((mainMenu_frameskip == 3) && (c != MAIN_MENU_ENTRY_FRAMESKIP || flash))
		write_text_inv(column, row, text_str_3);
	else
		write_text(column, row, text_str_3);
	column += strlen(text_str_3) + 1;
	if ((mainMenu_frameskip == 4) && (c != MAIN_MENU_ENTRY_FRAMESKIP || flash))
		write_text_inv(column, row, text_str_4);
	else
		write_text(column, row, text_str_4);
	column += strlen(text_str_4) + 1;
	if ((mainMenu_frameskip == 5) && (c != MAIN_MENU_ENTRY_FRAMESKIP || flash))
		write_text_inv(column, row, text_str_5);
	else
		write_text(column, row, text_str_5);
	column += strlen(text_str_5) + 1;
	if ((mainMenu_frameskip == -1) && (c != MAIN_MENU_ENTRY_FRAMESKIP || flash))
		write_text_inv(column, row, text_str_auto);
	else
		write_text(column, row, text_str_auto);

	row += 2;

	write_text(6, row, text_str_vpos);
	column = 17;

	if ((mainMenu_vpos == 0) && (c != MAIN_MENU_ENTRY_SCREEN_POSITION || flash))
		write_text_inv(column, row, text_str_0);
	else
		write_text(column, row, text_str_0);
	column += strlen(text_str_0) + 1;
	if ((mainMenu_vpos == 1) && (c != MAIN_MENU_ENTRY_SCREEN_POSITION || flash))
		write_text_inv(column, row, text_str_8);
	else
		write_text(column, row, text_str_8);
	column += strlen(text_str_8) + 1;
	if ((mainMenu_vpos == 2) && (c != MAIN_MENU_ENTRY_SCREEN_POSITION || flash))
		write_text_inv(column, row, text_str_16);
	else
		write_text(column, row, text_str_16);
	column += strlen(text_str_16) + 1;
	if ((mainMenu_vpos == 3) && (c != MAIN_MENU_ENTRY_SCREEN_POSITION || flash))
		write_text_inv(column, row, text_str_24);
	else
		write_text(column, row, text_str_24);
	column += strlen(text_str_24) + 1;
	if ((mainMenu_vpos == 4) && (c != MAIN_MENU_ENTRY_SCREEN_POSITION || flash))
		write_text_inv(column, row, text_str_32);
	else
		write_text(column, row, text_str_32);
	column += strlen(text_str_32) + 1;
	if ((mainMenu_vpos == 5) && (c != MAIN_MENU_ENTRY_SCREEN_POSITION || flash))
		write_text_inv(column, row, text_str_40);
	else
		write_text(column, row, text_str_40);

	row += 2;

	write_text(6, row, text_str_sound);
	column = 17;

	if (!mainMenu_sound && (c != MAIN_MENU_ENTRY_SOUND || flash))
		write_text_inv(column, row, text_str_off);
	else
		write_text(column, row, text_str_off);
	column += strlen(text_str_off) + 2;
	if (mainMenu_sound && (c != MAIN_MENU_ENTRY_SOUND || flash))
		write_text_inv(column, row, text_str_on);
	else
		write_text(column, row, text_str_on);

	row += 2;

	write_text(6, row, text_str_autosave);
	column = 17;
	
	if (!mainMenu_autosave && (c != MAIN_MENU_ENTRY_SAVE_DISKS || flash))
		write_text_inv(column, row, text_str_off);
	else
		write_text(column, row, text_str_off);
	column += strlen(text_str_off) + 2;
	if (mainMenu_autosave && (c != MAIN_MENU_ENTRY_SAVE_DISKS || flash))
		write_text_inv(column, row, text_str_on);
	else
		write_text(column, row, text_str_on);

	row += 2;
	write_text(6, row++, text_str_separator);

	if (c == MAIN_MENU_ENTRY_EJECT_DF1 && flash)
		write_text_inv(6, row++, text_str_eject);
	else
		write_text(6, row++, text_str_eject);

	write_text(6, row++, text_str_separator);

	if (c == MAIN_MENU_ENTRY_RESET_EMULATION && flash)
		write_text_inv(6, row++, text_str_reset);
	else
		write_text(6, row++, text_str_reset);

	write_text(6, row++, text_str_separator);

	if (c == MAIN_MENU_ENTRY_RESET_AND_RUN && flash)
		write_text_inv(6, row++, text_str_run);
	else
		write_text(6, row++, text_str_run);

	write_text(6, row++, text_str_separator);
	write_text(6, row++, text_str_separator);

	if (c == MAIN_MENU_ENTRY_EXIT_UAE && flash)
		write_text_inv(6, row++, text_str_exit);
	else
		write_text(6, row++, text_str_exit);

	text_flip();
	frame = (frame + 1) % 6;
}

static enum MainMenuEntry key_mainMenu(enum MainMenuEntry *sel)
{
	SDL_Event event;

	while (SDL_PollEvent(&event) > 0)
	{
		int left = 0, right = 0, up = 0, down = 0,
		    activate = 0, cancel = 0, reset = 0, load = 0, toStates = 0, run = 0;
		if (event.type == SDL_QUIT)
			return MAIN_MENU_ENTRY_EXIT_UAE;
		else if (event.type == SDL_KEYDOWN)
		{
			uae4all_play_click();
			switch(event.key.keysym.sym)
			{
				case SDLK_d:
				case SDLK_RIGHT: right = 1; break;
				case SDLK_a:
				case SDLK_LEFT: left = 1; break;
				case SDLK_w:
				case SDLK_UP: up = 1; break;
				case SDLK_s:
				case SDLK_DOWN: down = 1; break;
#ifdef DREAMCAST
				case SDLK_c:
				case SDLK_LSHIFT: load = 1; break;
				case SDLK_x:
				case SDLK_SPACE: toStates = 1; break;
				case SDLK_1:
				case SDLK_BACKSPACE: run = 1; break;
				case SDLK_2:
				case SDLK_TAB: reset = 1; break;
#elif defined(GCW0)
				case SDLK_c:
				case SDLK_SPACE: toStates = 1; break;
				case SDLK_x:
				case SDLK_LSHIFT: load = 1; break;
				case SDLK_1:
				case SDLK_BACKSPACE: reset = 1; break;
				case SDLK_2:
				case SDLK_TAB: run = 1; break;
#else
				case SDLK_c:
				case SDLK_LSHIFT: toStates = 1; break;
				case SDLK_x:
				case SDLK_SPACE: load = 1; break;
				case SDLK_1:
				case SDLK_BACKSPACE: reset = 1; break;
				case SDLK_2:
				case SDLK_TAB: run = 1; break;
#endif
				case SDLK_z:
				case SDLK_RETURN:
				case SDLK_e:
				case SDLK_LCTRL: activate = 1; break;
				case SDLK_q:
				case SDLK_LALT: cancel = 1; break;
			}
			if (cancel)
				return MAIN_MENU_ENTRY_RETURN_TO_EMULATION;
			else if (reset)
				return MAIN_MENU_ENTRY_RESET_EMULATION;
			else if (load)
				return MAIN_MENU_ENTRY_LOAD;
			else if (toStates)
				return MAIN_MENU_ENTRY_SAVED_STATES;
			else if (run)
				return MAIN_MENU_ENTRY_RESET_AND_RUN;
			else if (up)
			{
				if (*sel > 0) *sel = (enum MainMenuEntry) ((*sel - 1) % MAIN_MENU_ENTRY_COUNT);
				else *sel = (enum MainMenuEntry) (MAIN_MENU_ENTRY_COUNT - 1);
			}
			else if (down)
				*sel = (enum MainMenuEntry) ((*sel + 1) % MAIN_MENU_ENTRY_COUNT);
			else
			{
				switch (*sel)
				{
					case MAIN_MENU_ENTRY_THROTTLE:
						if (left)
							mainMenu_throttle = (mainMenu_throttle > 0)
								? mainMenu_throttle - 1
								: 5;
						else if (right)
							mainMenu_throttle = (mainMenu_throttle + 1) % 6;
						break;
					case MAIN_MENU_ENTRY_FRAMESKIP:
						if (left)
							mainMenu_frameskip = (mainMenu_frameskip > -1)
								? mainMenu_frameskip - 1
								: 5;
						else if (right)
							mainMenu_frameskip = (mainMenu_frameskip < 5)
								? mainMenu_frameskip + 1
								: -1;
						break;
					case MAIN_MENU_ENTRY_SCREEN_POSITION:
						if (left)
							mainMenu_vpos = (mainMenu_vpos > 0)
								? mainMenu_vpos - 1
								: 5;
						else if (right)
							mainMenu_vpos = (mainMenu_vpos + 1) % 6;
						break;
					case MAIN_MENU_ENTRY_SOUND:
						if (left || right)
							mainMenu_sound = ~mainMenu_sound;
						break;
					case MAIN_MENU_ENTRY_SAVE_DISKS:
						if (left || right)
							mainMenu_autosave = ~mainMenu_autosave;
						break;
					case MAIN_MENU_ENTRY_LOAD:
					case MAIN_MENU_ENTRY_SAVED_STATES:
					case MAIN_MENU_ENTRY_EJECT_DF1:
					case MAIN_MENU_ENTRY_RESET_EMULATION:
					case MAIN_MENU_ENTRY_RESET_AND_RUN:
					case MAIN_MENU_ENTRY_EXIT_UAE:
						if (activate)
							return *sel;
						break;
				}
			}
		}
	}

	return MAIN_MENU_ENTRY_NONE;
}

static void clear_events(void)
{
	SDL_Event event;
	while (SDL_PollEvent(&event));
}

static void raise_mainMenu()
{
	int i;

	text_draw_background();
	text_flip();
	for(i=0;i<10;i+=2)
	{
		text_draw_background();
		text_draw_window(40,(10-i)*24,260,200,text_str_title);
		text_flip();
	}
	clear_events();
}

static void unraise_mainMenu()
{
	int i;

	for(i=9;i>=0;i-=2)
	{
		text_draw_background();
		text_draw_window(40,(10-i)*24,260,200,text_str_title);
		text_flip();
	}
	text_draw_background();
	text_flip();
	clear_events();
}

int run_mainMenu()
{
#if defined(AUTO_RUN) || defined(AUTO_FRAMERATE) || defined(AUTO_PROFILER)
	return 1;
#else
#if !defined(DEBUG_UAE4ALL) && !defined(PROFILER_UAE4ALL)
	static enum MainMenuEntry c = MAIN_MENU_ENTRY_LOAD;
#else
	static enum MainMenuEntry c = MAIN_MENU_ENTRY_SOUND;
#endif

	while (1)
	{
		enum MainMenuEntry action = MAIN_MENU_ENTRY_NONE;
		raise_mainMenu();
		while (action == MAIN_MENU_ENTRY_NONE)
		{
			draw_mainMenu(c);
			action = key_mainMenu(&c);
		}
		unraise_mainMenu();
		switch (action)
		{
			case MAIN_MENU_ENTRY_SAVED_STATES:
#ifndef NO_SAVE_MENU
				run_menuSave();
				if (savestate_state == STATE_DORESTORE || savestate_state == STATE_DOSAVE)
					return 1; /* leave, returning to the emulation */
#endif
				break;
			case MAIN_MENU_ENTRY_LOAD:
				run_menuLoad();
				break;
			case MAIN_MENU_ENTRY_EJECT_DF1:
				return 3; /* leave, ejecting the floppy disk in DF1 */
			case MAIN_MENU_ENTRY_RETURN_TO_EMULATION:
				if (emulating)
					return 1; /* leave, returning to the emulation */
				break;
			case MAIN_MENU_ENTRY_RESET_EMULATION:
				if (emulating)
					return 2; /* leave, resetting */
				/* Fall through */
			case MAIN_MENU_ENTRY_RESET_AND_RUN:
				return 1; /* leave, returning to the emulation */
			case MAIN_MENU_ENTRY_EXIT_UAE:
#ifdef DREAMCAST
//malloc(16*1024*1024);
				arch_reboot();
#else
				do_leave_program();
				exit(0);
#endif
				break;
		}
	}
#endif
}

