 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Memory management
  *
  * (c) 1995 Bernd Schmidt
  */

#include "sysconfig.h"
#include "sysdeps.h"

#include "config.h"
#include "uae.h"
#include "options.h"
#include "memory.h"
#include "ersatz.h"
#include "custom.h"
#include "debug_uae4all.h"
#include "events.h"
#include "m68k/m68k_intrf.h"
#include "autoconf.h"
#include "savestate.h"

#include "zfile.h"

unsigned prefs_chipmem_size;

#ifdef USE_MAPPED_MEMORY
#include <sys/mman.h>
#endif

#ifdef USE_FAME_CORE
void clear_fame_mem_dummy(void);
#endif

/* Set by each memory handler that does not simply access real memory.  */
#ifdef USE_SPECIAL_MEM
int special_mem;
#endif

int ersatzkickfile = 0;

uae_u32 allocated_chipmem;
uae_u32 allocated_fastmem;
uae_u32 allocated_bogomem;
uae_u32 allocated_gfxmem;
uae_u32 allocated_z3fastmem;
uae_u32 allocated_a3000mem;

static long chip_filepos;
static long bogo_filepos;
static long rom_filepos;

#ifdef USE_LIB7Z
#include "lib7z/lzma.h"
#else
#include <zlib.h>
#endif
static long compressed_size;

addrbank *mem_banks[65536];

/* This has two functions. It either holds a host address that, when added
   to the 68k address, gives the host address corresponding to that 68k
   address (in which case the value in this array is even), OR it holds the
   same value as mem_banks, for those banks that have baseaddr==0. In that
   case, bit 0 is set (the memory access routines will take care of it).  */

uae_u8 *baseaddr[65536];

#ifdef NO_INLINE_MEMORY_ACCESS
static __inline__ uae_u32 longget (uaecptr addr)
{
    return call_mem_get_func (get_mem_bank (addr).lget, addr);
}
static __inline__ uae_u32 wordget (uaecptr addr)
{
    return call_mem_get_func (get_mem_bank (addr).wget, addr);
}
static __inline__ uae_u32 byteget (uaecptr addr)
{
    return call_mem_get_func (get_mem_bank (addr).bget, addr);
}
static __inline__ void longput (uaecptr addr, uae_u32 l)
{
    call_mem_put_func (get_mem_bank (addr).lput, addr, l);
}
static __inline__ void wordput (uaecptr addr, uae_u32 w)
{
    call_mem_put_func (get_mem_bank (addr).wput, addr, w);
}
static __inline__ void byteput (uaecptr addr, uae_u32 b)
{
    call_mem_put_func (get_mem_bank (addr).bput, addr, b);
}
#endif

uae_u32 chipmem_mask, kickmem_mask, extendedkickmem_mask, bogomem_mask, a3000mem_mask;

static int illegal_count;
/* A dummy bank that only contains zeros */

static uae_u32 dummy_lget (uaecptr) REGPARAM;
static uae_u32 dummy_wget (uaecptr) REGPARAM;
static uae_u32 dummy_bget (uaecptr) REGPARAM;
static void dummy_lput (uaecptr, uae_u32) REGPARAM;
static void dummy_wput (uaecptr, uae_u32) REGPARAM;
static void dummy_bput (uaecptr, uae_u32) REGPARAM;
static int dummy_check (uaecptr addr, uae_u32 size) REGPARAM;

uae_u32 REGPARAM2 dummy_lget (uaecptr addr)
{
#ifdef DEBUG_MEMORY
    dbgf("dummy_lget 0x%X\n",addr);
#endif
#ifdef USE_SPECIAL_MEM
    special_mem |= S_READ;
#endif
    return 0xFFFFFFFF;
}

uae_u32 REGPARAM2 dummy_wget (uaecptr addr)
{
#ifdef DEBUG_MEMORY
    dbgf("dummy_wget 0x%X\n",addr);
#endif
#ifdef USE_SPECIAL_MEM
    special_mem |= S_READ;
#endif
    return 0xFFFF;
}

uae_u32 REGPARAM2 dummy_bget (uaecptr addr)
{
#ifdef DEBUG_MEMORY
    dbgf("dummy_bget 0x%X\n",addr);
#endif
#ifdef USE_SPECIAL_MEM
    special_mem |= S_READ;
#endif
    return 0xFF;
}

void REGPARAM2 dummy_lput (uaecptr addr, uae_u32 l)
{
#ifdef DEBUG_MEMORY
    dbgf("dummy_lput 0x%X = 0x%X\n",addr,l);
#endif
#ifdef USE_SPECIAL_MEM
    special_mem |= S_WRITE;
#endif
}
void REGPARAM2 dummy_wput (uaecptr addr, uae_u32 w)
{
#ifdef DEBUG_MEMORY
    dbgf("dummy_wput 0x%X = 0x%X\n",addr,w);
#endif
#ifdef USE_SPECIAL_MEM
    special_mem |= S_WRITE;
#endif
}
void REGPARAM2 dummy_bput (uaecptr addr, uae_u32 b)
{
#ifdef DEBUG_MEMORY
    dbgf("dummy_bput 0x%X = 0x%X\n",addr,b);
#endif
#ifdef USE_SPECIAL_MEM
    special_mem |= S_WRITE;
#endif
}

int REGPARAM2 dummy_check (uaecptr addr, uae_u32 size)
{
#ifdef USE_SPECIAL_MEM
    special_mem |= S_READ;
#endif

    return 0;
}

/* A3000 "motherboard resources" bank.  */
static uae_u32 mbres_lget (uaecptr) REGPARAM;
static uae_u32 mbres_wget (uaecptr) REGPARAM;
static uae_u32 mbres_bget (uaecptr) REGPARAM;
static void mbres_lput (uaecptr, uae_u32) REGPARAM;
static void mbres_wput (uaecptr, uae_u32) REGPARAM;
static void mbres_bput (uaecptr, uae_u32) REGPARAM;
static int mbres_check (uaecptr addr, uae_u32 size) REGPARAM;

static int mbres_val = 0;

