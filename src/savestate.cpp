 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Save/restore emulator state
  *
  * (c) 1999-2001 Toni Wilen
  *
  * see below for ASF-structure
  */

 /* Features:
  *
  * - full CPU state (68000/68010/68020, no FPU)
  * - full CIA-A and CIA-B state (with all internal registers)
  * - saves all custom registers and audio internal state but not all registers are restored yet.
  * - only Chip-ram and Bogo-ram are saved and restored.
  * - disk drive type, imagefile, track and motor state
  * - Kickstart ROM version, address and size is saved. This data is not used during restore yet.
  */

 /* Notes:
  *
  * - blitter state is not saved, blitter is forced to finish immediately if it
  *   was active
  * - disk DMA state is completely saved (I hope so..)
  * - does not ask for statefile name and description. Currently uses DF0's disk
  *   image name (".adf" is replaced with ".asf")
  * - only Amiga state is restored, harddisk support, autoconfig, expansion boards etc..
  *   are not saved/restored (and probably never will).
  * - use this for saving games that can't be saved to disk
  */

 /* Usage :
  *
  * save:
  * 
  * set savestate_state = STATE_DOSAVE, savestate_filename = "..."
  *
  * restore:
  * 
  * set savestate_state = STATE_DORESTORE, savestate_filename = "..."
  *
  */

#include "uae.h"
#include "sysconfig.h"
#include "sysdeps.h"

#include "config.h"
#include "gui.h"
#include "options.h"
#include "memory.h"
#include "custom.h"
#include "sound.h"
#include "audio.h"
#include "m68k/m68k_intrf.h"
#include "debug_uae4all.h"

#ifdef USE_LIB7Z
#include "lib7z/lzma.h"
#ifdef USE_LIB7Z_HIGH_COMPRESSION
#define LZMA_COMPRESSION_LEVEL 9
#define LZMA_DICT_SIZE (65536*4)
#else
#define LZMA_COMPRESSION_LEVEL 1
#define LZMA_DICT_SIZE (65536*4)
#endif
#else
#include <zlib.h>
#endif

#include "savestate.h"

int savestate_state;

static char savestate_filename_default[]={
	'/', 't', 'm', 'p', '/', 'n', 'u', 'l', 'l', '.', 'a', 's', 'f', '\0', 
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
};
char *savestate_filename=(char *)&savestate_filename_default[0];
FILE *savestate_file=NULL;

extern SDL_Surface* prSDLScreen;

/* functions for reading/writing bytes, shorts and longs in big-endian
 * format independent of host machine's endianess */

void save_u32_func (uae_u8 **dstp, uae_u32 v)
{
    uae_u8 *dst = *dstp;
    *dst++ = (uae_u8)(v >> 24);
    *dst++ = (uae_u8)(v >> 16);
    *dst++ = (uae_u8)(v >> 8);
    *dst++ = (uae_u8)(v >> 0);
    *dstp = dst;
}
void save_u16_func (uae_u8 **dstp, uae_u16 v)
{
    uae_u8 *dst = *dstp;
    *dst++ = (uae_u8)(v >> 8);
    *dst++ = (uae_u8)(v >> 0);
    *dstp = dst;
}
void save_u8_func (uae_u8 **dstp, uae_u8 v)
{
    uae_u8 *dst = *dstp;
    *dst++ = v;
    *dstp = dst;
}
void save_string_func (uae_u8 **dstp, const char *from)
{
    uae_u8 *dst = *dstp;
    while(*from)
	*dst++ = *from++;
    *dst++ = 0;
    *dstp = dst;
}

