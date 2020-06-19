#include "sysconfig.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<3ds.h>

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
#include "uae3ds.h"

static const char *text_str_title="- UAE3DS v" VERSION3DS " by badda71";
static const char *text_str_load="Load disk image (X)";
static const char *text_str_save1="Load state (Y)";
static const char *text_str_save2="Save state";
static const char *text_str_throttle="Throttle";
static const char *text_str_frameskip="Frameskip";
static const char *text_str_autosave="Save disks";
static const char *text_str_vpos="Screen pos";
static const char *text_str_msens="Mouse sens";
static const char *text_str_keymap="Key mappings";
static const char *text_str_keymap1="Add";
static const char *text_str_keymap2="Delete";
static const char *text_str_keymap3="List";
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
static const char *text_str_on="on";
static const char *text_str_off="off";
static const char *text_str_separator="------------------------------";
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
	MAIN_MENU_ENTRY_SAVE_DISKS,
	MAIN_MENU_ENTRY_MOUSE_SENSITIVITY,
	MAIN_MENU_ENTRY_KEYMAP,
	MAIN_MENU_ENTRY_RESET_EMULATION,
	MAIN_MENU_ENTRY_RETURN_TO_EMULATION,
	MAIN_MENU_ENTRY_UPDATE,
	MAIN_MENU_ENTRY_EXIT_UAE,
	MAIN_MENU_ENTRY_COUNT, /* the number of entries to be shown */
};

int mainMenu_vpos=0;
int mainMenu_throttle=0;
int mainMenu_frameskip=-1;
int mainMenu_autosave=-1;
int mainMenu_msens=2;
int mainMenu_mappos=0;
int mainMenu_savepos=0;

int mainMenu_max_tap_time=250;
int mainMenu_click_time=100;
int mainMenu_single_tap_timeout=250;
int mainMenu_max_double_tap_time=250;
int mainMenu_locked_drag_timeout=5000;
int mainMenu_tap_and_drag_gesture=1;
int mainMenu_locked_drags=0;

static int save_ok=0;