uae_u32 REGPARAM2 mbres_lget (uaecptr addr)
{
#ifdef DEBUG_MEMORY
    dbgf("mbres_lget 0x%X\n",addr);
#endif
#ifdef USE_SPECIAL_MEM
    special_mem |= S_READ;
#endif

    return 0;
}

uae_u32 REGPARAM2 mbres_wget (uaecptr addr)
{
#ifdef DEBUG_MEMORY
    dbgf("mbres_wget 0x%X\n",addr);
#endif
#ifdef USE_SPECIAL_MEM
    special_mem |= S_READ;
#endif

    return 0;
}

uae_u32 REGPARAM2 mbres_bget (uaecptr addr)
{
#ifdef DEBUG_MEMORY
    dbgf("mbres_bget 0x%X\n",addr);
#endif
#ifdef USE_SPECIAL_MEM
    special_mem |= S_READ;
#endif

    return (addr & 0xFFFF) == 3 ? mbres_val : 0;
}

void REGPARAM2 mbres_lput (uaecptr addr, uae_u32 l)
{
#ifdef DEBUG_MEMORY
    dbgf("mbres_lput 0x%X = 0x%X\n",addr,l);
#endif
#ifdef USE_SPECIAL_MEM
    special_mem |= S_WRITE;
#endif
}
void REGPARAM2 mbres_wput (uaecptr addr, uae_u32 w)
{
#ifdef DEBUG_MEMORY
    dbgf("mbres_wput 0x%X = 0x%X\n",addr,w);
#endif
#ifdef USE_SPECIAL_MEM
    special_mem |= S_WRITE;
#endif
}
void REGPARAM2 mbres_bput (uaecptr addr, uae_u32 b)
{
#ifdef DEBUG_MEMORY
    dbgf("mbres_bput 0x%X = 0x%X\n",addr,b);
#endif
#ifdef USE_SPECIAL_MEM
    special_mem |= S_WRITE;
#endif

    if ((addr & 0xFFFF) == 3)
	mbres_val = b;
}

int REGPARAM2 mbres_check (uaecptr addr, uae_u32 size)
{
    return 0;
}

/* Chip memory */

uae_u8 *chipmemory;
uae_u16 *chipmemory_word;

#ifdef DEBUG_UAE4ALL
unsigned chipmem_checksum(void)
{
	unsigned *p=(unsigned *)chipmemory;
	unsigned max=allocated_chipmem/4;
	unsigned i,ret=0;
	if (p)
	for(i=0;i<max;i++)
		ret+=(i+1)*p[i];
	return ret;
}
#endif

static int chipmem_check (uaecptr addr, uae_u32 size) REGPARAM;
static uae_u8 *chipmem_xlate (uaecptr addr) REGPARAM;

uae_u32 REGPARAM2 chipmem_lget (uaecptr addr)
{
    uae_u32 *m;

    addr -= chipmem_start & chipmem_mask;
    addr &= chipmem_mask;
    m = (uae_u32 *)(chipmemory + addr);
    return swab_l(do_get_mem_long (m));
}

uae_u32 REGPARAM2 chipmem_wget (uaecptr addr)
{
    uae_u16 *m;

    addr -= chipmem_start & chipmem_mask;
    addr &= chipmem_mask;
    m = (uae_u16 *)(chipmemory + addr);
    return swab_w(do_get_mem_word (m));
}

uae_u32 REGPARAM2 chipmem_bget (uaecptr addr)
{
    addr -= chipmem_start & chipmem_mask;
    addr &= chipmem_mask;
    return chipmemory[addr];
}

void REGPARAM2 chipmem_lput (uaecptr addr, uae_u32 l)
{
    uae_u32 *m;

    addr -= chipmem_start & chipmem_mask;
    addr &= chipmem_mask;
    m = (uae_u32 *)(chipmemory + addr);
    do_put_mem_long (m, swab_l(l));
}

void REGPARAM2 chipmem_wput (uaecptr addr, uae_u32 w)
{
    uae_u16 *m;

    addr -= chipmem_start & chipmem_mask;
    addr &= chipmem_mask;
    m = (uae_u16 *)(chipmemory + addr);
    do_put_mem_word (m, swab_w(w));
}

void REGPARAM2 chipmem_bput (uaecptr addr, uae_u32 b)
{
    addr -= chipmem_start & chipmem_mask;
    addr &= chipmem_mask;
    chipmemory[addr] = b;
}

int REGPARAM2 chipmem_check (uaecptr addr, uae_u32 size)
{
    addr -= chipmem_start & chipmem_mask;
    addr &= chipmem_mask;
    return (addr + size) <= allocated_chipmem;
}

uae_u8 REGPARAM2 *chipmem_xlate (uaecptr addr)
{
    addr -= chipmem_start & chipmem_mask;
    addr &= chipmem_mask;
    return chipmemory + addr;
}

/* Slow memory */

static uae_u8 *bogomemory;

static uae_u32 bogomem_lget (uaecptr) REGPARAM;
static uae_u32 bogomem_wget (uaecptr) REGPARAM;
static uae_u32 bogomem_bget (uaecptr) REGPARAM;
static void bogomem_lput (uaecptr, uae_u32) REGPARAM;
static void bogomem_wput (uaecptr, uae_u32) REGPARAM;
static void bogomem_bput (uaecptr, uae_u32) REGPARAM;
static int bogomem_check (uaecptr addr, uae_u32 size) REGPARAM;
static uae_u8 *bogomem_xlate (uaecptr addr) REGPARAM;

uae_u32 REGPARAM2 bogomem_lget (uaecptr addr)
{
#ifdef DEBUG_MEMORY
    dbgf("bogomem_lget 0x%X\n",addr);
#endif
    uae_u32 *m;
    addr -= bogomem_start & bogomem_mask;
    addr &= bogomem_mask;
    m = (uae_u32 *)(bogomemory + addr);
    return SWAP_L(do_get_mem_long (m));
}

uae_u32 REGPARAM2 bogomem_wget (uaecptr addr)
{
#ifdef DEBUG_MEMORY
    dbgf("bogomem_wget 0x%X\n",addr);
#endif
    uae_u16 *m;
    addr -= bogomem_start & bogomem_mask;
    addr &= bogomem_mask;
    m = (uae_u16 *)(bogomemory + addr);
    return SWAP_W(do_get_mem_word (m));
}

