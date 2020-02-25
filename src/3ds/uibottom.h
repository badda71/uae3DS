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
	UIB_REPAINT
};

// exposed functions
extern void uib_update(void);
extern void uib_handle_event(SDL_Event *);
extern void uib_init();

// exposed variables
extern volatile enum uib_action uib_must_redraw;
#endif
