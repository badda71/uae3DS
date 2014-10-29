 /*
  * UAE - The Un*x Amiga Emulator
  *
  * memory management
  *
  * Copyright 1995 Bernd Schmidt
  */
#ifndef UAE_MEMORY_H
#define UAE_MEMORY_H

#if defined(WIN32)||defined(DINGOO)
#define bzero(BUF,SIZ) memset(BUF,0,SIZ)
#endif


extern void memory_reset (void);
extern unsigned chipmem_checksum(void);

extern int special_mem;
#define S_READ 1
#define S_WRITE 2


typedef uae_u32 (*mem_get_func)(uaecptr) REGPARAM;
typedef void (*mem_put_func)(uaecptr, uae_u32) REGPARAM;
typedef uae_u8 *(*xlate_func)(uaecptr) REGPARAM;
typedef int (*check_func)(uaecptr, uae_u32) REGPARAM;

extern char *address_space, *good_address_map;
extern uae_u8 *chipmemory;
extern uae_u16 *chipmemory_word;

extern uae_u32 allocated_chipmem;
extern uae_u32 allocated_fastmem;
extern uae_u32 allocated_bogomem;
extern uae_u32 allocated_gfxmem;
extern uae_u32 allocated_z3fastmem;
extern uae_u32 allocated_a3000mem;

#undef DIRECT_MEMFUNCS_SUCCESSFUL
#include "maccess.h"

#ifndef CAN_MAP_MEMORY
#undef USE_COMPILER
#endif

#if defined(USE_COMPILER) && !defined(USE_MAPPED_MEMORY)
#define USE_MAPPED_MEMORY
#endif

#define kickmem_size 0x080000

#define chipmem_start 0x00000000
#define bogomem_start 0x00C00000
#define a3000mem_start 0x07000000
#define kickmem_start 0x00F80000

extern int ersatzkickfile;
extern int cloanto_rom;

extern uae_u8* baseaddr[];

typedef struct {
    /* These ones should be self-explanatory... */
    mem_get_func lget, wget, bget;
    mem_put_func lput, wput, bput;
    /* Use xlateaddr to translate an Amiga address to a uae_u8 * that can
     * be used to address memory without calling the wget/wput functions.
     * This doesn't work for all memory banks, so this function may call
     * abort(). */
    xlate_func xlateaddr;
    /* To prevent calls to abort(), use check before calling xlateaddr.
     * It checks not only that the memory bank can do xlateaddr, but also
     * that the pointer points to an area of at least the specified size.
     * This is used for example to translate bitplane pointers in custom.c */
    check_func check;
    /* For those banks that refer to real memory, we can save the whole trouble
       of going through function calls, and instead simply grab the memory
       ourselves. This holds the memory address where the start of memory is
       for this particular bank. */
    uae_u8 *baseaddr;
} addrbank;

extern uae_u8 *filesysory;
extern uae_u8 *rtarea;

extern addrbank chipmem_bank;
extern addrbank kickmem_bank;
extern addrbank custom_bank;
extern addrbank clock_bank;
extern addrbank cia_bank;
extern addrbank rtarea_bank;
extern addrbank expamem_bank;
extern addrbank fastmem_bank;
extern addrbank gfxmem_bank;

extern void rtarea_init (void);
extern void rtarea_setup (void);
extern void expamem_init (void);
extern void expamem_reset (void);
extern void rtarea_cleanup (void);

extern uae_u32 gfxmem_start;
extern uae_u8 *gfxmemory;
extern uae_u32 gfxmem_mask;

/* Default memory access functions */

extern int default_check(uaecptr addr, uae_u32 size) REGPARAM;
extern uae_u8 *default_xlate(uaecptr addr) REGPARAM;

#define bankindex(addr) (((uaecptr)(addr)) >> 16)

