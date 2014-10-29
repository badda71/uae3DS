 /*
  * UAE - The Un*x Amiga Emulator
  *
  * MC68000 emulation
  *
  * (c) 1995 Bernd Schmidt
  */

#include "sysconfig.h"
#include "sysdeps.h"
#include "debug_uae4all.h"
#include "config.h"
#include "uae.h"
#include "options.h"
#include "events.h"
#include "memory.h"
#include "custom.h"
#include "autoconf.h"
#include "ersatz.h"
#include "gui.h"
#include "savestate.h"
#include "blitter.h"

#include "m68k/debug_m68k.h"
#include "compiler.h"

int m68k_speed=5;
int next_positions[512];
int *next_vpos=&next_positions[0];

static unsigned int2doint=0;

#ifdef DEBUG_UAE4ALL
#if !defined(START_DEBUG) || START_DEBUG==0 
int DEBUG_AHORA=1;
#else
int DEBUG_AHORA=0;
#endif
FILE *DEBUG_STR_FILE=NULL;
#endif

// #define DEBUG_INTERRUPTS_EXTRA

#ifdef DEBUG_UAE4ALL
void guarda(void)
{
	unsigned long ciclo=get_cycles();
	unsigned pc=_68k_getpc();
	unsigned sr=make_a_sr();
	unsigned a0=_68k_areg(0);
	unsigned a1=_68k_areg(1);
	unsigned a2=_68k_areg(2);
	unsigned a3=_68k_areg(3);
	unsigned a4=_68k_areg(4);
	unsigned a5=_68k_areg(5);
	unsigned a6=_68k_areg(6);
	unsigned a7=_68k_areg(7);
	unsigned d0=_68k_dreg(0);
	unsigned d1=_68k_dreg(1);
	unsigned d2=_68k_dreg(2);
	unsigned d3=_68k_dreg(3);
	unsigned d4=_68k_dreg(4);
	unsigned d5=_68k_dreg(5);
	unsigned d6=_68k_dreg(6);
	unsigned d7=_68k_dreg(7);

	FILE *f=fopen("/tmp/uae4all_guarda","wb");
	fwrite((void *)&pc,sizeof(unsigned),1,f);
	fwrite((void *)&sr,sizeof(unsigned),1,f);
	fwrite((void *)&a0,sizeof(unsigned),1,f);
	fwrite((void *)&a1,sizeof(unsigned),1,f);
	fwrite((void *)&a2,sizeof(unsigned),1,f);
	fwrite((void *)&a3,sizeof(unsigned),1,f);
	fwrite((void *)&a4,sizeof(unsigned),1,f);
	fwrite((void *)&a5,sizeof(unsigned),1,f);
	fwrite((void *)&a6,sizeof(unsigned),1,f);
	fwrite((void *)&a7,sizeof(unsigned),1,f);
	fwrite((void *)&d0,sizeof(unsigned),1,f);
	fwrite((void *)&d1,sizeof(unsigned),1,f);
	fwrite((void *)&d2,sizeof(unsigned),1,f);
	fwrite((void *)&d3,sizeof(unsigned),1,f);
	fwrite((void *)&d4,sizeof(unsigned),1,f);
	fwrite((void *)&d5,sizeof(unsigned),1,f);
	fwrite((void *)&d6,sizeof(unsigned),1,f);
	fwrite((void *)&d7,sizeof(unsigned),1,f);
	fwrite((void *)chipmemory,1,allocated_chipmem,f);
	fclose(f);
}

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
		uae_regs.sr=sr;
		MakeFromSR();
		_68k_setpc(pc);
	}
}
#endif


/* Opcode of faulting instruction */
uae_u16 last_op_for_exception_3;
/* PC at fault time */
uaecptr last_addr_for_exception_3;
/* Address that generated the exception */
uaecptr last_fault_for_exception_3;

int areg_byteinc[] = { 1,1,1,1,1,1,1,2 };
int imm8_table[] = { 8,1,2,3,4,5,6,7 };

int movem_index1[256];
int movem_index2[256];
int movem_next[256];

int fpp_movem_index1[256];
int fpp_movem_index2[256];
int fpp_movem_next[256];

cpuop_func *cpufunctbl[65536];

#define COUNT_INSTRS 0

#if COUNT_INSTRS
static unsigned long int instrcount[65536];
static uae_u16 opcodenums[65536];

static int compfn (const void *el1, const void *el2)
{
    return instrcount[*(const uae_u16 *)el1] < instrcount[*(const uae_u16 *)el2];
}

static char *icountfilename (void)
{
    char *name = getenv ("INSNCOUNT");
    if (name)
	return name;
    return COUNT_INSTRS == 2 ? "frequent.68k" : "insncount";
}

void dump_counts (void)
{
    FILE *f = fopen (icountfilename (), "w");
    unsigned long int total;
    int i;

    write_log ("Writing instruction count file...\n");
    for (i = 0; i < 65536; i++) {
	opcodenums[i] = i;
	total += instrcount[i];
    }
    qsort (opcodenums, 65536, sizeof(uae_u16), compfn);

    fprintf (f, "Total: %lu\n", total);
    for (i=0; i < 65536; i++) {
	unsigned long int cnt = instrcount[opcodenums[i]];
	struct instr *dp;
	struct mnemolookup *lookup;
	if (!cnt)
	    break;
	dp = table68k + opcodenums[i];
	for (lookup = lookuptab;lookup->mnemo != dp->mnemo; lookup++)
	    ;
	fprintf (f, "%04x: %lu %s\n", opcodenums[i], cnt, lookup->name);
    }
    fclose (f);
}
#else
void dump_counts (void)
{
}
#endif

int broken_in;

static unsigned long op_illg_1 (uae_u32 opcode) REGPARAM;

static unsigned long REGPARAM2 op_illg_1 (uae_u32 opcode)
{
    op_illg (opcode);
    return 4;
}

