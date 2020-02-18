 /*
  * UAE - The Un*x Amiga Emulator
  *
  * SDL interface
  *
  * Copyright 2001 Bernd Lachner (EMail: dev@lachner-net.de)
  *
  * Partialy based on the UAE X interface (xwin.c)
  *
  * Copyright 1995, 1996 Bernd Schmidt
  * Copyright 1996 Ed Hanway, Andre Beck, Samuel Devulder, Bruno Coste
  * Copyright 1998 Marcus Sundberg
  * DGA support by Kai Kollmorgen
  * X11/DGA merge, hotkeys and grabmouse by Marcus Sundberg
  */

void carga(void);
void guarda(void);

#include "sysconfig.h"
#include "sysdeps.h"

#include <unistd.h>
#include <signal.h>

#include <SDL/SDL.h>
#include <SDL/SDL_endian.h>

#include "config.h"
#include "uae.h"
#include "options.h"
#include "memory.h"
#include "xwin.h"
#include "custom.h"
#include "drawing.h"
#include "m68k/m68k_intrf.h"
#include "keyboard.h"
#include "keybuf.h"
#include "gui.h"
#include "debug.h"
#include "savestate.h"
#include "menu/menu.h"
#include "vkbd/vkbd.h"

#ifdef DREAMCAST
#include <SDL/SDL_dreamcast.h>
extern int __sdl_dc_emulate_mouse;
#endif

#include "debug_uae4all.h"

#include "vkbd.h"

extern int drawfinished;

int prefs_gfx_framerate, changed_gfx_framerate;

uae_u16 *prSDLScreenPixels;

char *gfx_mem=NULL;
unsigned gfx_rowbytes=0;

Uint32 uae4all_numframes=0;

#ifdef DEBUG_FRAMERATE

Uint32 uae4all_frameskipped=0;
double uae4all_framerate=0.0;

void uae4all_update_time(void)
{
	uae4all_numframes=0;
	uae4all_frameskipped=0;
	uae4all_framerate=0.0;
}

void uae4all_show_time(void)
{
	int i;
#if !defined(AUTO_FRAMERATE_SOUND) && !defined(NO_SOUND)
	extern double media_ratio;
	extern unsigned sound_cuantos[8];
	extern unsigned sound_ajustes;
	extern unsigned sound_alcanza_callback;
	extern unsigned sound_alcanza_render;
#endif
	double p=(((double)uae4all_frameskipped)*((double)100.0))/((double)uae4all_numframes);
	double p0=((100.0-p)*uae4all_framerate)/100.0;
	printf("---- frameskipping = %.4f%%\n",p);
	printf("---- framerate = %.4f/%.4f\n",p0,uae4all_framerate);
#if !defined(AUTO_FRAMERATE_SOUND) && !defined(NO_SOUND)
	printf("---- audio ratio media = %.2f msec\n",media_ratio/1000.0);
	for(i=0;i<8;i++)
		printf("distancia %i = %i -> %.2f%%\n",i,sound_cuantos[i],(((double)sound_cuantos[i])*100.0)/((double)sound_ajustes));
	printf("CALLBACK ALCANZA %i VECES A RENDER\n",sound_alcanza_callback);
	printf("RENDER ALCANZA %i VECES A CALLBACK\n",sound_alcanza_render);
#endif
}
#endif


#ifdef DREAMCAST
#define VIDEO_FLAGS_INIT SDL_HWSURFACE|SDL_FULLSCREEN
#else
#ifdef DINGOO
#define VIDEO_FLAGS_INIT SDL_SWSURFACE
#else
#define VIDEO_FLAGS_INIT SDL_HWSURFACE
#endif
#endif

#ifdef DOUBLEBUFFER
#define VIDEO_FLAGS VIDEO_FLAGS_INIT | SDL_DOUBLEBUF
#else
#define VIDEO_FLAGS VIDEO_FLAGS_INIT
#endif

/* Uncomment for debugging output */
/* #define DEBUG */

static __inline__ RETSIGTYPE sigbrkhandler (int foo) {}

