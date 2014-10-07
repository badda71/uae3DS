 /*
  * UAE - The Un*x Amiga Emulator
  *
  * CIA chip support
  *
  * Copyright 1995 Bernd Schmidt, Alessandro Bissacco
  * Copyright 1996, 1997 Stefan Reinauer, Christian Schmitt
  */

#include "sysconfig.h"
#include "sysdeps.h"
#include <assert.h>

#include "debug_uae4all.h"

#include "config.h"
#include "uae.h"
#include "options.h"
#include "thread.h"
#include "events.h"
#include "memory.h"
#include "custom.h"
#include "cia.h"
#include "serial.h"
#include "disk.h"
#include "xwin.h"
#include "keybuf.h"
#include "gui.h"
#include "savestate.h"


#define DIV10 (5*CYCLE_UNIT) /* Yes, a bad identifier. */

/* battclock stuff */
#define RTC_D_ADJ      8
#define RTC_D_IRQ      4
#define RTC_D_BUSY     2
#define RTC_D_HOLD     1
#define RTC_E_t1       8
#define RTC_E_t0       4
#define RTC_E_INTR     2
#define RTC_E_MASK     1
#define RTC_F_TEST     8
#define RTC_F_24_12    4
#define RTC_F_STOP     2
#define RTC_F_RSET     1

static unsigned int clock_control_d = RTC_D_ADJ + RTC_D_HOLD;
static unsigned int clock_control_e = 0;
static unsigned int clock_control_f = RTC_F_24_12;

unsigned int ciaaicr, ciaaimask, ciabicr, ciabimask;
unsigned int ciaacra, ciaacrb, ciabcra, ciabcrb;


/* Values of the CIA timers.  */
unsigned long ciaata, ciaatb, ciabta, ciabtb;
/* Computed by compute_passed_time.  */
unsigned long ciaata_passed, ciaatb_passed, ciabta_passed, ciabtb_passed;

unsigned long ciaatod, ciabtod, ciaatol, ciabtol, ciaaalarm, ciabalarm;
int ciaatlatch, ciabtlatch;

unsigned int ciabpra;

unsigned int gui_ledstate;

static unsigned long ciaala, ciaalb, ciabla, ciablb;
static int ciaatodon, ciabtodon;
static unsigned int ciaapra, ciaaprb, ciaadra, ciaadrb, ciaasdr;
static unsigned int ciabprb, ciabdra, ciabdrb, ciabsdr;
static int div10;
static int kbstate, kback, ciaasdr_unread = 0;


static __inline__ void setclr (unsigned int *_GCCRES_ p, unsigned int val)
{
    if (val & 0x80) {
	*p |= val & 0x7F;
    } else {
	*p &= ~val;
    }
}

static void RethinkICRA (void)
{
    if (ciaaimask & ciaaicr) {
	ciaaicr |= 0x80;
#ifdef DEBUG_CIA
	dbg("Cia INTREQ_0 por RethinkICRA");
#endif
	INTREQ_0 (0x8008);
    } else {
	ciaaicr &= 0x7F;
/*	custom_bank.wput(0xDFF09C,0x0008);*/
    }
}

static void RethinkICRB (void)
{
    if (ciabimask & ciabicr) {
	ciabicr |= 0x80;
#ifdef DEBUG_CIA
	dbg("Cia INTREQ_0 por RethinkICRB");
#endif
	INTREQ_0 (0xA000);
    } else {
	ciabicr &= 0x7F;
    }
}

void rethink_cias (void)
{
    RethinkICRA ();
    RethinkICRB ();
}

/* Figure out how many CIA timer cycles have passed for each timer since the
   last call of CIA_calctimers.  */

static void compute_passed_time (void)
{
    unsigned long int ccount = (get_cycles () - eventtab[ev_cia].oldcycles + div10);
    unsigned long int ciaclocks = ccount / DIV10;

    ciaata_passed = ciaatb_passed = ciabta_passed = ciabtb_passed = 0;

    /* CIA A timers */
    if ((ciaacra & 0x21) == 0x01) {
	assert ((ciaata+1) >= ciaclocks);
	ciaata_passed = ciaclocks;
    }
    if ((ciaacrb & 0x61) == 0x01) {
	assert ((ciaatb+1) >= ciaclocks);
	ciaatb_passed = ciaclocks;
    }

    /* CIA B timers */
    if ((ciabcra & 0x21) == 0x01) {
	assert ((ciabta+1) >= ciaclocks);
	ciabta_passed = ciaclocks;
    }
    if ((ciabcrb & 0x61) == 0x01) {
	assert ((ciabtb+1) >= ciaclocks);
	ciabtb_passed = ciaclocks;
    }
}

