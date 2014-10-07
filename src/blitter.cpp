 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Custom chip emulation
  *
  * (c) 1995 Bernd Schmidt, Alessandro Bissacco
  */


// #define USE_BLITTER_EXTRA_INLINE
#define STOP_WHEN_NASTY

#include "sysconfig.h"
#include "sysdeps.h"

#include "config.h"
#include "uae.h"
#include "options.h"
#include "debug_uae4all.h"
#include "events.h"
#include "memory.h"
#include "custom.h"
#include "m68k/m68k_intrf.h"
#include "blitter.h"

#ifdef USE_BLITTER_EXTRA_INLINE
#define _INLINE_ __inline__
#else
#define _INLINE_ 
#endif

#ifdef STOP_WHEN_NASTY
static __inline__ void setnasty(void)
{
#ifdef USE_FAME_CORE
	m68k_stop_emulating();
#endif
	set_special (SPCFLAG_BLTNASTY);
}
#else
#define setnasty() set_special (SPCFLAG_BLTNASTY)
#endif

uae_u16 oldvblts;
uae_u16 bltcon0,bltcon1;
uae_u32 bltapt,bltbpt,bltcpt,bltdpt;

int blinea_shift;
static uae_u16 blitlpos, blinea, blineb;
static uaecptr bltcnxlpt,bltdnxlpt;
static int blitsing; // blitline, blitdesc;
static int blitonedot,blitsign;
static long int bltwait;

struct bltinfo blt_info;

uae_u8 blit_filltable[256][4][2];
static uae_u16 blit_trashtable[BLITTER_MAX_WORDS];
enum blitter_states bltstate;

#ifdef USE_BLIT_MASKTABLE
uae_u32 blit_masktable[BLITTER_MAX_WORDS];
void build_blitfilltable(void)
{
    unsigned int d, fillmask;
    int i;

    for (i = 0; i < BLITTER_MAX_WORDS; i++)
	blit_masktable[i] = 0xFFFF;

    for (d = 0; d < 256; d++) {
	for (i = 0; i < 4; i++) {
	    int fc = i & 1;
	    uae_u8 data = d;
	    for (fillmask = 1; fillmask != 0x100; fillmask <<= 1) {
		uae_u16 tmp = data;
		if (fc) {
		    if (i & 2)
			data |= fillmask;
		    else
			data ^= fillmask;
		}
		if (tmp & fillmask) fc = !fc;
	    }
	    blit_filltable[d][i][0] = data;
	    blit_filltable[d][i][1] = fc;
	}
    }
}
#endif

static __inline__ uae_u8 * blit_xlateptr(uaecptr bltpt, int bytecount)
{
    if (!chipmem_bank.check(bltpt,bytecount)) return NULL;
    return chipmem_bank.xlateaddr(bltpt);
}

static __inline__ uae_u8 * blit_xlateptr_desc(uaecptr bltpt, int bytecount)
{
    if (!chipmem_bank.check(bltpt-bytecount, bytecount)) return NULL;
    return chipmem_bank.xlateaddr(bltpt);
}

#ifdef DEBUG_BLITTER
static _INLINE_ void print_bltinfo(struct bltinfo *_GCCRES_ b)
{
	dbg(" ---- bltinfo ----");
	if (b)
	{
		unsigned i, sum=0;
#ifdef USE_BLIT_MASKTABLE
		for(i=0;i<BLITTER_MAX_WORDS;i++)
			sum+=blit_masktable[i];
#endif
		dbgf("\t blitzero=0x%X, blitashift=0x%X, blitbshift=0x%X, blitdownashift=0x%X\n",b->blitzero,b->blitashift,b->blitbshift,b->blitdownashift);
		dbgf("\t blitdownbshift=0x%X, bltadat=0x%X, bltbdat=0x%X, bltcdat=0x%X\n",b->blitdownbshift,b->bltadat,b->bltbdat,b->bltcdat);
		dbgf("\t bltddat=0x%X, bltahold=0x%X, bltbhold=0x%X, bltafwm=0x%X\n",b->bltddat,b->bltahold,b->bltbhold,b->bltafwm);
		dbgf("\t bltalwm=0x%X, vblitsize=0x%X, hblitsize=0x%X, bltamod=0x%X\n",b->bltalwm,b->vblitsize,b->hblitsize,b->bltamod);
		dbgf("\t bltbmod=0x%X, bltcmod=0x%X, bltdmod=0x%X, masksum=0x%X\n",b->bltbmod,b->bltcmod,b->bltdmod,sum);
	}
	dbg("      ....... ");
	dbgf("\t oldvblts=0x%X, bltcon0=0x%X, bltcon1=0x%X, blinea_shift=0x%X\n",oldvblts,bltcon0,bltcon1,blinea_shift);
	dbgf("\t bltapt=0x%X, bltbpt=0x%X, bltcpt=0x%X, bltdpt\n",bltapt,bltbpt,bltcpt,bltdpt);
	dbg(" ---- ------- ----");
}
#endif

