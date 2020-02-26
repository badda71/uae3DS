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
#include "uae3ds.h"

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