/* Called to advance all CIA timers to the current time.  This expects that
   one of the timer values will be modified, and CIA_calctimers will be called
   in the same cycle.  */

static void CIA_update (void)
{
    unsigned long int ccount = (get_cycles () - eventtab[ev_cia].oldcycles + div10);
    unsigned long int ciaclocks = ccount / DIV10;

    int aovfla = 0, aovflb = 0, bovfla = 0, bovflb = 0;

    div10 = ccount % DIV10;

    /* CIA A timers */
    if ((ciaacra & 0x21) == 0x01) {
	assert ((ciaata+1) >= ciaclocks);
	if ((ciaata+1) == ciaclocks) {
	    aovfla = 1;
	    if ((ciaacrb & 0x61) == 0x41) {
		if (ciaatb-- == 0) aovflb = 1;
	    }
	}
	ciaata -= ciaclocks;
    }
    if ((ciaacrb & 0x61) == 0x01) {
	assert ((ciaatb+1) >= ciaclocks);
	if ((ciaatb+1) == ciaclocks) aovflb = 1;
	ciaatb -= ciaclocks;
    }

    /* CIA B timers */
    if ((ciabcra & 0x21) == 0x01) {
	assert ((ciabta+1) >= ciaclocks);
	if ((ciabta+1) == ciaclocks) {
	    bovfla = 1;
	    if ((ciabcrb & 0x61) == 0x41) {
		if (ciabtb-- == 0) bovflb = 1;
	    }
	}
	ciabta -= ciaclocks;
    }
    if ((ciabcrb & 0x61) == 0x01) {
	assert ((ciabtb+1) >= ciaclocks);
	if ((ciabtb+1) == ciaclocks) bovflb = 1;
	ciabtb -= ciaclocks;
    }
    if (aovfla) {
	ciaaicr |= 1; RethinkICRA();
	ciaata = ciaala;
	if (ciaacra & 0x8) ciaacra &= ~1;
    }
    if (aovflb) {
	ciaaicr |= 2; RethinkICRA();
	ciaatb = ciaalb;
	if (ciaacrb & 0x8) ciaacrb &= ~1;
    }
    if (bovfla) {
	ciabicr |= 1; RethinkICRB();
	ciabta = ciabla;
	if (ciabcra & 0x8) ciabcra &= ~1;
    }
    if (bovflb) {
	ciabicr |= 2; RethinkICRB();
	ciabtb = ciablb;
	if (ciabcrb & 0x8) ciabcrb &= ~1;
    }
}

/* Call this only after CIA_update has been called in the same cycle.  */

static void CIA_calctimers (void)
{
    long ciaatimea = -1, ciaatimeb = -1, ciabtimea = -1, ciabtimeb = -1;

    eventtab[ev_cia].oldcycles = get_cycles ();
    if ((ciaacra & 0x21) == 0x01) {
	ciaatimea = (DIV10 - div10) + DIV10 * ciaata;
    }
    if ((ciaacrb & 0x61) == 0x01) {
	ciaatimeb = (DIV10 - div10) + DIV10 * ciaatb;
    }

    if ((ciabcra & 0x21) == 0x01) {
	ciabtimea = (DIV10 - div10) + DIV10 * ciabta;
    }
    if ((ciabcrb & 0x61) == 0x01) {
	ciabtimeb = (DIV10 - div10) + DIV10 * ciabtb;
    }
    eventtab[ev_cia].active = (ciaatimea != -1 || ciaatimeb != -1
			       || ciabtimea != -1 || ciabtimeb != -1);
    if (eventtab[ev_cia].active) {
	unsigned long int ciatime = ~0L;
	if (ciaatimea != -1) ciatime = ciaatimea;
	if (ciaatimeb != -1 && ciaatimeb < ciatime) ciatime = ciaatimeb;
	if (ciabtimea != -1 && ciabtimea < ciatime) ciatime = ciabtimea;
	if (ciabtimeb != -1 && ciabtimeb < ciatime) ciatime = ciabtimeb;
	eventtab[ev_cia].evtime = ciatime + get_cycles ();
    }
    events_schedule();
}