static void blitter_dofast(void)
{
#ifdef USE_LARGE_BLITFUNC
    uaecptr bltadatptr = 0, bltbdatptr = 0, bltcdatptr = 0, bltddatptr = 0;
#endif
#ifndef USE_SHORT_BLITTABLE
    uae_u8 mt = (bltcon0 & 0xFF) | blt_info.blitfill;
#else
    uae_u8 mt = bltcon0 & 0xFF;
#endif

#ifdef USE_BLIT_MASKTABLE
    blit_masktable[0] = blt_info.bltafwm;
    blit_masktable[blt_info.hblitsize - 1] &= blt_info.bltalwm;
#endif

#ifdef DEBUG_BLITTER
    dbgf("blitter_dofast bltafwm=0x%X, bltcon0=0x%X\n",blt_info.bltafwm,bltcon0);
#endif
    if (bltcon0 & 0x800) {
//    if ((bltcon0 & 0x800)&&(bltapt<=0xFFFFF)) {
#ifndef USE_LARGE_BLITFUNC
	blt_info.pta = bltapt;
#else
	bltadatptr = bltapt; //&0xFFFFF;
#endif
/*
    	if (bltadatptr>0xFFFFF)
    	{
    		blit_masktable[0] = 0xFFFF;
 	  	blit_masktable[blt_info.hblitsize - 1] = 0xFFFF;
    		bltstate = BLT_done;
		return;
   	}
*/
	bltapt += ((blt_info.hblitsize*2) + blt_info.bltamod)*blt_info.vblitsize;
    }
#ifndef USE_LARGE_BLITFUNC
    else 
	blt_info.pta = 0;
#endif
    if (bltcon0 & 0x400) {
//    if ((bltcon0 & 0x400)&&(bltbpt<=0xFFFFF)) {
#ifndef USE_LARGE_BLITFUNC
	blt_info.ptb = bltbpt;
#else
	bltbdatptr = bltbpt; //&0xFFFFF;
#endif
/*
    	if (bltbdatptr>0xFFFFF)
    	{
    		blit_masktable[0] = 0xFFFF;
 	  	blit_masktable[blt_info.hblitsize - 1] = 0xFFFF;
    		bltstate = BLT_done;
		return;
   	}
*/
	bltbpt += ((blt_info.hblitsize*2) + blt_info.bltbmod)*blt_info.vblitsize;
    }
#ifndef USE_LARGE_BLITFUNC
    else 
	blt_info.ptb = 0;
#endif
    if (bltcon0 & 0x200) {
//    if ((bltcon0 & 0x200)&&(bltcpt<=0xFFFFF)) {
#ifndef USE_LARGE_BLITFUNC
	blt_info.ptc = bltcpt;
#else
	bltcdatptr = bltcpt; //&0xFFFFF;
#endif
/*
    	if (bltcdatptr>0xFFFFF)
    	{
    		blit_masktable[0] = 0xFFFF;
 	  	blit_masktable[blt_info.hblitsize - 1] = 0xFFFF;
    		bltstate = BLT_done;
		return;
   	}
*/
	bltcpt += ((blt_info.hblitsize*2) + blt_info.bltcmod)*blt_info.vblitsize;
    }
#ifndef USE_LARGE_BLITFUNC
    else 
	blt_info.ptc = 0;
#endif
    if (bltcon0 & 0x100) {
//    if ((bltcon0 & 0x100)&&(bltdpt<=0xFFFFF)) {
#ifndef USE_LARGE_BLITFUNC
	blt_info.ptd = bltdpt;
#else
	bltddatptr = bltdpt; //&0xFFFFF;
#endif
/*
    	if (bltddatptr>0xFFFFF)
    	{
    		blit_masktable[0] = 0xFFFF;
 	  	blit_masktable[blt_info.hblitsize - 1] = 0xFFFF;
    		bltstate = BLT_done;
		return;
   	}
*/
	bltdpt += ((blt_info.hblitsize*2) + blt_info.bltdmod)*blt_info.vblitsize;
    }
#ifndef USE_LARGE_BLITFUNC
    else 
	blt_info.ptd = 0;
#endif
#ifdef DEBUG_BLITTER
    print_bltinfo(&blt_info);
#endif

#ifdef USE_SHORT_BLITTABLE
    if (!blt_info.blitfill)
    {
#endif
#ifdef DEBUG_BLITTER
	dbgf("blitfunc_dofast[%i](0x%X,0x%X,0x%X,0x%X)\n",mt,bltadatptr, bltbdatptr, bltcdatptr, bltddatptr);
#endif
#ifndef USE_LARGE_BLITFUNC
	(*blitfunc_dofast[mt])(&blt_info);
#else
	(*blitfunc_dofast[mt])(bltadatptr, bltbdatptr, bltcdatptr, bltddatptr, &blt_info);
#endif
#ifdef USE_SHORT_BLITTABLE
    }
    else 
    {
#ifndef USE_LARGE_BLITFUNC
	    blitdofast_fill(&blt_info);
#else
	    blitdofast_fill(bltadatptr, bltbdatptr, bltcdatptr, bltddatptr, &blt_info);
#endif
    }
#endif
#ifdef USE_BLIT_MASKTABLE
    blit_masktable[0] = 0xFFFF;
    blit_masktable[blt_info.hblitsize - 1] = 0xFFFF;
#endif

    bltstate = BLT_done;
    blitter_done_notify ();
}

