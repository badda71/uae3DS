 /*
  * UAE - The Un*x Amiga Emulator
  *
  * routines to handle compressed file automatically
  *
  * (c) 1996 Samuel Devulder, Tim Gunn
  */

#include "sysconfig.h"
#include "sysdeps.h"

#include "config.h"
#include "uae.h"
#include "options.h"
#include "memory.h"
#include "zfile.h"

#include "savedisk.h"

#include <zlib.h>

#define MAX_COMP_SIZE (1024*128)

extern int mainMenu_autosave;

#ifdef DREAMCAST
#include <kos.h>
#include"save_icon.h"
#define DC_VRAM 0x04000000
#define VMUFILE_PAD 128+512
static unsigned char    *paquete=NULL;
static int              paquete_size=0;


static void prepare_save(void)
{
	if (paquete)
        	return;
	char *str="UAE4ALL";
	vmu_pkg_t pkg;
	memset(&pkg, 0, sizeof(pkg));
	strcpy(pkg.desc_short, str);
	strcpy(pkg.desc_long, str);
	strcpy(pkg.app_id, str);
	pkg.icon_cnt = 1;
	pkg.icon_anim_speed = 0;
	pkg.eyecatch_type = VMUPKG_EC_NONE;
	pkg.eyecatch_data = NULL;
	pkg.data_len = 4;
	pkg.data = (const uint8*)&pkg;
	memcpy((void *)&pkg.icon_pal[0],(void *)&vmu_savestate_icon_pal,32);
	pkg.icon_data = (const uint8*)&vmu_savestate_icon_data;
	vmu_pkg_build(&pkg, &paquete, &paquete_size);
	paquete_size-=4;
}

static void rebuild_paquete(char *name, unsigned size, unsigned char* data, FILE *f)
{
	unsigned short *crc=(unsigned short*) &paquete[0x46];
	unsigned *data_len=(unsigned *) &paquete[0x48];
	char *desc_long=(char *) &paquete[16];
	bzero(desc_long,32);
	strncpy(desc_long,name,31);
	*data_len=size;
	int i, c, n = 0;
	(*crc)=0;
	for (i = 0; i < paquete_size; i++)
	{
		if (i<VMUFILE_PAD)
			n ^= (paquete[i]<<8);
		else
			n ^= (data[i-VMUFILE_PAD]<<8);
		for (c = 0; c < 8; c++)
			if (n & 0x8000)
				 n = (n << 1) ^ 4129;
			else
				n = (n << 1);
	}
	(*crc)=(n & 0xffff);
	fwrite((void *)&paquete[0],1,VMUFILE_PAD,f);
}

static void set_vmu_pad(FILE *f)
{
	fseek(f,VMUFILE_PAD,SEEK_SET);
}

#else
#define VMUFILE_PAD 0
#define prepare_save()
#define rebuild_paquete(A,B,C,D)
#define set_vmu_pad(A)
#define maple_first_vmu() 1
#endif


static unsigned getFreeBlocks(void)
{
#ifdef DREAMCAST
	unsigned short buf16[255];
	int free_blocks=0,i=0;
	maple_device_t *dev;
	unsigned char addr=0;
	int p,u;
	uint8 v=maple_first_vmu();
	if (v)
	{
		if (maple_compat_resolve(v,&dev,MAPLE_FUNC_MEMCARD)!=0)
			return 0;
	}
	else
		return 0;

	return vmufs_free_blocks(dev);
#else
	return 0x10000;
#endif
}

static void eliminate_file(char *filename)
{
	FILE *f=fopen(filename,"r");
	if (f)
	{
		fclose(f);
		unlink(filename);
	}
}

#define VRAM_MAX_LEN (384*1024)
#define MAX_DISK_LEN 1024*(1024-128)
#define MAX_ROM_LEN  (MAX_DISK_LEN-VRAM_MAX_LEN)
static void *uae4all_rom_memory=NULL;
static unsigned uae4all_rom_len=0;
static unsigned uae4all_rom_pos=0;
#ifdef DREAMCAST
void *uae4all_vram_memory_free=(void *)0x05500000;
#endif

