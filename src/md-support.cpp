 /* 
  * UAE - The Un*x Amiga Emulator
  * 
  * Miscellaneous machine dependent support functions and definitions
  *
  * Copyright 1996 Bernd Schmidt
  */

#include "sysconfig.h"
#include "sysdeps.h"

#include "config.h"
#include "uae.h"
#include "options.h"

#ifndef USE_FAME_CORE
#include "m68k/uae/m68k.h"

struct flag_struct regflags;
#endif

void machdep_init (void)
{
}