/* SDL variable for output surface */
SDL_Surface *prSDLScreen = NULL;
/* Possible screen depth (0 terminated) */

static int red_bits, green_bits, blue_bits;
static int red_shift, green_shift, blue_shift;

static int current_width, current_height;
static SDL_Color arSDLColors[256];
static int ncolors = 0;
extern int emulated_mouse, emulated_mouse_button1, emulated_mouse_button2;

/* Keyboard and mouse */
int uae4all_keystate[256];


#ifdef USE_RASTER_DRAW
void flush_block (int ystart, int ystop)
#else
void flush_screen (void)
#endif
{
    uae4all_prof_start(13);
#ifdef DEBUG_GFX
    dbgf("Function: flush_block %d %d\n", ystart, ystop);
#endif
#if !defined(DREAMCAST) && !defined(DINGOO)
    if (SDL_MUSTLOCK(prSDLScreen))
    	SDL_UnlockSurface (prSDLScreen);
#endif
#ifndef DINGOO
#ifndef DOUBLEBUFFER
#ifdef USE_RASTER_DRAW
    SDL_UpdateRect(prSDLScreen, 0, ystart, current_width, ystop-ystart+1);
#else
    SDL_UpdateRect(prSDLScreen, 0, 0, 320, 240);
#endif
#endif
#endif
#ifdef USE_RASTER_DRAW
    if (drawfinished)
    {
	drawfinished=0;
#endif
	if (show_message)
	{
		show_message--;
		if (!show_message) {
			notice_screen_contents_lost();
		} else {
			_write_text_inv_n(prSDLScreen,0,29,30,show_message_str);
		}
	}
	if (emulated_mouse)
		vkbd_mouse();
	if (vkbd_mode)
		vkbd_key=vkbd_process();
#if defined(DOUBLEBUFFER) || defined(DINGOO)
	SDL_Flip(prSDLScreen);
#endif
#if !defined(DREAMCAST) && !defined(DINGOO)
    if (SDL_MUSTLOCK(prSDLScreen))
    	SDL_LockSurface (prSDLScreen);
#endif
#if defined(DOUBLEBUFFER) || defined(DINGOO)
	gfx_mem = (char*) prSDLScreen->pixels;
	prSDLScreenPixels = (uae_u16*) prSDLScreen->pixels;
	reset_screen_pointers();
#endif
#ifdef USE_RASTER_DRAW
    }
#endif
    uae4all_prof_end(13);
}

void black_screen_now(void)
{
	SDL_FillRect(prSDLScreen,NULL,0);
#if defined(DOUBLEBUFFER) || defined(DINGOO)
	SDL_Flip(prSDLScreen);
#else
	SDL_UpdateRect(prSDLScreen, 0, 0, 320, 240);
#endif

}

static __inline__ int bitsInMask (unsigned long mask)
{
	/* count bits in mask */
	int n = 0;
	while (mask)
	{
		n += mask & 1;
		mask >>= 1;
	}
	return n;
}

static __inline__ int maskShift (unsigned long mask)
{
	/* determine how far mask is shifted */
	int n = 0;
	while (!(mask & 1))
	{
		n++;
		mask >>= 1;
	}
	return n;
}

static int get_color (int r, int g, int b, xcolnr *cnp)
{
#ifdef DEBUG_GFX
	dbg("Function: get_color");
#endif

	*cnp = SDL_MapRGB(prSDLScreen->format, r, g, b);
	arSDLColors[ncolors].r = r;
	arSDLColors[ncolors].g = g;
	arSDLColors[ncolors].b = b;

	ncolors++;
	return 1;
}

static int init_colors (void)
{
	int i;

#ifdef DEBUG_GFX
	dbg("Function: init_colors");
#endif

		/* Truecolor: */
		red_bits = bitsInMask(prSDLScreen->format->Rmask);
		green_bits = bitsInMask(prSDLScreen->format->Gmask);
		blue_bits = bitsInMask(prSDLScreen->format->Bmask);
		red_shift = maskShift(prSDLScreen->format->Rmask);
		green_shift = maskShift(prSDLScreen->format->Gmask);
		blue_shift = maskShift(prSDLScreen->format->Bmask);
		alloc_colors64k (red_bits, green_bits, blue_bits, red_shift, green_shift, blue_shift);
		for (i = 0; i < 4096; i++)
			xcolors[i] = xcolors[i] * 0x00010001;

	return 1;
}

