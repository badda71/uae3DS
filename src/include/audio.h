 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Sound emulation stuff
  *
  * Copyright 1995, 1996, 1997 Bernd Schmidt
  */

#define PERIOD_MAX ULONG_MAX

#ifndef UAE4ALL_ALIGN
#error UAE4ALL_ALIGN NO DEFINIDO
#endif

#if defined(NO_THREADS) && defined(DREAMCAST)
#define PRE_SNDBUFFER_LEN (960)
#else
#define PRE_SNDBUFFER_LEN (1024)
#endif

#if !defined(DREAMCAST) && !defined(DINGOO)
#define SNDBUFFER_LEN (PRE_SNDBUFFER_LEN*2)
#else
#define SNDBUFFER_LEN PRE_SNDBUFFER_LEN
#endif

extern unsigned sound_quality;

/*
extern unsigned long audio_channel_per[];
extern uae_u8 audio_channel_dmaen[];
extern uae_u8 audio_channel_intreq2[];
extern uae_u8 audio_channel_data_written[];
extern uaecptr audio_channel_lc[];
extern uaecptr audio_channel_pt[];
extern int audio_channel_wper[];
extern unsigned int audio_channel_wlen[];
extern int audio_channel_last_sample[];
extern uae_u16 audio_channel_dat[];
extern uae_u16 audio_channel_nextdat[];
extern uae_u16 audio_channel_len[];
*/

struct audio_channel_data{
//    unsigned long adk_mask;
//    unsigned long evtime;
    unsigned long per;
    uae_u8 dmaen, intreq2, data_written;
    uaecptr lc, pt;
//    int state;
    int wper;
    unsigned int wlen;
//    int current_sample;
    int last_sample;
//    int vol;
    uae_u16 dat, nextdat, len;
} UAE4ALL_ALIGN;

/*
extern int audio_channel_current_sample[];
extern int audio_channel_vol[];
extern unsigned long audio_channel_adk_mask[];
extern int audio_channel_state[];
extern unsigned long audio_channel_evtime[];

extern struct audio_channel_data audio_channel[] UAE4ALL_ALIGN;
*/

/*
extern struct audio_channel_data *audchs[];
extern struct audio_channel_data audch_0;
extern struct audio_channel_data audch_1;
extern struct audio_channel_data audch_2;
extern struct audio_channel_data audch_3;
extern struct audio_channel_data audch_4;
extern struct audio_channel_data audch_5;
*/

/*
extern void aud0_handler (void);
extern void aud1_handler (void);
extern void aud2_handler (void);
extern void aud3_handler (void);
*/

extern void AUDxDAT (int nr, uae_u16 value);
extern void AUDxVOL (int nr, uae_u16 value);
extern void AUDxPER (int nr, uae_u16 value);
extern void AUDxLCH (int nr, uae_u16 value);
extern void AUDxLCL (int nr, uae_u16 value);
extern void AUDxLEN (int nr, uae_u16 value);

extern int init_audio (void);
extern void ahi_install (void);
extern void audio_reset (void);
extern void update_audio (void);
extern void schedule_audio (void);
extern void audio_evhandler (void);
// extern void audio_channel_enable_dma (int n_channel);
// extern void audio_channel_disable_dma (int n_channel);
extern void check_dma_audio(void);
extern void fetch_audio(void);
extern void update_adkmasks (void);