void CIA_handler (void)
{
    uae4all_prof_start(5);
    CIA_update ();
    CIA_calctimers ();
    uae4all_prof_end(5);
}

void cia_diskindex (void)
{
    ciabicr |= 0x10;
    RethinkICRB();
}

void CIA_hsync_handler (void)
{
    uae4all_prof_start(5);
    static unsigned int keytime = 0, sleepyhead = 0;

    if (ciabtodon)
	ciabtod++;
    ciabtod &= 0xFFFFFF;

    if (ciabtod == ciabalarm) {
	ciabicr |= 4; RethinkICRB();
    }

    if (keys_available() && kback && (++keytime & 15) == 0) {
	/*
	 * This hack lets one possible ciaaicr cycle go by without any key
	 * being read, for every cycle in which a key is pulled out of the
	 * queue.  If no hack is used, a lot of key events just get lost
	 * when you type fast.  With a simple hack that waits for ciaasdr
	 * to be read before feeding it another, it will keep up until the
	 * queue gets about 14 characters ahead and then lose events, and
	 * the mouse pointer will freeze while typing is being taken in.
	 * With this hack, you can type 30 or 40 characters ahead with little
	 * or no lossage, and the mouse doesn't get stuck.  The tradeoff is
	 * that the total slowness of typing appearing on screen is worse.
	 */
	if (ciaasdr_unread == 2)
	    ciaasdr_unread = 0;
	else if (ciaasdr_unread == 0) {
	    switch (kbstate) {
	     case 0:
		ciaasdr = (uae_s8)~0xFB; /* aaarghh... stupid compiler */
		kbstate++;
		break;
	     case 1:
		kbstate++;
		ciaasdr = (uae_s8)~0xFD;
		break;
	     case 2:
		ciaasdr = ~get_next_key();
		ciaasdr_unread = 1;      /* interlock to prevent lost keystrokes */
		break;
	    }
	    ciaaicr |= 8;
	    RethinkICRA();
	    sleepyhead = 0;
	} else if (!(++sleepyhead & 15))
	    ciaasdr_unread = 0;          /* give up on this key event after unread for a long time */
    }
    uae4all_prof_end(5);
}

void CIA_vsync_handler ()
{
    uae4all_prof_start(5);
    if (ciaatodon)
	ciaatod++;
    ciaatod &= 0xFFFFFF;
    if (ciaatod == ciaaalarm) {
	ciaaicr |= 4;
	RethinkICRA();
    }
    uae4all_prof_end(5);
}

static uae_u8 ReadCIAA (unsigned int addr)
{
    unsigned int tmp;

    compute_passed_time ();

    switch (addr & 0xf) {
    case 0:
	tmp = (DISK_status() & 0x3C);
	if (!buttonstate[0])
	    tmp |= 0x40;
	if (!(joy1button & 1))
	    tmp |= 0x80;
	return tmp;
    case 1:
	/* Returning 0xFF is necessary for Tie Break - otherwise its joystick
	   code won't work.  */
	return 0xFF;
    case 2:
	return ciaadra;
    case 3:
	return ciaadrb;
    case 4:
	return (ciaata - ciaata_passed) & 0xff;
    case 5:
	return (ciaata - ciaata_passed) >> 8;
    case 6:
	return (ciaatb - ciaatb_passed) & 0xff;
    case 7:
	return (ciaatb - ciaatb_passed) >> 8;
    case 8:
	if (ciaatlatch) {
	    ciaatlatch = 0;
	    return ciaatol & 0xff;
	} else
	    return ciaatod & 0xff;
    case 9:
	if (ciaatlatch)
	    return (ciaatol >> 8) & 0xff;
	else
	    return (ciaatod >> 8) & 0xff;
    case 10:
	ciaatlatch = 1;
	ciaatol = ciaatod; /* ??? only if not already latched? */
	return (ciaatol >> 16) & 0xff;
    case 12:
	if (ciaasdr == 1) ciaasdr_unread = 2;
	return ciaasdr;
    case 13:
	tmp = ciaaicr; ciaaicr = 0;
	RethinkICRA();
	return tmp;
    case 14:
	return ciaacra;
    case 15:
	return ciaacrb;
    }
    return 0;
}