uae_u32 restore_u32_func (uae_u8 **dstp)
{
    uae_u32 v;
    uae_u8 *dst = *dstp;
    v = (dst[0] << 24) | (dst[1] << 16) | (dst[2] << 8) | (dst[3]);
    *dstp = dst + 4;
    return v;
}
uae_u16 restore_u16_func (uae_u8 **dstp)
{
    uae_u16 v;
    uae_u8 *dst = *dstp;
    v=(dst[0] << 8) | (dst[1]);
    *dstp = dst + 2;
    return v;
}
uae_u8 restore_u8_func (uae_u8 **dstp)
{
    uae_u8 v;
    uae_u8 *dst = *dstp;
    v = dst[0];
    *dstp = dst + 1;
    return v;
}
char *restore_string_func (uae_u8 **dstp)
{
    int len;
    uae_u8 v;
    uae_u8 *dst = *dstp;
    char *top, *to;
    len = strlen((char *)dst) + 1;
    top = to = (char *)malloc (len);
    do {
	v = *dst++;
	*top++ = v;
    } while(v);
    *dstp = dst;
    return to; 
}

/* read and write IFF-style hunks */

static void save_chunk (FILE *f, const uae_u8 *chunk, long len, const char *name)
{
    uae_u8 tmp[4], *dst;
    uae_u8 zero[4]= { 0, 0, 0, 0 };

    if (!chunk)
	return;

    /* chunk name */
    fwrite (name, 1, 4, f);
    /* chunk size */
    dst = &tmp[0];
    save_u32 (len + 4 + 4 + 4);
    fwrite (&tmp[0], 1, 4, f);
    /* chunk flags */
    dst = &tmp[0];
    save_u32 (0);
    fwrite (&tmp[0], 1, 4, f);
    /* chunk data */
    fwrite (chunk, 1, len, f);
    /* alignment */
    len = 4 - (len & 3);
    if (len)
	fwrite (zero, 1, len, f);
}

static void save_chunk_compressed (FILE *f, const uae_u8 *chunk, long len, const char *name)
{
#ifndef DREAMCAST
	void *tmp=malloc(len);
#else
	extern void *uae4all_vram_memory_free;
	void *tmp=uae4all_vram_memory_free;
#endif
	long outSize=len;
#ifdef USE_LIB7Z
	Lzma_Encode((Byte *)tmp, (size_t *)&outSize, (const Byte *)chunk, (size_t)len, LZMA_COMPRESSION_LEVEL, LZMA_DICT_SIZE);
#else
	compress2((Bytef *)tmp,(uLongf*)&outSize,(const Bytef *)chunk,len,Z_BEST_COMPRESSION);
#endif
	save_chunk(f,(uae_u8*)tmp,outSize,name);
#ifndef DREAMCAST
	free(tmp);
#endif
}


static uae_u8 *restore_chunk (FILE *f, char *name, long *len, long *filepos)
{
    uae_u8 tmp[4], dummy[4], *mem, *src;
    uae_u32 flags;
    long len2;

    /* chunk name */
    fread (name, 1, 4, f);
    name[4] = 0;
    /* chunk size */
    fread (tmp, 1, 4, f);
    src = tmp;
    len2 = restore_u32 () - 4 - 4 - 4;
    if (len2 < 0)
	len2 = 0;
    *len = len2;
    if (len2 == 0)
	return 0;

    /* chunk flags */
    fread (tmp, 1, 4, f);
    src = tmp;
    flags = restore_u32 ();

    *filepos = ftell (f);
    /* chunk data.  RAM contents will be loaded during the reset phase,
       no need to malloc multiple megabytes here.  */
    if (strcmp (name, "CRAM") != 0
	&& strcmp (name, "BRAM") != 0
	&& strcmp (name, "FRAM") != 0
	&& strcmp (name, "ZRAM") != 0)
    {
	mem = (uae_u8 *)malloc (len2);
	fread (mem, 1, len2, f);
    } else {
	mem = 0;
	fseek (f, len2, SEEK_CUR);
    }

    /* alignment */
    len2 = 4 - (len2 & 3);
    if (len2)
	fread (dummy, 1, len2, f);
    return mem;
}

static void restore_header (uae_u8 *src)
{
    char *emuname, *emuversion, *description;

    restore_u32();
    emuname = restore_string ();
    emuversion = restore_string ();
    description = restore_string ();
    write_log ("Saved with: '%s %s', description: '%s'\n",
	emuname,emuversion,description);
    free (description);
    free (emuversion);
    free (emuname);
}