static void blitter_dofast_desc(void)
{
    int i,j;
#ifdef USE_LARGE_BLITFUNC
    uaecptr bltadatptr = 0, bltbdatptr = 0, bltcdatptr = 0, bltddatptr = 0;
#endif
#ifndef USE_SHORT_BLITTABLE
    uae_u8 mt = (bltcon0 & 0xFF) | blt_info.blitfill;
#else
    uae_u8 mt = bltcon0 & 0xFF;
#endif

#ifdef USE_BLIT_MASKTABLE
    blit_masktable[0] = blt_info.bltafwm;
    blit_masktable[blt_info.hblitsize - 1] &= blt_info.bltalwm;
#endif

#ifdef DEBUG_BLITTER
    dbgf("blitter_dofast_desc bltafwm=0x%X, bltcon0=0x%X\n",blt_info.bltafwm,bltcon0);
#endif
    if (bltcon0 & 0x800) {
#ifndef USE_LARGE_BLITFUNC
	blt_info.pta=bltapt;
#else
	bltadatptr = bltapt;
#endif
	bltapt -= ((blt_info.hblitsize*2) + blt_info.bltamod)*blt_info.vblitsize;
    }
#ifndef USE_LARGE_BLITFUNC
    else
	blt_info.pta=0;
#endif
    if (bltcon0 & 0x400) {
#ifndef USE_LARGE_BLITFUNC
	blt_info.ptb=bltbpt;
#else
	bltbdatptr = bltbpt;
#endif
	bltbpt -= ((blt_info.hblitsize*2) + blt_info.bltbmod)*blt_info.vblitsize;
    }
#ifndef USE_LARGE_BLITFUNC
    else
	blt_info.ptb=0;
#endif
    if (bltcon0 & 0x200) {
#ifndef USE_LARGE_BLITFUNC
	blt_info.ptc=bltcpt;
#else
	bltcdatptr = bltcpt;
#endif
	bltcpt -= ((blt_info.hblitsize*2) + blt_info.bltcmod)*blt_info.vblitsize;
    }
#ifndef USE_LARGE_BLITFUNC
    else
	blt_info.ptc=0;
#endif
    if (bltcon0 & 0x100) {
#ifndef USE_LARGE_BLITFUNC
	blt_info.ptd=bltdpt;
#else
	bltddatptr = bltdpt;
#endif
	bltdpt -= ((blt_info.hblitsize*2) + blt_info.bltdmod)*blt_info.vblitsize;
    }
#ifndef USE_LARGE_BLITFUNC
    else
	blt_info.ptd=0;
#endif

#ifdef DEBUG_BLITTER
    print_bltinfo(&blt_info);
#endif

#ifdef USE_SHORT_BLITTABLE
    if (!blt_info.blitfill)
    {
#endif
#ifdef DEBUG_BLITTER
	dbgf("blitfunc_dofast_desc[%i](0x%X,0x%X,0x%X,0x%X)\n",mt,bltadatptr, bltbdatptr, bltcdatptr, bltddatptr);
#endif
#ifndef USE_LARGE_BLITFUNC
	(*blitfunc_dofast_desc[mt])(&blt_info);
#else
	(*blitfunc_dofast_desc[mt])(bltadatptr, bltbdatptr, bltcdatptr, bltddatptr, &blt_info);
#endif
#ifdef USE_SHORT_BLITTABLE
    }
    else
    {
#ifndef USE_LARGE_BLITFUNC
	    blitdofast_desc_fill(&blt_info);
#else
	    blitdofast_desc_fill(bltadatptr, bltbdatptr, bltcdatptr, bltddatptr, &blt_info);
#endif
    }
#endif
#ifdef USE_BLIT_MASKTABLE
    blit_masktable[0] = 0xFFFF;
    blit_masktable[blt_info.hblitsize - 1] = 0xFFFF;
#endif

    bltstate = BLT_done;
    blitter_done_notify ();
}

