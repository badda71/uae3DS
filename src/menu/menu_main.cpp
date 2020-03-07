#include "sysconfig.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "menu.h"
#include "sysdeps.h"
#include "uae.h"
#include "options.h"
#include "sound.h"
#include "savestate.h"
#include "gui.h"
#include "uibottom.h"
#include "keyboard.h"
#include "update.h"

extern int emulating;

static const char *text_str_title="   -- UAE3DS v" VERSION3DS " ------";
static const char *text_str_load="Load disk image (X)";
static const char *text_str_save="Saved states (Y)";
static const char *text_str_throttle="Throttle";
static const char *text_str_frameskip="Frameskip";
static const char *text_str_autosave="Save disks";
static const char *text_str_vpos="Screen pos";
static const char *text_str_8="8";
static const char *text_str_16="16";
static const char *text_str_20="20";
static const char *text_str_24="24";
static const char *text_str_32="32";
static const char *text_str_40="40";
static const char *text_str_60="60";
static const char *text_str_80="80";
static const char *text_str_100="100";
static const char *text_str_0="0";
static const char *text_str_1="1";
static const char *text_str_2="2";
static const char *text_str_3="3";
static const char *text_str_4="4";
static const char *text_str_5="5";
static const char *text_str_auto="auto";
static const char *text_str_sound="Sound";
static const char *text_str_on="on";
static const char *text_str_off="off";
static const char *text_str_separator="------------------------------";
static const char *text_str_start="Start Amiga (R)";
static const char *text_str_reset="Reset Amiga (R)";
static const char *text_str_return="Return to Amiga (B)";
static const char *text_str_update="Check for Updates";
static const char *text_str_exit="Exit UAE3DS";

enum MainMenuEntry {
	MAIN_MENU_ENTRY_NONE = -1 /* pseudo-entry */,
	MAIN_MENU_ENTRY_LOAD,
	MAIN_MENU_ENTRY_SAVED_STATES,
	MAIN_MENU_ENTRY_THROTTLE,
	MAIN_MENU_ENTRY_FRAMESKIP,
	MAIN_MENU_ENTRY_SCREEN_POSITION,
	MAIN_MENU_ENTRY_SOUND,
	MAIN_MENU_ENTRY_SAVE_DISKS,
	MAIN_MENU_ENTRY_RESET_EMULATION,
	MAIN_MENU_ENTRY_RETURN_TO_EMULATION,
	MAIN_MENU_ENTRY_UPDATE,
	MAIN_MENU_ENTRY_EXIT_UAE,
	MAIN_MENU_ENTRY_COUNT, /* the number of entries to be shown */
};

int mainMenu_vpos=0;
int mainMenu_throttle=0;
int mainMenu_frameskip=-1;
int mainMenu_sound=-1;
int mainMenu_autosave=-1;

static void draw_mainMenu(enum MainMenuEntry c)
{
	static int frame = 0;
	int flash = frame / 3;
	int row = 4, col = 10;
		
	int column = 0;

	text_draw_background();
	text_draw_window(72,28,260,192,text_str_title);

	if (c == MAIN_MENU_ENTRY_LOAD && flash)
		write_text_inv(col, row++, text_str_load);
	else
		write_text(col, row++, text_str_load);

	write_text(col, row++, text_str_separator);

	if(emulating)
	{
		if (c == MAIN_MENU_ENTRY_SAVED_STATES && flash)
			write_text_inv(col, row++, text_str_save);
		else
			write_text(col, row++, text_str_save);
	}
	else
	{
		row++;
	}

	write_text(col, row, text_str_separator);
	row++;

	write_text(col, row, text_str_throttle);
	column = col + 11;

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

	write_text(col, row, text_str_frameskip);
	column = col + 11;

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

	write_text(col, row, text_str_vpos);
	column = col+11;

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

	write_text(col, row, text_str_sound);
	column = col+11;

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

	write_text(col, row, text_str_autosave);
	column = col+11;
	
	if (!mainMenu_autosave && (c != MAIN_MENU_ENTRY_SAVE_DISKS || flash))
		write_text_inv(column, row, text_str_off);
	else
		write_text(column, row, text_str_off);
	column += strlen(text_str_off) + 2;
	if (mainMenu_autosave && (c != MAIN_MENU_ENTRY_SAVE_DISKS || flash))
		write_text_inv(column, row, text_str_on);
	else
		write_text(column, row, text_str_on);

	row++;
	write_text(col, row++, text_str_separator);

	if (c == MAIN_MENU_ENTRY_RESET_EMULATION && flash)
		if(emulating)
			write_text_inv(col, row, text_str_reset);
		else
			write_text_inv(col, row, text_str_start);
	else
		if(emulating)
			write_text(col, row, text_str_reset);
		else
			write_text(col, row, text_str_start);

	row += 2;

	if(emulating)
	{
		if (c == MAIN_MENU_ENTRY_RETURN_TO_EMULATION && flash)
			write_text_inv(col, row++, text_str_return);
		else
			write_text(col, row++, text_str_return);
	}
	else
	{
		row++;
	}

	write_text(col, row++, text_str_separator);
	if (c == MAIN_MENU_ENTRY_UPDATE && flash)
		write_text_inv(col, row++, text_str_update);
	else
		write_text(col, row++, text_str_update);
	row++;

	if (c == MAIN_MENU_ENTRY_EXIT_UAE && flash)
		write_text_inv(col, row++, text_str_exit);
	else
		write_text(col, row++, text_str_exit);

	text_flip();
	frame = (frame + 1) % 6;
}

