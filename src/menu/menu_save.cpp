#include "sysconfig.h"

#ifndef NO_MENU
#ifndef NO_SAVE_MENU

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "menu.h"
#include "sysdeps.h"
#include "uae.h"
#include "options.h"
#include "sound.h"
#include "savestate.h"
#include "keyboard.h"
#include "uibottom.h"

static const char *text_str_title="Saved States";
static const char *text_str_savestate="Saved state #";
static const char *text_str_0="0";
static const char *text_str_1="1";
static const char *text_str_2="2";
static const char *text_str_3="3";
static const char *text_str_loadmem="Load state (Y)";
static const char *text_str_savemem="Save state (X)";
static const char *text_str_separator="----------------------";
static const char *text_str_exit="Main menu (B)";

extern int emulating;

int saveMenu_n_savestate=0;
int saveMenu_case=-1;

enum { SAVE_MENU_CASE_EXIT, SAVE_MENU_CASE_LOAD_MEM, SAVE_MENU_CASE_SAVE_MEM, SAVE_MENU_CASE_LOAD_VMU, SAVE_MENU_CASE_SAVE_VMU, SAVE_MENU_CASE_CANCEL };

static inline void draw_saveMenu(int c)
{
	static int b=0;
	int bb=(b%6)/3;
	int col = 13;

	text_draw_background();
	text_draw_window(96,40,208,172,text_str_title);
	write_text(col,6,text_str_separator);
	
	write_text(col,7,text_str_savestate);
	if ((saveMenu_n_savestate==0)&&((c!=0)||(bb)))
		write_text_inv(col+13,7,text_str_0);
	else
		write_text(col+13,7,text_str_0);
	if ((saveMenu_n_savestate==1)&&((c!=0)||(bb)))
		write_text_inv(col+15,7,text_str_1);
	else
		write_text(col+15,7,text_str_1);
	if ((saveMenu_n_savestate==2)&&((c!=0)||(bb)))
		write_text_inv(col+17,7,text_str_2);
	else
		write_text(col+17,7,text_str_2);
	if ((saveMenu_n_savestate==3)&&((c!=0)||(bb)))
		write_text_inv(col+19,7,text_str_3);
	else
		write_text(col+19,7,text_str_3);
	write_text(col,8,text_str_separator);

	write_text(col,10,text_str_separator);

	if ((c==1)&&(bb))
		write_text_inv(col,11,text_str_loadmem);
	else
		write_text(col,11,text_str_loadmem);

	write_text(col,12,text_str_separator);

	if ((c==2)&&(bb))
		write_text_inv(col,13,text_str_savemem);
	else
		write_text(col,13,text_str_savemem);

	write_text(col,14,text_str_separator);

	write_text(col,20,text_str_separator);

	write_text(col,22,text_str_separator);

	if ((c==5)&&(bb))
		write_text_inv(col,23,text_str_exit);
	else
		write_text(col,23,text_str_exit);
	write_text(col,24,text_str_separator);

	text_flip();
	b++;
}

static inline int key_saveMenu(int *cp)
{
	int c=(*cp);
	int back_c=-1;
	int end=0;
	int left=0, right=0, up=0, down=0;
	int hit0=0, hit1=0, hit2=0, hit3=0, hit4=0, hit5=0;
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
			if (hit1)
			{
				saveMenu_case=SAVE_MENU_CASE_CANCEL;
				end=1;
			}
			else if (hit2)
			{
				back_c=c;
				c=3;
				hit0=1;
			}
			else if (hit3)
			{
				back_c=c;
				c=4;
				hit0=1;
			}
			else if (hit4)
			{
				back_c=c;
				c=1;
				hit0=1;
			}
			else if (hit5)
			{
				back_c=c;
				c=2;
				hit0=1;
			}
			else if (up)
			{
				if (c>0) c=(c-1)%6;
				else c=5;
#ifndef DREAMCAST_SAVE_VMU
				if (c==4) c=2;
#endif
			}
			else if (down)
			{
				c=(c+1)%6;
#ifndef DREAMCAST_SAVE_VMU
				if (c==3) c=5;
#endif
			}
			else
			if (left)
			{
				if (saveMenu_n_savestate>0)
					saveMenu_n_savestate--;
				else
					saveMenu_n_savestate=3;
			}
			else if (right)
			{
				if (saveMenu_n_savestate<3)
					saveMenu_n_savestate++;
				else
					saveMenu_n_savestate=0;
			}
			switch(c)
			{
				case 0:
					break;
				case 1:
					if (hit0)
					{
						saveMenu_case=SAVE_MENU_CASE_LOAD_MEM;
						end=1;
					}
					break;
				case 2:
					if (hit0)
					{
						saveMenu_case=SAVE_MENU_CASE_SAVE_MEM;
						end=1;
					}
					break;
				case 5:
					if (hit0)
					{
						saveMenu_case=SAVE_MENU_CASE_EXIT;
						end=1;
					}
					break;
			}
		}
		if (back_c>=0)
		{
			c=back_c;
			back_c=-1;
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
	text_messagebox("ERROR", str, MB_OK);
}

int run_menuSave()
{
	static int c=0;
	int end;
	saveMenu_case=-1;

	if (!emulating)
	{
		show_error("Not running");
		return 0;
	}

	while(saveMenu_case<0)
	{
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
				{
				extern char uae4all_image_file[];
				strcpy(savestate_filename,uae4all_image_file);
				switch(saveMenu_n_savestate)
				{
					case 1:
						strcat(savestate_filename,"-1.asf");
					case 2:
						strcat(savestate_filename,"-2.asf");
					case 3:
						strcat(savestate_filename,"-3.asf");
					default: 
						strcat(savestate_filename,".asf");
				}
				FILE *f=fopen(savestate_filename,"rb");
				if (f)
				{
					fclose(f);
					savestate_state = STATE_DORESTORE;
					saveMenu_case=1;
				}
				else
				{
					show_error("Not Found");
					saveMenu_case=-1;
				}
				}
				break;
			case SAVE_MENU_CASE_SAVE_MEM:
				savestate_state = STATE_DOSAVE;
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

	return saveMenu_case;
}
#endif
#endif