static __inline__ void blitter_read(void)
{
#ifdef DEBUG_BLITTER
    dbgf("blitter_read -> dmaen=0x%X\n",dmaen(DMA_BLITTER));
#endif
    if (bltcon0 & 0xe00){
	if (!dmaen(DMA_BLITTER))
	    return ;//1;
	if (bltcon0 & 0x200) blt_info.bltcdat = chipmem_bank.wget(bltcpt);
    }
    bltstate = BLT_work;
#ifdef DEBUG_BLITTER
    dbgf("\t ret=0x%X\n",((bltcon0 & 0xE00) != 0));
#endif
//    return (bltcon0 & 0xE00) != 0;
}

static __inline__ void blitter_write(void)
{
#ifdef DEBUG_BLITTER
    dbgf("blitter_write -> dmaen=0x%X\n",dmaen(DMA_BLITTER));
#endif
    if (blt_info.bltddat) blt_info.blitzero = 0;
    if (bltcon0 & 0x100){
	if (!dmaen(DMA_BLITTER)) return ; //1;
	chipmem_bank.wput(bltdpt, blt_info.bltddat);
    }
    bltstate = BLT_next;
#ifdef DEBUG_BLITTER
    dbgf("\t ret=0x%X\n",((bltcon0 & 0x100) != 0));
#endif
//    return (bltcon0 & 0x100) != 0;
}

static __inline__ void blitter_line_incx(void)
{
    if (++blinea_shift == 16) {
	blinea_shift = 0;
	bltcnxlpt += 2;
	bltdnxlpt += 2;
    }
#ifdef DEBUG_BLITTER
    dbgf("blitter_line_incx -> blinea_shift=0x%X, bltcnxlpt=0x%X, bltdnxlpt=0x%X\n",blinea_shift,bltcnxlpt,bltdnxlpt);
#endif
}

static __inline__ void blitter_line_decx(void)
{
    if (blinea_shift-- == 0) {
	blinea_shift = 15;
	bltcnxlpt -= 2;
	bltdnxlpt -= 2;
    }
#ifdef DEBUG_BLITTER
    dbgf("blitter_line_decx -> blinea_shift=0x%X, bltcnxlpt=0x%X, bltdnxlpt=0x%X\n",blinea_shift,bltcnxlpt,bltdnxlpt);
#endif
}