static void build_cpufunctbl (void)
{
//printf("BUILD_CPU!!!!");
    int i;
    unsigned long opcode;
    struct cputbl *tbl = op_smalltbl_4_ff;

    for (opcode = 0; opcode < 65536; opcode++)
	cpufunctbl[opcode] = op_illg_1;
    for (i = 0; tbl[i].handler != NULL; i++) {
	if (! tbl[i].specific)
	    cpufunctbl[tbl[i].opcode] = tbl[i].handler;
    }
    for (opcode = 0; opcode < 65536; opcode++) {
	cpuop_func *f;

	if (table68k[opcode].mnemo == i_ILLG || table68k[opcode].clev > 0)
	    continue;

	if (table68k[opcode].handler != -1) {
	    f = cpufunctbl[table68k[opcode].handler];
	    if (f == op_illg_1)
		abort();
	    cpufunctbl[opcode] = f;
	}
    }
    for (i = 0; tbl[i].handler != NULL; i++) {
	if (tbl[i].specific)
	    cpufunctbl[tbl[i].opcode] = tbl[i].handler;
    }
}

static void update_68k_cycles (void)
{
    check_prefs_changed_cpu();
}

void init_m68k (void)
{
    int i;

    update_68k_cycles ();

    for (i = 0 ; i < 256 ; i++) {
	int j;
	for (j = 0 ; j < 8 ; j++) {
		if (i & (1 << j)) break;
	}
	movem_index1[i] = j;
	movem_index2[i] = 7-j;
	movem_next[i] = i & (~(1 << j));
    }
    for (i = 0 ; i < 256 ; i++) {
	int j;
	for (j = 7 ; j >= 0 ; j--) {
		if (i & (1 << j)) break;
	}
	fpp_movem_index1[i] = 7-j;
	fpp_movem_index2[i] = j;
	fpp_movem_next[i] = i & (~(1 << j));
    }
#if COUNT_INSTRS
    {
	FILE *f = fopen (icountfilename (), "r");
	memset (instrcount, 0, sizeof instrcount);
	if (f) {
	    uae_u32 opcode, count, total;
	    char name[20];
	    write_log ("Reading instruction count file...\n");
	    fscanf (f, "Total: %lu\n", &total);
	    while (fscanf (f, "%lx: %lu %s\n", &opcode, &count, name) == 3) {
		instrcount[opcode] = count;
	    }
	    fclose(f);
	}
    }
#endif
    
    read_table68k ();
    do_merges ();

//    write_log ("%d CPU functions\n", nr_cpuop_funcs);

    build_cpufunctbl ();
}

struct uae_regstruct uae_regs;
//struct uae_regstruct lastint_uae_regs;
static struct uae_regstruct uae_regs_backup[16];
static int backup_pointer = 0;
static long int m68kpc_offset;
// int lastint_no;

#define get_ibyte_1(o) get_byte(uae_regs.pc + (uae_regs.pc_p - uae_regs.pc_oldp) + (o) + 1)
#define get_iword_1(o) get_word(uae_regs.pc + (uae_regs.pc_p - uae_regs.pc_oldp) + (o))
#define get_ilong_1(o) get_long(uae_regs.pc + (uae_regs.pc_p - uae_regs.pc_oldp) + (o))

