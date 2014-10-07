#include"fade.h"


void fade16(SDL_Surface *screen, unsigned short n)
{
	int i,total=screen->w*screen->h;
	SDL_LockSurface(screen);
	unsigned short rs=screen->format->Rshift;
	unsigned short gs=screen->format->Gshift;
	unsigned short bs=screen->format->Bshift;
	unsigned short rm=screen->format->Rmask;
	unsigned short gm=screen->format->Gmask;
	unsigned short bm=screen->format->Bmask;
	unsigned short rM=rm>>rs;
	unsigned short gM=gm>>gs;
	unsigned short bM=bm>>bs;
	unsigned short * buff=(unsigned short*)screen->pixels;
	for(i=0;i<total;i++)
	{
		register unsigned short r=(buff[i]&rm)>>rs;
		register unsigned short g=(buff[i]&gm)>>gs;
		register unsigned short b=(buff[i]&bm)>>bs;
//		if (n>r)
		if (n+r<rM)
			r+=n;
		else
			r=rM;
//		if (n>g)
		if (n+g<gM)
			g+=n;
		else
			g=gM;
//		if (n>b)
		if (n+b<bM)
			b+=n;
		else
			b=bM;
		buff[i]=(((r<<rs)&rm) | ((g<<gs)&gm) | ((b<<bs)&bm));
	}
	SDL_UnlockSurface(screen);
}