static __inline__ void blitter_line_decy(void)
{
    bltcnxlpt -= blt_info.bltcmod;
    bltdnxlpt -= blt_info.bltcmod; /* ??? am I wrong or doesn't KS1.3 set bltdmod? */
    blitonedot = 0;
#ifdef DEBUG_BLITTER
    dbgf("blitter_line_decy -> bltcnxlpt=0x%X, bltdnxlpt=0x%X\n",bltcnxlpt,bltdnxlpt);
#endif
}

static __inline__ void blitter_line_incy(void)
{
    bltcnxlpt += blt_info.bltcmod;
    bltdnxlpt += blt_info.bltcmod; /* ??? */
    blitonedot = 0;
#ifdef DEBUG_BLITTER
    dbgf("blitter_line_incy -> bltcnxlpt=0x%X, bltdnxlpt=0x%X\n",bltcnxlpt,bltdnxlpt);
#endif
}

#ifndef USE_BLIT_FUNC
typedef uae_u32 (*blit_func_tbl_t)(uae_u32, uae_u32, uae_u32);
extern blit_func_tbl_t blit_func_tbl[0x100];
#define blit_func(srca,srcb,srcc,mt) (blit_func_tbl[mt])(srca,srcb,srcc)
#else
uae_u32 blit_func(uae_u32 srca, uae_u32 srcb, uae_u32 srcc, uae_u8 mt);
#endif

static void blitter_line(void)
{
	do {
		blitter_read();
		{
		    uae_u16 blitahold = blinea >> blinea_shift, blitbhold = blineb & 1 ? 0xFFFF : 0, blitchold = blt_info.bltcdat;
		    blt_info.bltddat = 0;
#ifdef DEBUG_BLITTER
		    dbgf("blitter_line blitahold=0x%X, blinea=0x%X, blinea_shift=0x%X, blineb=0x%X\n",blitahold,blinea,blinea_shift,blineb);
#endif
		    if (blitsing && blitonedot) blitahold = 0;
		    blitonedot = 1;
		    blt_info.bltddat = blit_func(blitahold, blitbhold, blitchold, bltcon0 & 0xFF);
		    if (!blitsign){
			bltapt += (uae_s16)blt_info.bltamod;
			if (bltcon1 & 0x10){
			    if (bltcon1 & 0x8)
				blitter_line_decy();
			    else
				blitter_line_incy();
			} else {
			    if (bltcon1 & 0x8)
				blitter_line_decx();
			    else
				blitter_line_incx();
			}
		    } else {
			bltapt += (uae_s16)blt_info.bltbmod;
		    }
		    if (bltcon1 & 0x10){
			if (bltcon1 & 0x4)
			    blitter_line_decx();
			else
			    blitter_line_incx();
		    } else {
			if (bltcon1 & 0x4)
			    blitter_line_decy();
			else
			    blitter_line_incy();
		    }
		    blitsign = 0 > (uae_s16)bltapt;
		    bltstate = BLT_write;
		}
		blitter_write();
		{
		    bltcpt = bltcnxlpt;
		    bltdpt = bltdnxlpt;
		    blineb = (blineb << 1) | (blineb >> 15);
		    if (--blt_info.vblitsize == 0) {
			bltstate = BLT_done;
		    } else {
			bltstate = BLT_read;
		    }
#ifdef DEBUG_BLITTER
		    dbgf("blitter_nxline -> bltcpt=0x%X, bltdpt=0x%X, blineb=0x%X\n",bltcpt,bltdpt,blineb);
#endif
		}
	} while (bltstate != BLT_done);
}

typedef void (*actually_do_blit_func)(void);
static actually_do_blit_func actually_do_blit=blitter_dofast;