static uae_u8 ReadCIAB (unsigned int addr)
{
    unsigned int tmp;

    compute_passed_time ();

    switch (addr & 0xf) {
    case 0:
	/* Returning some 1 bits is necessary for Tie Break - otherwise its joystick
	   code won't work.  */
	return ciabpra | 3;
    case 1:
	return ciabprb;
    case 2:
	return ciabdra;
    case 3:
	return ciabdrb;
    case 4:
	return (ciabta - ciabta_passed) & 0xff;
    case 5:
	return (ciabta - ciabta_passed) >> 8;
    case 6:
	return (ciabtb - ciabtb_passed) & 0xff;
    case 7:
	return (ciabtb - ciabtb_passed) >> 8;
    case 8:
	if (ciabtlatch) {
	    ciabtlatch = 0;
	    return ciabtol & 0xff;
	} else
	    return ciabtod & 0xff;
    case 9:
	if (ciabtlatch)
	    return (ciabtol >> 8) & 0xff;
	else
	    return (ciabtod >> 8) & 0xff;
    case 10:
	ciabtlatch = 1;
	ciabtol = ciabtod;
	return (ciabtol >> 16) & 0xff;
    case 12:
	return ciabsdr;
    case 13:
	tmp = ciabicr; ciabicr = 0; RethinkICRB();
	return tmp;
    case 14:
	return ciabcra;
    case 15:
	return ciabcrb;
    }
    return 0;
}

static void WriteCIAA (uae_u16 addr,uae_u8 val)
{
    int oldled, oldovl;
    switch (addr & 0xf) {
    case 0:
	oldovl = ciaapra & 1;
	oldled = ciaapra & 2;
	ciaapra = (ciaapra & ~0x3) | (val & 0x3);
	gui_ledstate &= ~1;
	gui_ledstate |= ((~ciaapra & 2) >> 1);
	gui_data.powerled = ((~ciaapra & 2) >> 1);


	if ((ciaapra & 1) != oldovl) {
	    int i = (allocated_chipmem>>16) > 32 ? allocated_chipmem >> 16 : 32;
	    
	    if (oldovl || ersatzkickfile) {
#ifdef DEBUG_MEMORY
		dbg("map_banks : chipmem_bank en WriteCIAA");
#endif
		map_banks (&chipmem_bank, 0, i, allocated_chipmem);
	    } else {
#ifdef DEBUG_MEMORY
		dbg("map_banks : kickmem_bank en WriteCIAA");
#endif
		/* Is it OK to do this for more than 2M of chip? */
		map_banks (&kickmem_bank, 0, i, 0x80000);
	    }
	}
	break;
    case 1:
	ciaaprb = val;
	ciaaicr |= 0x10;
	break;
    case 2:
	ciaadra = val; break;
    case 3:
	ciaadrb = val; break;
    case 4:
	CIA_update ();
	ciaala = (ciaala & 0xff00) | val;
	CIA_calctimers ();
	break;
    case 5:
	CIA_update ();
	ciaala = (ciaala & 0xff) | (val << 8);
	if ((ciaacra & 1) == 0)
	    ciaata = ciaala;
	if (ciaacra & 8) {
	    ciaata = ciaala;
	    ciaacra |= 1;
	}
	CIA_calctimers ();
	break;
    case 6:
	CIA_update ();
	ciaalb = (ciaalb & 0xff00) | val;
	CIA_calctimers ();
	break;
    case 7:
	CIA_update ();
	ciaalb = (ciaalb & 0xff) | (val << 8);
	if ((ciaacrb & 1) == 0)
	    ciaatb = ciaalb;
	if (ciaacrb & 8) {
	    ciaatb = ciaalb;
	    ciaacrb |= 1;
	}
	CIA_calctimers ();
	break;
    case 8:
	if (ciaacrb & 0x80) {
	    ciaaalarm = (ciaaalarm & ~0xff) | val;
	} else {
	    ciaatod = (ciaatod & ~0xff) | val;
	    ciaatodon = 1;
	}
	break;
    case 9:
	if (ciaacrb & 0x80) {
	    ciaaalarm = (ciaaalarm & ~0xff00) | (val << 8);
	} else {
	    ciaatod = (ciaatod & ~0xff00) | (val << 8);
	    ciaatodon = 0;
	}
	break;
    case 10:
	if (ciaacrb & 0x80) {
	    ciaaalarm = (ciaaalarm & ~0xff0000) | (val << 16);
	} else {
	    ciaatod = (ciaatod & ~0xff0000) | (val << 16);
	    ciaatodon = 0;
	}
	break;
    case 12:
	ciaasdr = val; break;
    case 13:
	setclr(&ciaaimask,val); break; /* ??? call RethinkICR() ? */
    case 14:
	CIA_update ();
	ciaacra = val;
	if (ciaacra & 0x10) {
	    ciaacra &= ~0x10;
	    ciaata = ciaala;
	}
	if (ciaacra & 0x40) {
	    kback = 1;
	}
	CIA_calctimers ();
	break;
    case 15:
	CIA_update ();
	ciaacrb = val;
	if (ciaacrb & 0x10) {
	    ciaacrb &= ~0x10;
	    ciaatb = ciaalb;
	}
	CIA_calctimers ();
	break;
    }
}