static enum MainMenuEntry key_mainMenu(enum MainMenuEntry *sel)
{
	SDL_Event event;

	while (SDL_PollEvent(&event) > 0)
	{
		if (uib_handle_event(&event)) continue;

		int left = 0, right = 0, up = 0, down = 0,
		    activate = 0, cancel = 0, reset = 0, load = 0, toStates = 0;
		if (event.type == SDL_QUIT)
			return MAIN_MENU_ENTRY_EXIT_UAE;
		else if (event.type == SDL_KEYDOWN)
		{
			uae4all_play_click();
			switch(event.key.keysym.sym)
			{
				case DS_RIGHT1:
				case DS_RIGHT2:
				case DS_RIGHT3:
				case AK_RT: right = 1; break;
				case DS_LEFT1:
				case DS_LEFT2:
				case DS_LEFT3:
				case AK_LF: left = 1; break;
				case DS_UP1:
				case DS_UP2:
				case DS_UP3:
				case AK_UP: up = 1; break;
				case DS_DOWN1:
				case DS_DOWN2:
				case DS_DOWN3:
				case AK_DN: down = 1; break;
				case AK_Y:
				case DS_Y: toStates = 1; break;
				case AK_X:
				case DS_X: load = 1; break;
				case DS_R:
				case AK_R: reset = 1; break;
				case AK_RET:
				case AK_SPC:
				case DS_START:
				case DS_A: activate = 1; break;
				case AK_ESC:
				case DS_B: cancel = 1; break;
			}
			if (cancel && emulating)
				return MAIN_MENU_ENTRY_RETURN_TO_EMULATION;
			else if (reset)
				return MAIN_MENU_ENTRY_RESET_EMULATION;
			else if (load)
				return MAIN_MENU_ENTRY_LOAD;
			else if (toStates && emulating)
				return MAIN_MENU_ENTRY_SAVED_STATES;
			else if (up)
			{
				if (*sel > 0) *sel = (enum MainMenuEntry) ((*sel - 1) % MAIN_MENU_ENTRY_COUNT);
				else *sel = (enum MainMenuEntry) (MAIN_MENU_ENTRY_COUNT - 1);

				if(!emulating && (*sel == MAIN_MENU_ENTRY_SAVED_STATES || *sel == MAIN_MENU_ENTRY_RETURN_TO_EMULATION))
				{
					if (*sel > 0) *sel = (enum MainMenuEntry) ((*sel - 1) % MAIN_MENU_ENTRY_COUNT);
					else *sel = (enum MainMenuEntry) (MAIN_MENU_ENTRY_COUNT - 1);
				}
			}
			else if (down)
			{
				*sel = (enum MainMenuEntry) ((*sel + 1) % MAIN_MENU_ENTRY_COUNT);

				if(!emulating && (*sel == MAIN_MENU_ENTRY_SAVED_STATES || *sel == MAIN_MENU_ENTRY_RETURN_TO_EMULATION))
				{
					*sel = (enum MainMenuEntry) ((*sel + 1) % MAIN_MENU_ENTRY_COUNT);
				}
			}
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
					case MAIN_MENU_ENTRY_RESET_EMULATION:
					case MAIN_MENU_ENTRY_RETURN_TO_EMULATION:
					case MAIN_MENU_ENTRY_UPDATE:
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
		text_draw_window(72,(10-i)*24,260,200,text_str_title);
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
		text_draw_window(72,(10-i)*24,260,200,text_str_title);
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
static enum MainMenuEntry c = MAIN_MENU_ENTRY_LOAD;

	while (1)
	{
		enum MainMenuEntry action = MAIN_MENU_ENTRY_NONE;
		raise_mainMenu();
		while (action == MAIN_MENU_ENTRY_NONE)
		{
			draw_mainMenu(c);
			SDL_Delay(10);
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
				run_menuDfSel();
				break;
			case MAIN_MENU_ENTRY_RESET_EMULATION:
				if (emulating)
					return 2; /* leave, resetting */
				/* Fall through */
			case MAIN_MENU_ENTRY_RETURN_TO_EMULATION:
				return 1; /* leave, returning to the emulation */
			case MAIN_MENU_ENTRY_UPDATE:
				if (!check_update()) break;
				/* Fall through */
			case MAIN_MENU_ENTRY_EXIT_UAE:
				storeConfig();
				do_leave_program();
				exit(0);
				break;
		}
	}
#endif
}

