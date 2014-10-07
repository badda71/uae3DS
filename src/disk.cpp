 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Floppy disk emulation
  *
  * Copyright 1995 Hannu Rummukainen
  * Copyright 1995-2001 Bernd Schmidt
  * Copyright 2000, 2001 Toni Wilen
  *
  * High Density Drive Handling by Dr. Adil Temel (C) 2001 [atemel1@hotmail.com]
  *
  */

#include <stdio.h>

#include "sysconfig.h"
#include "sysdeps.h"

#include "config.h"
#include "uae.h"
#include "options.h"
#include "thread.h"
#include "memory.h"
#include "debug_uae4all.h"
#include "events.h"
#include "custom.h"
#include "ersatz.h"
#include "disk.h"
#include "gui.h"
#include "zfile.h"
#include "autoconf.h"
#include "m68k/m68k_intrf.h"
#include "xwin.h"
#include "osemu.h"
#include "execlib.h"
#include "savestate.h"

#define maxhpos MAXHPOS

char prefs_df[NUM_DRIVES][128];
char changed_df[NUM_DRIVES][128];
char romfile[64];
#ifdef DREAMCAST
char romfile_sd[32];
#endif
int real_changed_df[NUM_DRIVES]={0,0};

/* writable track length with normal 2us bitcell/300RPM motor */
#define FLOPPY_WRITE_LEN (12650 / 2)
/* This works out to 341 */
#define FLOPPY_GAP_LEN (FLOPPY_WRITE_LEN - 11 * 544)
/* (cycles/bitcell) << 8, normal = ((2us/280ns)<<8) = ~1830 */
#define NORMAL_FLOPPY_SPEED 1830

/*
#ifdef DEBUG_DISK
*/
static int floppy_speed = NORMAL_FLOPPY_SPEED;
#define MAX_DISK_WORDS_PER_LINE 50	/* depends on floppy_speed */
/*
#else
static int floppy_speed = (NORMAL_FLOPPY_SPEED/8);
#define MAX_DISK_WORDS_PER_LINE 100
#endif
*/


/* UAE-1ADF (ADF_EXT2)
 * W	reserved
 * W	number of tracks (default 2*80=160)
 *
 * W	reserved
 * W	type, 0=normal AmigaDOS track, 1 = raw MFM
 * L	available space for track in bytes
 * L	track length in bits
 */

static int side, direction, writing;
static uae_u8 selected = 15, disabled=0;

static uae_u8 *writebuffer[544 * 22];

typedef enum { TRACK_AMIGADOS, TRACK_RAW, TRACK_RAW1 } image_tracktype;
typedef struct {
    uae_u16 len;
    uae_u32 offs;
    int bitlen;
    unsigned int sync;
    image_tracktype type;
} trackid;

#define MAX_TRACKS 328

#define DRIVE_ID_NONE  0xFFFFFFFF
#define DRIVE_ID_35DD  0x00000000
#define DRIVE_ID_35HD  0xAAAAAAAA
#define DRIVE_ID_525DD 0x55555555 /* 40 track 5.25 drive , kickstart does not recognize this */

typedef enum { ADF_NORMAL, ADF_EXT1, ADF_EXT2 } drive_filetype;
typedef struct {
    FILE *diskfile;
    drive_filetype filetype;
    trackid trackdata[MAX_TRACKS];
    int buffered_cyl, buffered_side;
    int cyl;
    int motoroff;
    int state;
    int wrprot;
    uae_u16 bigmfmbuf[0x8000];
    int mfmpos;
    int tracklen;
    int trackspeed;
    int dmalen;
    int num_tracks, num_secs;
    int dskchange;
    int dskchange_time;
    int dskready;
    int dskready_time;
    int steplimit;
    int ddhd; /* 1=DD 2=HD */
    int drive_id_scnt; /* drive id shift counter */
    unsigned long drive_id; /* drive id to be reported */
    char newname[256]; /* storage space for new filename during eject delay */
} drive;

static drive floppy[NUM_DRIVES];

/* Keeps track of whether the Amiga program seems to be using the data coming
   in from the disk; if this remains 0 for several seconds, we stop calling
   disk_doupdate_read, even if the motor is still on.  This is a rather large
   performance win.  This contains the number of frames since we last accessed
   the disk.  */
static int disk_data_used;

#ifdef DEBUG_DISK
static char *drive_id_name(drive *drv)
{
#ifdef DEBUG_DISK
    dbg("disc.c : drive_id_name");
#endif
    switch(drv->drive_id)
    {
    case DRIVE_ID_35HD : return "3.5HD";
    case DRIVE_ID_525DD: return "5.25DD";
    case DRIVE_ID_35DD : return "3.5DD";
    }
    return "UNKNOWN"; 
}
#endif

/* Simulate exact behaviour of an A3000T 3.5 HD disk drive. 
 * The drive reports to be a 3.5 DD drive whenever there is no
 * disk or a 3.5 DD disk is inserted. Only 3.5 HD drive id is reported
 * when a real 3.5 HD disk is inserted. -Adil
 */
static void drive_settype_id(drive *drv)
{
#ifdef DEBUG_DISK
    dbg("disc.c : drive_settype_id");
#endif
    if (!drv->diskfile || drv->num_secs == 11) {
	/* Double density */
	drv->drive_id = DRIVE_ID_35DD;
	drv->ddhd = 1;
    } else {
	/* High density */
	drv->drive_id = DRIVE_ID_35HD;
	drv->ddhd = 2;
    }
    drv->drive_id_scnt = 0;
#ifdef DEBUG_DISK
    write_log("drive_settype_id: DF%d: set to %s\n",drv-floppy, drive_id_name(drv));
#endif
}

/* code for track display */
static void update_drive_gui (int num)
{
#ifdef DEBUG_DISK
   dbg("disc.c : update_drive_gui");
#endif
   if (num>NUM_DRIVES) return;
    drive *drv = floppy + num;

    if (drv->state == gui_data.drive_motor[num]
	&& drv->cyl == gui_data.drive_track[num]
	&& ((writing && gui_data.drive_writing[num])
	    || (!writing && !gui_data.drive_writing[num]))) {
	return;
    }
    gui_data.drive_motor[num] = drv->state;
    gui_data.drive_track[num] = drv->cyl;
    if (!gui_data.drive_writing[num])
	gui_data.drive_writing[num] = writing;
    gui_ledstate &= ~(2 << num);
    if (drv->state)
	gui_ledstate |= 2 << num;
}

static void drive_fill_bigbuf (drive * drv);

FILE *DISK_validate_filename (const char *fname, int leave_open, int *wrprot)
{
#ifdef DEBUG_DISK
    dbgf("disc.c : DISK_validate_filename %s, %i\n",fname, leave_open);
#endif
    FILE *f = zfile_open (fname, "r+b");
    if (f) {
	if (wrprot)
	    *wrprot = 0;
    } else {
	if (wrprot)
	    *wrprot = 1;
	f = zfile_open (fname, "rb");
    }
    if (!leave_open)
	zfile_close (f);
    return f;
}