uae_s32 ShowEA (FILE *f, int reg, amodes mode, wordsizes size, char *buf)
{
    uae_u16 dp;
    uae_s8 disp8;
    uae_s16 disp16;
    int r;
    uae_u32 dispreg;
    uaecptr addr;
    uae_s32 offset = 0;
    char buffer[80];

    switch (mode){
     case Dreg:
	sprintf (buffer,"D%d", reg);
	break;
     case Areg:
	sprintf (buffer,"A%d", reg);
	break;
     case Aind:
	sprintf (buffer,"(A%d)", reg);
	break;
     case Aipi:
	sprintf (buffer,"(A%d)+", reg);
	break;
     case Apdi:
	sprintf (buffer,"-(A%d)", reg);
	break;
     case Ad16:
	disp16 = get_iword_1 (m68kpc_offset); m68kpc_offset += 2;
	addr = _68k_areg(reg) + (uae_s16)disp16;
	sprintf (buffer,"(A%d,$%04x) == $%08lx", reg, disp16 & 0xffff,
					(unsigned long)addr);
	break;
     case Ad8r:
	dp = get_iword_1 (m68kpc_offset); m68kpc_offset += 2;
	disp8 = dp & 0xFF;
	r = (dp & 0x7000) >> 12;
	dispreg = dp & 0x8000 ? _68k_areg(r) : _68k_dreg(r);
	if (!(dp & 0x800)) dispreg = (uae_s32)(uae_s16)(dispreg);
	dispreg <<= (dp >> 9) & 3;

	if (dp & 0x100) {
	    uae_s32 outer = 0, disp = 0;
	    uae_s32 base = _68k_areg(reg);
	    char name[10];
	    sprintf (name,"A%d, ",reg);
	    if (dp & 0x80) { base = 0; name[0] = 0; }
	    if (dp & 0x40) dispreg = 0;
	    if ((dp & 0x30) == 0x20) { disp = (uae_s32)(uae_s16)get_iword_1 (m68kpc_offset); m68kpc_offset += 2; }
	    if ((dp & 0x30) == 0x30) { disp = get_ilong_1 (m68kpc_offset); m68kpc_offset += 4; }
	    base += disp;

	    if ((dp & 0x3) == 0x2) { outer = (uae_s32)(uae_s16)get_iword_1 (m68kpc_offset); m68kpc_offset += 2; }
	    if ((dp & 0x3) == 0x3) { outer = get_ilong_1 (m68kpc_offset); m68kpc_offset += 4; }

	    if (!(dp & 4)) base += dispreg;
	    if (dp & 3) base = get_long (base);
	    if (dp & 4) base += dispreg;

	    addr = base + outer;
	    sprintf (buffer,"(%s%c%d.%c*%d+%ld)+%ld == $%08lx", name,
		    dp & 0x8000 ? 'A' : 'D', (int)r, dp & 0x800 ? 'L' : 'W',
		    1 << ((dp >> 9) & 3),
		    disp,outer,
		    (unsigned long)addr);
	} else {
	  addr = _68k_areg(reg) + (uae_s32)((uae_s8)disp8) + dispreg;
	  sprintf (buffer,"(A%d, %c%d.%c*%d, $%02x) == $%08lx", reg,
	       dp & 0x8000 ? 'A' : 'D', (int)r, dp & 0x800 ? 'L' : 'W',
	       1 << ((dp >> 9) & 3), disp8,
	       (unsigned long)addr);
	}
	break;
     case PC16:
	addr = m68k_getpc () + m68kpc_offset;
	disp16 = get_iword_1 (m68kpc_offset); m68kpc_offset += 2;
	addr += (uae_s16)disp16;
	sprintf (buffer,"(PC,$%04x) == $%08lx", disp16 & 0xffff,(unsigned long)addr);
	break;
     case PC8r:
	addr = m68k_getpc () + m68kpc_offset;
	dp = get_iword_1 (m68kpc_offset); m68kpc_offset += 2;
	disp8 = dp & 0xFF;
	r = (dp & 0x7000) >> 12;
	dispreg = dp & 0x8000 ? _68k_areg(r) : _68k_dreg(r);
	if (!(dp & 0x800)) dispreg = (uae_s32)(uae_s16)(dispreg);
	dispreg <<= (dp >> 9) & 3;

	if (dp & 0x100) {
	    uae_s32 outer = 0,disp = 0;
	    uae_s32 base = addr;
	    char name[10];
	    sprintf (name,"PC, ");
	    if (dp & 0x80) { base = 0; name[0] = 0; }
	    if (dp & 0x40) dispreg = 0;
	    if ((dp & 0x30) == 0x20) { disp = (uae_s32)(uae_s16)get_iword_1 (m68kpc_offset); m68kpc_offset += 2; }
	    if ((dp & 0x30) == 0x30) { disp = get_ilong_1 (m68kpc_offset); m68kpc_offset += 4; }
	    base += disp;

	    if ((dp & 0x3) == 0x2) { outer = (uae_s32)(uae_s16)get_iword_1 (m68kpc_offset); m68kpc_offset += 2; }
	    if ((dp & 0x3) == 0x3) { outer = get_ilong_1 (m68kpc_offset); m68kpc_offset += 4; }

	    if (!(dp & 4)) base += dispreg;
	    if (dp & 3) base = get_long (base);
	    if (dp & 4) base += dispreg;

	    addr = base + outer;
	    sprintf (buffer,"(%s%c%d.%c*%d+%ld)+%ld == $%08lx", name,
		    dp & 0x8000 ? 'A' : 'D', (int)r, dp & 0x800 ? 'L' : 'W',
		    1 << ((dp >> 9) & 3),
		    disp,outer,
		    (unsigned long)addr);
	} else {
	  addr += (uae_s32)((uae_s8)disp8) + dispreg;
	  sprintf (buffer,"(PC, %c%d.%c*%d, $%02x) == $%08lx", dp & 0x8000 ? 'A' : 'D',
		(int)r, dp & 0x800 ? 'L' : 'W',  1 << ((dp >> 9) & 3),
		disp8, (unsigned long)addr);
	}
	break;
     case absw:
	sprintf (buffer,"$%08lx", (unsigned long)(uae_s32)(uae_s16)get_iword_1 (m68kpc_offset));
	m68kpc_offset += 2;
	break;
     case absl:
	sprintf (buffer,"$%08lx", (unsigned long)get_ilong_1 (m68kpc_offset));
	m68kpc_offset += 4;
	break;
     case imm:
	switch (size){
	 case sz_byte:
	    sprintf (buffer,"#$%02x", (unsigned int)(get_iword_1 (m68kpc_offset) & 0xff));
	    m68kpc_offset += 2;
	    break;
	 case sz_word:
	    sprintf (buffer,"#$%04x", (unsigned int)(get_iword_1 (m68kpc_offset) & 0xffff));
	    m68kpc_offset += 2;
	    break;
	 case sz_long:
	    sprintf (buffer,"#$%08lx", (unsigned long)(get_ilong_1 (m68kpc_offset)));
	    m68kpc_offset += 4;
	    break;
	 default:
	    break;
	}
	break;
     case imm0:
	offset = (uae_s32)(uae_s8)get_iword_1 (m68kpc_offset);
	m68kpc_offset += 2;
	sprintf (buffer,"#$%02x", (unsigned int)(offset & 0xff));
	break;
     case imm1:
	offset = (uae_s32)(uae_s16)get_iword_1 (m68kpc_offset);
	m68kpc_offset += 2;
	sprintf (buffer,"#$%04x", (unsigned int)(offset & 0xffff));
	break;
     case imm2:
	offset = (uae_s32)get_ilong_1 (m68kpc_offset);
	m68kpc_offset += 4;
	sprintf (buffer,"#$%08lx", (unsigned long)offset);
	break;
     case immi:
	offset = (uae_s32)(uae_s8)(reg & 0xff);
	sprintf (buffer,"#$%08lx", (unsigned long)offset);
	break;
     default:
	break;
    }
    if (buf == 0)
	fprintf (f, "%s", buffer);
    else
	strcat (buf, buffer);
    return offset;
}

uae_u32 get_disp_ea_020 (uae_u32 base, uae_u32 dp)
{
    int reg = (dp >> 12) & 15;
    uae_s32 regd = uae_regs.uae_regs[reg];
    if ((dp & 0x800) == 0)
	regd = (uae_s32)(uae_s16)regd;
    regd <<= (dp >> 9) & 3;
    if (dp & 0x100) {
	uae_s32 outer = 0;
	if (dp & 0x80) base = 0;
	if (dp & 0x40) regd = 0;

	if ((dp & 0x30) == 0x20) base += (uae_s32)(uae_s16)next_iword();
	if ((dp & 0x30) == 0x30) base += next_ilong();

	if ((dp & 0x3) == 0x2) outer = (uae_s32)(uae_s16)next_iword();
	if ((dp & 0x3) == 0x3) outer = next_ilong();

	if ((dp & 0x4) == 0) base += regd;
	if (dp & 0x3) base = get_long (base);
	if (dp & 0x4) base += regd;

	return base + outer;
    } else {
	return base + (uae_s32)((uae_s8)dp) + regd;
    }
}

uae_u32 get_disp_ea_000 (uae_u32 base, uae_u32 dp)
{
    int reg = (dp >> 12) & 15;
    uae_s32 regd = uae_regs.uae_regs[reg];
#if 1
    if ((dp & 0x800) == 0)
	regd = (uae_s32)(uae_s16)regd;
    return base + (uae_s8)dp + regd;
#else
    /* Branch-free code... benchmark this again now that
     * things are no longer inline.  */
    uae_s32 regd16;
    uae_u32 mask;
    mask = ((dp & 0x800) >> 11) - 1;
    regd16 = (uae_s32)(uae_s16)regd;
    regd16 &= mask;
    mask = ~mask;
    base += (uae_s8)dp;
    regd &= mask;
    regd |= regd16;
    return base + regd;
#endif
}

