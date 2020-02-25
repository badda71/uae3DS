/*
  * UAE3DS - Amiga 500 emulator for Nintendo 3DS
  *
  * uibottom.c - functions for handling 3DS bottom screen
  *
  * Copyright 2020 Sebastian Weber
  */
 
#include <stdlib.h>
#include <SDL/SDL_image.h>
#include <3ds.h>
#include <citro3d.h>
#include <math.h>
#include "uibottom.h"

extern void log_citra(const char *format, ...);

typedef struct {
	unsigned w;
	unsigned h;
	float fw,fh;
	C3D_Tex tex;
} DS3_Image;

// exposed variables
volatile enum uib_action uib_must_redraw = UIB_NO;

// static sprites
static DS3_Image background_spr;
//static DS3_Image spinner_spr;

// static variables
static Handle repaintRequired;
static int uib_isinit=0;
static u8 *gpusrc=NULL;

// static functions
// ================

// sprite handling funtions
extern C3D_Mtx projection2;
extern C3D_RenderTarget* VideoSurface2;
extern int uLoc_projection;
extern Handle privateSem1;
extern "C" void SDL_RequestCall(void(*callback)(void*,int), void *param);

#define CLEAR_COLOR 0x000000FF
// Used to convert textures to 3DS tiled format
// Note: vertical flip flag set so 0,0 is top left of texture
#define TEXTURE_TRANSFER_FLAGS \
	(GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) | GX_TRANSFER_RAW_COPY(0) | \
	GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8) | \
	GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

#define TEX_MIN_SIZE 64

static unsigned int mynext_pow2(unsigned int v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v >= TEX_MIN_SIZE ? v : TEX_MIN_SIZE;
}

#define B2T(x) (int)(((x)*400.0f)/320.0f+0.5f)

//---------------------------------------------------------------------------------
static inline void  drawImage( DS3_Image *img, int x, int y, int w, int h, int deg) {
//---------------------------------------------------------------------------------
	if (!img) return;
	if (!w) w=img->w;
	if (!h) h=img->h;
	float fw = (float)w;
	float fx = (float)x;

	int x1,x2,x3,x4,y1,y2,y3,y4;
	if (deg) {
		// rotated draw
		float fh = (float)h;
		float fy = (float)y;
		float rad = ((float)deg) * M_PI / 180.0f;
		float c = cosf(rad);
		float s = sinf(rad);
		float cx = fx + fw / 2.0f;
		float cy = fy + fh / 2.0f;
		x1 = B2T(c*(fx-cx)-s*(fy-cy)+cx);
		y1 = (int)(s*(fx-cx)+c*(fy-cy)+cy+0.5f);
		x2 = B2T(c*(fx-cx)-s*(fy+fh-cy)+cx);
		y2 = (int)(s*(fx-cx)+c*(fy+fh-cy)+cy+0.5f);
		x3 = B2T(c*(fx+fw-cx)-s*(fy-cy)+cx);
		y3 = (int)(s*(fx+fw-cx)+c*(fy-cy)+cy+0.5f);
		x4 = B2T(c*(fx+fw-cx)-s*(fy+fh-cy)+cx);
		y4 = (int)(s*(fx+fw-cx)+c*(fy+fh-cy)+cy+0.5f);
	} else {
		x1 = B2T(fx);
		y1 = y;
		x2 = B2T(fx);
		y2 = y + h;
		x3 = B2T(fx + fw);
		y3 = y;
		x4 = B2T(fx + fw);
		y4 = y + h;
	}

	C3D_TexBind(0, &(img->tex));
	// Draw a textured quad directly
	C3D_ImmDrawBegin(GPU_TRIANGLE_STRIP);
		C3D_ImmSendAttrib( x1, y1, 0.5f, 0.0f);	// v0 = position
		C3D_ImmSendAttrib( 0.0f, 0.0f, 0.0f, 0.0f);	// v1 = texcoord0

		C3D_ImmSendAttrib( x2, y2, 0.5f, 0.0f);
		C3D_ImmSendAttrib( 0.0f, img->fh, 0.0f, 0.0f);

		C3D_ImmSendAttrib( x3, y3, 0.5f, 0.0f);		// v0 = position
		C3D_ImmSendAttrib( img->fw, 0.0f, 0.0f, 0.0f);

		C3D_ImmSendAttrib( x4, y4, 0.5f, 0.0f);		// v0 = position
		C3D_ImmSendAttrib( img->fw, img->fh, 0.0f, 0.0f);
	C3D_ImmDrawEnd();
}

