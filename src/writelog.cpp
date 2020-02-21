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

static void vlog_citra(const char *format, va_list arg ) {
	char buf[2000];
    vsnprintf(buf, 2000, format, arg);
	svcOutputDebugString(buf, strlen(buf));
}

void log_citra(const char *format, ...)
{
    va_list argptr;
    va_start(argptr, format);
	vlog_citra(format, argptr);
    va_end(argptr);
}

void write_log_standard (const char *fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
    vprintf (fmt, ap);
	printf("\n");
	vlog_citra(fmt, ap);
	va_end (ap);
}

#endif

