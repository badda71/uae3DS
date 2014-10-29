 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Screen drawing functions
  *
  * Copyright 1995-2000 Bernd Schmidt
  * Copyright 1995 Alessandro Bissacco
  * Copyright 2000,2001 Toni Wilen
  */

// #define USE_DRAWING_EXTRA_INLINE
#define UNROLL_PFIELD
#define UNROLL_DRAW_SPRITES

/* There are a couple of concepts of "coordinates" in this file.
   - DIW coordinates
   - DDF coordinates (essentially cycles, resolution lower than lores by a factor of 2)
   - Pixel coordinates
     * in the Amiga's resolution as determined by BPLCON0 ("Amiga coordinates")
     * in the window resolution as determined by the preferences ("window coordinates").
     * in the window resolution, and with the origin being the topmost left corner of
       the window ("native coordinates")
   One note about window coordinates.  The visible area depends on the width of the
   window, and the centering code.  The first visible horizontal window coordinate is
   often _not_ 0, but the value of VISIBLE_LEFT_BORDER instead.

   One important thing to remember: DIW coordinates are in the lowest possible
   resolution.

   To prevent extremely bad things (think pixels cut in half by window borders) from
   happening, all ports should restrict window widths to be multiples of 16 pixels.  */

#include "sysconfig.h"
#include "sysdeps.h"

#include <ctype.h>
#include <assert.h>

#include "config.h"
#include "uae.h"
#include "options.h"
#include "thread.h"
#include "memory.h"
#include "custom.h"
#include "m68k/m68k_intrf.h"
#include "xwin.h"
#include "autoconf.h"
#include "gui.h"
#include "drawing.h"
#include "savestate.h"
#include "sound.h"
#include "debug_uae4all.h"

#ifdef USE_DRAWING_EXTRA_INLINE
#define _INLINE_ __inline__
#else
#define _INLINE_ 
#endif


#define GFXVIDINFO_PIXBYTES 2
#define GFXVIDINFO_WIDTH 320
#define GFXVIDINFO_HEIGHT 240
#define MAXBLOCKLINES 240
#define VISIBLE_LEFT_BORDER 72
#define VISIBLE_RIGHT_BORDER 392
#define LINETOSCR_X_ADJUST_BYTES 144
/*
#define VISIBLE_LEFT_BORDER 64
#define VISIBLE_RIGHT_BORDER 384
#define LINETOSCR_X_ADJUST_BYTES 128
*/

#define maxhpos MAXHPOS

/* The shift factor to apply when converting between Amiga coordinates and window
   coordinates.  Zero if the resolution is the same, positive if window coordinates
   have a higher resolution (i.e. we're stretching the image), negative if window
   coordinates have a lower resolution (i.e. we're shrinking the image).  */
static int res_shift;

static int interlace_seen = 0;

extern int drawfinished;

/* Lookup tables for dual playfields.  The dblpf_*1 versions are for the case
   that playfield 1 has the priority, dbplpf_*2 are used if playfield 2 has
   priority.  If we need an array for non-dual playfield mode, it has no number.  */
/* The dbplpf_ms? arrays contain a shift value.  plf_spritemask is initialized
   to contain two 16 bit words, with the appropriate mask if pf1 is in the
   foreground being at bit offset 0, the one used if pf2 is in front being at
   offset 16.  */

static int dblpf_ms1[256], dblpf_ms2[256], dblpf_ms[256];
static int dblpf_ind1[256], dblpf_ind2[256];

static int dblpf_2nd1[256], dblpf_2nd2[256];

static int dblpfofs[] = { 0, 2, 4, 8, 16, 32, 64, 128 };

static int sprite_offs[256];

static uae_u32 clxtab[256];

/* Video buffer description structure. Filled in by the graphics system
 * dependent code. */

/* OCS/ECS color lookup table. */
xcolnr xcolors[4096];

struct color_entry colors_for_drawing;

/* The size of these arrays is pretty arbitrary; it was chosen to be "more
   than enough".  The coordinates used for indexing into these arrays are
   almost, but not quite, Amiga coordinates (there's a constant offset).  */
static union {
    /* Let's try to align this thing. */
    double uupzuq;
    long int cruxmedo;
    uae_u8 apixels[MAX_PIXELS_PER_LINE * 2];
    uae_u16 apixels_w[MAX_PIXELS_PER_LINE * 2 / 2];
    uae_u32 apixels_l[MAX_PIXELS_PER_LINE * 2 / 4];
} pixdata UAE4ALL_ALIGN;


uae_u16 spixels[2 * MAX_SPR_PIXELS];
/* Eight bits for every pixel.  */
union sps_union spixstate;

char *xlinebuffer;

static int *amiga2aspect_line_map, *native2amiga_line_map;
static char *row_map[2049] UAE4ALL_ALIGN;
static int max_drawn_amiga_line;

/* line_draw_funcs: pfield_do_linetoscr, pfield_do_fill_line */
typedef void (*line_draw_func)(int, int);

#define LINE_UNDECIDED 1
#define LINE_DECIDED 2
#define LINE_DONE 7

static char *line_drawn;
#ifdef USE_LINESTATE
static char linestate[(MAXVPOS + 1)*2 + 32] UAE4ALL_ALIGN;
#endif

uae_u8 line_data[(MAXVPOS + 1) * 2][MAX_PLANES * MAX_WORDS_PER_LINE * 2] UAE4ALL_ALIGN;

/* Centering variables.  */
static int min_diwstart, max_diwstop;
static int thisframe_y_adjust;
static int thisframe_y_adjust_real, max_ypos_thisframe, min_ypos_for_screen;
static int extra_y_adjust;

/* A frame counter that forces a redraw after at least one skipped frame in
   interlace mode.  */
static int last_redraw_point;

#ifdef USE_RASTER_DRAW
static int first_drawn_line, last_drawn_line;
static int first_block_line, last_block_line;
#endif

/* These are generated by the drawing code from the line_decisions array for
   each line that needs to be drawn.  These are basically extracted out of
   bit fields in the hardware registers.  */
static int bpldualpf, bpldualpfpri, bpldualpf2of, bplplanecnt, bplres;
static uae_u32 plf_sprite_mask;
static int sbasecol[2];

int inhibit_frame;

int framecnt = 0;
#ifndef USE_ALL_LINES
int framecnt_hack = 0;
#else
#define framecnt_hack 0
#endif
#ifdef USE_RASTER_DRAW
static int frame_redraw_necessary;
#else
#define frame_redraw_necessary 1
#endif

#if defined(NO_THREADS) && defined(DREAMCAST)
// SOLO PARA DREAMCAST PORQUE TIENE UN SOUNDBUFFER DE 960
#define UMBRAL 21
#else
#define UMBRAL 20
#endif
#define PARTIDA (UMBRAL/2)

extern Uint32 uae4all_numframes;

#ifdef DEBUG_FRAMERATE
extern Uint32 uae4all_frameskipped;
extern double uae4all_framerate;
#endif

static Uint32 proximo_frameskip;
extern int *tabla_ajuste;

void reset_frameskip()
{
	proximo_frameskip=SDL_GetTicks();
}

static __inline__ void count_frame (void)
{
    uae4all_numframes++;

#ifdef AUTO_PROFILER
    if (uae4all_numframes==AUTO_PROFILER)
    {
	    puts("PROFILER..."); fflush(stdout);
	    prefs_gfx_framerate=0;
#ifdef AUTO_PROFILER_SOUND
	    changed_produce_sound=2;
	    sound_default_evtime();
	    check_prefs_changed_audio();
#endif
	    uae4all_prof_init();
    }
#ifdef MAX_AUTO_PROFILER
    else if (uae4all_numframes==MAX_AUTO_PROFILER)
    {
	    uae4all_prof_show();
	    exit(0);
    }
#endif

#endif

#ifdef DEBUG_FRAMERATE
#ifdef AUTO_FRAMERATE
    if (uae4all_numframes==AUTO_FRAMERATE)
    {
	    uae4all_frameskipped=0;
	    prefs_gfx_framerate=-1;
#ifdef AUTO_FRAMERATE_SOUND
	    changed_produce_sound=2;
	    sound_default_evtime();
	    check_prefs_changed_audio();
#endif
    }
    else if (uae4all_numframes>AUTO_FRAMERATE)
    {
	static Uint32 start_time=0;
	static Uint32 start_numframes=0;

	if (!start_time)
	{
		start_time=SDL_GetTicks();
		start_numframes=uae4all_numframes;
	}
	else
	{
		Uint32 now=SDL_GetTicks();
		if (now-start_time>=1000)
		{
			if (uae4all_framerate!=0.0)
				uae4all_framerate=(uae4all_framerate+((double)(uae4all_numframes-start_numframes)))/2.0;
			else
				uae4all_framerate=(double)(uae4all_numframes-start_numframes);
			start_time=now;
			start_numframes=uae4all_numframes;
		}
	}
	    
    }
#ifdef MAX_AUTO_FRAMERATE
    if (uae4all_numframes==MAX_AUTO_FRAMERATE)
    {
	    uae4all_numframes-=AUTO_FRAMERATE;
	    uae4all_show_time();
	    exit(0);
    }
#endif
#endif
#endif
    if (prefs_gfx_framerate>=0)
    {
    	framecnt++;
    	if (framecnt > prefs_gfx_framerate)
		framecnt = 0;
#ifdef DEBUG_FRAMERATE
	else
		uae4all_frameskipped++;
#endif
    }
    else
    {
	static int cuantos=0;

	Uint32 ahora=SDL_GetTicks();
	proximo_frameskip+=UMBRAL;

#ifdef NO_THREADS
	if (!produce_sound)
		proximo_frameskip--;
	else
		proximo_frameskip+=(tabla_ajuste[uae4all_numframes%9]);
#endif
	if ((ahora-PARTIDA)>proximo_frameskip)
	{	
		cuantos++;
		if (cuantos>5)
		{
			proximo_frameskip=ahora+2;
			framecnt=0;
			cuantos=0;
		}
		else
		{
			framecnt=1;
#ifdef DEBUG_FRAMERATE
			uae4all_frameskipped++;
#endif
		}
	}
	else
	{
		if ((ahora+PARTIDA)<proximo_frameskip)
		{
#ifdef DREAMCAST
//			SDL_Delay(proximo_frameskip-ahora-PARTIDA+1);
#else
			SDL_Delay(proximo_frameskip-ahora);
#endif
			proximo_frameskip=SDL_GetTicks();
		}
		framecnt=0;
		cuantos=0;
	}
    }
}

int coord_native_to_amiga_x (int x)
{
    x += VISIBLE_LEFT_BORDER;
    x <<= 1;
    return x + 2*DISPLAY_LEFT_SHIFT - 2*DIW_DDF_OFFSET;
}