static void clear_events(void) {
	SDL_Event event;
	while (SDL_PollEvent(&event));
}
/* restore all subsystems */

void restore_state (const char *filename)
{
    FILE *f;
    uae_u8 *chunk,*end;
    char name[5];
    long len;
    long filepos;
    int i=0;

#if !defined(DREAMCAST) && !defined(DINGOO)
    if (SDL_MUSTLOCK(prSDLScreen))
    	SDL_UnlockSurface (prSDLScreen);
#endif
#ifdef DOUBLEBUFFER
	/* Avoid flickering the emulated screen around the progress window */
	SDL_FillRect(prSDLScreen, NULL, SDL_MapRGB(prSDLScreen->format, 0, 0, 0));
	SDL_Flip(prSDLScreen);
	SDL_FillRect(prSDLScreen, NULL, SDL_MapRGB(prSDLScreen->format, 0, 0, 0));
	SDL_Flip(prSDLScreen);
#endif

    gui_show_window_bar(0, 10, 1);
#ifdef DREAMCAST
	extern void reinit_sdcard(void);
	reinit_sdcard();
#endif
#ifdef DEBUG_SAVESTATE
    puts("-->restore_state");fflush(stdout);
#endif
    chunk = 0;
    f = fopen (filename, "rb");
    if (!f) {
#ifdef DREAMCAST
	char *ad=(char *)calloc(strlen(filename)+16,1);
	strcpy(ad,"/sd/uae4all/");
	strcat(ad,filename);
	f=fopen(ad,"rb");
	free(ad);
	if (!f)
#endif
	goto error;
    }

    chunk = restore_chunk (f, name, &len, &filepos);
    if (!chunk || memcmp (name, "ASF ", 4)) {
	write_log ("%s is not an AmigaStateFile\n",filename);
	goto error;
    }
    savestate_file = f;
    restore_header (chunk);
    free (chunk);
    savestate_state = STATE_RESTORE;
    for (;;) {
	chunk = restore_chunk (f, name, &len, &filepos);
	write_log ("Chunk '%s' size %d\n", name, len);
#ifdef DEBUG_SAVESTATE
	puts(name);fflush(stdout);
#endif
	if (!strcmp (name, "END "))
	    break;
	{
		if (i&1)
			gui_show_window_bar(i/2, 10, 1);
		if (i<20)
			i++;
	}
	if (!strcmp (name, "CRAM")) {
	    restore_cram (len, filepos);
	    continue;
	}
	else if (!strcmp (name, "BRAM")) {
	    restore_bram (len, filepos);
	    continue;
	} else if (!strcmp (name, "FRAM")) {
	    restore_fram (len, filepos);
	    continue;
	} else if (!strcmp (name, "ZRAM")) {
	    restore_zram (len, filepos);
	    continue;
	}

	if (!strcmp (name, "CPU "))
	    end = restore_cpu (chunk);
	else if (!strcmp (name, "AGAC"))
	    end = restore_custom_agacolors (chunk);
	else if (!strcmp (name, "SPR0"))
	    end = restore_custom_sprite (chunk, 0);
	else if (!strcmp (name, "SPR1"))
	    end = restore_custom_sprite (chunk, 1);
	else if (!strcmp (name, "SPR2"))
	    end = restore_custom_sprite (chunk, 2);
	else if (!strcmp (name, "SPR3"))
	    end = restore_custom_sprite (chunk, 3);
	else if (!strcmp (name, "SPR4"))
	    end = restore_custom_sprite (chunk, 4);
	else if (!strcmp (name, "SPR5"))
	    end = restore_custom_sprite (chunk, 5);
	else if (!strcmp (name, "SPR6"))
	    end = restore_custom_sprite (chunk, 6);
	else if (!strcmp (name, "SPR7"))
	    end = restore_custom_sprite (chunk, 7);
	else if (!strcmp (name, "CIAA"))
	    end = restore_cia (0, chunk);
	else if (!strcmp (name, "CIAB"))
	    end = restore_cia (1, chunk);
	else if (!strcmp (name, "CHIP"))
	    end = restore_custom (chunk);
	else if (!strcmp (name, "AUD0"))
	    end = restore_audio (chunk, 0);
	else if (!strcmp (name, "AUD1"))
	    end = restore_audio (chunk, 1);
	else if (!strcmp (name, "AUD2"))
	    end = restore_audio (chunk, 2);
	else if (!strcmp (name, "AUD3"))
	    end = restore_audio (chunk, 3);
	else if (!strcmp (name, "DISK"))
	    end = restore_floppy (chunk);
	else if (!strcmp (name, "DSK0"))
	    end = restore_disk (0, chunk);
	else if (!strcmp (name, "DSK1"))
	    end = restore_disk (1, chunk);
	else if (!strcmp (name, "DSK2"))
	    end = restore_disk (2, chunk);
	else if (!strcmp (name, "DSK3"))
	    end = restore_disk (3, chunk);
	else if (!strcmp (name, "EXPA"))
	    end = restore_expansion (chunk);
	else if (!strcmp (name, "ROM "))
	    end = restore_rom (chunk);
	else
	    write_log ("unknown chunk '%s' size %d bytes\n", name, len);
	if (len != end - chunk)
	    write_log ("Chunk '%s' total size %d bytes but read %d bytes!\n",
		       name, len, end - chunk);
	free (chunk);
    }
    gui_show_window_bar(10, 10, 1);
    clear_events();
#ifdef DEBUG_SAVESTATE
    puts("-->OK");fflush(stdout);
    printf("RESTORED state=%X, flags=%X, PC=%X\n",savestate_state,_68k_spcflags,_68k_getpc());fflush(stdout);
#endif
#ifdef AUTO_SAVESTATE
//	DEBUG_AHORA=1;
#endif
    goto end;

    error:
#ifdef DEBUG_SAVESTATE
    puts("-->ERROR");fflush(stdout);
#endif
    savestate_state = 0;
    savestate_file = 0;
    if (chunk)
	free (chunk);
    if (f)
	fclose (f);
    resume_sound();
    if (produce_sound)
    	update_audio();
    notice_screen_contents_lost();
    gui_set_message("Error loadstate", 50);

    end:
#if !defined(DREAMCAST) && !defined(DINGOO)
    if (SDL_MUSTLOCK(prSDLScreen))
    	SDL_LockSurface (prSDLScreen);
#endif
	return;
}