uae_u32 REGPARAM2 bogomem_bget (uaecptr addr)
{
#ifdef DEBUG_MEMORY
    dbgf("bogomem_bget 0x%X\n",addr);
#endif
    addr -= bogomem_start & bogomem_mask;
    addr &= bogomem_mask;
    return bogomemory[addr];
}

void REGPARAM2 bogomem_lput (uaecptr addr, uae_u32 l)
{
#ifdef DEBUG_MEMORY
    dbgf("bogomem_lput 0x%X = 0x%X\n",addr,l);
#endif
    uae_u32 *m;
    addr -= bogomem_start & bogomem_mask;
    addr &= bogomem_mask;
    m = (uae_u32 *)(bogomemory + addr);
    do_put_mem_long (m, SWAP_L(l));
}

void REGPARAM2 bogomem_wput (uaecptr addr, uae_u32 w)
{
#ifdef DEBUG_MEMORY
    dbgf("bogomem_wput 0x%X = 0x%X\n",addr,w);
#endif
    uae_u16 *m;
    addr -= bogomem_start & bogomem_mask;
    addr &= bogomem_mask;
    m = (uae_u16 *)(bogomemory + addr);
    do_put_mem_word (m, SWAP_W(w));
}

void REGPARAM2 bogomem_bput (uaecptr addr, uae_u32 b)
{
#ifdef DEBUG_MEMORY
    dbgf("bogomem_bput 0x%X = 0x%X\n",addr,b);
#endif
    addr -= bogomem_start & bogomem_mask;
    addr &= bogomem_mask;
    bogomemory[addr] = b;
}

int REGPARAM2 bogomem_check (uaecptr addr, uae_u32 size)
{
    addr -= bogomem_start & bogomem_mask;
    addr &= bogomem_mask;
    return (addr + size) <= allocated_bogomem;
}

uae_u8 REGPARAM2 *bogomem_xlate (uaecptr addr)
{
    addr -= bogomem_start & bogomem_mask;
    addr &= bogomem_mask;
    return bogomemory + addr;
}

/* A3000 motherboard fast memory */

static uae_u8 *a3000memory;

static uae_u32 a3000mem_lget (uaecptr) REGPARAM;
static uae_u32 a3000mem_wget (uaecptr) REGPARAM;
static uae_u32 a3000mem_bget (uaecptr) REGPARAM;
static void a3000mem_lput (uaecptr, uae_u32) REGPARAM;
static void a3000mem_wput (uaecptr, uae_u32) REGPARAM;
static void a3000mem_bput (uaecptr, uae_u32) REGPARAM;
static int a3000mem_check (uaecptr addr, uae_u32 size) REGPARAM;
static uae_u8 *a3000mem_xlate (uaecptr addr) REGPARAM;

uae_u32 REGPARAM2 a3000mem_lget (uaecptr addr)
{
#ifdef DEBUG_MEMORY
    dbgf("a3000mem_lget 0x%X\n",addr);
#endif
    uae_u32 *m;
    addr -= a3000mem_start & a3000mem_mask;
    addr &= a3000mem_mask;
    m = (uae_u32 *)(a3000memory + addr);
    return SWAP_L(do_get_mem_long (m));
}

uae_u32 REGPARAM2 a3000mem_wget (uaecptr addr)
{
#ifdef DEBUG_MEMORY
    dbgf("a3000mem_wget 0x%X\n",addr);
#endif
    uae_u16 *m;
    addr -= a3000mem_start & a3000mem_mask;
    addr &= a3000mem_mask;
    m = (uae_u16 *)(a3000memory + addr);
    return SWAP_W(do_get_mem_word (m));
}

uae_u32 REGPARAM2 a3000mem_bget (uaecptr addr)
{
#ifdef DEBUG_MEMORY
    dbgf("a3000mem_bget 0x%X\n",addr);
#endif
    addr -= a3000mem_start & a3000mem_mask;
    addr &= a3000mem_mask;
    return a3000memory[addr];
}

void REGPARAM2 a3000mem_lput (uaecptr addr, uae_u32 l)
{
#ifdef DEBUG_MEMORY
    dbgf("a3000mem_lput 0x%X = 0x%X\n",addr,l);
#endif
    uae_u32 *m;
    addr -= a3000mem_start & a3000mem_mask;
    addr &= a3000mem_mask;
    m = (uae_u32 *)(a3000memory + addr);
    do_put_mem_long (m, SWAP_L(l));
}

void REGPARAM2 a3000mem_wput (uaecptr addr, uae_u32 w)
{
#ifdef DEBUG_MEMORY
    dbgf("a3000mem_wput 0x%X = 0x%X\n",addr,w);
#endif
    uae_u16 *m;
    addr -= a3000mem_start & a3000mem_mask;
    addr &= a3000mem_mask;
    m = (uae_u16 *)(a3000memory + addr);
    do_put_mem_word (m, SWAP_W(w));
}

void REGPARAM2 a3000mem_bput (uaecptr addr, uae_u32 b)
{
#ifdef DEBUG_MEMORY
    dbgf("a3000mem_bput 0x%X = 0x%X\n",addr,b);
#endif
    addr -= a3000mem_start & a3000mem_mask;
    addr &= a3000mem_mask;
    a3000memory[addr] = b;
}

int REGPARAM2 a3000mem_check (uaecptr addr, uae_u32 size)
{
    addr -= a3000mem_start & a3000mem_mask;
    addr &= a3000mem_mask;
    return (addr + size) <= allocated_a3000mem;
}

uae_u8 REGPARAM2 *a3000mem_xlate (uaecptr addr)
{
    addr -= a3000mem_start & a3000mem_mask;
    addr &= a3000mem_mask;
    return a3000memory + addr;
}

/* Kick memory */

uae_u8 *kickmemory;

static unsigned kickmem_checksum=0;
static unsigned get_kickmem_checksum(void)
{
	unsigned *p=(unsigned *)kickmemory;
	unsigned ret=0;
	if (p)
	{
		unsigned max=kickmem_size/4;
		unsigned i;
		for(i=0;i<max;i++)
			ret+=(i+1)*p[i];
	}
	return ret;
}