void MakeSR (void)
{
    uae_regs.sr = ((uae_regs.t1 << 15) | (uae_regs.t0 << 14)
	       | (uae_regs.s << 13) | (uae_regs.m << 12) | (uae_regs.intmask << 8)
	       | (GET_XFLG << 4) | (GET_NFLG << 3) | (GET_ZFLG << 2) | (GET_VFLG << 1)
	       | GET_CFLG);
}

void MakeFromSR (void)
{
    int oldm = uae_regs.m;
    int olds = uae_regs.s;

    uae_regs.t1 = (uae_regs.sr >> 15) & 1;
    uae_regs.t0 = (uae_regs.sr >> 14) & 1;
    uae_regs.s = (uae_regs.sr >> 13) & 1;
    uae_regs.m = (uae_regs.sr >> 12) & 1;
    uae_regs.intmask = (uae_regs.sr >> 8) & 7;
    SET_XFLG ((uae_regs.sr >> 4) & 1);
    SET_NFLG ((uae_regs.sr >> 3) & 1);
    SET_ZFLG ((uae_regs.sr >> 2) & 1);
    SET_VFLG ((uae_regs.sr >> 1) & 1);
    SET_CFLG (uae_regs.sr & 1);
	if (olds != uae_regs.s) {
	    if (olds) {
		uae_regs.isp = _68k_areg(7);
		_68k_areg(7) = uae_regs.usp;
	    } else {
		uae_regs.usp = _68k_areg(7);
		_68k_areg(7) = uae_regs.isp;
	    }
	}

    set_special (SPCFLAG_INT);
    if (uae_regs.t1 || uae_regs.t0)
	set_special (SPCFLAG_TRACE);
    else
    	/* Keep SPCFLAG_DOTRACE, we still want a trace exception for
	   SR-modifying instructions (including STOP).  */
	unset_special (SPCFLAG_TRACE);
}

void Exception(int nr, uaecptr oldpc)
{
    uae_u32 currpc = m68k_getpc ();

    compiler_flush_jsr_stack();
    MakeSR();

    if (!uae_regs.s) {
	uae_regs.usp = _68k_areg(7);
	    _68k_areg(7) = uae_regs.isp;
	uae_regs.s = 1;
    }
// /*
	if (nr == 2 || nr == 3) {
	    _68k_areg(7) -= 12;
	    // ??????? 
	    if (nr == 3) {
		put_long (_68k_areg(7), last_fault_for_exception_3);
		put_word (_68k_areg(7)+4, last_op_for_exception_3);
		put_long (_68k_areg(7)+8, last_addr_for_exception_3);
	    }
	    write_log ("Exception!\n");
	    goto kludge_me_do;
	}
// */
    _68k_areg(7) -= 4;
    put_long (_68k_areg(7), currpc);
kludge_me_do:
    _68k_areg(7) -= 2;
    put_word (_68k_areg(7), uae_regs.sr);
    m68k_setpc (get_long (uae_regs.vbr + 4*nr));
    fill_prefetch_0 ();
    uae_regs.t1 = uae_regs.t0 = uae_regs.m = 0;
    unset_special (SPCFLAG_TRACE | SPCFLAG_DOTRACE);
}

static void Interrupt (int nr)
{
#ifdef DEBUG_INTERRUPTS
//	dbgf("Interrupt %i\n",nr);
#endif
    assert(nr < 8 && nr >= 0);
//    lastint_uae_regs = uae_regs;
//    lastint_no = nr;
    Exception(nr+24, 0);

    uae_regs.intmask = nr;
    set_special (SPCFLAG_INT);
}

static uae_u32 caar, cacr, itt0, itt1, dtt0, dtt1, tc, mmusr, urp, srp;

int m68k_move2c (int regno, uae_u32 *regp)
{
    {
	switch (regno) {
	case 0: uae_regs.sfc = *regp & 7; break;
	case 1: uae_regs.dfc = *regp & 7; break;
	case 2: cacr = *regp & (0x3); break;
	case 3: tc = *regp & 0xc000; break;
	    /* Mask out fields that should be zero.  */
	case 4: itt0 = *regp & 0xffffe364; break;
	case 5: itt1 = *regp & 0xffffe364; break;
	case 6: dtt0 = *regp & 0xffffe364; break;
	case 7: dtt1 = *regp & 0xffffe364; break;
	  
	case 0x800: uae_regs.usp = *regp; break;
	case 0x801: uae_regs.vbr = *regp; break;
	case 0x802: caar = *regp & 0xfc; break;
	case 0x803: uae_regs.msp = *regp; if (uae_regs.m == 1) _68k_areg(7) = uae_regs.msp; break;
	case 0x804: uae_regs.isp = *regp; if (uae_regs.m == 0) _68k_areg(7) = uae_regs.isp; break;
	case 0x805: mmusr = *regp; break;
	case 0x806: urp = *regp; break;
	case 0x807: srp = *regp; break;
	default:
	    op_illg (0x4E7B);
	    return 0;
	}
    }
    return 1;
}

int m68k_movec2 (int regno, uae_u32 *regp)
{
    {
	switch (regno) {
	case 0: *regp = uae_regs.sfc; break;
	case 1: *regp = uae_regs.dfc; break;
	case 2: *regp = cacr; break;
	case 3: *regp = tc; break;
	case 4: *regp = itt0; break;
	case 5: *regp = itt1; break;
	case 6: *regp = dtt0; break;
	case 7: *regp = dtt1; break;
	case 0x800: *regp = uae_regs.usp; break;
	case 0x801: *regp = uae_regs.vbr; break;
	case 0x802: *regp = caar; break;
	case 0x803: *regp = uae_regs.m == 1 ? _68k_areg(7) : uae_regs.msp; break;
	case 0x804: *regp = uae_regs.m == 0 ? _68k_areg(7) : uae_regs.isp; break;
	case 0x805: *regp = mmusr; break;
	case 0x806: *regp = urp; break;
	case 0x807: *regp = srp; break;
	default:
	    op_illg (0x4E7A);
	    return 0;
	}
    }
    return 1;
}