static void WriteCIAB (uae_u16 addr,uae_u8 val)
{
    int oldval;
    switch (addr & 0xf) {
    case 0:
	    ciabpra  = val;
	break;
    case 1:
	ciabprb = val; DISK_select(val); break;
    case 2:
	ciabdra = val; break;
    case 3:
	ciabdrb = val; break;
    case 4:
	CIA_update ();
	ciabla = (ciabla & 0xff00) | val;
	CIA_calctimers ();
	break;
    case 5:
	CIA_update ();
	ciabla = (ciabla & 0xff) | (val << 8);
	if ((ciabcra & 1) == 0)
	    ciabta = ciabla;
	if (ciabcra & 8) {
	    ciabta = ciabla;
	    ciabcra |= 1;
	}
	CIA_calctimers ();
	break;
    case 6:
	CIA_update ();
	ciablb = (ciablb & 0xff00) | val;
	CIA_calctimers ();
	break;
    case 7:
	CIA_update ();
	ciablb = (ciablb & 0xff) | (val << 8);
	if ((ciabcrb & 1) == 0)
	    ciabtb = ciablb;
	if (ciabcrb & 8) {
	    ciabtb = ciablb;
	    ciabcrb |= 1;
	}
	CIA_calctimers ();
	break;
    case 8:
	if (ciabcrb & 0x80) {
	    ciabalarm = (ciabalarm & ~0xff) | val;
	} else {
	    ciabtod = (ciabtod & ~0xff) | val;
	    ciabtodon = 1;
	}
	break;
    case 9:
	if (ciabcrb & 0x80) {
	    ciabalarm = (ciabalarm & ~0xff00) | (val << 8);
	} else {
	    ciabtod = (ciabtod & ~0xff00) | (val << 8);
	    ciabtodon = 0;
	}
	break;
    case 10:
	if (ciabcrb & 0x80) {
	    ciabalarm = (ciabalarm & ~0xff0000) | (val << 16);
	} else {
	    ciabtod = (ciabtod & ~0xff0000) | (val << 16);
	    ciabtodon = 0;
	}
	break;
    case 12:
	ciabsdr = val;
	break;
    case 13:
	setclr(&ciabimask,val);
	break;
    case 14:
	CIA_update ();
	ciabcra = val;
	if (ciabcra & 0x10) {
	    ciabcra &= ~0x10;
	    ciabta = ciabla;
	}
	CIA_calctimers ();
	break;
    case 15:
	CIA_update ();
	ciabcrb = val;
	if (ciabcrb & 0x10) {
	    ciabcrb &= ~0x10;
	    ciabtb = ciablb;
	}
	CIA_calctimers ();
	break;
    }
}

void CIA_reset (void)
{
    kback = 1;
    kbstate = 0;

    if (!savestate_state)
    {
    	ciaatlatch = ciabtlatch = 0;
 	ciaapra = 3; ciaaprb = ciaadra = ciaadrb = ciaasdr = 0;
    	ciabprb = ciabdra = ciabdrb = ciabsdr = 0;
    	ciaatod = ciabtod = 0; ciaatodon = ciabtodon = 0;
    	ciaaicr = ciabicr = ciaaimask = ciabimask = 0;
 	ciaacra = ciaacrb = ciabcra = ciabcrb = 0x4; /* outmode = toggle; */
 	ciaala = ciaalb = ciabla = ciablb = ciaata = ciaatb = ciabta = ciabtb = 0xFFFF;
 	ciabpra = 0x8C;
 	div10 = 0;
        ciaasdr_unread = 0;
    }

    CIA_calctimers ();
    if (! ersatzkickfile) {
	int i = allocated_chipmem > 0x200000 ? allocated_chipmem >> 16 : 32;
#ifdef DEBUG_MEMORY
	dbg("map_banks : kickmem_bank en CIA_reset");
#endif
	map_banks (&kickmem_bank, 0, i, 0x80000);
    }
    if (savestate_state)
    {
	/* Reset oldovl and oldled */
	uae_u8 v = ReadCIAA (0);
	WriteCIAA (0,3);
	WriteCIAA (0,0);
	WriteCIAA (0,v);
	/* select drives */
	DISK_select (ciabprb);
    }

}