int coord_native_to_amiga_y (int y)
{
    return native2amiga_line_map[y] + thisframe_y_adjust - minfirstline;
}

static __inline__ int res_shift_from_window (int x)
{
    if (res_shift >= 0)
	return x >> res_shift;
    return x << -res_shift;
}

static __inline__ int res_shift_from_amiga (int x)
{
    if (res_shift >= 0)
	return x >> res_shift;
    return x << -res_shift;
}

#ifdef USE_RASTER_DRAW
void notice_screen_contents_lost (void)
{
    frame_redraw_necessary = 2;

}
#endif

static struct decision *dp_for_drawing;
static struct draw_info *dip_for_drawing;

/* Record DIW of the current line for use by centering code.  */
void record_diw_line (int first, int last)
{
    if (last > max_diwstop)
	max_diwstop = last;
    if (first < min_diwstart)
	min_diwstart = first;
}

/*
 * Screen update macros/functions
 */

/* The important positions in the line: where do we start drawing the left border,
   where do we start drawing the playfield, where do we start drawing the right border.
   All of these are forced into the visible window (VISIBLE_LEFT_BORDER .. VISIBLE_RIGHT_BORDER).
   PLAYFIELD_START and PLAYFIELD_END are in window coordinates.  */
static int playfield_start, playfield_end;
static int pixels_offset;
static int src_pixel;
/* How many pixels in window coordinates which are to the left of the left border.  */
static int unpainted;

#define LNAME pfiled_do_linetoscr_0
#define SRC_INC 1
#include "linetoscr.h"
#undef SRC_INC
#undef LNAME

#define LNAME pfiled_do_linetoscr_1
#define SRC_INC 2
#include "linetoscr.h"
#undef SRC_INC
#undef LNAME

#define LNAME pfiled_do_linetoscr_0_dual
#define SRC_INC 1
#include "linetoscr2.h"
#undef SRC_INC
#undef LNAME

#define LNAME pfiled_do_linetoscr_1_dual
#define SRC_INC 2
#include "linetoscr2.h"
#undef SRC_INC
#undef LNAME


static line_draw_func *pfield_do_linetoscr=(line_draw_func *)pfiled_do_linetoscr_0;

static void pfield_do_fill_line(int start, int stop)
{
    register uae_u16 *b = &(((uae_u16 *)xlinebuffer)[start]);
    register xcolnr col = colors_for_drawing.acolors[0];
    register int i;
    register int max=(stop-start);
    for (i = 0; i < max; i++,b++)
	*b = col;
}

/* Initialize the variables necessary for drawing a line.
 * This involves setting up start/stop positions and display window
 * borders.  */
static _INLINE_ void pfield_init_linetoscr (void)
{
    /* First, get data fetch start/stop in DIW coordinates.  */
    int ddf_left = (dp_for_drawing->plfleft << 1) + DIW_DDF_OFFSET;
    int ddf_right = (dp_for_drawing->plfright << 1) + DIW_DDF_OFFSET;
    /* Compute datafetch start/stop in pixels; native display coordinates.  */
    int native_ddf_left = coord_hw_to_window_x (ddf_left);
    int native_ddf_right = coord_hw_to_window_x (ddf_right);

    int linetoscr_diw_start = dp_for_drawing->diwfirstword;
    int linetoscr_diw_end = dp_for_drawing->diwlastword;

    if (dip_for_drawing->nr_sprites == 0) {
	if (linetoscr_diw_start < native_ddf_left)
	    linetoscr_diw_start = native_ddf_left;
	if (linetoscr_diw_end > native_ddf_right)
	    linetoscr_diw_end = native_ddf_right;
    }

    /* Perverse cases happen. */
    if (linetoscr_diw_end < linetoscr_diw_start)
	linetoscr_diw_end = linetoscr_diw_start;

    playfield_start = linetoscr_diw_start;
    playfield_end = linetoscr_diw_end;

    if (playfield_start < VISIBLE_LEFT_BORDER)
	playfield_start = VISIBLE_LEFT_BORDER;
    if (playfield_start > VISIBLE_RIGHT_BORDER)
	playfield_start = VISIBLE_RIGHT_BORDER;
    if (playfield_end < VISIBLE_LEFT_BORDER)
	playfield_end = VISIBLE_LEFT_BORDER;
    if (playfield_end > VISIBLE_RIGHT_BORDER)
	playfield_end = VISIBLE_RIGHT_BORDER;

    /* Now, compute some offsets.  */

    res_shift = - bplres;
    if (res_shift) {
	if (bpldualpf)
		pfield_do_linetoscr=(line_draw_func *)pfiled_do_linetoscr_1_dual;
	else
		pfield_do_linetoscr=(line_draw_func *)pfiled_do_linetoscr_1;
    } else {
	if (bpldualpf)
		pfield_do_linetoscr=(line_draw_func *)pfiled_do_linetoscr_0_dual;
	else
		pfield_do_linetoscr=(line_draw_func *)pfiled_do_linetoscr_0;
    }
    ddf_left -= DISPLAY_LEFT_SHIFT;
    ddf_left <<= bplres;
    pixels_offset = MAX_PIXELS_PER_LINE - ddf_left;

    unpainted = VISIBLE_LEFT_BORDER < playfield_start ? 0 : VISIBLE_LEFT_BORDER - playfield_start;
    src_pixel = MAX_PIXELS_PER_LINE + res_shift_from_window (playfield_start - native_ddf_left + unpainted);

    if (dip_for_drawing->nr_sprites == 0)
	return;
    /* Must clear parts of apixels.  */
    if (linetoscr_diw_start < native_ddf_left) {
	int size = res_shift_from_window (native_ddf_left - linetoscr_diw_start);
	linetoscr_diw_start = native_ddf_left;
	bzero(pixdata.apixels + MAX_PIXELS_PER_LINE - size, size);
    }
    if (linetoscr_diw_end > native_ddf_right) {
	int pos = res_shift_from_window (native_ddf_right - native_ddf_left);
	int size = res_shift_from_window (linetoscr_diw_end - native_ddf_right);
	linetoscr_diw_start = native_ddf_left;
	uae4all_memclr(pixdata.apixels + MAX_PIXELS_PER_LINE + pos, size);
    }
}


static __inline__ void fill_line (void)
{
    int nints, nrem;
    int *start;
    xcolnr val;

    nints = GFXVIDINFO_WIDTH >> 1;
    nrem = nints & 7;
    nints &= ~7;
    start = (int *)(((char *)xlinebuffer) + (VISIBLE_LEFT_BORDER << 1));
    val = colors_for_drawing.acolors[0];
    val |= val << 16;
#ifdef DEBUG_BLITTER
    dbgf("fill_line -> nints=%i, nrem=%i, val=%i\n",nints,nrem,val);
#endif
    for (; nints > 0; nints -= 8, start += 8) {
	*start = val;
	*(start+1) = val;
	*(start+2) = val;
	*(start+3) = val;
	*(start+4) = val;
	*(start+5) = val;
	*(start+6) = val;
	*(start+7) = val;
    }

    switch (nrem) {
     case 7:
	*start++ = val;
     case 6:
	*start++ = val;
     case 5:
	*start++ = val;
     case 4:
	*start++ = val;
     case 3:
	*start++ = val;
     case 2:
	*start++ = val;
     case 1:
	*start = val;
    }
}

static __inline__ void gen_pfield_tables (void)
{
    int i;

    /* For now, the AJA stuff is broken in the dual playfield case. We encode
     * sprites in dpf mode by ORing the pixel value with 0x80. To make dual
     * playfield rendering easy, the lookup tables contain are made linear for
     * values >= 128. That only works for OCS/ECS, though. */

    for (i = 0; i < 256; i++) {
	int plane1 = (i & 1) | ((i >> 1) & 2) | ((i >> 2) & 4) | ((i >> 3) & 8);
	int plane2 = ((i >> 1) & 1) | ((i >> 2) & 2) | ((i >> 3) & 4) | ((i >> 4) & 8);

	dblpf_2nd1[i] = plane1 == 0 ? (plane2 == 0 ? 0 : 2) : 1;
	dblpf_2nd2[i] = plane2 == 0 ? (plane1 == 0 ? 0 : 1) : 2;

	dblpf_ms1[i] = plane1 == 0 ? (plane2 == 0 ? 16 : 8) : 0;
	dblpf_ms2[i] = plane2 == 0 ? (plane1 == 0 ? 16 : 0) : 8;
	dblpf_ms[i] = i == 0 ? 16 : 8;
	if (plane2 > 0)
	    plane2 += 8;
	dblpf_ind1[i] = i >= 128 ? i & 0x7F : (plane1 == 0 ? plane2 : plane1);
	dblpf_ind2[i] = i >= 128 ? i & 0x7F : (plane2 == 0 ? plane1 : plane2);

	sprite_offs[i] = (i & 15) ? 0 : 2;

	clxtab[i] = ((((i & 3) && (i & 12)) << 9)
		     | (((i & 3) && (i & 48)) << 10)
		     | (((i & 3) && (i & 192)) << 11)
		     | (((i & 12) && (i & 48)) << 12)
		     | (((i & 12) && (i & 192)) << 13)
		     | (((i & 48) && (i & 192)) << 14));
    }
}


#ifndef UNROLL_DRAW_SPRITES

static __inline__ void draw_sprites_1 (struct sprite_entry *_GCCRES_ e, int dualpf,
				   int doubling, int has_attach)
{
    int *shift_lookup = dualpf ? (bpldualpfpri ? dblpf_ms2 : dblpf_ms1) : dblpf_ms;
    uae_u16 *buf = spixels + e->first_pixel;
    uae_u8 *stbuf = spixstate.bytes + e->first_pixel;
    int pos, window_pos;
    uae_u8 xor_val = (uae_u8)(dp_for_drawing->bplcon4 >> 8);

    buf -= e->pos;
    stbuf -= e->pos;

    window_pos = e->pos + ((DIW_DDF_OFFSET - DISPLAY_LEFT_SHIFT) );
    if (doubling)
	window_pos <<= 1;
    window_pos += pixels_offset;
    for (pos = e->pos; pos < e->max; pos += 1) {
	int maskshift, plfmask;
	unsigned int v = buf[pos];

	maskshift = shift_lookup[pixdata.apixels[window_pos]];
	plfmask = (plf_sprite_mask >> maskshift) >> maskshift;
	v &= ~plfmask;
	if (v != 0) {
	    unsigned int vlo, vhi, col;
	    unsigned int v1 = v & 255;
	    int offs;
	    if (v1 == 0)
		offs = 4 + sprite_offs[v >> 8];
	    else
		offs = sprite_offs[v1];

	    v >>= offs << 1;
	    v &= 15;
 
	    if (has_attach && (stbuf[pos] & (1 << offs))) {
		col = v;
		    col += 16;
	    } else {
		vlo = v & 3;
		vhi = (v & (vlo - 1)) >> 2;
		col = (vlo | vhi);
		    col += 16;
		col += (offs << 1);
	    }
	    if (dualpf) {
		    col += 128;
		    if (doubling)
			pixdata.apixels_w[window_pos >> 1] = col | (col << 8);
		    else
			pixdata.apixels[window_pos] = col;
	    } else {
		if (doubling)
		    pixdata.apixels_w[window_pos >> 1] = col | (col << 8);
		else
		    pixdata.apixels[window_pos] = col;
	    }
	}
	window_pos += 1 << doubling;
    }
}

