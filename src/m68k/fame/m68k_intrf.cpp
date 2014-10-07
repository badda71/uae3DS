#ifdef DREAMCAST
#include<kos.h>
#endif

#define NO_SHORT_EVENTS
#define PROTECT_INFINITE

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "sysconfig.h"
#include "sysdeps.h"
#include "debug_uae4all.h"
#include "config.h"
#include "uae.h"
#include "options.h"
#include "memory.h"
#include "custom.h"
#include "autoconf.h"
#include "ersatz.h"
#include "debug.h"
#include "compiler.h"
#include "gui.h"
#include "savestate.h"
#include "blitter.h"
#include "events.h"

#include "m68k/debug_m68k.h"

#ifdef FAME_INTERRUPTS_PATCH
int uae4all_go_interrupt=0;
#endif

#ifdef DEBUG_UAE4ALL
#if !defined(START_DEBUG) || START_DEBUG==0 
int DEBUG_AHORA=1;
#else
int DEBUG_AHORA=0;
#endif
#ifdef DEBUG_FILE
FILE *DEBUG_STR_FILE=NULL;
#endif
#endif

#ifdef DEBUG_UAE4ALL
void carga(void)
{
	unsigned long ciclo;
	unsigned pc,sr,a0,a1,a2,a3,a4,a5,a6,a7,d0,d1,d2,d3,d4,d5,d6,d7;
	FILE *f=fopen("/tmp/uae4all_guarda","rb");
	if (f)
	{
		fread((void *)&pc,sizeof(unsigned),1,f);
		fread((void *)&sr,sizeof(unsigned),1,f);
		fread((void *)&a0,sizeof(unsigned),1,f);
		fread((void *)&a1,sizeof(unsigned),1,f);
		fread((void *)&a2,sizeof(unsigned),1,f);
		fread((void *)&a3,sizeof(unsigned),1,f);
		fread((void *)&a4,sizeof(unsigned),1,f);
		fread((void *)&a5,sizeof(unsigned),1,f);
		fread((void *)&a6,sizeof(unsigned),1,f);
		fread((void *)&a7,sizeof(unsigned),1,f);
		fread((void *)&d0,sizeof(unsigned),1,f);
		fread((void *)&d1,sizeof(unsigned),1,f);
		fread((void *)&d2,sizeof(unsigned),1,f);
		fread((void *)&d3,sizeof(unsigned),1,f);
		fread((void *)&d4,sizeof(unsigned),1,f);
		fread((void *)&d5,sizeof(unsigned),1,f);
		fread((void *)&d6,sizeof(unsigned),1,f);
		fread((void *)&d7,sizeof(unsigned),1,f);
		fread((void *)chipmemory,1,allocated_chipmem,f);
		fclose(f);
		{
			unsigned char *p=(unsigned char *)chipmemory;
			unsigned i;
			for (i=0;i<allocated_chipmem;i+=2)
			{
				unsigned char t=p[i];
				p[i]=p[i+1];
				p[i+1]=t;
			}
		}
		_68k_areg(0)=a0;
		_68k_areg(1)=a1;
		_68k_areg(2)=a2;
		_68k_areg(3)=a3;
		_68k_areg(4)=a4;
		_68k_areg(5)=a5;
		_68k_areg(6)=a6;
		_68k_areg(7)=a7;
		_68k_dreg(0)=d0;
		_68k_dreg(1)=d1;
		_68k_dreg(2)=d2;
		_68k_dreg(3)=d3;
		_68k_dreg(4)=d4;
		_68k_dreg(5)=d5;
		_68k_dreg(6)=d6;
		_68k_dreg(7)=d7;
		_68k_sreg=sr;
		_68k_setpc(pc);
	}
}
#endif


static unsigned short mimemoriadummy[65536/2];

void clear_fame_mem_dummy(void)
{
	memset((void *)&mimemoriadummy[0],0,65536);
}

int m68k_speed=5;
static double cycles_factor=1.0;
static unsigned timeslice_shift=6;
int next_positions[512];
int *next_vpos=&next_positions[0];