static int drive_insert (drive * drv, int dnum, const char *fname)
{
    unsigned char buffer[2 + 2 + 4 + 4];
    trackid *tid;

#ifdef DEBUG_DISK
    dbgf("disc.c : DRIVE_INSERT %i - %s\n",dnum,fname);
#endif
    drv->diskfile = DISK_validate_filename (fname, 1, &drv->wrprot);
    if (drv->diskfile == 0) {
	drv->tracklen = FLOPPY_WRITE_LEN * 2 * 8;
	drv->trackspeed = floppy_speed;
	return 0;
    }
    strncpy ((char *)&prefs_df[drv-floppy], fname, 127);
    prefs_df[drv-floppy][127] = 0;
    strncpy ((char *)&changed_df[drv-floppy], fname, 127);
    changed_df[drv-floppy][127] = 0;

    uae4all_fread (buffer, sizeof (char), 8, drv->diskfile);
    if (strncmp ((char *) buffer, "UAE-1ADF", 8) == 0) {
	int offs;
	int i;

	uae4all_fread (buffer, 1, 4, drv->diskfile);
	drv->num_tracks = buffer[2] * 256 + buffer[3];
	drv->filetype = ADF_EXT2;
	drv->num_secs = 11;
	offs = 8 + 2 + 2 + drv->num_tracks * (2 + 2 + 4 + 4);

	for (i = 0; i < drv->num_tracks; i++) {
	    tid = &drv->trackdata[i];
	    uae4all_fread (buffer, 2 + 2 + 4 + 4, 1, drv->diskfile);
	    tid->type = (image_tracktype)(buffer[2] * 256 + buffer[3]);
	    tid->len = buffer[5] * 65536 + buffer[6] * 256 + buffer[7];
	    tid->bitlen = buffer[9] * 65536 + buffer[10] * 256 + buffer[11];
	    tid->offs = offs;
	    if (tid->type == TRACK_AMIGADOS) {
		if (tid->len > 20000)
		    drv->num_secs = 22;
	    }
	    offs += tid->len;
	}
    } else if (strncmp ((char *) buffer, "UAE--ADF", 8) == 0) {
	int offs = 160 * 4 + 8;
	int i;

	drv->filetype = ADF_EXT1;
	drv->num_tracks = 160;
	drv->num_secs = 11;

	drv->wrprot = 1;	/* write to adf_ext1 not implemented */
	for (i = 0; i < 160; i++) {
	    tid = &drv->trackdata[i];
	    uae4all_fread (buffer, 4, 1, drv->diskfile);
	    tid->sync = buffer[0] * 256 + buffer[1];
	    tid->len = buffer[2] * 256 + buffer[3];
	    tid->offs = offs;
	    if (tid->sync == 0) {
		tid->type = TRACK_AMIGADOS;
		tid->bitlen = 0;
	    } else {
		tid->type = TRACK_RAW1;
		tid->bitlen = tid->len * 8;
	    }
	    offs += tid->len;
	}
    } else {
	int i;

	drv->filetype = ADF_NORMAL;

	uae4all_fseek (drv->diskfile, 0, SEEK_END);
	i = uae4all_ftell (drv->diskfile);
	uae4all_fseek (drv->diskfile, 0, SEEK_SET);

	/* High-density disk? */
	if (i >= 160 * 22 * 512)
	    drv->num_tracks = i / (512 * (drv->num_secs = 22));
	else
	    drv->num_tracks = i / (512 * (drv->num_secs = 11));

	if (drv->num_tracks > MAX_TRACKS)
	    write_log ("Your diskfile is too big!\n");
	for (i = 0; i < drv->num_tracks; i++) {
	    tid = &drv->trackdata[i];
	    tid->type = TRACK_AMIGADOS;
	    tid->len = 512 * drv->num_secs;
	    tid->bitlen = 0;
	    tid->offs = i * 512 * drv->num_secs;
	}
    }
    drive_settype_id(drv);	/* Set DD or HD drive */
    drv->buffered_side = 2;	/* will force read */
    drive_fill_bigbuf (drv);
    return 1;
}

static int drive_empty (drive * drv)
{
#ifdef DEBUG_DISK
    dbg("disc.c : drive_empty");
#endif
    return drv->diskfile == 0;
}

static void drive_step (drive * drv)
{
#ifdef DEBUG_DISK
    dbg("disc.c : drive_step");
#endif
    if (drv->steplimit)
	return;
    /* A1200's floppy drive needs at least 30 raster lines between steps
     * but we'll use very small value for better compatibility with faster CPU emulation
     * (stupid trackloaders with CPU delay loops)
     */
    drv->steplimit = 2;
    if (!drive_empty (drv))
	drv->dskchange = 0;
    if (direction) {
	if (drv->cyl)
	    drv->cyl--;
/*	else
	    write_log("program tried to step beyond track zero\n");
	    "no-click" programs does that
*/
  } else {
	if (drv->cyl < 83)
	    drv->cyl++;
	else
	    write_log("program tried to step over track 83\n");
    }

#ifdef DEBUG_DISK
    write_log ("track %d\n", drv->cyl);
#endif
}

static int drive_track0 (drive * drv)
{
#ifdef DEBUG_DISK
    dbg("disc.c : drive_track0");
#endif
    return drv->cyl == 0;
}

static int drive_writeprotected (drive * drv)
{
#ifdef DEBUG_DISK
    dbg("disc.c : drive_writeprotected");
#endif
    return drv->wrprot || drv->diskfile == NULL;
}

static int drive_running (drive * drv)
{
#ifdef DEBUG_DISK
    dbg("disc.c : drive_running");
#endif
    return !drv->motoroff;
}

static void drive_motor (drive * drv, int off)
{
#ifdef DEBUG_DISK
    dbg("disc.c : drive_motor");
#endif
    /* A value of 5 works out to a guaranteed delay of 1/2 of a second
       Higher values are dangerous, e.g. a value of 8 breaks the RSI
       demo.  */
    if (drv->motoroff && !off) {
	drv->dskready_time = 5;
	disk_data_used = 0;
	drv->drive_id_scnt = 0; /* Reset id shift reg counter */
#ifdef DEBUG_DISK
	write_log("drive_motor: Selected DF%d: reset id shift reg.\n",drv-floppy);
#endif
    }
    drv->motoroff = off;
    if (drv->motoroff) {
	drv->dskready = 0;
	drv->dskready_time = 0;
    }
}

static void read_floppy_data (drive * drv, int track, int offset, unsigned char *dst, int len)
{
#ifdef DEBUG_DISK
    dbgf("disc.c : read_floppy_data track %i len %i\n",track,len);
#endif
    unsigned i;
    trackid *ti;

    ti = &drv->trackdata[track];
    uae4all_fseek (drv->diskfile, ti->offs + offset, SEEK_SET);
    uae4all_fread (dst, 1, len, drv->diskfile);
}

/* Megalomania does not like zero MFM words... */
static void mfmcode (uae_u16 * mfm, int words)
{
    uae_u32 lastword = 0;
#ifdef DEBUG_DISK
    dbg("disc.c : mfmcode");
#endif

    while (words--) {
	uae_u32 v = *mfm;
	uae_u32 lv = (lastword << 16) | v;
	uae_u32 nlv = 0x55555555 & ~lv;
	uae_u32 mfmbits = (nlv << 1) & (nlv >> 1);

	*mfm++ = v | mfmbits;
	lastword = v;
    }
}

