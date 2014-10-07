 /* 
  * UAE - The Un*x Amiga Emulator
  * 
  * Memory access functions
  *
  * Copyright 1996 Bernd Schmidt
  */

#ifndef MACCESS_UAE_H
#define MACCESS_UAE_H

static __inline__ uae_u32 do_get_mem_long(uae_u32 *_GCCRES_ a)
{
#ifdef USE_FAME_CORE
    register uae_u16 *b=(uae_u16 *)a;
    register uae_u32 b1=b[0];
    b1<<=16;
    return (b1|b[1]);
#else
    uae_u8 *b = (uae_u8 *)a;
    
    return (*b << 24) | (*(b+1) << 16) | (*(b+2) << 8) | (*(b+3));
#endif
}

static __inline__ uae_u16 do_get_mem_word(uae_u16 *_GCCRES_ a)
{
#ifdef USE_FAME_CORE
    return (*a);
#else
    uae_u8 *b = (uae_u8 *)a;
    
    return (*b << 8) | (*(b+1));
#endif
}

static __inline__ uae_u8 do_get_mem_byte(uae_u8 *_GCCRES_ a)
{
/*
#ifdef USE_FAME_CORE
    a= (uae_u8 *)(((unsigned)a)^1);
#endif
*/
    return *a;
}

static __inline__ void do_put_mem_long(uae_u32 *_GCCRES_ a, uae_u32 v)
{
#ifdef USE_FAME_CORE
    register uae_u16 *b=(uae_u16 *)a;
    b[0]=(v>>16)&0xffff;
    b[1]=v&0xffff;
#else
    uae_u8 *b = (uae_u8 *)a;
    
    *b = v >> 24;
    *(b+1) = v >> 16;    
    *(b+2) = v >> 8;
    *(b+3) = v;
#endif
}

static __inline__ void do_put_mem_word(uae_u16 *_GCCRES_ a, uae_u16 v)
{
#ifdef USE_FAME_CORE
    (*a)=v;
#else
    uae_u8 *b = (uae_u8 *)a;
    
    *b = v >> 8;
    *(b+1) = v;
#endif
}

static __inline__ void do_put_mem_byte(uae_u8 *_GCCRES_ a, uae_u8 v)
{
/*
#ifdef USE_FAME_CORE
     a= (uae_u8 *)(((unsigned)a)^1);
#endif
*/
    *a = v;
}

#define call_mem_get_func(func, addr) ((*func)(addr))
#define call_mem_put_func(func, addr, v) ((*func)(addr, v))

#undef NO_INLINE_MEMORY_ACCESS
#undef MD_HAVE_MEM_1_FUNCS

#endif