M68K_CONTEXT micontexto;
M68K_PROGRAM miprograma[257];
M68K_DATA midato_read_8[257];
M68K_DATA midato_read_16[257];
M68K_DATA midato_write_8[257];
M68K_DATA midato_write_16[257];
static unsigned micontexto_fpa[256];

unsigned mispcflags=0;

#if !defined(DREAMCAST) || defined(DEBUG_UAE4ALL)
int in_m68k_go = 0;
#endif

static int do_specialties (int cycles)
{
    if (mispcflags & SPCFLAG_COPPER)
    {
#if defined(DEBUG_M68K) || defined(DEBUG_CYCLES)
	dbg("do_specialties -> do_copper");    
#endif
        do_copper ();
    }

    /*n_spcinsns++;*/
    while ((mispcflags & SPCFLAG_BLTNASTY) && cycles > 0) {
        int c = blitnasty();
        if (!c) {
            cycles -= 2 * CYCLE_UNIT;
            if (cycles < CYCLE_UNIT)
                cycles = 0;
            c = 1;
        }
#if defined(DEBUG_M68K) || defined(DEBUG_CYCLES)
	dbgf("do_specialties -> do_cycles BLTNASTY %i\n",c);
#endif
        do_cycles(c * CYCLE_UNIT);
        if (mispcflags & SPCFLAG_COPPER)
	{
#if defined(DEBUG_M68K) || defined(DEBUG_CYCLES)
	    dbg("do_specialties -> do_copper BLTNASTY");
#endif
            do_copper ();
	}
    }

#ifdef DEBUG_M68K
    while (M68KCONTEXT.execinfo & 0x0080) {
	if (mispcflags & SPCFLAG_BRK)
		break;
	else
		if (M68KCONTEXT.execinfo & 0x0080)
		{
			int intr = intlev ();
			if (intr != -1 && intr > _68k_intmask)
			{
				M68KCONTEXT.execinfo &= 0xFF7F;
				break;
			}
		}
	dbg("CPU STOPPED !");
        do_cycles(4 * CYCLE_UNIT);
        if (mispcflags & SPCFLAG_COPPER)
	{
	    dbg("do_specialties -> do_copper STOPPED");
            do_copper ();
	}
    }
    unset_special (SPCFLAG_STOP);
#endif

#if !defined(FAME_INTERRUPTS_SECURE_PATCH) && defined(FAME_INTERRUPTS_PATCH)
    if (uae4all_go_interrupt)
    {
	M68KCONTEXT.interrupts[0]=(M68KCONTEXT.interrupts[0]&1)|uae4all_go_interrupt;
	M68KCONTEXT.execinfo&=0xFF67;
	uae4all_go_interrupt=0;
    }
#endif

#ifdef SPECIAL_DEBUG_INTERRUPTS
    if ((mispcflags & SPCFLAG_DOINT)&&(!(mispcflags & SPCFLAG_INT))) {
	int intr = intlev ();
#ifdef DEBUG_INTERRUPTS_EXTRA
	dbgf("DOINT : intr = %i, intmask=%i\n", intr, _68k_intmask);
#endif
	unset_special (SPCFLAG_DOINT);
	if (intr != -1 && intr > _68k_intmask) {
		M68KCONTEXT.execinfo&=0xFF6F;
		m68k_raise_irq(intr,M68K_AUTOVECTORED_IRQ);
//		m68k_emulate(0);
	}
    }
    if (mispcflags & SPCFLAG_INT) {
#ifdef DEBUG_INTERRUPTS_EXTRA
	dbg("ESTAMOS EN INT -> PASAMOS A DOINT");
#endif
	unset_special (SPCFLAG_INT);
	set_special (SPCFLAG_DOINT);
    }
#endif

    if (mispcflags & SPCFLAG_BRK) {
#ifdef DEBUG_SAVESTATE
	printf("BRK state=%X, flags=%X, PC=%X\n",savestate_state,_68k_spcflags,_68k_getpc());fflush(stdout);
#endif
        unset_special (SPCFLAG_BRK);
        return 1;
    }
    return 0;
}