STATIC_INLINE int
div_unsigned(uae_u32 src_hi, uae_u32 src_lo, uae_u32 div, uae_u32 *quot, uae_u32 *rem)
{
	uae_u32 q = 0, cbit = 0;
	int i;

	if (div <= src_hi) {
	    return 1;
	}
	for (i = 0 ; i < 32 ; i++) {
		cbit = src_hi & 0x80000000ul;
		src_hi <<= 1;
		if (src_lo & 0x80000000ul) src_hi++;
		src_lo <<= 1;
		q = q << 1;
		if (cbit || div <= src_hi) {
			q |= 1;
			src_hi -= div;
		}
	}
	*quot = q;
	*rem = src_hi;
	return 0;
}

void m68k_divl (uae_u32 opcode, uae_u32 src, uae_u16 extra, uaecptr oldpc)
{
#if defined(uae_s64)
    if (src == 0) {
	Exception (5, oldpc);
	return;
    }
    if (extra & 0x800) {
	// signed variant 
	uae_s64 a = (uae_s64)(uae_s32)_68k_dreg((extra >> 12) & 7);
	uae_s64 quot, rem;

	if (extra & 0x400) {
	    a &= 0xffffffffu;
	    a |= (uae_s64)_68k_dreg(extra & 7) << 32;
	}
	rem = a % (uae_s64)(uae_s32)src;
	quot = a / (uae_s64)(uae_s32)src;
	if ((quot & UVAL64(0xffffffff80000000)) != 0
	    && (quot & UVAL64(0xffffffff80000000)) != UVAL64(0xffffffff80000000))
	{
	    SET_VFLG (1);
	    SET_NFLG (1);
	    SET_CFLG (0);
	} else {
	    if (((uae_s32)rem < 0) != ((uae_s64)a < 0)) rem = -rem;
	    SET_VFLG (0);
	    SET_CFLG (0);
	    SET_ZFLG (((uae_s32)quot) == 0);
	    SET_NFLG (((uae_s32)quot) < 0);
	    _68k_dreg(extra & 7) = rem;
	    _68k_dreg((extra >> 12) & 7) = quot;
	}
    } else {
	// unsigned
	uae_u64 a = (uae_u64)(uae_u32)_68k_dreg((extra >> 12) & 7);
	uae_u64 quot, rem;

	if (extra & 0x400) {
	    a &= 0xffffffffu;
	    a |= (uae_u64)_68k_dreg(extra & 7) << 32;
	}
	rem = a % (uae_u64)src;
	quot = a / (uae_u64)src;
	if (quot > 0xffffffffu) {
	    SET_VFLG (1);
	    SET_NFLG (1);
	    SET_CFLG (0);
	} else {
	    SET_VFLG (0);
	    SET_CFLG (0);
	    SET_ZFLG (((uae_s32)quot) == 0);
	    SET_NFLG (((uae_s32)quot) < 0);
	    _68k_dreg(extra & 7) = rem;
	    _68k_dreg((extra >> 12) & 7) = quot;
	}
    }
#else
    if (src == 0) {
	Exception (5, oldpc);
	return;
    }
    if (extra & 0x800) {
	// signed variant 
	uae_s32 lo = (uae_s32)_68k_dreg((extra >> 12) & 7);
	uae_s32 hi = lo < 0 ? -1 : 0;
	uae_s32 save_high;
	uae_u32 quot, rem;
	uae_u32 sign;

	if (extra & 0x400) {
	    hi = (uae_s32)_68k_dreg(extra & 7);
	}
	save_high = hi;
	sign = (hi ^ src);
	if (hi < 0) {
	    hi = ~hi;
	    lo = -lo;
	    if (lo == 0) hi++;
	}
	if ((uae_s32)src < 0) src = -src;
	if (div_unsigned(hi, lo, src, &quot, &rem) ||
	    (sign & 0x80000000) ? quot > 0x80000000 : quot > 0x7fffffff) {
	    SET_VFLG (1);
	    SET_NFLG (1);
	    SET_CFLG (0);
	} else {
	    if (sign & 0x80000000) quot = -quot;
	    if (((uae_s32)rem < 0) != (save_high < 0)) rem = -rem;
	    SET_VFLG (0);
	    SET_CFLG (0);
	    SET_ZFLG (((uae_s32)quot) == 0);
	    SET_NFLG (((uae_s32)quot) < 0);
	    _68k_dreg(extra & 7) = rem;
	    _68k_dreg((extra >> 12) & 7) = quot;
	}
    } else {
	// unsigned
	uae_u32 lo = (uae_u32)_68k_dreg((extra >> 12) & 7);
	uae_u32 hi = 0;
	uae_u32 quot, rem;

	if (extra & 0x400) {
	    hi = (uae_u32)_68k_dreg(extra & 7);
	}
	if (div_unsigned(hi, lo, src, &quot, &rem)) {
	    SET_VFLG (1);
	    SET_NFLG (1);
	    SET_CFLG (0);
	} else {
	    SET_VFLG (0);
	    SET_CFLG (0);
	    SET_ZFLG (((uae_s32)quot) == 0);
	    SET_NFLG (((uae_s32)quot) < 0);
	    _68k_dreg(extra & 7) = rem;
	    _68k_dreg((extra >> 12) & 7) = quot;
	}
    }
#endif
}

STATIC_INLINE void
mul_unsigned(uae_u32 src1, uae_u32 src2, uae_u32 *dst_hi, uae_u32 *dst_lo)
{
	uae_u32 r0 = (src1 & 0xffff) * (src2 & 0xffff);
	uae_u32 r1 = ((src1 >> 16) & 0xffff) * (src2 & 0xffff);
	uae_u32 r2 = (src1 & 0xffff) * ((src2 >> 16) & 0xffff);
	uae_u32 r3 = ((src1 >> 16) & 0xffff) * ((src2 >> 16) & 0xffff);
	uae_u32 lo;

	lo = r0 + ((r1 << 16) & 0xffff0000ul);
	if (lo < r0) r3++;
	r0 = lo;
	lo = r0 + ((r2 << 16) & 0xffff0000ul);
	if (lo < r0) r3++;
	r3 += ((r1 >> 16) & 0xffff) + ((r2 >> 16) & 0xffff);
	*dst_lo = lo;
	*dst_hi = r3;
}

