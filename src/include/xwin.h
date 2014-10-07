 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Interface to the graphics system (X, SVGAlib)
  *
  * Copyright 1995-1997 Bernd Schmidt
  */

typedef long int xcolnr;

typedef int (*allocfunc_type)(int, int, int, xcolnr *);

extern xcolnr xcolors[4096];

extern int buttonstate[3];
extern int newmousecounters;
extern int lastmx, lastmy;
extern int ievent_alive;

extern int graphics_setup (void);
extern int graphics_init (void);
extern void graphics_leave (void);
extern void handle_events (void);
extern void setup_brkhandler (void);

extern void flush_line (int);
extern void flush_block (int, int);
extern void flush_screen (void);
void black_screen_now(void);

#if !defined(DREAMCAST) && !defined(DINGOO)
extern int lockscr (void);
extern void unlockscr (void);
#endif

extern int debuggable (void);
extern int needmousehack (void);
extern void togglemouse (void);
extern void LED (int);

extern int bits_in_mask (unsigned long mask);
extern int mask_shift (unsigned long mask);
extern unsigned long doMask (int p, int bits, int shift);
extern unsigned long doMask256 (int p, int bits, int shift);
extern void setup_maxcol (int);
extern void alloc_colors256 (int (*)(int, int, int, xcolnr *));
extern void alloc_colors64k (int, int, int, int, int, int);
extern void setup_greydither (int bits, allocfunc_type allocfunc);
extern void setup_greydither_maxcol (int maxcol, allocfunc_type allocfunc);
extern void setup_dither (int bits, allocfunc_type allocfunc);
extern void DitherLine (uae_u8 *l, uae_u16 *r4g4b4, int x, int y, uae_s16 len, int bits) ASM_SYM_FOR_FUNC("DitherLine");


extern char *gfx_mem;
extern unsigned gfx_rowbytes;

/* For ports using tui.c, this should be built by graphics_setup(). */
extern struct bstring *video_mode_menu;
extern void vidmode_menu_selected(int);
