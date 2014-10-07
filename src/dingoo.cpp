#include <stdlib.h>
#include <stdio.h> 
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/soundcard.h>
#include <sys/ioctl.h>

/* Define this to the CPU frequency */
#define DEFAULT_CPU_FREQ 336000000    /* CPU clock: 336 MHz */
#define CFG_EXTAL 12000000    /* EXT clock: 12 Mhz */

// SDRAM Timings, unit: ns
#define SDRAM_TRAS	45	/* RAS# Active Time */
#define SDRAM_RCD	20	/* RAS# to CAS# Delay */
#define SDRAM_TPC	20	/* RAS# Precharge Time */
#define SDRAM_TRWL	7	/* Write Latency Time */
#define SDRAM_TREF	15625	/* Refresh period: 4096 refresh cycles/64ms */ 

/* Clock Control Register */
#define CPM_CPCCR_I2CS        (1 << 31)
#define CPM_CPCCR_CLKOEN    (1 << 30)
#define CPM_CPCCR_UCS        (1 << 29)
#define CPM_CPCCR_UDIV_BIT    23
#define CPM_CPCCR_UDIV_MASK    (0x3f << CPM_CPCCR_UDIV_BIT)
#define CPM_CPCCR_CE        (1 << 22)
#define CPM_CPCCR_PCS        (1 << 21)
#define CPM_CPCCR_LDIV_BIT    16
#define CPM_CPCCR_LDIV_MASK    (0x1f << CPM_CPCCR_LDIV_BIT)
#define CPM_CPCCR_MDIV_BIT    12
#define CPM_CPCCR_MDIV_MASK    (0x0f << CPM_CPCCR_MDIV_BIT)
#define CPM_CPCCR_PDIV_BIT    8
#define CPM_CPCCR_PDIV_MASK    (0x0f << CPM_CPCCR_PDIV_BIT)
#define CPM_CPCCR_HDIV_BIT    4
#define CPM_CPCCR_HDIV_MASK    (0x0f << CPM_CPCCR_HDIV_BIT)
#define CPM_CPCCR_CDIV_BIT    0
#define CPM_CPCCR_CDIV_MASK    (0x0f << CPM_CPCCR_CDIV_BIT)

/* I2S Clock Divider Register */
#define CPM_I2SCDR_I2SDIV_BIT    0
#define CPM_I2SCDR_I2SDIV_MASK    (0x1ff << CPM_I2SCDR_I2SDIV_BIT)

/* PLL Control Register */
#define CPM_CPPCR_PLLM_BIT    23
#define CPM_CPPCR_PLLM_MASK    (0x1ff << CPM_CPPCR_PLLM_BIT)
#define CPM_CPPCR_PLLN_BIT    18
#define CPM_CPPCR_PLLN_MASK    (0x1f << CPM_CPPCR_PLLN_BIT)
#define CPM_CPPCR_PLLOD_BIT    16
#define CPM_CPPCR_PLLOD_MASK    (0x03 << CPM_CPPCR_PLLOD_BIT)
#define CPM_CPPCR_PLLS        (1 << 10)
#define CPM_CPPCR_PLLBP        (1 << 9)
#define CPM_CPPCR_PLLEN        (1 << 8)
#define CPM_CPPCR_PLLST_BIT    0
#define CPM_CPPCR_PLLST_MASK    (0xff << CPM_CPPCR_PLLST_BIT)


static volatile unsigned long  *jz_cpmregl;
static volatile unsigned short *jz_emcregs; 