/* CIA memory access */

static uae_u32 cia_lget (uaecptr) REGPARAM;
static uae_u32 cia_wget (uaecptr) REGPARAM;
static uae_u32 cia_bget (uaecptr) REGPARAM;
static void cia_lput (uaecptr, uae_u32) REGPARAM;
static void cia_wput (uaecptr, uae_u32) REGPARAM;
static void cia_bput (uaecptr, uae_u32) REGPARAM;

addrbank cia_bank = {
    cia_lget, cia_wget, cia_bget,
    cia_lput, cia_wput, cia_bput,
    default_xlate, default_check, NULL
};

static void cia_wait (void)
{
    if (!div10)
	return;
    do_cycles(DIV10 - div10 + CYCLE_UNIT);
    CIA_handler ();
}

uae_u32 REGPARAM2 cia_bget (uaecptr addr)
{
    int r = (addr & 0xf00) >> 8;
#ifdef USE_SPECIAL_MEM
    special_mem |= S_READ;
#endif
    cia_wait ();
    switch ((addr >> 12) & 3)
    {
    case 0:
	return (addr & 1) ? ReadCIAA (r) : ReadCIAB (r);
    case 1:
	return (addr & 1) ? 0xff : ReadCIAB (r);
    case 2:
	return (addr & 1) ? ReadCIAA (r) : 0xff;
    }
    return 0xff;
}

uae_u32 REGPARAM2 cia_wget (uaecptr addr)
{
    int r = (addr & 0xf00) >> 8;
#ifdef USE_SPECIAL_MEM
    special_mem |= S_READ;
#endif
    cia_wait ();
    switch ((addr >> 12) & 3)
    {
    case 0:
	return (ReadCIAB (r) << 8) | ReadCIAA (r);
    case 1:
	return (ReadCIAB (r) << 8) | 0xff;
    case 2:
	return (0xff << 8) | ReadCIAA (r);
    }
    return 0xffff;
}

uae_u32 REGPARAM2 cia_lget (uaecptr addr)
{
    uae_u32 v;
#ifdef USE_SPECIAL_MEM
    special_mem |= S_READ;
#endif
    v = cia_wget (addr) << 16;
    v |= cia_wget (addr + 2);
    return v;
}

void REGPARAM2 cia_bput (uaecptr addr, uae_u32 value)
{
#ifndef USE_FAME_CORE
    value&=0xFF;
#endif
    int r = (addr & 0xf00) >> 8;
#ifdef USE_SPECIAL_MEM
    special_mem |= S_WRITE;
#endif
    cia_wait ();
    if ((addr & 0x2000) == 0)
	WriteCIAB (r, value);
    if ((addr & 0x1000) == 0)
	WriteCIAA (r, value);
}

void REGPARAM2 cia_wput (uaecptr addr, uae_u32 value)
{
#ifndef USE_FAME_CORE
    value&=0xFFFF;
#endif
    value = value;
    int r = (addr & 0xf00) >> 8;
#ifdef USE_SPECIAL_MEM
    special_mem |= S_WRITE;
#endif
    cia_wait ();
    if ((addr & 0x2000) == 0)
	WriteCIAB (r, value >> 8);
    if ((addr & 0x1000) == 0)
	WriteCIAA (r, value & 0xff);
}

void REGPARAM2 cia_lput (uaecptr addr, uae_u32 value)
{
    value = value;
#ifdef USE_SPECIAL_MEM
    special_mem |= S_WRITE;
#endif
    cia_wput (addr, value >> 16);
    cia_wput (addr + 2, value & 0xffff);
}

/* battclock memory access */

static uae_u32 clock_lget (uaecptr) REGPARAM;
static uae_u32 clock_wget (uaecptr) REGPARAM;
static uae_u32 clock_bget (uaecptr) REGPARAM;
static void clock_lput (uaecptr, uae_u32) REGPARAM;
static void clock_wput (uaecptr, uae_u32) REGPARAM;
static void clock_bput (uaecptr, uae_u32) REGPARAM;