static void uae4all_reset(void)
{
    int i;
    for(i=1;i<8;i++)
#if defined(DEBUG_INTERRUPTS)
    	micontexto.interrupts[i]=0xFF;
#else
    	micontexto.interrupts[i]=0x18+i;
#endif
    micontexto.interrupts[0]=0;
    m68k_set_context(&micontexto);
    m68k_reset();
    for(i=1;i<8;i++)
#if defined(DEBUG_INTERRUPTS)
    	M68KCONTEXT.interrupts[i]=0xFF;
#else
    	M68KCONTEXT.interrupts[i]=0x18+i;
#endif
    M68KCONTEXT.interrupts[0]=0;
    mispcflags=0;
    _68k_areg(7) = get_long (0x00f80000);
    _68k_setpc(get_long (0x00f80004));
    _68k_sreg = 0x2700;
    mispcflags=0;
#ifdef DEBUG_FRAMERATE
    uae4all_update_time();
#endif
}

static void m68k_run (void)
{
	unsigned cycles, cycles_actual=M68KCONTEXT.cycles_counter;
#ifdef DEBUG_M68K
	dbg("m68k_run");
#endif
	for (;;) {
#ifdef DEBUG_M68K
		dbg_cycle(m68k_fetch(m68k_get_pc(),0));
		m68k_emulate(1);

#else
		uae4all_prof_start(0);
#ifdef DEBUG_TIMESLICE
		unsigned ts=(nextevent - currcycle)>>timeslice_shift;
#endif
#ifdef DEBUG_CYCLES
		dbgf("m68k_emulate(%i)\n",(nextevent - currcycle)>>timeslice_shift);
#endif
#if 0
// defined(FAME_INTERRUPTS_SECURE_PATCH) && defined(FAME_INTERRUPTS_PATCH)
		if (uae4all_go_interrupt)
			m68k_emulate(FAME_INTERRUPTS_PATCH);
		else
#endif
			m68k_emulate((nextevent - currcycle)>>timeslice_shift);
#ifdef DEBUG_CYCLES
		dbg("!m68k_emulate");
#endif
		uae4all_prof_end(0);
#endif
#ifdef FAME_INTERRUPTS_PATCH
		if (uae4all_go_interrupt)
		{
//			M68KCONTEXT.interrupts[0]=(M68KCONTEXT.interrupts[0]&1)|uae4all_go_interrupt;
			M68KCONTEXT.interrupts[0]=uae4all_go_interrupt;
			M68KCONTEXT.execinfo&=0xFF67;
			uae4all_go_interrupt=0;
		}
#endif
#ifdef DEBUG_M68K

		if (M68KCONTEXT.execinfo & 0x0080)
			mispcflags|=SPCFLAG_STOP;
#endif
                uae4all_prof_start(1);

#ifdef DEBUG_M68K
		cycles=3413;
#else
		cycles=((unsigned)(((double)(M68KCONTEXT.cycles_counter-cycles_actual))*cycles_factor))<<8;

#ifdef DEBUG_INTERRUPTS
		dbgf("cycles=%i (%i) -> PC=%.8X\n",cycles>>8,(nextevent - currcycle)>>timeslice_shift,m68k_get_pc());
#endif

#ifdef DEBUG_TIMESLICE
		unsigned real_ts=(M68KCONTEXT.cycles_counter-cycles_actual);
		int diff_ts=((int)ts)-((int)real_ts);
		static int media_diff_ts=0, media_ts=0;
		if (media_ts)
		{
			media_diff_ts=(media_diff_ts+diff_ts)/2;
			media_ts=(media_ts+((int)real_ts))/2;
		}
		else
		{
			media_diff_ts=diff_ts;
			media_ts=real_ts;
		}
		static unsigned count_ts=0;
		if (!(count_ts&1023))
			printf("MEDIA TIMESLICE=%i, DIFF MEDIA=%i\n",media_ts,media_diff_ts);
		count_ts++;
#endif

#ifdef NO_SHORT_EVENTS
#ifdef PROTECT_INFINITE
		unsigned cuentalo=0;
#endif
		do{
#endif
#endif
			do_cycles(cycles);
			if (mispcflags)
				if (do_specialties (cycles))
					return;
#ifndef DEBUG_M68K
#ifdef NO_SHORT_EVENTS
			cycles=2048;
#ifdef PROTECT_INFINITE
			cuentalo++;
			if (cuentalo>1024)
			{
				quit_program=2;
				return;
			}
#endif
		}while((nextevent - currcycle)<=2048);
#endif
		cycles_actual=M68KCONTEXT.cycles_counter;
#endif
                uae4all_prof_end(1);
	}
}