int graphics_setup (void)
{
#ifdef DEBUG_GFX
    dbg("Function: graphics_setup");
#endif

    /* Initialize the SDL library */
    if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
    {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        return 0;
    }

    return 1;
}


static void graphics_subinit (void)
{
	Uint32 uiSDLVidModFlags;

#ifdef DEBUG_GFX
	dbg("Function: graphics_subinit");
#endif

	/* Open SDL Window in current mode */
	uiSDLVidModFlags = SDL_HWSURFACE; //SDL_SWSURFACE;
#ifdef DEBUG_GFX
	dbgf("Resolution: %d x %d\n", current_width, current_height);
#endif

	if (prSDLScreen==NULL)
#ifdef DREAMCAST
		prSDLScreen = SDL_SetVideoMode(current_width, current_height, 16, uiSDLVidModFlags|VIDEO_FLAGS);
#else
		prSDLScreen = SDL_SetVideoMode(current_width, current_height, 16, uiSDLVidModFlags|VIDEO_FLAGS);
#endif
	if (prSDLScreen == NULL)
	{
		fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
		return;
	}
	else
	{
#if !defined(DREAMCAST) && !defined(DINGOO)
		if (SDL_MUSTLOCK(prSDLScreen))
			SDL_LockSurface(prSDLScreen);
#endif
		prSDLScreenPixels=(uae_u16 *)prSDLScreen->pixels;
#ifdef DEBUG_GFX
		dbgf("Bytes per Pixel: %d\n", prSDLScreen->format->BytesPerPixel);
		dbgf("Bytes per Line: %d\n", prSDLScreen->pitch);
#endif
		memset(prSDLScreen->pixels, 0, current_width * current_height * prSDLScreen->format->BytesPerPixel);
#if !defined(DREAMCAST) && !defined(DINGOO)
		if (SDL_MUSTLOCK(prSDLScreen))
			SDL_UnlockSurface(prSDLScreen);
#endif
#if !defined(DOUBLEBUFFER) && !defined(DINGOO)
		SDL_UpdateRect(prSDLScreen, 0, 0, current_width, current_height);
#else
		SDL_Flip(prSDLScreen);
#endif
#ifndef DREAMCAST
		/* Set UAE window title and icon name */
		SDL_WM_SetCaption("UAE4ALL","UAE4ALL");
		/* Hide mouse cursor */
		SDL_ShowCursor(SDL_DISABLE);
#endif
		/* Initialize structure for Amiga video modes */
#if !defined(DREAMCAST) && !defined(DINGOO)
		if (SDL_MUSTLOCK(prSDLScreen))
			SDL_LockSurface(prSDLScreen);
#endif
		gfx_mem = (char *)prSDLScreen->pixels;
		gfx_rowbytes = prSDLScreen->pitch;
	}
#ifdef DEBUG_GFX
	dbgf("current_height=%i\n",current_height);
#endif
	lastmx = lastmy = 0;
	newmousecounters = 0;
}


int graphics_init (void)
{
	int i,j;

#ifdef DEBUG_GFX
	dbg("Function: graphics_init");
#endif

	current_width = PREFS_GFX_WIDTH;
	current_height = PREFS_GFX_HEIGHT;

	graphics_subinit ();


    if (!init_colors ())
		return 0;

    buttonstate[0] = buttonstate[1] = buttonstate[2] = 0;
    for (i = 0; i < 256; i++)
	uae4all_keystate[i] = 0;

#ifdef DEBUG_FRAMERATE
    uae4all_update_time();
#endif
    return 1;
}

static void graphics_subshutdown (void)
{
#ifdef DEBUG_GFX
    dbg("Function: graphics_subshutdown");
#endif

    SDL_FreeSurface(prSDLScreen);
}