/*
 * A1000 kickstart RAM handling
 *
 * RESET instruction unhides boot ROM and disables write protection
 * write access to boot ROM hides boot ROM and enables write protection
 *
 */
static int a1000_kickstart_mode;
static uae_u8 *a1000_bootrom;
static void a1000_handle_kickstart (int mode)
{
    if (mode == 0) {
	a1000_kickstart_mode = 0;
	memcpy (kickmemory, kickmemory + 262144, 262144);
    } else {
	a1000_kickstart_mode = 1;
	memset (kickmemory, 0, 262144);
	memcpy (kickmemory, a1000_bootrom, 8192);
	memcpy (kickmemory + 131072, a1000_bootrom, 8192);
    }
}

static uae_u32 kickmem_lget (uaecptr) REGPARAM;
static uae_u32 kickmem_wget (uaecptr) REGPARAM;
static uae_u32 kickmem_bget (uaecptr) REGPARAM;
static void kickmem_lput (uaecptr, uae_u32) REGPARAM;
static void kickmem_wput (uaecptr, uae_u32) REGPARAM;
static void kickmem_bput (uaecptr, uae_u32) REGPARAM;
static int kickmem_check (uaecptr addr, uae_u32 size) REGPARAM;
static uae_u8 *kickmem_xlate (uaecptr addr) REGPARAM;

uae_u32 REGPARAM2 kickmem_lget (uaecptr addr)
{
    uae_u32 *m;
    addr -= kickmem_start & kickmem_mask;
    addr &= kickmem_mask;
    m = (uae_u32 *)(kickmemory + addr);
    return swab_l(do_get_mem_long (m));
}

uae_u32 REGPARAM2 kickmem_wget (uaecptr addr)
{
    uae_u16 *m;
    addr -= kickmem_start & kickmem_mask;
    addr &= kickmem_mask;
    m = (uae_u16 *)(kickmemory + addr);
    return swab_w(do_get_mem_word (m));
}

uae_u32 REGPARAM2 kickmem_bget (uaecptr addr)
{
    addr -= kickmem_start & kickmem_mask;
    addr &= kickmem_mask;
    return kickmemory[addr];
}

void REGPARAM2 kickmem_lput (uaecptr addr, uae_u32 b)
{
    uae_u32 *m;
    if (a1000_kickstart_mode) {
	if (addr >= 0xfc0000) {
	    addr -= kickmem_start & kickmem_mask;
	    addr &= kickmem_mask;
	    m = (uae_u32 *)(kickmemory + addr);
	    do_put_mem_long (m, swab_l(b));
	    return;
	} else
	    a1000_handle_kickstart (0);
    }
}

void REGPARAM2 kickmem_wput (uaecptr addr, uae_u32 b)
{
    uae_u16 *m;
    if (a1000_kickstart_mode) {
	if (addr >= 0xfc0000) {
	    addr -= kickmem_start & kickmem_mask;
	    addr &= kickmem_mask;
	    m = (uae_u16 *)(kickmemory + addr);
	    do_put_mem_word (m, swab_w(b));
	    return;
	} else
	    a1000_handle_kickstart (0);
    }
}

void REGPARAM2 kickmem_bput (uaecptr addr, uae_u32 b)
{
    if (a1000_kickstart_mode) {
	if (addr >= 0xfc0000) {
	    addr -= kickmem_start & kickmem_mask;
	    addr &= kickmem_mask;
	    kickmemory[addr] = b;
	    return;
	} else
	    a1000_handle_kickstart (0);
    }
}

int REGPARAM2 kickmem_check (uaecptr addr, uae_u32 size)
{
    addr -= kickmem_start & kickmem_mask;
    addr &= kickmem_mask;
    return (addr + size) <= kickmem_size;
}

uae_u8 REGPARAM2 *kickmem_xlate (uaecptr addr)
{
    addr -= kickmem_start & kickmem_mask;
    addr &= kickmem_mask;
    return kickmemory + addr;
}

/* CD32/CDTV extended kick memory */

uae_u8 *extendedkickmemory;
static int extendedkickmem_size;
static uae_u32 extendedkickmem_start;

#define EXTENDED_ROM_CD32 1
#define EXTENDED_ROM_CDTV 2

static int extromtype (void)
{
    switch (extendedkickmem_size) {
    case 524288:
	return EXTENDED_ROM_CD32;
    case 262144:
	return EXTENDED_ROM_CDTV;
    }
    return 0;
}

static uae_u32 extendedkickmem_lget (uaecptr) REGPARAM;
static uae_u32 extendedkickmem_wget (uaecptr) REGPARAM;
static uae_u32 extendedkickmem_bget (uaecptr) REGPARAM;
static void extendedkickmem_lput (uaecptr, uae_u32) REGPARAM;
static void extendedkickmem_wput (uaecptr, uae_u32) REGPARAM;
static void extendedkickmem_bput (uaecptr, uae_u32) REGPARAM;
static int extendedkickmem_check (uaecptr addr, uae_u32 size) REGPARAM;
static uae_u8 *extendedkickmem_xlate (uaecptr addr) REGPARAM;

uae_u32 REGPARAM2 extendedkickmem_lget (uaecptr addr)
{
#ifdef DEBUG_MEMORY
    dbgf("extendedkickmem_lget 0x%X\n",addr);
#endif
    uae_u32 *m;
    addr -= extendedkickmem_start & extendedkickmem_mask;
    addr &= extendedkickmem_mask;
    m = (uae_u32 *)(extendedkickmemory + addr);
    return swab_l(do_get_mem_long (m));
}

uae_u32 REGPARAM2 extendedkickmem_wget (uaecptr addr)
{
#ifdef DEBUG_MEMORY
    dbgf("extendedkickmem_wget 0x%X\n",addr);
#endif
    uae_u16 *m;
    addr -= extendedkickmem_start & extendedkickmem_mask;
    addr &= extendedkickmem_mask;
    m = (uae_u16 *)(extendedkickmemory + addr);
    return swab_w(do_get_mem_word (m));
}

