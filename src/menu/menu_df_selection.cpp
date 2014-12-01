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
#include "disk.h"


extern int emulating;

static const char *text_str_title_df_menu="Select disk";
static const char *text_str_load_df0="Load DF0 image (X)";
static const char *text_str_load_df1="Load DF1 image (Y)";
static const char *text_str_eject_df0="Eject DF0";
static const char *text_str_eject_df1="Eject DF1";
static const char *text_str_back="Main Menu (B)";
static const char *text_str_separator="------------------------------";
static const char *text_str_df0="DF0:";
static const char *text_str_df1="DF1:";
static const char *text_str_empty="empty";

extern char uae4all_image_file[];
extern char uae4all_image_file2[];

static int scroll = 0;
static int scroll2 = 0;
static int pause_scroll_timer = 0;
static int pause_scroll_timer2 = 0;

enum DfMenuEntry {
	DF_MENU_ENTRY_NONE = -1 /* pseudo-entry */,
	DF_MENU_ENTRY_LOAD_DF0,
	DF_MENU_ENTRY_LOAD_DF1,
	DF_MENU_ENTRY_EJECT_DF0,
	DF_MENU_ENTRY_EJECT_DF1,
	DF_MENU_ENTRY_BACK,
	DF_MENU_ENTRY_COUNT, /* the number of entries to be shown */
};

int dfMenu_vpos=1;


static void draw_dfMenu(enum DfMenuEntry c)
{
	static int frame = 0;
	int flash = frame / 3;
	int row = 4, column = 0;
	char image_text[27];
	static int b = 0;
	int update_scroll = !(b%5);
	int visible_len = 26;

	text_draw_background();

	text_draw_window(40,28,260,192,text_str_title_df_menu);

	write_text(6, row, text_str_df0);
	if(uae4all_image_file[0])
	{
		int len = strlen(uae4all_image_file);

		if(len > visible_len)
		{
			if(!pause_scroll_timer)
			{
				if(update_scroll)
					scroll++;
				if(scroll >= len - visible_len)
					pause_scroll_timer = 60;
			}
			else
			{
				pause_scroll_timer--;

				if(!pause_scroll_timer)
				{
					if(scroll2 > 0)
						pause_scroll_timer = 1;
					else
						scroll = 0;
				}

				// Wait for scroll2
				if(!pause_scroll_timer && scroll2 > 0)
				{
					pause_scroll_timer = 1;
				}
			}
		}
		else
		{
			scroll = 0;
			pause_scroll_timer = 0;
		}

		strncpy(image_text, &uae4all_image_file[scroll], visible_len);
		image_text[26] = '\0';
		write_text(10, row++, image_text);
	}
	else
		write_text(10, row++, text_str_empty);
	row++;

	write_text(6, row, text_str_df1);
	if(uae4all_image_file2[0])
	{
		int len = strlen(uae4all_image_file2);

		if(len > visible_len)
		{
			if(!pause_scroll_timer2)
			{
				if(update_scroll)
					scroll2++;
				if(scroll2 >= len - visible_len)
					pause_scroll_timer2 = 60;
			}
			else
			{
				pause_scroll_timer2--;

				if(!pause_scroll_timer2)
				{
					// Wait for scroll
					if(scroll > 0)
						pause_scroll_timer = 1;
					else
						scroll2 = 0;
				}
			}
		}
		else
		{
			scroll2 = 0;
			pause_scroll_timer2 = 0;
		}

		strncpy(image_text, &uae4all_image_file2[scroll2], visible_len);
		image_text[26] = '\0';
		write_text(10, row++, image_text);
	}
	else
		write_text(10, row++, text_str_empty);

	row += 2;

	if (c == DF_MENU_ENTRY_LOAD_DF0 && flash)
		write_text_inv(6, row++, text_str_load_df0);
	else
		write_text(6, row++, text_str_load_df0);

	row++;

	if (c == DF_MENU_ENTRY_LOAD_DF1 && flash)
		write_text_inv(6, row++, text_str_load_df1);
	else
		write_text(6, row++, text_str_load_df1);

	write_text(6, row++, text_str_separator);

	if (c == DF_MENU_ENTRY_EJECT_DF0 && flash)
		write_text_inv(6, row++, text_str_eject_df0);
	else
		write_text(6, row++, text_str_eject_df0);

	row++;

	if (c == DF_MENU_ENTRY_EJECT_DF1 && flash)
		write_text_inv(6, row++, text_str_eject_df1);
	else
		write_text(6, row++, text_str_eject_df1);

	write_text(6, row++, text_str_separator);

	if (c == DF_MENU_ENTRY_BACK && flash)
		write_text_inv(6, row++, text_str_back);
	else
		write_text(6, row++, text_str_back);

	text_flip();
	frame = (frame + 1) % 6;

	if(pause_scroll_timer == 1 && pause_scroll_timer2 == 1)
	{
		pause_scroll_timer = 0;
		pause_scroll_timer2 = 0;
		scroll = 0;
		scroll2 = 0;
	}

	b++;
}

