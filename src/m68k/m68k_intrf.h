

#ifdef USE_FAME_CORE

#if defined(DREAMCAST) || defined(USE_FAME_CORE_C)
#define M68KCONTEXT m68kcontext
#else
#define M68KCONTEXT _m68kcontext
#endif

extern int uae4all_go_interrupt;

#include "m68k/fame/fame.h"
#include "memory.h"

void init_memmaps(addrbank* banco);
void map_zone(unsigned addr, addrbank* banco, unsigned realstart);
void m68k_go (int may_quit);
void init_m68k (void);

extern M68K_CONTEXT M68KCONTEXT;
extern unsigned mispcflags;

#define flush_icache(X) do {} while (0)

#define _68k_dreg(num) (M68KCONTEXT.dreg[(num)])
#define _68k_areg(num) (M68KCONTEXT.areg[(num)])
#define _68k_sreg 	M68KCONTEXT.sr
#define _68k_ispreg 	M68KCONTEXT.areg[7]
#define _68k_mspreg 	M68KCONTEXT.asp
#define _68k_uspreg 	M68KCONTEXT.asp
#define _68k_intmask   ((M68KCONTEXT.sr >> 8) & 7)
#define _68k_incpc(o)  (M68KCONTEXT.pc += (o))
#define _68k_spcflags	mispcflags

static __inline__ unsigned _68k_getpc(void)
{
//	return M68KCONTEXT.pc;
	return m68k_get_pc();
}
static __inline__ void _68k_setpc(unsigned mipc)
{
	M68KCONTEXT.pc=mipc;
	m68k_set_register(M68K_REG_PC, mipc);
}

static __inline__ void set_special (unsigned x)
{
    _68k_spcflags |= x;
}

static __inline__ void unset_special (uae_u32 x)
{
    _68k_spcflags &= ~x;
}

static __inline__ void fill_prefetch_0 (void)
{
}

static __inline__ void dump_counts (void)
{
}

#else

#define _68k_getpc	m68k_getpc
#define _68k_setpc	m68k_setpc
#define _68k_areg	m68k_areg
#define _68k_dreg	m68k_dreg
#define _68k_incp	m68k_incp
#define _68k_sreg 	uae_regs.s
#define _68k_ispreg 	uae_regs.isp
#define _68k_mspreg 	uae_regs.msp
#define _68k_uspreg 	uae_regs.usp
#define _68k_intmask	uae_regs.intmask
#define _68k_incpc(o)   m68k_incpc(o)
#define _68k_spcflags	uae_regs.spcflags

#include "custom.h"
#include "m68k/uae/newcpu.h"

#endif
