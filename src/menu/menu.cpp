#include "sysconfig.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "config.h"
#include "menu.h"

#include <SDL/SDL_mixer.h>

#include "sysdeps.h"
#include "uae.h"
#include "options.h"
#include "zfile.h"
#include "sound.h"

#include "msg.h"
#include "fade.h"
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

static Uint32 menu_inv_color2=0, menu_inv_color=0, menu_win0_color=0, menu_win1_color=0;
static Uint32 menu_barra0_color=0, menu_barra1_color=0;
static Uint32 menu_win0_color_base=0, menu_win1_color_base=0;

void write_text_pos(int x, int y, const char* str);
void write_num(int x, int y, int v);
int menu_msg_pos=330;
int menu_moving=1;
Uint32 menu_msg_time=0x12345678;

#ifdef DREAMCAST
extern int __sdl_dc_emulate_keyboard;
#endif

static void obten_colores(void)
{
	FILE *f=fopen(DATA_PREFIX "colors.txt", "rt");
	if (f)
	{
		Uint32 r,g,b;
		fscanf(f,"menu_inv_color=0x%X,0x%X,0x%X\n",&r,&g,&b);
		menu_inv_color=SDL_MapRGB(text_screen->format,r,g,b);
		fscanf(f,"menu_win0_color=0x%X,0x%X,0x%X\n",&r,&g,&b);
		menu_win0_color=SDL_MapRGB(text_screen->format,r,g,b);
		fscanf(f,"menu_win1_color=0x%X,0x%X,0x%X\n",&r,&g,&b);
		menu_win1_color=SDL_MapRGB(text_screen->format,r,g,b);
		fscanf(f,"menu_barra0_color=0x%X,0x%X,0x%X\n",&r,&g,&b);
		menu_barra0_color=SDL_MapRGB(text_screen->format,r,g,b);
		fscanf(f,"menu_barra1_color=0x%X,0x%X,0x%X\n",&r,&g,&b);
		menu_barra1_color=SDL_MapRGB(text_screen->format,r,g,b);
		fclose(f);
	}
	else
	{
		menu_inv_color=SDL_MapRGB(text_screen->format, 0x20, 0x20, 0x40);
		menu_win0_color=SDL_MapRGB(text_screen->format, 0x10, 0x08, 0x08);
		menu_win1_color=SDL_MapRGB(text_screen->format, 0x20, 0x10, 0x10);
		menu_barra0_color=SDL_MapRGB(text_screen->format, 0x30, 0x20, 0x20);
		menu_barra1_color=SDL_MapRGB(text_screen->format, 0x50, 0x40, 0x40);
	}
	menu_inv_color2=SDL_MapRGB(text_screen->format, 0x70, 0x70, 0x8a);
	menu_win0_color_base=menu_win0_color;
	menu_win1_color_base=menu_win1_color;
}

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

static void text_draw_menu_msg()
{
	write_text_pos(menu_msg_pos,0,menu_msg);
	if (menu_msg_pos<MAX_SCROLL_MSG)
		menu_msg_pos=330;
	else
		menu_msg_pos--;
}


static void update_window_color(void)
{
	static int cambio=0;
	static int spin=0;

	Uint8 r,g,b;
	int cambio2=cambio>>3;
	SDL_GetRGB(menu_win0_color_base,text_screen->format,&r,&g,&b);
	if (((int)r)-cambio2>0) r-=cambio2;
	else r=0;
	if (((int)g)-cambio2>0) g-=cambio2;
	else g=0;
	if (((int)b)-cambio2>0) b-=cambio2;
	else b=0;
	menu_win0_color=SDL_MapRGB(text_screen->format,r,g,b);
	SDL_GetRGB(menu_win1_color_base,text_screen->format,&r,&g,&b);
	if (((int)r)-cambio>0) r-=cambio;
	else r=0;
	if (((int)g)-cambio>0) g-=cambio;
	else g=0;
	if (((int)b)-cambio>0) b-=cambio;
	else b=0;
	menu_win1_color=SDL_MapRGB(text_screen->format,r,g,b);
	if (spin)
	{
		if (cambio<=0) spin=0;
		else cambio-=2;

	}
	else
	{
		if (cambio>=24) spin=1;
		else cambio+=2;
	}
}

