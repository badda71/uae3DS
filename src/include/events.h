 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Events
  * These are best for low-frequency events. Having too many of them,
  * or using them for events that occur too frequently, can cause massive
  * slowdown.
  *
  * Copyright 1995-1998 Bernd Schmidt
  */

#include "rpt.h"

extern frame_time_t vsynctime, vsyncmintime;
extern void reset_frame_rate_hack (void);
extern int rpt_available;

extern void compute_vsynctime (void);

extern unsigned long currcycle, nextevent;
extern unsigned long sample_evtime;
typedef void (*evfunc)(void);

struct ev
{
    int active;
    unsigned long int evtime, oldcycles;
    evfunc handler;
};

enum {
    ev_hsync, ev_copper, ev_audio, ev_cia, ev_blitter, ev_disk,
    ev_max
};

extern struct ev eventtab[ev_max];

static __inline__ void events_schedule (void)
{
    int i;

    unsigned long int mintime = ~0L;
    for (i = 0; i < ev_max; i++) {
	if (eventtab[i].active) {
	    unsigned long int eventtime = eventtab[i].evtime - currcycle;
	    if (eventtime < mintime)
		mintime = eventtime;
	}
    }
    nextevent = currcycle + mintime;
}

static __inline__ void do_cycles_slow (unsigned long cycles_to_add)
{
#ifdef DEBUG_CYCLES
    dbgf("do_cycles(%i)\n",cycles_to_add);
#endif
    while ((nextevent - currcycle) <= cycles_to_add) {
        int i;
        cycles_to_add -= (nextevent - currcycle);
        currcycle = nextevent;

        for (i = 0; i < ev_max; i++) {
	    if (eventtab[i].active && eventtab[i].evtime == currcycle) {
#ifdef DEBUG_CYCLES
		    dbgf("EVENTO %i\n",i);
#endif
		(*eventtab[i].handler)();
#ifdef DEBUG_CYCLES
		    dbgf("!EVENTO %i\n",i);
#endif
	    }
	}
        events_schedule();
    }
    currcycle += cycles_to_add;
#ifdef DEBUG_CYCLES
    dbg("!do_cycles");
#endif
}

/* This is a special-case function.  Normally, all events should lie in the
   future; they should only ever be active at the current cycle during
   do_cycles.  However, a snapshot is saved during do_cycles, and so when
   restoring it, we may have other events pending.  */
static __inline__ void handle_active_events (void)
{
    int i;
    for (i = 0; i < ev_max; i++) {
	if (eventtab[i].active && eventtab[i].evtime == currcycle) {
	    (*eventtab[i].handler)();
	}
    }
}

static __inline__ unsigned long get_cycles (void)
{
    return currcycle;
}

extern void init_eventtab (void);


#define do_cycles do_cycles_slow