static void *uae4all_disk_memory[4]={ NULL ,NULL ,NULL ,NULL };
static void *uae4all_extra_buffer=NULL;
static unsigned uae4all_disk_len[4]={ 0 ,0 ,0 ,0 };
static unsigned uae4all_disk_pos[4]={ 0 ,0 ,0 ,0 };
static unsigned char uae4all_disk_used[4]= { 0 ,0 ,0 ,0 };
static int uae4all_disk_writed[4]= { 0, 0, 0, 0 };
static int uae4all_disk_writed_now[4]= { 0, 0, 0, 0 };
static void *uae4all_disk_orig[4]={ NULL, NULL, NULL, NULL };
static unsigned uae4all_disk_crc[4]={ 0, 0, 0, 0 };
static unsigned uae4all_disk_actual_crc[4]={ 0, 0, 0, 0};

void zfile_exit (void)
{
	int i;
	for(i=0;i<NUM_DRIVES;i++)
		if (uae4all_disk_memory[i])
		{
#ifndef DREAMCAST
			free(uae4all_disk_memory[i]);
#endif
			uae4all_disk_used[i]=0;
			uae4all_disk_memory[i]=NULL;
		}
}

int zfile_close (FILE *f)
{
	int i;
	for(i=0;i<NUM_DRIVES;i++)
		if (f==uae4all_disk_memory[i])
		{
			uae4all_disk_used[i]=0;
			break;
		}
	return 0;
}


#ifdef USE_ZFILE
#define mi_z_type gzFile
#define mi_z_open(NAME,P) gzopen(NAME,P)
#define mi_z_seek(F,O,P) gzseek(F,O,P)
#define mi_z_tell(F) gztell(F)
#define mi_z_read(F,M,S) gzread(F,M,S)
#define mi_z_close(F) gzclose(F)
#else
#define mi_z_type FILE *
#define mi_z_open(NAME,P) fopen(NAME,P)
#define mi_z_seek(F,O,P) fseek(F,O,P)
#define mi_z_tell(F) ftell(F)
#define mi_z_read(F,M,S) fread(M,1,S,F)
#define mi_z_close(F) fclose(F)
#endif

static int  try_to_read_disk(int i,const char *name)
{
    mi_z_type f=mi_z_open(name,"rb");
    if (f)
    {
	    int readed=mi_z_read(f,uae4all_disk_memory[i],MAX_DISK_LEN);
	    mi_z_close(f);
	    if (readed>0)
 	    {
	    	uae4all_disk_len[i]=readed;
	    	return readed;
	    }
    }
#ifdef USE_ZFILE
    FILE * f2=fopen(name,"rb");
    if (f2)
    {
	    fseek(f2,0,SEEK_END);
	    uae4all_disk_len[i]=ftell(f2);
	    fseek(f2,0,SEEK_SET);
	    if (uae4all_disk_len[i]>MAX_DISK_LEN)
		uae4all_disk_len[i]=MAX_DISK_LEN;
	    uae4all_disk_len[i]=fread(uae4all_disk_memory[i],1,uae4all_disk_len[i],f2);
	    fclose(f2);
	    return (uae4all_disk_len[i]);
    }
#endif
    return 0;
}

static char __uae4all_write_namefile[32];

static char *get_namefile(unsigned num)
{
	unsigned crc=uae4all_disk_crc[num];
	sprintf((char *)&__uae4all_write_namefile[0],SAVE_PREFIX "%.8X.ads",crc);
	return (char *)&__uae4all_write_namefile[0];
}

static void uae4all_disk_real_write(int num)
{
	unsigned new_crc=savedisk_get_checksum(uae4all_disk_memory[num],MAX_DISK_LEN);
	if (new_crc!=uae4all_disk_actual_crc[num])
	{
		void *buff=uae4all_disk_memory[num];
		void *buff_patch=uae4all_extra_buffer;
		memset(buff_patch,0,MAX_DISK_LEN);
		unsigned changed=savedisk_get_changes(buff,MAX_DISK_LEN,buff_patch,uae4all_disk_orig[num]);
		if ((changed)&&(changed<MAX_DISK_LEN))
		{
			char *namefile=get_namefile(num);
			void *bc=calloc(1,MAX_COMP_SIZE);
			unsigned long sizecompressed=MAX_COMP_SIZE;
			int retc=compress2((Bytef *)bc,&sizecompressed,(const Bytef *)uae4all_extra_buffer,changed,Z_BEST_COMPRESSION);
			if (retc>=0)
			{
				unsigned usado=0;
				{
					FILE *f=fopen(namefile,"rb");
					if (f)
					{
						fseek(f,0,SEEK_END);
						usado=ftell(f);
						fclose(f);
						usado/=512;
					}
				}
				if ( ((getFreeBlocks()+usado)*512) >=(sizecompressed+VMUFILE_PAD))
				{
					eliminate_file(namefile);
					FILE *f=fopen(namefile,"wb");
					if (f)
					{
						rebuild_paquete(prefs_df[num], sizecompressed, (unsigned char*) bc, f);
						fwrite((void *)&sizecompressed,1,4,f);
						fwrite(bc,1,sizecompressed,f);
						fclose(f);
					}
				}
			}
			free(bc);
			uae4all_disk_actual_crc[num]=new_crc;
		}
	}
}