static void drive_fill_bigbuf (drive * drv)
{
#ifdef DEBUG_DISK
    dbg("disc.c : drive_fill_bigbuf");
#endif
    int tr = drv->cyl * 2 + side;
    trackid *ti = drv->trackdata + tr;

    if (!drv->diskfile || tr >= drv->num_tracks) {
	drv->tracklen = FLOPPY_WRITE_LEN * drv->ddhd * 2 * 8;
	drv->trackspeed = floppy_speed;
	memset (drv->bigmfmbuf, 0xaa, FLOPPY_WRITE_LEN * 2 * drv->ddhd);
	return;
    }
    if (drv->buffered_cyl == drv->cyl && drv->buffered_side == side)
	return;

    if (ti->type == TRACK_AMIGADOS) {
#ifdef DEBUG_DISK
	dbg("disc.c : drive_fill_bigbuf --- DISCO AMIGADOS ---");
#endif
	/* Normal AmigaDOS format track */
	int sec;
	drv->tracklen = (drv->num_secs * 544 + FLOPPY_GAP_LEN) * 2 * 8;
	memset (drv->bigmfmbuf, 0xaa, FLOPPY_GAP_LEN * 2);

	for (sec = 0; sec < drv->num_secs; sec++) {
	    uae_u8 secbuf[544];
	    int i;
	    uae_u16 *mfmbuf = drv->bigmfmbuf + 544 * sec + FLOPPY_GAP_LEN;
	    uae_u32 deven, dodd;
	    uae_u32 hck = 0, dck = 0;

	    secbuf[0] = secbuf[1] = 0x00;
	    secbuf[2] = secbuf[3] = 0xa1;
	    secbuf[4] = 0xff;
	    secbuf[5] = tr;
	    secbuf[6] = sec;
	    secbuf[7] = drv->num_secs - sec;

	    for (i = 8; i < 24; i++)
		secbuf[i] = 0;

	    read_floppy_data (drv, tr, sec * 512, &secbuf[32], 512);

	    mfmbuf[0] = mfmbuf[1] = 0xaaaa;
	    mfmbuf[2] = mfmbuf[3] = 0x4489;

	    deven = ((secbuf[4] << 24) | (secbuf[5] << 16)
		     | (secbuf[6] << 8) | (secbuf[7]));
	    dodd = deven >> 1;
	    deven &= 0x55555555;
	    dodd &= 0x55555555;

	    mfmbuf[4] = dodd >> 16;
	    mfmbuf[5] = dodd;
	    mfmbuf[6] = deven >> 16;
	    mfmbuf[7] = deven;

	    for (i = 8; i < 48; i++)
		mfmbuf[i] = 0xaaaa;
	    for (i = 0; i < 512; i += 4) {
		deven = ((secbuf[i + 32] << 24) | (secbuf[i + 33] << 16)
			 | (secbuf[i + 34] << 8) | (secbuf[i + 35]));
		dodd = deven >> 1;
		deven &= 0x55555555;
		dodd &= 0x55555555;
		mfmbuf[(i >> 1) + 32] = dodd >> 16;
		mfmbuf[(i >> 1) + 33] = dodd;
		mfmbuf[(i >> 1) + 256 + 32] = deven >> 16;
		mfmbuf[(i >> 1) + 256 + 33] = deven;
	    }

	    for (i = 4; i < 24; i += 2)
		hck ^= (mfmbuf[i] << 16) | mfmbuf[i + 1];

	    deven = dodd = hck;
	    dodd >>= 1;
	    mfmbuf[24] = dodd >> 16;
	    mfmbuf[25] = dodd;
	    mfmbuf[26] = deven >> 16;
	    mfmbuf[27] = deven;

	    for (i = 32; i < 544; i += 2)
		dck ^= (mfmbuf[i] << 16) | mfmbuf[i + 1];

	    deven = dodd = dck;
	    dodd >>= 1;
	    mfmbuf[28] = dodd >> 16;
	    mfmbuf[29] = dodd;
	    mfmbuf[30] = deven >> 16;
	    mfmbuf[31] = deven;
	    mfmcode (mfmbuf + 4, 544 - 4);
	}
#ifdef DEBUG_DISK
	write_log ("amigados read track %d\n", tr);
#endif
    } else {
#ifdef DEBUG_DISK
	dbg("disc.c : drive_fill_bigbuf --- DISCO NORMAL ---");
#endif
	int i;
	int base_offset = ti->type == TRACK_RAW ? 0 : 1;
	drv->tracklen = ti->bitlen + 16 * base_offset;
	drv->bigmfmbuf[0] = ti->sync;
	read_floppy_data (drv, tr, 0, (unsigned char *) (drv->bigmfmbuf + base_offset), (ti->bitlen + 7) / 8);
	for (i = base_offset; i < (drv->tracklen + 15) / 16; i++) {
	    uae_u16 *mfm = drv->bigmfmbuf + i;
	    uae_u8 *data = (uae_u8 *) mfm;
	    *mfm = 256 * *data + *(data + 1);
	}
#if 0 && defined DEBUG_DISK
	write_log ("rawtrack %d\n", tr);
#endif
    }
    drv->buffered_side = side;
    drv->buffered_cyl = drv->cyl;
    drv->trackspeed = floppy_speed * drv->tracklen / (2 * 8 * FLOPPY_WRITE_LEN);
}

/* Update ADF_EXT2 track header */
static void diskfile_update (drive * drv, int len, uae_u8 type)
{
    trackid *ti;
    int track;
    uae_u8 buf[2 + 2 + 4 + 4], *zerobuf;

#ifdef DEBUG_DISK
    dbg("disc.c : diskfile_update");
#endif
    if (drv->filetype != ADF_EXT2)
	return;

    track = drv->cyl * 2 + side;
    ti = &drv->trackdata[track];
    ti->bitlen = len;
    uae4all_fseek (drv->diskfile, 8 + 4 + (2 + 2 + 4 + 4) * track, SEEK_SET);
    memset (buf, 0, sizeof buf);
    ti->type = (image_tracktype) type;
    buf[3] = ti->type;
    do_put_mem_long ((uae_u32 *) (buf + 4), swab_l(ti->len));
    do_put_mem_long ((uae_u32 *) (buf + 8), swab_l(ti->bitlen));
    uae4all_fwrite (buf, sizeof (buf), 1, drv->diskfile);
    if (ti->len > (len + 7) / 8) {
	zerobuf = (uae_u8 *)xmalloc (ti->len);
	memset (zerobuf, 0, ti->len);
	uae4all_fseek (drv->diskfile, ti->offs, SEEK_SET);
	uae4all_fwrite (zerobuf, 1, ti->len, drv->diskfile);
	free (zerobuf);
    }
#ifdef DEBUG_DISK
    write_log ("track %d, raw track length %d written (total size %d)\n", track, (ti->bitlen + 7) / 8, ti->len);
#endif
}

