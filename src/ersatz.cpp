 /*
  * UAE - The Un*x Amiga Emulator
  *
  * A "replacement" for a missing Kickstart
  * Warning! Q&D
  *
  * (c) 1995 Bernd Schmidt
  */

#include "sysconfig.h"
#include "sysdeps.h"

#include "config.h"
#include "uae.h"
#include "options.h"
#include "memory.h"
#include "custom.h"
#include "m68k/m68k_intrf.h"
#include "cia.h"
#include "disk.h"
#include "ersatz.h"

#define EOP_INIT     0
#define EOP_NIMP     1
#define EOP_SERVEINT 2
#define EOP_DOIO     3
#define EOP_OPENLIB  4
#define EOP_AVAILMEM 5
#define EOP_ALLOCMEM 6
#define EOP_ALLOCABS 7
#define EOP_LOOP 8

void init_ersatz_rom (uae_u8 *data)
{
    write_log ("Trying to use Kickstart replacement.\n");
    *data++ = 0x00; *data++ = 0x08; /* initial SP */
    *data++ = 0x00; *data++ = 0x00;
    *data++ = 0x00; *data++ = 0xF8; /* initial PC */
    *data++ = 0x00; *data++ = 0x08;

    *data++ = 0xFF; *data++ = 0x0D;
    *data++ = 0x00; *data++ = EOP_INIT;
    *data++ = 0xFF; *data++ = 0x0D;
    *data++ = 0x00; *data++ = EOP_NIMP;

    *data++ = 0xFF; *data++ = 0x0D;
    *data++ = 0x00; *data++ = EOP_LOOP;
    *data++ = 0xFF; *data++ = 0x0D;
    *data++ = 0x00; *data++ = EOP_DOIO;

    *data++ = 0x4E; *data++ = 0x75;
    *data++ = 0xFF; *data++ = 0x0D;
    *data++ = 0x00; *data++ = EOP_SERVEINT;
    *data++ = 0x4E; *data++ = 0x73;

    *data++ = 0xFF; *data++ = 0x0D;
    *data++ = 0x00; *data++ = EOP_AVAILMEM;
    *data++ = 0x4E; *data++ = 0x75;
    *data++ = 0xFF; *data++ = 0x0D;

    *data++ = 0x00; *data++ = EOP_ALLOCMEM;
    *data++ = 0x4E; *data++ = 0x75;
    *data++ = 0xFF; *data++ = 0x0D;
    *data++ = 0x00; *data++ = EOP_ALLOCABS;

    *data++ = 0x4E; *data++ = 0x75;
}

static void ersatz_doio (void)
{
    uaecptr request = _68k_areg(1);
    switch (get_word (request + 0x1C)) {
     case 9: /* TD_MOTOR is harmless */
     case 2: case 0x8002: /* READ commands */
	break;

     default:
	write_log ("Only CMD_READ supported in DoIO()\n");
	return;
    }
    {
	uaecptr dest = get_long (request + 0x28);
	int start = get_long (request + 0x2C) / 512;
	int nsecs = get_long (request + 0x24) / 512;
	int tr = start / 11;
	int sec = start % 11;
	while (nsecs--) {
	    DISK_ersatz_read (tr, sec, dest);
	    dest += 512;
	    if (++sec == 11)
		sec = 0, tr++;
	}
    }
}