void graphics_leave (void)
{
#ifdef DEBUG_GFX
    dbg("Function: graphics_leave");
#endif

    graphics_subshutdown ();

	SDL_VideoQuit();

    dumpcustom ();
}

/* Decode KeySyms. This function knows about all keys that are common
 * between different keyboard languages. */
static int kc_decode (SDL_keysym *prKeySym)
{
    switch (prKeySym->sym)
    {
    case SDLK_b: return AK_B;
    case SDLK_c: return AK_C;
    case SDLK_d: return AK_D;
    case SDLK_e: return AK_E;
    case SDLK_f: return AK_F;
    case SDLK_g: return AK_G;
    case SDLK_h: return AK_H;
    case SDLK_i: return AK_I;
    case SDLK_j: return AK_J;
    case SDLK_k: return AK_K;
    case SDLK_l: return AK_L;
    case SDLK_n: return AK_N;
    case SDLK_o: return AK_O;
    case SDLK_p: return AK_P;
    case SDLK_r: return AK_R;
    case SDLK_s: return AK_S;
    case SDLK_t: return AK_T;
    case SDLK_u: return AK_U;
    case SDLK_v: return AK_V;
    case SDLK_x: return AK_X;

    case SDLK_0: return AK_0;
    case SDLK_1: return AK_1;
    case SDLK_2: return AK_2;
    case SDLK_3: return AK_3;
    case SDLK_4: return AK_4;
    case SDLK_5: return AK_5;
    case SDLK_6: return AK_6;
    case SDLK_7: return AK_7;
    case SDLK_8: return AK_8;
    case SDLK_9: return AK_9;

    case SDLK_KP0: return AK_NP0;
    case SDLK_KP1: return AK_NP1;
    case SDLK_KP2: return AK_NP2;
    case SDLK_KP3: return AK_NP3;
    case SDLK_KP4: return AK_NP4;
    case SDLK_KP5: return AK_NP5;
    case SDLK_KP6: return AK_NP6;
    case SDLK_KP7: return AK_NP7;
    case SDLK_KP8: return AK_NP8;
    case SDLK_KP9: return AK_NP9;
    case SDLK_KP_DIVIDE: return AK_NPDIV;
    case SDLK_KP_MULTIPLY: return AK_NPMUL;
    case SDLK_KP_MINUS: return AK_NPSUB;
    case SDLK_KP_PLUS: return AK_NPADD;
    case SDLK_KP_PERIOD: return AK_NPDEL;
    case SDLK_KP_ENTER: return AK_ENT;

    case SDLK_F1: return AK_F1;
    case SDLK_F2: return AK_F2;
    case SDLK_F3: return AK_F3;
    case SDLK_F4: return AK_F4;
    case SDLK_F5: return AK_F5;
    case SDLK_F6: return AK_F6;
    case SDLK_F7: return AK_F7;
    case SDLK_F8: return AK_F8;
    case SDLK_F9: return AK_F9;
    case SDLK_F10: return AK_F10;

    case SDLK_BACKSPACE: return AK_BS;
    case SDLK_DELETE: return AK_DEL;
    case SDLK_LCTRL: return AK_CTRL;
    case SDLK_RCTRL: return AK_RCTRL;
    case SDLK_TAB: return AK_TAB;
    case SDLK_LALT: return AK_LALT;
    case SDLK_RALT: return AK_RALT;
    case SDLK_RMETA: return AK_RAMI;
    case SDLK_LMETA: return AK_LAMI;
    case SDLK_RETURN: return AK_RET;
    case SDLK_SPACE: return AK_SPC;
    case SDLK_LSHIFT: return AK_LSH;
    case SDLK_RSHIFT: return AK_RSH;
    case SDLK_ESCAPE: return AK_ESC;

    case SDLK_INSERT: return AK_HELP;
    case SDLK_HOME: return AK_NPLPAREN;
    case SDLK_END: return AK_NPRPAREN;
    case SDLK_CAPSLOCK: return AK_CAPSLOCK;

    case SDLK_UP: return AK_UP;
    case SDLK_DOWN: return AK_DN;
    case SDLK_LEFT: return AK_LF;
    case SDLK_RIGHT: return AK_RT;

    case SDLK_PAGEUP: return AK_RAMI;          /* PgUp mapped to right amiga */
    case SDLK_PAGEDOWN: return AK_LAMI;        /* PgDn mapped to left amiga */

    default: return -1;
    }
}

