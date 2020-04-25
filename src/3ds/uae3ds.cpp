/*
  * UAE3DS - Amiga 500 emulator for Nintendo 3DS
  *
  * uae3ds.c - functions specific to uae3ds port
  *
  * Copyright 2020 Sebastian Weber
  */
 
#include <3ds.h>
#include <string.h>
#include <SDL/SDL.h>
#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"
#include "uae.h"
#include "uae3ds.h"
#include "uibottom.h"
#include "menu.h"

// thread safe hash functions

// Fowler-Noll-Vo Hash (FNV1a)
u32 hashKey(u8 *key) {
	u32 hash=0x811C9DC5;
	while (*key!=0) hash=(*(key++)^hash) * 0x01000193;
	return hash;
}

void tsh_init(tsh_object *o, int size, void (*free_callback)(void *val)) {
	svcCreateMutex(&(o->mutex), false);
	o->hash=(tsh_item*)malloc(size*sizeof(tsh_item));
	memset(o->hash, 0, size*sizeof(tsh_item));
	o->size=size;
	o->locked=0;
	o->free_callback=free_callback;
}

void tsh_free(tsh_object *o) {
	// free all emements
	int i;
	for (i=0;i<o->size;i++) {
		if (o->hash[i].key != NULL)
			free(o->hash[i].key);
		if (o->hash[i].val != NULL && o->free_callback)
			o->free_callback(o->hash[i].val);
	}
	free(o->hash);
}

void *tsh_get(tsh_object *o, char *key) {
//log_citra("enter %s: %s",__func__,key);
	if (key==NULL) return NULL;
	int i=hashKey((u8*)key) % o->size;
	int count=0;
	void *r=NULL;
	svcWaitSynchronization(o->mutex, U64_MAX);
	while (o->hash[i].key != NULL && ++count <= o->size) {
		if (strcmp(key,o->hash[i].key)==0) {
			r=o->hash[i].val;
			break;
		}
		++i; i %= o->size;
	}
	svcReleaseMutex(o->mutex);
	return r;
}

char *stralloc(const char *src) {
	char *r = (char*)malloc(strlen(src)+1);
	strcpy(r,src);
	return r;
}

int tsh_put(tsh_object *o, char *key, void *val) {
//log_citra("enter %s: %s",__func__,key);
	if (key==NULL) return -1;
	int i=hashKey((u8*)key) % o->size;
	int count = 0;
	svcWaitSynchronization(o->mutex, U64_MAX);
	while (o->hash[i].key != NULL && strcmp(o->hash[i].key, key) != 0) {
		++i; i %= o->size;
		if (++count >= o->size) {
			svcReleaseMutex(o->mutex);
			return -1;
		}
	}
	if (o->hash[i].key != NULL) {
		free(o->hash[i].key);
		if (o->free_callback) o->free_callback(o->hash[i].val);
	}
	o->hash[i].key = (val == NULL) ? NULL : stralloc(key);
	o->hash[i].val = val;
	svcReleaseMutex(o->mutex);
	return 0;
}

// thread safe queue functions
void tsq_init(tsq_object *o, int size) {
	svcCreateMutex(&(o->mutex), false);
	o->queue=(void**)malloc(size*sizeof(void*));
	memset(o->queue, 0, size*sizeof(void*));
	o->size=size;
	o->head=o->tail=0;
	o->locked=0;
}

void tsq_free(tsq_object *o) {
	tsq_lock(o, 1);
	free(o->queue);
}

void tsq_lock(tsq_object *o, int lock) {
	svcWaitSynchronization(o->mutex, U64_MAX);
	o->locked=lock;
	svcReleaseMutex(o->mutex);
}

void *tsq_get(tsq_object* o) {
	void *r = NULL;
	svcWaitSynchronization(o->mutex, U64_MAX);
	if (o->tail != o->head) {
		r = o->queue[o->tail];
		o->queue[o->tail]=NULL;
		++o->tail;
		o->tail %= o->size;
	}
	svcReleaseMutex(o->mutex);
	return r;
}

void *tsq_put(tsq_object* o, void *p) {
	void *r = NULL;
	svcWaitSynchronization(o->mutex, U64_MAX);
	if (!o->locked) {
		r=o->queue[o->head];
		o->queue[o->head]=p;
		++o->head;
		o->head %= o->size;
	}
	svcReleaseMutex(o->mutex);
	return r;
}

// threadworker-related vars / functions
#define MAXPENDING 10

static SDL_Thread *worker=NULL;
static SDL_sem *worker_sem=NULL;
static int (*worker_fn[MAXPENDING])(void *);
static void *worker_data[MAXPENDING];

static int worker_thread(void *data) {
	static int p2=0;
	while( 1 ) {
		SDL_SemWait(worker_sem);
		(worker_fn[p2])(worker_data[p2]);
		worker_fn[p2]=NULL;
		p2=(p2+1)%MAXPENDING;
	}
	return 0;
}

void worker_init() {
	worker_sem = SDL_CreateSemaphore(0);
	worker = SDL_CreateThread(worker_thread,NULL);
	memset(worker_fn, 0, sizeof(worker_fn));
	memset(worker_data, 0, sizeof(worker_data));
}

int start_worker(int (*fn)(void *), void *data) {
	static int p1=0;
	if (!worker_sem) worker_init();
	if (worker_fn[p1] != NULL) return -1;
	worker_fn[p1]=fn;
	worker_data[p1]=data;
	SDL_SemPost(worker_sem);	
	p1=(p1+1)%MAXPENDING;
	return 0;
}

static int keymaps[0x200][2] = {0};

