 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Joystick emulation prototypes
  *
  * Copyright 1995 Bernd Schmidt
  */

struct joy_range
{
    int minx, maxx, miny, maxy;
};

extern struct joy_range dzone0, dzone1;

extern void read_joystick (int nr, unsigned int *dir, int *button);
extern void init_joystick (void);
extern void close_joystick (void);

extern int nr_joysticks;