void m68k_mull (uae_u32 opcode, uae_u32 src, uae_u16 extra)
{
#if defined(uae_s64)
    if (extra & 0x800) {
	/* signed variant */
	uae_s64 a = (uae_s64)(uae_s32)_68k_dreg((extra >> 12) & 7);

	a *= (uae_s64)(uae_s32)src;
	SET_VFLG (0);
	SET_CFLG (0);
	SET_ZFLG (a == 0);
	SET_NFLG (a < 0);
	if (extra & 0x400)
	    _68k_dreg(extra & 7) = a >> 32;
	else if ((a & UVAL64(0xffffffff80000000)) != 0
		 && (a & UVAL64(0xffffffff80000000)) != UVAL64(0xffffffff80000000))
	{
	    SET_VFLG (1);
	}
	_68k_dreg((extra >> 12) & 7) = (uae_u32)a;
    } else {
	/* unsigned */
	uae_u64 a = (uae_u64)(uae_u32)_68k_dreg((extra >> 12) & 7);

	a *= (uae_u64)src;
	SET_VFLG (0);
	SET_CFLG (0);
	SET_ZFLG (a == 0);
	SET_NFLG (((uae_s64)a) < 0);
	if (extra & 0x400)
	    _68k_dreg(extra & 7) = a >> 32;
	else if ((a & UVAL64(0xffffffff00000000)) != 0) {
	    SET_VFLG (1);
	}
	_68k_dreg((extra >> 12) & 7) = (uae_u32)a;
    }
#else
    if (extra & 0x800) {
	/* signed variant */
	uae_s32 src1,src2;
	uae_u32 dst_lo,dst_hi;
	uae_u32 sign;

	src1 = (uae_s32)src;
	src2 = (uae_s32)_68k_dreg((extra >> 12) & 7);
	sign = (src1 ^ src2);
	if (src1 < 0) src1 = -src1;
	if (src2 < 0) src2 = -src2;
	mul_unsigned((uae_u32)src1,(uae_u32)src2,&dst_hi,&dst_lo);
	if (sign & 0x80000000) {
		dst_hi = ~dst_hi;
		dst_lo = -dst_lo;
		if (dst_lo == 0) dst_hi++;
	}
	SET_VFLG (0);
	SET_CFLG (0);
	SET_ZFLG (dst_hi == 0 && dst_lo == 0);
	SET_NFLG (((uae_s32)dst_hi) < 0);
	if (extra & 0x400)
	    _68k_dreg(extra & 7) = dst_hi;
	else if ((dst_hi != 0 || (dst_lo & 0x80000000) != 0)
		 && ((dst_hi & 0xffffffff) != 0xffffffff
		     || (dst_lo & 0x80000000) != 0x80000000))
	{
	    SET_VFLG (1);
	}
	_68k_dreg((extra >> 12) & 7) = dst_lo;
    } else {
	/* unsigned */
	uae_u32 dst_lo,dst_hi;

	mul_unsigned(src,(uae_u32)_68k_dreg((extra >> 12) & 7),&dst_hi,&dst_lo);

	SET_VFLG (0);
	SET_CFLG (0);
	SET_ZFLG (dst_hi == 0 && dst_lo == 0);
	SET_NFLG (((uae_s32)dst_hi) < 0);
	if (extra & 0x400)
	    _68k_dreg(extra & 7) = dst_hi;
	else if (dst_hi != 0) {
	    SET_VFLG (1);
	}
	_68k_dreg((extra >> 12) & 7) = dst_lo;
    }
#endif
}
static const char* ccnames[] =
{ "T ","F ","HI","LS","CC","CS","NE","EQ",
  "VC","VS","PL","MI","GE","LT","GT","LE" };

void m68k_reset (void)
{
    uae_regs.kick_mask = 0x00F80000;
    uae_regs.spcflags = 0;

    if (savestate_state == STATE_RESTORE)
    {
	    m68k_setpc (uae_regs.pc);
	    uae_regs.s = (uae_regs.sr >> 13) & 1;
	    MakeFromSR();
	    if (uae_regs.s)
		_68k_areg(7) = uae_regs.isp;
	    else
		_68k_areg(7) = uae_regs.usp;
	    return;
    }
    _68k_areg(7) = get_long (0x00f80000);
    m68k_setpc (get_long (0x00f80004));
    refill_prefetch (m68k_getpc (), 0);
    fill_prefetch_0 ();
    uae_regs.s = 1;
    uae_regs.m = 0;
    uae_regs.stopped = 0;
    uae_regs.t1 = 0;
    uae_regs.t0 = 0;
    SET_ZFLG (0);
    SET_XFLG (0);
    SET_CFLG (0);
    SET_VFLG (0);
    SET_NFLG (0);
    uae_regs.intmask = 7;
    uae_regs.vbr = uae_regs.sfc = uae_regs.dfc = 0;
    uae_regs.fpcr = uae_regs.fpsr = uae_regs.fpiar = 0;
}

unsigned long REGPARAM2 op_illg (uae_u32 opcode)
{
    uaecptr pc = m68k_getpc ();

#ifdef DEBUG_UAE4ALL
    dbgf("INVALID OPCODE 0x%X at PC=0x%X -> ",opcode,pc);
#endif
    if (cloanto_rom && (opcode & 0xF100) == 0x7100) {
#ifdef DEBUG_UAE4ALL
	dbg("cloanto");
#endif
	_68k_dreg((opcode >> 9) & 7) = (uae_s8)(opcode & 0xFF);
	m68k_incpc (2);
	fill_prefetch_0 ();
	return 4;
    }

    compiler_flush_jsr_stack ();
    if (opcode == 0x4E7B && get_long (0x10) == 0 && (pc & 0xF80000) == 0xF80000) {
#ifdef DEBUG_UAE4ALL
	dbg("68020");
#endif
	write_log ("Your Kickstart requires a 68020 CPU. Giving up.\n");
	broken_in = 1;
	set_special (SPCFLAG_BRK);
	quit_program = 1;
    }
    if (opcode == 0xFF0D) {
	if ((pc & 0xF80000) == 0xF80000) {
#ifdef DEBUG_UAE4ALL
	    dbg("dummy");
#endif
	    // This is from the dummy Kickstart replacement
	    uae_u16 arg = get_iword (2);
	    m68k_incpc (4);
	    ersatz_perform (arg);
	    fill_prefetch_0 ();
	    return 4;
	} else if ((pc & 0xFFFF0000) == RTAREA_BASE) {
#ifdef DEBUG_UAE4ALL
	    dbg("stop");
#endif
	    // User-mode STOP replacement 
	    m68k_setstopped (1);
	    return 4;
	}
    }

    if ((opcode & 0xF000) == 0xA000 && (pc & 0xFFFF0000) == RTAREA_BASE) {
#ifdef DEBUG_UAE4ALL
	dbg("call");
#endif
	// Calltrap.
#ifdef USE_AUTOCONFIG
	m68k_incpc(2);
	call_calltrap (opcode & 0xFFF);
	fill_prefetch_0 ();
#endif
	return 4;
    }

    if ((opcode & 0xF000) == 0xF000) {
#ifdef DEBUG_UAE4ALL
	dbg("exp8");
#endif
	Exception(0xB,0);
	return 4;
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
	Exception(0xA,0);
	return 4;
    }
#ifdef DEBUG_UAE4ALL
    dbg("Real invalid");
#endif
    write_log ("Illegal instruction: %04x at %08lx\n", opcode, pc);
    Exception (4,0);
    return 4;
}