const char *get_3ds_keyname(int k)
{
	extern sdl_3dsbuttons buttons3ds[];
	int i;
	for (i=0; buttons3ds[i].key !=0; ++i)
		if (k==buttons3ds[i].key) return buttons3ds[i].name;
	for (i=0; uikbd_keypos[i].key !=-1; ++i)
		if (k==uikbd_keypos[i].key) return uikbd_keypos[i].name;
	return "unknown";
}

char *get_3ds_mapping_name(int i) {
	static char buf[41];
	if (!keymaps[i][0]) return NULL;
	snprintf(buf,41,"%s",get_3ds_keyname(keymaps[i][0]));
	if (keymaps[i][1] && strlen(buf) < 40) {
		snprintf(buf+strlen(buf), 41-strlen(buf), " + %s",get_3ds_keyname(keymaps[i][1]));
	}
	return buf;
}

static int pollKey(char *title, char *message, int time) {
	u32 t, bt;
	SDL_Event e;
	int ret=0;
	char buf[strlen(message)+20];
	bt = SDL_GetTicks();
	while ((t = SDL_GetTicks() - bt) < time) {
		sprintf(buf, "%s\n\n%d sec",message,(time/1000)-(t/1000));
		text_messagebox(title, buf, MB_NONE);
		SDL_Delay(20);
		if (SDL_PollEvent(&e) &&
			!uib_handle_event(&e) &&
			e.type == SDL_KEYDOWN &&
			e.key.keysym.sym > 0) {
			ret = e.key.keysym.sym;
		}
		uib_update();
		if (ret > 0) break;
	}
	return ret;
}

#define POLLTIME 5000

void uae3ds_mapping_add()
{
	char buf[256];
	int x,i;
	int target[2] = {0};
	
	if (!(i=pollKey(" Add Key Mapping","Press source key",POLLTIME))) return;
	for (x = 0; x < 2; x++) {
		snprintf(buf, 256, "Press target key %d/2", x+1);
		target[x]=pollKey(" Add Key Mapping",buf,POLLTIME);
		if (!target[x]) break;
	}
	if (!target[0]) return;
	keymaps[i][0]=target[0];
	keymaps[i][1]=target[1];
	snprintf(buf, 256, "Added key mapping:\n\n%s -> %s",
		get_3ds_keyname(i),
		get_3ds_mapping_name(i));
	text_messagebox(" Add Key Mapping", buf, MB_OK);
}

void uae3ds_mapping_del()
{
	char buf[256];
	int i;
	
	if ((i=pollKey(" Delete Key Mapping","Press source key",POLLTIME))<0) return;
	if (!keymaps[i][0]) {
		snprintf(buf, 256, "Key '%s' is not mapped", get_3ds_keyname(i));
	} else {
		snprintf(buf, 256, "Mapping for key '%s' deleted", get_3ds_keyname(i));
		keymaps[i][0] = keymaps[i][1] = 0;
	}
	text_messagebox(" Delete Key Mapping", buf, MB_OK);
}

void uae3ds_mapping_list()
{
	char buf[41 * 30] = {0};
	int pos, got = 0;

	for (pos = 0; pos < 0x200; pos++) {
		if (keymaps[pos][0]) {
			got = 1;
			if (strlen(buf) > 20*41) {
				sprintf(buf + strlen(buf), "\nMORE >>");
				text_messagebox(" List Key Mappings", buf, MB_OK);
				*buf=0;
			}
			sprintf(buf + strlen(buf), "%s%-10s : ", strlen(buf)?"\n":"", get_3ds_keyname(pos));
			sprintf(buf + strlen(buf), "%-27s", get_3ds_mapping_name(pos));
		}
	}
	if (*buf != 0) {
		text_messagebox(" List Key Mappings", buf, MB_OK);
	} else if (!got) {
		text_messagebox(" List Key Mappings", "No key mappings defined", MB_OK);
	}
}

void uae3ds_mapping_apply(SDL_Event *e)
{
	int sym;
	if (e->type != SDL_KEYUP && e->type != SDL_KEYDOWN) return;
	sym = e->key.keysym.sym;
	if (!keymaps[sym][0]) return;
	e->key.keysym.sym = keymaps[sym][0];
	if (!keymaps[sym][1]) return;
	SDL_Event e1;
	e1.type = e->type;
	e1.key.keysym.sym = keymaps[sym][1];
	SDL_PushEvent(&e1);
}

void uae3ds_mapping_loadbuf(char *s)
{
	unsigned long int l;
	while ((l = strtoul(s, &s, 16)) != 0) {
		int k = (l >> 18) & 0x1ff;
		keymaps[k][0] = (l >> 9) & 0x1ff;
		keymaps[k][1] = l & 0x1ff;
		while (*s=='_') ++s;
	}
}

char *uae3ds_mapping_savebuf()
{
	char *s;
	int i, count=0;
	for (i=0; i<0x200; i++) {
		if (keymaps[i][0]) ++count;
	}
	s=calloc(count*9+1, 1);
	for (i=0; i<0x200; i++) {
		if (keymaps[i][0]) {
			sprintf(s+strlen(s),"%s%x", strlen(s)?"_":"", (i << 18) | ((keymaps[i][0] & 0x1ff) << 9) | (keymaps[i][1] & 0x1ff));
		}
	}
	return s;
}

void uae3ds_mapping_save()
{
	char *cfgfile = ROM_PATH_PREFIX CFG_FILE_NAME;
	char *s = uae3ds_mapping_savebuf();

	FILE *f;
	if ((f = fopen(cfgfile, "w")) != NULL)
	{
		fprintf(f, "keymappings=%s", s);
		fclose(f);	
	}
	free(s);
	text_messagebox(" Save Key Mappings", "Key mappings saved", MB_OK);
}