static void ersatz_init (void)
{
    int f;
    uaecptr request;
    uaecptr a;

    if (disk_empty (0)) {
	write_log ("You need to have a diskfile in DF0 to use the Kickstart replacement!\n");
	uae_quit ();
	_68k_setpc (0xF80010);
	return;
    }

    _68k_sreg = 0;
    /* Set some interrupt vectors */
    for (a = 8; a < 0xC0; a += 4) {
	put_long (a, 0xF8001A);
    }
    _68k_ispreg = _68k_mspreg = _68k_uspreg = 0x800;
    _68k_areg(7) = 0x80000;
#ifndef USE_FAME_CORE
    _68k_intmask = 0;
#endif

    /* Build a dummy execbase */
    put_long (4, _68k_areg(6) = 0x676);
    put_byte (0x676 + 0x129, 0);
    for (f = 1; f < 105; f++) {
	put_word (0x676 - 6*f, 0x4EF9);
	put_long (0x676 - 6*f + 2, 0xF8000C);
    }
    /* Some "supported" functions */
    put_long (0x676 - 456 + 2, 0xF80014);
    put_long (0x676 - 216 + 2, 0xF80020);
    put_long (0x676 - 198 + 2, 0xF80026);
    put_long (0x676 - 204 + 2, 0xF8002c);
    put_long (0x676 - 210 + 2, 0xF8002a);

    /* Build an IORequest */
    request = 0x800;
    put_word (request + 0x1C, 2);
    put_long (request + 0x28, 0x4000);
    put_long (request + 0x2C, 0);
    put_long (request + 0x24, 0x200 * 4);
    _68k_areg(1) = request;
    ersatz_doio ();
    /* kickstart disk loader */
    if (get_long(0x4000) == 0x4b49434b) {
	/* a kickstart disk was found in drive 0! */
	write_log ("Loading Kickstart rom image from Kickstart disk\n");
	/* print some notes... */
	write_log ("NOTE: if UAE crashes set CPU to 68000 and/or chipmem size to 512KB!\n");

	/* read rom image from kickstart disk */
	put_word (request + 0x1C, 2);
	put_long (request + 0x28, 0xF80000);
	put_long (request + 0x2C, 0x200);
	put_long (request + 0x24, 0x200 * 512);
	_68k_areg(1) = request;
	ersatz_doio ();

	/* read rom image once ajain to mirror address space.
	   not elegant, but it works... */
	put_word (request + 0x1C, 2);
	put_long (request + 0x28, 0xFC0000);
	put_long (request + 0x2C, 0x200);
	put_long (request + 0x24, 0x200 * 512);
	_68k_areg(1) = request;
	ersatz_doio ();

	disk_eject (0);

	_68k_setpc (0xFC0002);
	fill_prefetch_0 ();
	uae_reset ();
	ersatzkickfile = 0;
	return;
    }

    _68k_setpc (0x400C);
    fill_prefetch_0 ();

    /* Init the hardware */
    put_long (0x3000, 0xFFFFFFFEul);
    put_long (0xDFF080, 0x3000);
    put_word (0xDFF088, 0);
    put_word (0xDFF096, 0xE390);
    put_word (0xDFF09A, 0xE02C);
    put_word (0xDFF09E, 0x0000);
    put_word (0xDFF092, 0x0038);
    put_word (0xDFF094, 0x00D0);
    put_word (0xDFF08E, 0x2C81);
    put_word (0xDFF090, 0xF4C1);
    put_word (0xDFF02A, 0x8000);

    put_byte (0xBFD100, 0xF7);
    put_byte (0xBFEE01, 0);
    put_byte (0xBFEF01, 0x08);
    put_byte (0xBFDE00, 0x04);
    put_byte (0xBFDF00, 0x84);
    put_byte (0xBFDD00, 0x9F);
    put_byte (0xBFED01, 0x9F);
}

void ersatz_perform (uae_u16 what)
{
    switch (what) {
     case EOP_INIT:
	ersatz_init ();
	break;

     case EOP_SERVEINT:
	/* Just reset all the interrupt request bits */
	put_word (0xDFF09C, get_word (0xDFF01E) & 0x3FFF);
	break;

     case EOP_DOIO:
	ersatz_doio ();
	break;

     case EOP_AVAILMEM:
	_68k_dreg(0) = _68k_dreg(1) & 4 ? 0 : 0x70000;
	break;

     case EOP_ALLOCMEM:
	_68k_dreg(0) = _68k_dreg(1) & 4 ? 0 : 0x0F000;
	break;

     case EOP_ALLOCABS:
	_68k_dreg(0) = _68k_areg(1);
	break;

     case EOP_NIMP:
	write_log ("Unimplemented Kickstart function called\n");
	uae_quit ();
	
	/* fall through */
     case EOP_LOOP:
	_68k_setpc (0xF80010);
	break;

     case EOP_OPENLIB:
     default:
	write_log ("Internal error. Giving up.\n");
	return;
    }
}
