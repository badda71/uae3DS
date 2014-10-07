 /*
  * UAE - The Un*x Amiga Emulator
  *
  * CIA chip support
  *
  * (c) 1995 Bernd Schmidt
  */

extern void CIA_reset (void);
extern void CIA_vsync_handler (void);
extern void CIA_hsync_handler (void);
extern void CIA_handler (void);

extern void diskindex_handler (void);

extern void rethink_cias (void);
extern unsigned int ciaaicr,ciaaimask,ciabicr,ciabimask;
extern unsigned int ciaacra,ciaacrb,ciabcra,ciabcrb;
extern unsigned int ciabpra;
extern unsigned long ciaata,ciaatb,ciabta,ciabtb;
extern unsigned long ciaatod,ciabtod,ciaatol,ciabtol,ciaaalarm,ciabalarm;
extern int ciaatlatch,ciabtlatch;
