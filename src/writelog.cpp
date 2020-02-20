 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Standard write_log that writes to the console
  *
  * Copyright 2001 Bernd Schmidt
  */
#include "sysconfig.h"
#include "sysdeps.h"
#include <3ds.h>

#if defined(DEBUG_UAE4ALL) && defined(UAE_CONSOLE)

void log_citra(const char *format, ...)
{
	char buf[2000];

    va_list argptr;
    va_start(argptr, format);
    vsprintf(buf, format, argptr);
    va_end(argptr);
	svcOutputDebugString(buf, strlen(buf));
}

void write_log_standard (const char *fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
    vfprintf (stdout, fmt, ap);
    /* Technique stolen from GCC.  */
    {
	int x1, x2, x3, x4, x5, x6, x7, x8;
	x1 = va_arg (ap, int);
	x2 = va_arg (ap, int);
	x3 = va_arg (ap, int);
	x4 = va_arg (ap, int);
	x5 = va_arg (ap, int);
	x6 = va_arg (ap, int);
	x7 = va_arg (ap, int);
	x8 = va_arg (ap, int);
	log_citra(fmt, x1, x2, x3, x4, x5, x6, x7, x8);
    }
}

#endif