addrbank clock_bank = {
    clock_lget, clock_wget, clock_bget,
    clock_lput, clock_wput, clock_bput,
    default_xlate, default_check, NULL
};

uae_u32 REGPARAM2 clock_lget (uaecptr addr)
{
#ifdef USE_SPECIAL_MEM
    special_mem |= S_READ;
#endif
    return clock_bget (addr + 3);
}

uae_u32 REGPARAM2 clock_wget (uaecptr addr)
{
#ifdef USE_SPECIAL_MEM
    special_mem |= S_READ;
#endif
    return clock_bget (addr + 1);
}

uae_u32 REGPARAM2 clock_bget (uaecptr addr)
{
    time_t t = time(0);
    struct tm *ct;

    ct = localtime (&t);
#ifdef USE_SPECIAL_MEM
    special_mem |= S_READ;
#endif

    switch (addr & 0x3f) {
    case 0x03: return ct->tm_sec % 10;
    case 0x07: return ct->tm_sec / 10;
    case 0x0b: return ct->tm_min % 10;
    case 0x0f: return ct->tm_min / 10;
    case 0x13: return ct->tm_hour % 10;
    case 0x17: return ct->tm_hour / 10;
    case 0x1b: return ct->tm_mday % 10;
    case 0x1f: return ct->tm_mday / 10;
    case 0x23: return (ct->tm_mon+1) % 10;
    case 0x27: return (ct->tm_mon+1) / 10;
    case 0x2b: return ct->tm_year % 10;
    case 0x2f: return ct->tm_year / 10;

    case 0x33: return ct->tm_wday;  /*Hack by -=SR=- */
    case 0x37: return clock_control_d;
    case 0x3b: return clock_control_e;
    case 0x3f: return clock_control_f;
    }
    return 0;
}

void REGPARAM2 clock_lput (uaecptr addr, uae_u32 value)
{
#ifdef USE_SPECIAL_MEM
    special_mem |= S_WRITE;
#endif
    /* No way */
}

void REGPARAM2 clock_wput (uaecptr addr, uae_u32 value)
{
#ifdef USE_SPECIAL_MEM
    special_mem |= S_WRITE;
#endif
    /* No way */
}

void REGPARAM2 clock_bput (uaecptr addr, uae_u32 value)
{
#ifdef USE_SPECIAL_MEM
    special_mem |= S_WRITE;
#endif
    switch (addr & 0x3f) {
    case 0x37: clock_control_d = value; break;
    case 0x3b: clock_control_e = value; break;
    case 0x3f: clock_control_f = value; break;
    }
}


/* CIA-A and CIA-B save/restore code */

uae_u8 *restore_cia (int num, uae_u8 *src)
{
    uae_u8 b;
    uae_u16 w;
    uae_u32 l;

    /* CIA registers */
    b = restore_u8 ();					/* 0 PRA */
    if (num) ciabpra = b; else ciaapra = b;
    b = restore_u8 ();					/* 1 PRB */
    if (num) ciabprb = b; else ciaaprb = b;
    b = restore_u8 ();					/* 2 DDRA */
    if (num) ciabdra = b; else ciaadra = b;
    b = restore_u8 ();					/* 3 DDRB */
    if (num) ciabdrb = b; else ciaadrb = b;
    w = restore_u16 ();					/* 4 TA */
    if (num) ciabta = w; else ciaata = w;
    w = restore_u16 ();					/* 6 TB */
    if (num) ciabtb = w; else ciaatb = w;
    l = restore_u8 ();					/* 8/9/A TOD */
    l |= restore_u8 () << 8;
    l |= restore_u8 () << 16;
    if (num) ciabtod = l; else ciaatod = l;
    restore_u8 ();						/* B unused */
    b = restore_u8 ();					/* C SDR */
    if (num) ciabsdr = b; else ciaasdr = b;
    b = restore_u8 ();					/* D ICR INFORMATION (not mask!) */
    if (num) ciabicr = b; else ciaaicr = b;
    b = restore_u8 ();					/* E CRA */
    if (num) ciabcra = b; else ciaacra = b;
    b = restore_u8 ();					/* F CRB */
    if (num) ciabcrb = b; else ciaacrb = b;

/* CIA internal data */

    b = restore_u8 ();					/* ICR MASK */
    if (num) ciabimask = b; else ciaaimask = b;
    w = restore_u8 ();					/* timer A latch */
    w |= restore_u8 () << 8;
    if (num) ciabla = w; else ciaala = w;
    w = restore_u8 ();					/* timer B latch */
    w |= restore_u8 () << 8;
    if (num) ciablb = w; else ciaalb = w;
    w = restore_u8 ();					/* TOD latched value */
    w |= restore_u8 () << 8;
    w |= restore_u8 () << 16;
    if (num) ciabtol = w; else ciaatol = w;
    l = restore_u8 ();					/* alarm */
    l |= restore_u8 () << 8;
    l |= restore_u8 () << 16;
    if (num) ciabalarm = l; else ciaaalarm = l;
    b = restore_u8 ();
    if (num) ciabtlatch = b & 1; else ciaatlatch = b & 1;	/* is TOD latched? */
    if (num) ciabtodon = b & 2; else ciaatodon = b & 2;		/* is TOD stopped? */
    if (num) {
	div10 = CYCLE_UNIT * restore_u8 ();
    }
    return src;
}

