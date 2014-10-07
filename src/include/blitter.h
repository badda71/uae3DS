 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Blitter emulation
  *
  * (c) 1995 Bernd Schmidt
  */

struct bltinfo {
    int blitzero;
    int blitashift,blitbshift,blitdownashift,blitdownbshift;
    uae_u16 bltadat, bltbdat, bltcdat,bltddat,bltahold,bltbhold,bltafwm,bltalwm;
    int vblitsize,hblitsize;
    int bltamod,bltbmod,bltcmod,bltdmod;
#ifndef USE_LARGE_BLITFUNC
    uaecptr pta, ptb, ptc, ptd;
#endif
    int blitfc,blitfill,blitife;
};
extern enum blitter_states {
    BLT_done, BLT_init, BLT_read, BLT_work, BLT_write, BLT_next
} bltstate;

extern struct bltinfo blt_info;

extern uae_u16 bltsize, oldvblts;
extern uae_u16 bltcon0,bltcon1;
extern int blinea_shift;
extern uae_u32 bltapt,bltbpt,bltcpt,bltdpt;

extern long blit_firstline_cycles;
#ifdef USE_MAYBE_BLIT
extern void maybe_blit (int);
#define maybe_blit1() maybe_blit(1)
#define maybe_blit0() maybe_blit(0)
#else
#define maybe_blit0() { if (bltstate != BLT_done) blitter_handler (); }
#define maybe_blit1() { if (bltstate != BLT_done && get_cycles() >= blit_firstline_cycles) blitter_handler (); }
#endif
extern int blitnasty (void);
extern void blitter_handler (void);
extern void build_blitfilltable (void);
extern void do_blitter (void);
extern void blitter_done_notify (void);
#ifndef USE_LARGE_BLITFUNC
typedef void blitter_func(struct bltinfo *_GCCRES_);
#else
typedef void blitter_func(uaecptr, uaecptr, uaecptr, uaecptr, struct bltinfo *_GCCRES_);
#endif

#define BLITTER_MAX_WORDS 2048

#ifndef USE_SHORT_BLITTABLE
extern blitter_func const *blitfunc_dofast[512];
extern blitter_func const *blitfunc_dofast_desc[512];
#else
extern blitter_func const *blitfunc_dofast[256];
extern blitter_func const *blitfunc_dofast_desc[256];
#endif
extern uae_u32 blit_masktable[BLITTER_MAX_WORDS];
extern blitter_func blitdofast_fill;
extern blitter_func blitdofast_desc_fill;