#define MFMMASK 0x55555555
static uae_u32 getmfmlong (uae_u16 * mbuf)
{
#ifdef DEBUG_DISK
    dbg("disc.c : getmfmlong");
#endif
    return ((*mbuf << 16) | *(mbuf + 1)) & MFMMASK;
}

static int drive_write_adf_amigados (drive * drv)
{
#ifdef DEBUG_DISK
    dbg("disc.c : drive_write_adf_amigados");
#endif
    int i, secwritten = 0;
    int fwlen = FLOPPY_WRITE_LEN * drv->ddhd;
    int length = 2 * fwlen;
    int drvsec= drv->num_secs;
    uae_u32 odd, even, chksum, id, dlong;
    uae_u8 *secdata;
    uae_u8 secbuf[544];
    uae_u16 *mbuf = drv->bigmfmbuf;
    uae_u16 *mend = mbuf + length;
    char sectable[22];
    memset (sectable, 0, sizeof (sectable));
    memcpy (mbuf + fwlen, mbuf, fwlen * sizeof (uae_u16));
    mend -= (4 + 16 + 8 + 512);
    while (secwritten < drvsec) {
	int trackoffs;

	do {
	    while (*mbuf++ != 0x4489) {
		if (mbuf >= mend)
		    return 1;
	    }
	} while (*mbuf++ != 0x4489);

	odd = getmfmlong (mbuf);
	even = getmfmlong (mbuf + 2);
	mbuf += 4;
	id = (odd << 1) | even;

	trackoffs = (id & 0xff00) >> 8;
	if (trackoffs + 1 > drvsec) {
	    if (drv->filetype == ADF_EXT2)
		return 2;
	    write_log ("Disk write: weird sector number %d\n", trackoffs);
	    continue;
	}
	chksum = odd ^ even;
	for (i = 0; i < 4; i++) {
	    odd = getmfmlong (mbuf);
	    even = getmfmlong (mbuf + 8);
	    mbuf += 2;

	    dlong = (odd << 1) | even;
	    if (dlong) {
		if (drv->filetype == ADF_EXT2)
		    return 6;
		secwritten = -200;
	    }
	    chksum ^= odd ^ even;
	}			/* could check here if the label is nonstandard */
	mbuf += 8;
	odd = getmfmlong (mbuf);
	even = getmfmlong (mbuf + 2);
	mbuf += 4;
	if (((odd << 1) | even) != chksum || ((id & 0x00ff0000) >> 16) != drv->cyl * 2 + side) {
	    if (drv->filetype == ADF_EXT2)
		return 3;
	    write_log ("Disk write: checksum error on sector %d header\n", trackoffs);
	    continue;
	}
	odd = getmfmlong (mbuf);
	even = getmfmlong (mbuf + 2);
	mbuf += 4;
	chksum = (odd << 1) | even;
	secdata = secbuf + 32;
	for (i = 0; i < 128; i++) {
	    odd = getmfmlong (mbuf);
	    even = getmfmlong (mbuf + 256);
	    mbuf += 2;
	    dlong = (odd << 1) | even;
	    *secdata++ = dlong >> 24;
	    *secdata++ = dlong >> 16;
	    *secdata++ = dlong >> 8;
	    *secdata++ = dlong;
	    chksum ^= odd ^ even;
	}
	mbuf += 256;
	if (chksum) {
	    if (drv->filetype == ADF_EXT2)
		return 4;
	    write_log ("Disk write: sector %d, data checksum error\n", trackoffs);
	    continue;
	}
	sectable[trackoffs] = 1;
	secwritten++;
	memcpy (writebuffer + trackoffs * 512, secbuf + 32, 512);
    }
    if (drv->filetype == ADF_EXT2 && (secwritten == 0 || secwritten < 0))
	return 5;
    if (secwritten == 0)
	write_log ("Disk write in unsupported format\n");
    if (secwritten < 0)
	write_log ("Disk write: sector labels ignored\n");

    diskfile_update (drv, drvsec * 512 * 8, TRACK_AMIGADOS);
    for (i = 0; i < drvsec; i++) {
	uae4all_fseek (drv->diskfile, drv->trackdata[drv->cyl * 2 + side].offs + i * 512, SEEK_SET);
	uae4all_fwrite (writebuffer + i * 512, sizeof (uae_u8), 512, drv->diskfile);
    }

    return 0;
}

/* write raw track to disk file */
static int drive_write_ext2 (drive * drv)
{
#ifdef DEBUG_DISK
    dbg("disc.c : drive_write_ext2");
#endif
    trackid *ti;
    int len, track, i;

    track = drv->cyl * 2 + side;
    ti = &drv->trackdata[track];
    len = (drv->tracklen + 7) / 8;
    if (len > ti->len) {
	write_log ("disk raw write: image file's track %d is too small (%d < %d)!\n", track, ti->len, len);
	return 0;
    }
    diskfile_update (drv, drv->tracklen, TRACK_RAW);
    for (i = 0; i < drv->trackdata[track].len / 2; i++) {
	uae_u16 *mfm = drv->bigmfmbuf + i;
	uae_u8 *data = (uae_u8 *) mfm;
	*mfm = 256 * *data + *(data + 1);
    }
    uae4all_fseek (drv->diskfile, ti->offs, SEEK_SET);
    uae4all_fwrite (drv->bigmfmbuf, 1, len, drv->diskfile);
    return 1;
}

static void drive_write_data (drive * drv)
{
    int ret;
#ifdef DEBUG_DISK
    dbg("disc.c : drive_write_data");
#endif
    if (drive_writeprotected (drv))
	return;
    switch (drv->filetype) {
    case ADF_NORMAL:
	drive_write_adf_amigados (drv);
	break;
    case ADF_EXT1:
	write_log ("writing to ADF_EXT1 not supported\n");
	return;
    case ADF_EXT2:
	ret = drive_write_adf_amigados (drv);
	if (ret) {
	    /*write_log("not an amigados track (error %d), writing as raw track\n",ret); */
	    drive_write_ext2 (drv);
	}
	break;
    }
    drv->buffered_side = 2;	/* will force read */
}

static void drive_eject (drive * drv)
{
#ifdef DEBUG_DISK
    dbg("disc.c : drive_eject");
#endif
    if (!drive_empty (drv))
	zfile_close (drv->diskfile);
    drv->diskfile = 0;
    drv->dskchange = 1;
    drive_settype_id(drv); /* Back to 35 DD */
#ifdef DEBUG_DISK
    write_log ("eject drive %d\n", drv - &floppy[0]);
#endif
}

/* We use this function if we have no Kickstart ROM.
 * No error checking - we trust our luck. */
void DISK_ersatz_read (int tr, int sec, uaecptr dest)
{
#ifdef DEBUG_DISK
    dbg("disc.c : DISK_ersatz_read");
#endif
    uae_u8 *dptr = get_real_address (dest);
    uae4all_fseek (floppy[0].diskfile, floppy[0].trackdata[tr].offs + sec * 512, SEEK_SET);
    uae4all_fread (dptr, 1, 512, floppy[0].diskfile);
}

