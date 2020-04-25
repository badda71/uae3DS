/*
  * UAE3DS - Amiga 500 emulator for Nintendo 3DS
  *
  * uibottom.h - functions for handling 3DS bottom screen
  *
  * Copyright 2020 Sebastian Weber
  */


#ifndef UAE_UIBOTTOM_H
#define UAE_UIBOTTOM_H

// exposed definitions
enum uib_action {
	UIB_NO = 0,
	UIB_REPAINT = 1,
	UIB_RECALC_KEYPRESS = 2,
	UIB_RECALC_STATUSBAR = 4
};

#define DS_A 0x101
#define DS_B 0x102
#define DS_X 0x103
#define DS_Y 0x104
#define DS_L 0x105
#define DS_R 0x106
#define DS_ZL 0x107
#define DS_ZR 0x108
#define DS_START 0x109
#define DS_SELECT 0x10a
#define DS_UP1 0x10b
#define DS_DOWN1 0x10c
#define DS_LEFT1 0x10d
#define DS_RIGHT1 0x10e
#define DS_UP2 0x10f
#define DS_DOWN2 0x110
#define DS_LEFT2 0x111
#define DS_RIGHT2 0x112
#define DS_UP3 0x113
#define DS_DOWN3 0x114
#define DS_LEFT3 0x115
#define DS_RIGHT3 0x116
#define DS_TOUCH 0x117

typedef struct {
	int x,y,w,h,key,sticky,flags;
	const char *name;
} uikbd_key;

// exposed functions
extern void uib_update(void);
extern int uib_handle_event(SDL_Event *);
extern void uib_init();

// exposed variables
extern uikbd_key uikbd_keypos[];
extern volatile enum uib_action uib_must_redraw;
#endif
