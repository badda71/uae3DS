#include "sysconfig.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "config.h"
#include "menu.h"

#include <SDL/SDL_mixer.h>
#include <SDL/SDL_image.h>

#include "sysdeps.h"
#include "uae.h"
#include "options.h"
#include "zfile.h"
#include "sound.h"

#include "uibottom.h"
#include "keyboard.h"

#ifdef HOME_DIR
#include "homedir.h"
#endif

#ifdef DREAMCAST
void reinit_sdcard(void);
#define VIDEO_FLAGS_INIT SDL_HWSURFACE|SDL_FULLSCREEN
#else
#define VIDEO_FLAGS_INIT SDL_HWSURFACE
#endif

#ifdef DOUBLEBUFFER
#define VIDEO_FLAGS VIDEO_FLAGS_INIT | SDL_DOUBLEBUF
#else
#define VIDEO_FLAGS VIDEO_FLAGS_INIT
#endif

SDL_Surface *text_screen=NULL, *text_image, *text_background, *text_window_background;

void write_text_pos(int x, int y, const char* str);
void write_num(int x, int y, int v);
int menu_msg_pos=330;
int menu_moving=1;
Uint32 menu_msg_time=0x12345678;

#ifdef DREAMCAST
extern int __sdl_dc_emulate_keyboard;
#endif

Uint32 menu_text_color=0;			// text
Uint32 menu_text_color_inactive=0;	// inactive text
Uint32 menu_inv_color=0;			// dark text underlay
Uint32 menu_inv2_color=0;			// light text underlay (almost like background)
Uint32 menu_win0_color=0;			// window dropshadow
Uint32 menu_win1_color=0;			// window frame
Uint32 menu_barra0_color=0;			// progress bar color 0
Uint32 menu_barra1_color=0;			// progress bar color 1

static void obten_colores(void)
{
	menu_text_color=SDL_MapRGB(text_screen->format, 0x30, 0x30, 0x30);
	menu_text_color_inactive=SDL_MapRGB(text_screen->format, 0x80, 0x80, 0x80);
	menu_inv_color=SDL_MapRGB(text_screen->format, 0x9a, 0x9a, 0xaa);
	menu_inv2_color=SDL_MapRGB(text_screen->format, 0xce, 0xcf, 0xce);
	menu_win0_color=SDL_MapRGB(text_screen->format, 0x20, 0x20, 0x30);
	menu_win1_color=SDL_MapRGB(text_screen->format, 0x70, 0x70, 0x80);
	menu_barra0_color=SDL_MapRGB(text_screen->format, 0x40, 0x40, 0x50);
	menu_barra1_color=SDL_MapRGB(text_screen->format, 0x80, 0x80, 0x90);
}

/*
void menu_raise(void)
{
	int i;
	for(i=80;i>=0;i-=16)
	{
#ifdef MENU_MUSIC
		Mix_VolumeMusic(MUSIC_VOLUME-(i<<1));
#endif
		text_draw_background();
		fade16(text_screen,i);
		text_flip();
		SDL_Delay(10);
	}
}

void menu_unraise(void)
{
	int i;
	for(i=0;i<=80;i+=16)
	{
#ifdef MENU_MUSIC
		Mix_VolumeMusic(MUSIC_VOLUME-(i<<1));
#endif
		text_draw_background();
		fade16(text_screen,i);
		text_flip();
		SDL_Delay(10);
	}
}
*/

void text_draw_background()
{
	SDL_BlitSurface(text_background,NULL,text_screen,NULL);
}

void text_flip(void)
{
	SDL_Delay(10);
	SDL_BlitSurface(text_screen,NULL,prSDLScreen,NULL);
	uib_update();
	SDL_Flip(prSDLScreen);
}

