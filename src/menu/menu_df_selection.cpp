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
#include "disk.h"
#include "uibottom.h"
#include "keyboard.h"

extern int emulating;

static const char *text_str_title_df_menu="Select disk";
static const char *text_str_load_df0="DF0 image (X)";
static const char *text_str_load_df1="DF1 image (Y)";
static const char *text_str_eject="<Eject>";
static const char *text_str_back="Main Menu (B)";
static const char *text_str_separator="----------------------------------";
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
	DF_MENU_ENTRY_LOAD,
	DF_MENU_ENTRY_FAV,
	DF_MENU_ENTRY_EJECT,
	DF_MENU_ENTRY_BACK,
	DF_MENU_ENTRY_COUNT, /* the number of entries to be shown */
};

int dfMenu_vpos=1;
int dfMenu_df=0;
int dfMenu_fav=0;

char *favorites[NUM_FAV] = {0};

static void draw_dfMenu(enum DfMenuEntry c)
{
	static Scrollstatus ss1 = {0};
	static Scrollstatus ss2 = {0};
	static int frame = 0;
	int flash = frame / 3;
	const int height = 24;
	const int width = 34;
	int row = 15*8-height*4, col = 25*8-width*4;

	text_draw_background();
	text_draw_window(192-width*4, 112-height*4, (width+2)*8, (height+2)*8, text_str_title_df_menu);

	write_text_pos(col, row, text_str_df0);
	write_text_pos(col, row+12, text_str_df1);
	write_text_pos_scroll (&ss1, 0, 2,
		col+4*8, row, width - 4, uae4all_image_file[0]?uae4all_image_file:text_str_empty,
		col+4*8, row+12, width - 4, uae4all_image_file2[0]?uae4all_image_file2:text_str_empty);
	row+=20;
	write_text_pos(col, row, text_str_separator);
	row+=8;

	if (dfMenu_df==0 && (c != DF_MENU_ENTRY_LOAD || flash))
		write_text_inv_pos(col, row, text_str_load_df0);
	else
		write_text_pos(col, row, text_str_load_df0);

	if (dfMenu_df==1 && (c != DF_MENU_ENTRY_LOAD || flash))
		write_text_inv_pos(col+17*8, row, text_str_load_df1);
	else
		write_text_pos(col+17*8, row, text_str_load_df1);
	row+=8;

	write_text_pos(col, row, text_str_separator);
	row+=8;
	// fav list
	char text[width+1];
	static int old_fav=dfMenu_fav;
	for (int i=0; i < NUM_FAV; ++i) {
		if (old_fav != dfMenu_fav) {
			memset(&ss2, 0, sizeof(ss2));
			old_fav = dfMenu_fav;
		}
		if (favorites[i]) {
			if (dfMenu_fav==i && c == DF_MENU_ENTRY_FAV) {
				if (flash)
					write_text_pos_scroll(&ss2, 1, 1, col, row, width, strrchr(favorites[i],'/')+1);
				else
					write_text_pos_scroll(&ss2, 0, 1, col, row, width, strrchr(favorites[i],'/')+1);
			} else {
				snprintf(text,width+1,"%s",strrchr(favorites[i],'/')+1);
				write_text_pos(col, row, text);
			}
		} else
			write_text_pos(col, row, " --");
		row+=12;
	}

	// eject
	if (c == DF_MENU_ENTRY_EJECT && flash)
		write_text_inv_pos(col, row, text_str_eject);
	else
		write_text_pos(col, row, text_str_eject);
	row+=8;

	write_text_pos(col, row, text_str_separator);
	row+=8;

	if (c == DF_MENU_ENTRY_BACK && flash)
		write_text_inv_pos(col, row, text_str_back);
	else
		write_text_pos(col, row, text_str_back);
	row+=8;

	text_flip();
	frame = (frame + 1) % 6;
}

static enum DfMenuEntry key_dfMenu(enum DfMenuEntry *sel)
{
	SDL_Event event;