#define draw_sprites_normal_sp_lo_nat(ENTRY) draw_sprites_1	(ENTRY, 0, 0, 0)
#define draw_sprites_normal_dp_lo_nat(ENTRY) draw_sprites_1	(ENTRY, 1, 0, 0)
#define draw_sprites_normal_sp_lo_at(ENTRY)  draw_sprites_1	(ENTRY, 0, 0, 1)
#define draw_sprites_normal_dp_lo_at(ENTRY)  draw_sprites_1	(ENTRY, 1, 0, 1)
#define draw_sprites_normal_sp_hi_nat(ENTRY) draw_sprites_1	(ENTRY, 0, 1, 0)
#define draw_sprites_normal_dp_hi_nat(ENTRY) draw_sprites_1	(ENTRY, 1, 1, 0)
#define draw_sprites_normal_sp_hi_at(ENTRY)  draw_sprites_1	(ENTRY, 0, 1, 1)
#define draw_sprites_normal_dp_hi_at(ENTRY)  draw_sprites_1	(ENTRY, 1, 1, 1)

#define decide_draw_sprites()

static __inline__ void draw_sprites_ecs (struct sprite_entry *_GCCRES_ e)
{
    uae4all_prof_start(12);
    if (e->has_attached)
	if (bplres == 1)
		if (bpldualpf)
		    draw_sprites_normal_dp_hi_at (e);
		else
		    draw_sprites_normal_sp_hi_at (e);
	else
		if (bpldualpf)
		    draw_sprites_normal_dp_lo_at (e);
		else
		    draw_sprites_normal_sp_lo_at (e);
    else
	if (bplres == 1)
		if (bpldualpf)
		    draw_sprites_normal_dp_hi_nat (e);
		else
		    draw_sprites_normal_sp_hi_nat (e);
	else
		if (bpldualpf)
		    draw_sprites_normal_dp_lo_nat (e);
		else
		    draw_sprites_normal_sp_lo_nat (e);
    uae4all_prof_end(12);
}


#else

static void draw_sprites_normal_sp_lo_nat(struct sprite_entry *_GCCRES_ e)
{
    int *shift_lookup = dblpf_ms;
    uae_u16 *buf = spixels + e->first_pixel;
    uae_u8 *stbuf = spixstate.bytes + e->first_pixel;
    int pos, window_pos;
    uae_u8 xor_val = (uae_u8)(dp_for_drawing->bplcon4 >> 8);

    buf -= e->pos;
    stbuf -= e->pos;

    window_pos = e->pos + ((DIW_DDF_OFFSET - DISPLAY_LEFT_SHIFT) );
    window_pos += pixels_offset;
    unsigned max=e->max;
    for (pos = e->pos; pos < max; pos++) {
	int maskshift, plfmask;
	unsigned int v = buf[pos];

	maskshift = shift_lookup[pixdata.apixels[window_pos]];
	plfmask = (plf_sprite_mask >> maskshift) >> maskshift;
	v &= ~plfmask;
	if (v != 0) {
	    unsigned int vlo, vhi, col;
	    unsigned int v1 = v & 255;
	    int offs;
	    if (v1 == 0)
		offs = 4 + sprite_offs[v >> 8];
	    else
		offs = sprite_offs[v1];
	    v >>= offs << 1;
	    v &= 15;
	    vlo = v & 3;
	    vhi = (v & (vlo - 1)) >> 2;
	    col = (vlo | vhi);
	    col += 16;
	    col += (offs << 1);
	    pixdata.apixels[window_pos] = col;
	}
	window_pos ++;
    }
}

static void draw_sprites_normal_dp_lo_nat(struct sprite_entry *_GCCRES_ e)
{
    int *shift_lookup = (bpldualpfpri ? dblpf_ms2 : dblpf_ms1);
    uae_u16 *buf = spixels + e->first_pixel;
    uae_u8 *stbuf = spixstate.bytes + e->first_pixel;
    int pos, window_pos;
    uae_u8 xor_val = (uae_u8)(dp_for_drawing->bplcon4 >> 8);

    buf -= e->pos;
    stbuf -= e->pos;

    window_pos = e->pos + ((DIW_DDF_OFFSET - DISPLAY_LEFT_SHIFT) );
    window_pos += pixels_offset;
    unsigned max=e->max;
    for (pos = e->pos; pos < max; pos++) {
	int maskshift, plfmask;
	unsigned int v = buf[pos];

	maskshift = shift_lookup[pixdata.apixels[window_pos]];
	plfmask = (plf_sprite_mask >> maskshift) >> maskshift;
	v &= ~plfmask;
	if (v != 0) {
	    unsigned int vlo, vhi, col;
	    unsigned int v1 = v & 255;
	    int offs;
	    if (v1 == 0)
		offs = 4 + sprite_offs[v >> 8];
	    else
		offs = sprite_offs[v1];

	    v >>= offs << 1;
	    v &= 15;
 
	    vlo = v & 3;
	    vhi = (v & (vlo - 1)) >> 2;
	    col = (vlo | vhi);
	    col += 16;
	    col += (offs << 1);
	    col += 128;
	    pixdata.apixels[window_pos] = col;
	}
	window_pos++;
    }
}


static void draw_sprites_normal_sp_lo_at(struct sprite_entry *_GCCRES_ e)
{
    int *shift_lookup = dblpf_ms;
    uae_u16 *buf = spixels + e->first_pixel;
    uae_u8 *stbuf = spixstate.bytes + e->first_pixel;
    int pos, window_pos;
    uae_u8 xor_val = (uae_u8)(dp_for_drawing->bplcon4 >> 8);

    buf -= e->pos;
    stbuf -= e->pos;

    window_pos = e->pos + ((DIW_DDF_OFFSET - DISPLAY_LEFT_SHIFT) );
    window_pos += pixels_offset;
    unsigned max=e->max;
    for (pos = e->pos; pos < max; pos++) {
	int maskshift, plfmask;
	unsigned int v = buf[pos];

	maskshift = shift_lookup[pixdata.apixels[window_pos]];
	plfmask = (plf_sprite_mask >> maskshift) >> maskshift;
	v &= ~plfmask;
	if (v != 0) {
	    unsigned int vlo, vhi, col;
	    unsigned int v1 = v & 255;
	    int offs;
	    if (v1 == 0)
		offs = 4 + sprite_offs[v >> 8];
	    else
		offs = sprite_offs[v1];

	    v >>= offs << 1;
	    v &= 15;
 
	    if ((stbuf[pos] & (1 << offs))) {
		col = v;
		col += 16;
	    } else {
		vlo = v & 3;
		vhi = (v & (vlo - 1)) >> 2;
		col = (vlo | vhi);
		col += 16;
		col += (offs << 1);
	    }
	    pixdata.apixels[window_pos] = col;
	}
	window_pos++;
    }
}


static void draw_sprites_normal_dp_lo_at(struct sprite_entry *_GCCRES_ e)
{
    int *shift_lookup = (bpldualpfpri ? dblpf_ms2 : dblpf_ms1);
    uae_u16 *buf = spixels + e->first_pixel;
    uae_u8 *stbuf = spixstate.bytes + e->first_pixel;
    int pos, window_pos;
    uae_u8 xor_val = (uae_u8)(dp_for_drawing->bplcon4 >> 8);

    buf -= e->pos;
    stbuf -= e->pos;

    window_pos = e->pos + ((DIW_DDF_OFFSET - DISPLAY_LEFT_SHIFT) );
    window_pos += pixels_offset;
    unsigned max=e->max;
    for (pos = e->pos; pos < max; pos++) {
	int maskshift, plfmask;
	unsigned int v = buf[pos];

	maskshift = shift_lookup[pixdata.apixels[window_pos]];
	plfmask = (plf_sprite_mask >> maskshift) >> maskshift;
	v &= ~plfmask;
	if (v != 0) {
	    unsigned int vlo, vhi, col;
	    unsigned int v1 = v & 255;
	    int offs;
	    if (v1 == 0)
		offs = 4 + sprite_offs[v >> 8];
	    else
		offs = sprite_offs[v1];

	    v >>= offs << 1;
	    v &= 15;
 
	    if ((stbuf[pos] & (1 << offs))) {
		col = v;
		col += 16;
	    } else {
		vlo = v & 3;
		vhi = (v & (vlo - 1)) >> 2;
		col = (vlo | vhi);
		col += 16;
		col += (offs << 1);
	    }
	    col += 128;
	    pixdata.apixels[window_pos] = col;
	}
	window_pos++;
    }
}

static void draw_sprites_normal_sp_hi_nat(struct sprite_entry *_GCCRES_ e)
{
    int *shift_lookup = dblpf_ms;
    uae_u16 *buf = spixels + e->first_pixel;
    uae_u8 *stbuf = spixstate.bytes + e->first_pixel;
    int pos, window_pos;
    uae_u8 xor_val = (uae_u8)(dp_for_drawing->bplcon4 >> 8);

    buf -= e->pos;
    stbuf -= e->pos;

    window_pos = e->pos + ((DIW_DDF_OFFSET - DISPLAY_LEFT_SHIFT) );
    window_pos <<= 1;
    window_pos += pixels_offset;
    unsigned max=e->max;
    for (pos = e->pos; pos < max; pos ++) {
	int maskshift, plfmask;
	unsigned int v = buf[pos];

	maskshift = shift_lookup[pixdata.apixels[window_pos]];
	plfmask = (plf_sprite_mask >> maskshift) >> maskshift;
	v &= ~plfmask;
	if (v != 0) {
	    unsigned int vlo, vhi, col;
	    unsigned int v1 = v & 255;
	    int offs;
	    if (v1 == 0)
		offs = 4 + sprite_offs[v >> 8];
	    else
		offs = sprite_offs[v1];

	    v >>= offs << 1;
	    v &= 15;
 
	    vlo = v & 3;
	    vhi = (v & (vlo - 1)) >> 2;
	    col = (vlo | vhi);
	    col += 16;
	    col += (offs << 1);
	    pixdata.apixels_w[window_pos >> 1] = col | (col << 8);
	}
	window_pos += 2;
    }
}


