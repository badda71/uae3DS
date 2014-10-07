 /* 
  * UAE - The Un*x Amiga Emulator
  * 
  * Support for Linux/USS sound
  * 
  * Copyright 1997 Bernd Schmidt
  */

#include "sysconfig.h"
#include "sysdeps.h"

#include "config.h"
#include "uae.h"
#include "options.h"
#include "memory.h"
#include "debug_uae4all.h"
#include "events.h"
#include "custom.h"
#include "gensound.h"
#include "sound.h"
#include "audio.h"

#include "custom.h"
#include "xwin.h"
#include "drawing.h"

#ifdef DREAMCAST
#include <SDL_dreamcast.h>
#endif


extern unsigned long next_sample_evtime;

int produce_sound=0;
int changed_produce_sound=0;

#define DEFAULT_SOUND_FREQ_ADJUST DEFAULT_SOUND_FREQ

#ifndef NO_THREADS
#define USE_SOUND_SEMS 1
#define USE_SOUND_WAIT 1
#define SOUND_EVTIME_SUPER_THROTTLE (MAXHPOS_PAL*244*VBLANK_HZ_PAL*CYCLE_UNIT/DEFAULT_SOUND_FREQ)
#define SOUND_EVTIME_THROTTLE	(MAXHPOS_PAL*270*VBLANK_HZ_PAL*CYCLE_UNIT/DEFAULT_SOUND_FREQ)
#define SOUND_EVTIME_NORMAL	(MAXHPOS_PAL*312 *VBLANK_HZ_PAL*CYCLE_UNIT/DEFAULT_SOUND_FREQ)
#else
#define SOUND_EVTIME_SUPER_THROTTLE (MAXHPOS_PAL*240*VBLANK_HZ_PAL*CYCLE_UNIT/DEFAULT_SOUND_FREQ)
#define SOUND_EVTIME_THROTTLE	(MAXHPOS_PAL*256*VBLANK_HZ_PAL*CYCLE_UNIT/DEFAULT_SOUND_FREQ)
#define SOUND_EVTIME_NORMAL	(MAXHPOS_PAL*282 *VBLANK_HZ_PAL*CYCLE_UNIT/DEFAULT_SOUND_FREQ)
#endif

static uae_u16 sndbuffer_all[(SNDBUFFER_LEN*8)+(32*8)] UAE4ALL_ALIGN;
static uae_u16 *sndbuffer[8] __attribute__ ((__aligned__ (32))) = {
	&sndbuffer_all[0],
	&sndbuffer_all[(SNDBUFFER_LEN*1)+(32*1)],
	&sndbuffer_all[(SNDBUFFER_LEN*2)+(32*2)],
	&sndbuffer_all[(SNDBUFFER_LEN*3)+(32*3)],
	&sndbuffer_all[(SNDBUFFER_LEN*4)+(32*4)],
	&sndbuffer_all[(SNDBUFFER_LEN*5)+(32*5)],
	&sndbuffer_all[(SNDBUFFER_LEN*6)+(32*6)],
	&sndbuffer_all[(SNDBUFFER_LEN*7)+(32*7)],
};

unsigned n_callback_sndbuff, n_render_sndbuff;
uae_u16 *callback_sndbuff, *render_sndbuff, *sndbufpt;

#ifndef PROFILER_UAE4ALL

int tablas_ajuste[8][9]=
{
	{ 9 , 9 , 9 , 9 , 9 , 9 , 9 , 9 , 9 },	// 0
	{ 3 , 4 , 3 , 4 , 3 , 4 , 3 , 4 , 3 },	// 1 
	{ 1 , 2 , 1 , 2 , 1 , 2 , 1 , 2 , 1 },	// 2 
	{ 1 , 0 , 1 , 0 , 1 , 0 , 1 , 0 , 1 },	// 3 
	{ 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },	// 4 
	{ 0 ,-1 , 0 ,-1 , 0 ,-1 , 0 ,-1 , 0 },	// 5 
	{-2 ,-1 ,-2 ,-1 ,-2 ,-1 ,-2 ,-1 ,-2 },	// 6 
	{-4 ,-3 ,-4 ,-3 ,-4 ,-3 ,-4 ,-3 ,-4 },	// 7
};

#else

