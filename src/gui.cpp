 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Interface to the Tcl/Tk GUI
  *
  * Copyright 1996 Bernd Schmidt
  */

#include <math.h>
#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"
#include "uae.h"
#include "options.h"
#include "gui.h"
#include "menu.h"
#include "debug_uae4all.h"
#include "custom.h"
#include "memory.h"
#include "xwin.h"
#include "drawing.h"
#include "sound.h"
#include "audio.h"
#include "keybuf.h"
#include "disk.h"
#include "savestate.h"
#include "joystick.h"
#include "uibottom.h"
#include "autofire.h"
#include "uae3ds.h"

#ifdef HOME_DIR
#include "homedir.h"
#endif

#include <SDL/SDL.h>
#define SDL_NumJoysticks(x) 0

#ifdef PROFILER_UAE4ALL
unsigned long long uae4all_prof_initial[UAE4ALL_PROFILER_MAX];
unsigned long long uae4all_prof_sum[UAE4ALL_PROFILER_MAX];
unsigned long long uae4all_prof_executed[UAE4ALL_PROFILER_MAX];
#endif

#define VIDEO_FLAGS_INIT SDL_HWSURFACE // |SDL_CONSOLEBOTTOM

#ifdef DOUBLEBUFFER
#define VIDEO_FLAGS VIDEO_FLAGS_INIT | SDL_DOUBLEBUF
#else
#define VIDEO_FLAGS VIDEO_FLAGS_INIT
#endif

static char _show_message_str[40]={
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
};

unsigned int show_message=0;
char *show_message_str=(char *)&_show_message_str[0];

extern SDL_Surface *prSDLScreen;
extern struct uae_prefs changed_prefs;
extern struct uae_prefs currprefs;
extern SDL_Joystick *uae4all_joy0, *uae4all_joy1;

extern int keycode2amiga(SDL_keysym *prKeySym);
extern int uae4all_keystate[];

int emulated_mouse_speed=4;
int emulating=0;
char uae4all_image_file[128];
char uae4all_image_file2[128];

int drawfinished=0;

extern int mainMenu_throttle, mainMenu_frameskip, mainMenu_sound, mainMenu_case, mainMenu_autosave, mainMenu_vpos;

int emulated_left=0;
int emulated_right=0;
int emulated_top=0;
int emulated_bot=0;
int emulated_button1=0;
int emulated_button2=0;
int emulated_mouse=0;
int emulated_mouse_button1=0;
int emulated_mouse_button2=0;

void loadConfig()
{
#if defined(HOME_DIR)
	FILE *f;
	char *config = (char *)malloc(strlen(config_dir) + strlen("/uae3DS.cfg") + 1);
	extern char last_directory[PATH_MAX];

	if(config == NULL)
		return;

	sprintf(config, "%s/uae3DS.cfg", config_dir);

	f = fopen(config, "r");

	if(f == NULL)
	{
		printf("Failed to open config file: \"%s\" for reading.\n", config);
		free(config);
		return;
	}

	char line[PATH_MAX * 10 + 30];

	while(fgets(line, sizeof(line), f))
	{
		char *arg = strchr(line, ' ');

		if(!arg)
		{
			continue;
		}
		*arg = '\0';
		arg++;

		if(!strcmp(line, "THROTTLE"))
			sscanf(arg, "%d", &mainMenu_throttle);
		else if(!strcmp(line, "FRAMESKIP"))
			sscanf(arg, "%d", &mainMenu_frameskip);
		else if(!strcmp(line, "SCREEN_POS"))
			sscanf(arg, "%d", &mainMenu_vpos);
		else if(!strcmp(line, "SOUND"))
			sscanf(arg, "%d", &mainMenu_sound);
		else if(!strcmp(line, "SAVE_DISKS"))
			sscanf(arg, "%d", &mainMenu_autosave);
		else if(!strcmp(line, "LAST_DIR"))
		{
			int len = strlen(arg);

			if(len == 0 || len > sizeof(last_directory) - 1)
			{
				continue;
			}

			if(arg[len-1] == '\n')
			{
				arg[len-1] = '\0';
			}

			strcpy(last_directory, arg);
		}
		else if(!strcmp(line, "MAX_TAP_TIME"))
			sscanf(arg, "%d", &mainMenu_max_tap_time);
		else if(!strcmp(line, "CLICK_TIME"))
			sscanf(arg, "%d", &mainMenu_click_time);
		else if(!strcmp(line, "SINGLE_TAP_TIMEOUT"))
			sscanf(arg, "%d", &mainMenu_single_tap_timeout);
		else if(!strcmp(line, "MAX_DOUBLE_TAP_TIME"))
			sscanf(arg, "%d", &mainMenu_max_double_tap_time);
		else if(!strcmp(line, "LOCKED_DRAG_TIMEOUT"))
			sscanf(arg, "%d", &mainMenu_locked_drag_timeout);
		else if(!strcmp(line, "TAP_AND_DRAG_GESTURE"))
			sscanf(arg, "%d", &mainMenu_tap_and_drag_gesture);
		else if(!strcmp(line, "LOCKED_DRAGS"))
			sscanf(arg, "%d", &mainMenu_locked_drags);
		else if(!strcmp(line, "KEYMAPPINGS"))
			uae3ds_mapping_loadbuf(arg);
		else if(!strcmp(line, "FAVORITES"))
			menu_load_favorites(arg);
	}

	fclose(f);
	free(config);
#endif
}