	while (SDL_PollEvent(&event) > 0)
	{
		if (uib_handle_event(&event)) continue;
		int left = 0, right = 0, up = 0, down = 0,
		    activate = 0, cancel = 0, load_df0 = 0, load_df1 = 0;
		if (event.type == SDL_QUIT)
			return DF_MENU_ENTRY_BACK;
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
				case DS_Y: load_df1 = 1; break;
				case AK_X:
				case DS_X: load_df0 = 1; break;
				case AK_RET:
				case AK_SPC:
				case DS_START:
				case DS_A: activate = 1; break;
				case AK_ESC:
				case DS_B: cancel = 1; break;
			}
			if (cancel)
				return DF_MENU_ENTRY_BACK;
			else if (load_df0)
			{
				dfMenu_df=0;
				return DF_MENU_ENTRY_LOAD;
			}
			else if (load_df1)
			{
				dfMenu_df=1;
				return DF_MENU_ENTRY_LOAD;
			}
			else if (up)
			{
				if (*sel == DF_MENU_ENTRY_FAV && dfMenu_fav > 0)
					--dfMenu_fav;
				else {
					while(1) {
						if (*sel > 0) *sel = (enum DfMenuEntry) ((*sel - 1) % DF_MENU_ENTRY_COUNT);
						else *sel = (enum DfMenuEntry) (DF_MENU_ENTRY_COUNT - 1);
						if (*sel != DF_MENU_ENTRY_FAV) break;
						dfMenu_fav=0;
						while(dfMenu_fav < NUM_FAV-1 && favorites[dfMenu_fav+1]!=NULL)
							++dfMenu_fav;
						if (favorites[0]!=NULL) break;
					}
				}
			}
			else if (down)
				if (*sel == DF_MENU_ENTRY_FAV && dfMenu_fav < NUM_FAV-1 && favorites[dfMenu_fav+1] != NULL)
					++dfMenu_fav;
				else
					while (1) {
						*sel = (enum DfMenuEntry) ((*sel + 1) % DF_MENU_ENTRY_COUNT);
						if (*sel != DF_MENU_ENTRY_FAV) break;
						dfMenu_fav=0;
						if (favorites[0]!=NULL) break;
					}
			else if (left || right) {
				if (*sel == DF_MENU_ENTRY_LOAD ||
					*sel == DF_MENU_ENTRY_FAV ||
					*sel == DF_MENU_ENTRY_EJECT) {
					dfMenu_df = (dfMenu_df + 1) % 2;
				}
			}
			else
			{
				switch (*sel)
				{
					case DF_MENU_ENTRY_LOAD:
					case DF_MENU_ENTRY_EJECT:
					case DF_MENU_ENTRY_BACK:
					case DF_MENU_ENTRY_FAV:
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
		text_draw_window(72,(10-i)*24,260,200,text_str_title_df_menu);
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
		text_draw_window(72,(10-i)*24,260,200,text_str_title_df_menu);
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
	dfMenu_fav=0;
	char *p;
	extern char last_directory[];

	static enum DfMenuEntry c = DF_MENU_ENTRY_LOAD;

	while (1)
	{
		enum DfMenuEntry action = DF_MENU_ENTRY_NONE;
		raise_dfMenu();
		while (action == DF_MENU_ENTRY_NONE)
		{
			draw_dfMenu(c);
			SDL_Delay(10);
			action = key_dfMenu(&c);
		}
		unraise_dfMenu();
		switch (action)
		{
			case DF_MENU_ENTRY_LOAD:
				run_menuLoad(dfMenu_df ? DF_1 : DF_0);
				break;
			case DF_MENU_ENTRY_EJECT:
				if (dfMenu_df) {
					uae4all_image_file2[0]=0;
					disk_eject(1);
				} else {
					uae4all_image_file[0]=0;
					disk_eject(0);
				}
				break;
			case DF_MENU_ENTRY_FAV:
				p=strrchr(favorites[dfMenu_fav],'/');
				*p=0;
				chdir(favorites[dfMenu_fav]);
				getcwd(last_directory, PATH_MAX);
				snprintf(dfMenu_df ? uae4all_image_file2 : uae4all_image_file,
					128, "%s", p+1);
				*p='/';
				break;
			case DF_MENU_ENTRY_BACK:
				return 1; /* leave, returning to main menu */
		}
	}
#endif
}

char *stralloc(char *s) {
	if (!s) return NULL;
	char *p=(char*)malloc(strlen(s)+1);
	if (!p) return NULL;
	strcpy(p, s);
	return p;
}

char *concat(char *s, ...) {
	char *p, *ret;
	if (!s) return NULL;
	int l = strlen(s);

	va_list vl;
	va_start(vl,s);
	while ((p = va_arg(vl, char*))!=NULL) {
		l += strlen(p);
	}
	va_end(vl);

	ret = (char*)malloc(l);
	if (!ret) return NULL;
	strcpy(ret, s);
	l=strlen(ret);
	va_start(vl,s);
	while ((p = va_arg(vl, char*))!=NULL) {
		strcpy(ret + l, p);
		l += strlen(p);
	}
	va_end(vl);
	return ret;
}

void menu_addFavImage(char *path)
{
	int num=0, i, i1;
	extern char last_directory[];
	char *p;

	// complete path
	if (strchr(path, '/')) {
		p=stralloc(path);
	} else {
		p=(char*)malloc(strlen(path)+strlen(last_directory)+1);
		sprintf(p,"%s%s",last_directory, path);
	}

	// get current queue size
	while (num < NUM_FAV && favorites[num]!=NULL)
		++num;

	// already in queue?
	for( i = 0 ; i < num ; ++i ) {
		if (strcmp(favorites[i],p)==0) break;
	}
	if (i == NUM_FAV) i = NUM_FAV-1;

	// move elements one up
	if (i < num) free(favorites[i]);
	for (i1 = i; i1 > 0 ; --i1) {
		favorites[i1]=favorites[i1-1];
	}
	favorites[0] = p;
}

void menu_load_favorites(char *s)
{
	if (!s) return;
	char *saveptr, *p;
	int i=0;
	p = strtok_r(s, "|", &saveptr);
	while (p) {
		favorites[i++]=stralloc(p);
		p = strtok_r(NULL, "|", &saveptr);
	}
}

char *menu_save_favorites()
{
	int l=0;
	char *p;
	for (int i=0; favorites[i]!=NULL; ++i)
		l += strlen(favorites[i])+1;
	p=(char*)malloc(l);
	if (!p) return NULL;
	*p=0;
	for (int i=0; favorites[i]!=NULL; ++i)
		sprintf(p+strlen(p), "%s%s", i==0 ? "" : "|", favorites[i]);
	return p;
}