void m68k_go (int may_quit)
{
    gui_purge_events();
#if !defined(DREAMCAST) || defined(DEBUG_UAE4ALL)
    if (in_m68k_go || !may_quit) {
#ifdef DEBUG_UAE4ALL
        puts("Bug! m68k_go is not reentrant.\n");
#endif
        return;
    }

    in_m68k_go++;
#endif
    quit_program = 2;
    for (;;) {
#ifdef DEBUG_SAVESTATE
	printf("m68k_go state=%X, flags=%X, PC=%X\n",savestate_state,_68k_spcflags,_68k_getpc());fflush(stdout);
#endif
        if (quit_program > 0) {
            if (quit_program == 1)
                break;
            quit_program = 0;
	    if (savestate_state == STATE_RESTORE)
	    {
#ifdef DEBUG_SAVESTATE
		    puts("Restaurando");fflush(stdout);
#endif
		    restore_state (savestate_filename);
		    mispcflags = 0;
//		    _m68k_setpc(M68KCONTEXT.pc);
	    }
            reset_all_systems ();
#ifdef DEBUG_SAVESTATE
	    printf("-->reset_all_systems state=%X, flags=%X, PC=%X\n",savestate_state,_68k_spcflags,_68k_getpc());fflush(stdout);
#endif
            customreset ();
#ifdef DEBUG_SAVESTATE
	    printf("-->customreset state=%X, flags=%X, PC=%X\n",savestate_state,_68k_spcflags,_68k_getpc());fflush(stdout);
#endif
            /* We may have been restoring state, but we're done now.  */
            handle_active_events ();
            if (mispcflags)
                do_specialties (0);
        }

	if (!savestate_state)
		uae4all_reset ();
	savestate_restore_finish ();
        m68k_run();
    }
#if !defined(DREAMCAST) || defined(DEBUG_UAE4ALL)
    in_m68k_go--;
#endif
#ifdef DEBUG_UAE4ALL
    puts("BYE?");
#endif
}

void init_m68k (void)
{
	m68k_init();
}

static void m68k_exception(unsigned n)
{
	unsigned pc=m68k_get_pc();
	unsigned sr=m68k_get_register(M68K_REG_SR);
	unsigned sp=m68k_get_register(M68K_REG_A7);

	if (!(sr&0x2000))
	{
		unsigned asp=m68k_get_register(M68K_REG_ASP);
		m68k_set_register(M68K_REG_SR,(sr&0x7FF)|0x2000);
		m68k_set_register(M68K_REG_ASP,sp);
		m68k_set_register(M68K_REG_A7,asp-6);
		sp=asp;
	}
	else
		m68k_set_register(M68K_REG_A7,sp-6);

	put_long(sp-4,pc);
	put_word(sp-6,sr);

	m68k_set_register(M68K_REG_PC,m68k_fetch((n*4)+2,0)+(m68k_fetch(n*4,0)<<16));

	m68k_release_timeslice();
	M68KCONTEXT.execinfo&=0x65;
}


