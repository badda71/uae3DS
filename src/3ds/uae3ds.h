/*
  * UAE3DS - Amiga 500 emulator for Nintendo 3DS
  *
  * uae3ds.h - functions specific to uae3ds port
  *
  * Copyright 2020 Sebastian Weber
  */
#include "3ds.h"
#include <SDL/SDL.h>

typedef struct {
	Handle mutex;
	void **queue;
	int size;
	int head;
	int tail;
	int locked;
} tsq_object;

typedef struct {
	char *key;
	void *val;
} tsh_item;

typedef struct {
	Handle mutex;
	tsh_item *hash;
	int size;
	int locked;
	void (*free_callback)(void *val);
} tsh_object;

extern u32 hashKey(u8 *key);
extern void tsh_init(tsh_object *o, int size, void (*free_callback)(void *val));
extern void *tsh_get(tsh_object *o, char *key);
extern int tsh_put(tsh_object *o, char *key, void *val);
extern void tsh_free(tsh_object *o);

extern void tsq_init(tsq_object *o, int size);
extern void tsq_lock(tsq_object *o, int lock);
extern void tsq_free(tsq_object *o);
extern void *tsq_get(tsq_object *o);
extern void *tsq_put(tsq_object *o, void *p);
extern int start_worker(int (*fn)(void *), void *data);

extern void uae3ds_mapping_add();
extern void uae3ds_mapping_del();
extern void uae3ds_mapping_list();
extern void uae3ds_mapping_apply(SDL_Event *e);
extern void uae3ds_mapping_loadbuf(char *s);
extern char *uae3ds_mapping_savebuf();
extern char *stralloc(char *s);
extern char *concat(char *s, ...);
extern char amiga2ascii(int key);

extern char *scan_qr_code(SDL_Surface *s);
extern int asprintf (char **str, const char *fmt, ...);
extern int vasprintf (char **str, const char *fmt, va_list args);
extern const char *humanSize(uint64_t bytes);
extern int getWifiStatus();
