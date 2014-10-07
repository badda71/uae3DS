#include<SDL.h>

#define VKBD_X 20
#define VKBD_Y 200

#define VKBD_LEFT	1
#define VKBD_RIGHT	2	
#define VKBD_UP		4
#define VKBD_DOWN	8
#define VKBD_BUTTON	16
#define VKBD_BUTTON2	32
#define VKBD_BUTTON3	64
#define VKBD_BUTTON4	128

int vkbd_init(void);
void vkbd_quit(void);
void vkbd_redraw(void);
void vkbd_mouse(void);
SDLKey vkbd_process(void);
void vkbd_init_button2(void);

extern int vkbd_mode;
extern int vkbd_move;
extern SDLKey vkbd_key;
extern int vkbd_keysave;
extern SDLKey vkbd_button2;
extern SDLKey vkbd_button3;
extern SDLKey vkbd_button4;