static _INLINE_ void blit_init(void)
{
    int blitline = bltcon1 & 1;
    blitlpos = 0;
    blt_info.blitzero = 1;
    blt_info.blitashift = bltcon0 >> 12;
    blt_info.blitdownashift = 16 - blt_info.blitashift;
    blt_info.blitbshift = bltcon1 >> 12;
    blt_info.blitdownbshift = 16 - blt_info.blitbshift;

    if (blitline) {
	actually_do_blit=blitter_line;
	if (blt_info.hblitsize != 2)
	    write_log ("weird hblitsize in linemode: %d\n", blt_info.hblitsize);

	bltcnxlpt = bltcpt;
	bltdnxlpt = bltdpt;
	blitsing = bltcon1 & 0x2;
	blinea = blt_info.bltadat;
	blineb = (blt_info.bltbdat >> blt_info.blitbshift) | (blt_info.bltbdat << (16-blt_info.blitbshift));
#if 0
	if (blineb != 0xFFFF && blineb != 0)
	    write_log ("%x %x %d %x\n", blineb, blt_info.bltbdat, blt_info.blitbshift, bltcon1);
#endif
	blitsign = bltcon1 & 0x40;
	blitonedot = 0;
#ifdef DEBUG_BLITTER
	dbgf("blit_init blinea=0x%X, blineb=0x%X, bltcnxlpt=0x%X, bltdnxlpt=0x%X,blitsing=0x%X\n",blinea,blineb,bltcnxlpt,bltdnxlpt,blitsing);
#endif
    } else {
	blt_info.blitfc = !!(bltcon1 & 0x4);
	blt_info.blitife = bltcon1 & 0x8;
#ifndef USE_SHORT_BLITTABLE
	blt_info.blitfill = (bltcon1 & 0x18)?0x100:0;
#else
	blt_info.blitfill = bltcon1 & 0x18;
#endif
#if 0
	if ((bltcon1 & 0x18) == 0x18) {
	    /* Digital "Trash" demo does this; others too. Apparently, no
	     * negative effects. */
	    static int warn = 1;
	    if (warn)
		write_log ("warning: weird fill mode (further messages suppressed)\n");
	    warn = 0;
	}
#endif
	if (bltcon1 & 0x2)
		actually_do_blit=blitter_dofast_desc;
	else
		actually_do_blit=blitter_dofast;
#if 0
	if (blt_info.blitfill && !blitdesc) {
	    static int warn = 1;
	    if (warn)
		write_log ("warning: blitter fill without desc (further messages suppressed)\n");
	    warn = 0;
	}
#endif
#ifdef DEBUG_BLITTER
	dbgf("blit_init blitfc=0x%X, blitife=0x%X, blitfill=0x%X, blitdesc=0x%X\n",blt_info.blitfc,blt_info.blitife,blt_info.blitfill,blitdesc);
#endif
    }
}

#if 0
static _INLINE_ void actually_do_blit(void)
{
#ifdef DEBUG_BLITTER
    dbgf("actually_do_blit -> blitline=0x%X\n",blitline);
#endif
    if (blitline) {
	    blitter_line();
    } else {
	/*blitcount[bltcon0 & 0xff]++;  blitter debug */
	if (blitdesc) blitter_dofast_desc();
	else blitter_dofast();
    }
}
#endif


void blitter_handler(void)
{
	uae4all_prof_start(6);
#ifdef DEBUG_BLITTER
    dbg(" blitter_handler(void)");
#endif
    if (!dmaen(DMA_BLITTER)) {
#ifdef DEBUG_BLITTER
	dbg("ACTIVADO EVENTO BLITTER");
#endif
	eventtab[ev_blitter].active = 1;
	eventtab[ev_blitter].oldcycles = get_cycles ();
	eventtab[ev_blitter].evtime = 10 * CYCLE_UNIT + get_cycles (); /* wait a little */
	uae4all_prof_end(6);
	return; /* gotta come back later. */
    }
    actually_do_blit();

    INTREQ(0x8040);

    eventtab[ev_blitter].active = 0;
    unset_special (SPCFLAG_BLTNASTY);
    uae4all_prof_end(6);
}

static uae_u8 blit_cycle_diagram_start[][10] =
{
    { 0, 1, 0 },		/* 0 */
    { 0, 2, 4,0 },		/* 1 */
    { 0, 2, 3,0 },		/* 2 */
    { 2, 3, 3,0, 0,3,4 },	/* 3 */
    { 0, 3, 2,0,0 },		/* 4 */
    { 2, 3, 2,0, 0,2,4 },	/* 5 */
    { 0, 3, 2,3,0 },		/* 6 */
    { 3, 4, 2,3,0, 0,2,3,4 },	/* 7 */
    { 0, 2, 1,0 },		/* 8 */
    { 2, 2, 1,0, 1,4 },		/* 9 */
    { 0, 2, 1,3 },		/* A */
    { 3, 3, 1,3,0, 1,3,4 },	/* B */
    { 2, 3, 1,2, 0,1,2 },	/* C */
    { 3, 3, 1,2,0, 1,2,4 },	/* D */
    { 0, 3, 1,2,3 },		/* E */
    { 4, 4, 1,2,3,0, 1,2,3,4 }	/* F */
};