static int decode_us (SDL_keysym *prKeySym)
{
    switch(prKeySym->sym)
    {
	/* US specific */
    case SDLK_a: return AK_A;
    case SDLK_m: return AK_M;
    case SDLK_q: return AK_Q;
    case SDLK_y: return AK_Y;
    case SDLK_w: return AK_W;
    case SDLK_z: return AK_Z;
    case SDLK_LEFTBRACKET: return AK_LBRACKET;
    case SDLK_RIGHTBRACKET: return AK_RBRACKET;
    case SDLK_COMMA: return AK_COMMA;
    case SDLK_PERIOD: return AK_PERIOD;
    case SDLK_SLASH: return AK_SLASH;
    case SDLK_SEMICOLON: return AK_SEMICOLON;
    case SDLK_MINUS: return AK_MINUS;
    case SDLK_EQUALS: return AK_EQUAL;
	/* this doesn't work: */
    case SDLK_BACKQUOTE: return AK_QUOTE;
    case SDLK_QUOTE: return AK_BACKQUOTE;
    case SDLK_BACKSLASH: return AK_BACKSLASH;
    default: return -1;
    }
}

int keycode2amiga(SDL_keysym *prKeySym)
{
    int iAmigaKeycode = kc_decode(prKeySym);
    if (iAmigaKeycode == -1)
            return decode_us(prKeySym);
    return iAmigaKeycode;
}

static int refresh_necessary = 0;

