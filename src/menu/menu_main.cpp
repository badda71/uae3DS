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

int mainMenu_case=-1;

enum { MAIN_MENU_CASE_REBOOT, MAIN_MENU_CASE_LOAD, MAIN_MENU_CASE_RUN, MAIN_MENU_CASE_RESET, MAIN_MENU_CASE_CANCEL, MAIN_MENU_CASE_EJECT, MAIN_MENU_CASE_SAVE };

static void draw_mainMenu(int c)
{
	static int b=0;
	int bb=(b%6)/3;

	text_draw_background();
	text_draw_window(40,20,260,216,text_str_title);
	if ((c==0)&&(bb))
		write_text_inv(6,3,text_str_load);
	else
		write_text(6,3,text_str_load);
	write_text(6,4,text_str_separator);
	
	write_text(6,5,text_str_separator);
	if ((c==1)&&(bb))
		write_text_inv(6,6,text_str_save);
	else
		write_text(6,6,text_str_save);

	write_text(6,7,text_str_separator);

	write_text(6,9,text_str_throttle);
	if ((mainMenu_throttle==0)&&((c!=2)||(bb)))
		write_text_inv(17,9,text_str_0);
	else
		write_text(17,9,text_str_0);
	if ((mainMenu_throttle==1)&&((c!=2)||(bb)))
		write_text_inv(19,9,text_str_20);
	else
		write_text(19,9,text_str_20);
	if ((mainMenu_throttle==2)&&((c!=2)||(bb)))
		write_text_inv(22,9,text_str_40);
	else
		write_text(22,9,text_str_40);
	if ((mainMenu_throttle==3)&&((c!=2)||(bb)))
		write_text_inv(25,9,text_str_60);
	else
		write_text(25,9,text_str_60);
	if ((mainMenu_throttle==4)&&((c!=2)||(bb)))
		write_text_inv(28,9,text_str_80);
	else
		write_text(28,9,text_str_80);
	if ((mainMenu_throttle==5)&&((c!=2)||(bb)))
		write_text_inv(31,9,text_str_100);
	else
		write_text(31,9,text_str_100);

	write_text(6,11,text_str_frameskip);
	if ((mainMenu_frameskip==0)&&((c!=3)||(bb)))
		write_text_inv(17,11,text_str_0);
	else
		write_text(17,11,text_str_0);
	if ((mainMenu_frameskip==1)&&((c!=3)||(bb)))
		write_text_inv(19,11,text_str_1);
	else
		write_text(19,11,text_str_1);
	if ((mainMenu_frameskip==2)&&((c!=3)||(bb)))
		write_text_inv(21,11,text_str_2);
	else
		write_text(21,11,text_str_2);
	if ((mainMenu_frameskip==3)&&((c!=3)||(bb)))
		write_text_inv(23,11,text_str_3);
	else
		write_text(23,11,text_str_3);
	if ((mainMenu_frameskip==4)&&((c!=3)||(bb)))
		write_text_inv(25,11,text_str_4);
	else
		write_text(25,11,text_str_4);
	if ((mainMenu_frameskip==5)&&((c!=3)||(bb)))
		write_text_inv(27,11,text_str_5);
	else
		write_text(27,11,text_str_5);
	if ((mainMenu_frameskip==-1)&&((c!=3)||(bb)))
		write_text_inv(29,11,text_str_auto);
	else
		write_text(29,11,text_str_auto);

	write_text(6,13,text_str_vpos);
	if ((mainMenu_vpos==0)&&((c!=4)||(bb)))
		write_text_inv(17,13,text_str_0);
	else
		write_text(17,13,text_str_0);
	if ((mainMenu_vpos==1)&&((c!=4)||(bb)))
		write_text_inv(19,13,text_str_8);
	else
		write_text(19,13,text_str_8);
	if ((mainMenu_vpos==2)&&((c!=4)||(bb)))
		write_text_inv(21,13,text_str_16);
	else
		write_text(21,13,text_str_16);
	if ((mainMenu_vpos==3)&&((c!=4)||(bb)))
		write_text_inv(24,13,text_str_24);
	else
		write_text(24,13,text_str_24);
	if ((mainMenu_vpos==4)&&((c!=4)||(bb)))
		write_text_inv(27,13,text_str_32);
	else
		write_text(27,13,text_str_32);
	if ((mainMenu_vpos==5)&&((c!=4)||(bb)))
		write_text_inv(30,13,text_str_40);
	else
		write_text(30,13,text_str_40);

	write_text(6,15,text_str_sound);
	if ((!mainMenu_sound)&&((c!=5)||(bb)))
		write_text_inv(17,15,text_str_off);
	else
		write_text(17,15,text_str_off);
	if ((mainMenu_sound)&&((c!=5)||(bb)))
		write_text_inv(22,15,text_str_on);
	else
		write_text(22,15,text_str_on);
	
	write_text(6,17,text_str_autosave);
	if ((!mainMenu_autosave)&&((c!=6)||(bb)))
		write_text_inv(17,17,text_str_off);
	else
		write_text(17,17,text_str_off);
	if ((mainMenu_autosave)&&((c!=6)||(bb)))
		write_text_inv(22,17,text_str_on);
	else
		write_text(22,17,text_str_on);

	write_text(6,19,text_str_separator);
	if ((c==7)&&(bb))
		write_text_inv(6,20,text_str_eject);
	else
		write_text(6,20,text_str_eject);

	write_text(6,21,text_str_separator);
	if ((c==8)&&(bb))
		write_text_inv(6,22,text_str_reset);
	else
		write_text(6,22,text_str_reset);
	write_text(6,23,text_str_separator);

	if ((c==9)&&(bb))
		write_text_inv(6,24,text_str_run);
	else
		write_text(6,24,text_str_run);
	write_text(6,25,text_str_separator);

	write_text(6,26,text_str_separator);
	if ((c==10)&&(bb))
		write_text_inv(6,27,text_str_exit);
	else
		write_text(6,27,text_str_exit);
//	write_text(6,28,text_str_separator);

	text_flip();
	b++;
}