static void draw_sprites_normal_dp_hi_nat(struct sprite_entry *_GCCRES_ e)
{
    int *shift_lookup = (bpldualpfpri ? dblpf_ms2 : dblpf_ms1);
    uae_u16 *buf = spixels + e->first_pixel;
    uae_u8 *stbuf = spixstate.bytes + e->first_pixel;
    int pos, window_pos;
    uae_u8 xor_val = (uae_u8)(dp_for_drawing->bplcon4 >> 8);

    buf -= e->pos;
    stbuf -= e->pos;

    window_pos = e->pos + ((DIW_DDF_OFFSET - DISPLAY_LEFT_SHIFT) );
    window_pos <<= 1;
    window_pos += pixels_offset;
    unsigned max=e->max;
    for (pos = e->pos; pos < max; pos ++) {
	int maskshift, plfmask;
	unsigned int v = buf[pos];

	maskshift = shift_lookup[pixdata.apixels[window_pos]];
	plfmask = (plf_sprite_mask >> maskshift) >> maskshift;
	v &= ~plfmask;
	if (v != 0) {
	    unsigned int vlo, vhi, col;
	    unsigned int v1 = v & 255;
	    int offs;
	    if (v1 == 0)
		offs = 4 + sprite_offs[v >> 8];
	    else
		offs = sprite_offs[v1];

	    v >>= offs << 1;
	    v &= 15;
 
	    vlo = v & 3;
	    vhi = (v & (vlo - 1)) >> 2;
	    col = (vlo | vhi);
	    col += 16;
	    col += (offs << 1);
	    col += 128;
	    pixdata.apixels_w[window_pos >> 1] = col | (col << 8);
	}
	window_pos += 2;
    }
}


static void draw_sprites_normal_sp_hi_at(struct sprite_entry *_GCCRES_ e)
{
    int *shift_lookup = dblpf_ms;
    uae_u16 *buf = spixels + e->first_pixel;
    uae_u8 *stbuf = spixstate.bytes + e->first_pixel;
    int pos, window_pos;
    uae_u8 xor_val = (uae_u8)(dp_for_drawing->bplcon4 >> 8);

    buf -= e->pos;
    stbuf -= e->pos;

    window_pos = e->pos + ((DIW_DDF_OFFSET - DISPLAY_LEFT_SHIFT) );
    window_pos <<= 1;
    window_pos += pixels_offset;
    unsigned max=e->max;
    for (pos = e->pos; pos < max; pos++) {
	int maskshift, plfmask;
	unsigned int v = buf[pos];

	maskshift = shift_lookup[pixdata.apixels[window_pos]];
	plfmask = (plf_sprite_mask >> maskshift) >> maskshift;
	v &= ~plfmask;
	if (v != 0) {
	    unsigned int vlo, vhi, col;
	    unsigned int v1 = v & 255;
	    int offs;
	    if (v1 == 0)
		offs = 4 + sprite_offs[v >> 8];
	    else
		offs = sprite_offs[v1];

	    v >>= offs << 1;
	    v &= 15;
 
	    if ((stbuf[pos] & (1 << offs))) {
		col = v;
		col += 16;
	    } else {
		vlo = v & 3;
		vhi = (v & (vlo - 1)) >> 2;
		col = (vlo | vhi);
		col += 16;
		col += (offs << 1);
	    }
	    pixdata.apixels_w[window_pos >> 1] = col | (col << 8);
	}
	window_pos += 2;
    }
}


static void draw_sprites_normal_dp_hi_at(struct sprite_entry *_GCCRES_ e)
{
    int *shift_lookup = (bpldualpfpri ? dblpf_ms2 : dblpf_ms1);
    uae_u16 *buf = spixels + e->first_pixel;
    uae_u8 *stbuf = spixstate.bytes + e->first_pixel;
    int pos, window_pos;
    uae_u8 xor_val = (uae_u8)(dp_for_drawing->bplcon4 >> 8);

    buf -= e->pos;
    stbuf -= e->pos;

    window_pos = e->pos + ((DIW_DDF_OFFSET - DISPLAY_LEFT_SHIFT) );
    window_pos <<= 1;
    window_pos += pixels_offset;
    
    unsigned max=e->max;
    for (pos = e->pos; pos < max; pos++) {
	int maskshift, plfmask;
	unsigned int v = buf[pos];

	maskshift = shift_lookup[pixdata.apixels[window_pos]];
	plfmask = (plf_sprite_mask >> maskshift) >> maskshift;
	v &= ~plfmask;
	if (v != 0) {
	    unsigned int vlo, vhi, col;
	    unsigned int v1 = v & 255;
	    int offs;
	    if (v1 == 0)
		offs = 4 + sprite_offs[v >> 8];
	    else
		offs = sprite_offs[v1];

	    v >>= offs << 1;
	    v &= 15;
 
	    if ((stbuf[pos] & (1 << offs))) {
		col = v;
		    col += 16;
	    } else {
		vlo = v & 3;
		vhi = (v & (vlo - 1)) >> 2;
		col = (vlo | vhi);
		col += 16;
		col += (offs << 1);
	    }
	    col += 128;
	    pixdata.apixels_w[window_pos >> 1] = col | (col << 8);
	}
	window_pos += 2;
    }
}

typedef void (*draw_sprites_func)(struct sprite_entry *_GCCRES_ e);
static draw_sprites_func draw_sprites_dp_hi[2]={
	draw_sprites_normal_dp_hi_nat, draw_sprites_normal_dp_hi_at };
static draw_sprites_func draw_sprites_sp_hi[2]={
	draw_sprites_normal_sp_hi_nat, draw_sprites_normal_sp_hi_at };
static draw_sprites_func draw_sprites_dp_lo[2]={
	draw_sprites_normal_dp_lo_nat, draw_sprites_normal_dp_lo_at };
static draw_sprites_func draw_sprites_sp_lo[2]={
	draw_sprites_normal_sp_lo_nat, draw_sprites_normal_sp_lo_at };

static draw_sprites_func *draw_sprites_punt=draw_sprites_sp_lo;


static __inline__ void decide_draw_sprites(void) {
	if (bplres == 1)
		if (bpldualpf)
			draw_sprites_punt=draw_sprites_dp_hi;
		else
			draw_sprites_punt=draw_sprites_sp_hi;
	else
		if (bpldualpf)
			draw_sprites_punt=draw_sprites_dp_lo;
		else
			draw_sprites_punt=draw_sprites_sp_lo;
}

static __inline__ void draw_sprites_ecs (struct sprite_entry *_GCCRES_ e)
{
	uae4all_prof_start(12);
	draw_sprites_punt[e->has_attached](e);
	uae4all_prof_end(12);
}

#endif



#define MERGE(a,b,mask,shift) {\
    register uae_u32 tmp = mask & (a ^ (b >> shift)); \
    a ^= tmp; \
    b ^= (tmp << shift); \
}

#define MERGE_0(a,b,mask,shift) {\
   register uae_u32 tmp = mask & (b>>shift); \
   a = tmp; \
   b ^= (tmp << shift); \
}

#define GETLONG(P) (*(uae_u32 *)P)
#define DATA_POINTER(n) (line_data[lineno] + (n)*MAX_WORDS_PER_LINE*2)

#define DO_SWLONG(A,V) {\
	register uae_u8 *b = (uae_u8 *)(A); \
	register uae_u32 v = (V); \
	*b++ = v >> 24; \
	*b++ = v >> 16; \
	*b++ = v >> 8; \
	*b = v; \
}

#ifndef UNROLL_PFIELD

static __inline__ void pfield_doline_1 (uae_u32 *_GCCRES_ pixels, int wordcount, int planes)
{
    while (wordcount-- > 0) {
	uae_u32 b0, b1, b2, b3, b4, b5, b6, b7;

	b0 = 0, b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0, b6 = 0, b7 = 0;
	switch (planes) {
	case 8: b0 = GETLONG ((uae_u32 *)real_bplpt[7]); real_bplpt[7] += 4;
	case 7: b1 = GETLONG ((uae_u32 *)real_bplpt[6]); real_bplpt[6] += 4;
	case 6: b2 = GETLONG ((uae_u32 *)real_bplpt[5]); real_bplpt[5] += 4;
	case 5: b3 = GETLONG ((uae_u32 *)real_bplpt[4]); real_bplpt[4] += 4;
	case 4: b4 = GETLONG ((uae_u32 *)real_bplpt[3]); real_bplpt[3] += 4;
	case 3: b5 = GETLONG ((uae_u32 *)real_bplpt[2]); real_bplpt[2] += 4;
	case 2: b6 = GETLONG ((uae_u32 *)real_bplpt[1]); real_bplpt[1] += 4;
	case 1: b7 = GETLONG ((uae_u32 *)real_bplpt[0]); real_bplpt[0] += 4;
	}

	MERGE (b0, b1, 0x55555555, 1);
	MERGE (b2, b3, 0x55555555, 1);
	MERGE (b4, b5, 0x55555555, 1);
	MERGE (b6, b7, 0x55555555, 1);

	MERGE (b0, b2, 0x33333333, 2);
	MERGE (b1, b3, 0x33333333, 2);
	MERGE (b4, b6, 0x33333333, 2);
	MERGE (b5, b7, 0x33333333, 2);

	MERGE (b0, b4, 0x0f0f0f0f, 4);
	MERGE (b1, b5, 0x0f0f0f0f, 4);
	MERGE (b2, b6, 0x0f0f0f0f, 4);
	MERGE (b3, b7, 0x0f0f0f0f, 4);

	MERGE (b0, b1, 0x00ff00ff, 8);
	MERGE (b2, b3, 0x00ff00ff, 8);
	MERGE (b4, b5, 0x00ff00ff, 8);
	MERGE (b6, b7, 0x00ff00ff, 8);

	MERGE (b0, b2, 0x0000ffff, 16);
	DO_SWLONG(pixels, b0);
	DO_SWLONG(pixels + 4, b2);
	MERGE (b1, b3, 0x0000ffff, 16);
	DO_SWLONG(pixels + 2, b1);
	DO_SWLONG(pixels + 6, b3);
	MERGE (b4, b6, 0x0000ffff, 16);
	DO_SWLONG(pixels + 1, b4);
	DO_SWLONG(pixels + 5, b6);
	MERGE (b5, b7, 0x0000ffff, 16);
	DO_SWLONG(pixels + 3, b5);
	DO_SWLONG(pixels + 7, b7);
	pixels += 8;
    }
}


