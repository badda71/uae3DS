#include "sysconfig.h"

#ifndef NO_MENU
#ifndef NO_SAVE_MENU

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "menu.h"
#include "sysdeps.h"
#include "config.h"
#include "uae.h"
#include "options.h"
#include "sound.h"
#include "savestate.h"
#include "keyboard.h"
#include "uibottom.h"
#include "uae3ds.h"

typedef struct {
	char *name;
	int snaps;
} ASFFile;

static ASFFile (*files)[] = NULL;
static int nrfiles = 0;
static int offset = 0;

static char *text_str_title;
static const char *text_str_title1="Load State";
static const char *text_str_title2="Save State";
//static const char *text_str_savestate="Saved state #";
//static const char *text_str_0="0";
//static const char *text_str_1="1";
//static const char *text_str_2="2";
//static const char *text_str_3="3";
//static const char *text_str_loadmem="Load state (Y)";
//static const char *text_str_savemem="Save state (X)";
static const char *text_str_separator="----------------------------------------------";
static const char *text_str_exit="Main menu (B)";

extern int emulating;

int saveMenu_n_savestate=0;
int saveMenu_n_savestate_real=0;
int saveMenu_case=-1;

static SaveMode mode = MODE_LOAD;

enum { 
	SAVE_MENU_CASE_EXIT,
	SAVE_MENU_CASE_LOAD_MEM,
	SAVE_MENU_CASE_SAVE_MEM,
	SAVE_MENU_CASE_LOAD_VMU,
	SAVE_MENU_CASE_SAVE_VMU,
	SAVE_MENU_CASE_CANCEL };

static inline void draw_saveMenu(int c)
{
	static int b=0;
	int bb=(b%6)/3;
	int i;
	char buf[2]={0};
	static int old_c=c;

	static Scrollstatus ss = {0};

	const int lines = 8;
	const int height = 26;
	const int width = 46;
	int row = 15*8-height*4, col = 25*8-width*4;

	text_draw_background();
	text_draw_window(192-width*4, 112-height*4, (width+2)*8, (height+2)*8, text_str_title);

	if ((c==0)&&(bb))
		write_text_inv_pos(col,row,text_str_exit);
	else
		write_text_pos(col,row,text_str_exit);
	row+=8;
	write_text_pos(col,row,text_str_separator);
	row+=8;

	// adjust offest if needed
	if (c>0) {
		if (c-1 >= offset + lines) offset = c-lines;
		if (c-1 < offset) offset = c-1;
	}
	if (nrfiles > lines) {
		draw_scrollbar(col + width * 8 + 2, row - 2, 6, lines * 2 * 12, nrfiles, lines, offset);
	}
	char text[width+1];

	for (i = offset; i < nrfiles && i < offset + lines; ++i) {
		if (old_c != c) {
			memset(&ss, 0, sizeof(ss));
			old_c = c;
		}
		if (c==i+1) {
			write_text_pos_scroll(&ss, 1, 1, col, row, width, (*files)[i].name);
		} else {
			snprintf(text,width+1,"%s",(*files)[i].name);
			write_text_pos(col, row, text);
		}

		row+=12;
		for(int x=0; x<4; ++x) {
			extern SDL_Color text_color;
			SDL_Color back;
			buf[0]=x+'0';
			if (!((*files)[i].snaps & (1<<x))) {
				back = text_color;
				text_color = (SDL_Color){0x80,0x80,0x80,0x00};
			}
			if (saveMenu_n_savestate==x && c==i+1 && bb)
				write_text_inv_pos(col+(2+x*2)*8,row,buf);
			else
				write_text_pos(col+(2+x*2)*8,row,buf);
			if (!((*files)[i].snaps & (1<<x))) {
				text_color = back;
			}
		}
		row+=12;
	}

	text_flip();
	b++;
}

static inline int key_saveMenu(int *cp)
{
	int c=(*cp);
	int end=0;
	int left=0, right=0, up=0, down=0;
	int hit0=0, hit1=0, hit2=0, hit3=0, hit4=0, hit5=0;
	static int oldc;
	SDL_Event event;

	while (SDL_PollEvent(&event) > 0)
	{
		if (uib_handle_event(&event)) continue;

		if (event.type == SDL_QUIT)
		{
			saveMenu_case=SAVE_MENU_CASE_EXIT;
			end=-1;
		}
		else
		if (event.type == SDL_KEYDOWN)
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
				case AK_RET:
				case AK_SPC:
				case DS_START:
				case DS_A: hit0=1; break;
				case DS_R:
				case AK_R: hit3=1; break;
				case DS_L:
				case AK_L: hit2=1; break;
				case AK_X:
				case DS_X: hit5=1; break;
				case AK_Y:
				case DS_Y: hit4=1; break;
				case AK_ESC:
				case DS_B: hit1=1; break;
			}
			if (hit0) {		// A
				if (c==0) {
					saveMenu_case=SAVE_MENU_CASE_EXIT;
					end=1;
				} else if (mode == MODE_LOAD) {
					saveMenu_case = SAVE_MENU_CASE_LOAD_MEM;
					end=1;
				} else 	if (
					!((*files)[c-1].snaps & (1<<saveMenu_n_savestate)) ||
					text_messagebox(text_str_title, "Overwrite previous state file?", MB_YESNO)==0)
				{
					saveMenu_case = SAVE_MENU_CASE_SAVE_MEM;
					end=1;
				}
			}
			else if (hit1)	// B
			{
				saveMenu_case=SAVE_MENU_CASE_CANCEL;
				end=1;
			}
			else if (hit2)	// L
			{
			}
			else if (hit3)	// R
			{
			}
			else if (hit4)	// Y
			{
			}
			else if (hit5)	// X
			{
			}
			else if (up || down)
			{
				if (mode == MODE_LOAD) {
					c += up ? -1 : 1;
					if (c < 0) c=nrfiles;
					if (c > nrfiles) c=0;
					if (c) {
						saveMenu_n_savestate = saveMenu_n_savestate_real;
						int d=0;
						while (saveMenu_n_savestate < 0 || saveMenu_n_savestate > 3 ||
							((*files)[c-1].snaps & (1 << saveMenu_n_savestate)) == 0) {
							d=(-1*d)+(d>0?-1:1);
							saveMenu_n_savestate += d;
						}
					}
				} else {
					if (c==0) c=oldc;
					else {oldc=c;c=0;}
				}
			}
			else if (left || right)
			{
				if (c>0) {
					do {
						saveMenu_n_savestate = (saveMenu_n_savestate + (left ? 3 : 1)) % 4;
					} while (
						mode != MODE_SAVE &&
						(((*files)[c-1].snaps & (1 << saveMenu_n_savestate)) == 0));
					saveMenu_n_savestate_real = saveMenu_n_savestate;
				}
			}
		}
	}


	(*cp)=c;
	return end;
}