void uae_chk_handler(unsigned vector)
{
	unsigned opcode=m68k_fetch(m68k_get_pc(),0);
	unsigned pc=m68k_get_pc();

#ifdef DEBUG_UAE4ALL
	dbgf("INVALID OPCODE 0x%X at PC=0x%X -> ",opcode,pc);
#endif
	if (cloanto_rom && (opcode & 0xF100) == 0x7100) {
#ifdef DEBUG_UAE4ALL
		dbg("cloanto");
#endif
		_68k_dreg((opcode >> 9) & 7) = (uae_s8)(opcode & 0xFF);
		m68k_set_register(M68K_REG_PC,pc+2);
		return;
	}

	if (opcode == 0x4E7B && get_long (0x10) == 0 && (pc & 0xF80000) == 0xF80000) {
#ifdef DEBUG_UAE4ALL
		dbg("68020");
#endif
		write_log ("Your Kickstart requires a 68020 CPU. Giving up.\n");
		set_special (SPCFLAG_BRK);
		quit_program = 1;
		return;
	}

	if (opcode == 0xFF0D) {
		if ((pc & 0xF80000) == 0xF80000) {
#ifdef DEBUG_UAE4ALL
			dbg("dummy");
#endif
			// This is from the dummy Kickstart replacement
			uae_u16 arg = m68k_fetch(pc+2,0);
			m68k_set_register(M68K_REG_PC,pc+4);
			ersatz_perform (arg);
			return;
		}
		else
		if ((pc & 0xFFFF0000) == RTAREA_BASE) {
#ifdef DEBUG_UAE4ALL
			dbg("stop");
#endif
			// User-mode STOP replacement
			M68KCONTEXT.execinfo|=0x0080;
#ifdef DEBUG_M68K			
			mispcflags|=SPCFLAG_STOP;
#endif
			m68k_set_register(M68K_REG_PC,pc+2);
			return;
		}
	}

	if ((opcode & 0xF000) == 0xA000 && (pc & 0xFFFF0000) == RTAREA_BASE) {
#ifdef DEBUG_UAE4ALL
		dbg("call");
#endif
		// Calltrap.
#ifdef USE_AUTOCONFIG
		m68k_set_register(M68K_REG_PC,pc+2);
		call_calltrap (opcode & 0xFFF);
#endif
		return;
	}

	if ((opcode & 0xF000) == 0xF000) {
#ifdef DEBUG_UAE4ALL
		dbg("exp8");
#endif
		// Exception 0xB
		m68k_exception(0xB);
		return;
	}

	if ((opcode & 0xF000) == 0xA000) {
		if ((pc & 0xFFFF0000) == RTAREA_BASE) {
#ifdef DEBUG_UAE4ALL
			dbgf("call +");
#endif
			// Calltrap.
#ifdef USE_AUTOCONFIG
			call_calltrap (opcode & 0xFFF);
#endif
		}
#ifdef DEBUG_UAE4ALL
		dbg("expA");
#endif
		m68k_exception(0xA);
		return;
	}

#ifdef DEBUG_UAE4ALL
	dbg("Real invalid");
#endif
	write_log ("Illegal instruction: %04x at %08lx\n", opcode, pc);
	m68k_exception(0x4);
}