#define pfield_doline_n1(DTA,CNT) pfield_doline_1 (DTA, CNT, 1)
#define pfield_doline_n2(DTA,CNT) pfield_doline_1 (DTA, CNT, 2)
#define pfield_doline_n3(DTA,CNT) pfield_doline_1 (DTA, CNT, 3)
#define pfield_doline_n4(DTA,CNT) pfield_doline_1 (DTA, CNT, 4)
#define pfield_doline_n5(DTA,CNT) pfield_doline_1 (DTA, CNT, 5)
#define pfield_doline_n6(DTA,CNT) pfield_doline_1 (DTA, CNT, 6)
#define pfield_doline_n7(DTA,CNT) pfield_doline_1 (DTA, CNT, 7)
#define pfield_doline_n8(DTA,CNT) pfield_doline_1 (DTA, CNT, 8)

static _INLINE_ void pfield_doline (int lineno)
{
    uae4all_prof_start(11);
    int wordcount = dp_for_drawing->plflinelen;
    uae_u32 *data = pixdata.apixels_l + MAX_PIXELS_PER_LINE/4;

    real_bplpt[0] = DATA_POINTER (0);
    real_bplpt[1] = DATA_POINTER (1);
    real_bplpt[2] = DATA_POINTER (2);
    real_bplpt[3] = DATA_POINTER (3);
    real_bplpt[4] = DATA_POINTER (4);
    real_bplpt[5] = DATA_POINTER (5);
    real_bplpt[6] = DATA_POINTER (6);
    real_bplpt[7] = DATA_POINTER (7);

    switch (bplplanecnt) {
    default: break;
    case 0: uae4all_memclr(data, wordcount << 5); break;
    case 1: pfield_doline_n1 (data, wordcount); break;
    case 2: pfield_doline_n2 (data, wordcount); break;
    case 3: pfield_doline_n3 (data, wordcount); break;
    case 4: pfield_doline_n4 (data, wordcount); break;
    case 5: pfield_doline_n5 (data, wordcount); break;
    case 6: pfield_doline_n6 (data, wordcount); break;
    case 7: pfield_doline_n7 (data, wordcount); break;
    case 8: pfield_doline_n8 (data, wordcount); break;
    }
    uae4all_prof_end(11);
}

#else


static void pfield_doline_n1 (uae_u32 *_GCCRES_ pixels, int wordcount, int lineno)
{
    real_bplpt[0] = DATA_POINTER (0);
    while (wordcount-- > 0) {
	uae_u32 b0,b1,b2,b3,b4,b5,b6,b7;
	b7 = GETLONG ((uae_u32 *)real_bplpt[0]); real_bplpt[0] += 4;

	MERGE_0(b6, b7, 0x55555555, 1);

	MERGE_0(b4, b6, 0x33333333, 2);
	MERGE_0(b5, b7, 0x33333333, 2);

	MERGE_0(b0, b4, 0x0f0f0f0f, 4);
	MERGE_0(b1, b5, 0x0f0f0f0f, 4);
	MERGE_0(b2, b6, 0x0f0f0f0f, 4);
	MERGE_0(b3, b7, 0x0f0f0f0f, 4);

	MERGE (b0, b1, 0x00ff00ff, 8);
	MERGE (b2, b3, 0x00ff00ff, 8);
	MERGE (b4, b5, 0x00ff00ff, 8);
	MERGE (b6, b7, 0x00ff00ff, 8);

	MERGE (b0, b2, 0x0000ffff, 16);
	DO_SWLONG(pixels, b0);
	DO_SWLONG(pixels + 4, b2);
	MERGE (b1, b3, 0x0000ffff, 16);
	DO_SWLONG(pixels + 2, b1);
	DO_SWLONG(pixels + 6, b3);
	MERGE (b4, b6, 0x0000ffff, 16);
	DO_SWLONG(pixels + 1, b4);
	DO_SWLONG(pixels + 5, b6);
	MERGE (b5, b7, 0x0000ffff, 16);
	DO_SWLONG(pixels + 3, b5);
	DO_SWLONG(pixels + 7, b7);
	pixels += 8;
    }
}

static void pfield_doline_n2 (uae_u32 *_GCCRES_ pixels, int wordcount, int lineno)
{
    real_bplpt[0] = DATA_POINTER (0);
    real_bplpt[1] = DATA_POINTER (1);
    while (wordcount-- > 0) {
	uae_u32 b0,b1,b2,b3,b4,b5,b6,b7;
	b6 = GETLONG ((uae_u32 *)real_bplpt[1]); real_bplpt[1] += 4;
	b7 = GETLONG ((uae_u32 *)real_bplpt[0]); real_bplpt[0] += 4;

	MERGE (b6, b7, 0x55555555, 1);

	MERGE_0(b4, b6, 0x33333333, 2);
	MERGE_0(b5, b7, 0x33333333, 2);

	MERGE_0(b0, b4, 0x0f0f0f0f, 4);
	MERGE_0(b1, b5, 0x0f0f0f0f, 4);
	MERGE_0(b2, b6, 0x0f0f0f0f, 4);
	MERGE_0(b3, b7, 0x0f0f0f0f, 4);

	MERGE (b0, b1, 0x00ff00ff, 8);
	MERGE (b2, b3, 0x00ff00ff, 8);
	MERGE (b4, b5, 0x00ff00ff, 8);
	MERGE (b6, b7, 0x00ff00ff, 8);

	MERGE (b0, b2, 0x0000ffff, 16);
	DO_SWLONG(pixels, b0);
	DO_SWLONG(pixels + 4, b2);
	MERGE (b1, b3, 0x0000ffff, 16);
	DO_SWLONG(pixels + 2, b1);
	DO_SWLONG(pixels + 6, b3);
	MERGE (b4, b6, 0x0000ffff, 16);
	DO_SWLONG(pixels + 1, b4);
	DO_SWLONG(pixels + 5, b6);
	MERGE (b5, b7, 0x0000ffff, 16);
	DO_SWLONG(pixels + 3, b5);
	DO_SWLONG(pixels + 7, b7);
	pixels += 8;
    }
}

static void pfield_doline_n3 (uae_u32 *_GCCRES_ pixels, int wordcount, int lineno)
{
    real_bplpt[0] = DATA_POINTER (0);
    real_bplpt[1] = DATA_POINTER (1);
    real_bplpt[2] = DATA_POINTER (2);
    while (wordcount-- > 0) {
	uae_u32 b0,b1,b2,b3,b4,b5,b6,b7;
	b5 = GETLONG ((uae_u32 *)real_bplpt[2]); real_bplpt[2] += 4;
	b6 = GETLONG ((uae_u32 *)real_bplpt[1]); real_bplpt[1] += 4;
	b7 = GETLONG ((uae_u32 *)real_bplpt[0]); real_bplpt[0] += 4;

	MERGE_0(b4, b5, 0x55555555, 1);
	MERGE (b6, b7, 0x55555555, 1);

	MERGE (b4, b6, 0x33333333, 2);
	MERGE (b5, b7, 0x33333333, 2);

	MERGE_0(b0, b4, 0x0f0f0f0f, 4);
	MERGE_0(b1, b5, 0x0f0f0f0f, 4);
	MERGE_0(b2, b6, 0x0f0f0f0f, 4);
	MERGE_0(b3, b7, 0x0f0f0f0f, 4);

	MERGE (b0, b1, 0x00ff00ff, 8);
	MERGE (b2, b3, 0x00ff00ff, 8);
	MERGE (b4, b5, 0x00ff00ff, 8);
	MERGE (b6, b7, 0x00ff00ff, 8);

	MERGE (b0, b2, 0x0000ffff, 16);
	DO_SWLONG(pixels, b0);
	DO_SWLONG(pixels + 4, b2);
	MERGE (b1, b3, 0x0000ffff, 16);
	DO_SWLONG(pixels + 2, b1);
	DO_SWLONG(pixels + 6, b3);
	MERGE (b4, b6, 0x0000ffff, 16);
	DO_SWLONG(pixels + 1, b4);
	DO_SWLONG(pixels + 5, b6);
	MERGE (b5, b7, 0x0000ffff, 16);
	DO_SWLONG(pixels + 3, b5);
	DO_SWLONG(pixels + 7, b7);
	pixels += 8;
    }
}

static void pfield_doline_n4 (uae_u32 *_GCCRES_ pixels, int wordcount, int lineno)
{
    real_bplpt[0] = DATA_POINTER (0);
    real_bplpt[1] = DATA_POINTER (1);
    real_bplpt[2] = DATA_POINTER (2);
    real_bplpt[3] = DATA_POINTER (3);
    while (wordcount-- > 0) {
	uae_u32 b0,b1,b2,b3,b4,b5,b6,b7;
	b4 = GETLONG ((uae_u32 *)real_bplpt[3]); real_bplpt[3] += 4;
	b5 = GETLONG ((uae_u32 *)real_bplpt[2]); real_bplpt[2] += 4;
	b6 = GETLONG ((uae_u32 *)real_bplpt[1]); real_bplpt[1] += 4;
	b7 = GETLONG ((uae_u32 *)real_bplpt[0]); real_bplpt[0] += 4;

	MERGE (b4, b5, 0x55555555, 1);
	MERGE (b6, b7, 0x55555555, 1);

	MERGE (b4, b6, 0x33333333, 2);
	MERGE (b5, b7, 0x33333333, 2);

	MERGE_0(b0, b4, 0x0f0f0f0f, 4);
	MERGE_0(b1, b5, 0x0f0f0f0f, 4);
	MERGE_0(b2, b6, 0x0f0f0f0f, 4);
	MERGE_0(b3, b7, 0x0f0f0f0f, 4);

	MERGE (b0, b1, 0x00ff00ff, 8);
	MERGE (b2, b3, 0x00ff00ff, 8);
	MERGE (b4, b5, 0x00ff00ff, 8);
	MERGE (b6, b7, 0x00ff00ff, 8);

	MERGE (b0, b2, 0x0000ffff, 16);
	DO_SWLONG(pixels, b0);
	DO_SWLONG(pixels + 4, b2);
	MERGE (b1, b3, 0x0000ffff, 16);
	DO_SWLONG(pixels + 2, b1);
	DO_SWLONG(pixels + 6, b3);
	MERGE (b4, b6, 0x0000ffff, 16);
	DO_SWLONG(pixels + 1, b4);
	DO_SWLONG(pixels + 5, b6);
	MERGE (b5, b7, 0x0000ffff, 16);
	DO_SWLONG(pixels + 3, b5);
	DO_SWLONG(pixels + 7, b7);
	pixels += 8;
    }
}