void disk_eject (int num)
{
#ifdef DEBUG_DISK
    dbg("disc.c : disk_eject");
#endif
    if (num<NUM_DRIVES)
    {
	drive_eject (floppy + num);
	prefs_df[num][0] = changed_df[num][0] = 0;
	floppy[num].newname[0] = 0;
    }
}

void disk_insert (int num, const char *name)
{
#ifdef DEBUG_DISK
    dbg("disc.c : disk_insert");
#endif
    if (num<NUM_DRIVES)
    {
    	drive *drv = floppy + num;
    	if (name[0] == 0) {
		disk_eject (num);
    	} else if (!drive_empty(drv) || drv->dskchange_time > 0) {
		drive_eject (drv);
		/* set dskchange_time, disk_insert() will be
		 * called from DISK_check_change() after 1 second delay
	 	* this makes sure that all programs detect disk change correctly
	 	*/
		strcpy (drv->newname, name);
		drv->dskchange_time = 10; /* 1 second disk change delay */
    	} else {
		/* no delayed insert if drive is already empty */
		drive_insert (drv, num, name);
    	}
    }
}

void DISK_check_change (void)
{
    int i;

#ifdef DEBUG_DISK
    dbg("disc.c : DISK_check_change");
#endif
    for (i = 0; i < NUM_DRIVES; i++) {
	uae4all_flush_disk(i);
	drive *drv = floppy + i;
	if (real_changed_df[i])
	{
	real_changed_df[i]=0;
	if (strcmp (prefs_df[i], changed_df[i])) {
	    strcpy (prefs_df[i], changed_df[i]);
	    disk_insert (i, prefs_df[i]);
	}
	}
	/* emulate drive motor turn on time */
	if (drv->dskready_time) {
	    drv->dskready_time--;
	    if (drv->dskready_time == 0)
		drv->dskready = 1;
	}
	/* delay until new disk image is inserted */
	if (drv->dskchange_time) {
	    drv->dskchange_time--;
	    if (drv->dskchange_time == 0) {
		drive_insert (drv, i, drv->newname);
#ifdef DEBUG_DISK
		write_log ("delayed insert, drive %d, image '%s'\n", i, drv->newname);
#endif
		drv->newname[0] = 0;
	    }
	}
    }
}

int disk_empty (int num)
{
#ifdef DEBUG_DISK
    dbg("disc.c : disk_empty");
#endif
    return drive_empty (floppy + num);
}

static int step;

void DISK_select (uae_u8 data)
{
    int step_pulse, lastselected;
    int dr;
    static uae_u8 prevdata;

#ifdef DEBUG_DISK
    dbgf("disc.c : DISK_select 0x%X\n",data);
#endif
    lastselected = selected;
    selected = (data >> 3) & 15;
    selected |= disabled;
    if (selected != lastselected)
	disk_data_used = 1;
    side = 1 - ((data >> 2) & 1);

    direction = (data >> 1) & 1;
    step_pulse = data & 1;
    if (step != step_pulse) {
	step = step_pulse;
	if (step) {
	    for (dr = 0; dr < NUM_DRIVES; dr++) {
		if (!(selected & (1 << dr))) {
		    drive_step (floppy + dr);
		}
	    }
	}
    }
    for (dr = 0; dr < NUM_DRIVES; dr++) {
	/* motor on/off workings tested with small assembler code on real Amiga 1200. */
	/* motor flipflop is set only when drive select goes from high to low */ 
	if (!(selected & (1 << dr)) && (lastselected & (1 << dr)) ) {
	    if (floppy[dr].motoroff) {
		/* motor off: if motor bit = 0 in prevdata or data -> turn motor on */
		if ((prevdata & 0x80) == 0 || (data & 0x80) == 0)
		    drive_motor (floppy + dr, 0);
	    } else {
		/* motor on: if motor bit = 1 in prevdata only (motor flag state in data has no effect)
		   -> turn motor off */
		if (prevdata & 0x80)
		    drive_motor (floppy + dr, 1);
	    }
	}
    }
    for (dr = 0; dr < NUM_DRIVES; dr++) {
	floppy[dr].state = (!(selected & (1 << dr))) | !floppy[dr].motoroff;
	update_drive_gui (dr);
    }
    prevdata = data;
}

uae_u8 DISK_status (void)
{
#ifdef DEBUG_DISK
    dbg("disc.c : DISK_status");
#endif
    uae_u8 st = 0x3c;
    int dr;

    for (dr = 0; dr < NUM_DRIVES; dr++) {
	drive *drv = floppy + dr;
	if (!(selected & (1 << dr))) {
	    if (drive_running (drv)) {
		if (drv->dskready)
		    st &= ~0x20;
	    } else {
		/* report drive ID */
		if (drv->drive_id & (1L << (31 - drv->drive_id_scnt)))
		    st |= 0x20;
		else
		    st &= ~0x20;
#ifdef DEBUG_DISK		
		write_log("DISK_status: sel %d id %s [0x%08lx, bit #%02d: %d]\n",
		    dr,drive_id_name(drv), drv->drive_id << drv->drive_id_scnt, 31 - drv->drive_id_scnt, st & 0x20 ? 1:0);
#endif
		/* Left shift id reg bit should be done with each LH transition of drv_sel */
		drv->drive_id_scnt++;
		drv->drive_id_scnt &= 31;
	    }
	    if (drive_track0 (drv))
		st &= ~0x10;
	    if (drive_writeprotected (drv))
		st &= ~8;
	    if (drv->dskchange)
		st &= ~4;
	} 
    }
#ifdef DEBUG_DISK
    dbgf("disc.c : DISK_status 0x%X returned\n",st);
#endif
    return st;
}

#define DISK_INDEXSYNC 1
#define DISK_WORDSYNC 2

static uae_u16 dskbytr_tab[MAX_DISK_WORDS_PER_LINE * 2 + 1];
static uae_u8 dskbytr_cycle[MAX_DISK_WORDS_PER_LINE * 2 + 1];
static short wordsync_cycle[MAX_DISK_WORDS_PER_LINE * 2 + 1];
static uae_u32 dma_tab[MAX_DISK_WORDS_PER_LINE + 1];
static uae_u8 disk_sync[MAXHPOS];
static int disk_sync_cycle;
static int dskdmaen, dsklength;
static uae_u16 dsksync;
static uae_u32 dskpt;

static void disk_dmafinished (void)
{
#ifdef DEBUG_DISK
    dbg("disc.c : disk_dmafinished");
#endif
    INTREQ (0x8002);
    dskdmaen = 0; /* ??? */
#ifdef DEBUG_DISK
    write_log("disk dma finished %08.8X\n", dskpt);
#endif
}    