void init_memmaps(addrbank* banco)
{
	unsigned i;

	memset(&micontexto,0,sizeof(M68K_CONTEXT));

	memset(&micontexto_fpa,0,sizeof(unsigned)*256);

	micontexto_fpa[0x04]=(unsigned)&uae_chk_handler;
//	micontexto_fpa[0x10]=(unsigned)&uae_chk_handler; // FAME BUG !!!
	micontexto.icust_handler = (unsigned int*)&micontexto_fpa;

	micontexto.fetch=(M68K_PROGRAM *)&miprograma;
	micontexto.read_byte=(M68K_DATA *)&midato_read_8;
	micontexto.read_word=(M68K_DATA *)&midato_read_16;
	micontexto.write_byte=(M68K_DATA *)&midato_write_8;
	micontexto.write_word=(M68K_DATA *)&midato_write_16;

	micontexto.sv_fetch=(M68K_PROGRAM *)&miprograma;
	micontexto.sv_read_byte=(M68K_DATA *)&midato_read_8;
	micontexto.sv_read_word=(M68K_DATA *)&midato_read_16;
	micontexto.sv_write_byte=(M68K_DATA *)&midato_write_8;
	micontexto.sv_write_word=(M68K_DATA *)&midato_write_16;
	
	micontexto.user_fetch=(M68K_PROGRAM *)&miprograma;
	micontexto.user_read_byte=(M68K_DATA *)&midato_read_8;
	micontexto.user_read_word=(M68K_DATA *)&midato_read_16;
	micontexto.user_write_byte=(M68K_DATA *)&midato_write_8;
	micontexto.user_write_word=(M68K_DATA *)&midato_write_16;

	micontexto.reset_handler=NULL;
	micontexto.iack_handler=NULL;
	
	for(i=0;i<256;i++)
	{
		unsigned offset=(unsigned)banco->baseaddr;
		unsigned low_addr=(i<<16);
		unsigned high_addr=((i+1)<<16)-1;
		void *data=NULL;
		void *mem_handler_r8=NULL;
		void *mem_handler_r16=NULL;
		void *mem_handler_w8=NULL;
		void *mem_handler_w16=NULL;

		if (offset)
			data=(void *)(offset-low_addr);
		else
		{
			mem_handler_r8=(void *)banco->bget;
			mem_handler_r16=(void *)banco->wget;
			mem_handler_w8=(void *)banco->bput;
			mem_handler_w16=(void *)banco->wput;
		}

		miprograma[i].low_addr=low_addr;
		miprograma[i].high_addr=high_addr;
		miprograma[i].offset=((unsigned)&mimemoriadummy)-low_addr;
		midato_read_8[i].low_addr=low_addr;
		midato_read_8[i].high_addr=high_addr;
		midato_read_8[i].mem_handler=mem_handler_r8;
		midato_read_8[i].data=data;
		midato_read_16[i].low_addr=low_addr;
		midato_read_16[i].high_addr=high_addr;
		midato_read_16[i].mem_handler=mem_handler_r16;
		midato_read_16[i].data=data;
		midato_write_8[i].low_addr=low_addr;
		midato_write_8[i].high_addr=high_addr;
		midato_write_8[i].mem_handler=mem_handler_w8;
		midato_write_8[i].data=data;
		midato_write_16[i].low_addr=low_addr;
		midato_write_16[i].high_addr=high_addr;
		midato_write_16[i].mem_handler=mem_handler_w16;
		midato_write_16[i].data=data;
	}
	miprograma[256].low_addr=(unsigned)-1;
	miprograma[256].high_addr=(unsigned)-1;
	miprograma[256].offset=(unsigned)NULL;
	midato_read_8[256].low_addr=(unsigned)-1;
	midato_read_8[256].high_addr=(unsigned)-1;
	midato_read_8[256].mem_handler=NULL;
	midato_read_8[256].data=NULL;
	midato_read_16[256].low_addr=(unsigned)-1;
	midato_read_16[256].high_addr=(unsigned)-1;
	midato_read_16[256].mem_handler=NULL;
	midato_read_16[256].data=NULL;
	midato_write_8[256].low_addr=(unsigned)-1;
	midato_write_8[256].high_addr=(unsigned)-1;
	midato_write_8[256].mem_handler=NULL;
	midato_write_8[256].data=NULL;
	midato_write_16[256].low_addr=(unsigned)-1;
	midato_write_16[256].high_addr=(unsigned)-1;
	midato_write_16[256].mem_handler=NULL;
	midato_write_16[256].data=NULL;

	m68k_set_context(&micontexto);
}