static void pfield_doline_n5 (uae_u32 *_GCCRES_ pixels, int wordcount, int lineno)
{
    real_bplpt[0] = DATA_POINTER (0);
    real_bplpt[1] = DATA_POINTER (1);
    real_bplpt[2] = DATA_POINTER (2);
    real_bplpt[3] = DATA_POINTER (3);
    real_bplpt[4] = DATA_POINTER (4);
    while (wordcount-- > 0) {
	uae_u32 b0,b1,b2,b3,b4,b5,b6,b7;
	b3 = GETLONG ((uae_u32 *)real_bplpt[4]); real_bplpt[4] += 4;
	b4 = GETLONG ((uae_u32 *)real_bplpt[3]); real_bplpt[3] += 4;
	b5 = GETLONG ((uae_u32 *)real_bplpt[2]); real_bplpt[2] += 4;
	b6 = GETLONG ((uae_u32 *)real_bplpt[1]); real_bplpt[1] += 4;
	b7 = GETLONG ((uae_u32 *)real_bplpt[0]); real_bplpt[0] += 4;

	MERGE_0(b2, b3, 0x55555555, 1);
	MERGE (b4, b5, 0x55555555, 1);
	MERGE (b6, b7, 0x55555555, 1);

	MERGE_0(b0, b2, 0x33333333, 2);
	MERGE_0(b1, b3, 0x33333333, 2);
	MERGE (b4, b6, 0x33333333, 2);
	MERGE (b5, b7, 0x33333333, 2);

	MERGE (b0, b4, 0x0f0f0f0f, 4);
	MERGE (b1, b5, 0x0f0f0f0f, 4);
	MERGE (b2, b6, 0x0f0f0f0f, 4);
	MERGE (b3, b7, 0x0f0f0f0f, 4);

	MERGE (b0, b1, 0x00ff00ff, 8);
	MERGE (b2, b3, 0x00ff00ff, 8);
	MERGE (b4, b5, 0x00ff00ff, 8);
	MERGE (b6, b7, 0x00ff00ff, 8);

	MERGE (b0, b2, 0x0000ffff, 16);
	DO_SWLONG(pixels, b0);
	DO_SWLONG(pixels + 4, b2);
	MERGE (b1, b3, 0x0000ffff, 16);
	DO_SWLONG(pixels + 2, b1);
	DO_SWLONG(pixels + 6, b3);
	MERGE (b4, b6, 0x0000ffff, 16);
	DO_SWLONG(pixels + 1, b4);
	DO_SWLONG(pixels + 5, b6);
	MERGE (b5, b7, 0x0000ffff, 16);
	DO_SWLONG(pixels + 3, b5);
	DO_SWLONG(pixels + 7, b7);
	pixels += 8;
    }
}

static void pfield_doline_n6 (uae_u32 *_GCCRES_ pixels, int wordcount, int lineno)
{
    real_bplpt[0] = DATA_POINTER (0);
    real_bplpt[1] = DATA_POINTER (1);
    real_bplpt[2] = DATA_POINTER (2);
    real_bplpt[3] = DATA_POINTER (3);
    real_bplpt[4] = DATA_POINTER (4);
    real_bplpt[5] = DATA_POINTER (5);
    while (wordcount-- > 0) {
	uae_u32 b0,b1,b2,b3,b4,b5,b6,b7;
	b2 = GETLONG ((uae_u32 *)real_bplpt[5]); real_bplpt[5] += 4;
	b3 = GETLONG ((uae_u32 *)real_bplpt[4]); real_bplpt[4] += 4;
	b4 = GETLONG ((uae_u32 *)real_bplpt[3]); real_bplpt[3] += 4;
	b5 = GETLONG ((uae_u32 *)real_bplpt[2]); real_bplpt[2] += 4;
	b6 = GETLONG ((uae_u32 *)real_bplpt[1]); real_bplpt[1] += 4;
	b7 = GETLONG ((uae_u32 *)real_bplpt[0]); real_bplpt[0] += 4;

	MERGE (b2, b3, 0x55555555, 1);
	MERGE (b4, b5, 0x55555555, 1);
	MERGE (b6, b7, 0x55555555, 1);

	MERGE_0(b0, b2, 0x33333333, 2);
	MERGE_0(b1, b3, 0x33333333, 2);
	MERGE (b4, b6, 0x33333333, 2);
	MERGE (b5, b7, 0x33333333, 2);

	MERGE (b0, b4, 0x0f0f0f0f, 4);
	MERGE (b1, b5, 0x0f0f0f0f, 4);
	MERGE (b2, b6, 0x0f0f0f0f, 4);
	MERGE (b3, b7, 0x0f0f0f0f, 4);

	MERGE (b0, b1, 0x00ff00ff, 8);
	MERGE (b2, b3, 0x00ff00ff, 8);
	MERGE (b4, b5, 0x00ff00ff, 8);
	MERGE (b6, b7, 0x00ff00ff, 8);

	MERGE (b0, b2, 0x0000ffff, 16);
	DO_SWLONG(pixels, b0);
	DO_SWLONG(pixels + 4, b2);
	MERGE (b1, b3, 0x0000ffff, 16);
	DO_SWLONG(pixels + 2, b1);
	DO_SWLONG(pixels + 6, b3);
	MERGE (b4, b6, 0x0000ffff, 16);
	DO_SWLONG(pixels + 1, b4);
	DO_SWLONG(pixels + 5, b6);
	MERGE (b5, b7, 0x0000ffff, 16);
	DO_SWLONG(pixels + 3, b5);
	DO_SWLONG(pixels + 7, b7);
	pixels += 8;
    }
}

static void pfield_doline_n7 (uae_u32 *_GCCRES_ pixels, int wordcount, int lineno)
{
    real_bplpt[0] = DATA_POINTER (0);
    real_bplpt[1] = DATA_POINTER (1);
    real_bplpt[2] = DATA_POINTER (2);
    real_bplpt[3] = DATA_POINTER (3);
    real_bplpt[4] = DATA_POINTER (4);
    real_bplpt[5] = DATA_POINTER (5);
    real_bplpt[6] = DATA_POINTER (6);
    while (wordcount-- > 0) {
	uae_u32 b0,b1,b2,b3,b4,b5,b6,b7;
	b1 = GETLONG ((uae_u32 *)real_bplpt[6]); real_bplpt[6] += 4;
	b2 = GETLONG ((uae_u32 *)real_bplpt[5]); real_bplpt[5] += 4;
	b3 = GETLONG ((uae_u32 *)real_bplpt[4]); real_bplpt[4] += 4;
	b4 = GETLONG ((uae_u32 *)real_bplpt[3]); real_bplpt[3] += 4;
	b5 = GETLONG ((uae_u32 *)real_bplpt[2]); real_bplpt[2] += 4;
	b6 = GETLONG ((uae_u32 *)real_bplpt[1]); real_bplpt[1] += 4;
	b7 = GETLONG ((uae_u32 *)real_bplpt[0]); real_bplpt[0] += 4;

	MERGE_0(b0, b1, 0x55555555, 1);
	MERGE (b2, b3, 0x55555555, 1);
	MERGE (b4, b5, 0x55555555, 1);
	MERGE (b6, b7, 0x55555555, 1);

	MERGE (b0, b2, 0x33333333, 2);
	MERGE (b1, b3, 0x33333333, 2);
	MERGE (b4, b6, 0x33333333, 2);
	MERGE (b5, b7, 0x33333333, 2);

	MERGE (b0, b4, 0x0f0f0f0f, 4);
	MERGE (b1, b5, 0x0f0f0f0f, 4);
	MERGE (b2, b6, 0x0f0f0f0f, 4);
	MERGE (b3, b7, 0x0f0f0f0f, 4);

	MERGE (b0, b1, 0x00ff00ff, 8);
	MERGE (b2, b3, 0x00ff00ff, 8);
	MERGE (b4, b5, 0x00ff00ff, 8);
	MERGE (b6, b7, 0x00ff00ff, 8);

	MERGE (b0, b2, 0x0000ffff, 16);
	DO_SWLONG(pixels, b0);
	DO_SWLONG(pixels + 4, b2);
	MERGE (b1, b3, 0x0000ffff, 16);
	DO_SWLONG(pixels + 2, b1);
	DO_SWLONG(pixels + 6, b3);
	MERGE (b4, b6, 0x0000ffff, 16);
	DO_SWLONG(pixels + 1, b4);
	DO_SWLONG(pixels + 5, b6);
	MERGE (b5, b7, 0x0000ffff, 16);
	DO_SWLONG(pixels + 3, b5);
	DO_SWLONG(pixels + 7, b7);
	pixels += 8;
    }
}


static void pfield_doline_n8 (uae_u32 *_GCCRES_ pixels, int wordcount, int lineno)
{
    real_bplpt[0] = DATA_POINTER (0);
    real_bplpt[1] = DATA_POINTER (1);
    real_bplpt[2] = DATA_POINTER (2);
    real_bplpt[3] = DATA_POINTER (3);
    real_bplpt[4] = DATA_POINTER (4);
    real_bplpt[5] = DATA_POINTER (5);
    real_bplpt[6] = DATA_POINTER (6);
    real_bplpt[7] = DATA_POINTER (7);
    while (wordcount-- > 0) {
	uae_u32 b0,b1,b2,b3,b4,b5,b6,b7;
	b0 = GETLONG ((uae_u32 *)real_bplpt[7]); real_bplpt[7] += 4;
	b1 = GETLONG ((uae_u32 *)real_bplpt[6]); real_bplpt[6] += 4;
	b2 = GETLONG ((uae_u32 *)real_bplpt[5]); real_bplpt[5] += 4;
	b3 = GETLONG ((uae_u32 *)real_bplpt[4]); real_bplpt[4] += 4;
	b4 = GETLONG ((uae_u32 *)real_bplpt[3]); real_bplpt[3] += 4;
	b5 = GETLONG ((uae_u32 *)real_bplpt[2]); real_bplpt[2] += 4;
	b6 = GETLONG ((uae_u32 *)real_bplpt[1]); real_bplpt[1] += 4;
	b7 = GETLONG ((uae_u32 *)real_bplpt[0]); real_bplpt[0] += 4;

	MERGE (b0, b1, 0x55555555, 1);
	MERGE (b2, b3, 0x55555555, 1);
	MERGE (b4, b5, 0x55555555, 1);
	MERGE (b6, b7, 0x55555555, 1);

	MERGE (b0, b2, 0x33333333, 2);
	MERGE (b1, b3, 0x33333333, 2);
	MERGE (b4, b6, 0x33333333, 2);
	MERGE (b5, b7, 0x33333333, 2);

	MERGE (b0, b4, 0x0f0f0f0f, 4);
	MERGE (b1, b5, 0x0f0f0f0f, 4);
	MERGE (b2, b6, 0x0f0f0f0f, 4);
	MERGE (b3, b7, 0x0f0f0f0f, 4);

	MERGE (b0, b1, 0x00ff00ff, 8);
	MERGE (b2, b3, 0x00ff00ff, 8);
	MERGE (b4, b5, 0x00ff00ff, 8);
	MERGE (b6, b7, 0x00ff00ff, 8);

	MERGE (b0, b2, 0x0000ffff, 16);
	DO_SWLONG(pixels, b0);
	DO_SWLONG(pixels + 4, b2);
	MERGE (b1, b3, 0x0000ffff, 16);
	DO_SWLONG(pixels + 2, b1);
	DO_SWLONG(pixels + 6, b3);
	MERGE (b4, b6, 0x0000ffff, 16);
	DO_SWLONG(pixels + 1, b4);
	DO_SWLONG(pixels + 5, b6);
	MERGE (b5, b7, 0x0000ffff, 16);
	DO_SWLONG(pixels + 3, b5);
	DO_SWLONG(pixels + 7, b7);
	pixels += 8;
    }
}