uae_u32 REGPARAM2 extendedkickmem_bget (uaecptr addr)
{
#ifdef DEBUG_MEMORY
    dbgf("extendedkickmem_bget 0x%X\n",addr);
#endif
    addr -= extendedkickmem_start & extendedkickmem_mask;
    addr &= extendedkickmem_mask;
    return extendedkickmemory[addr];
}

void REGPARAM2 extendedkickmem_lput (uaecptr addr, uae_u32 b)
{
#ifdef DEBUG_MEMORY
    dbgf("extendedkickmem_lput 0x%X = 0x%X\n",addr,b);
#endif
}

void REGPARAM2 extendedkickmem_wput (uaecptr addr, uae_u32 b)
{
#ifdef DEBUG_MEMORY
    dbgf("extendedkickmem_wput 0x%X = 0x%X\n",addr,b);
#endif
}

void REGPARAM2 extendedkickmem_bput (uaecptr addr, uae_u32 b)
{
#ifdef DEBUG_MEMORY
    dbgf("extendedkickmem_bput 0x%X = 0x%X\n",addr,b);
#endif
}

int REGPARAM2 extendedkickmem_check (uaecptr addr, uae_u32 size)
{
    addr -= extendedkickmem_start & extendedkickmem_mask;
    addr &= extendedkickmem_mask;
    return (addr + size) <= extendedkickmem_size;
}

uae_u8 REGPARAM2 *extendedkickmem_xlate (uaecptr addr)
{
    addr -= extendedkickmem_start & extendedkickmem_mask;
    addr &= extendedkickmem_mask;
    return extendedkickmemory + addr;
}

/* Default memory access functions */

int REGPARAM2 default_check (uaecptr a, uae_u32 b)
{
    return 0;
}

uae_u8 REGPARAM2 *default_xlate (uaecptr a)
{
    write_log ("Your Amiga program just did something terribly stupid\n");
    uae_reset ();
    return kickmem_xlate (get_long (0xF80000));	/* So we don't crash. */
}

/* Address banks */

addrbank dummy_bank = {
    dummy_lget, dummy_wget, dummy_bget,
    dummy_lput, dummy_wput, dummy_bput,
    default_xlate, dummy_check, NULL
};

addrbank mbres_bank = {
    mbres_lget, mbres_wget, mbres_bget,
    mbres_lput, mbres_wput, mbres_bput,
    default_xlate, mbres_check, NULL
};

addrbank chipmem_bank = {
    chipmem_lget, chipmem_wget, chipmem_bget,
    chipmem_lput, chipmem_wput, chipmem_bput,
    chipmem_xlate, chipmem_check, NULL
};

addrbank bogomem_bank = {
    bogomem_lget, bogomem_wget, bogomem_bget,
    bogomem_lput, bogomem_wput, bogomem_bput,
    bogomem_xlate, bogomem_check, NULL
};

addrbank a3000mem_bank = {
    a3000mem_lget, a3000mem_wget, a3000mem_bget,
    a3000mem_lput, a3000mem_wput, a3000mem_bput,
    a3000mem_xlate, a3000mem_check, NULL
};

addrbank kickmem_bank = {
    kickmem_lget, kickmem_wget, kickmem_bget,
    kickmem_lput, kickmem_wput, kickmem_bput,
    kickmem_xlate, kickmem_check, NULL
};

addrbank extendedkickmem_bank = {
    extendedkickmem_lget, extendedkickmem_wget, extendedkickmem_bget,
    extendedkickmem_lput, extendedkickmem_wput, extendedkickmem_bput,
    extendedkickmem_xlate, extendedkickmem_check, NULL
};

static int decode_cloanto_rom (uae_u8 *mem, int size, int real_size)
{
	return 0;
}

static int kickstart_checksum (uae_u8 *mem, int size)
{
    uae_u32 cksum = 0, prevck = 0;
    int i;
    for (i = 0; i < size; i += 4) {
	uae_u32 data = mem[i] * 65536 * 256 + mem[i + 1] * 65536 + mem[i + 2] * 256 + mem[i + 3];
	cksum += data;
	if (cksum < prevck)
	    cksum++;
	prevck = cksum;
    }
    if (cksum != 0xFFFFFFFFul) {
	write_log ("Kickstart checksum incorrect. You probably have a corrupted ROM image.\n");
    }
    return 0;
}

static int read_kickstart (FILE *f, uae_u8 *mem, int size, int dochecksum, int *cloanto_rom)
{
    unsigned char buffer[20];
    int i, cr = 0;

    if (cloanto_rom)
	*cloanto_rom = 0;
    i = uae4all_rom_fread (buffer, 1, 11, f);
    if (strncmp ((char *) buffer, "AMIROMTYPE1", 11) != 0) {
	uae4all_rom_fseek (f, 0, SEEK_SET);
    } else {
	cr = 1;
    }

    i = uae4all_rom_fread (mem, 1, size, f);
    if (i == 8192) {
	a1000_bootrom = (uae_u8*)xmalloc (8192);
	memcpy (a1000_bootrom, kickmemory, 8192);
	a1000_handle_kickstart (1);
    } else if (i == size / 2) {
	memcpy (mem + size / 2, mem, i);
    } else if (i != size) {
	write_log ("Error while reading Kickstart.\n");
	uae4all_rom_fclose (f);
	return 0;
    }
    uae4all_rom_fclose (f);

    if (cr)
	decode_cloanto_rom (mem, size, i);
    if (dochecksum && i >= 262144)
	kickstart_checksum (mem, size);
    if (cloanto_rom)
	*cloanto_rom = cr;
    return 1;
}

static int load_extendedkickstart (void)
{
	return 0;
}

static void swab_kickstart(void)
{
#ifdef USE_FAME_CORE
	unsigned i;
	unsigned char *km=kickmemory;
	for(i=0;i<kickmem_size;i+=2)
	{
		unsigned char b1=km[i];
		km[i]=km[i+1];
		km[i+1]=b1;

	}
#endif
}