static void makeTexture(C3D_Tex *tex, u8 *mygpusrc, unsigned hw, unsigned hh) {
	s32 i;
	svcWaitSynchronization(privateSem1, U64_MAX);
	// init texture
	C3D_TexDelete(tex);
	C3D_TexInit(tex, hw, hh, GPU_RGBA8);
	C3D_TexSetFilter(tex, GPU_NEAREST, GPU_NEAREST);

	// Convert image to 3DS tiled texture format
	GSPGPU_FlushDataCache(mygpusrc, hw*hh*4);
	C3D_SyncDisplayTransfer ((u32*)mygpusrc, GX_BUFFER_DIM(hw,hh), (u32*)(tex->data), GX_BUFFER_DIM(hw,hh), TEXTURE_TRANSFER_FLAGS);
	GSPGPU_FlushDataCache(tex->data, hw*hh*4);
	svcReleaseSemaphore(&i, privateSem1, 1);
}

// this function is NOT thread safe - it uses static buffer gpusrc!!
static void makeImage(DS3_Image *img, u8 *pixels, unsigned w, unsigned h, int noconv) {

	img->w=w;
	img->h=h;
	unsigned hw=mynext_pow2(w);
	img->fw=(float)(w)/hw;
	unsigned hh=mynext_pow2(h);
	img->fh=(float)(h)/hh;

	if (noconv) { // pixels are already in a transferable format (buffer in linear RAM, ABGR pixel format, pow2-dimensions)
		makeTexture(&(img->tex), pixels, hw, hh);
	} else {
		// GX_DisplayTransfer needs input buffer in linear RAM
		// memset(gpusrc,0,hw*hh*4);

		// copy to linear buffer, convert from RGBA to ABGR
		u8* src=pixels; u8 *dst;
		for(unsigned y = 0; y < h; y++) {
			dst=gpusrc+y*hw*4;
			for (unsigned x=0; x<w; x++) {
				int r = *src++;
				int g = *src++;
				int b = *src++;
				int a = *src++;

				*dst++ = a;
				*dst++ = b;
				*dst++ = g;
				*dst++ = r;
			}
		}
		makeTexture(&(img->tex), gpusrc, hw, hh);
	}

	return;
}

static int loadImage(DS3_Image *img, char *fname) {
	SDL_Surface *s=IMG_Load(fname);
	if (!s) return -1;

	makeImage(img, (u8*)s->pixels, s->w,s->h,0);
	SDL_FreeSurface(s);
	return 0;
}

// bottom handling functions
// =========================
static inline void requestRepaint() {
	svcSignalEvent(repaintRequired);
}

static void uib_repaint(void *param, int topupdated) {
	s32 c;
	
	if (svcWaitSynchronization(repaintRequired, 0)) return;
	svcClearEvent(repaintRequired);

	svcWaitSynchronization(privateSem1, U64_MAX);

	// Render the scene
	C3D_RenderTargetClear(VideoSurface2, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
	C3D_FrameDrawOn(VideoSurface2);

	// background
	drawImage(&background_spr, 0, 0, 0, 0, 0);
	
	// spinner
	//static int deg=0;
	//drawImage(&spinner_spr, (320 - spinner_spr.w)/2, (240 - spinner_spr.h)/2, 0, 0, deg);
	//deg = (deg+30) % 360;

	svcReleaseSemaphore(&c, privateSem1, 1);
}

// shutdown bottom
static void uib_shutdown() {
	if (!uib_isinit) return;

	SDL_RequestCall(NULL, NULL);

	svcCloseHandle(repaintRequired);

	if (gpusrc) linearFree(gpusrc);
	gpusrc=NULL;

	uib_isinit = 0;
}

// init bottom
void uib_init() {
	if (uib_isinit) return;
	uib_isinit=1;

	// init gpusrc
	gpusrc = (u8*)linearAlloc(512*256*4);

	// pre-load sprites
	loadImage(&background_spr, "romfs:/background.png");
	//loadImage(&spinner_spr, "romfs:/spinner.png");

	svcCreateEvent(&repaintRequired, RESET_ONESHOT);
	SDL_RequestCall(uib_repaint, NULL);

	uib_must_redraw |= UIB_REPAINT;
	uib_update();
	atexit(uib_shutdown);
}

// exposed functions
void uib_update(void)
{
	enum uib_action uib_must_redraw_local;

	// init if needed
	if (!uib_isinit) uib_init();

	if (uib_must_redraw) {
		// needed for mutithreading
		uib_must_redraw_local = uib_must_redraw;
		uib_must_redraw = UIB_NO;
		requestRepaint();
	}
//	requestRepaint();
}

void uib_handle_event(SDL_Event *e) {

}