void storeConfig()
{
#if defined(HOME_DIR)
	FILE *f;
	char *config = (char *)malloc(strlen(config_dir) + strlen("/uae3DS.cfg") + 1);
	extern char last_directory[PATH_MAX];

	if(config == NULL)
		return;

	sprintf(config, "%s/uae3DS.cfg", config_dir);

	f = fopen(config, "w");

	if(f == NULL)
	{
		printf("Failed to open config file: \"%s\" for writing.\n", config);
		free(config);
		return;
	}

	char *s1 = uae3ds_mapping_savebuf();
	char *s2 = menu_save_favorites();

	fprintf(f,
		"THROTTLE %d\n"
		"FRAMESKIP %d\n"
		"SCREEN_POS %d\n"
		"SOUND %d\n"
		"SAVE_DISKS %d\n"
		"MAX_TAP_TIME %d\n"
		"CLICK_TIME %d\n"
		"SINGLE_TAP_TIMEOUT %d\n"
		"MAX_DOUBLE_TAP_TIME %d\n"
		"LOCKED_DRAG_TIMEOUT %d\n"
		"TAP_AND_DRAG_GESTURE %d\n"
		"LOCKED_DRAGS %d\n"
		"KEYMAPPINGS %s\n"
		"FAVORITES %s\n",
		mainMenu_throttle,
		mainMenu_frameskip,
		mainMenu_vpos,
		mainMenu_sound,
		mainMenu_autosave,
		mainMenu_max_tap_time,
		mainMenu_click_time,
		mainMenu_single_tap_timeout,
		mainMenu_max_double_tap_time,
		mainMenu_locked_drag_timeout,
		mainMenu_tap_and_drag_gesture,
		mainMenu_locked_drags,
		s1,
		s2
	);

	if(last_directory[0])
	{
		fprintf(f, "LAST_DIR %s\n", last_directory);
	}

	fclose(f);
	free(config);
#endif
}

static void getChanges(void)
{
    m68k_speed=mainMenu_throttle;
#ifndef NO_SOUND
    if (mainMenu_sound)
    {
	    changed_produce_sound=2;
	    sound_default_evtime();
    }
    else
#endif
	    changed_produce_sound=0;
    changed_gfx_framerate=mainMenu_frameskip;
    init_hz();
}

