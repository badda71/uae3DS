#ifdef DREAMCAST
#include <kos.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "savedisk.h"




unsigned savedisk_get_checksum(void *mem, unsigned size)
{
	unsigned i,ret=0;
	unsigned char *p=(unsigned char *)mem;
	for(i=0;i<size;i++)
		ret+=(i+1)*(((unsigned)p[i])+1);
	return ret;
}



void savedisk_apply_changes(void *mem, void *patch, unsigned patch_size)
{
	unsigned *src=(unsigned *)patch;
	unsigned char *dst=(unsigned char *)mem;
	unsigned pos=0;
	patch_size/=sizeof(unsigned);
	while(pos<patch_size)
	{
		unsigned n=(src[pos++])*SAVEDISK_SLOT;
		memcpy((void *)&dst[n],(void *)&src[pos],SAVEDISK_SLOT);
		pos+=(SAVEDISK_SLOT/sizeof(unsigned));
	}
}


unsigned savedisk_get_changes_file(void *mem, unsigned size, void *patch, char *filename)
{
	unsigned ret=0;
	if (size%SAVEDISK_SLOT)
		size++;
	size/=SAVEDISK_SLOT;
	FILE *f=fopen(filename,"rb");
	if (f)
	{
		unsigned pos=0;
		unsigned char *src=(unsigned char *)mem;
		unsigned *dest=(unsigned *)patch;
		while(size--)
		{
			unsigned i=(ret/sizeof(unsigned));
			unsigned o=pos*SAVEDISK_SLOT;
			dest[i++]=pos;
			unsigned n=fread((void *)&dest[i],1,SAVEDISK_SLOT,f);
			if (!n)
				break;
			if (memcmp((void *)&src[o],(void *)&dest[i],n))
			{
				memcpy((void *)&dest[i],(void *)&src[o],SAVEDISK_SLOT);
				ret+=sizeof(unsigned)+SAVEDISK_SLOT;
			}
			pos++;
		}

		fclose(f);
	}
	return ret;
}

unsigned savedisk_get_changes(void *mem, unsigned size, void *patch, void *orig)
{
	unsigned ret=0;
	if (size%SAVEDISK_SLOT)
		size++;
	size/=SAVEDISK_SLOT;
	if (orig)
	{
		unsigned pos=0;
		unsigned char *src=(unsigned char *)mem;
		unsigned *dest=(unsigned *)patch;
		unsigned char *orig_p=(unsigned char *)orig;
		while(size --)
		{
			unsigned i=(ret/sizeof(unsigned));
			unsigned o=pos*SAVEDISK_SLOT;
			dest[i++]=pos;
			if (memcmp((void *)&src[o],(void *)&orig_p[o],SAVEDISK_SLOT))
			{
				memcpy((void *)&dest[i],(void *)&src[o],SAVEDISK_SLOT);
				ret+=sizeof(unsigned)+SAVEDISK_SLOT;
			}
			pos++;
		}
	}
	return ret;
}