void map_zone(unsigned addr, addrbank* banco, unsigned realstart)
{
	unsigned offset=(unsigned)banco->baseaddr;
	if (addr>255)
		return;

	unsigned low_addr=(addr<<16);
	unsigned high_addr=((addr+1)<<16)-1;
	m68k_get_context(&micontexto);

#ifdef DEBUG_MAPPINGS
	dbgf("map_zone: 0x%.8X (0x%.8X 0x%.8X)",addr<<16,low_addr,high_addr);
#endif

	if (offset)
	{
#ifdef DEBUG_MAPPINGS
		dbg(" offset");
#endif
		offset+=((addr-realstart)<<16);
		miprograma[addr].low_addr=low_addr;
		miprograma[addr].high_addr=high_addr;
		miprograma[addr].offset=offset-low_addr;
		midato_read_8[addr].low_addr=low_addr;
		midato_read_8[addr].high_addr=high_addr;
		midato_read_8[addr].mem_handler=NULL;
		midato_read_8[addr].data=(void *)(offset-low_addr);
		midato_read_16[addr].low_addr=low_addr;
		midato_read_16[addr].high_addr=high_addr;
		midato_read_16[addr].mem_handler=NULL;
		midato_read_16[addr].data=(void *)(offset-low_addr);
		midato_write_8[addr].low_addr=low_addr;
		midato_write_8[addr].high_addr=high_addr;
		midato_write_8[addr].mem_handler=NULL;
		midato_write_8[addr].data=(void *)(offset-low_addr);
		midato_write_16[addr].low_addr=low_addr;
		midato_write_16[addr].high_addr=high_addr;
		midato_write_16[addr].mem_handler=NULL;
		midato_write_16[addr].data=(void *)(offset-low_addr);
	}
	else
	{
#ifdef DEBUG_MAPPINGS
		dbg(" handler");
#endif
		miprograma[addr].low_addr=low_addr;
		miprograma[addr].high_addr=high_addr;
		miprograma[addr].offset=((unsigned)&mimemoriadummy)-low_addr;
		midato_read_8[addr].low_addr=low_addr;
		midato_read_8[addr].high_addr=high_addr;
		midato_read_8[addr].mem_handler=(void*)banco->bget;
		midato_read_8[addr].data=NULL;
		midato_read_16[addr].low_addr=low_addr;
		midato_read_16[addr].high_addr=high_addr;
		midato_read_16[addr].mem_handler=(void*)banco->wget;
		midato_read_16[addr].data=NULL;
		midato_write_8[addr].low_addr=low_addr;
		midato_write_8[addr].high_addr=high_addr;
		midato_write_8[addr].mem_handler=(void*)banco->bput;
		midato_write_8[addr].data=NULL;
		midato_write_16[addr].low_addr=low_addr;
		midato_write_16[addr].high_addr=high_addr;
		midato_write_16[addr].mem_handler=(void*)banco->wput;
		midato_write_16[addr].data=NULL;
	}
	m68k_set_context(&micontexto);
}

