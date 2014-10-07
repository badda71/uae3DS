 /*
  * UAE - The Un*x Amiga Emulator
  * 
  * Generic thread support doesn't exist.
  * 
  * Copyright 1997 Bernd Schmidt
  */

#ifdef NO_THREADS

#undef SUPPORT_PENGUINS
#undef SUPPORT_THREADS

typedef int uae_sem_t;
#define uae_sem_init(a,b,c)
#define uae_sem_destroy(a)
#define uae_sem_post(a)
#define uae_sem_wait(a)
#define uae_sem_trywait(a) 0
#define uae_sem_getvalue(a,b) 0

typedef int smp_comm_pipe;
#define write_comm_pipe_int(a,b,c)
#define read_comm_pipe_int_blocking(a) 0
#define write_comm_pipe_pvoid(a,b,c)
#define read_comm_pipe_pvoid_blocking(a) 0
#define init_comm_pipe(a,b,c)
#define comm_pipe_has_data(a) 0

typedef int uae_thread_id;


#else


#include "SDL.h"
#include "SDL_thread.h"

/* Sempahores. We use POSIX semaphores; if you are porting this to a machine
 * with different ones, make them look like POSIX semaphores. */
typedef SDL_sem *uae_sem_t;

#define uae_sem_init(PSEM, DUMMY, INIT) do { \
    *PSEM = SDL_CreateSemaphore (INIT); \
} while (0)
#define uae_sem_destroy(PSEM) SDL_DestroySemaphore (*PSEM)
#define uae_sem_post(PSEM) SDL_SemPost (*PSEM)
#define uae_sem_wait(PSEM) SDL_SemWait (*PSEM)
#define uae_sem_trywait(PSEM) SDL_SemTryWait (*PSEM)
#define uae_sem_getvalue(PSEM) SDL_SemValue (*PSEM)

#include "commpipe.h"

typedef SDL_Thread *uae_thread_id;
#define BAD_THREAD NULL

static __inline__ int uae_start_thread (void *(*f) (void *), void *arg, uae_thread_id *foo)
{
    *foo = SDL_CreateThread ((int (*)(void *))f, arg);
    return *foo == 0;
}

/* Do nothing; thread exits if thread function returns.  */
#define UAE_THREAD_EXIT do {} while (0)

#endif