static int key_mainMenu(int *cp)
{
	int back_c=-1;
	int c=(*cp);
	int end=0;
	int left=0, right=0, up=0, down=0, hit0=0, hit1=0, hit2=0, hit3=0, hit4=0, hit5=0;
	SDL_Event event;

	while (SDL_PollEvent(&event) > 0)
	{
		if (event.type == SDL_QUIT)
		{
			mainMenu_case=MAIN_MENU_CASE_REBOOT;
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
#ifdef DREAMCAST
				case SDLK_c:
				case SDLK_LSHIFT: hit3=1; break;
				case SDLK_x:
				case SDLK_SPACE: hit4=1; break;
				case SDLK_1:
				case SDLK_BACKSPACE: hit5=1; break;
				case SDLK_2:
				case SDLK_TAB: hit2=1; break;
#else
				case SDLK_c:
				case SDLK_LSHIFT: hit4=1; break;
				case SDLK_x:
				case SDLK_SPACE: hit3=1; break;
				case SDLK_1:
				case SDLK_BACKSPACE: hit2=1; break;
				case SDLK_2:
				case SDLK_TAB: hit5=1; break;
#endif
				case SDLK_z:
				case SDLK_RETURN:
				case SDLK_e:
				case SDLK_LCTRL: hit0=1; break;
				case SDLK_q:
				case SDLK_LALT: hit1=1; break;
			}
			if (hit1)
			{
				mainMenu_case=MAIN_MENU_CASE_CANCEL;
				end=1;
			}
			else if (hit2)
			{
				back_c=c;
				hit0=1;
				c=7;
			}
			else if (hit3)
			{
				mainMenu_case=MAIN_MENU_CASE_LOAD;
				end=1;
			}
			else if (hit4)
			{
				mainMenu_case=MAIN_MENU_CASE_SAVE;
				end=1;
			}
			else if (hit5)
			{
				back_c=c;
				hit0=1;
				c=8;
			}
			else if (up)
			{
				if (c>0) c=(c-1)%11;
				else c=10;
			}
			else if (down)
				c=(c+1)%11;
			switch(c)
			{
				case 0:
					if (hit0)
					{
						mainMenu_case=MAIN_MENU_CASE_LOAD;
						end=1;
					}
					break;
				case 1:
					if (hit0)
					{
						mainMenu_case=MAIN_MENU_CASE_SAVE;
						end=1;
					}
					break;
				case 2:
					if (left)
					{
						if (mainMenu_throttle>0)
							mainMenu_throttle--;
						else
							mainMenu_throttle=5;
					}
					else if (right)
					{
						if (mainMenu_throttle<5)
							mainMenu_throttle++;
						else
							mainMenu_throttle=0;
					}
					break;
				case 3:
					if (left)
					{
						if (mainMenu_frameskip>-1)
							mainMenu_frameskip--;
						else
							mainMenu_frameskip=5;
					}
					else if (right)
					{
						if (mainMenu_frameskip<5)
							mainMenu_frameskip++;
						else
							mainMenu_frameskip=-1;
					}
					break;
				case 4:
					if (left)
					{
						if (mainMenu_vpos>0)
							mainMenu_vpos--;
						else
							mainMenu_vpos=5;
					}
					else if (right)
					{
						if (mainMenu_vpos<5)
							mainMenu_vpos++;
						else
							mainMenu_vpos=0;
					}
					break;
				case 5:
					if ((left)||(right))
						mainMenu_sound=~mainMenu_sound;
					break;
				case 6:
					if ((left)||(right))
						mainMenu_autosave=~mainMenu_autosave;
					break;
				case 7:
					if (hit0)
					{
						mainMenu_case=MAIN_MENU_CASE_EJECT;
						end=1;
					}
					break;
				case 8:
					if (hit0)
					{
						mainMenu_case=MAIN_MENU_CASE_RESET;
						end=1;
					}
					break;
				case 9:
					if (hit0)
					{
						mainMenu_case=MAIN_MENU_CASE_RUN;
						end=1;
					}
					break;
				case 10:
					if (hit0)
					{
						mainMenu_case=MAIN_MENU_CASE_REBOOT;
						end=1;
					}
					break;
			}
			if (back_c>=0)
			{
				c=back_c;
				back_c=-1;
			}
		}
	}


	(*cp)=c;
	return end;
}