static void draw_mainMenu(enum MainMenuEntry c)
{
	static int frame = 0;
	int flash = frame / 3;
	int row = 4*8, col = 10*8;
		
	int column = 0;

	text_draw_background();
	text_draw_window(72,28,260,184,text_str_title);

	if (c == MAIN_MENU_ENTRY_LOAD && flash)
		write_text_inv_pos(col, row, text_str_load);
	else
		write_text_pos(col, row, text_str_load);

	row+=8;
	write_text_pos(col, row, text_str_separator);
	row+=8;
	if (!save_ok) mainMenu_savepos=0;
	if (c == MAIN_MENU_ENTRY_SAVED_STATES && mainMenu_savepos==0 && flash)
		write_text_inv_pos(col, row, text_str_save1);
	else
		write_text_pos(col, row, text_str_save1);

	if (!save_ok) menu_set_text_color(menu_text_color_inactive);
	if (c == MAIN_MENU_ENTRY_SAVED_STATES && mainMenu_savepos==1 && flash)
		write_text_inv_pos(col+16*8, row, text_str_save2);
	else
		write_text_pos(col+16*8, row, text_str_save2);
	if (!save_ok) menu_restore_text_color();
	row+=8;
	write_text_pos(col, row, text_str_separator);
	row+=8;

	write_text_pos(col, row, text_str_throttle);
	column = col + 11 * 8;

	if ((mainMenu_throttle == 0) && (c != MAIN_MENU_ENTRY_THROTTLE || flash))
		write_text_inv_pos(column, row, text_str_0);
	else
		write_text_pos(column, row, text_str_0);
	column += 8*(strlen(text_str_0) + 1);
	if ((mainMenu_throttle == 1) && (c != MAIN_MENU_ENTRY_THROTTLE || flash))
		write_text_inv_pos(column, row, text_str_20);
	else
		write_text_pos(column, row, text_str_20);
	column += 8*(strlen(text_str_20) + 1);
	if ((mainMenu_throttle == 2) && (c != MAIN_MENU_ENTRY_THROTTLE || flash))
		write_text_inv_pos(column, row, text_str_40);
	else
		write_text_pos(column, row, text_str_40);
	column += 8*(strlen(text_str_40) + 1);
	if ((mainMenu_throttle == 3) && (c != MAIN_MENU_ENTRY_THROTTLE || flash))
		write_text_inv_pos(column, row, text_str_60);
	else
		write_text_pos(column, row, text_str_60);
	column += 8*(strlen(text_str_60) + 1);
	if ((mainMenu_throttle == 4) && (c != MAIN_MENU_ENTRY_THROTTLE || flash))
		write_text_inv_pos(column, row, text_str_80);
	else
		write_text_pos(column, row, text_str_80);
	column += 8*(strlen(text_str_80) + 1);
	if ((mainMenu_throttle == 5) && (c != MAIN_MENU_ENTRY_THROTTLE || flash))
		write_text_inv_pos(column, row, text_str_100);
	else
		write_text_pos(column, row, text_str_100);

	row += 12;

	write_text_pos(col, row, text_str_frameskip);
	column = col + 11*8;

	if ((mainMenu_frameskip == 0) && (c != MAIN_MENU_ENTRY_FRAMESKIP || flash))
		write_text_inv_pos(column, row, text_str_0);
	else
		write_text_pos(column, row, text_str_0);
	column += 8*(strlen(text_str_0) + 1);
	if ((mainMenu_frameskip == 1) && (c != MAIN_MENU_ENTRY_FRAMESKIP || flash))
		write_text_inv_pos(column, row, text_str_1);
	else
		write_text_pos(column, row, text_str_1);
	column += 8*(strlen(text_str_1) + 1);
	if ((mainMenu_frameskip == 2) && (c != MAIN_MENU_ENTRY_FRAMESKIP || flash))
		write_text_inv_pos(column, row, text_str_2);
	else
		write_text_pos(column, row, text_str_2);
	column += 8*(strlen(text_str_2) + 1);
	if ((mainMenu_frameskip == 3) && (c != MAIN_MENU_ENTRY_FRAMESKIP || flash))
		write_text_inv_pos(column, row, text_str_3);
	else
		write_text_pos(column, row, text_str_3);
	column += 8*(strlen(text_str_3) + 1);
	if ((mainMenu_frameskip == 4) && (c != MAIN_MENU_ENTRY_FRAMESKIP || flash))
		write_text_inv_pos(column, row, text_str_4);
	else
		write_text_pos(column, row, text_str_4);
	column += 8*(strlen(text_str_4) + 1);
	if ((mainMenu_frameskip == 5) && (c != MAIN_MENU_ENTRY_FRAMESKIP || flash))
		write_text_inv_pos(column, row, text_str_5);
	else
		write_text_pos(column, row, text_str_5);
	column += 8*(strlen(text_str_5) + 1);
	if ((mainMenu_frameskip == -1) && (c != MAIN_MENU_ENTRY_FRAMESKIP || flash))
		write_text_inv_pos(column, row, text_str_auto);
	else
		write_text_pos(column, row, text_str_auto);

	row += 12;

	write_text_pos(col, row, text_str_vpos);
	column = col+11*8;

	if ((mainMenu_vpos == 0) && (c != MAIN_MENU_ENTRY_SCREEN_POSITION || flash))
		write_text_inv_pos(column, row, text_str_0);
	else
		write_text_pos(column, row, text_str_0);
	column += 8*(strlen(text_str_0) + 1);
	if ((mainMenu_vpos == 1) && (c != MAIN_MENU_ENTRY_SCREEN_POSITION || flash))
		write_text_inv_pos(column, row, text_str_8);
	else
		write_text_pos(column, row, text_str_8);
	column += 8*(strlen(text_str_8) + 1);
	if ((mainMenu_vpos == 2) && (c != MAIN_MENU_ENTRY_SCREEN_POSITION || flash))
		write_text_inv_pos(column, row, text_str_16);
	else
		write_text_pos(column, row, text_str_16);
	column += 8*(strlen(text_str_16) + 1);
	if ((mainMenu_vpos == 3) && (c != MAIN_MENU_ENTRY_SCREEN_POSITION || flash))
		write_text_inv_pos(column, row, text_str_24);
	else
		write_text_pos(column, row, text_str_24);
	column += 8*(strlen(text_str_24) + 1);
	if ((mainMenu_vpos == 4) && (c != MAIN_MENU_ENTRY_SCREEN_POSITION || flash))
		write_text_inv_pos(column, row, text_str_32);
	else
		write_text_pos(column, row, text_str_32);
	column += 8*(strlen(text_str_32) + 1);
	if ((mainMenu_vpos == 5) && (c != MAIN_MENU_ENTRY_SCREEN_POSITION || flash))
		write_text_inv_pos(column, row, text_str_40);
	else
		write_text_pos(column, row, text_str_40);

	row += 12;

	write_text_pos(col, row, text_str_autosave);
	column = col+11*8;
	
	if (!mainMenu_autosave && (c != MAIN_MENU_ENTRY_SAVE_DISKS || flash))
		write_text_inv_pos(column, row, text_str_off);
	else
		write_text_pos(column, row, text_str_off);
	column += 8*(strlen(text_str_off) + 2);
	if (mainMenu_autosave && (c != MAIN_MENU_ENTRY_SAVE_DISKS || flash))
		write_text_inv_pos(column, row, text_str_on);
	else
		write_text_pos(column, row, text_str_on);

	row += 12;
	write_text_pos(col, row, text_str_msens);
	column = col+11*8;
	if ((mainMenu_msens== 1) && (c != MAIN_MENU_ENTRY_MOUSE_SENSITIVITY || flash))
		write_text_inv_pos(column, row, text_str_1);
	else
		write_text_pos(column, row, text_str_1);
	column += 8*(strlen(text_str_1) + 1);
	if ((mainMenu_msens== 2) && (c != MAIN_MENU_ENTRY_MOUSE_SENSITIVITY || flash))
		write_text_inv_pos(column, row, text_str_2);
	else
		write_text_pos(column, row, text_str_2);
	column += 8*(strlen(text_str_2) + 1);
	if ((mainMenu_msens== 3) && (c != MAIN_MENU_ENTRY_MOUSE_SENSITIVITY || flash))
		write_text_inv_pos(column, row, text_str_3);
	else
		write_text_pos(column, row, text_str_3);
	column += 8*(strlen(text_str_3) + 1);
	if ((mainMenu_msens== 4) && (c != MAIN_MENU_ENTRY_MOUSE_SENSITIVITY || flash))
		write_text_inv_pos(column, row, text_str_4);
	else
		write_text_pos(column, row, text_str_4);

	row+=12;
	write_text_pos(col, row, text_str_keymap);
	row+=12;
	column = col + 2*8;
	if (mainMenu_mappos == 0 && c == MAIN_MENU_ENTRY_KEYMAP && flash)
		write_text_inv_pos(column, row, text_str_keymap1);
	else
		write_text_pos(column, row, text_str_keymap1);
	column += 8*(strlen(text_str_keymap1) + 1);
	if (mainMenu_mappos == 1 && c == MAIN_MENU_ENTRY_KEYMAP && flash)
		write_text_inv_pos(column, row, text_str_keymap2);
	else
		write_text_pos(column, row, text_str_keymap2);
	column += 8*(strlen(text_str_keymap2) + 1);
	if (mainMenu_mappos == 2 && c == MAIN_MENU_ENTRY_KEYMAP && flash)
		write_text_inv_pos(column, row, text_str_keymap3);
	else
		write_text_pos(column, row, text_str_keymap3);
	column += 8*(strlen(text_str_keymap3) + 1);

	row+=8;
	write_text_pos(col, row, text_str_separator);
	row+=8;

	if (c == MAIN_MENU_ENTRY_RESET_EMULATION && flash)
		write_text_inv_pos(col, row, text_str_reset);
	else
		write_text_pos(col, row, text_str_reset);

	row += 12;
	if (c == MAIN_MENU_ENTRY_RETURN_TO_EMULATION && flash)
		write_text_inv_pos(col, row, text_str_return);
	else
		write_text_pos(col, row, text_str_return);
	
	row += 8;
	write_text_pos(col, row, text_str_separator);
	row += 8;
	if (c == MAIN_MENU_ENTRY_UPDATE && flash)
		write_text_inv_pos(col, row, text_str_update);
	else
		write_text_pos(col, row, text_str_update);

	row += 12;
	if (c == MAIN_MENU_ENTRY_EXIT_UAE && flash)
		write_text_inv_pos(col, row, text_str_exit);
	else
		write_text_pos(col, row, text_str_exit);

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
			if (cancel)
				return MAIN_MENU_ENTRY_RETURN_TO_EMULATION;
			else if (reset)
				return MAIN_MENU_ENTRY_RESET_EMULATION;
			else if (load)
				return MAIN_MENU_ENTRY_LOAD;
			else if (toStates) {
				mainMenu_savepos = 0;
				return MAIN_MENU_ENTRY_SAVED_STATES;
			} else if (up)
			{
				if (*sel > 0) *sel = (enum MainMenuEntry) ((*sel - 1) % MAIN_MENU_ENTRY_COUNT);
				else *sel = (enum MainMenuEntry) (MAIN_MENU_ENTRY_COUNT - 1);
			}
			else if (down)
			{
				*sel = (enum MainMenuEntry) ((*sel + 1) % MAIN_MENU_ENTRY_COUNT);
			}
			else
			{
				switch (*sel)
				{
					case MAIN_MENU_ENTRY_SAVED_STATES:
						if (left || right)
							mainMenu_savepos = save_ok ? ((mainMenu_savepos + 1) % 2) : 0;
						else if (activate)
							return *sel;
						break;
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
					case MAIN_MENU_ENTRY_SAVE_DISKS:
						if (left || right)
							mainMenu_autosave = ~mainMenu_autosave;
						break;
					case MAIN_MENU_ENTRY_MOUSE_SENSITIVITY:
						if (left)
							--mainMenu_msens;
						else if (right)
							++mainMenu_msens;
						mainMenu_msens = ((mainMenu_msens + 3) % 4) + 1; 
						break;
					case MAIN_MENU_ENTRY_KEYMAP:
						if (left)
							--mainMenu_mappos;
						else if (right)
							++mainMenu_mappos;
						else if (activate) {
							switch (mainMenu_mappos) {
							case 0:	// add
								uae3ds_mapping_add();
								break;
							case 1: // del
								uae3ds_mapping_del();
								break;
							default: // list
								uae3ds_mapping_list();
								break;
							}
						}
						mainMenu_mappos = (mainMenu_mappos + 3) % 3;
						break;
					case MAIN_MENU_ENTRY_LOAD:
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
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

#if defined(AUTO_RUN) || defined(AUTO_FRAMERATE) || defined(AUTO_PROFILER)
	return 1;
#else
static enum MainMenuEntry c = MAIN_MENU_ENTRY_LOAD;

	extern char uae4all_image_file[];

	while (1)
	{
		enum MainMenuEntry action = MAIN_MENU_ENTRY_NONE;
		save_ok = uae4all_image_file[0] ? 1 : 0;
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
				run_menuSave(mainMenu_savepos? MODE_SAVE : MODE_LOAD);
				if (savestate_state == STATE_DORESTORE || savestate_state == STATE_DOSAVE)
					return 1; /* leave, returning to the emulation */
#endif
				break;
			case MAIN_MENU_ENTRY_LOAD:
				run_menuDfSel();
				break;
			case MAIN_MENU_ENTRY_RESET_EMULATION:
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
	SDL_EnableKeyRepeat(0, 0);
}