int gui_init (void)
{
//Se ejecuta justo despues del MAIN
    if (prSDLScreen==NULL)
	prSDLScreen=SDL_SetVideoMode(400,240,16,VIDEO_FLAGS);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_JoystickEventState(SDL_IGNORE);
    SDL_JoystickOpen(0);
    if (prSDLScreen!=NULL)
    {
	emulating=0;
	uae4all_image_file[0]=0;
	uae4all_image_file2[0]=0;
	init_text(1);
	loadConfig();
	// start menu as soon as possible
	//run_mainMenu();
	SDL_Event e;
	e.type = SDL_KEYDOWN;
	e.key.keysym.sym = DS_SELECT;
	e.key.keysym.mod = KMOD_MODE; // not mappable
	SDL_PushEvent(&e);
	e.type = SDL_KEYUP;
	SDL_PushEvent(&e);

	quit_text();
	uae4all_pause_music();
	emulating=1;
	getChanges();
	check_all_prefs();
	reset_frameskip();
    black_screen_now();
#ifdef DEBUG_FRAMERATE
	uae4all_update_time();
#endif
#ifdef PROFILER_UAE4ALL
	uae4all_prof_init();
	uae4all_prof_add("M68K");			// 0
	uae4all_prof_add("EVENTS");			// 1
	uae4all_prof_add("HSync");			// 2
	uae4all_prof_add("Copper");			// 3
	uae4all_prof_add("Audio");			// 4
	uae4all_prof_add("CIA");			// 5
	uae4all_prof_add("Blitter");			// 6
	uae4all_prof_add("Vsync");			// 7
	uae4all_prof_add("update_fetch");		// 8
	uae4all_prof_add("linetoscr");			// 9
	uae4all_prof_add("do_long_fetch");		// 10
	uae4all_prof_add("pfield_doline");		// 11
	uae4all_prof_add("draw_sprites_ecs");		// 12
	uae4all_prof_add("flush_block");		// 13
	uae4all_prof_add("SET_INTERRUPT");		// 14
/*
	uae4all_prof_add("15");		// 15
	uae4all_prof_add("16");		// 16
	uae4all_prof_add("17");		// 17
	uae4all_prof_add("18");		// 18
	uae4all_prof_add("19");		// 19
	uae4all_prof_add("20");		// 20
	uae4all_prof_add("21");		// 21
	uae4all_prof_add("22");		// 22
*/
#endif
	return 0;
    }
    return -1;
}

#define show_mhz()
#define inc_dingoo_mhz()
#define dec_dingoo_mhz()
#define inc_dingoo_brightness()
#define dec_dingoo_brightness()
#define inc_dingoo_volumen()
#define dec_dingoo_volumen()

int gui_update (void)
{
    extern char *savestate_filename;
#ifndef NO_SAVE_MENU
    extern int saveMenu_n_savestate;
#endif
// SE EJECUTA DESPUES DE INICIAR EL CORE 68k
    strcpy(changed_df[0],uae4all_image_file);
    strcpy(changed_df[1],uae4all_image_file2);
	strcpy(savestate_filename, SAVESTATE_PREFIX);
	strcat(savestate_filename, uae4all_image_file[0] ? uae4all_image_file : "null");
#ifndef NO_SAVE_MENU
    switch(saveMenu_n_savestate)
    {
	    case 1:
    		strcat(savestate_filename,"-1.asf");
			break;
	    case 2:
    		strcat(savestate_filename,"-2.asf");
			break;
	    case 3:
    		strcat(savestate_filename,"-3.asf");
			break;
	    default: 
    	   	strcat(savestate_filename,".asf");
    }
#endif
    real_changed_df[0]=1;
    real_changed_df[1]=1;
    show_mhz();
    return 0;
}


