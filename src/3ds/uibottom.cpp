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
#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"
#include "keyboard.h"
#include "uibottom.h"
#include "uae3ds.h"
#include "gui.h"
#include "menu.h"

uikbd_key uikbd_keypos[] = {
	//  x,  y,   w,   h,         key,shft,stky, flgs, name
	// toggle kb button
	{   0,-15,  36,  15,         255,   0,   0,  0,  "ToggleKB"},
	// 1st Row
	{   0,  0,  32,  16,      AK_ESC,   0,   0,  0,  "ESC"},
	{  32,  0,  16,  16,       AK_F1,   0,   0,  0,  "F1"},
	{  48,  0,  16,  16,       AK_F2,   0,   0,  0,  "F2"},
	{  64,  0,  16,  16,       AK_F3,   0,   0,  0,  "F3"},
	{  80,  0,  16,  16,       AK_F4,   0,   0,  0,  "F4"},
	{  96,  0,  16,  16,       AK_F5,   0,   0,  0,  "F5"},
	{ 112,  0,  16,  16,       AK_F6,   0,   0,  0,  "F6"},
	{ 128,  0,  16,  16,       AK_F7,   0,   0,  0,  "F7"},
	{ 144,  0,  16,  16,       AK_F8,   0,   0,  0,  "F8"},
	{ 160,  0,  16,  16,       AK_F9,   0,   0,  0,  "F9"},
	{ 176,  0,  16,  16,      AK_F10,   0,   0,  0,  "F10"},
	{ 192,  0,  32,  16,      AK_DEL,   0,   0,  0,  "DEL"},
	{ 224,  0,  32,  16,     AK_HELP,   0,   0,  0,  "HELP"},
	{ 256,  0,  16,  16, AK_NPLPAREN,   0,   0,  0,  "NP ("},
	{ 272,  0,  16,  16, AK_NPRPAREN,   0,   0,  0,  "NP )"},
	{ 288,  0,  16,  16,    AK_NPDIV,   0,   0,  0,  "NP /"},
	{ 304,  0,  16,  16,    AK_NPMUL,   0,   0,  0,  "NP *"},
	// 2nd Row
	{   0, 16,  32,  16,AK_BACKQUOTE,   0,   0,  0,  "`"},
	{  32, 16,  16,  16,        AK_1,   0,   0,  0,  "1"},
	{  48, 16,  16,  16,        AK_2,   0,   0,  0,  "2"},
	{  64, 16,  16,  16,        AK_3,   0,   0,  0,  "3"},
	{  80, 16,  16,  16,        AK_4,   0,   0,  0,  "4"},
	{  96, 16,  16,  16,        AK_5,   0,   0,  0,  "5"},
	{ 112, 16,  16,  16,        AK_6,   0,   0,  0,  "6"},
	{ 128, 16,  16,  16,        AK_7,   0,   0,  0,  "7"},
	{ 144, 16,  16,  16,        AK_8,   0,   0,  0,  "8"},
	{ 160, 16,  16,  16,        AK_9,   0,   0,  0,  "9"},
	{ 176, 16,  16,  16,        AK_0,   0,   0,  0,  "0"},
	{ 192, 16,  16,  16,    AK_MINUS,   0,   0,  0,  "-"},
	{ 208, 16,  16,  16,    AK_EQUAL,   0,   0,  0,  "="},
	{ 224, 16,  16,  16,AK_BACKSLASH,   0,   0,  0,  "\\"},
	{ 240, 16,  16,  16,       AK_BS,   0,   0,  0,  "Backspace"},
	{ 256, 16,  16,  16,      AK_NP7,   0,   0,  0,  "NP 7"},
	{ 272, 16,  16,  16,      AK_NP8,   0,   0,  0,  "NP 8"},
	{ 288, 16,  16,  16,      AK_NP9,   0,   0,  0,  "NP 9"},
	{ 304, 16,  16,  16,    AK_NPSUB,   0,   0,  0,  "NP -"},
	// 3rd Row
	{   0, 32,  32,  16,      AK_TAB,   0,   0,  0,  "TAB"},
	{  32, 32,  16,  16,        AK_Q,   0,   0,  0,  "Q"},
	{  48, 32,  16,  16,        AK_W,   0,   0,  0,  "w"},
	{  64, 32,  16,  16,        AK_E,   0,   0,  0,  "E"},
	{  80, 32,  16,  16,        AK_R,   0,   0,  0,  "R"},
	{  96, 32,  16,  16,        AK_T,   0,   0,  0,  "T"},
	{ 112, 32,  16,  16,        AK_Y,   0,   0,  0,  "Y"},
	{ 128, 32,  16,  16,        AK_U,   0,   0,  0,  "U"},
	{ 144, 32,  16,  16,        AK_I,   0,   0,  0,  "I"},
	{ 160, 32,  16,  16,        AK_O,   0,   0,  0,  "O"},
	{ 176, 32,  16,  16,        AK_P,   0,   0,  0,  "P"},
	{ 192, 32,  16,  16, AK_LBRACKET,   0,   0,  0,  "["},
	{ 208, 32,  16,  16, AK_RBRACKET,   0,   0,  0,  "]"},
	{ 224, 32,  32,  32,      AK_RET,   0,   0,  0,  "RETURN"},
	{ 256, 32,  16,  16,      AK_NP4,   0,   0,  0,  "NP 4"},
	{ 272, 32,  16,  16,      AK_NP5,   0,   0,  0,  "NP 5"},
	{ 288, 32,  16,  16,      AK_NP6,   0,   0,  0,  "NP 6"},
	{ 304, 32,  16,  16,    AK_NPADD,   0,   0,  0,  "NP +"},
	// 4th Row
	{   0, 48,  32,  16,     AK_CTRL,   0,   2,  0,  "CTRL"},
	{  32, 48,  16,  16,        AK_A,   0,   0,  0,  "A"},
	{  48, 48,  16,  16,        AK_S,   0,   0,  0,  "S"},
	{  64, 48,  16,  16,        AK_D,   0,   0,  0,  "D"},
	{  80, 48,  16,  16,        AK_F,   0,   0,  0,  "F"},
	{  96, 48,  16,  16,        AK_G,   0,   0,  0,  "G"},
	{ 112, 48,  16,  16,        AK_H,   0,   0,  0,  "H"},
	{ 128, 48,  16,  16,        AK_J,   0,   0,  0,  "J"},
	{ 144, 48,  16,  16,        AK_K,   0,   0,  0,  "K"},
	{ 160, 48,  16,  16,        AK_L,   0,   0,  0,  "L"},
	{ 176, 48,  16,  16,AK_SEMICOLON,   0,   0,  0,  ";"},
	{ 192, 48,  16,  16,    AK_QUOTE,   0,   0,  0,  "'"},
	{ 256, 48,  16,  16,      AK_NP1,   0,   0,  0,  "NP 1"},
	{ 272, 48,  16,  16,      AK_NP2,   0,   0,  0,  "NP 2"},
	{ 288, 48,  16,  16,      AK_NP3,   0,   0,  0,  "NP 3"},
	{ 304, 48,  16,  48,      AK_ENT,   0,   0,  0,  "ENTER"},
	// 5th Row
	{   0, 64,  48,  16,      AK_LSH,   0,   1,  0,  "LSHIFT"},
	{  48, 64,  16,  16,        AK_Z,   0,   0,  0,  "Z"},
	{  64, 64,  16,  16,        AK_X,   0,   0,  0,  "X"},
	{  80, 64,  16,  16,        AK_C,   0,   0,  0,  "C"},
	{  96, 64,  16,  16,        AK_V,   0,   0,  0,  "V"},
	{ 112, 64,  16,  16,        AK_B,   0,   0,  0,  "B"},
	{ 128, 64,  16,  16,        AK_N,   0,   0,  0,  "N"},
	{ 144, 64,  16,  16,        AK_M,   0,   0,  0,  "M"},
	{ 160, 64,  16,  16,    AK_COMMA,   0,   0,  0,  ","},
	{ 176, 64,  16,  16,   AK_PERIOD,   0,   0,  0,  "."},
	{ 192, 64,  16,  16,    AK_SLASH,   0,   0,  0,  "/"},
	{ 208, 64,  32,  16,      AK_LSH,   0,   1,  0,  "RSHIFT"},
	{ 240, 64,  16,  16,       AK_UP,   0,   0,  0,  "C_UP"},
	{ 256, 64,  32,  16,      AK_NP0,   0,   0,  0,  "NP 0"},
	{ 288, 64,  16,  16,    AK_NPDEL,   0,   0,  0,  "NP ."},
	// 6th row
	{   0, 80,  32,  17,     AK_LALT,   0,   4,  0,  "LALT"},
	{  32, 80,  16,  17,     AK_LAMI,   0,   8,  0,  "LAMI"},
	{  48, 80, 144,  17,      AK_SPC,   0,   0,  0,  "SPACE"},
	{ 192, 80,  16,  17,     AK_LAMI,   0,   8,  0,  "RAMI"},
	{ 208, 80,  16,  17,     AK_LALT,   0,   4,  0,  "RALT"},
	{ 224, 80,  16,  17,       AK_LF,   0,   0,  0,  "C_LEFT"},
	{ 240, 80,  16,  17,       AK_DN,   0,   0,  0,  "C_DOWN"},
	{ 256, 80,  16,  17,       AK_RT,   0,   0,  0,  "C_RIGHT"},
	// Finish
	{   0,  0,   0,   0,          -1,   0,   0,  0,  ""}
};

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
static DS3_Image kbd1_spr;
static DS3_Image kbd2_spr;
static DS3_Image twistyup_spr;
static DS3_Image twistydn_spr;
static DS3_Image keymask_spr;