static int load_kickstart (void)
{
    FILE *f = uae4all_rom_fopen(romfile, "rb");
#ifdef DREAMCAST
    if (!f)
    	f = uae4all_rom_fopen(romfile_sd, "rb");
#endif

    if (f == NULL) {
#if defined(AMIGA)||defined(__POS__)
#define USE_UAE_ERSATZ "USE_UAE_ERSATZ"
	if (!getenv (USE_UAE_ERSATZ)) {
	    write_log ("Using current ROM. (create ENV:%s to " "use uae's ROM replacement)\n", USE_UAE_ERSATZ);
	    memcpy (kickmemory, (char *) 0x1000000 - kickmem_size, kickmem_size);
	    kickstart_checksum (kickmemory, kickmem_size);
	    goto chk_sum;
	}
#endif
	return 0;
    }

    if (!read_kickstart (f, kickmemory, kickmem_size, 1, &cloanto_rom))
	return 0;

    return 1;
}

char *address_space, *good_address_map;
int good_address_fd;

#ifndef NATMEM_OFFSET

uae_u8 *mapped_malloc (size_t s, const char *file)
{
    return (uae_u8 *)xmalloc (s);
}

void mapped_free (uae_u8 *p)
{
    free (p);
}
#else

#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/mman.h>

shmpiece *shm_start = NULL;
int canbang = 1;

static void dumplist (void)
{
    shmpiece *x = shm_start;
#ifdef DEBUG_MEMORY
    dbgf ("Start Dump:\n");
#endif
    while (x) {
#ifdef DEBUG_MEMORY
	dbgf ("  this=%p, Native %p, id %d, prev=%p, next=%p, size=0x%08x\n",
		x, x->native_address, x->id, x->prev, x->next, x->size);
#endif
	x = x->next;
    }
#ifdef DEBUG_MEMORY
    dbgf ("End Dump:\n");
#endif
}

static shmpiece *find_shmpiece (uae_u8 *base)
{
    shmpiece *x = shm_start;

    while (x && x->native_address != base)
	x = x->next;
    if (!x) {
#ifdef DEBUG_MEMORY
	dbgf ("NATMEM: Failure to find mapping at %p\n", base);
#endif
	dumplist ();
	canbang = 0;
	return 0;
    }
    return x;
}

static void delete_shmmaps (uae_u32 start, uae_u32 size)
{
    if (!canbang)
	return;

    while (size) {
	uae_u8 *base = mem_banks[bankindex (start)]->baseaddr;
	if (base) {
	    shmpiece *x;
	    base = ((uae_u8 *) NATMEM_OFFSET) + start;

	    x = find_shmpiece (base);
	    if (!x)
		return;

	    if (x->size > size) {
#ifdef DEBUG_MEMORY
		dbgf ("NATMEM: Failure to delete mapping at %08x(size %08x, delsize %08x)\n", start, x->size, size);
#endif
		dumplist ();
		canbang = 0;
		return;
	    }
	    shmdt (x->native_address);
	    size -= x->size;
	    start += x->size;
	    if (x->next)
		x->next->prev = x->prev;	/* remove this one from the list */
	    if (x->prev)
		x->prev->next = x->next;
	    else
		shm_start = x->next;
	    free (x);
	} else {
	    size -= 0x10000;
	    start += 0x10000;
	}
    }
}

static void add_shmmaps (uae_u32 start, addrbank *what)
{
    shmpiece *x = shm_start;
    shmpiece *y;
    uae_u8 *base = what->baseaddr;

    if (!canbang)
	return;
    if (!base)
	return;

    x = find_shmpiece (base);
    if (!x)
	return;
    y = xmalloc (sizeof (shmpiece));
    *y = *x;
    base = ((uae_u8 *) NATMEM_OFFSET) + start;
    y->native_address = shmat (y->id, base, 0);
    if (y->native_address == (void *) -1) {
#ifdef DEBUG_MEMORY
	dbgf ("NATMEM: Failure to map existing at %08x(%p)\n", start, base);
#endif
	perror ("shmat");
	dumplist ();
	canbang = 0;
	return;
    }
    y->next = shm_start;
    y->prev = NULL;
    if (y->next)
	y->next->prev = y;
    shm_start = y;
}

uae_u8 *mapped_malloc (size_t s, const char *file)
{
    int id;
    void *answer;
    shmpiece *x;

    if (!canbang)
	return xmalloc (s);

    id = shmget (IPC_PRIVATE, s, 0x1ff, file);
    if (id == 1) {
	canbang = 0;
	return mapped_malloc (s, file);
    }
    answer = shmat (id, 0, 0);
    shmctl (id, IPC_RMID, NULL);
    if (answer != (void *) -1) {
	x = xmalloc (sizeof (shmpiece));
	x->native_address = answer;
	x->id = id;
	x->size = s;
	x->next = shm_start;
	x->prev = NULL;
	if (x->next)
	    x->next->prev = x;
	shm_start = x;

	return answer;
    }
    canbang = 0;
    return mapped_malloc (s, file);
}

void mapped_free (uae_u8 *base)
{
    shmpiece *x = find_shmpiece (base);
    if (!x)
	return;
    shmdt (x->native_address);
}

#endif

static void init_mem_banks (void)
{
    int i;
    for (i = 0; i < 65536; i++)
	put_mem_bank (i << 16, &dummy_bank, 0);
#ifdef USE_FAME_CORE
    if (!savestate_state)
 	   init_memmaps(&dummy_bank);
#endif
}