static void pfield_doline_n0 (uae_u32 *_GCCRES_ pixels, int wordcount, int lineno)
{
	uae4all_memclr(pixels, wordcount << 5);
}

static void pfield_doline_dummy (uae_u32 *_GCCRES_ pixels, int wordcount, int lineno)
{
}

typedef void (*pfield_doline_func)(uae_u32 *_GCCRES_, int, int);
static pfield_doline_func pfield_doline_n[16]={
	pfield_doline_n0, pfield_doline_n1,pfield_doline_n2,pfield_doline_n3,
	pfield_doline_n4, pfield_doline_n5,pfield_doline_n6,pfield_doline_n7,
	pfield_doline_n8, pfield_doline_dummy,pfield_doline_dummy,pfield_doline_dummy,
	pfield_doline_dummy,pfield_doline_dummy,pfield_doline_dummy,pfield_doline_dummy
};


static __inline__ void pfield_doline (int lineno)
{
    uae4all_prof_start(11);
    pfield_doline_n[bplplanecnt](pixdata.apixels_l + MAX_PIXELS_PER_LINE/4,dp_for_drawing->plflinelen,lineno);
    uae4all_prof_end(11);
}

#endif



static _INLINE_ void init_row_map (void)
{
    int i;
    for (i = 0; i < GFXVIDINFO_HEIGHT + 1; i++)
	row_map[i] = gfx_mem + gfx_rowbytes * i;
}

static _INLINE_ void init_aspect_maps (void)
{
    int i, maxl;
    double native_lines_per_amiga_line;

    if (native2amiga_line_map)
	free (native2amiga_line_map);
    if (amiga2aspect_line_map)
	free (amiga2aspect_line_map);

    /* At least for this array the +1 is necessary. */
    amiga2aspect_line_map = (int *)xmalloc (sizeof (int) * (MAXVPOS + 1)*2 + 1);
    native2amiga_line_map = (int *)xmalloc (sizeof (int) * GFXVIDINFO_HEIGHT);

	native_lines_per_amiga_line = 1;

    maxl = (MAXVPOS + 1);
    min_ypos_for_screen = minfirstline;
    max_drawn_amiga_line = -1;
    for (i = 0; i < maxl; i++) {
	int v = (int) ((i - min_ypos_for_screen) * native_lines_per_amiga_line);
	if (v >= GFXVIDINFO_HEIGHT && max_drawn_amiga_line == -1)
	    max_drawn_amiga_line = i - min_ypos_for_screen;
	if (i < min_ypos_for_screen || v >= GFXVIDINFO_HEIGHT)
	    v = -1;
	amiga2aspect_line_map[i] = v;
    }

    for (i = 0; i < GFXVIDINFO_HEIGHT; i++)
	native2amiga_line_map[i] = -1;

    if (native_lines_per_amiga_line < 1) {
	/* Must omit drawing some lines. */
	for (i = maxl - 1; i > min_ypos_for_screen; i--) {
	    if (amiga2aspect_line_map[i] == amiga2aspect_line_map[i-1]) {
		    amiga2aspect_line_map[i] = -1;
	    }
	}
    }

    for (i = maxl-1; i >= min_ypos_for_screen; i--) {
	int j;
	if (amiga2aspect_line_map[i] == -1)
	    continue;
	for (j = amiga2aspect_line_map[i]; j < GFXVIDINFO_HEIGHT && native2amiga_line_map[j] == -1; j++)
	    native2amiga_line_map[j] = i;
    }
}

/*
 * A raster line has been built in the graphics buffer. Tell the graphics code
 * to do anything necessary to display it.
 */
#ifdef USE_RASTER_DRAW
static _INLINE_ void do_flush_line (int lineno)
{
    if (lineno < first_drawn_line)
	first_drawn_line = lineno;
    if (lineno > last_drawn_line)
	last_drawn_line = lineno;

	if ((last_block_line+1) != lineno) {
	    if (first_block_line != -2)
		flush_block (first_block_line, last_block_line);
	    first_block_line = lineno;
	}
	last_block_line = lineno;
	if (last_block_line - first_block_line >= MAXBLOCKLINES) {
	    flush_block (first_block_line, last_block_line);
	    first_block_line = last_block_line = -2;
	}
}

/*
 * One drawing frame has been finished. Tell the graphics code about it.
 */

static __inline__ void do_flush_screen (int start, int stop)
{
    if (first_block_line != -2) 
	flush_block (first_block_line, last_block_line);
}
#else
#define do_flush_line(LNO)
#endif

static int drawing_color_matches;
static enum { color_match_acolors, color_match_full } color_match_type;

/* Set up colors_for_drawing to the state at the beginning of the currently drawn
   line.  Try to avoid copying color tables around whenever possible.  */
static __inline__ void adjust_drawing_colors (int ctable)
{
    if (drawing_color_matches != ctable) {
	    uae4all_memcpy(colors_for_drawing.acolors, curr_color_tables[ctable].acolors,
	        sizeof colors_for_drawing.acolors);
	    color_match_type = color_match_acolors;
	drawing_color_matches = ctable;
    }
}

static _INLINE_ void do_color_changes (line_draw_func worker_border, line_draw_func worker_pfield)
{
    int i, lastpos = VISIBLE_LEFT_BORDER;
    const int maxifor=dip_for_drawing->last_color_change;
    struct color_change *cc=&curr_color_changes[dip_for_drawing->first_color_change];
    for(i=dip_for_drawing->first_color_change;i<=maxifor;i++,cc++) {
	const int regno = cc->regno;
	const unsigned int value = cc->value;
	int nextpos, nextpos_in_range;
	if (i == maxifor)
	    nextpos = max_diwlastword;
	else
	    nextpos = coord_hw_to_window_x (cc->linepos << 1);

	nextpos_in_range = nextpos;
	if (nextpos > VISIBLE_RIGHT_BORDER)
	    nextpos_in_range = VISIBLE_RIGHT_BORDER;

	if (nextpos_in_range > lastpos) {
	    if (lastpos < playfield_start) {
		int t = nextpos_in_range <= playfield_start ? nextpos_in_range : playfield_start;
		(*worker_border) (lastpos, t);
		lastpos = t;
	    }
	}
	if (nextpos_in_range > lastpos) {
	    if (lastpos >= playfield_start && lastpos < playfield_end) {
		int t = nextpos_in_range <= playfield_end ? nextpos_in_range : playfield_end;
		(*worker_pfield) (lastpos, t);
		lastpos = t;
	    }
	}
	if (nextpos_in_range > lastpos) {
	    if (lastpos >= playfield_end)
		(*worker_border) (lastpos, nextpos_in_range);
	    lastpos = nextpos_in_range;
	}
	if (i != dip_for_drawing->last_color_change) {
	    if (regno != -1)
	    {
		color_reg_set (&colors_for_drawing, regno, value);
		colors_for_drawing.acolors[regno] = getxcolor (value);
	    }
	}
	if (lastpos >= VISIBLE_RIGHT_BORDER)
	    break;
    }
}

/* We only save hardware registers during the hardware frame. Now, when
 * drawing the frame, we expand the data into a slightly more useful
 * form. */
static __inline__ void pfield_expand_dp_bplcon (void)
{
    int plf1pri, plf2pri;
    bplres = dp_for_drawing->bplres;
    bplplanecnt = dp_for_drawing->nr_planes;

    plf1pri = dp_for_drawing->bplcon2 & 7;
    plf2pri = (dp_for_drawing->bplcon2 >> 3) & 7;
    plf_sprite_mask = 0xFFFF0000 << (4 * plf2pri);
    plf_sprite_mask |= (0xFFFF << (4 * plf1pri)) & 0xFFFF;
    bpldualpf = (dp_for_drawing->bplcon0 & 0x400) == 0x400;
    bpldualpfpri = (dp_for_drawing->bplcon2 & 0x40) == 0x40;
    bpldualpf2of = (dp_for_drawing->bplcon3 >> 10) & 7;
    sbasecol[0] = ((dp_for_drawing->bplcon4 >> 4) & 15) << 4;
    sbasecol[1] = ((dp_for_drawing->bplcon4 >> 0) & 15) << 4;
}

static __inline__ void pfield_draw_line (int lineno, int gfx_ypos, int follow_ypos)
{
    int border = 0;

    dp_for_drawing = line_decisions + lineno;
    dip_for_drawing = curr_drawinfo + lineno;
    if (dp_for_drawing->plfleft == -1)
	    border = 1;

#ifdef USE_LINESTATE
    linestate[lineno] = LINE_DONE;
#endif
    xlinebuffer = row_map[gfx_ypos];
    xlinebuffer -= LINETOSCR_X_ADJUST_BYTES;

    if (!border) {
	pfield_expand_dp_bplcon ();

	pfield_init_linetoscr ();
	pfield_doline (lineno);

	adjust_drawing_colors (dp_for_drawing->ctable);

	{
	    int i;
	    decide_draw_sprites();
	    for (i = 0; i < dip_for_drawing->nr_sprites; i++) {
		    draw_sprites_ecs (curr_sprite_entries + dip_for_drawing->first_sprite_entry + i);
	    }
	}
	do_color_changes (pfield_do_fill_line, (void (*)(int, int))pfield_do_linetoscr);
	do_flush_line (gfx_ypos);
    } else {
	adjust_drawing_colors (dp_for_drawing->ctable);

	if (dip_for_drawing->nr_color_changes == 0) {
	    fill_line ();
	    do_flush_line (gfx_ypos);
	    return;
	}

	playfield_start = VISIBLE_RIGHT_BORDER;
	playfield_end = VISIBLE_RIGHT_BORDER;

	do_color_changes (pfield_do_fill_line, pfield_do_fill_line);
	do_flush_line (gfx_ypos);
    }
}

