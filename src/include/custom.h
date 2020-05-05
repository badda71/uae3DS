 /*
  * UAE - The Un*x Amiga Emulator
  *
  * custom chip support
  *
  * (c) 1995 Bernd Schmidt
  */

/* These are the masks that are ORed together in the chipset_mask option.
 * If CSMASK_AJA is set, the ECS bits are guaranteed to be set as well.  */

#ifndef UAE_CUSTOM_H
#define UAE_CUSTOM_H

#define CSMASK_ECS_AGNUS 1
#define CSMASK_ECS_DENISE 2

uae_u32 get_copper_address(int copno);

extern void custom_init (void);
extern void customreset (void);
extern int intlev (void);
extern void dumpcustom (void);

extern void do_disk (void);
extern void do_copper (void);

extern void notice_new_xcolors (void);
#ifdef USE_RASTER_DRAW
extern void notice_screen_contents_lost (void);
#elif !defined(DOUBLEBUFFER) && !defined(STATUS_ALWAYS)
#define notice_screen_contents_lost() { extern int back_drive_track0; back_drive_track0=-1; }
#else
#define notice_screen_contents_lost() { }
#endif

extern void custom_prepare_savestate (void);
extern void init_hz (void);

/* Set to 1 to leave out the current frame in average frame time calculation.
 * Useful if the debugger was active.  */
extern int bogusframe;

extern uae_u16 dmacon;
extern uae_u16 intena,intreq;

// extern int current_hpos (void);
// extern int vpos;

extern int find_copper_record (uaecptr, int *, int *);

extern int n_frames;

#define dmaen(DMAMASK) (int)((DMAMASK & dmacon) && (dmacon & 0x200))

#define SPCFLAG_STOP 2
#define SPCFLAG_COPPER 4
#define SPCFLAG_INT 8
#define SPCFLAG_BRK 16
#define SPCFLAG_EXTRA_CYCLES 32
#define SPCFLAG_TRACE 64
#define SPCFLAG_DOTRACE 128
#define SPCFLAG_DOINT 256
#define SPCFLAG_BLTNASTY 512
#define SPCFLAG_EXEC 1024
#define SPCFLAG_MODE_CHANGE 8192


extern uae_u16 adkcon;

extern unsigned int joy0dir, joy1dir;
extern int joy0button, joy1button;

extern void INTREQ (uae_u16);
extern void INTREQ_0 (uae_u16);
extern uae_u16 INTREQR (void);

/* maximums for statically allocated tables */

#define MAXHPOS 227
#define MAXVPOS 312
//#define MAXVPOS 280

/* PAL/NTSC values */

/* The HRM says: The vertical blanking area (PAL) ranges from line 0 to line 29,
 * and no data can be displayed there. Nevertheless, we lose some overscan data
 * if minfirstline is set to 29. */

#define MAXHPOS_PAL MAXHPOS
#define MAXHPOS_NTSC MAXHPOS
//#define MAXVPOS_PAL 280
#define MAXVPOS_PAL 312
//#define MAXVPOS_PAL 282
#define MAXVPOS_NTSC 262
//#define MINFIRSTLINE_PAL 21
#define MINFIRSTLINE_PAL (42-8)
//#define MINFIRSTLINE_NTSC 18
#define MINFIRSTLINE_NTSC (39-8)
//#define VBLANK_ENDLINE_PAL 29
#define VBLANK_ENDLINE_PAL (32-8)
//#define VBLANK_ENDLINE_NTSC 24
#define VBLANK_ENDLINE_NTSC (21-8)
#define VBLANK_HZ_PAL 50
#define VBLANK_HZ_NTSC 60

extern int maxhpos, maxvpos, minfirstline, numscrlines;
extern unsigned long syncbase;
#define NUMSCRLINES (maxvpos+1-minfirstline+1)

#define DMA_AUD0      0x0001
#define DMA_AUD1      0x0002
#define DMA_AUD2      0x0004
#define DMA_AUD3      0x0008
#define DMA_DISK      0x0010
#define DMA_SPRITE    0x0020
#define DMA_BLITTER   0x0040
#define DMA_COPPER    0x0080
#define DMA_BITPLANE  0x0100
#define DMA_MASTER    0x0200
#define DMA_BLITPRI   0x0400

extern unsigned long frametime, timeframes;

/* 100 words give you 1600 horizontal pixels. Should be more than enough for
 * superhires. Don't forget to update the definition in genp2c.c as well.
 * needs to be larger for superhires support */
//#define MAX_WORDS_PER_LINE 100
#define MAX_WORDS_PER_LINE 40

extern uae_u32 hirestab_h[256][2];
extern uae_u32 lorestab_h[256][4];

extern uae_u32 hirestab_l[256][1];
extern uae_u32 lorestab_l[256][2];

extern int bpl_off[8];

/* get resolution from bplcon0 */
//#define GET_RES(CON0) (((CON0) & 0x8000) ? RES_HIRES : ((CON0) & 0x40) ? RES_SUPERHIRES : RES_LORES)
#define GET_RES(CON0) (((CON0) & 0x8000) >> 15)
/* get sprite width from FMODE */
#define GET_SPRITEWIDTH(FMODE) ((((FMODE) >> 2) & 3) == 3 ? 64 : (((FMODE) >> 2) & 3) == 0 ? 16 : 32)
/* Compute the number of bitplanes from a value written to BPLCON0  */
#define GET_PLANES(x) ((((x) >> 12) & 7) | (((x) & 0x10) >> 1))

#endif