void savestate_restore_finish (void)
{
    if (savestate_state != STATE_RESTORE)
	return;
#ifdef DEBUG_SAVESTATE
    printf("-->savestate_restore_finish state=%X, flags=%X, PC=%X\n",savestate_state,_68k_spcflags,_68k_getpc());fflush(stdout);
#endif
    fclose (savestate_file);
#ifdef DINGOO
    sync();
#endif
    resume_sound();
    if (produce_sound)
    	update_audio();
    savestate_file = 0;
    savestate_state = 0;
//    unset_special(SPCFLAG_BRK);
    notice_screen_contents_lost();
    gui_set_message("Restored", 50);
}

/* Save all subsystems  */

void save_state (const char *filename, const char *description)
{
    uae_u8 header[1000];
    char tmp[100];
    uae_u8 *dst;
    FILE *f;
    int len,i;
    char name[5];

#if !defined(DREAMCAST) && !defined(DINGOO)
    if (SDL_MUSTLOCK(prSDLScreen))
    	SDL_UnlockSurface (prSDLScreen);
#endif
#ifdef DOUBLEBUFFER
	/* Avoid flickering the emulated screen around the progress window */
	SDL_FillRect(prSDLScreen, NULL, SDL_MapRGB(prSDLScreen->format, 0, 0, 0));
	SDL_Flip(prSDLScreen);
	SDL_FillRect(prSDLScreen, NULL, SDL_MapRGB(prSDLScreen->format, 0, 0, 0));
	SDL_Flip(prSDLScreen);
#endif

    gui_show_window_bar(0, 10, 0);
#ifdef DREAMCAST
	extern void reinit_sdcard(void);
	reinit_sdcard();
#endif
#ifdef DEBUG_SAVESTATE
    printf("-->save_state('%s','%s'\n",filename,description);fflush(stdout);
#endif
    f = fopen (filename, "wb");
    if (!f) {
#ifdef DREAMCAST
	char *ad=(char *)calloc(strlen(filename)+16,1);
	strcpy(ad,"/sd/uae4all/");
	strcat(ad,filename);
	f=fopen(ad,"wb");
	free(ad);
	if (!f)
#endif
	return;
    } 

    gui_show_window_bar(0, 10, 0);
#ifdef DEBUG_SAVESTATE
    puts("--> save CPU");fflush(stdout);
#endif
    dst = header;
    save_u32 (0);
    save_string("UAE");
    sprintf (tmp, "%d.%d.%d", UAEMAJOR, UAEMINOR, UAESUBREV);
    save_string (tmp);
    save_string (description);
    save_chunk (f, header, dst-header, "ASF ");

    dst = save_cpu (&len);
    save_chunk (f, dst, len, "CPU ");
    free (dst);

    gui_show_window_bar(1, 10, 0);
#ifdef DEBUG_SAVESTATE
    puts("--> save DSK");fflush(stdout);
#endif
    strcpy(name, "DSKx");
    for (i = 0; i < 4; i++) {
	dst = save_disk (i, &len);
	if (dst) {
	    name[3] = i + '0';
	    save_chunk (f, dst, len, name);
	    free (dst);
	}
    }
    dst = save_floppy (&len);
    save_chunk (f, dst, len, "DISK");
    free (dst);

    gui_show_window_bar(2, 10, 0);
#ifdef DEBUG_SAVESTATE
    puts("--> save CHIP");fflush(stdout);
#endif
    dst = save_custom (&len);
    save_chunk (f, dst, len, "CHIP");
    free (dst);

    gui_show_window_bar(3, 10, 0);
#ifdef DEBUG_SAVESTATE
    puts("--> save AGAC");fflush(stdout);
#endif
    dst = save_custom_agacolors (&len);
    save_chunk (f, dst, len, "AGAC");
    free (dst);

    gui_show_window_bar(4, 10, 0);
#ifdef DEBUG_SAVESTATE
    puts("--> save SPR");fflush(stdout);
#endif
    strcpy (name, "SPRx");
    for (i = 0; i < 8; i++) {
	dst = save_custom_sprite (&len, i);
	name[3] = i + '0';
	save_chunk (f, dst, len, name);
	free (dst);
    }

    gui_show_window_bar(5, 10, 0);
#ifdef DEBUG_SAVESTATE
    puts("--> save AUD");fflush(stdout);
#endif
    strcpy (name, "AUDx");
    for (i = 0; i < 4; i++) {
	dst = save_audio (&len, i);
	name[3] = i + '0';
	save_chunk (f, dst, len, name);
	free (dst);
    }

    gui_show_window_bar(6, 10, 0);
#ifdef DEBUG_SAVESTATE
    puts("--> save CIA");fflush(stdout);
#endif
    dst = save_cia (0, &len);
    save_chunk (f, dst, len, "CIAA");
    free (dst);

    dst = save_cia (1, &len);
    save_chunk (f, dst, len, "CIAB");
    free (dst);

    gui_show_window_bar(7, 10, 0);
#ifdef DEBUG_SAVESTATE
    puts("--> save EXPA");fflush(stdout);
#endif
    dst = save_expansion (&len);
    save_chunk (f, dst, len, "EXPA");
#ifdef DEBUG_SAVESTATE
    puts("--> save CRAM");fflush(stdout);
#endif
    dst = save_cram (&len);
    save_chunk_compressed (f, dst, len, "CRAM");
#ifdef DEBUG_SAVESTATE
    puts("--> save BRAM");fflush(stdout);
#endif
    dst = save_bram (&len);
    save_chunk (f, dst, len, "BRAM");
#ifdef DEBUG_SAVESTATE
    puts("--> save FRAM");fflush(stdout);
#endif
    dst = save_fram (&len);
    save_chunk (f, dst, len, "FRAM");
#ifdef DEBUG_SAVESTATE
    puts("--> save ZRAM");fflush(stdout);
#endif
    dst = save_zram (&len);
    save_chunk (f, dst, len, "ZRAM");

    gui_show_window_bar(8, 10, 0);
#ifdef DEBUG_SAVESTATE
    puts("--> save ROM");fflush(stdout);
#endif
    dst = save_rom (1, &len);
    do {
	if (!dst)
	    break;
	save_chunk (f, dst, len, "ROM ");
#ifndef DREAMCAST
	free (dst);
#endif
    } while ((dst = save_rom (0, &len)));

    gui_show_window_bar(9, 10, 0);
#ifdef DEBUG_SAVESTATE
    puts("--> save END");fflush(stdout);
#endif
    fwrite ("END ", 1, 4, f);
    fwrite ("\0\0\0\08", 1, 4, f);
    write_log ("Save of '%s' complete\n", filename);
    fclose (f);
#ifdef DINGOO
    sync();
#endif
    gui_show_window_bar(10, 10, 0);
    clear_events();
    notice_screen_contents_lost();
#ifdef DEBUG_SAVESTATE
    printf("SAVED state=%X, flags=%X, PC=%X\n",savestate_state,_68k_spcflags,_68k_getpc());fflush(stdout);
#endif
#ifdef START_DEBUG_SAVESTATE
	DEBUG_AHORA=1;
#endif
#if !defined(DREAMCAST) && !defined(DINGOO)
    if (SDL_MUSTLOCK(prSDLScreen))
    	SDL_LockSurface (prSDLScreen);
#endif
}