static void uae4all_initsave(unsigned num)
{
#ifndef DREAMCAST
	if (!uae4all_disk_orig[num])
		uae4all_disk_orig[num]=malloc(MAX_DISK_LEN);
#else
	uae4all_disk_orig[num]=(void *)(DC_VRAM+(MAX_DISK_LEN*(num+4)));
#endif
	memcpy(uae4all_disk_orig[num],uae4all_disk_memory[num],MAX_DISK_LEN);
	uae4all_disk_crc[num]=savedisk_get_checksum(uae4all_disk_orig[num],MAX_DISK_LEN);
	if ((!mainMenu_autosave)||(!maple_first_vmu()))
		return;
	FILE *f=fopen(get_namefile(num),"rb");
	if (f)
	{
		void *bc=calloc(1,MAX_COMP_SIZE);
		unsigned long n;
		set_vmu_pad(f);
		fread((void *)&n,1,4,f);
		if (fread(bc,1,n,f)>=n)
		{
			unsigned long sizeuncompressed=MAX_DISK_LEN;
			int retc=uncompress((Bytef *)uae4all_extra_buffer,&sizeuncompressed,(const Bytef *)bc,n);
			if (retc>=0)
			{
				savedisk_apply_changes(uae4all_disk_memory[num],uae4all_extra_buffer,sizeuncompressed);
			}
			else
			{
				fclose(f);
				f=NULL;
				eliminate_file(get_namefile(num));
			}
		}
		free(bc);
		if (f)
			fclose(f);
	}
	uae4all_disk_actual_crc[num]=savedisk_get_checksum(uae4all_disk_memory[num],MAX_DISK_LEN);
}



FILE *zfile_open (const char *name, const char *mode)
{
    int i;
    for(i=0;i<NUM_DRIVES;i++)
    	if (uae4all_disk_memory[i]==NULL)
    	{
#ifndef DREAMCAST
		uae4all_disk_memory[i]=malloc(MAX_DISK_LEN);
#else
		uae4all_disk_memory[i]=(void *)(DC_VRAM+(MAX_DISK_LEN*(i+1)));
#endif
		bzero(uae4all_disk_memory[i],MAX_DISK_LEN);
		uae4all_disk_used[i]=0;
        }
    if (uae4all_extra_buffer==NULL)
#ifndef DREAMCAST
		uae4all_extra_buffer=malloc(MAX_DISK_LEN);
#else
		uae4all_extra_buffer=(void *)(DC_VRAM+(MAX_DISK_LEN*(NUM_DRIVES+1)));
#endif
    for(i=0;i<NUM_DRIVES;i++)
	if (!uae4all_disk_used[i])
		break;
    if (i>=NUM_DRIVES)
	return NULL;

    if (try_to_read_disk(i,name))
    {

	    uae4all_disk_pos[i]=0;
	    uae4all_disk_writed[i]=0;
	    uae4all_disk_used[i]=1;
	    uae4all_initsave(i);
	    return (FILE *)uae4all_disk_memory[i];
    }
    return NULL;
}


size_t uae4all_fread( void *ptr, size_t tam, size_t nmiemb, FILE *flujo)
{
	int i;
	for(i=0;i<NUM_DRIVES;i++)
		if (flujo==uae4all_disk_memory[i])
			break;
	if (i>=NUM_DRIVES)
		return 0;
	if (uae4all_disk_pos[i]>=uae4all_disk_len[i])
		return 0;
	memcpy(ptr,(void *)(((unsigned)uae4all_disk_memory[i])+((unsigned)uae4all_disk_pos[i])),tam*nmiemb);
	uae4all_disk_pos[i]+=tam*nmiemb;
	return nmiemb;
}

size_t uae4all_fwrite( void *ptr, size_t tam, size_t nmiemb, FILE *flujo)
{
	int i;
	for(i=0;i<NUM_DRIVES;i++)
		if (flujo==uae4all_disk_memory[i])
			break;
	if (i>=NUM_DRIVES)
		return 0;
	if (uae4all_disk_pos[i]>=uae4all_disk_len[i])
		return 0;
	memcpy((void *)(((unsigned)uae4all_disk_memory[i])+((unsigned)uae4all_disk_pos[i])),ptr,tam*nmiemb);
	uae4all_disk_pos[i]+=tam*nmiemb;
	uae4all_disk_writed[i]=1;
	return nmiemb;
}