static inline void raise_saveMenu()
{
	int i;

	text_draw_background();
	text_flip();
	for(i=0;i<10;i++)
	{
		text_draw_background();
		text_draw_window(96,(10-i)*24,208,172,text_str_title);
		text_flip();
	}
}

static inline void unraise_saveMenu()
{
	int i;

	for(i=9;i>=0;i--)
	{
		text_draw_background();
		text_draw_window(96,(10-i)*24,208,172,text_str_title);
		text_flip();
	}
	text_draw_background();
	text_flip();
}

static void show_error(const char *str)
{
	text_messagebox("ERROR", (char*)str, MB_OK);
}

static void addFile(const char *file, int nr) {
	int i;
	for (i=0; i<nrfiles; i++) {
		if (strcmp(file, (*files)[i].name) == 0) break;
	}

	if (i==nrfiles) {
		++nrfiles;
		files = (ASFFile (*)[])realloc(files , nrfiles * sizeof(ASFFile));
		(*files)[i].name=stralloc((char*)file);
		(*files)[i].snaps=0;
	}
	(*files)[i].snaps |= nr;
}

static int mycmp(const void *a, const void *b) {
	return strcasecmp((*((ASFFile*)a)).name,(*((ASFFile*)b)).name);
}

int run_menuSave(SaveMode m)
{
	static int c=0;
	int end, i;
	saveMenu_case=-1;

	mode = m;

	if (!emulating)
	{
		show_error("Not running");
		return 0;
	}

	while(saveMenu_case<0)
	{
		// get list of save states in save state dir
		char *p;
		DIR *dir;
		extern char uae4all_image_file[];
		struct dirent *entry;
		if ((dir = opendir(SAVESTATE_PREFIX)) == NULL) break;
		while ((entry = readdir(dir)) != NULL) {
			p = entry->d_name + strlen(entry->d_name) - 6;
			if (entry->d_type == DT_DIR ||
				p < entry->d_name ||
				strcasecmp(p+2,".asf") != 0 ||
				p[0]!='-' ||
				p[1]<'0' ||
				p[1]>'3')
				continue;
			p[0]=0;

			addFile(entry->d_name, 1 << (p[1]-'0'));
		}
		closedir(dir);
		if (mode == MODE_SAVE)
			addFile(uae4all_image_file[0] ? uae4all_image_file : "null", 0);

		qsort(*files, nrfiles, sizeof(ASFFile), mycmp);
		if (mode == MODE_SAVE) {
			for (i=0;i<nrfiles;++i)
				if (strcmp(uae4all_image_file[0] ? uae4all_image_file : "null", (*files)[i].name) == 0) {
				c=i+1;
				break;
			}
		}
		text_str_title = (char*)( mode == MODE_LOAD ? text_str_title1 : text_str_title2);

		raise_saveMenu();
		end=0;
		while(!end)
		{
			draw_saveMenu(c);
			SDL_Delay(10);
			end=key_saveMenu(&c);
		}
		unraise_saveMenu();
		switch(saveMenu_case)
		{
			case SAVE_MENU_CASE_LOAD_MEM:
			case SAVE_MENU_CASE_SAVE_MEM:
				if (saveMenu_case == SAVE_MENU_CASE_LOAD_MEM) {
					savestate_state = STATE_DORESTORE;
				} else {
					savestate_state =  STATE_DOSAVE;
				}
				snprintf(savestate_filename, 100,
					"%s%s-%d.asf", SAVESTATE_PREFIX, (*files)[c-1].name, saveMenu_n_savestate);
				saveMenu_case=1;
				break;
			case SAVE_MENU_CASE_EXIT:	
			case SAVE_MENU_CASE_CANCEL:	
				saveMenu_case=1;
				break;
			default:
				saveMenu_case=-1;
		}
	}
	
	// free the file listing
	if (files) {
		for (i=0; i<nrfiles; ++i)
			free((*files)[i].name);
		free(files);
		files = NULL;
	}
	nrfiles=0;

	return saveMenu_case;
}
#endif
#endif
