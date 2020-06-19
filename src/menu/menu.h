#include<stdio.h>
#include<SDL/SDL.h>

extern SDL_Surface *prSDLScreen;

#define MENU_FILE_BACKGROUND DATA_PREFIX "bg_top.png"
#define MENU_FILE_WINDOW DATA_PREFIX "window.bmp"
#define MENU_FILE_TEXT DATA_PREFIX "8x8font.bmp"
#ifdef DREAMCAST
#ifdef AUTO_RUN
#define MENU_DIR_DEFAULT "/cd/"
#else
#define MENU_DIR_DEFAULT "/"
#endif
#elif defined(_3DS)
//#define MENU_DIR_DEFAULT "/media/"
#define MENU_DIR_DEFAULT "."	// 3DS
#else
#define MENU_DIR_DEFAULT "."
#endif

enum SaveMode {
	MODE_SAVE,
	MODE_LOAD
};

enum DiskOrder
{
	DF_0,
	DF_1,
};

typedef enum {
	MB_NONE,
	MB_OK,
	MB_YESNO
} mb_mode;

enum str_alignment {
	ALIGN_LEFT,
	ALIGN_RIGHT,
	ALIGN_CENTER
};

enum font_size {
	FONT_NORMAL
};

typedef struct {
	int frame;
	int waittimeout;
	int offset;
} Scrollstatus;

void text_draw_background();
void init_text(int splash);
void quit_text(void);
void write_text(int x, int y, const char *str);
void write_text_pos(int x, int y, const char *str);
void write_text_inv(int x, int y, const char *str);
void write_text_inv_pos(int x, int y, const char *str);
void write_text_inv_n(int x, int y, int n, const char *str);
void write_centered_text(int y, const char *str);
void write_num(int x, int y, int v);
void write_num_inv(int x, int y, int v);
void text_draw_window(int x, int y, int w, int h, const char *title);
void text_draw_barra(int x, int y, int w, int h, int per, int max);
void text_draw_window_bar(int x, int y, int w, int h, int per, int max, const char *title);
void _write_text(SDL_Surface *sf, int x, int y, const char *str);
void _write_text_pos(SDL_Surface *sf, int x, int y, const char *str);
void _write_text_inv(SDL_Surface *sf, int x, int y, const char *str);
void _write_text_inv_n(SDL_Surface *sf, int x, int y, int n, const char *str);
void _write_centered_text(SDL_Surface *sf, int y, char * str);
void _write_num(SDL_Surface *sf, int x, int y, int v);
void _write_num_inv(SDL_Surface *sf, int x, int y, int v);
void _text_draw_window(SDL_Surface *sf, int x, int y, int w, int h, const char *title);
void _text_draw_window_bar(SDL_Surface *sf, int x, int y, int w, int h, int per, int max, const char *title);
void write_text_full (SDL_Surface *s, const char *str, int x, int y, int maxchars, enum str_alignment align, enum font_size size, Uint32 col, int inv);
int text_messagebox(char *title, char *text, mb_mode mode);
void write_text_pos_scroll(Scrollstatus *ss, int invers, int nr, ... );
void draw_scrollbar(int x, int y, int w, int h, int total, int visible, int offset);

// void text_draw_menu_msg();
void text_flip(void);

#define NUM_FAV 10
extern char *favorites[NUM_FAV];
extern void menu_addFavImage(char *path);
extern void menu_load_favorites(char *s);
extern char *menu_save_favorites();

void drawPleaseWait(void);
//void menu_raise(void);
//void menu_unraise(void);
#define menu_raise(x)
#define menu_unraise(x)

int run_mainMenu();
int run_menuDfSel();
int run_menuLoad(enum DiskOrder new_df_num);
int run_menuSave(SaveMode m);
int run_menuGame();
int run_menuControl();

extern int mainMenu_vpos;
extern int mainMenu_throttle;
extern int mainMenu_frameskip;
extern int mainMenu_sound;
extern int mainMenu_autosave;
extern int mainMenu_msens;
extern int mainMenu_mappos;
extern int mainMenu_max_tap_time;
extern int mainMenu_click_time;
extern int mainMenu_single_tap_timeout;
extern int mainMenu_max_double_tap_time;
extern int mainMenu_locked_drag_timeout;
extern int mainMenu_tap_and_drag_gesture;
extern int mainMenu_locked_drags;

extern Uint32 menu_text_color;			// text
extern Uint32 menu_text_color_inactive;	// inactive text
extern Uint32 menu_inv_color;			// blink text underlay
extern Uint32 menu_inv2_color;			// light text underlay (almost like background)
extern Uint32 menu_win0_color;			// window dropshadow
extern Uint32 menu_win1_color;			// window frame

extern void menu_set_text_color(Uint32 c);
extern void menu_restore_text_color();