static void disk_events (int last)
{
#ifdef DEBUG_DISK
    dbgf("disc.c : disk_events(%i) -> get_cycles()=%i, maxhpos=%i\n",last,get_cycles (),maxhpos);
#endif
    eventtab[ev_disk].active = 0;
    for (disk_sync_cycle = last; disk_sync_cycle < maxhpos; disk_sync_cycle++) {
	if (disk_sync[disk_sync_cycle]) {
	    eventtab[ev_disk].oldcycles = get_cycles ();
	    eventtab[ev_disk].evtime = get_cycles () + (disk_sync_cycle - last) * CYCLE_UNIT;
	    eventtab[ev_disk].active = 1;
#ifdef DEBUG_DISK
    dbgf("disc.c : evtime=%i, active=%i\n",eventtab[ev_disk].evtime,eventtab[ev_disk].active);
#endif
	    events_schedule ();
	    return;
	}
    }
}

extern void cia_diskindex (void);

void DISK_handler (void)
{
#ifdef DEBUG_DISK
    dbg("disc.c : DISK_handler");
#endif
    eventtab[ev_disk].active = 0;
    if (disk_sync[disk_sync_cycle] & DISK_WORDSYNC)
	INTREQ (0x9000);
    if (disk_sync[disk_sync_cycle] & DISK_INDEXSYNC)
	cia_diskindex ();
#ifdef DEBUG_DISK
    dbgf("disc.c : disk_sync[%i]=0\n",disk_sync_cycle);
#endif
    disk_sync[disk_sync_cycle] = 0;
    disk_events (disk_sync_cycle);
}

static int dma_enable, bitoffset;
static uae_u32 word;

/* Always carried through to the next line.  */
static int disk_hpos;

/* emulate disk write dma for full horizontal line */
static void disk_doupdate_write (drive * drv)
{
#ifdef DEBUG_DISK
    dbg("disc.c : disk_doupdate_write");
#endif
    int hpos = disk_hpos;
    while (hpos < (maxhpos << 8)) {
	drv->mfmpos++;
	drv->mfmpos %= drv->tracklen;
	if (!drv->mfmpos) {
	    disk_sync[hpos >> 8] |= DISK_INDEXSYNC;
#ifdef DEBUG_DISK
    dbgf("disc.c : disk_sync[%i] |= %i =%i\n",(hpos >> 8),DISK_INDEXSYNC,disk_sync[hpos >> 8]);
#endif
	    disk_events (0);
	}
	if ((dmacon & 0x210) == 0x210 && dskdmaen == 3) {
	    bitoffset++;
	    bitoffset &= 15;
	    if (!bitoffset) {
		drv->bigmfmbuf[drv->mfmpos >> 4] = swab_w(get_word (dskpt));
		dskpt += 2;
		dsklength--;
		if (dsklength == 0) {
		    disk_dmafinished ();
		    drive_write_data (drv);
		}
	    }
	}
	hpos += drv->trackspeed;
    }
    disk_hpos = hpos - (maxhpos << 8);
}

/* get one bit from MFM bit stream */
static uae_u32 getonebit (uae_u16 * mfmbuf, int mfmpos, uae_u32 word)
{
    uae_u16 *buf;

#ifdef DEBUG_DISK
    dbg("disc.c : getonebit");
#endif
    buf = &mfmbuf[mfmpos >> 4];
    word <<= 1;
    word |= (buf[0] & (1 << (15 - (mfmpos & 15)))) ? 1 : 0;
    return word;
}

#define WORDSYNC_CYCLES 7 /* (~7 * 280ns = 2us) */

/* emulate disk read dma for full horizontal line */
static void disk_doupdate_read (drive * drv)
{
    int hpos = disk_hpos;
    int is_sync = 0;
    int j = 0, k = 1, l = 0;
    uae_u16 synccheck;
    static int dskbytr_last = 0, wordsync_last = -1;

#ifdef DEBUG_DISK
    dbg("disc.c : disk_doupdate_read");
#endif
    dskbytr_tab[0] = dskbytr_tab[dskbytr_last];

    if (wordsync_last >= 0 && maxhpos - wordsync_cycle[wordsync_last] < WORDSYNC_CYCLES)
	wordsync_cycle[l++] = (maxhpos - wordsync_cycle[wordsync_last]) - WORDSYNC_CYCLES;
    wordsync_last = -1;

    while (hpos < (maxhpos << 8)) {
	if (drv->dskready)
	    word = getonebit (drv->bigmfmbuf, drv->mfmpos, word);
	else
	    word <<= 1;
	drv->mfmpos++;
	drv->mfmpos %= drv->tracklen;
	if (!drv->mfmpos) {
	    disk_sync[hpos >> 8] |= DISK_INDEXSYNC;
#ifdef DEBUG_DISK
    dbgf("disc.c : disk_sync[%i] |= %i =%i\n",(hpos >> 8),DISK_INDEXSYNC,disk_sync[hpos >> 8]);
#endif
	    is_sync = 1;
	}
	if (bitoffset == 31 && dma_enable) {
	    dma_tab[j++] = (word >> 16) & 0xffff;
	    if (j == MAX_DISK_WORDS_PER_LINE - 1) {
		write_log ("Bug: Disk DMA buffer overflow!\n");
		j--;
	    }
	}
	if (bitoffset == 15 || bitoffset == 23 || bitoffset == 31) {
	    dskbytr_tab[k] = (word >> 8) & 0xff;
	    dskbytr_tab[k] |= 0x8000;
	    dskbytr_last = k;
	    dskbytr_cycle[k++] = hpos >> 8;
	}
	synccheck = (word >> 8) & 0xffff;
	if (synccheck == dsksync) {
	    if (adkcon & 0x400) {
		if (bitoffset != 23 || !dma_enable)
		    bitoffset = 7;
		dma_enable = 1;
	    }
	    wordsync_last = l;
	    wordsync_cycle[l++] = hpos >> 8;
	    disk_sync[hpos >> 8] |= DISK_WORDSYNC;
#ifdef DEBUG_DISK
    dbgf("disc.c : disk_sync[%i] |= %i =%i\n",(hpos >> 8),DISK_WORDSYNC,disk_sync[hpos >> 8]);
#endif
	    is_sync = 1;
	}
	bitoffset++;
	if (bitoffset == 32) bitoffset = 16;
	hpos += drv->trackspeed;
    }
    dma_tab[j] = 0xffffffff;
    dskbytr_cycle[k] = 255;
    wordsync_cycle[l] = 255;
    if (is_sync)
	disk_events (0);

    disk_hpos = hpos - (maxhpos << 8);
}

/* disk DMA fetch happens on real Amiga at the beginning of next horizontal line
   (cycles 9, 11 and 13 according to hardware manual) We transfer all DMA'd
   data at cycle 0. I don't think any program cares about this small difference.

   We must handle dsklength = 0 because some copy protections use it to detect
   wordsync without transferring any data.
*/
static void dodmafetch (void)
{
    int i=0;

#ifdef DEBUG_DISK
    dbg("disc.c : dodmafetch");
#endif
    while (dma_tab[i] != 0xffffffff && dskdmaen == 2 && (dmacon & 0x210) == 0x210) {
	if (dsklength > 0) {
#ifdef DEBUG_DISK
	    dbgf("put_word(0x%X,0x%X)\n",dskpt, swab_w(dma_tab[i++]));
#endif
	    put_word (dskpt, swab_w(dma_tab[i++]));
	    dskpt += 2;
	    dsklength--;
	}
	if (dsklength == 0) {
	    disk_dmafinished ();
	    break;
	}
    }
    dma_tab[0] = 0xffffffff;
}