static void goMenu(void)
{
   int exitmode=0;
   int autosave=mainMenu_autosave;
   if (quit_program != 0)
	    return;
#ifdef PROFILER_UAE4ALL
   uae4all_prof_show();
#endif
#ifdef DEBUG_FRAMERATE
   uae4all_show_time();
#endif
   emulating=1;
   init_text(0);
   pause_sound();
   menu_raise();
   exitmode=run_mainMenu();
   notice_screen_contents_lost();
   resume_sound();
   if ((!(strcmp(prefs_df[0],uae4all_image_file))) || ((!(strcmp(prefs_df[1],uae4all_image_file2)))))
	   menu_unraise();
   quit_text();
#ifdef DREAMCAST
   SDL_DC_EmulateKeyboard(SDL_FALSE);
#endif
    getChanges();
    if (exitmode==1 || exitmode==2)
    {
    	    extern char *savestate_filename;
#ifndef NO_SAVE_MENU
    	    extern int saveMenu_n_savestate;
#endif
	    if (strcmp(changed_df[0],uae4all_image_file))
	    {
            	strcpy(changed_df[0],uae4all_image_file);
	    	real_changed_df[0]=1;
	    }
	    if (strcmp(changed_df[1],uae4all_image_file2))
	    {
            	strcpy(changed_df[1],uae4all_image_file2);
	    	real_changed_df[1]=1;
	    }
		strcpy(savestate_filename, SAVESTATE_PREFIX);
		strcat(savestate_filename, uae4all_image_file[0] ? uae4all_image_file : "null");
#ifndef NO_SAVE_MENU
	    switch(saveMenu_n_savestate)
    	    {
	    	case 1:
    			strcat(savestate_filename,"-1.asf");
				break;
	    	case 2:
    			strcat(savestate_filename,"-2.asf");
				break;
	    	case 3:
    			strcat(savestate_filename,"-3.asf");
				break;
	    	default: 
    	   		strcat(savestate_filename,".asf");
    	    }
#endif
    }
    if (exitmode==2)
    {
	    if (autosave!=mainMenu_autosave)
	    {
	    	prefs_df[0][0]=0;
	   	prefs_df[1][0]=0;
	    }
	    black_screen_now();
	    show_mhz();
	    uae_reset ();
    }
    check_all_prefs();
    gui_purge_events();
    black_screen_now();
    notice_screen_contents_lost();
#ifdef DEBUG_FRAMERATE
    uae4all_update_time();
#endif
#ifdef PROFILER_UAE4ALL
    uae4all_prof_init();
#endif
}

int nowSuperThrottle=0;

static void goSuperThrottle(void)
{
	if (!nowSuperThrottle)
	{
		nowSuperThrottle=1;
		m68k_speed=1; //6;
		changed_produce_sound=0;
		changed_gfx_framerate=80;
		check_prefs_changed_cpu();
		check_prefs_changed_audio();
		check_prefs_changed_custom();
		gui_set_message("SuperThrottle On",1000);
	}
}

static void leftSuperThrottle(void)
{
	if (nowSuperThrottle)
	{
		nowSuperThrottle=0;
		getChanges();
		check_prefs_changed_cpu();
		check_prefs_changed_audio();
		check_prefs_changed_custom();
		gui_set_message("SuperThrottle Off",1000);
	}
}

static void inc_throttle(int sgn)
{
	char n[40];
	static Uint32 last=0;
	Uint32 now=SDL_GetTicks();
	if (now-last<555)
		return;
	last=now;
	if (sgn>0)
	{
		if (mainMenu_throttle<5)
			mainMenu_throttle++;
	}
	else if (sgn<0)
		if (mainMenu_throttle>0)
			mainMenu_throttle--;
	nowSuperThrottle=0;
	getChanges();
	check_prefs_changed_cpu();
	check_prefs_changed_audio();
	check_prefs_changed_custom();
	sprintf((char *)&n[0],"Throttle %i",mainMenu_throttle*20);
	gui_set_message((char *)&n[0],1000);
}

static int in_goMenu=0;
extern "C" void N3DS_SetScalingDirect(float x, float y, int permanent);

void gui_handle_events (SDL_Event *e)
{
	static int scale = 100;
	char buf[50];
	
	int v,t = e->type;
	if ((v=(t == SDL_KEYDOWN)) || t == SDL_KEYUP) {
		switch (e->key.keysym.sym) {
		case DS_A:
			emulated_button1=v; break;
		case DS_B:
			emulated_button2=v; break;
		case DS_Y:
		case DS_ZL:
			buttonstate[0] = v; break;
		case DS_X:
		case DS_L:
			buttonstate[2] = v; break;
		case DS_UP1:
		case DS_UP2:
			emulated_top=v; break;
		case DS_DOWN1:
		case DS_DOWN2:
			emulated_bot=v; break;
		case DS_LEFT1:
		case DS_LEFT2:
			emulated_left=v; break;
		case DS_RIGHT1:
		case DS_RIGHT2:
			emulated_right=v; break;
		case DS_UP3:
			if (v) mainMenu_vpos += 2;
		case DS_DOWN3:
			if (v) {
				mainMenu_vpos -= 1;
				if (mainMenu_vpos > 5) mainMenu_vpos=5;
				if (mainMenu_vpos < 0) mainMenu_vpos=0;
				snprintf(buf,50,"VPOS %d",mainMenu_vpos*8);
				gui_set_message(buf,1000);
				getChanges();
				check_all_prefs();
			    notice_screen_contents_lost();
			}
			break;
		case DS_LEFT3:
			if (v) scale += 10;
		case DS_RIGHT3:
			if (v) {
				scale -= 5;
				if (scale < 100) scale=100;
				if (scale > 200) scale=200;
				float f = scale==100 ? 1.0f : ((float)scale/100.0f);
				N3DS_SetScalingDirect(f, f, 0);
				snprintf(buf,50,"Scale %d%%",scale);
				gui_set_message(buf,1000);
			}
			break;
		case DS_SELECT:
			if (v) goMenu();
			break;
		case DS_START:
			if (v) {
				if (nowSuperThrottle)
					leftSuperThrottle();
				else
					goSuperThrottle();
			}
			break;
		case DS_R:
			if (v) autofire_start();
			else autofire_stop();
		default:
			break;
		}
	}
}