long blit_firstline_cycles;
static long blit_first_cycle;
static int blit_last_cycle;
static uae_u8 *blit_diag;

void do_blitter(void)
{
    extern int mainMenu_throttle;
    int blit_cycles;
    uae4all_prof_start(6);
#ifdef DEBUG_BLITTER
    dbg("DO_BLITTER");
#endif
    int ch = (bltcon0 & 0x0f00) >> 8;
    blit_diag = blit_cycle_diagram_start[ch];

    blit_firstline_cycles = blit_first_cycle = get_cycles ();
    blit_last_cycle = 0;
#ifdef DEBUG_BLITTER
    dbgf("DO_BLITTER ch=0x%X\n",ch);
#endif
#ifndef USE_BLITTER_DELAYED
    blit_cycles = 1;
#else
    blit_cycles=1<<mainMenu_throttle;
#endif

#ifdef DEBUG_BLITTER
    dbgf("DO_BLITTER blitline=0x%X, blit_cycles=0x%X, blit_firstline_cycles=0x%X\n",blitline,blit_cycles,blit_firstline_cycles);
#endif

    blit_init();

    eventtab[ev_blitter].active = 1;
#ifdef DEBUG_BLITTER
//  dbg("ACTIVADO EVENTO BLITTER TRAS DO_BLITTER");
#endif
    eventtab[ev_blitter].oldcycles = get_cycles ();
    eventtab[ev_blitter].evtime = blit_cycles *  CYCLE_UNIT + get_cycles ();
    events_schedule();

#ifdef DEBUG_BLITTER
    dbgf("DO_BLITTER dmaen=0x%X\n",dmaen(DMA_BLITPRI));
#endif
    if (dmaen(DMA_BLITPRI))
        setnasty();
    else
    	unset_special (SPCFLAG_BLTNASTY);

    uae4all_prof_end(6);
}

#ifdef USE_MAYBE_BLIT
void maybe_blit (int modulo)
{
#ifdef DEBUG_BLITTER
    dbgf("maybe_blit (%i) -> bltstate=0x%X\n",modulo,bltstate);
#endif
    if (bltstate == BLT_done)
	return;

/*
    if (!eventtab[ev_blitter].active)
	write_log ("FOO!!?\n");
*/

    if (modulo && get_cycles() < blit_firstline_cycles)
	return;
    blitter_handler ();
}
#endif

int blitnasty (void)
{
    uae4all_prof_start(6);
#ifdef DEBUG_BLITTER
    dbgf("blitnasty -> bltstate=0x%X, dmaen=0x%X\n",bltstate,dmaen(DMA_BLITTER));
#endif
    int cycles, ccnt;
#ifndef USE_FAME_CORE
    if (!(_68k_spcflags & SPCFLAG_BLTNASTY))
	return 0;
#endif
    if (bltstate == BLT_done)
	return 0;
    if (!dmaen(DMA_BLITTER))
	return 0;
    cycles = (get_cycles () - blit_first_cycle) / CYCLE_UNIT;
    ccnt = 0;
#ifdef DEBUG_BLITTER
    dbgf("\t blit_last_cycle=0x%X, cycles=0x%X\n",blit_last_cycle,cycles);
#endif
    while (blit_last_cycle < cycles) {
	int c;
	if (blit_last_cycle < blit_diag[0])
	    c = blit_diag[blit_last_cycle + 2];
	else
	    c = blit_diag[((blit_last_cycle - blit_diag[0]) % blit_diag[1]) + 2 + blit_diag[0]];
	blit_last_cycle ++;
	if (!c)
	    return 0;
	ccnt++;
    }
    uae4all_prof_end(6);
    return ccnt;
}