static void DISK_start (void)
{
    int dr;

#ifdef DEBUG_DISK
    dbg("disc.c : DISK_start");
#endif
    for (dr = 0; dr < NUM_DRIVES; dr++) {
	drive *drv = &floppy[dr];
	if (!(selected & (1 << dr))) {
	    int tr = drv->cyl * 2 + side;
	    trackid *ti = drv->trackdata + tr;

	    if (dskdmaen == 3) {
		drv->tracklen = FLOPPY_WRITE_LEN * drv->ddhd * 8 * 2;
		drv->trackspeed = floppy_speed;
	    }
	    /* Ugh.  A nasty hack.  Assume ADF_EXT1 tracks are always read
	       from the start.  */
	    if (ti->type == TRACK_RAW1)
		drv->mfmpos = 0;
	}
    }
    dma_enable = (adkcon & 0x400) ? 0 : 1;
    word = 0;
    bitoffset = 0;
    dma_tab[0] = 0xffffffff;
}

static int linecounter;

void DISK_update (int vpos)
{
    int dr;
    static int back_vpos=0;
    int count=vpos-back_vpos;

    back_vpos=vpos;
    if (count<=0) count=1;

#ifdef DEBUG_DISK
    dbg("disc.c : DISK_update");
#endif
while(count--) {
    for (dr = 0; dr < NUM_DRIVES; dr++) {
	drive *drv = &floppy[dr];
	if (drv->steplimit) {
	    drv->steplimit--;
	}
    }
    if (linecounter) {
	linecounter--;
	if (!linecounter) {
	    disk_dmafinished ();
	}
	continue;
    }

    dodmafetch ();

    for (dr = 0; dr < NUM_DRIVES; dr++) {
	drive *drv = &floppy[dr];
	if (drv->motoroff)
	    continue;
	if (selected & (1 << dr)) {
	    drv->mfmpos += count *((maxhpos << 8) / drv->trackspeed);
	    drv->mfmpos %= drv->tracklen;
	    continue;
	}
	drive_fill_bigbuf (drv);
	drv->mfmpos %= drv->tracklen;
	if (dskdmaen > 1)
	    disk_data_used = 0;

	if (dskdmaen == 3)
	    disk_doupdate_write (drv);
	else if (1 || disk_data_used < MAXVPOS * 50 * 2) {
	    disk_data_used++;
	    disk_doupdate_read (drv);
	}
	break;
    }
}
}

void DSKLEN (uae_u16 v, int hpos)
{
#ifdef DEBUG_DISK
    dbg("disc.c : DSKLEN");
#endif
    if (v & 0x8000) {
	dskdmaen = dskdmaen == 1 ? 2 : 1;
    } else {
	/* Megalomania does this */
	if (dskdmaen == 3)
	    write_log ("warning: Disk write DMA aborted, %d words left\n", dsklength);
	dskdmaen = 0;
    }
    dsklength = v & 0x3ffe;
    if (dskdmaen <= 1)
	return;
    if (v & 0x4000)
	dskdmaen = 3;
#ifdef DEBUG_DISK
    {
	int dr;
	for (dr = 0; dr < NUM_DRIVES; dr++) {
	    drive *drv = &floppy[dr];
	    if (drv->motoroff)
		continue;
	    if ((selected & (1 << dr)) == 0)
		break;
	}
	if (dr == 4)
	    write_log ("disk DMA started, but no drive ready!\n");
	else
	    write_log ("disk %s DMA started for %d words (%08.8X) sync=%04.4X track %d\n",
		       dskdmaen == 3 ? "write" : "read", dsklength, dskpt,
		       (adkcon & 0x400) ? dsksync : 0xffff,
		       floppy[dr].cyl * 2 + side);
    }
#endif
    /* Megalomania does not work if DSKLEN write is delayed to next horizontal line,
       also it seems some copy protections require this fix */
    DISK_start ();

    /* Try to make floppy access from Kickstart faster.  */
    if (dskdmaen != 2)
	return;
    {
	int dr;
	uaecptr pc = _68k_getpc ();
	if ((pc & 0xF80000) != 0xF80000)
	    return;
	for (dr = 0; dr < NUM_DRIVES; dr++) {
	    drive *drv = &floppy[dr];
	    if (drv->motoroff)
		continue;
	    if ((selected & (1 << dr)) == 0) {
		int pos = drv->mfmpos & ~15;
		int i;

		drive_fill_bigbuf (drv);
		if (adkcon & 0x400) {
		    for (i = 0; i < drv->tracklen; i += 16) {
			pos += 16;
			pos %= drv->tracklen;
			if (drv->bigmfmbuf[pos >> 4] == dsksync) {
			    /* must skip first disk sync marker */
			    pos += 16;
			    pos %= drv->tracklen;
			    break;
			}
		    }
		    if (i >= drv->tracklen)
			return;
		}
		while (dsklength-- > 0) {
#ifdef DEBUG_DISK
		    dbgf("put_word(0x%X,0x%X)\n",dskpt, swab_w(drv->bigmfmbuf[pos >> 4]));
#endif
		    put_word (dskpt, swab_w(drv->bigmfmbuf[pos >> 4]));
		    dskpt += 2;
		    pos += 16;
		    pos %= drv->tracklen;
		}
		INTREQ (0x9000);
		linecounter = 2;
		dskdmaen = 0;
		return;
	    }
	}
    }
}

/* this is very unoptimized. DSKBYTR is used very rarely, so it should not matter. */

uae_u16 DSKBYTR (int hpos)
{
#ifdef DEBUG_DISK
    dbg("disc.c : DSKBYTR");
#endif
    uae_u16 v;
    int i;

    i = 0;
    while (hpos > dskbytr_cycle[i + 1])
	i++;
    v = dskbytr_tab[i];
    dskbytr_tab[i] &= ~0x8000;
    if (wordsync_cycle[0] != 255) {
	i = 0;
	while (hpos < wordsync_cycle[i])
	    i++;
	if (hpos - wordsync_cycle[i] <= WORDSYNC_CYCLES)
	    v |= 0x1000;
    }
    if (dskdmaen && (dmacon & 0x210) == 0x210)
	v |= 0x4000;
    if (dskdmaen == 3)
	v |= 0x2000;

    disk_data_used = 0;

    return v;
}

/* not a real hardware register */
uae_u16 DSKDATR (int hpos)
{
#ifdef DEBUG_DISK
    dbg("disc.c : DSKDATR");
#endif
    return 0;
}

void DSKSYNC (uae_u16 v)
{
#ifdef DEBUG_DISK
    dbg("disc.c : DSKSYNC");
#endif
    dsksync = v;
}

void DSKDAT (uae_u16 v)
{
#ifdef DEBUG_DISK
    dbg("disc.c : DSKDAT");
#endif
    static int count = 0;
    if (count < 5) {
	count++;
	write_log ("DSKDAT written. Not good.");
	if (count == 5)
	    write_log ("(further messages suppressed)");

	write_log ("\n");
    }
}
void DSKPTH (uae_u16 v)
{
#ifdef DEBUG_DISK
    dbg("disc.c : DSKPTH");
#endif
    dskpt = (dskpt & 0xffff) | ((uae_u32) v << 16);
}