/*

My (Toni Wilen <twilen@arabuusimiehet.com>)
proposal for Amiga-emulators' state-save format

Feel free to comment...

This is very similar to IFF-fileformat
Every hunk must end to 4 byte boundary,
fill with zero bytes if needed

version 0.7

HUNK HEADER (beginning of every hunk)

        hunk name (4 ascii-characters)
        hunk size (including header)
        hunk flags             

        bit 0 = chunk contents are compressed with zlib (maybe RAM chunks only?)

HEADER

        "ASF " (AmigaStateFile)
        
	statefile version
        emulator name ("uae", "fellow" etc..)
        emulator version string (example: "0.8.15")
        free user writable comment string

CPU

         "CPU "

        CPU model               4 (68000,68010 etc..)
        CPU typeflags           bit 0=EC-model or not
        D0-D7                   8*4=32
        A0-A6                   7*4=32
        PC                      4
        prefetch address        4
        prefetch data           4
        USP                     4
        ISP                     4
        SR/CCR                  2
        flags                   4 (bit 0=CPU was HALTed)

        CPU specific registers

        68000: SR/CCR is last saved register
        68010: save also DFC,SFC and VBR
        68020: all 68010 registers and CAAR,CACR and MSP
        etc..

        DFC                     4 (010+)
        SFC                     4 (010+)
        VBR                     4 (010+)

        CAAR                    4 (020-030)
        CACR                    4 (020+)
        MSP                     4 (020+)

MMU (when and if MMU is supported in future..)

        MMU model               4 (68851,68030,68040)

        // 68040 fields

        ITT0                    4
        ITT1                    4
        DTT0                    4
        DTT1                    4
        URP                     4
        SRP                     4
        MMUSR                   4
        TC                      2

		
FPU (only if used)

	"FPU "

        FPU model               4 (68881 or 68882)
        FPU typeflags           4 (keep zero)

        FP0-FP7                 4+2 (80 bits)
        FPCR                    4
        FPSR                    4
        FPIAR                   4

CUSTOM CHIPS

        "CHIP"

        chipset flags   4      OCS=0,ECSAGNUS=1,ECSDENISE=2,AGA=4
                               ECSAGNUS and ECSDENISE can be combined

        DFF000-DFF1FF   352    (0x120 - 0x17f and 0x0a0 - 0xdf excluded)

        sprite registers (0x120 - 0x17f) saved with SPRx chunks
        audio registers (0x0a0 - 0xdf) saved with AUDx chunks

AGA COLORS

        "AGAC"

        AGA color               8 banks * 32 registers *
        registers               LONG (XRGB) = 1024

SPRITE

        "SPR0" - "SPR7"


        SPRxPT                  4
        SPRxPOS                 2
        SPRxCTL                 2
        SPRxDATA                2
        SPRxDATB                2
        AGA sprite DATA/DATB    3 * 2 * 2
        sprite "armed" status   1

        sprites maybe armed in non-DMA mode
        use bit 0 only, other bits are reserved


AUDIO
        "AUD0" "AUD1" "AUD2" "AUD3"

        audio state             1
        machine mode
        AUDxVOL                 1
	irq?                    1
	data_written?           1
        internal AUDxLEN        2
        AUDxLEN                 2
	internal AUDxPER        2
	AUDxPER                 2
        internal AUDxLC         4
	AUDxLC                  4
	evtime?                 4

BLITTER

        "BLIT"

        internal blitter state

        blitter running         1
        anything else?

CIA

        "CIAA" and "CIAB"

        BFE001-BFEF01   16*1 (CIAA)
        BFD000-BFDF00   16*1 (CIAB)

        internal registers

        IRQ mask (ICR)  1 BYTE
        timer latches   2 timers * 2 BYTES (LO/HI)
        latched tod     3 BYTES (LO/MED/HI)
        alarm           3 BYTES (LO/MED/HI)
        flags           1 BYTE
                        bit 0=tod latched (read)
                        bit 1=tod stopped (write)
	div10 counter	1 BYTE

FLOPPY DRIVES

        "DSK0" "DSK1" "DSK2" "DSK3"

        drive state

        drive ID-word           4
        state                   1 (bit 0: motor on, bit 1: drive disabled)
        rw-head track           1
        dskready                1
        id-mode                 1 (ID mode bit number 0-31)
        floppy information

        bits from               4
        beginning of track
        CRC of disk-image       4 (used during restore to check if image
                                  is correct)
        disk-image              null-terminated
        file name

INTERNAL FLOPPY CONTROLLER STATUS

        "DISK"

        current DMA word        2
        DMA word bit offset     1
        WORDSYNC found          1 (no=0,yes=1)
        hpos of next bit        1
        DSKLENGTH status        0=off,1=written once,2=written twice

RAM SPACE 

        "xRAM" (CRAM = chip, BRAM = bogo, FRAM = fast, ZFRAM = Z3)

        start address           4 ("bank"=chip/slow/fast etc..)
        of RAM "bank"
        RAM "bank" size         4
        RAM flags               4
        RAM "bank" contents

ROM SPACE

        "ROM "

        ROM start               4
        address
        size of ROM             4
        ROM type                4 KICK=0
        ROM flags               4
        ROM version             2
        ROM revision            2
        ROM CRC                 4 see below
        ROM-image               null terminated, see below
        ID-string
        ROM contents            (Not mandatory, use hunk size to check if
                                this hunk contains ROM data or not)

        Kickstart ROM:
         ID-string is "Kickstart x.x"
         ROM version: version in high word and revision in low word
         Kickstart ROM version and revision can be found from ROM start
         + 12 (version) and +14 (revision)

        ROM version and CRC is only meant for emulator to automatically
        find correct image from its ROM-directory during state restore.

        Usually saving ROM contents is not good idea.


END
        hunk "END " ends, remember hunk size 8!


EMULATOR SPECIFIC HUNKS

Read only if "emulator name" in header is same as used emulator.
Maybe useful for configuration?

misc:

- save only at position 0,0 before triggering VBLANK interrupt
- all data must be saved in bigendian format
- should we strip all paths from image file names?

*/