extern addrbank *mem_banks[65536];
extern uae_u8 *baseaddr[65536];
#define get_mem_bank(addr) (*mem_banks[bankindex(addr)])
#define put_mem_bank(addr, b, realstart) \
do { \
    (mem_banks[bankindex(addr)] = (b)); \
    if ((b)->baseaddr) \
        baseaddr[bankindex(addr)] = (b)->baseaddr - (realstart); \
    else \
        baseaddr[bankindex(addr)] = (uae_u8*)(((long)b)+1); \
} while (0)

extern void memory_init (void);
extern void memory_cleanup (void);
extern void map_banks (addrbank *bank, int first, int count, int realsize);

#ifndef NO_INLINE_MEMORY_ACCESS

#define longget(addr) (call_mem_get_func(get_mem_bank(addr).lget, addr))
#define wordget(addr) (call_mem_get_func(get_mem_bank(addr).wget, addr))
#define byteget(addr) (call_mem_get_func(get_mem_bank(addr).bget, addr))
#define longput(addr,l) (call_mem_put_func(get_mem_bank(addr).lput, addr, l))
#define wordput(addr,w) (call_mem_put_func(get_mem_bank(addr).wput, addr, w))
#define byteput(addr,b) (call_mem_put_func(get_mem_bank(addr).bput, addr, b))

#else

extern uae_u32 alongget(uaecptr addr);
extern uae_u32 awordget(uaecptr addr);
extern uae_u32 longget(uaecptr addr);
extern uae_u32 wordget(uaecptr addr);
extern uae_u32 byteget(uaecptr addr);
extern void longput(uaecptr addr, uae_u32 l);
extern void wordput(uaecptr addr, uae_u32 w);
extern void byteput(uaecptr addr, uae_u32 b);

#endif

#ifndef MD_HAVE_MEM_1_FUNCS

#define longget_1 longget
#define wordget_1 wordget
#define byteget_1 byteget
#define longput_1 longput
#define wordput_1 wordput
#define byteput_1 byteput

#endif

static __inline__ uae_u32 get_long(uaecptr addr)
{
    return longget_1(addr);
}
static __inline__ uae_u32 get_word(uaecptr addr)
{
    return wordget_1(addr);
}
static __inline__ uae_u32 get_byte(uaecptr addr)
{
    return byteget_1(addr);
}
static __inline__ void put_long(uaecptr addr, uae_u32 l)
{
    longput_1(addr, l);
}
static __inline__ void put_word(uaecptr addr, uae_u32 w)
{
    wordput_1(addr, w);
}
static __inline__ void put_byte(uaecptr addr, uae_u32 b)
{
    byteput_1(addr, b);
}

static __inline__ uae_u8 * get_real_address(uaecptr addr)
{
    return get_mem_bank(addr).xlateaddr(addr);
}

static __inline__ int valid_address(uaecptr addr, uae_u32 size)
{
    return get_mem_bank(addr).check(addr, size);
}

/* For faster access in custom chip emulation.  */
extern uae_u32 chipmem_lget (uaecptr) REGPARAM;
extern uae_u32 chipmem_wget (uaecptr) REGPARAM;
extern uae_u32 chipmem_bget (uaecptr) REGPARAM;
extern void chipmem_lput (uaecptr, uae_u32) REGPARAM;
extern void chipmem_wput (uaecptr, uae_u32) REGPARAM;
extern void chipmem_bput (uaecptr, uae_u32) REGPARAM;

#if !defined(USE_FAME_CORE) || defined(UAE_MEMORY_ACCESS)
#define CHIPMEM_WGET(PT) chipmem_wget(PT)
#define CHIPMEM_WPUT(PT,DA) chipmem_wput(PT,DA)
#else
#ifdef ERROR_WHEN_MEMORY_OVERRUN
static __inline__ unsigned short CHIPMEM_WGET(unsigned addr)
{
	if (addr>0x000FFFFF)
	{
		printf("CHIPMEM_WGET ERROR: DIRECCION 0x.8X INVALIDA !!!\n",addr);
		exit(1);
	}
	return (*((unsigned short *)&chipmemory[(unsigned)addr]));
}