void init_text(int splash)
{
	SDL_Surface *tmp;

	if (prSDLScreen==NULL)
	{
		SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK);
		prSDLScreen=SDL_SetVideoMode(400,240,16,VIDEO_FLAGS);
    		SDL_ShowCursor(SDL_DISABLE);
 	   	SDL_JoystickEventState(SDL_ENABLE);
    		SDL_JoystickOpen(0);
	}

	if (!text_screen)
	{
		text_screen=SDL_CreateRGBSurface(prSDLScreen->flags,prSDLScreen->w,prSDLScreen->h,prSDLScreen->format->BitsPerPixel,prSDLScreen->format->Rmask,prSDLScreen->format->Gmask,prSDLScreen->format->Bmask,prSDLScreen->format->Amask);
		text_image=SDL_LoadBMP(MENU_FILE_TEXT);
		if (text_screen==NULL)
			exit(-1);
		if (text_image==NULL)
			exit(-2);
		SDL_SetColorKey(text_image, SDL_SRCCOLORKEY, 0x00000000);
		tmp=IMG_Load(MENU_FILE_BACKGROUND);
		if (tmp==NULL)
			exit(-3);
		text_background=SDL_DisplayFormat(tmp);
		SDL_FreeSurface(tmp);
		if (text_background==NULL)
			exit(-3);
		tmp=SDL_LoadBMP(MENU_FILE_WINDOW);
		if (tmp==NULL)
			exit(-4);
		text_window_background=SDL_DisplayFormat(tmp);
		SDL_FreeSurface(tmp);
		if (text_window_background==NULL)
			exit(-5);
	}
	if (splash)
	{
		int toexit=0;

		obten_colores();
		uae4all_init_sound();
		toexit = 0;

		while(!toexit)
		{
			if (!uae4all_init_rom(romfile))
				break;

			char buf[128];
			snprintf(buf,128,"kick.rom not found in\n%s",config_dir);
			text_messagebox("--- ERROR ---", buf, MB_OK);
			toexit=1;
		}

		if(toexit)
		{
			SDL_Quit();
			exit(1);
		}
	}
	else
	{
	//	SDL_FillRect(text_screen,NULL,0xFFFFFFFF);
	//	text_flip();
		uae4all_resume_music();
	}
	menu_msg_time=SDL_GetTicks();
}


void quit_text(void)
{
/*
	SDL_FreeSurface(text_image);
	SDL_FreeSurface(text_background);
	SDL_FreeSurface(text_window_background);
	SDL_FreeSurface(text_screen);
*/
}

typedef struct {
	SDL_Surface *img;
	int w;
	int h;
} font_info;

void get_font_info(font_info *f, enum font_size size) {
	switch (size) {
		default:
			f->img = text_image;
			f->w = f->h = 8;
	}
}

void write_text_full (SDL_Surface *s, const char *str, int x, int y, int maxchars, enum str_alignment align, enum font_size size, Uint32 col, int inv) {
	int xof, w;
	if (str==NULL || str[0]==0) return;
	font_info f;
	w=strlen(str);
	if (maxchars > 0 && w > maxchars) w=maxchars;
	get_font_info(&f,size);

	switch (align) {
		case ALIGN_CENTER:
			xof=x-(w*f.w/2);
			break;
		case ALIGN_RIGHT:
			xof=x-w*f.w;
			break;
		default:
			xof=x;
	}

	if (inv) {
		SDL_Rect dest;
		dest.x = xof - 2;
		dest.y = y - 2;
		dest.w = (strlen(str) * 8) + 4;
		dest.h = 12;
		SDL_FillRect(s, &dest, menu_inv_color);
	}

	int c;
	SDL_Color scol;
	SDL_GetRGB(col, text_screen->format, &(scol.r), &(scol.g), &(scol.b));
	SDL_SetPalette(f.img, SDL_LOGPAL, &scol, 1, 1);
	for (int i=0; i < w; i++) {
		c=(str[i] & 0x7f)-32;
		if (c<0) c=0;
		SDL_BlitSurface(
			f.img,
			&(SDL_Rect){.x=(c&0x0f)*f.w, .y=(c>>4)*f.h, .w=f.w, .h=f.h},
			s,
			&(SDL_Rect){.x = xof+i*f.w, .y = y});
	}
	SDL_SetPalette(f.img, SDL_LOGPAL, &(SDL_Color){0xff,0xff,0xff,0}, 1, 1);
}