static void clear_events(void)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
		SDL_Delay(20);
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
	static int c=0;
#else
	static int c=5;
#endif
	int end;
	mainMenu_case=-1;

	while(mainMenu_case<0)
	{
		raise_mainMenu();
		end=0;
		while(!end)
		{
			draw_mainMenu(c);
			end=key_mainMenu(&c);
		}
		unraise_mainMenu();
		switch(mainMenu_case)
		{
			case MAIN_MENU_CASE_SAVE:
#ifndef NO_SAVE_MENU
				run_menuSave();
				if (savestate_state == STATE_DORESTORE || savestate_state == STATE_DOSAVE)
					mainMenu_case=1;
				else
					mainMenu_case=-1;
#else
				mainMenu_case=-1;
#endif
				break;
			case MAIN_MENU_CASE_LOAD:
				run_menuLoad();	
				mainMenu_case=-1;
				break;
			case MAIN_MENU_CASE_EJECT:
				mainMenu_case=3;
				break;
			case MAIN_MENU_CASE_CANCEL:
				if (emulating)
					mainMenu_case=1;
				else
					mainMenu_case=-1;
				break;
			case MAIN_MENU_CASE_RESET:
				if (emulating)
				{
					mainMenu_case=2;
					break;
				}
			case MAIN_MENU_CASE_RUN:
				mainMenu_case=1;
				break;
			case MAIN_MENU_CASE_REBOOT:
#ifdef DREAMCAST
//malloc(16*1024*1024);
				arch_reboot();
#else
				do_leave_program();
				exit(0);
#endif
				break;
			default:
				mainMenu_case=-1;
		}
	}

	return mainMenu_case;
#endif
}