int uae4all_fseek( FILE *flujo, long desplto, int origen)
{
	int i;
	for(i=0;i<NUM_DRIVES;i++)
		if (flujo==uae4all_disk_memory[i])
			break;
	if (i>=NUM_DRIVES)
		return -1;
	switch(origen)
	{
		case SEEK_SET:
			uae4all_disk_pos[i]=desplto;
			break;
		case SEEK_CUR:
			uae4all_disk_pos[i]+=desplto;
			break;
		default:
			uae4all_disk_pos[i]=uae4all_disk_len[i];
	}
	if (uae4all_disk_pos[i]<=uae4all_disk_len[i])
		return 0;
	uae4all_disk_pos[i]=uae4all_disk_len[i];
	return -1;
}

long uae4all_ftell( FILE *flujo)
{
	int i;
	for(i=0;i<NUM_DRIVES;i++)
		if (flujo==uae4all_disk_memory[i])
			break;
	if (i>=NUM_DRIVES)
		return 0;
	return uae4all_disk_pos[i];
}

int uae4all_init_rom(const char *name)
{
	prepare_save();
	FILE *f=fopen(name,"rb");
	if (f)
	{
#ifndef DREAMCAST
		uae4all_rom_memory=calloc(1,MAX_ROM_LEN);
#else
		uae4all_rom_memory=(void *)(DC_VRAM+VRAM_MAX_LEN);
		bzero(uae4all_rom_memory,MAX_ROM_LEN);
#endif
		fseek(f,0,SEEK_END);
		uae4all_rom_len=ftell(f);
		fseek(f,0,SEEK_SET);
		if (uae4all_rom_len>MAX_ROM_LEN)
		    uae4all_rom_len=MAX_ROM_LEN;
		fread(uae4all_rom_memory,uae4all_rom_len,1,f);
		uae4all_rom_pos=0;
		fclose(f);
#ifdef DREAMCAST
		uae4all_vram_memory_free=(void *)(DC_VRAM+(MAX_DISK_LEN*(NUM_DRIVES+2)));
#endif
		return 0;
	}
	return -1;
}

void uae4all_rom_reinit(void)
{
	prepare_save();
	uae4all_rom_memory=NULL;
}

FILE *uae4all_rom_fopen(const char *name, const char *mode)
{
	prepare_save();
	if (!uae4all_rom_memory)
		uae4all_init_rom(name);
	return ((FILE *)uae4all_rom_memory);
}

int uae4all_rom_fclose(FILE *flujo)
{
	return (flujo!=((FILE *)uae4all_rom_memory));
}

int uae4all_rom_fseek( FILE *flujo, long desplto, int origen)
{
	if ((!flujo)||(!uae4all_rom_memory))
		return 0;
	switch(origen)
	{
		case SEEK_SET:
			uae4all_rom_pos=desplto;
			break;
		case SEEK_CUR:
			uae4all_rom_pos+=desplto;
			break;
		default:
			uae4all_rom_pos=uae4all_rom_len;
	}
	if (uae4all_rom_pos<=uae4all_rom_len)
		return 0;
	uae4all_rom_pos=uae4all_rom_len;
	return -1;
}


size_t uae4all_rom_fread(void *ptr, size_t tam, size_t nmiemb, FILE *flujo)
{
	unsigned rpos=uae4all_rom_pos;
	if ((!flujo)||(!uae4all_rom_memory))
		return 0;
	if (uae4all_rom_pos>=uae4all_rom_len)
		return 0;
	memcpy(ptr,(void *)(((unsigned)uae4all_rom_memory)+((unsigned)uae4all_rom_pos)),tam*nmiemb);

	uae4all_rom_fseek(flujo,tam*nmiemb,SEEK_CUR);
	return (uae4all_rom_pos-rpos)/tam;
}

void uae4all_flush_disk(int n)
{
	if ((uae4all_disk_writed[n])&&(mainMenu_autosave))
	{
		if (maple_first_vmu())
		{
			if (uae4all_disk_writed_now[n]>6)
			{
				uae4all_disk_real_write(n);
				uae4all_disk_writed[n]=0;
				uae4all_disk_writed_now[n]=0;
			}
			else
				uae4all_disk_writed_now[n]++;
		}
	}
}