static _INLINE_ void init_drawing_frame (void)
{
    init_hardware_for_drawing_frame ();

#ifdef USE_LINESTATE
    {
	register int i=0;
	register int max=(maxvpos>>2)+1;
	register unsigned *ptr=(unsigned *)&linestate[0];
	for(i=0;i<max;i++,ptr++)
		*ptr=0x01010101;
    }
#endif

#ifdef USE_RASTER_DRAW
    last_drawn_line = 0;
    first_drawn_line = 32767;

    first_block_line = last_block_line = -2;
    if (frame_redraw_necessary)
	frame_redraw_necessary--;
#endif

    thisframe_y_adjust = minfirstline;

    thisframe_y_adjust_real = thisframe_y_adjust;
    max_ypos_thisframe = (maxvpos - thisframe_y_adjust);

    max_diwstop = 0;
    min_diwstart = 10000;

    drawing_color_matches = -1;
}

/*
 * Some code to put status information on the screen.
 */

#define TD_PADX 10
#define TD_PADY 2
#define TD_WIDTH 32
#define TD_LED_WIDTH 24
#define TD_LED_HEIGHT 4

#define TD_RIGHT 1
#define TD_BOTTOM 2

static int td_pos = (TD_RIGHT|TD_BOTTOM);

#define TD_NUM_WIDTH 7
#define TD_NUM_HEIGHT 7

#define TD_TOTAL_HEIGHT (TD_PADY * 2 + TD_NUM_HEIGHT)

static const char *numbers = { /* ugly */
"------ ------ ------ ------ ------ ------ ------ ------ ------ ------ "
"-xxxxx ---xx- -xxxxx -xxxxx -x---x -xxxxx -xxxxx -xxxxx -xxxxx -xxxxx "
"-x---x ----x- -----x -----x -x---x -x---- -x---- -----x -x---x -x---x "
"-x---x ----x- -xxxxx -xxxxx -xxxxx -xxxxx -xxxxx ----x- -xxxxx -xxxxx "
"-x---x ----x- -x---- -----x -----x -----x -x---x ---x-- -x---x -----x "
"-xxxxx ----x- -xxxxx -xxxxx -----x -xxxxx -xxxxx ---x-- -xxxxx -xxxxx "
"------ ------ ------ ------ ------ ------ ------ ------ ------ ------ "
};

#if !defined(DOUBLEBUFFER) && !defined(STATUS_ALWAYS)
int back_drive_track0=-1,back_drive_motor0=-1;
static int back_drive_track1=-1,back_drive_motor1=-1;
static int back_powerled=-1;
#endif

static __inline__ void putpixel (int x, xcolnr c8)
{
	register uae_u16 *p = (uae_u16 *)xlinebuffer + x;
	*p = c8;
}

static _INLINE_ void write_tdnumber (int x, int y, int num)
{
    int j;
    uae_u8 *numptr;
    
    numptr = (uae_u8 *)(numbers + num * TD_NUM_WIDTH + 10 * TD_NUM_WIDTH * y);
    for (j = 0; j < TD_NUM_WIDTH; j++) {
	putpixel (x + j, *numptr == 'x' ? xcolors[0xfff] : xcolors[0x000]);
	numptr++;
    }
}

static _INLINE_ void draw_status_line (int line)
{
    int x, y, i, j, led, on;
    int on_rgb, off_rgb, c;
    uae_u8 *buf;
    
    if (td_pos & TD_RIGHT)
        x = GFXVIDINFO_WIDTH - TD_PADX - 5*TD_WIDTH;
    else
        x = TD_PADX;

    y = line - (GFXVIDINFO_HEIGHT - TD_TOTAL_HEIGHT);
    xlinebuffer = row_map[line];

    uae4all_memclr(xlinebuffer, GFXVIDINFO_WIDTH * GFXVIDINFO_PIXBYTES);

    x+=100 - (TD_WIDTH*(NUM_DRIVES-1));
    for (led = 0; led < (NUM_DRIVES+1); led++) {
	int track;
	if (led > 0) {
	    track = gui_data.drive_track[led-1];
	    on = gui_data.drive_motor[led-1];
	    on_rgb = 0x0f0;
	    off_rgb = 0x040;
	} else {
	    track = -1;
	    on = gui_data.powerled;
	    on_rgb = 0xf00;
	    off_rgb = 0x400;
	}
	c = xcolors[on ? on_rgb : off_rgb];

	for (j = 0; j < TD_LED_WIDTH; j++) 
	    putpixel (x + j, c);

	if (y >= TD_PADY && y - TD_PADY < TD_NUM_HEIGHT) {
	    if (track >= 0) {
		int offs = (TD_WIDTH - 2 * TD_NUM_WIDTH) / 2;
		write_tdnumber (x + offs, y - TD_PADY, track / 10);
		write_tdnumber (x + offs + TD_NUM_WIDTH, y - TD_PADY, track % 10);
	    }
	}
	x += TD_WIDTH;
    }
}

void check_all_prefs(void)
{

	check_prefs_changed_audio ();
	check_prefs_changed_custom ();
	check_prefs_changed_cpu ();
	if (check_prefs_changed_gfx ()) {
	    init_row_map ();
	    init_aspect_maps ();
	    notice_screen_contents_lost ();
	    notice_new_xcolors ();
	}
}

static _INLINE_ void finish_drawing_frame (void)
{
    int i;

    for (i = 0; i < max_ypos_thisframe; i++) {
	int where,i1;
	int line = i + thisframe_y_adjust_real;

#ifdef USE_LINESTATE
	if (linestate[line] == LINE_UNDECIDED)
	    break;
#endif

	i1 = i + min_ypos_for_screen;
	where = amiga2aspect_line_map[i1];
#if defined(USE_ALL_LINES) || !defined(USE_LINESTATE)
	if (where >= GFXVIDINFO_HEIGHT - TD_TOTAL_HEIGHT)
	    break;
#endif
	if (where == -1)
	    continue;
	pfield_draw_line (line, where, amiga2aspect_line_map[i1 + 1]);
    }
#ifdef USE_RASTER_DRAW
    if (   (frame_redraw_necessary) ||
#else
    if (
#endif
#if !defined(DOUBLEBUFFER) && !defined(STATUS_ALWAYS)
	   (back_drive_track0!=gui_data.drive_track[0])
	|| (back_drive_motor0!=gui_data.drive_motor[0])
#if NUM_DRIVES > 1
	|| (back_drive_track1!=gui_data.drive_track[1])
	|| (back_drive_motor1!=gui_data.drive_motor[1])
#endif
	|| (back_powerled!=gui_data.powerled)
#else
	1
#endif
	)
    {
#if !defined(DOUBLEBUFFER) && !defined(STATUS_ALWAYS)
	back_drive_track0=gui_data.drive_track[0];
	back_drive_motor0=gui_data.drive_motor[0];
#if NUM_DRIVES > 1
	back_drive_track1=gui_data.drive_track[1];
	back_drive_motor1=gui_data.drive_motor[1];
#endif
	back_powerled=gui_data.powerled;
#endif
 	for (i = 0; i < TD_TOTAL_HEIGHT; i++) {
		int line = GFXVIDINFO_HEIGHT - TD_TOTAL_HEIGHT + i;
		draw_status_line (line);
		do_flush_line (line);
    	}
    }
#ifdef USE_RASTER_DRAW
    drawfinished=1;
    do_flush_screen (first_drawn_line, last_drawn_line);
#else
    flush_screen ();
#endif
}


void vsync_handle_redraw (int long_frame, int lof_changed)
{
    last_redraw_point++;
    if (lof_changed || ! interlace_seen || last_redraw_point >= 2 || long_frame) {
	last_redraw_point = 0;
	interlace_seen = 0;

	if (framecnt == 0 || framecnt_hack) {
	    framecnt = 0;
	    finish_drawing_frame ();
	}
#ifndef USE_ALL_LINES
	framecnt_hack = 0;
#endif

	/* At this point, we have finished both the hardware and the
	 * drawing frame. Essentially, we are outside of all loops and
	 * can do some things which would cause confusion if they were
	 * done at other times.
	 */

	if (savestate_state == STATE_DOSAVE)
	{
		custom_prepare_savestate ();
		savestate_state = STATE_SAVE;
		pause_sound();
		save_state (savestate_filename, "Description!");
		resume_sound();
    		gui_set_message("Saved", 50);
		savestate_state = 0;
	}
	else
		if (savestate_state == STATE_DORESTORE)
		{
			pause_sound();
			savestate_state = STATE_RESTORE;
		        uae_reset ();
		}

	if (quit_program < 0) {
	    quit_program = -quit_program;
	    set_inhibit_frame (IHF_QUIT_PROGRAM);
	    set_special (SPCFLAG_BRK);
#ifdef USE_FAME_CORE
            m68k_stop_emulating();
#endif
	    return;
	}

	count_frame ();

	if (inhibit_frame != 0)
	    framecnt = 1;

	if (framecnt == 0)
	    init_drawing_frame ();
    }
}

#if defined(USE_LINESTATE) || !defined(USE_ALL_LINES)
void hsync_record_line_state (int lineno, int changed)
{
    char *state;

    if (framecnt != 0)
	return;
#ifdef USE_LINESTATE
    state = linestate + lineno;
#ifdef USE_RASTER_DRAW
    changed += frame_redraw_necessary;
#endif
#endif
#ifndef USE_ALL_LINES
    if (amiga2aspect_line_map[lineno - thisframe_y_adjust_real + min_ypos_for_screen] >= GFXVIDINFO_HEIGHT - TD_TOTAL_HEIGHT) {
#ifdef USE_LINESTATE
	    *state = LINE_UNDECIDED;
#endif
	    framecnt = framecnt_hack = 1;
    } else
#endif
#ifdef USE_LINESTATE
  	  *state = changed ? LINE_DECIDED : LINE_DONE;
#else
    ;
#endif
}
#endif

void reset_drawing (void)
{
    int i;

    inhibit_frame = 0;

    max_diwstop = 0;

#ifdef USE_LINESTATE
    for (i = 0; i < sizeof linestate / sizeof *linestate; i++)
	linestate[i] = LINE_UNDECIDED;
#endif

    xlinebuffer = gfx_mem;

    init_aspect_maps ();

    if (line_drawn == 0)
	line_drawn = (char *)xmalloc (GFXVIDINFO_HEIGHT);

    init_row_map();

    last_redraw_point = 0;

    uae4all_memclr(spixels, sizeof spixels);
    uae4all_memclr(&spixstate, sizeof spixstate);

    init_drawing_frame ();
}

void reset_screen_pointers (void)
{
	xlinebuffer = gfx_mem;
	init_row_map();
}

void drawing_init ()
{
    native2amiga_line_map = 0;
    amiga2aspect_line_map = 0;
    line_drawn = 0;

    gen_pfield_tables();
}

