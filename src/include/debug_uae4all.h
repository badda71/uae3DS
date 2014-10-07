#include <SDL.h>

#ifndef DEBUG_UAE4ALL_H
#define DEBUG_UAE4ALL_H

#ifdef DEBUG_UAE4ALL

extern int DEBUG_AHORA;
#ifdef DEBUG_FILE
extern FILE *DEBUG_STR_FILE;
#else
#define DEBUG_STR_FILE stdout
#endif

#ifdef DEBUG_UAE4ALL_FFLUSH
#define UAE4ALL_FFLUSH fflush(DEBUG_STR_FILE);
#else
#define UAE4ALL_FFLUSH
#endif

#ifdef DEBUG_FILE
#define dbg(TEXTO) \
	if (DEBUG_AHORA) \
	{ \
		fprintf(DEBUG_STR_FILE,"%s\n",TEXTO); \
		UAE4ALL_FFLUSH \
	}


#define dbgf(FORMATO, RESTO...) \
	if (DEBUG_AHORA) \
	{ \
		fprintf (DEBUG_STR_FILE,FORMATO , ## RESTO); \
		UAE4ALL_FFLUSH \
	}
#else
#define dbg(TEXTO) \
	if (DEBUG_AHORA) \
	{ \
		puts(TEXTO); \
		UAE4ALL_FFLUSH \
	}


#define dbgf(FORMATO, RESTO...) \
	if (DEBUG_AHORA) \
	{ \
		printf(FORMATO , ## RESTO); \
		UAE4ALL_FFLUSH \
	}
#endif


static __inline__ void dbgsum(char *str, void *buff, unsigned len)
{
	if (DEBUG_AHORA)
	{
		unsigned char *p=(unsigned char *)buff;
		unsigned i,ret=0;
		for(i=0;i<len;i++,p++)
			ret+=(*p)*i;
		fprintf(DEBUG_STR_FILE,"%s : 0x%X\n",str,ret);
#ifdef DEBUG_UAE4ALL_FFLUSH
		fflush(DEBUG_STR_FILE);
#endif
	}
}


#else

#define dbg(TEXTO)
#define dbgf(FORMATO, RESTO...)
#define debsum(STR)

#endif



#ifndef PROFILER_UAE4ALL

#define uae4all_prof_start(A)
#define uae4all_prof_end(A)

#else

#define UAE4ALL_PROFILER_MAX 256

extern unsigned long long uae4all_prof_initial[UAE4ALL_PROFILER_MAX];
extern unsigned long long uae4all_prof_sum[UAE4ALL_PROFILER_MAX];
extern unsigned long long uae4all_prof_executed[UAE4ALL_PROFILER_MAX];

static __inline__ void uae4all_prof_start(unsigned a)
{
	uae4all_prof_executed[a]++;
#ifndef DREAMCAST
	uae4all_prof_initial[a]=SDL_GetTicks();
#else
	uae4all_prof_initial[a]=timer_us_gettime64();
#endif
}


static __inline__ void uae4all_prof_end(unsigned a)
{
#ifndef DREAMCAST
	uae4all_prof_sum[a]+=SDL_GetTicks()-uae4all_prof_initial[a];
#else
	extern unsigned uae4all_prof_total;
	int i;
	for(i=0;i<uae4all_prof_total;i++)
		uae4all_prof_initial[i]+=6;
	uae4all_prof_sum[a]+=timer_us_gettime64()-uae4all_prof_initial[a]+2;
#endif
}

void uae4all_prof_init(void);
void uae4all_prof_add(char *msg);
void uae4all_prof_show(void);

#endif


#endif


