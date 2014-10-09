/*
 * Data used for communication between custom.c and drawing.c.
 * 
 * Copyright 1996-1998 Bernd Schmidt
 */

#define MAX_PLANES 6

#define RES_LORES 0
#define RES_HIRES 1
#define RES_SUPERHIRES 2

#ifndef UAE4ALL_ALIGN
#error UAE4ALL_ALIGN NO DEFINIDO
#endif

/* calculate shift depending on resolution (replaced "decided_hires ? 4 : 8") (TW) */
#define RES_SHIFT(res) ((res) == RES_LORES ? 8 : (res) == RES_HIRES ? 4 : 2)

/* According to the HRM, pixel data spends a couple of cycles somewhere in the chips
   before it appears on-screen.  */
#define DIW_DDF_OFFSET 9

/* We ignore that many lores pixels at the start of the display. These are
 * invisible anyway due to hardware DDF limits. */
#define DISPLAY_LEFT_SHIFT 0x40
#define PIXEL_XPOS(HPOS) ((((HPOS)<<1) - DISPLAY_LEFT_SHIFT + DIW_DDF_OFFSET - 1) )

//#define max_diwlastword (PIXEL_XPOS(maxhpos) + 16)
#define max_diwlastword (PIXEL_XPOS(maxhpos) - 6)

// extern int sprite_width;

static __inline__ int coord_hw_to_window_x (int x)
{
    x -= DISPLAY_LEFT_SHIFT;
    return x;
}

static __inline__ int coord_window_to_hw_x (int x)
{
    return x + DISPLAY_LEFT_SHIFT;
}

static __inline__ int coord_diw_to_window_x (int x)
{
    return (x - DISPLAY_LEFT_SHIFT + DIW_DDF_OFFSET - 1);
}

static __inline__ int coord_window_to_diw_x (int x)
{
    x = coord_window_to_hw_x (x);
    return x - DIW_DDF_OFFSET;
}

extern int framecnt;


/* color values in two formats: 12 (OCS/ECS) or 24 (AJA) bit Amiga RGB (color_uae_regs),
 * and the native color value; both for each Amiga hardware color register. 
 *
 * !!! See color_reg_xxx functions below before touching !!!
 */
struct color_entry {
    uae_u16 color_uae_regs_ecs[32];
    xcolnr acolors[256];
} UAE4ALL_ALIGN;


static __inline__ xcolnr getxcolor (int c)
{
	return xcolors[c];
}

static __inline__ int color_reg_get (struct color_entry *_GCCRES_ ce, int c)
{
	return ce->color_uae_regs_ecs[c];
}

static __inline__ void color_reg_set (struct color_entry *_GCCRES_ ce, int c, int v)
{
	ce->color_uae_regs_ecs[c] = v;
}

static __inline__ int color_reg_cmp (struct color_entry *_GCCRES_ ce1, struct color_entry *_GCCRES_ ce2)
{
	return memcmp (ce1->color_uae_regs_ecs, ce2->color_uae_regs_ecs, sizeof (uae_u16) * 32);    
}
/* ugly copy hack, is there better solution? */
static __inline__ void color_reg_cpy (struct color_entry *_GCCRES_ dst, struct color_entry *_GCCRES_ src)
{
	/* copy first 32 acolors and color_uae_regs_ecs */
	uae4all_memcpy(dst->color_uae_regs_ecs, src->color_uae_regs_ecs,
		sizeof(struct color_entry) - sizeof(xcolnr) * (256-32));
}

/*
 * The idea behind this code is that at some point during each horizontal
 * line, we decide how to draw this line. There are many more-or-less
 * independent decisions, each of which can be taken at a different horizontal
 * position.
 * Sprites and color changes are handled specially: There isn't a single decision,
 * but a list of structures containing information on how to draw the line.
 */

struct color_change {
    int linepos;
    int regno;
    unsigned long value;
} UAE4ALL_ALIGN;

/* 440 rather than 880, since sprites are always lores.  */
#define MAX_PIXELS_PER_LINE 1760

/* No divisors for MAX_PIXELS_PER_LINE; we support AJA and may one day
   want to use SHRES sprites.  */
#define MAX_SPR_PIXELS (((MAXVPOS + 1)*2 + 1) * MAX_PIXELS_PER_LINE)