static void allocate_memory (void)
{
    if (allocated_chipmem != prefs_chipmem_size) {
	if (chipmemory)
	    mapped_free (chipmemory);
	chipmemory = 0;

	allocated_chipmem = prefs_chipmem_size;
	chipmem_mask = allocated_chipmem - 1;

	chipmemory = (uae_u8 *)mapped_malloc (allocated_chipmem, "chip");

	if (chipmemory == 0) {
	    write_log ("Fatal error: out of memory for chipmem.\n");
	    allocated_chipmem = 0;
	} else
	    do_put_mem_long ((uae_u32 *)(chipmemory + 4), swab_l(0));
    }

    if (savestate_state == STATE_RESTORE)
    {
	    fseek (savestate_file, chip_filepos, SEEK_SET);
#ifndef DREAMCAST
	    void *tmp=malloc(compressed_size);
#else
	    extern void *uae4all_vram_memory_free;
	    void *tmp=uae4all_vram_memory_free;
#endif
	    int outSize=allocated_chipmem;
	    int inSize=compressed_size;
	    int res;
	    fread (tmp, 1, compressed_size, savestate_file);
#ifdef USE_LIB7Z
	    res=Lzma_Decode((Byte *)chipmemory, (size_t *)&outSize, (const Byte *)tmp, (size_t *)&inSize);
#else
	    res=uncompress((Bytef *)chipmemory,(uLongf*)&outSize,(const Bytef *)tmp,inSize);
#endif
#ifndef DREAMCAST
	    free(tmp);
#endif
#ifdef USE_LIB7Z
	    if(res != SZ_OK)
#else
	    if(res < 0 )
#endif
	    {
		   allocated_chipmem=compressed_size;
	    	   fseek (savestate_file, chip_filepos, SEEK_SET);
	    }
#ifdef USE_LIB7Z
	    if(res != SZ_OK)
#else
	    if(res < 0 )
#endif
	    fread (chipmemory, 1, allocated_chipmem, savestate_file);
	    if (allocated_bogomem > 0)
	    {
		    fseek (savestate_file, bogo_filepos, SEEK_SET);
		    fread (bogomemory, 1, allocated_bogomem, savestate_file);
	    }
    }

    chipmem_bank.baseaddr = chipmemory;
    bogomem_bank.baseaddr = bogomemory;
    chipmemory_word=(uae_u16 *)chipmemory;
}

static void reload_kickstart(void)
{
    load_extendedkickstart ();
    if (!load_kickstart ()) {
	init_ersatz_rom (kickmemory);
	ersatzkickfile = 1;
    }
    swab_kickstart();
    kickmem_checksum=get_kickmem_checksum();
}

void memory_reset (void)
{
    int i, custom_start;

#ifdef DEBUG_MEMORY
    dbg("memory_reset!");
#endif
#ifdef NATMEM_OFFSET
    delete_shmmaps (0, 0xFFFF0000);
#endif
    init_mem_banks ();

    memset(chipmemory,0,allocated_chipmem);
#ifdef USE_FAME_CORE
    clear_fame_mem_dummy();
#endif
    rtarea_cleanup();

    allocate_memory ();

    /* Map the chipmem into all of the lower 8MB */
    i = allocated_chipmem > 0x200000 ? (allocated_chipmem >> 16) : 32;
#ifdef DEBUG_MEMORY
    dbg("map_banks : chipmem_bank");
#endif
    map_banks (&chipmem_bank, 0x00, i, allocated_chipmem);

    custom_start = 0xC0;

#ifdef DEBUG_MEMORY
    dbg("map_banks : custom_bank");
#endif
    map_banks (&custom_bank, custom_start, 0xE0 - custom_start, 0);
#ifdef DEBUG_MEMORY
    dbg("map_banks : cia_bank");
#endif
    map_banks (&cia_bank, 0xA0, 32, 0);
#ifdef DEBUG_MEMORY
    dbg("map_banks : clock_bank");
#endif
    map_banks (&clock_bank, 0xDC, 1, 0);

    /* @@@ Does anyone have a clue what should be in the 0x200000 - 0xA00000
     * range on an Amiga without expansion memory?  */
    custom_start = allocated_chipmem >> 16;
    if (custom_start < 0x20)
	custom_start = 0x20;
#ifdef DEBUG_MEMORY
    dbg("map_banks : dummy_bank");
#endif
    map_banks (&dummy_bank, custom_start, 0xA0 - custom_start, 0);
    /*map_banks (&mbres_bank, 0xDE, 1); */

    if (bogomemory != 0) {
	int t = allocated_bogomem >> 16;
	if (t > 0x1C)
	    t = 0x1C;
#ifdef DEBUG_MEMORY
    dbg("map_banks : bogomem_bank");
#endif
	map_banks (&bogomem_bank, 0xC0, t, allocated_bogomem);
    }
    if (a3000memory != 0)
    {
#ifdef DEBUG_MEMORY
	dbg("map_banks : a3000mem_bank");
#endif
	map_banks (&a3000mem_bank, a3000mem_start >> 16, allocated_a3000mem >> 16, allocated_a3000mem);
    }

#ifdef DEBUG_MEMORY
    dbg("map_banks : rtarea_bank");
#endif
    map_banks (&rtarea_bank, RTAREA_BASE >> 16, 1, 0);

#ifdef DEBUG_MEMORY
    dbg("map_banks : kickmem_bank");
#endif
    map_banks (&kickmem_bank, 0xF8, 8, 0);
    if (a1000_bootrom)
	a1000_handle_kickstart (1);

    switch (extromtype ()) {
    case EXTENDED_ROM_CDTV:
#ifdef DEBUG_MEMORY
	dbg("map_banks : extendedkickmem_bank");
#endif
	map_banks (&extendedkickmem_bank, 0xF0, 4, 0);
	break;
    case EXTENDED_ROM_CD32:
#ifdef DEBUG_MEMORY
	dbg("map_banks : extendedkickmem_bank");
#endif
	map_banks (&extendedkickmem_bank, 0xE0, 8, 0);
	break;
    default:
	if (cloanto_rom)
	{
#ifdef DEBUG_MEMORY
	    dbg("map_banks : kickmem_bank");
#endif
	    map_banks (&kickmem_bank, 0xE0, 8, 0);
	}
    }
    if (kickmem_checksum!=get_kickmem_checksum())
    {
	unsigned chksum=kickmem_checksum;
	reload_kickstart();
	if (chksum!=kickmem_checksum)
	{
		uae4all_rom_reinit();
		reload_kickstart();
	}
    }
}


void memory_init (void)
{
#ifdef DEBUG_MEMORY
    dbg("memory_init!");
#endif
    allocated_chipmem = 0;
    allocated_bogomem = 0;
    allocated_a3000mem = 0;
    kickmemory = 0;
    extendedkickmemory = 0;
    chipmemory = 0;
    a3000memory = 0;
    bogomemory = 0;

    kickmemory = mapped_malloc (kickmem_size, "kick");
    kickmem_bank.baseaddr = kickmemory;

    reload_kickstart();

    init_mem_banks ();
    memory_reset ();

    kickmem_mask = kickmem_size - 1;
    extendedkickmem_mask = extendedkickmem_size - 1;
}