void _write_text_pos(SDL_Surface *sf, int x, int y, const char *str)
{
	write_text_full (sf, str, x, y, 0, ALIGN_LEFT, FONT_NORMAL, menu_text_color, 0);
}

void write_text_pos(int x, int y, const char *str)
{
	_write_text_pos(text_screen, x, y, str);
}

void _write_text(SDL_Surface *sf, int x, int y, const char *str)
{
	_write_text_pos(sf, x*8, y*8, str);
}

void write_text(int x, int y, const char *str)
{
	_write_text(text_screen, x, y, str);
}

/* Write text, inverted: */
void write_text_inv_pos(int x, int y, const char *str)
{
  write_text_full (text_screen, str, x, y, 0, ALIGN_LEFT, FONT_NORMAL, menu_text_color, 1);
}

void write_text_inv(int x, int y, const char *str)
{
  write_text_inv_pos(x*8, y*8, str);
}

void _write_text_inv(SDL_Surface *sf, int x, int y, const char *str)
{
	SDL_Surface *back=text_screen;
	text_screen=sf;
	write_text_inv(x,y,str);
	text_screen=back;
}

// ... is list of scrollargs {int x, int y, int width, char *str}
void write_text_pos_scroll(Scrollstatus *ss, int invers, int nr, ... ) 
{
	const int speed = 6;
	const int wait = 6;
	int i, x, y, w, nrend = nr, offset;
	char *str;

	va_list vl;
	va_start(vl,nr);
	for (i=0; i < nr; ++i) {
		x = va_arg(vl, int);
		y = va_arg(vl, int);
		w = va_arg(vl, int);
		str = va_arg(vl, char*);
		char text[w+1];
		if (!str) continue;
		int len = strlen(str);
		if (len <= w) {
			if (invers) write_text_inv_pos(x,y,str);
			else write_text_pos(x,y,str);
			continue;
		}

		if (ss->offset >= len - w) {
			offset = len - w;
		} else {
			--nrend;
			offset = ss->offset;
		}
		snprintf(text, w+1, "%s", str + offset);
		if (invers) write_text_inv_pos(x,y,text);
		else write_text_pos(x,y,text);
	}
	va_end(vl);

	// advance step
	ss->frame = (ss->frame + 1) % speed;
	if (ss->frame == 0) {
		// advance step
		if (ss->waittimeout) {
			--ss->waittimeout;
			if (!ss->waittimeout) {
				if (ss->offset) {
					ss->offset = 0;
				} else {
					++ss->offset;
				}
			}
		} else {
			if (nrend == nr)
				ss->waittimeout = wait;
			else if (ss->offset == 0)
				ss->waittimeout = wait;
			else
				++ss->offset;
		}
	}
}

/* Write text, horizontally centered... */

void write_centered_text(int y, const char *str)
{
  write_text(20 - (strlen(str) / 2), y/2, str);
}

void _write_centered_text(SDL_Surface *sf, int x, int y, const char *str)
{
	SDL_Surface *back=text_screen;
	text_screen=sf;
	write_centered_text(y,str);
	text_screen=back;
}

/* Write numbers on the option prSDLScreen: */

void write_num(int x, int y, int v)
{
  char str[24];
  
  sprintf(str, "%d", v);
  write_text(x, y, str);
}

void _write_num(SDL_Surface *sf, int x, int y, int v)
{
	SDL_Surface *back=text_screen;
	text_screen=sf;
	write_num(x,y,v);
	text_screen=back;
}

void write_num_inv(int x, int y, int v)
{
  SDL_Rect dest;
  int i,l=1;

  for(i=10;i<1000000;i*=10)
	if (v/i)
		l++;
  	else
		break;
  	
  dest.x = (x * 8) -2 ;
  dest.y = (y * 8) /*10*/ - 2;
  dest.w = (l * 8) + 4;
  dest.h = 12;

  SDL_FillRect(text_screen, &dest, menu_inv_color);

  write_num(x, y, v);
}

