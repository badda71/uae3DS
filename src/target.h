 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Target specific stuff, Acorn RISC OS version
  *
  * Copyright 1997 Bernd Schmidt
  */

#define OPTIONSFILENAME "uaerc"

/* ? */

#define TARGET_NAME "Dreamcast"
#define DEFPRTNAME ""

#define UNSUPPORTED_OPTION_p
#define UNSUPPORTED_OPTION_I

// #define PICASSO96_SUPPORTED
#ifdef DEBUG_UAE4ALL
#define write_log write_log_standard
#else
#define write_log(...)
#endif