static int sdram_convert(unsigned int pllin,unsigned int *sdram_freq)
{
	register unsigned int ns, tmp;
 
	ns = 1000000000 / pllin;
	/* Set refresh registers */
	tmp = SDRAM_TREF/ns;
	tmp = tmp/64 + 1;
	if (tmp > 0xff) tmp = 0xff;
        *sdram_freq = tmp; 

	return 0;

}

 
static void pll_init(unsigned int clock)
{
	register unsigned int cfcr, plcr1;
	unsigned int sdramclock = 0;
	int n2FR[33] = {
		0, 0, 1, 2, 3, 0, 4, 0, 5, 0, 0, 0, 6, 0, 0, 0,
		7, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0,
		9
	};
  	int div[5] = {1, 3, 3, 3, 3}; /* divisors of I:S:P:L:M */
	int nf, pllout2;

	cfcr = CPM_CPCCR_CLKOEN |
		(n2FR[div[0]] << CPM_CPCCR_CDIV_BIT) | 
		(n2FR[div[1]] << CPM_CPCCR_HDIV_BIT) | 
		(n2FR[div[2]] << CPM_CPCCR_PDIV_BIT) |
		(n2FR[div[3]] << CPM_CPCCR_MDIV_BIT) |
		(n2FR[div[4]] << CPM_CPCCR_LDIV_BIT);

	pllout2 = (cfcr & CPM_CPCCR_PCS) ? clock : (clock / 2);

	/* Init UHC clock */
    	jz_cpmregl[0x6C>>2] = pllout2 / 48000000 - 1;

	nf = clock * 2 / CFG_EXTAL;
	plcr1 = ((nf - 2) << CPM_CPPCR_PLLM_BIT) | /* FD */
		(0 << CPM_CPPCR_PLLN_BIT) |	/* RD=0, NR=2 */
		(0 << CPM_CPPCR_PLLOD_BIT) |    /* OD=0, NO=1 */
		(0x20 << CPM_CPPCR_PLLST_BIT) | /* PLL stable time */
		CPM_CPPCR_PLLEN;                /* enable PLL */          

	/* init PLL */
      	jz_cpmregl[0] = cfcr;
    	jz_cpmregl[0x10>>2] = plcr1;
	
  	sdram_convert(clock,&sdramclock);
  	if(sdramclock > 0)
  	{
		jz_emcregs[0x8C>>1] = sdramclock;
		jz_emcregs[0x88>>1] = sdramclock;	
  	}
}


void dingoo_set_clock(unsigned int mhz) 
{
	unsigned long jz_dev=open("/dev/mem",   O_RDWR);  
	volatile unsigned long *jz_emcregl;

	jz_cpmregl=(unsigned long  *)mmap(0, 0x80, PROT_READ|PROT_WRITE, MAP_SHARED, jz_dev, 0x10000000);
	jz_emcregl=(unsigned long  *)mmap(0, 0x90, PROT_READ|PROT_WRITE, MAP_SHARED, jz_dev, 0x13010000);
	jz_emcregs=(unsigned short *)jz_emcregl;

	if (mhz > 430) mhz = 430;
	if (mhz < 200) mhz = 200;

	pll_init(mhz*1000000);

	munmap((void *)jz_cpmregl, 0x80); 
	munmap((void *)jz_emcregl, 0x90); 	
	close(jz_dev);
}

unsigned int dingoo_get_clock(void)
{
	unsigned long dev=open("/dev/mem",   O_RDWR);  
	volatile unsigned long  *jz=(unsigned long  *)mmap(0, 0x80, PROT_READ|PROT_WRITE, MAP_SHARED, dev, 0x10000000);
	unsigned int plcr1=jz[0x10>>2];
	int nf=2+(plcr1 >> CPM_CPPCR_PLLM_BIT);
	munmap((void *)jz, 0x80); 
	close(dev);
	return 2+(nf*6);
}

void dingoo_set_brightness(int value)
{
	unsigned long dev=open("/proc/jz/lcd_backlight", O_RDWR);
	if (dev)
	{
		char backlight[4];
		if (value > 99) value = 99;
		if (value < 0) value = 0;
		snprintf(backlight, 3, "%d", value);
		backlight[3]='\0';
		write(dev, backlight, 4);
		close(dev);
		sync();
	}
}

int dingoo_get_brightness(void)
{
	unsigned long dev=open("/proc/jz/lcd_backlight", O_RDWR);
	if (dev)
	{
		char backlight[4]={ 0, 0, 0, 0 };
		read(dev, backlight, 4);
		backlight[3]='\0';
		close(dev);
		return 1+atoi(backlight);
	}
	return 0;
}

void dingoo_set_volumen(int value)
{
	unsigned long dev=open("/dev/mixer", O_WRONLY);
	if (dev)
	{
		if (value > 100) value = 100;
		if (value < 0) value = 0;
		value|=(value<<8);
		ioctl(dev, SOUND_MIXER_WRITE_VOLUME, &value);
		close(dev);
		sync();
	}
}

int dingoo_get_volumen(void)
{
	unsigned long dev=open("/dev/mixer", O_WRONLY);
	if (dev)
	{
		int value=0;
		ioctl(dev, SOUND_MIXER_READ_VOLUME, &value);
		close(dev);
		return value&0xff;
	}
	return 0;
}