void gui_changesettings (void)
{
	dbg("GUI: gui_changesettings");
}

void gui_update_gfx (void)
{
// ANTES DE LA ENTRADA EN VIDEO
//	dbg("GUI: gui_update_gfx");
}

void gui_set_message(const char *msg, int msecs)
{
	show_message=SDL_GetTicks() + msecs;
	strncpy(show_message_str, msg, 36);
}

void gui_show_window_bar(int per, int max, int case_title)
{
	const char *title;
	if (case_title)
		title="  Restore State";
	else
		title="  Save State";
	_text_draw_window_bar(prSDLScreen,120,64,172,48,per,max,title);
#if defined(DOUBLEBUFFER) || defined(DINGOO)
	SDL_Flip(prSDLScreen);
#endif
}

/*
int run_menuGame() {}
int run_menuControl() {}
*/


#ifdef PROFILER_UAE4ALL

static unsigned uae4all_prof_total_initial=0;
unsigned uae4all_prof_total=0;
static char *uae4all_prof_msg[UAE4ALL_PROFILER_MAX];

void uae4all_prof_init(void)
{
	unsigned i;
#ifndef DREAMCAST
	unsigned long long s=SDL_GetTicks();
#else
	unsigned long long s=timer_us_gettime64();
#endif
	for(i=0;i<UAE4ALL_PROFILER_MAX;i++)
	{
		uae4all_prof_initial[i]=s;
		uae4all_prof_sum[i]=0;
		uae4all_prof_executed[i]=0;
		if (!uae4all_prof_total)
			uae4all_prof_msg[i]=NULL;
	}
	uae4all_prof_total_initial=s;
}

void uae4all_prof_add(char *msg)
{
	if (uae4all_prof_total<UAE4ALL_PROFILER_MAX)
	{
		uae4all_prof_msg[uae4all_prof_total]=msg;	
		uae4all_prof_total++;
	}
}

void uae4all_prof_show(void)
{
	unsigned i;
	double toper=0;
#ifndef DREAMCAST
	unsigned long long to=SDL_GetTicks()-uae4all_prof_total_initial;
#else
	unsigned long long to=uae4all_prof_sum[0]+uae4all_prof_sum[1];
	for(i=0;i<uae4all_prof_total;i++)
		if (uae4all_prof_sum[i]>to)
			uae4all_prof_sum[i]=0;
#endif

	puts("\n\n\n\n");
	puts("--------------------------------------------");
	for(i=0;i<uae4all_prof_total;i++)
	{
		unsigned long long t0=uae4all_prof_sum[i];
		double percent=(double)t0;
		percent*=100.0;
		percent/=(double)to;
		toper+=percent;
#ifdef DREAMCAST
		t0/=1000;
#endif
		printf("%s: %.2f%% -> Ticks=%i -> %iK veces\n",uae4all_prof_msg[i],percent,((unsigned)t0),(unsigned)(uae4all_prof_executed[i]>>10));
	}
	printf("TOTAL: %.2f%% -> Ticks=%i\n",toper,to);
	puts("--------------------------------------------"); fflush(stdout);
}
#endif