struct sprite_entry
{
    unsigned short pos;
    unsigned short max;
    unsigned int first_pixel;
    unsigned int has_attached;
} UAE4ALL_ALIGN;

union sps_union {
    uae_u8 bytes[2 * MAX_SPR_PIXELS];
    uae_u32 words[2 * MAX_SPR_PIXELS / 4];
} UAE4ALL_ALIGN;

extern union sps_union spixstate;
extern uae_u16 spixels[MAX_SPR_PIXELS * 2];

/* Way too much... */
#define MAX_REG_CHANGE ((MAXVPOS + 1) * 2 * MAXHPOS)

#ifdef OS_WITHOUT_MEMORY_MANAGEMENT
extern struct color_change *color_changes[2];
#else
extern struct color_change color_changes[2][MAX_REG_CHANGE];
#endif

extern struct color_entry color_tables[2][(MAXVPOS+1) * 2];
extern struct color_entry *curr_color_tables, *prev_color_tables;

extern struct sprite_entry *curr_sprite_entries, *prev_sprite_entries;
extern struct color_change *curr_color_changes, *prev_color_changes;
extern struct draw_info *curr_drawinfo, *prev_drawinfo;

/* struct decision contains things we save across drawing frames for
 * comparison (smart update stuff). */
struct decision {
    /* Records the leftmost access of BPL1DAT.  */
    int plfleft, plfright, plflinelen;
    /* Display window: native coordinates, depend on lores state.  */
    int diwfirstword, diwlastword;
    int ctable;

    uae_u16 bplcon0, bplcon2;
    uae_u16 bplcon3, bplcon4;
    uae_u8 nr_planes;
    uae_u8 bplres;
} UAE4ALL_ALIGN;

/* Anything related to changes in hw registers during the DDF for one
 * line. */
struct draw_info {
    int first_sprite_entry, last_sprite_entry;
    int first_color_change, last_color_change;
    int nr_color_changes, nr_sprites;
} UAE4ALL_ALIGN;

// extern int next_sprite_entry;

extern struct decision line_decisions[2 * (MAXVPOS+1) + 1];
extern struct draw_info line_drawinfo[2][2 * (MAXVPOS+1) + 1];

extern uae_u8 line_data[(MAXVPOS+1) * 2][MAX_PLANES * MAX_WORDS_PER_LINE * 2];

extern uae_u8 *real_bplpt[8];

/* Functions in drawing.c.  */
extern int coord_native_to_amiga_y (int);
extern int coord_native_to_amiga_x (int);

extern void record_diw_line (int first, int last);
extern void hardware_line_completed (int lineno);

/* Determine how to draw a scan line.  */
enum nln_how {
    /* All lines on a non-doubled display. */
    nln_normal,
    /* Non-interlace, doubled display.  */
    nln_doubled,
    /* Interlace, doubled display, upper line.  */
    nln_upper,
    /* Interlace, doubled display, lower line.  */
    nln_lower,
    /* This line normal, next one black.  */
    nln_nblack
};

#if defined(USE_LINESTATE) || !defined(USE_ALL_LINES)
extern void hsync_record_line_state (int lineno, int changed);
#else
#define hsync_record_line_state(LN,CH)
#endif
extern void vsync_handle_redraw (int long_frame, int lof_changed);
extern void init_hardware_for_drawing_frame (void);
extern void reset_drawing (void);
extern void reset_screen_pointers (void);
extern void drawing_init (void);
extern void notice_interlace_seen (void);

/* Finally, stuff that shouldn't really be shared.  */

extern int thisframe_first_drawn_line, thisframe_last_drawn_line;
extern int diwfirstword,diwlastword;

#define IHF_SCROLLLOCK 0
#define IHF_QUIT_PROGRAM 1
#define IHF_SOUNDADJUST 3

extern int inhibit_frame;

#define set_inhibit_frame(NBIT) inhibit_frame |= 1 << NBIT
#define clear_inhibit_frame(NBIT) inhibit_frame &= ~(1 << NBIT)
#define toggle_inhibit_frame (NBIT) inhibit_frame ^= 1 << NBIT

void reset_frameskip(void);
void check_all_prefs(void);
void delay_frameskip(unsigned);
