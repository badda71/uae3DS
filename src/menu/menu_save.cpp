
#ifndef NO_MENU
#ifndef NO_SAVE_MENU

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

static const char *text_str_title="Saved States";
static const char *text_str_savestate="Saved state #";
static const char *text_str_0="0";
static const char *text_str_1="1";
static const char *text_str_2="2";
static const char *text_str_3="3";
#ifdef DREAMCAST_SAVE_VMU
static const char *text_str_loadmem="Load from memory (Y)";
static const char *text_str_savemem="Save to memory (X)";
static const char *text_str_loadvmu="Load from VMU (L)";
static const char *text_str_savevmu="Save to VMU (R)";
#else
static const char *text_str_loadmem="Load state (Y)";
static const char *text_str_savemem="Save state (X)";
#endif
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

	text_draw_background();
	text_draw_window(64,40,208,172,text_str_title);
	write_text(9,6,text_str_separator);
	
	write_text(9,7,text_str_savestate);
	if ((saveMenu_n_savestate==0)&&((c!=0)||(bb)))
		write_text_inv(22,7,text_str_0);
	else
		write_text(22,7,text_str_0);
	if ((saveMenu_n_savestate==1)&&((c!=0)||(bb)))
		write_text_inv(24,7,text_str_1);
	else
		write_text(24,7,text_str_1);
	if ((saveMenu_n_savestate==2)&&((c!=0)||(bb)))
		write_text_inv(26,7,text_str_2);
	else
		write_text(26,7,text_str_2);
	if ((saveMenu_n_savestate==3)&&((c!=0)||(bb)))
		write_text_inv(28,7,text_str_3);
	else
		write_text(28,7,text_str_3);
	write_text(9,8,text_str_separator);

	write_text(9,10,text_str_separator);

	if ((c==1)&&(bb))
		write_text_inv(9,11,text_str_loadmem);
	else
		write_text(9,11,text_str_loadmem);

	write_text(9,12,text_str_separator);

	if ((c==2)&&(bb))
		write_text_inv(9,13,text_str_savemem);
	else
		write_text(9,13,text_str_savemem);

	write_text(9,14,text_str_separator);

#ifdef DREAMCAST_SAVE_VMU
	write_text(9,16,text_str_separator);

	if ((c==3)&&(bb))
		write_text_inv(9,17,text_str_loadvmu);
	else
		write_text(9,17,text_str_loadvmu);

	write_text(9,18,text_str_separator);

	if ((c==4)&&(bb))
		write_text_inv(9,19,text_str_savevmu);
	else
		write_text(9,19,text_str_savevmu);
#endif

	write_text(9,20,text_str_separator);

	write_text(9,22,text_str_separator);

	if ((c==5)&&(bb))
		write_text_inv(9,23,text_str_exit);
	else
		write_text(9,23,text_str_exit);
	write_text(9,24,text_str_separator);

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
				case SDLK_d:
				case SDLK_RIGHT: right=1; break;
				case SDLK_a:
				case SDLK_LEFT: left=1; break;
				case SDLK_w:
				case SDLK_UP: up=1; break;
				case SDLK_s:
				case SDLK_DOWN: down=1; break;
				case SDLK_z:
				case SDLK_RETURN:
				case SDLK_e:
				case SDLK_LCTRL: hit0=1; break;
#ifdef DREAMCAST_SAVE_VMU
				case SDLK_2:
				case SDLK_BACKSPACE: hit2=1; break;
				case SDLK_1:
				case SDLK_TAB: hit3=1; break;
				case SDLK_x:
				case SDLK_SPACE: hit4=1; break;
				case SDLK_c:
				case SDLK_LSHIFT: hit5=1; break;
#elif defined(GCW0)
				case SDLK_2:
				case SDLK_BACKSPACE: hit3=1; break;
				case SDLK_1:
				case SDLK_TAB: hit2=1; break;
				case SDLK_x:
				case SDLK_LSHIFT: hit5=1; break;
				case SDLK_c:
				case SDLK_SPACE: hit4=1; break;