static __inline__ void CHIPMEM_WPUT(unsigned addr,unsigned data)
{
	if (addr>0x000FFFFF)
	{
		printf("CHIPMEM_WPUT ERROR: DIRECCION 0x.8X INVALIDA !!!\n",addr);
		exit(1);
	}
	((*((unsigned short *)&chipmemory[(unsigned)addr]))=data);
}

#else
#ifndef SAFE_MEMORY_ACCESS
#define CHIPMEM_WGET(PT) (*((unsigned short *)&chipmemory[(unsigned)PT]))
#define CHIPMEM_WPUT(PT,DA) ((*((unsigned short *)&chipmemory[(unsigned)PT]))=DA)
#else
#define CHIPMEM_WGET(PT) (*((unsigned short *)&chipmemory[((unsigned)PT)&0x000FFFFF]))
#define CHIPMEM_WPUT(PT,DA) ((*((unsigned short *)&chipmemory[((unsigned)PT)&0x000FFFFF]))=DA)
#endif
#endif
#endif

#ifdef NATMEM_OFFSET

typedef struct shmpiece_reg {
    uae_u8 *native_address;
    int id;
    uae_u32 size;
    struct shmpiece_reg *next;
    struct shmpiece_reg *prev;
} shmpiece;

extern shmpiece *shm_start;
extern int canbang;

#endif

extern uae_u8 *mapped_malloc (size_t, const char *);
extern void mapped_free (uae_u8 *);

#if defined(DREAMCAST) && defined(DC_SQ)

#define UAE4ALL_ALIGN __attribute__ ((__aligned__ (32)))

#define uae4all_no_memcpy memcpy
#define uae4all_no_memclr bzero


static __inline__ void uae4all_memcpy(void *_GCCRES_ dest, void *_GCCRES_ src, int n)
{

	register unsigned int *d = (unsigned int *)(void *)
		(0xe0000000 | (((unsigned long)dest) & 0x03ffffe0));
	register unsigned int *dreal=(unsigned int *)dest;
	register unsigned int *s = (unsigned int *)src;

	{
		register unsigned qa=((((unsigned int)dest)>>26)<<2)&0x1c;
		QACR0 = qa;
		QACR1 = qa;
	}

	n>>=5;
	while(n--)
	{
		asm("pref @%0" : : "r" (s + 8));
		d[0] = *(s++);
		d[1] = *(s++);
		d[2] = *(s++);
		d[3] = *(s++);
		d[4] = *(s++);
		d[5] = *(s++);
		d[6] = *(s++);
		d[7] = *(s++);
		asm("pref @%0" : : "r" (d));
		asm("ocbi @%0" : : "r" (dreal));
		d += 8;
		dreal += 8;
	}
	d = (unsigned int *)0xe0000000;
	d[0] = d[8] = 0;
}


static __inline__ void uae4all_memclr(void *_GCCRES_ dest, int n)
{
	register unsigned int *d = (unsigned int *)(void *)
		(0xe0000000 | (((unsigned long)dest) & 0x03ffffe0));

	{
		register unsigned qa=((((unsigned int)dest)>>26)<<2)&0x1c;
		QACR0 = qa;
		QACR1 = qa;
	}

	d[0]=d[1]=d[2]=d[3]=d[4]=d[5]=d[6]=d[7]=d[8]=d[9]=d[10]=d[11]=d[12]=d[13]=d[14]=d[15]=0;
	
	n>>=5;
	while(n--)
	{
		asm("pref @%0" : : "r" (d));
		d+=8;
	}
	d = (unsigned int *)0xe0000000;
	d[0] = d[8] = 0;
}

#else

#define UAE4ALL_ALIGN
#define uae4all_memcpy memcpy
#define uae4all_memclr bzero

#endif

#endif