void handle_events (void)
{
    SDL_Event rEvent;
    int iAmigaKeyCode;
    int i, j;
    int iIsHotKey = 0;
#ifdef DEBUG_EVENTS
    dbg("Function: handle_events");
#endif

#if !defined(DREAMCAST) && !defined(DINGOO)
    if (SDL_MUSTLOCK(prSDLScreen))
    	SDL_UnlockSurface (prSDLScreen);
#endif

#if defined(MAX_AUTOEVENTS) || defined(AUTOEVENTS)
	{
		static unsigned cuenta=0;
/*
		switch(cuenta&63)
		{
			case 8:
				if ((cuenta<6500)||(cuenta>8000))
					joy1button=1;
				break;
			case 16:
				buttonstate[0]=1; break;
			case 24:
				joy1button=0; break;
			case 28:
				if (cuenta>11000)
					joy1dir=3;
				break;
			case 32:
				buttonstate[0]=0; break;
			case 62:
				joy1dir=0; break;
		}
//		lastmy+=8;
		switch(cuenta&127)
		{
			case 20:
				record_key(0x12); break;
			case 40:
				record_key(0x13); break;
			case 60:
				record_key(0x88); break;
			case 80:
				record_key(0x89); break;
		}
*/
/*
if (cuenta==7700)
{
strcpy(changed_df[0],"prueba2.adz");
real_changed_df[0]=1;
joy1button=1;
}
*/

// Defender of the Crown
 /*
switch(cuenta)
{
case 2600:
lastmx+=80;
break;
case 2610:
buttonstate[0]=1; break;
break;
case 2615:
buttonstate[0]=0; break;
break;
case 4500:
lastmy+=100;
break;
case 4510:
buttonstate[0]=1; break;
break;
case 4515:
buttonstate[0]=0; break;
break;
case 4640:
lastmy-=60;
lastmx+=550;
break;
case 4700:
lastmx+=200;
break;
case 4710:
buttonstate[0]=1; break;
break;
case 4715:
buttonstate[0]=0; break;
break;


}
// printf("%i -> %.8X\n",cuenta,chipmem_checksum());
 */


#if defined(START_DEBUG) && !defined(START_DEBUG_SAVESTATE)
		if (cuenta==START_DEBUG)
		{
#ifdef DEBUG_FILE
			if (!DEBUG_STR_FILE)
				DEBUG_STR_FILE=fopen(DEBUG_FILE,"wt");
#endif
			DEBUG_AHORA=1;
		}
#else
#ifdef START_DEBUG_SAVESTATE
		if (cuenta==START_DEBUG)
			savestate_state = STATE_DOSAVE;
#endif
#endif
#ifdef AUTO_SAVESTATE
		if (cuenta==AUTO_SAVESTATE)
			savestate_state = STATE_DORESTORE;
#endif

#ifdef MAX_AUTOEVENTS
#ifdef DEBUG_EVENTS
		dbgf(" AUTO EVENTS: %i =?= %i\n",cuenta,MAX_AUTOEVENTS);
#endif
		if (cuenta>MAX_AUTOEVENTS)
		{
			int i;
#ifdef DEBUG_FILE
			fclose(DEBUG_STR_FILE);
			SDL_Delay(100);
			for(i=0;i<0x10000;i+=78)
			{
				SDL_FillRect(prSDLScreen, NULL, i);
				SDL_Flip(prSDLScreen);
			}
			SDL_Delay(333);
#endif
			exit(0);
		}
		else
			dbgf("handle_events %i\n",cuenta);
#endif
		cuenta++;
	}
#else
    /* Handle GUI events */
    gui_handle_events ();

#ifdef EMULATED_JOYSTICK
	{
		if ((vkbd_button3==(SDLKey)0)&&(!vkbd_mode))
			buttonstate[0]=emulated_mouse_button1;
		if ((vkbd_button4==(SDLKey)0)&&(!vkbd_mode))
			buttonstate[2]=emulated_mouse_button2;
	}
#endif

    while (SDL_PollEvent(&rEvent))
    {
	switch (rEvent.type)
	{
	case SDL_QUIT:
#ifdef DEBUG_EVENTS
	    dbg("Event: quit");
#endif
	    uae_quit();
	    break;
	    break;
	case SDL_JOYBUTTONDOWN:
	    if (vkbd_mode) break;
	    if ((rEvent.jbutton.button==6) && (vkbd_button2!=(SDLKey)0))
		    rEvent.key.keysym.sym=vkbd_button2;
	    else if ((rEvent.jbutton.button==5) && (vkbd_button3!=(SDLKey)0))
		    rEvent.key.keysym.sym=vkbd_button3;
	    else if ((rEvent.jbutton.button==1) && (vkbd_button4!=(SDLKey)0))
		    rEvent.key.keysym.sym=vkbd_button4;
	    else
	    	break;
        case SDL_KEYDOWN:
#ifdef DEBUG_EVENTS
	    dbg("Event: key down");
#endif
#ifndef DREAMCAST
	    if ((rEvent.key.keysym.sym!=SDLK_F11)&&(rEvent.key.keysym.sym!=SDLK_F12)&&(rEvent.key.keysym.sym!=SDLK_PAGEUP)
#ifdef EMULATED_JOYSTICK
		&&(rEvent.key.keysym.sym!=SDLK_ESCAPE)&&((rEvent.key.keysym.sym!=SDLK_SPACE)||((rEvent.key.keysym.sym==SDLK_SPACE)&&(vkbd_button3!=(SDLKey)0)&&(!vkbd_mode)))&&(rEvent.key.keysym.sym!=SDLK_LCTRL)&&((rEvent.key.keysym.sym!=SDLK_LALT)||((rEvent.key.keysym.sym==SDLK_LALT)&&(vkbd_button2!=(SDLKey)0)&&(!vkbd_mode)))&&(rEvent.key.keysym.sym!=SDLK_RETURN)&&((rEvent.key.keysym.sym!=SDLK_LSHIFT)||((rEvent.key.keysym.sym==SDLK_LSHIFT)&&(vkbd_button4!=(SDLKey)0)&&(!vkbd_mode)))&&(rEvent.key.keysym.sym!=SDLK_TAB)&&(rEvent.key.keysym.sym!=SDLK_BACKSPACE)&&(rEvent.key.keysym.sym!=SDLK_UP)&&(rEvent.key.keysym.sym!=SDLK_DOWN)&&(rEvent.key.keysym.sym!=SDLK_LEFT)&&(rEvent.key.keysym.sym!=SDLK_RIGHT)
#endif
			    )
	    {
		    if ((rEvent.key.keysym.sym==SDLK_LALT)&&(vkbd_button2!=(SDLKey)0)&&(!vkbd_mode))
			    rEvent.key.keysym.sym=vkbd_button2;
		    else
		    if ((rEvent.key.keysym.sym==SDLK_LSHIFT)&&(vkbd_button4!=(SDLKey)0)&&(!vkbd_mode))
			    rEvent.key.keysym.sym=vkbd_button4;
		    else
		    if ((rEvent.key.keysym.sym==SDLK_SPACE)&&(vkbd_button3!=(SDLKey)0)&&(!vkbd_mode))
			    rEvent.key.keysym.sym=vkbd_button3;
#else
	    {
#endif
		iAmigaKeyCode = keycode2amiga(&(rEvent.key.keysym));
		if (iAmigaKeyCode >= 0)
		{
		    if (!uae4all_keystate[iAmigaKeyCode])
		    {
			uae4all_keystate[iAmigaKeyCode] = 1;
			record_key(iAmigaKeyCode << 1);
		    }
		}
	    }
	    break;
	case SDL_JOYBUTTONUP:
	    if (vkbd_mode) break;
	    if ((rEvent.jbutton.button==6) && (vkbd_button2!=(SDLKey)0))
		    rEvent.key.keysym.sym=vkbd_button2;
	    else if ((rEvent.jbutton.button==5) && (vkbd_button3!=(SDLKey)0))
		    rEvent.key.keysym.sym=vkbd_button3;
	    else if ((rEvent.jbutton.button==1) && (vkbd_button4!=(SDLKey)0))
		    rEvent.key.keysym.sym=vkbd_button4;
	    else
	    	break;
	case SDL_KEYUP:
#ifdef DEBUG_EVENTS
	    dbg("Event: key up");
#endif
#ifndef DREAMCAST
	    if ((rEvent.key.keysym.sym!=SDLK_F11)&&(rEvent.key.keysym.sym!=SDLK_F12)&&(rEvent.key.keysym.sym!=SDLK_PAGEUP)
#ifdef EMULATED_JOYSTICK
		&&(rEvent.key.keysym.sym!=SDLK_ESCAPE)&&((rEvent.key.keysym.sym!=SDLK_SPACE)||((rEvent.key.keysym.sym==SDLK_SPACE)&&(vkbd_button3!=(SDLKey)0)&&(!vkbd_mode)))&&(rEvent.key.keysym.sym!=SDLK_LCTRL)&&((rEvent.key.keysym.sym!=SDLK_LALT)||((rEvent.key.keysym.sym==SDLK_LALT)&&(vkbd_button2!=(SDLKey)0)&&(!vkbd_mode)))&&(rEvent.key.keysym.sym!=SDLK_RETURN)&&((rEvent.key.keysym.sym!=SDLK_LSHIFT)||((rEvent.key.keysym.sym==SDLK_LSHIFT)&&(vkbd_button4!=(SDLKey)0)&&(!vkbd_mode)))&&(rEvent.key.keysym.sym!=SDLK_TAB)&&(rEvent.key.keysym.sym!=SDLK_BACKSPACE)&&(rEvent.key.keysym.sym!=SDLK_UP)&&(rEvent.key.keysym.sym!=SDLK_DOWN)&&(rEvent.key.keysym.sym!=SDLK_LEFT)&&(rEvent.key.keysym.sym!=SDLK_RIGHT)
#endif
			    )
	    {
		    if ((rEvent.key.keysym.sym==SDLK_LALT)&&(vkbd_button2!=(SDLKey)0)&&(!vkbd_mode))
			    rEvent.key.keysym.sym=vkbd_button2;
		    else
		    if ((rEvent.key.keysym.sym==SDLK_LSHIFT)&&(vkbd_button4!=(SDLKey)0)&&(!vkbd_mode))
			    rEvent.key.keysym.sym=vkbd_button4;
		    else
		    if ((rEvent.key.keysym.sym==SDLK_SPACE)&&(vkbd_button3!=(SDLKey)0)&&(!vkbd_mode))
			    rEvent.key.keysym.sym=vkbd_button3;
#else
	    {
#endif
		iAmigaKeyCode = keycode2amiga(&(rEvent.key.keysym));
		if (iAmigaKeyCode >= 0)
		{
		    uae4all_keystate[iAmigaKeyCode] = 0;
		    record_key((iAmigaKeyCode << 1) | 1);
		}
	    }
	    break;
	case SDL_MOUSEBUTTONDOWN:
#ifdef DEBUG_EVENTS
	    dbg("Event: mouse button down");
#endif
#ifdef DREAMCAST
	    if (__sdl_dc_emulate_mouse)
	    {
	    	if (vkbd_mode) break;
	    	if (rEvent.button.button==5 ) {
			if (vkbd_button3==(SDLKey)0)
			    buttonstate[0] = 1;
			else 
				break;
		}
	    	else if (rEvent.button.button==1)  {
			if (vkbd_button4==(SDLKey)0)
		 	   buttonstate[2] = 1;
			else
				break;
		}
	    }
	    else
	    	if (rEvent.button.button)
			buttonstate[2]=1;
		else
			buttonstate[0]=1;
#else
	    	buttonstate[(rEvent.button.button-1)%3] = 1;
#endif
	    break;
	case SDL_MOUSEBUTTONUP:
#ifdef DEBUG_EVENTS
	    dbg("Event: mouse button up");
#endif
#ifdef DREAMCAST
	    if (__sdl_dc_emulate_mouse)
	    {
	    	if (vkbd_mode) break;
	    	if (rEvent.button.button==5) {
			if (vkbd_button3==(SDLKey)0)
			    buttonstate[0] = 0;
			else
				break;
		}
	    	else if (rEvent.button.button==1) {
			if (vkbd_button4==(SDLKey)0)
			    buttonstate[2] = 0;
			else
				break;
		}
	    }
	    else
	    	if (rEvent.button.button)
			buttonstate[2]=0;
		else
			buttonstate[0]=0;
				
#else
	    	buttonstate[(rEvent.button.button-1)%3] = 0;
#endif
	    break;
	case SDL_MOUSEMOTION:
#ifdef DEBUG_EVENTS
	    dbg("Event: mouse motion");
#endif
	    lastmx += rEvent.motion.xrel<<1;
	    lastmy += rEvent.motion.yrel<<1;
	    newmousecounters = 1;
	    break;
	}
    }
#endif

#if !defined(DREAMCAST) && !defined(DINGOO)
    if (SDL_MUSTLOCK(prSDLScreen))
    	SDL_LockSurface (prSDLScreen);
#endif

    /* Handle UAE reset */
/*
    if ((uae4all_keystate[AK_CTRL] || uae4all_keystate[AK_RCTRL]) && uae4all_keystate[AK_LAMI] && uae4all_keystate[AK_RAMI])
	uae_reset ();
*/
#ifdef DEBUG_EVENTS
    dbg(" handle_events -> terminado");
#endif
}

int check_prefs_changed_gfx (void)
{
	extern int mainMenu_vpos;
	static int last_vpos=0;
	int ret=(last_vpos!=mainMenu_vpos);
	last_vpos=mainMenu_vpos;
	return ret;
}

int debuggable (void)
{
    return 1;
}

int needmousehack (void)
{
    return 1;
}

void gui_purge_events(void)
{
	SDL_Event event;
	SDL_Delay(150);
	while(SDL_PollEvent(&event))
		SDL_Delay(10);
	keybuf_init();
}