#else
				case SDLK_2:
				case SDLK_BACKSPACE: hit3=1; break;
				case SDLK_1:
				case SDLK_TAB: hit2=1; break;
				case SDLK_x:
				case SDLK_SPACE: hit5=1; break;
				case SDLK_c:
				case SDLK_LSHIFT: hit4=1; break;
#endif
				case SDLK_q:
				case SDLK_LALT: hit1=1; break;
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
#ifdef DREAMCAST_SAVE_VMU
				case 3:
					if (hit0)
					{
						saveMenu_case=SAVE_MENU_CASE_LOAD_VMU;
						end=1;
					}
					break;
				case 4:
					if (hit0)
					{
						saveMenu_case=SAVE_MENU_CASE_SAVE_VMU;
						end=1;
					}
					break;
#endif
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

//	uae4all_play_save();
	text_draw_background();
	text_flip();
	for(i=0;i<8;i++)
	{
		text_draw_background();
		text_draw_window(128-(8*i),(8-i)*24,144+(8*i),172,"");
		text_flip();
		SDL_Delay(15);
	}
}

static inline void unraise_saveMenu()
{
	int i;

	for(i=7;i>=0;i--)
	{
		text_draw_background();
		text_draw_window(128-(8*i),(8-i)*24,144+(8*i),160,"");
		text_flip();
		SDL_Delay(15);
	}
	text_draw_background();
	text_flip();
}

static void show_error(const char *str)
{
	int i;
	
//	uae4all_play_error();
	for(i=0;i<40;i++)
	{
//		menu_moving=0;
		text_draw_background();
		text_draw_window(64,64,160,40,"ERROR !");
		write_text(9,9,str);
		text_flip();
	}
	SDL_Delay(2500);
}


static void show_please_wait(const char *title)
{
//	menu_moving=0;
	text_draw_background();
	text_draw_window(64,64,172,32,title);
//	uae4all_play_wait();
	write_text(12,9,"Please wait");
	text_flip();
	SDL_Delay(1000);
}

int run_menuSave()
{
	static int c=0;
	int end;
	saveMenu_case=-1;

	if (!emulating)
	{
		show_error("No running");
		return 0;
	}

	while(saveMenu_case<0)
	{
		raise_saveMenu();
		end=0;
		while(!end)
		{
			draw_saveMenu(c);
			end=key_saveMenu(&c);
		}
		unraise_saveMenu();
		switch(saveMenu_case)
		{
			case SAVE_MENU_CASE_LOAD_MEM:
//				show_please_wait("Loading");
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
#ifdef DREAMCAST
				extern void reinit_sdcard(void);
				reinit_sdcard();
#endif
				FILE *f=fopen(savestate_filename,"rb");
#ifdef DREAMCAST
				if (!f) {
					char *ad=(char *)calloc(strlen(savestate_filename)+16,1);
					strcpy(ad,"/sd/uae4all/");
					strcat(ad,savestate_filename);
					f=fopen(ad,"rb");
					free(ad);
				}
#endif
				if (f)
				{
					fclose(f);
					savestate_state = STATE_DORESTORE;
					saveMenu_case=1;
				}
				else
				{
					show_error("No exists");
					saveMenu_case=-1;
				}
				}
				break;
			case SAVE_MENU_CASE_SAVE_MEM:
//				show_please_wait("Saving");
				savestate_state = STATE_DOSAVE;
				saveMenu_case=1;
				break;
#ifdef DREAMCAST_SAVE_VMU
			case SAVE_MENU_CASE_LOAD_VMU:
				show_please_wait("VMU Load");
				if (loadstate_vmu(saveMenu_n_savestate))
				{
					show_error("No VMU Saved");
					saveMenu_case=-1;
				}
				else
					saveMenu_case=1;
				break;
			case SAVE_MENU_CASE_SAVE_VMU:
				show_please_wait("VMU Save");
				if (savestate_vmu(saveMenu_n_savestate))
				{
					show_error("VMU Overflow");
					saveMenu_case=-1;
				}
				else
					saveMenu_case=1;
				break;
#endif
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