void memory_cleanup (void)
{
    if (a3000memory)
	mapped_free (a3000memory);
    if (bogomemory)
	mapped_free (bogomemory);
    if (kickmemory)
	mapped_free (kickmemory);
    if (a1000_bootrom)
	free (a1000_bootrom);
    if (chipmemory)
	mapped_free (chipmemory);

    a3000memory = 0;
    bogomemory = 0;
    kickmemory = 0;
    a1000_bootrom = 0;
    chipmemory = 0;
}

void map_banks (addrbank *bank, int start, int size, int realsize)
{
    int bnr;
    unsigned long int hioffs = 0, endhioffs = 0x100;
    addrbank *orgbank = bank;
    uae_u32 realstart = start;

#ifdef DEBUG_MEMORY
    dbg("Map");
#endif
    flush_icache (1);		/* Sure don't want to keep any old mappings around! */
#ifdef NATMEM_OFFSET
    delete_shmmaps (start << 16, size << 16);
#endif

    if (!realsize)
	realsize = size << 16;

    if ((size << 16) < realsize) {
	write_log ("Please report to bmeyer@cs.monash.edu.au, and mention:\n");
	write_log ("Broken mapping, size=%x, realsize=%x\n", size, realsize);
	write_log ("Start is %x\n", start);
	write_log ("Reducing memory sizes, especially chipmem, may fix this problem\n");
	return;
    }

    if (start >= 0x100) {
	int real_left = 0;
	for (bnr = start; bnr < start + size; bnr++) {
	    if (!real_left) {
		realstart = bnr;
		real_left = realsize >> 16;
#ifdef NATMEM_OFFSET
		add_shmmaps (realstart << 16, bank);
#endif
	    }
	    put_mem_bank (bnr << 16, bank, realstart << 16);
#ifdef USE_FAME_CORE
	    map_zone(bnr,bank,realstart);
#endif
	    real_left--;
	}
#ifdef DEBUG_MEMORY
	dbg("!Map");
#endif
	return;
    }
    for (hioffs = 0; hioffs < endhioffs; hioffs += 0x100) {
	int real_left = 0;
	for (bnr = start; bnr < start + size; bnr++) {
	    if (!real_left) {
		realstart = bnr + hioffs;
		real_left = realsize >> 16;
#ifdef NATMEM_OFFSET
		add_shmmaps (realstart << 16, bank);
#endif
	    }
	    put_mem_bank ((bnr + hioffs) << 16, bank, realstart << 16);
#ifdef USE_FAME_CORE
	    map_zone(bnr+hioffs,bank,realstart);
#endif
	    real_left--;
	}
    }
#ifdef DEBUG_MEMORY
    dbg("!Map!");
#endif
}


/* memory save/restore code */

uae_u8 *save_cram (int *len)
{
    *len = allocated_chipmem;
    return chipmemory;
}

uae_u8 *save_bram (int *len)
{
    *len = allocated_bogomem;
    return bogomemory;
}

void restore_cram (int len, long filepos)
{
    chip_filepos = filepos;
    compressed_size=len;
}

void restore_bram (int len, long filepos)
{
    bogo_filepos = filepos;
}

uae_u8 *restore_rom (uae_u8 *src)
{
    restore_u32 ();
    restore_u32 ();
    restore_u32 ();
    restore_u32 ();
    restore_u32 ();

    return src;
}

uae_u8 *save_rom (int first, int *len)
{
    static int count;
    uae_u8 *dst, *dstbak;
    uae_u8 *mem_real_start;
    int mem_start, mem_size, mem_type, i, saverom;

    saverom = 0;
    if (first)
	count = 0;
    for (;;) {
	mem_type = count;
	switch (count) {
	case 0:		/* Kickstart ROM */
	    mem_start = 0xf80000;
	    mem_real_start = kickmemory;
	    mem_size = kickmem_size;
	    /* 256KB or 512KB ROM? */
#ifndef DREAMCAST
	    for (i = 0; i < mem_size / 2 - 4; i++) {
		if (longget (i + mem_start) != longget (i + mem_start + mem_size / 2))
		    break;
	    }
	    if (i == mem_size / 2 - 4) {
		mem_size /= 2;
		mem_start += 262144;
	    }
#endif
	    mem_type = 0;
	    break;
	default:
	    return 0;
	}
	count++;
	if (mem_size)
	    break;
    }
#ifndef DREAMCAST
    dstbak = dst = (uae_u8 *)malloc (4 + 4 + 4 + 4 + 4 + mem_size);
#else
    extern void *uae4all_vram_memory_free;
    dstbak = dst = (uae_u8 *) uae4all_vram_memory_free;
#endif
    save_u32 (mem_start);
    save_u32 (mem_size);
    save_u32 (mem_type);
    save_u32 (longget (mem_start + 12));	/* version+revision */
    save_u32 (0);
    sprintf ((char *)dst, "Kickstart %d.%d", wordget (mem_start + 12), wordget (mem_start + 14));
    dst += strlen ((char *)dst) + 1;
    if (saverom) {
	for (i = 0; i < mem_size; i++)
	    *dst++ = byteget (mem_start + i);
    }
    *len = dst - dstbak;
    return dstbak;
}

uae_u8 *save_fram (int *len)
{
    *len = 0; //allocated_fastmem;
    return NULL;
}

uae_u8 *save_zram (int *len)
{
    *len = 0; //allocated_z3fastmem;
    return NULL;
}

void restore_fram (int len, long filepos)
{
}

void restore_zram (int len, long filepos)
{
}

uae_u8 *save_expansion (int *len)
{
    static uae_u8 t[20], *dst = t;
    save_u32 (0);
    save_u32 (0);
    *len = 8;
    return dst;
}

uae_u8 *restore_expansion (uae_u8 *src)
{
    restore_u32 ();
    restore_u32 ();
    return src;
}