void _write_num_inv(SDL_Surface *sf, int x, int y, int v)
{
	SDL_Surface *back=text_screen;
	text_screen=sf;
	write_num_inv(x,y,v);
	text_screen=back;
}

void text_draw_window(int x, int y, int w, int h, const char *title)
{
	int i,j;
	SDL_Rect dest;

	x=(x/8)*8;
	y=(y/8)*8;
	w=(w/8)*8;
	h=(h/8)*8;

	dest.x = x + 6;
	dest.y = y - 4;
	dest.w = w + 6;
	dest.h = h + 18;
	SDL_FillRect(text_screen, &dest, menu_win0_color);

	dest.x = x - 2;
	dest.y = y - 10; //12;
	dest.w = w + 4;
	dest.h = h + 14; //16;
	SDL_FillRect(text_screen, &dest, menu_win1_color);

	SDL_SetClipRect(text_screen, &(SDL_Rect){.x=x, .y=y, .w=w, .h=h});
	for(i=0;i<w;i+=32)
		for(j=0;j<h;j+=24)
		{
			dest.x=x+i;
			dest.y=y+j;
			dest.w=32;
			dest.h=24;
			SDL_BlitSurface(text_window_background,NULL,text_screen,&dest);
		}
	SDL_SetClipRect(text_screen, NULL);

	write_text(x / 8, y / 8 - 1, "OOO");
	write_text(x / 8 + MAX(((w / 8 -strlen(title)) / 2), 4), y / 8 - 1, title);
}

void _text_draw_window(SDL_Surface *sf, int x, int y, int w, int h, const char *title)
{
	SDL_Surface *back=text_screen;
	text_screen=sf;
	text_draw_window(x,y,w,h,title);
	text_screen=back;
}

void text_draw_barra(int x, int y, int w, int h, int per, int max)
{
	SDL_Rect dest;
if (h>5) h-=4;
	dest.x=x-1;
	dest.y=y-1;
	dest.w=w+2;
	dest.h=h+2;
	SDL_FillRect(text_screen, &dest, menu_barra1_color); //0xdddd);
	if (per>max) per=max;
	dest.x=x;
	dest.y=y;
	dest.h=h;
	dest.w=(w*per)/max;
	SDL_FillRect(text_screen, &dest, menu_barra0_color); //0x8888);
}

void text_draw_window_bar(int x, int y, int w, int h, int per, int max, const char *title)
{
	text_draw_window(x,y,w,h,title);
	text_draw_barra(x+4, y+28, w-24, 12, per, max);
	write_text((x/8)+4,(y/8)+1,"Please wait");
}

void _text_draw_window_bar(SDL_Surface *sf, int x, int y, int w, int h, int per, int max, const char *title)
{
	SDL_Surface *back=text_screen;
	text_screen=sf;
	text_draw_window_bar(x,y,w,h,per,max,title);
	text_screen=back;
}

#define FONT_H 8
#define FONT_W 8
#define MSGBOX_PADDING 8