void DSKPTL (uae_u16 v)
{
#ifdef DEBUG_DISK
    dbg("disc.c : DSKPTL");
#endif
    dskpt = (dskpt & ~0xffff) | (v);
}

void DISK_init (void)
{
#ifdef DEBUG_DISK
    dbg("disc.c : DISK_init");
#endif
    int dr;

    dma_tab[0] = 0xffffffff;
    dskbytr_cycle[0] = 0;
    dskbytr_cycle[1] = 255;
    wordsync_cycle[0] = 255;

    for (dr = 0; dr < NUM_DRIVES; dr++) {
	drive *drv = &floppy[dr];
	/* reset all drive types to 3.5 DD */
	drive_settype_id (drv);
	if (!drive_insert (drv, dr, prefs_df[dr]))
	    disk_eject (dr);
    }
    if (disk_empty (0))
	write_log ("No disk in drive 0.\n");
}

void DISK_reset (void)
{
#ifdef DEBUG_DISK
    dbg("disc.c : DISK_reset");
#endif
    int i;

    if (savestate_state)
	    return;

    disk_hpos = 0;
    disk_data_used = 0;
    disabled = 0;
    for (i = NUM_DRIVES; i < 4; i++)
	disabled |= 1 << i;
    dma_tab[0] = 0xffffffff;
    dskbytr_cycle[0] = 0;
    dskbytr_cycle[1] = 255;
    wordsync_cycle[0] = 255;
    for (i = 0 ; i < NUM_DRIVES; i++)
    {
	    drive *drv = &floppy[i];
	    drv->cyl=0;
	    drv->state=0;
            drv->dskchange = 1;
	    drv->mfmpos = 0;
    }
}


/* Disk save/restore code */

void DISK_save_custom (uae_u32 *pdskpt, uae_u16 *pdsklength, uae_u16 *pdsksync, uae_u16 *pdskdatr, uae_u16 *pdskbytr)
{
    if(pdskpt) *pdskpt = dskpt;
    if(pdsklength) *pdsklength = dsklength;
    if(pdsksync) *pdsksync = dsksync;
    if(pdskdatr) *pdskdatr = 0;
    if(pdskbytr) *pdskbytr = dskbytr_tab[0];
}

void DISK_restore_custom (uae_u32 pdskpt, uae_u16 pdsklength, uae_u16 pdskdatr, uae_u16 pdskbytr)
{
    dskpt = pdskpt;
    dsklength = pdsklength;
    dskbytr_tab[0] = pdskbytr;
}

uae_u8 *restore_disk(int num,uae_u8 *src)
{
    drive *drv;
    uae_u32 drive_id, mfmpos;
    int state;
    uae_u8 cyl, dskready, drive_id_scnt;

    drive_id=restore_u32 ();
    state=restore_u8 ();
    cyl=restore_u8 ();
    dskready=restore_u8 ();
    drive_id_scnt=restore_u8 ();
    mfmpos = restore_u32 ();
    restore_u32 ();

    if (num<NUM_DRIVES)
    {
    	drv = &floppy[num];
    	disabled &= ~(1 << num);
  	  drv->drive_id = drive_id;
    	if (state & 2)
       		disabled |= 1 << num;
    	else
       		drv->motoroff = (state & 1) ? 0 : 1;
    	drv->cyl = cyl;
    	drv->dskready = dskready;
    	drv->drive_id_scnt = drive_id_scnt;
    	drv->mfmpos = mfmpos;
    	strncpy(changed_df[num],(char *)src,127);
    	changed_df[num][127] = 0;
	{
		FILE *f=fopen(changed_df[num],"rb");
		extern char uae4all_image_file[];
		extern char uae4all_image_file2[];
		if (f)
		{
			fclose(f);
			if (!num)
			{
				if (strcmp(uae4all_image_file,changed_df[0]))
				{
    					strcpy(uae4all_image_file,changed_df[0]);
					real_changed_df[num] = 1;
				}
			}
 			else
			{
				if (strcmp(uae4all_image_file2,changed_df[1]))
				{
					strcpy(uae4all_image_file2,changed_df[1]);
					real_changed_df[num] = 1;
				}
			}
		}
	}
    }
    else
    {
 	    extern int mainMenu_throttle;
	    m68k_speed=mainMenu_throttle=mfmpos;
	    check_prefs_changed_cpu();
    }

    src = (uae_u8 *)(((unsigned)src)+strlen((char *)src) + 1);

    return src;
}

uae_u8 *save_disk(int num,int *len)
{
    uae_u8 *dstbak,*dst;
    drive *drv;

    dstbak = dst = (uae_u8 *)malloc (2+1+1+1+1+4+4+256);
    if (num<NUM_DRIVES)
    {
    	drv = &floppy[num];
    	save_u32 (drv->drive_id);	    /* drive type ID */
    	save_u8 ((drv->motoroff ? 0:1) | ((disabled & (1 << num)) ? 2 : 0));  /* state */
    	save_u8 (drv->cyl);		    /* cylinder */
    	save_u8 (drv->dskready);	    /* dskready */
    	save_u8 (drv->drive_id_scnt);   /* id mode position */
    	save_u32 (drv->mfmpos);	    /* disk position */
    	save_u32 (0);		    /* CRC of disk image */
    	strcpy ((char *)dst, prefs_df[num]);/* image name */
    }
    else
    {
	save_u32 (DRIVE_ID_NONE);	/* drive type ID */
	save_u8 (((disabled & (1 << num)) ? 2 : 0)); /* state */
	save_u8 (0);	/* cylinder */
	save_u8 (0);	/* dskready */
	save_u8 (0);	/* id mode position */
	save_u32 (m68k_speed);	/* disk position */
	save_u32 (0);	/* CRC of disk image */
	dst[0]=dst[1]=0;
    }

    dst = (uae_u8 *)(((unsigned)dst)+strlen((char *)dst) + 1);
    *len = dst - dstbak;
    return dstbak;
}

/* internal floppy controller variables */

uae_u8 *restore_floppy(uae_u8 *src)
{
    word = restore_u16();
    bitoffset = restore_u8();
    dma_enable = restore_u8();
    disk_hpos = restore_u8();
    dskdmaen = restore_u8();
    word |= restore_u16() << 16;

    return src;
}

uae_u8 *save_floppy(int *len)
{
    uae_u8 *dstbak, *dst;

    /* flush dma buffer before saving */
    dodmafetch();

    dstbak = dst = (uae_u8 *)malloc(2+1+1+1+1+2);
    save_u16 (word);		/* current fifo (low word) */
    save_u8 (bitoffset);	/* dma bit offset */
    save_u8 (dma_enable);	/* disk sync found */
    save_u8 (disk_hpos);	/* next bit read position */
    save_u8 (dskdmaen);		/* dma status */
    save_u16 (word >> 16);	/* current fifo (high word) */

    *len = dst - dstbak;
    return dstbak;
}