void mmu_op(uae_u32 opcode, uae_u16 extra)
{
    if ((opcode & 0xFE0) == 0x0500) {
	// PFLUSH 
	mmusr = 0;
	write_log ("PFLUSH\n");
    } else if ((opcode & 0x0FD8) == 0x548) {
	// PTEST 
	write_log ("PTEST\n");
    } else
	op_illg (opcode);
}

static int n_insns = 0, n_spcinsns = 0;

static uaecptr last_trace_ad = 0;

static void do_trace (void)
{
    if (uae_regs.t1) {
	last_trace_ad = m68k_getpc ();
	unset_special (SPCFLAG_TRACE);
	set_special (SPCFLAG_DOTRACE);
    }
}

static int do_specialties (int cycles)
{
    if (uae_regs.spcflags & SPCFLAG_COPPER)
    {
#ifdef DEBUG_M68K
	dbg("do_specialties -> do_copper");
#endif
	do_copper ();
    }

    /*n_spcinsns++;*/
    while ((uae_regs.spcflags & SPCFLAG_BLTNASTY) && cycles > 0) {
	int c = blitnasty();
	if (!c) {
	    cycles -= 2 * CYCLE_UNIT;
	    if (cycles < CYCLE_UNIT)
		cycles = 0;
	    c = 1;
	}
#ifdef DEBUG_M68K
	dbgf("do_specialties -> do_cycles BLTNASTY %i\n",c);
#endif
	do_cycles (c * CYCLE_UNIT);
	if (uae_regs.spcflags & SPCFLAG_COPPER)
	{
#ifdef DEBUG_M68K
	    dbg("do_specialties -> do_copper BLTNASTY");
#endif
	    do_copper ();
	}
    }

    run_compiled_code();
    if (uae_regs.spcflags & SPCFLAG_DOTRACE) {
#ifdef DEBUG_INTERRUPTS_EXTRA
	dbgf("EXCEPCION TRAZA, PC=0x%X\n",m68k_getpc());
#endif
	Exception (9,last_trace_ad);
    }

#if defined(DEBUG_M68K) && !defined(SPECIAL_DEBUG_INTERRUPTS)
    if (int2doint)
    {
#ifdef DEBUG_INTERRUPTS_EXTRA
	    dbg("--> INT2DOINT");
#endif
	    int2doint=0;
	    unset_special(SPCFLAG_INT);
	    set_special(SPCFLAG_DOINT);
    }
#endif

    while (uae_regs.spcflags & SPCFLAG_STOP) {
	if (uae_regs.spcflags & (SPCFLAG_INT | SPCFLAG_DOINT)){
	    int intr = intlev ();
	    unset_special (SPCFLAG_INT | SPCFLAG_DOINT);
	    if (intr != -1 && intr > uae_regs.intmask) {
#ifndef DEBUG_M68K
		Interrupt (intr);
#else
		set_special(SPCFLAG_INT);
		unset_special(SPCFLAG_DOINT);
#endif
		uae_regs.stopped = 0;
		unset_special (SPCFLAG_STOP);
		break;
	    }
	}
#ifdef DEBUG_M68K
	dbg("CPU STOPPED !");
#endif
	do_cycles (4 * CYCLE_UNIT);
	if (uae_regs.spcflags & SPCFLAG_COPPER)
	{
#ifdef DEBUG_M68K
    		dbg("do_specialties -> do_copper STOPPED");
#endif
    		do_copper ();
	}
    }

    if (uae_regs.spcflags & SPCFLAG_TRACE)
	do_trace ();

    if ((uae_regs.spcflags & SPCFLAG_DOINT)&&(!(uae_regs.spcflags & SPCFLAG_INT))) {
	int intr = intlev ();
#ifdef DEBUG_INTERRUPTS_EXTRA
	dbgf("DOINT : intr = %i, intmask=%i\n", intr,uae_regs.intmask);
#endif
	unset_special (SPCFLAG_DOINT);
	if (intr != -1 && intr > uae_regs.intmask) {
	    Interrupt (intr);
	    uae_regs.stopped = 0;
	}
    }
    if (uae_regs.spcflags & SPCFLAG_INT) {
#ifdef DEBUG_INTERRUPTS_EXTRA
	dbg("ESTAMOS EN INT -> PASAMOS A DOINT");
#endif
	unset_special (SPCFLAG_INT);
	set_special (SPCFLAG_DOINT);
    }
    if (uae_regs.spcflags & SPCFLAG_BRK ) {
printf("BRK state=%X, flags=%X, PC=%X\n",savestate_state,_68k_spcflags,_68k_getpc());fflush(stdout);
	unset_special (SPCFLAG_BRK);
	return 1;
    }
    return 0;
}

static double cycles_factor;


