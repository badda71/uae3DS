 /*
  * UAE - The Un*x Amiga Emulator
  *
  * routines to handle compressed file automatically
  *
  * (c) 1996 Samuel Devulder
  */

extern FILE *zfile_open (const char *, const char *);
extern int zfile_close (FILE *);
extern void zfile_exit (void);

extern size_t uae4all_fread( void *ptr, size_t tam, size_t nmiemb, FILE *flujo);
extern size_t uae4all_fwrite( void *ptr, size_t tam, size_t nmiemb, FILE *flujo);
extern int uae4all_fseek( FILE *flujo, long desplto, int origen);
extern long uae4all_ftell( FILE *flujo);

extern int uae4all_init_rom(const char *name);
extern size_t uae4all_rom_fread( void *ptr, size_t tam, size_t nmiemb, FILE *flujo);
extern int uae4all_rom_fseek( FILE *flujo, long desplto, int origen);
extern FILE *uae4all_rom_fopen(const char *name, const char *mode);
extern int uae4all_rom_fclose(FILE *flujo);
extern void uae4all_rom_reinit(void);
extern void uae4all_flush_disk(int n);