static enum DfMenuEntry key_dfMenu(enum DfMenuEntry *sel)
{
	SDL_Event event;

	while (SDL_PollEvent(&event) > 0)
	{
		int left = 0, right = 0, up = 0, down = 0,
		    activate = 0, cancel = 0, load_df0 = 0, load_df1 = 0;
		if (event.type == SDL_QUIT)
			return DF_MENU_ENTRY_BACK;
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
				case SDLK_LSHIFT: load_df0 = 1; break;
				case SDLK_x:
				case SDLK_SPACE: load_df1 = 1; break;
#elif defined(GCW0)
				case SDLK_c:
				case SDLK_SPACE: load_df1 = 1; break;
				case SDLK_x:
				case SDLK_LSHIFT: load_df0 = 1; break;
#else
				case SDLK_c:
				case SDLK_LSHIFT: load_df1 = 1; break;
				case SDLK_x:
				case SDLK_SPACE: load_df0 = 1; break;
#endif
				case SDLK_z:
				case SDLK_RETURN:
				case SDLK_e:
				case SDLK_LCTRL: activate = 1; break;
				case SDLK_q:
				case SDLK_LALT: cancel = 1; break;
			}
			if (cancel)
				return DF_MENU_ENTRY_BACK;
			else if (load_df0)
				return DF_MENU_ENTRY_LOAD_DF0;
			else if (load_df1)
				return DF_MENU_ENTRY_LOAD_DF1;
			else if (up)
			{
				if (*sel > 0) *sel = (enum DfMenuEntry) ((*sel - 1) % DF_MENU_ENTRY_COUNT);
				else *sel = (enum DfMenuEntry) (DF_MENU_ENTRY_COUNT - 1);
			}
			else if (down)
				*sel = (enum DfMenuEntry) ((*sel + 1) % DF_MENU_ENTRY_COUNT);
			else
			{
				switch (*sel)
				{
					case DF_MENU_ENTRY_LOAD_DF0:
					case DF_MENU_ENTRY_LOAD_DF1:
					case DF_MENU_ENTRY_EJECT_DF0:
					case DF_MENU_ENTRY_EJECT_DF1:
					case DF_MENU_ENTRY_BACK:
						if (activate)
							return *sel;
						break;
				}
			}
		}
	}

	return DF_MENU_ENTRY_NONE;
}

static void clear_events(void)
{
	SDL_Event event;
	while (SDL_PollEvent(&event));
}

static void raise_dfMenu()
{
	int i;

	text_draw_background();
	text_flip();
	for(i=0;i<10;i+=2)
	{
		text_draw_background();
		text_draw_window(40,(10-i)*24,260,200,text_str_title_df_menu);
		text_flip();
	}
	clear_events();
}

static void unraise_dfMenu()
{
	int i;

	for(i=9;i>=0;i-=2)
	{
		text_draw_background();
		text_draw_window(40,(10-i)*24,260,200,text_str_title_df_menu);
		text_flip();
	}
	text_draw_background();
	text_flip();
	clear_events();
}

int run_menuDfSel()
{
#if defined(AUTO_RUN) || defined(AUTO_FRAMERATE) || defined(AUTO_PROFILER)
	return 1;
#else
	// Reset df file name scroll variables
	scroll = 0;
	scroll2 = 0;
	pause_scroll_timer = 0;
	pause_scroll_timer2 = 0;

	static enum DfMenuEntry c = DF_MENU_ENTRY_LOAD_DF0;

	while (1)
	{
		enum DfMenuEntry action = DF_MENU_ENTRY_NONE;
		raise_dfMenu();
		while (action == DF_MENU_ENTRY_NONE)
		{
			draw_dfMenu(c);
			action = key_dfMenu(&c);
		}
		unraise_dfMenu();
		switch (action)
		{
			case DF_MENU_ENTRY_LOAD_DF0:
				run_menuLoad(DF_0);
				return 1;
			case DF_MENU_ENTRY_LOAD_DF1:
				run_menuLoad(DF_1);
				return 1;
			case DF_MENU_ENTRY_EJECT_DF0:
				uae4all_image_file[0]=0;
				disk_eject(0);
				break;
			case DF_MENU_ENTRY_EJECT_DF1:
				uae4all_image_file2[0]=0;
				disk_eject(1);
				break;
			case DF_MENU_ENTRY_BACK:
				return 1; /* leave, returning to main menu */
		}
	}
#endif
}