/* Same thing, but don't use prefetch to get opcode.  */
static void m68k_run (void)
{
    for (;;) {
	int cycles;
	uae_u32 opcode = get_iword (0);

#ifdef DEBUG_M68K
	int exec_opcode=1;
	dbg_cycle(opcode);
#ifndef SPECIAL_DEBUG_INTERRUPTS
	if ((uae_regs.spcflags & SPCFLAG_DOINT)&&(!(uae_regs.spcflags & SPCFLAG_INT))) {
		int intr = intlev ();
		if (intr != -1 && intr > uae_regs.intmask) 
		{
#ifdef DEBUG_INTERRUPTS_EXTRA
			dbg("EN ESPERA");
#endif
			exec_opcode=0;
			int2doint++;
		}
	}
#endif
	if (exec_opcode)
	{
#endif
		uae4all_prof_start(0);
		cycles = (*cpufunctbl[opcode])(opcode);
		uae4all_prof_end(0);
#ifdef DEBUG_M68K

/*
if ((opcode==0xD984)&&(_68k_dreg(4)==0x00000000))
{
_68k_sreg&=0xFFFD;
SET_VFLG(0);
}
*/

	}
	cycles=3413;
#else
	cycles = (((double)cycles)*cycles_factor);
#endif

	uae4all_prof_start(1);
        do_cycles (cycles);
	if (uae_regs.spcflags) {
	    if (do_specialties (cycles))
		return;
	}
	uae4all_prof_end(1);
    }
}

int in_m68k_go = 0;

void m68k_go (int may_quit)
{
    gui_purge_events();
    if (in_m68k_go || !may_quit) {
	write_log ("Bug! m68k_go is not reentrant.\n");
	abort ();
    }

    update_68k_cycles ();

    in_m68k_go++;
    for (;;) {
printf("m68k_go state=%X, flags=%X, PC=%X\n",savestate_state,_68k_spcflags,_68k_getpc());fflush(stdout);
	if (quit_program > 0) {
	    if (quit_program == 1)
		break;
	    quit_program = 0;
	    if (savestate_state == STATE_RESTORE) {
puts("Restaurando");fflush(stdout);
		    restore_state (savestate_filename);
	    }
	    m68k_reset ();
	    reset_all_systems ();
	    customreset ();
	    /* We may have been restoring state, but we're done now. */
	    savestate_restore_finish ();
	    handle_active_events ();
	    if (uae_regs.spcflags)
		do_specialties (0);
	}
	m68k_run();
    }
    in_m68k_go--;
#ifdef DEBUG_UAE4ALL
    puts("BYE?");
#endif
}

void check_prefs_changed_cpu (void)
{
	int i;
	switch(m68k_speed)
	{
		case 6:
			cycles_factor=4.0;
			for(i=0;i<40;i++)
				next_vpos[i]=40;
			for(i=40;i<280;i++)
				next_vpos[i]=i+1;
			for(i=280;i<312;i++)
				next_vpos[i]=510;
			break;
		case 5:
			cycles_factor=(3.0/2.0);
			for(i=0;i<40;i++)
				next_vpos[i]=40;
			for(i=40;i<280;i++)
				next_vpos[i]=i+1;
			for(i=280;i<312;i++)
				next_vpos[i]=510;
			break;
		case 4:
			cycles_factor=(3.0/2.0);
			for(i=0;i<40;i++)
				next_vpos[i]=40;
			for(i=40;i<279;i++)
				next_vpos[i]=i+1;
			for(i=279;i<312;i++)
				next_vpos[i]=510;
			break;
		case 3:
			cycles_factor=(4.0/3.0);
			for(i=0;i<20;i++)
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
			cycles_factor=(4.0/3.0);
			for(i=0;i<20;i++)
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
			cycles_factor=1.0;
			for(i=0;i<512;i++)
				next_vpos[i]=i+1;
			break;
		default:
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
#if 0
    switch (model) {
    case 68000:
	currprefs.cpu_level = 0;
	break;
    case 68010:
	currprefs.cpu_level = 1;
	break;
    case 68020:
	currprefs.cpu_level = 2;
	break;
    default:
	write_log ("Unknown cpu type %d\n", model);
	break;
    }
#endif

    flags = restore_u32();
//    currprefs.address_space_24 = 0;
//    if (flags & CPUTYPE_EC)
//	currprefs.address_space_24 = 1;
    for (i = 0; i < 15; i++)
	uae_regs.uae_regs[i] = restore_u32 ();
    uae_regs.pc = restore_u32 ();
    /* We don't actually use this - we deliberately set prefetch_pc to a
       zero so that prefetch isn't used for the first insn after a state
       restore.  */
    uae_regs.prefetch = restore_u32 ();
    uae_regs.prefetch_pc = uae_regs.pc + 128;
    uae_regs.usp = restore_u32 ();
    uae_regs.isp = restore_u32 ();
    uae_regs.sr = restore_u16 ();
    l = restore_u32();
    if (l & CPUMODE_HALT) {
	uae_regs.stopped = 1;
	set_special (SPCFLAG_STOP);
    } else
	uae_regs.stopped = 0;
#if 0
    if (model >= 68010) {
	regs.dfc = restore_u32 ();
	regs.sfc = restore_u32 ();
	regs.vbr = restore_u32 ();
    }
    if (model >= 68020) {
	caar = restore_u32 ();
	cacr = restore_u32 ();
	uae_regs.msp = restore_u32 ();
    }
#endif
    write_log ("CPU %d%s%03d, PC=%08.8X\n",
	       model/1000, flags & 1 ? "EC" : "", model % 1000, uae_regs.pc);

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
    for(i = 0;i < 15; i++) save_u32 (uae_regs.uae_regs[i]);	/* D0-D7 A0-A6 */
    save_u32 (m68k_getpc ());				/* PC */
    save_u32 (uae_regs.prefetch);				/* prefetch */
    MakeSR ();
    save_u32 (!uae_regs.s ? uae_regs.uae_regs[15] : uae_regs.usp);	/* USP */
    save_u32 (uae_regs.s ? uae_regs.uae_regs[15] : uae_regs.isp);	/* ISP */
    save_u16 (uae_regs.sr);				/* SR/CCR */
    save_u32 (uae_regs.stopped ? CPUMODE_HALT : 0);	/* flags */
#if 0
    if(model >= 68010) {
	save_u32 (regs.dfc);				/* DFC */
	save_u32 (regs.sfc);				/* SFC */
	save_u32 (regs.vbr);				/* VBR */
    }
    if(model >= 68020) {
	save_u32 (caar);				/* CAAR */
	save_u32 (cacr);				/* CACR */
	save_u32 (regs.msp);				/* MSP */
    }
#endif
    *len = dst - dstbak;
    return dstbak;
}