int text_messagebox(char *title, char *message, mb_mode mode) {
	int height = 1;
	int width = strlen(title) + 4,w;
	char buf[200];
	char *c;
	extern SDL_Surface *text_screen;

	int maxwidth = (text_screen->w - MSGBOX_PADDING * 2) / FONT_W - 2;
	int maxheight = text_screen->h / FONT_H - 3 - MSGBOX_PADDING * 2 / FONT_H;;

	for (c=message, w=0; *c!=0; c++)
	{
		if (*c == '\n')
		{
			++height;
			if (width<w) width=w;
			w=0;
		}
		else
		{
			++w;
			if (w > maxwidth) {
 				++height;
				width=maxwidth;
				w=0;
			}
		}
	}
	if (width<w) width=w;
	height += mode==MB_NONE ? 0 : 2;
	if (mode == MB_OK && width<4) width=4;
	if (mode == MB_YESNO && width<9) width=9;
	if (width > maxwidth) width = maxwidth;
	if (height > maxwidth) height = maxheight;

	int x= (text_screen->w / FONT_W - width) / 2;
	int y= (text_screen->h / FONT_H - height) / 2;
	
	int selected=0;
	int frame = 0;
	do {
		int yo=0;
		SDL_Event e;
		text_draw_background();
		text_draw_window(x * FONT_W - MSGBOX_PADDING, y * FONT_H - MSGBOX_PADDING, width*FONT_W + MSGBOX_PADDING*2, height*FONT_H + MSGBOX_PADDING*2, title);

		char *n=NULL;
		for (c=message; *c!=0; c=n)
		{
			n=strchr(c,'\n');
			if (!n) n=c+strlen(c);
			if (n-c > width) n=c+width;
			snprintf(buf, n-c+1, "%s", c);
			write_text(x, y+yo, buf);
			++yo;
			if (*n == '\n') ++n;
		}

		int flash = frame / 3;

		if (mode != MB_NONE &&
			SDL_PollEvent(&e) &&
			!uib_handle_event(&e) &&
			e.type==SDL_KEYDOWN)
		{
			switch (e.key.keysym.sym) {
			case DS_RIGHT1:
			case DS_RIGHT2:
			case DS_RIGHT3:
			case AK_RT:
			case DS_DOWN1:
			case DS_DOWN2:
			case DS_DOWN3:
			case AK_DN:
			case DS_LEFT1:
			case DS_LEFT2:
			case DS_LEFT3:
			case AK_LF:
			case DS_UP1:
			case DS_UP2:
			case DS_UP3:
			case AK_UP: if (mode == MB_YESNO) selected = (selected+1) % 2; break;
			case DS_A:
			case DS_START:
			case AK_RET:
			case AK_SPC: return selected; break;
			case AK_ESC:
			case DS_B: return -1; break;
			}
		}
		switch (mode) {
			case MB_OK:
				write_text(x+width/2-2, y+yo+1, "    ");
				if (flash)
					write_text_inv(x+width/2-1, y+yo+1, "OK");
				else
					write_text(x+width/2-1, y+yo+1, "OK");
				break;
			case MB_YESNO:
				write_text(x+width/2-5, y+yo+1, "         ");
				if (flash && selected==0)
					write_text_inv(x+width/2-4, y+yo+1, "YES");
				else
					write_text(x+width/2-4, y+yo+1, "YES");
				if (flash && selected==1)
					write_text_inv(x+width/2+1, y+yo+1, "NO");
				else
					write_text(x+width/2+1, y+yo+1, "NO");
				break;
		}
		// text_flip() but without the SDL_Delay
		 SDL_BlitSurface(text_screen,NULL,prSDLScreen,NULL);
		 uib_update();
		 SDL_Flip(prSDLScreen);
		if (mode == MB_NONE) break;
		SDL_Delay(20);
		frame = (frame + 1) % 6;
	} while (1);
	return 0;
}

static void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

void draw_scrollbar(int x, int y, int w, int h, int total, int visible, int offset)
{
	int i;
	Uint32 scrollbar_color = SDL_MapRGB(text_screen->format, 0x30, 0x30, 0x30);

	// draw lines
	for (i=1;i<w;++i) {
		putpixel(text_screen, x+i, y, scrollbar_color);
		putpixel(text_screen, x+i, y+h-1, scrollbar_color);
	}
	for (i=1;i<h-1;++i) {
		putpixel(text_screen, x, y+i, scrollbar_color);
		//putpixel(text_screen, x+w-1, y+i, scrollbar_color);
	}
	int by = y + (offset * h) / total;
	int bh = (visible * h) / total;
	SDL_FillRect(text_screen, &((SDL_Rect){x+1, by, w-1, bh}), scrollbar_color);
}

static Uint32 backup_col;

void menu_set_text_color(Uint32 c)
{
	backup_col = menu_text_color;
	menu_text_color = c;
}

extern void menu_restore_text_color()
{
	menu_text_color = backup_col;
}