uae_u8 *save_cia (int num, int *len)
{
    uae_u8 *dstbak,*dst, b;
    uae_u16 t;

    dstbak = dst = (uae_u8 *)malloc (16 + 12 + 1);

    compute_passed_time ();

    /* CIA registers */

    b = num ? ciabpra : ciaapra;				/* 0 PRA */
    save_u8 (b);
    b = num ? ciabprb : ciaaprb;				/* 1 PRB */
    save_u8 (b);
    b = num ? ciabdra : ciaadra;				/* 2 DDRA */
    save_u8 (b); 
    b = num ? ciabdrb : ciaadrb;				/* 3 DDRB */
    save_u8 (b);
    t = (num ? ciabta - ciabta_passed : ciaata - ciaata_passed);/* 4 TA */
    save_u16 (t);
    t = (num ? ciabtb - ciabtb_passed : ciaatb - ciaatb_passed);/* 8 TB */
    save_u16 (t);
    b = (num ? ciabtod : ciaatod);			/* 8 TODL */
    save_u8 (b);
    b = (num ? ciabtod >> 8 : ciaatod >> 8);		/* 9 TODM */
    save_u8 (b);
    b = (num ? ciabtod >> 16 : ciaatod >> 16);		/* A TODH */
    save_u8 (b);
    save_u8 (0);						/* B unused */
    b = num ? ciabsdr : ciaasdr;				/* C SDR */
    save_u8 (b);
    b = num ? ciabicr : ciaaicr;				/* D ICR INFORMATION (not mask!) */
    save_u8 (b);
    b = num ? ciabcra : ciaacra;				/* E CRA */
    save_u8 (b);
    b = num ? ciabcrb : ciaacrb;				/* F CRB */
    save_u8 (b);

    /* CIA internal data */

    save_u8 (num ? ciabimask : ciaaimask);			/* ICR */
    b = (num ? ciabla : ciaala);			/* timer A latch LO */
    save_u8 (b);
    b = (num ? ciabla >> 8 : ciaala >> 8);		/* timer A latch HI */
    save_u8 (b);
    b = (num ? ciablb : ciaalb);			/* timer B latch LO */
    save_u8 (b);
    b = (num ? ciablb >> 8 : ciaalb >> 8);		/* timer B latch HI */
    save_u8 (b);
    b = (num ? ciabtol : ciaatol);			/* latched TOD LO */
    save_u8 (b);
    b = (num ? ciabtol >> 8 : ciaatol >> 8);		/* latched TOD MED */
    save_u8 (b);
    b = (num ? ciabtol >> 16 : ciaatol >> 16);		/* latched TOD HI */
    save_u8 (b);
    b = (num ? ciabalarm : ciaaalarm);			/* alarm LO */
    save_u8 (b);
    b = (num ? ciabalarm >> 8 : ciaaalarm >>8 );	/* alarm MED */
    save_u8 (b);
    b = (num ? ciabalarm >> 16 : ciaaalarm >> 16);	/* alarm HI */
    save_u8 (b);
    b = 0;
    if (num)
	b |= ciabtlatch ? 1 : 0;
    else
	b |= ciaatlatch ? 1 : 0; /* is TOD latched? */
    if (num)
	b |= ciabtodon ? 2 : 0;
    else
	b |= ciaatodon ? 2 : 0;   /* TOD stopped? */
    save_u8 (b);
    if (num) {
	/* Save extra state with CIAB.  */
	save_u8 (div10 / CYCLE_UNIT);
    }
    *len = dst - dstbak;
    return dstbak;
}
