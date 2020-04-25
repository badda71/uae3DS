#include "sysconfig.h"
#include "sysdeps.h"
#include <3ds.h>
#include <SDL/SDL.h>
#include "uibottom.h"
#include "autofire.h"

//The thread that will be used
static SDL_Thread *af_thread = NULL;
static Handle af_start = 0;
static volatile int af_active=0;
static int af_init = 0;
int autofire_speed = 10;

static int autofire_thread( void *data ) {
	SDL_Event e1;
	SDL_Event e2;
	e1.type = SDL_KEYDOWN;
	e2.type = SDL_KEYUP;
	e1.key.keysym.sym = e2.key.keysym.sym = DS_A;

	//While the program is not over
	while( af_active >= 0 ) {
		svcWaitSynchronization(af_start, U64_MAX);
		svcClearEvent(af_start);
		int s=500/autofire_speed;
		
		// post fire events
		while (af_active!=0) {
			SDL_PushEvent(&e1);
			SDL_Delay(s);
			SDL_PushEvent(&e2);
			SDL_Delay(s);
		}
		svcClearEvent(af_start);
	}
	return 0;
}

int autofire_init() {
	// start autofire threads
	if (!af_init) {
		svcCreateEvent(&af_start,0);
		if ((af_thread = SDL_CreateThread(autofire_thread, NULL)) == NULL ) {
			svcCloseHandle(af_start);
			return -1;
		}
		af_init=1;
		return 0;
	}
	return -1;
}

int autofire_shutdown() {
	if (af_init) {
		af_active=-1;
		if (af_thread) SDL_KillThread (af_thread);
		if (af_start) svcCloseHandle(af_start);
		af_init=0;
		return 0;
	}
	return -1;
}

int autofire_start() {
	if (!af_init && autofire_init()!=0) return -1;
	af_active =1;
	svcSignalEvent(af_start);
	return 0;
}

int autofire_stop() {
	if (!af_init && autofire_init()!=0) return -1;
	svcClearEvent(af_start);
	af_active = 0;
	return 0;
}