int tablas_ajuste[8][9]=
{
	{ 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
	{ 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
	{ 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
	{ 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
	{ 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
	{ 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
	{ 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
	{ 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
};

#endif


int *tabla_ajuste=(int *)&tablas_ajuste[4];

#ifdef NO_SOUND


void finish_sound_buffer (void) {  }

int setup_sound (void) { sound_available = 0; return 0; }

void close_sound (void) { }

int init_sound (void) { return 0; }

void pause_sound (void) { }

void resume_sound (void) { }

void uae4all_init_sound(void) { }

void uae4all_play_click(void) { }

void uae4all_pause_music(void) { }

void uae4all_resume_music(void) { }

#else 

#include "thread.h"
#include <SDL.h>

#ifdef USE_SOUND_SEMS
static uae_sem_t data_available_sem,callback_done_sem;
#endif


#ifdef MENU_MUSIC
#include <SDL_mixer.h>
enum{
	SAMPLE_CLICK,
	NUM_SAMPLES
};
static char *sample_filename[NUM_SAMPLES]={
	DATA_PREFIX "click.wav"
};
static Mix_Chunk *sample_wave[NUM_SAMPLES];
#define play_sample(NSAMPLE) Mix_PlayChannel(0,sample_wave[(NSAMPLE)],0)
#define play_sampleS(NSAMPLE) Mix_PlayChannel(-1,sample_wave[(NSAMPLE)],0)
#endif

int sound_fd;
static int have_sound = 0;
static unsigned long formats;

static SDL_AudioSpec spec;

static int in_callback, closing_sound=-1;


void sound_default_evtime(void)
{
    switch(m68k_speed)
    {
	case 0:
	case 1:
	    	scaled_sample_evtime=SOUND_EVTIME_NORMAL;
		break;
	case 2:
	case 3:
		scaled_sample_evtime=SOUND_EVTIME_THROTTLE;
		break;
	default:
		scaled_sample_evtime=SOUND_EVTIME_SUPER_THROTTLE;
    }
    schedule_audio();
    n_render_sndbuff=4;
    sndbufpt=render_sndbuff=sndbuffer[4];
    n_callback_sndbuff=0;
    callback_sndbuff=sndbuffer[0];
}


#ifdef DEBUG_FRAMERATE
double media_ratio=0;
static __inline__ void calcule_audio_ratio(void)
{
	unsigned long long ahora=timer_us_gettime64();
	static unsigned long long antes=0;

	if (antes)
	{
		if (media_ratio!=0)
			media_ratio=(media_ratio+(((double)(ahora-antes))))/2.0;
		else
			media_ratio=(double)(ahora-antes);
		antes=ahora;
	}
	else
		antes=ahora;
}

unsigned sound_cuantos[8]={ 0,0,0,0,0,0,0,0 };
unsigned sound_ajustes=0;
unsigned sound_alcanza_callback=0;
unsigned sound_alcanza_render=0;
#endif

static void sound_callback (void *userdata, Uint8 *stream, int len)
{
#ifdef DEBUG_FRAMERATE
    calcule_audio_ratio();
#endif
#ifdef DEBUG_SOUND
    dbg("sound.c : sound_callback");
#endif
    in_callback = 1;
    if (! closing_sound) {
#ifdef USE_SOUND_SEMS
         while ((!closing_sound) && (((unsigned)render_sndbuff)==((unsigned)callback_sndbuff)))
	 		uae_sem_wait (&data_available_sem);
#else
	n_callback_sndbuff=(n_callback_sndbuff+1)&7;
	callback_sndbuff=sndbuffer[n_callback_sndbuff];
	if (n_callback_sndbuff==n_render_sndbuff)
	{
    		n_render_sndbuff=6;
    		sndbufpt=render_sndbuff=sndbuffer[6];
    		n_callback_sndbuff=0;
		callback_sndbuff=sndbuffer[0];
#ifdef DEBUG_FRAMERATE
		sound_alcanza_callback++;
#endif
	}
#endif
#ifdef USE_SOUND_SEMS
	 if (((unsigned)render_sndbuff)!=((unsigned)callback_sndbuff)) 
#endif
	 {
#ifndef DREAMCAST
 		memcpy(stream,callback_sndbuff,len);
#else
		SDL_DC_SetSoundBuffer(callback_sndbuff);
#endif
#if defined(USE_SOUND_SEMS) && defined(USE_SOUND_WAIT)
		callback_sndbuff=render_sndbuff;
		uae_sem_post (&callback_done_sem);
#endif
	 }
#if defined(DEBUG_SOUND) && defined(USE_SOUND_SEMS)
	 else
		dbg("SOUND TIMEOUT");
#endif
    }
    in_callback = 0;
#ifdef DEBUG_SOUND
    dbg(" sound.c : ! sound_callback");
#endif
}

void finish_sound_buffer (void)
{
#ifdef DEBUG_SOUND
	dbg("sound.c : finish_sound_buffer");
#endif
#ifndef USE_SOUND_SEMS
	n_render_sndbuff=(n_render_sndbuff+1)&7;
	render_sndbuff=sndbufpt=sndbuffer[n_render_sndbuff];
	tabla_ajuste=(int *)&tablas_ajuste[(n_callback_sndbuff-n_render_sndbuff)&7];
#ifdef DEBUG_FRAMERATE
	sound_cuantos[(n_callback_sndbuff-n_render_sndbuff)&7]++;
	sound_ajustes++;
#endif
	if (n_callback_sndbuff==n_render_sndbuff)
	{
		n_render_sndbuff=0;
		sndbufpt=render_sndbuff=sndbuffer[0];
		n_callback_sndbuff=6;
		callback_sndbuff=sndbuffer[6];
#ifdef DEBUG_FRAMERATE
		sound_alcanza_render++;
#endif
	}
#endif

	closing_sound=0;
#ifdef USE_SOUND_SEMS
	uae_sem_post (&data_available_sem);
#ifdef USE_SOUND_WAIT
	while ((!closing_sound) && (callback_sndbuff!=render_sndbuff))
		uae_sem_wait (&callback_done_sem);
	if (callback_sndbuff==sndbuffer[0])
		render_sndbuff=sndbufpt=sndbuffer[4];
	else
		render_sndbuff=sndbufpt=sndbuffer[0];
#endif
#endif
#ifdef DEBUG_SOUND
	dbg(" sound.c : ! finish_sound_buffer");
#endif
}

static int get_soundbuf_size(void)
{
	int size=SNDBUFFER_LEN>>1;
    	int channels = 1;
	return (size * DEFAULT_SOUND_BITS / 8 * channels);
}

/* Try to determine whether sound is available.  This is only for GUI purposes.  */
int setup_sound (void)
{
#ifdef DEBUG_SOUND
    dbg("sound.c : setup_sound");
#endif
    spec.freq = DEFAULT_SOUND_FREQ_ADJUST;
    spec.format = AUDIO_S16;
    spec.channels = 1;
    spec.samples = SNDBUFFER_LEN>>1;
    spec.callback = sound_callback;
    spec.userdata = NULL;
// #ifndef DREAMCAST
#if 0
    if (SDL_OpenAudio (&spec, NULL) < 0) {
	write_log ("Couldn't open audio: %s\n", SDL_GetError());
	return 0;
    }
    SDL_CloseAudio ();
#endif
    sound_available = 1;
#ifdef DEBUG_SOUND
    dbg(" sound.c : ! setup_sound");
#endif
    return 1;
}

static int open_sound (void)
{
#ifdef DEBUG_SOUND
    dbg("sound.c : open_sound");
#endif

#ifndef MENU_MUSIC
    static int passed=0;
#endif

    spec.freq = DEFAULT_SOUND_FREQ_ADJUST;
    spec.format = DEFAULT_SOUND_BITS == 8 ? AUDIO_U8 : AUDIO_S16;
    spec.channels = 1;
    spec.samples = SNDBUFFER_LEN>>1;
    spec.callback = sound_callback;
    spec.userdata = 0;

#ifndef MENU_MUSIC
    if (!passed)
{
//printf("SDL_OpenAudio %i %i %i\n",spec.freq,spec.channels,spec.samples);
	if (SDL_OpenAudio (&spec, NULL) < 0)
	{
//puts(SDL_GetError()); exit(0);
		write_log (stderr, "Couldn't open audio: %s\n", SDL_GetError());
		return 0;
    	}
//puts("DESPUES");
}
#else
    SDL_PauseAudio (1);
    Mix_HookMusic(&sound_callback,NULL);
#endif
    have_sound = 1;

//    sound_default_evtime(0);
    scaled_sample_evtime_ok = 1;

    sound_available = 1;
    write_log ("SDL sound driver found and configured for %d bits at %d Hz, buffer is %d samples\n",
	       DEFAULT_SOUND_BITS, spec.freq, spec.samples);
    n_callback_sndbuff=0;
    callback_sndbuff=sndbuffer[n_callback_sndbuff];
    n_render_sndbuff=4;
    render_sndbuff=sndbufpt=sndbuffer[n_render_sndbuff];
#ifndef MENU_MUSIC
    passed++;
#endif
#ifdef DEBUG_SOUND
    dbg(" sound.c : ! open_sound");
#endif
    return 1;
}

void close_sound (void)
{
#ifdef DEBUG_SOUND
    dbg("sound.c : close_sound");
#endif
    if (! have_sound)
	return;

    SDL_PauseAudio (1);
    if (in_callback) {
	closing_sound = -1;
#ifdef USE_SOUND_SEMS
	uae_sem_post (&data_available_sem);
#endif
    }
    SDL_Delay(333);
//    SDL_CloseAudio ();
    have_sound = 0;
#ifdef DEBUG_SOUND
    dbg(" sound.c : ! close_sound");
#endif
}

int init_sound (void)
{
#ifdef DEBUG_SOUND
    dbg("sound.c : init_sound");
#endif
    in_callback = 0;
    closing_sound = -1;

    have_sound=open_sound();

    SDL_PauseAudio (0);

#ifdef DEBUG_SOUND
    dbg(" sound.c : ! init_sound");
#endif
    return have_sound;
}

void pause_sound (void)
{
#ifdef DEBUG_SOUND
    dbg("sound.c : pause_sound");
#endif
    closing_sound=-1;
#ifdef USE_SOUND_SEMS
    uae_sem_post (&data_available_sem);
#endif
    SDL_Delay(333);
    SDL_PauseAudio (1);
#ifdef DREAMCAST
    SDL_DC_RestoreSoundBuffer();
#endif
#ifdef MENU_MUSIC
    Mix_HookMusic(NULL,NULL);
    Mix_VolumeMusic(MUSIC_VOLUME);
    SDL_PauseAudio(0);
#endif
#ifdef DEBUG_SOUND
    dbg(" sound.c : ! pause_sound");
#endif
}

void resume_sound (void)
{
#ifdef DEBUG_SOUND
    dbg("sound.c : resume_sound");
#endif
#ifdef MENU_MUSIC
    SDL_PauseAudio (1);
    Mix_HookMusic(&sound_callback,NULL);
#endif
    SDL_PauseAudio (0);
//    closing_sound=0;
#ifdef DEBUG_SOUND
    dbg(" sound.c : ! resume_sound");
#endif
}

void uae4all_init_sound(void)
{
#ifdef DEBUG_SOUND
    dbg("sound.c : uae4all_init_sound");
#endif
#ifdef MENU_MUSIC
	unsigned i;
    	int freq = DEFAULT_SOUND_FREQ_ADJUST;
    	int format = DEFAULT_SOUND_BITS == 8 ? AUDIO_U8 : AUDIO_S16;
    	int channels = 1;
    	int samples = SNDBUFFER_LEN>>1;
	Mix_OpenAudio(freq, format, channels, samples);
#ifdef DEBUG_SOUND
	dbgf("Freq=%i, Channels=%i, Buff=%i\n",freq,channels,samples);
#endif
	for(i=0;i<NUM_SAMPLES;i++)
		sample_wave[i]=Mix_LoadWAV(sample_filename[i]);
	Mix_PlayMusic(Mix_LoadMUS(DATA_PREFIX "music.mod"),-1);
	Mix_VolumeMusic(MUSIC_VOLUME);
#ifdef USE_SOUND_SEMS
	uae_sem_init (&data_available_sem, 0, 0);
	uae_sem_init (&callback_done_sem, 0, 0);
#endif
#endif
#ifdef DEBUG_SOUND
    dbg(" sound.c : ! uae4all_init_sound");
#endif
}

void uae4all_pause_music(void)
{
#ifdef DEBUG_SOUND
    dbg("sound.c : pause_music");
#endif
#ifdef MENU_MUSIC
//    closing_sound=-1;
    SDL_PauseAudio (1);
#endif
#ifdef DEBUG_SOUND
    dbg(" sound.c : ! pause_music");
#endif
}

void uae4all_resume_music(void)
{
#ifdef DEBUG_SOUND
    dbg("sound.c : resume_music");
#endif
#ifdef MENU_MUSIC
    SDL_PauseAudio (1);
#endif
#ifdef DEBUG_SOUND
    dbg(" sound.c : ! resume_music");
#endif
}

void uae4all_play_click(void)
{
#ifdef MENU_MUSIC
	play_sampleS(SAMPLE_CLICK);
#endif
}
#endif