void check_prefs_changed_cpu (void)
{
	int i;
	switch(m68k_speed)
	{
		case 6:
			timeslice_shift=9;
			cycles_factor=1.86;
			for(i=0;i<4;i++)
				next_vpos[i]=4;
			next_vpos[4]=5;
			for(i=5;i<20;i++)
				next_vpos[i]=20;
			for(i=20;i<280;i++)
				next_vpos[i]=i+1;
			for(i=280;i<306;i++)
				next_vpos[i]=306;
			for(i=306;i<311;i++)
				next_vpos[i]=i+1;
			for(i=311;i<512;i++)
				next_vpos[i]=510;
			break;
		case 5:
			timeslice_shift=6;
			cycles_factor=(4.0/3.0);
			for(i=0;i<4;i++)
				next_vpos[i]=4;
			next_vpos[4]=5;
			for(i=5;i<40;i++)
				next_vpos[i]=40;
			for(i=40;i<280;i++)
				next_vpos[i]=i+1;
			for(i=280;i<312;i++)
				next_vpos[i]=510;
			break;
		case 4:
			timeslice_shift=7;
			cycles_factor=(4.0/3.0);
			for(i=0;i<4;i++)
				next_vpos[i]=4;
			next_vpos[4]=5;
			for(i=5;i<40;i++)
				next_vpos[i]=40;
			for(i=40;i<280;i++)
				next_vpos[i]=i+1;
			for(i=280;i<312;i++)
				next_vpos[i]=510;
			break;
		case 3:
			timeslice_shift=7;
			cycles_factor=(7.0/6.0);
			for(i=0;i<4;i++)
				next_vpos[i]=4;
			next_vpos[4]=5;
			for(i=5;i<20;i++)
				next_vpos[i]=20;
			for(i=20;i<280;i++)
				next_vpos[i]=i+1;
			for(i=280;i<306;i++)
				next_vpos[i]=306;
			for(i=306;i<311;i++)
				next_vpos[i]=i+1;
			for(i=311;i<512;i++)
				next_vpos[i]=510;
			break;
		case 2:
			timeslice_shift=8;
			cycles_factor=(7.0/6.0);
			for(i=0;i<4;i++)
				next_vpos[i]=4;
			next_vpos[4]=5;
			for(i=5;i<20;i++)
				next_vpos[i]=20;
			for(i=20;i<280;i++)
				next_vpos[i]=i+1;
			for(i=280;i<306;i++)
				next_vpos[i]=306;
			for(i=306;i<311;i++)
				next_vpos[i]=i+1;
			for(i=311;i<512;i++)
				next_vpos[i]=510;
			break;
		case 1:
			timeslice_shift=7;
			cycles_factor=1.0;
			for(i=0;i<512;i++)
				next_vpos[i]=i+1;
			break;
		default:
			timeslice_shift=8;
			cycles_factor=1.0;
			for(i=0;i<512;i++)
				next_vpos[i]=i+1;

	}
	next_vpos[511]=0;
}

/* CPU save/restore code */

#define CPUTYPE_EC 1
#define CPUMODE_HALT 1

uae_u8 *restore_cpu (uae_u8 *src)
{
    int i,model,flags;
    uae_u32 l;

    model = restore_u32();
    flags = restore_u32();
    for (i = 0; i < 8; i++)
	    _68k_dreg(i)=restore_u32 ();
    for (i = 0; i < 8; i++)
	    _68k_areg(i)=restore_u32 ();
    _68k_setpc(restore_u32 ());
    /* We don't actually use this - we deliberately set prefetch_pc to a
       zero so that prefetch isn't used for the first insn after a state
       restore.  */
    /* uae_regs.prefetch = */ restore_u32 ();
    /* uae_regs.prefetch_pc =  uae_regs.pc + 128; */
    _68k_mspreg = restore_u32 ();
    /* uae_regs.isp = */ restore_u32 ();
    _68k_sreg = restore_u16 ();
    l = restore_u32();
    if (l & CPUMODE_HALT) {
	M68KCONTEXT.execinfo|=0x0080;
	mispcflags=SPCFLAG_STOP;
    } else {
	M68KCONTEXT.execinfo&=~0x0080;
	mispcflags=0;
    }
    write_log ("CPU %d%s%03d, PC=%08.8X\n",
	       model/1000, flags & 1 ? "EC" : "", model % 1000, _68k_getpc());

    return src;
}


uae_u8 *save_cpu (int *len)
{
    uae_u8 *dstbak,*dst;
    int model,i;

    dstbak = dst = (uae_u8 *)malloc(4+4+15*4+4+4+4+4+2+4+4+4+4+4+4+4);
    model = 68000;
    save_u32 (model);					/* MODEL */
    save_u32 (1); //currprefs.address_space_24 ? 1 : 0);	/* FLAGS */
    for(i = 0;i < 8; i++)
	    save_u32 (_68k_dreg(i));
    for(i = 0;i < 8; i++)
	    save_u32 (_68k_areg(i));
    save_u32 (_68k_getpc ());				/* PC */
    save_u32 (0); //uae_regs.prefetch);				/* prefetch */
    save_u32 (_68k_mspreg);
    save_u32 (_68k_areg(7));
    save_u16 (_68k_sreg);				/* SR/CCR */
    save_u32 (M68KCONTEXT.execinfo&0x0080 ? CPUMODE_HALT : 0);	/* flags */
    *len = dst - dstbak;
    return dstbak;
}