void text_draw_background()
{
	static int pos_x=12345678;
	static int pos_y=12345678;
	SDL_Rect r;
	int i,j;
	int w=text_screen->w+text_background->w-1;
	int h=text_screen->h+text_background->h-1;

	if (menu_moving)
	{
		if (pos_x>=0) pos_x=-text_screen->w;
		else pos_x++;
		if (pos_y>=0) pos_y=-text_screen->h;
		else pos_y++;
	}

	for(i=pos_x;i<w;i+=text_background->w)
		for(j=pos_y;j<h;j+=text_background->h)
		{
			r.x=i;
			r.y=j;
			r.w=text_background->w;
			r.h=text_background->h;
			SDL_BlitSurface(text_background,NULL,text_screen,&r);
		}
	if (menu_moving)
	{
		text_draw_menu_msg();
		update_window_color();
	}
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
		prSDLScreen=SDL_SetVideoMode(320,240,16,VIDEO_FLAGS);
    		SDL_ShowCursor(SDL_DISABLE);
 	   	SDL_JoystickEventState(SDL_ENABLE);
    		SDL_JoystickOpen(0);
	}

	if (!text_screen)
	{
		text_screen=SDL_CreateRGBSurface(prSDLScreen->flags,prSDLScreen->w,prSDLScreen->h,prSDLScreen->format->BitsPerPixel,prSDLScreen->format->Rmask,prSDLScreen->format->Gmask,prSDLScreen->format->Bmask,prSDLScreen->format->Amask);
		tmp=SDL_LoadBMP(MENU_FILE_TEXT);
		if (text_screen==NULL || tmp==NULL)
			exit(-1);
		text_image=SDL_DisplayFormat(tmp);
		SDL_FreeSurface(tmp);
		if (text_image==NULL)
			exit(-2);
		SDL_SetColorKey(text_image,(SDL_SRCCOLORKEY | SDL_RLEACCEL),SDL_MapRGB(text_image -> format, 0, 0, 0));
		tmp=SDL_LoadBMP(MENU_FILE_BACKGROUND);
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
		SDL_Event ev;
		int toexit=0;

		obten_colores();
		uae4all_init_sound();
		toexit = 0;

		while(!toexit)
		{
			SDL_Event ev;
#ifdef HOME_DIR
			int config_dir_len = strlen(config_dir);
#endif
			if (!uae4all_init_rom(romfile))
				break;
			text_draw_background();
			text_draw_window(54,110,250,64,"--- ERROR ---");
#ifdef HOME_DIR
			write_text(9,14,"kick.rom not found in:");

			if(config_dir_len < 26) /* Center the text */
			{
				write_text(7 + 13 - config_dir_len/2, 16, config_dir);
			}
			else
			{
				write_text(7,16,config_dir);
			}

#else
			write_text(11,14,"kick.rom not found");
			write_text(8,16,"Press any button to exit");
#endif
			text_flip();
			while(SDL_PollEvent(&ev))
			{
				if (uib_handle_event(&ev)) continue;
				if (ev.type==SDL_QUIT)
					toexit = 1;
				else
				if (ev.type==SDL_KEYDOWN)
				{
					toexit = 1;
				}
				else if (ev.type==SDL_JOYBUTTONDOWN)
				{
					toexit = 1;
				}
			}	

			text_flip();
		}

		if(toexit)
		{
			SDL_Quit();
			exit(1);
		}
	}
	else
	{
		SDL_FillRect(text_screen,NULL,0xFFFFFFFF);
		text_flip();
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

void write_text_pos(int x, int y, const char *str)
{
  int i, c;
  SDL_Rect src, dest;
  
  for (i = 0; str[i] != '\0'; i++)
    {
      c = -1;
      
      if (str[i] >= '0' && str[i] <= '9')
	c = str[i] - '0';
      else if (str[i] >= 'A' && str[i] <= 'Z')
	c = str[i] - 'A' + 10;
      else if (str[i] >= 'a' && str[i] <= 'z')
	c = str[i] - 'a' + 36;
      else if (str[i] == '#')
	c = 62;
      else if (str[i] == '=')
	c = 63;
      else if (str[i] == '.')
	c = 64;
      else if (str[i] == '_')
	c = -2;
      else if (str[i] == '-')
	c = -3;
      else if (str[i] == '(')
	c = 65;
      else if (str[i] == ')')
	c = 66;
      else if (str[i] == '?')
	c = 67;
      
      if (c >= 0)
	{
	  src.x = c * 8;
	  src.y = 0;
	  src.w = 8;
	  src.h = 8;
	  
	  dest.x = x + (i * 8);
	  dest.y = y;
	  dest.w = 8;
	  dest.h = 8;
	  
	  SDL_BlitSurface(text_image, &src,
			  text_screen, &dest);
	}
      else if (c == -2 || c == -3)
	{
	  dest.x = x + (i * 8);
	  
	  if (c == -2)
	    dest.y = y  + 7;
	  else if (c == -3)
	    dest.y = y  + 3;
	  
	  dest.w = 8;
	  dest.h = 1;
	  
	  SDL_FillRect(text_screen, &dest, menu_barra0_color);
	}
    }
}

void _write_text_pos(SDL_Surface *sf, int x, int y, const char *str)
{
	SDL_Surface *back=text_screen;
	text_screen=sf;
	write_text_pos(x,y,str);
	text_screen=back;
}

void write_text(int x, int y, const char *str)
{
  int i, c;
  SDL_Rect src, dest;
  
  for (i = 0; str[i] != '\0'; i++)
    {
      c = -1;
      
      if (str[i] >= '0' && str[i] <= '9')
	c = str[i] - '0';
      else if (str[i] >= 'A' && str[i] <= 'Z')
	c = str[i] - 'A' + 10;
      else if (str[i] >= 'a' && str[i] <= 'z')
	c = str[i] - 'a' + 36;
      else if (str[i] == '#')
	c = 62;
      else if (str[i] == '=')
	c = 63;
      else if (str[i] == '.')
	c = 64;
      else if (str[i] == '_')
	c = -2;
      else if (str[i] == '-')
	c = -3;
      else if (str[i] == '/')
	c = -4;
      else if (str[i] == ':')
	c = -5;
      else if (str[i] == '(')
	c = 65;
      else if (str[i] == ')')
	c = 66;
      else if (str[i] == '?')
	c = 67;
      
      if (c >= 0)
	{
	  src.x = c * 8;
	  src.y = 0;
	  src.w = 8;
	  src.h = 8;
	  
	  dest.x = (x + i) * 8;
	  dest.y = y * 8; //10;
	  dest.w = 8;
	  dest.h = 8;
	  
	  SDL_BlitSurface(text_image, &src,
			  text_screen, &dest);
	}
      else if (c == -2 || c == -3)
	{
	  dest.x = (x + i) * 8;
	  
	  if (c == -2)
	    dest.y = y * 8 /*10*/ + 7;
	  else if (c == -3)
	    dest.y = y * 8 /*10*/ + 3;
	  
	  dest.w = 8;
	  dest.h = 1;
	  
	  SDL_FillRect(text_screen, &dest, menu_barra0_color);
	}
      else if (c == -4)
	{
	  /* upper segment of '/' */
	  dest.x = (x + i) * 8 + 4;
	  dest.y = y * 8 /*10*/;
	  dest.w = 1;
	  dest.h = 2;
	  SDL_FillRect(text_screen, &dest, menu_barra0_color);

	  /* middle segment of '/' */
	  dest.x = (x + i) * 8 + 3;
	  dest.y = y * 8 /*10*/ + 2;
	  dest.w = 1;
	  dest.h = 3;
	  SDL_FillRect(text_screen, &dest, menu_barra0_color);

	  /* lower segment of '/' */
	  dest.x = (x + i) * 8 + 2;
	  dest.y = y * 8 /*10*/ + 5;
	  dest.w = 1;
	  dest.h = 2;
	  SDL_FillRect(text_screen, &dest, menu_barra0_color);
	}
      else if (c == -5)
	{
	  /* upper point of ':' */
	  dest.x = (x + i) * 8 + 2;
	  dest.y = y * 8 /*10*/ + 2;
	  dest.w = 1;
	  dest.h = 1;
	  SDL_FillRect(text_screen, &dest, menu_barra0_color);

	  /* lower point of ':' */
	  dest.x = (x + i) * 8 + 2;
	  dest.y = y * 8 /*10*/ + 6;
	  dest.w = 1;
	  dest.h = 1;
	  SDL_FillRect(text_screen, &dest, menu_barra0_color);
	}
    }
}

void _write_text(SDL_Surface *sf, int x, int y, const char *str)
{
	SDL_Surface *back=text_screen;
	text_screen=sf;
	write_text(x,y,str);
	text_screen=back;
}


/* Write text, inverted: */

void write_text_inv(int x, int y, const char *str)
{
  SDL_Rect dest;
  
  
  dest.x = (x * 8) -2 ;
  dest.y = (y * 8) /*10*/ - 2;
  dest.w = (strlen(str) * 8) + 4;
  dest.h = 12;

  SDL_FillRect(text_screen, &dest, menu_inv_color);

  write_text(x, y, str);
}

void _write_text_inv(SDL_Surface *sf, int x, int y, const char *str)
{
	SDL_Surface *back=text_screen;
	text_screen=sf;
	write_text_inv(x,y,str);
	text_screen=back;
}

void write_text_inv_n(int x, int y, int n, const char *str)
{
  SDL_Rect dest;
  
  
  dest.x = (x * 8) ;
  dest.y = (y * 8) /*10*/ - 3;
  dest.w = (n*8)+2;
  dest.h = 11;

  SDL_FillRect(text_screen, &dest, menu_inv_color);

  dest.x = (x * 8) +1;
  dest.y = (y * 8) /*10*/ -2;
  dest.w = (n*8);
  dest.h = 9;

  SDL_FillRect(text_screen, &dest, menu_inv_color2);

  write_text(x+1, y, str);
}

void _write_text_inv_n(SDL_Surface *sf, int x, int y, int n, const char *str)
{
	SDL_Surface *back=text_screen;
	text_screen=sf;
	write_text_inv_n(x,y,n,str);
	text_screen=back;
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
	write_text(x / 8 + ((w / 8 -strlen(title)) / 2), y / 8 - 1, title);
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
		}
	}
	if (width<w) width=w;
	height += mode==MB_NONE ? 0 : 2;

	int maxwidth = (text_screen->w - MSGBOX_PADDING * 2) / FONT_W - 2;
	if (mode == MB_OK && width<4) width=4;
	if (mode == MB_YESNO && width<9) width=9;
	if (width > maxwidth) width = maxwidth;

	int maxheight = text_screen->h / FONT_H - 3 - MSGBOX_PADDING * 2 / FONT_H;;
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
		for (c=message; c!=1; c=n+1)
		{
			n=strchr(c,'\n');
			snprintf(buf, MIN(n?n-c:strlen(c), width)+1, "%s", c);
			write_text(x, y+yo, buf);
			++yo;
		}

		int flash = frame / 3;

		if (SDL_PollEvent(&e) &&
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