// dynamic sprites
static DS3_Image statusbar_spr;

// SDL Surfaces
SDL_Surface *statusbar_img=NULL;

// static variables
static Handle repaintRequired;
static int uib_isinit=0;
static u8 *gpusrc=NULL;
static u8 *status_gpusrc=NULL;
static int kb_y_pos = 0;
static volatile int set_kb_y_pos = -10000;
static int kb_activekey=-1;
static int sticky=0;
static unsigned char keysPressed[256];

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

static void makeTexture(C3D_Tex *tex, const u8 *mygpusrc, unsigned hw, unsigned hh) {
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
static void makeImage(DS3_Image *img, const u8 *pixels, unsigned w, unsigned h, int noconv) {

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
		const u8* src=pixels; u8 *dst;
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

static int loadImage(DS3_Image *img, const char *fname) {
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
	int i;
	
	if (svcWaitSynchronization(repaintRequired, 0)) return;
	svcClearEvent(repaintRequired);
	svcWaitSynchronization(privateSem1, U64_MAX);

	if (set_kb_y_pos != -10000) {
		kb_y_pos=set_kb_y_pos;
		set_kb_y_pos=-10000;
	}

	// Render the scene
	C3D_RenderTargetClear(VideoSurface2, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
	C3D_FrameDrawOn(VideoSurface2);

	// background
	drawImage(&background_spr, 0, 0, 0, 0, 0);

	// statusbar
	drawImage(&statusbar_spr, 0, 0, 0, 0, 0);

	// keyboard
	DS3_Image *kb=(sticky & 1) == 1 ? &(kbd2_spr):&(kbd1_spr);
	drawImage(kb, 0, kb_y_pos, 0, 0, 0);
	// keyboard twisty
	DS3_Image *tw = kb_y_pos > (240 - kb->h) ? &(twistyup_spr):&(twistydn_spr);
	drawImage(tw, 0, kb_y_pos - tw->h, 0, 0, 0);
	// keys pressed
	uikbd_key *k;
	for (i=0;uikbd_keypos[i].key!=-1; ++i) {
		k=&(uikbd_keypos[i]);
		if (k->flags==1 || keysPressed[i]==0) continue;
		drawImage(&(keymask_spr),k->x,k->y+kb_y_pos,k->w,k->h,0);
	}

	svcReleaseSemaphore(&c, privateSem1, 1);
}

// shutdown bottom
static void uib_shutdown() {
	if (!uib_isinit) return;

	SDL_RequestCall(NULL, NULL);

	svcCloseHandle(repaintRequired);

	if (gpusrc) linearFree(gpusrc);
	gpusrc = NULL;
	
	SDL_FreeSurface(statusbar_img);
	if (status_gpusrc) linearFree(status_gpusrc);
	status_gpusrc = NULL;

	uib_isinit = 0;
}

static void keypress_recalc() {
	int state,key;

	memset(keysPressed,0,sizeof(keysPressed));
	for (key = 0; uikbd_keypos[key].key!=-1; ++key) {
		state=0;
		if (key == kb_activekey) state=1;
		else if (uikbd_keypos[key].sticky & sticky) state=1;
		keysPressed[key]=state;
	}
}

static void *alloc_copy(void *p, size_t s) {
	void *d=malloc(s);
	memcpy(d,p,s);
	return d;
}

typedef struct animation {
	int *var;
	int from;
	int to;
} animation;

typedef struct animation_set {
	int steps;
	int delay;
	int nr;
	void (*callback)(void *);
	void *callback_data;
	void (*callback2)(void *);
	void *callback2_data;
	animation anim[];
} animation_set;

static int animate(void *data){
	animation_set *a=(animation_set*)data;
	int steps = a->steps != 0 ? a->steps : 15 ;
	int delay = a->delay != 0 ? a->delay : 16 ; // 1/60 sec, one 3ds frame
	for (int s=0; s <= steps; s++) {
		for (int i=0; i < a->nr; i++) {
			*(a->anim[i].var)=
				a->anim[i].from +
				(((a->anim[i].to - a->anim[i].from) * s ) / steps);
		}
		if (a->callback) (a->callback)(a->callback_data);
		if (s != steps) SDL_Delay(delay);
	}
	if (a->callback2) (a->callback2)(a->callback2_data);
	free(data);
	return 0;
}

static void anim_callback(void *param) {
	requestRepaint();
}

static void toggle_keyboard() {
	int y1=240-kbd1_spr.h;

	start_worker(animate, alloc_copy(&((int[]){
		0, 0, 1, // steps, delay, nr
		(int)anim_callback, 0, // callback, callback_data
		0,0, // callback2, callback2_data
		(int)(&set_kb_y_pos), kb_y_pos < 240 ? y1 : 240, kb_y_pos < 240 ? 240 : y1
	}), 10*sizeof(int)));
}

// exposed functions
void uib_init() {
	if (uib_isinit) return;
	uib_isinit=1;

	// init gpusrc
	gpusrc = (u8*)linearAlloc(512*256*4);

	// pre-load sprites
	loadImage(&background_spr, "romfs:/background.png");
	loadImage(&kbd1_spr, "romfs:/kbd1.png");
	loadImage(&kbd2_spr, "romfs:/kbd2.png");
	loadImage(&twistyup_spr, "romfs:/twistyup.png");
	loadImage(&twistydn_spr, "romfs:/twistydn.png");
	makeImage(&keymask_spr, (const u8[]){0x00, 0x00, 0x00, 0x80},1,1,0);

	
	// set up status sprite / framebuf
	status_gpusrc = (u8*)linearAlloc(512*16*4);
	memset(status_gpusrc, 0, 512*16*4);
	statusbar_img = SDL_CreateRGBSurfaceFrom((void *)status_gpusrc,
                        512, 16, 32, 512*4,
                        0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
	SDL_SetAlpha(statusbar_img, 0, 255);

	statusbar_spr.w=320;
	statusbar_spr.h=16;
	statusbar_spr.fw=(float)(statusbar_spr.w)/512.0f;
	statusbar_spr.fh=(float)(statusbar_spr.h)/16.0f;
	makeTexture(&(statusbar_spr.tex), status_gpusrc, 512, 16);

	// other stuff
	kb_y_pos = 240 - kbd1_spr.h;

	svcCreateEvent(&repaintRequired, RESET_ONESHOT);
	SDL_RequestCall(uib_repaint, NULL);

	uib_must_redraw |= UIB_REPAINT;
	uib_update();
	atexit(uib_shutdown);
}

// status line stuff

#define TD_PADX 9
#define TD_PADY 2
#define TD_WIDTH 32
#define TD_LED_WIDTH 24

#define ON_RGB_D 0x00ff00ff
#define OFF_RGB_D 0x0044000ff
#define ON_RGB_P 0xff0000ff
#define OFF_RGB_P 0x4400000ff

#define TD_NUM_WIDTH 7
#define TD_NUM_HEIGHT 7

#define TD_TOTAL_HEIGHT (TD_PADY * 2 + TD_NUM_HEIGHT)
#define TD_STARTX 218

static const char *numbers = {
"------ ------ ------ ------ ------ ------ ------ ------ ------ ------ "
"-xxxxx ---xx- -xxxxx -xxxxx -x---x -xxxxx -xxxxx -xxxxx -xxxxx -xxxxx "
"-x---x ----x- -----x -----x -x---x -x---- -x---- -----x -x---x -x---x "
"-x---x ----x- -xxxxx -xxxxx -xxxxx -xxxxx -xxxxx ----x- -xxxxx -xxxxx "
"-x---x ----x- -x---- -----x -----x -----x -x---x ---x-- -x---x -----x "
"-xxxxx ----x- -xxxxx -xxxxx -----x -xxxxx -xxxxx ---x-- -xxxxx -xxxxx "
"------ ------ ------ ------ ------ ------ ------ ------ ------ ------ "
};

static void uib_paint_led(int x, u32 col, int num)
{
	int y,j;
	
	for (y = 0; y < TD_TOTAL_HEIGHT; ++y) {
		for (j = 0 ; j < TD_LED_WIDTH; ++j) {
			*((u32*)(status_gpusrc + ((x + j + y * 512)<<2))) = col;
		}
	}

	if (num >= 0) {
		x += TD_PADX;
		u8 *nump1 = (u8 *)(numbers + (num/10) * TD_NUM_WIDTH);
		u8 *nump2 = (u8 *)(numbers + (num%10) * TD_NUM_WIDTH);
		for (y = 0; y < TD_NUM_HEIGHT; ++y) {
			for (j = 0 ; j < TD_NUM_WIDTH; ++j) {
				*((u32*)(status_gpusrc + ((x + j + (y + TD_PADY) * 512)<<2))) = 
					nump1[j + y * 10 * TD_NUM_WIDTH] == 'x'? 0xffffffff : 0x000000ff;
				*((u32*)(status_gpusrc + ((TD_NUM_WIDTH + x + j + (y + TD_PADY) * 512)<<2))) = 
					nump2[j + y * 10 * TD_NUM_WIDTH] == 'x'? 0xffffffff : 0x000000ff;
			}
		}
	}
}

static void uib_statusbar_recalc()
{
	static uae_u8 dt0 = -1;
	static uae_u8 dt1 = -1;
	static uae_u8 dm0 = -1;
	static uae_u8 dm1 = -1;
	static uae_u8 pow = -1;
	static int fps = -1;
	int upd=0;
	extern int fps_counter;
	static char buf[100]={0};


	if (dt0 != gui_data.drive_track[0] || 
		dm0 != gui_data.drive_motor[0])
	{
		dt0 = gui_data.drive_track[0];
		dm0 = gui_data.drive_motor[0];
		uib_paint_led(TD_STARTX + TD_WIDTH, dm0 ? ON_RGB_D : OFF_RGB_D, dt0);
		upd=1;
	}
	if (dt1 != gui_data.drive_track[1] || 
		dm1 != gui_data.drive_motor[1])
	{
		dt1 = gui_data.drive_track[1];
		dm1 = gui_data.drive_motor[1];
		uib_paint_led(TD_STARTX + 2*TD_WIDTH, dm1 ? ON_RGB_D : OFF_RGB_D, dt1);
		upd=1;
	}
	
	if (pow != gui_data.powerled ||
		fps != fps_counter)	
	{
		pow = gui_data.powerled;
		fps = fps_counter;
		uib_paint_led(TD_STARTX, pow ? ON_RGB_P : OFF_RGB_P, fps_counter);
		upd=1;
	}

	if (show_message)
	{
		show_message--;
		if (!show_message) {
			SDL_FillRect(statusbar_img,
				&(SDL_Rect){.x=0, .y=0, .w=TD_STARTX-TD_WIDTH+TD_LED_WIDTH, .h=TD_TOTAL_HEIGHT}, 0x00000000);
			*buf=0;
			upd=1;
		} else {
			if (strcmp(buf, show_message_str)) {
				strncpy(buf, show_message_str, 99);
				SDL_FillRect(statusbar_img,
					&(SDL_Rect){.x=0, .y=0, .w=TD_STARTX-TD_WIDTH+TD_LED_WIDTH, .h=TD_TOTAL_HEIGHT}, 0x70708aff);			
				write_text_full (statusbar_img, buf, 2, 2, TD_STARTX/8-1, ALIGN_LEFT, FONT_NORMAL, (SDL_Color){0x30,0x30,0x30,0});
				upd=1;
			}
		}
	}
	if (upd) {
		makeTexture(&(statusbar_spr.tex), statusbar_img->pixels, 512, 16);
		uib_must_redraw |= UIB_REPAINT;
	}
}

void uib_update(void)
{
	enum uib_action uib_must_redraw_local;

	// init if needed
	if (!uib_isinit) uib_init();

	uib_statusbar_recalc();

	if (uib_must_redraw) {
		// needed for mutithreading
		uib_must_redraw_local = uib_must_redraw;
		uib_must_redraw = UIB_NO;
		
		if (uib_must_redraw_local & UIB_RECALC_KEYPRESS) {
			keypress_recalc();
		}
		requestRepaint();
	}
//	requestRepaint();
}

#define DOUBLECLICK_TIME 500

int uib_handle_event(SDL_Event *e) {
	static SDL_Event sdl_e;
	int i,x,y;
	static Uint32 gesture1_time=0;
	static int gesture1_active=0;

	if (e->type == SDL_KEYDOWN) {
		if (e->key.keysym.sym == 255) {
			toggle_keyboard();
			return 1;
		}
	}

	switch (e->type) {
		case SDL_MOUSEMOTION:
			return 0;
		case SDL_MOUSEBUTTONUP:
			if (gesture1_active) {
				sdl_e.type = SDL_KEYUP;
				sdl_e.key.keysym.sym = sdl_e.key.keysym.unicode = DS_ZL;
				SDL_PushEvent(&sdl_e);
				gesture1_active=0;
				return 0;
			}
			if (kb_activekey==-1) return 0; // did not get the button down, so ignore button up
			i=kb_activekey;
			kb_activekey=-1;
			break;
		case SDL_MOUSEBUTTONDOWN:
			x = e->button.x;
			y = e->button.y;
			for (i = 0; uikbd_keypos[i].key != -1 ; ++i) {
				// keyboard button
				if (x >= uikbd_keypos[i].x &&
					x <  uikbd_keypos[i].x + uikbd_keypos[i].w &&
					y >= uikbd_keypos[i].y + kb_y_pos &&
					y <  uikbd_keypos[i].y + uikbd_keypos[i].h + kb_y_pos) break;
			}
			if (uikbd_keypos[i].key == -1) {
				// touch outside of keyboard - check for gesture
				Uint32 t = SDL_GetTicks();
				if (t-gesture1_time < DOUBLECLICK_TIME) {
					sdl_e.type = SDL_KEYDOWN;
					sdl_e.key.keysym.sym = sdl_e.key.keysym.unicode = DS_ZL;
					SDL_PushEvent(&sdl_e);
					gesture1_active=1;
				} else {
					gesture1_time = t;
				}
				return 0;
			}
			if (i==kb_activekey) return 1; // ignore button down on an already pressed key
			kb_activekey=i;
			break;
		default:
			return 0;
	}

	// sticky key press
	if (uikbd_keypos[i].sticky>0) {
		if (e->button.type == SDL_MOUSEBUTTONDOWN) {
			sticky = sticky ^ uikbd_keypos[i].sticky;
			sdl_e.type = sticky & uikbd_keypos[i].sticky ? SDL_KEYDOWN : SDL_KEYUP;
			sdl_e.key.keysym.sym = uikbd_keypos[i].key;
			sdl_e.key.keysym.unicode = 0;
			SDL_PushEvent(&sdl_e);
		}
	} else {
		// normal key press
		sdl_e.type = e->button.type == SDL_MOUSEBUTTONDOWN ? SDL_KEYDOWN : SDL_KEYUP;
		sdl_e.key.keysym.sym = uikbd_keypos[i].key;
		if ((sticky & 1) && uikbd_keypos[i].shift)
			sdl_e.key.keysym.unicode = uikbd_keypos[i].shift;
		else
			sdl_e.key.keysym.unicode = sdl_e.key.keysym.sym;
		SDL_PushEvent(&sdl_e);
	}
	uib_must_redraw |= UIB_RECALC_KEYPRESS;
	return 1;
}