#include "sysconfig.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "menu.h"

#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "sysdeps.h"
#include "uae.h"
#include "options.h"
#include "sound.h"
#include "uibottom.h"
#include "keyboard.h"

#ifdef HOME_DIR
#include "homedir.h"
#endif

#ifdef NAME_MAX
#define MAX_FILELEN NAME_MAX
#else
#define MAX_FILELEN 29
#endif

typedef struct{
	char d_name[MAX_FILELEN+2];
	char d_type;
}fichero;

#ifdef DREAMCAST
#define chdir(A) fs_chdir(A)
#endif

enum DiskOrder df_num;
extern char uae4all_image_file[];
extern char uae4all_image_file2[];

static const char *text_str_load_separator="----------------------------------";
static const char *text_str_load_dir="#DIR#";
static const char *text_str_load_title="            File manager           -";
fichero *text_dir_files=NULL;
int text_dir_num_files=0, text_dir_num_files_index=0;
char last_directory[PATH_MAX];

char *text_load=NULL;

#ifdef DREAMCAST
#define MAX_FILES_PER_DIR 1024
#else
#define MAX_FILES_PER_DIR 16384
#endif
#define SHOW_MAX_FILES 13

#ifdef DREAMCAST
#define chdir(A) fs_chdir(A)
static char actual_dir[128];
#endif


static int min_in_dir=0, max_in_dir=SHOW_MAX_FILES;

static int compare_names(const void *a, const void *b)
{
	if(((fichero *)a)->d_type != ((fichero *)b)->d_type)
		return (((fichero *)a)->d_type==DT_DIR ? -1 : 1);

	return strcasecmp(((fichero *)a)->d_name,((fichero *)b)->d_name);
}

static void copyCompleteName(char *dest, int n)
{
	char *src=text_dir_files[n].d_name;
	if (strlen(src)<MAX_FILELEN)
		strcpy(dest,src);
	else
	{
#ifdef DREAMCAST
		DIR *d=opendir(actual_dir);
#else
		DIR *d=opendir(".");
#endif
		if (d)
		{
			int i,indice=0,buscado=src[MAX_FILELEN+1];
			for(i=0;i<MAX_FILES_PER_DIR;i++)
			{
				struct dirent *actual=readdir(d);
				if (actual==NULL)
				{
					dest[0]=0;
					break;
				}
				if (!strncmp(src,actual->d_name,MAX_FILELEN))
				{
					if (indice==buscado)
					{
						strcpy(dest,actual->d_name);
						break;
					}
					indice++;
				}
			}
			closedir(d);
		}
		else
			dest[0]=0;
	}

}

static int checkFiles(void)
{
	char *buf=(char *)calloc(1,2046);
	int i,max=text_dir_num_files;
	int ret=(text_dir_num_files<1);
	if (max>16)
		max=16;
	for(i=0;(i<max)&&(!ret);i++)
	{
		copyCompleteName(buf,i);
		if (!buf[0])
			ret=1;
		else
		if (!text_dir_files[i].d_type)
		{
			FILE *f=fopen(buf,"rb");
			if (!f)
				ret=1;
			else
				fclose(f);
		}
		else
		if (strcmp(buf,".."))
		{
			DIR *d=opendir(buf);
			if (!d)
				ret=1;
			else
				closedir(d);
		}
	}
	free(buf);
	return ret;
}

static int getFiles(const char *dir)
{
	int i,j;
	DIR *d;
	text_dir_num_files_index=0;
	text_dir_num_files=0;
	min_in_dir=0;
	max_in_dir=SHOW_MAX_FILES;

	if (text_dir_files!=NULL)
		free(text_dir_files);
	text_dir_files=(fichero *)calloc(sizeof(fichero),MAX_FILES_PER_DIR);

	d=opendir(dir);
	if (d==NULL)
		return -1;
	for(i=0;i<MAX_FILES_PER_DIR;i++)
	{
		struct dirent *actual=readdir(d);
		if (actual==NULL)
			break;
		if ((!strcmp(actual->d_name,"."))||(!strcmp(actual->d_name,"kick.rom"))||
		    (!strcmp(actual->d_name,"ip.bin"))||(!strcmp(actual->d_name,"1st_read.bin")))
		{
			i--;
			continue;
		}
		if (strcmp(actual->d_name,"..") && actual->d_name[0] == '.')
		{
			i--;
			continue;
		}
		if (actual->d_type==DT_REG && strlen(actual->d_name)>3)
		{
			char *final=(char *)&actual->d_name[strlen(actual->d_name)-3];
			if (!((!strcasecmp(final,"adf"))||(!strcasecmp(final,"adz"))))
			{
				i--;
				continue;
			}
		}
		memset(text_dir_files[i].d_name,0,MAX_FILELEN+1);
		strncpy(text_dir_files[i].d_name,actual->d_name,MAX_FILELEN);
		if (strlen(text_dir_files[i].d_name)==MAX_FILELEN)
		{
			int jjg,indice=0;
			for(jjg=0;jjg<i;jjg++)
				if (!(strcmp(text_dir_files[i].d_name,text_dir_files[jjg].d_name)))
						indice++;
			text_dir_files[jjg].d_name[MAX_FILELEN+1]=indice;
		}
		text_dir_files[i].d_type=actual->d_type & DT_DIR;
	}
	closedir(d);

	chdir(dir);
	getcwd(last_directory, PATH_MAX);

	// add the '..' - ctrulib does not include it
	if (strcmp("/",last_directory)) {
		strcpy(text_dir_files[i].d_name,"..");
		text_dir_files[i++].d_type=DT_DIR;
	}
	text_dir_num_files=i;
	qsort(text_dir_files, text_dir_num_files, sizeof(fichero), compare_names);

	return 0;
}

static void draw_loadMenu(int c)
{
	int i,j;
	static int b=0;
	int bb=(b%6)/3;
	static int lastSelected = 0;
	static int scroll = 0;
	static int pauseScrollTimer = 15;
	int updateScroll = !(b%5);
	int padding;
	int len;
	int visibleLen;
	SDL_Rect r;
	extern SDL_Surface *text_screen;
	r.x=80-24; r.y=0; r.h=240;
	int col = 8;

	text_draw_background();
	text_draw_window(80-24,12,160+64+64,220,text_str_load_title);

	if (text_dir_num_files_index<min_in_dir)
	{
		min_in_dir=text_dir_num_files_index;
		max_in_dir=text_dir_num_files_index+SHOW_MAX_FILES;
	}
	else
		if (text_dir_num_files_index>=max_in_dir)
		{
			max_in_dir=text_dir_num_files_index+1;
			min_in_dir=max_in_dir-SHOW_MAX_FILES;
		}
	if (max_in_dir>text_dir_num_files)
		max_in_dir=text_dir_num_files-min_in_dir;


	for (i=min_in_dir,j=1;i<max_in_dir;i++,j+=2)
	{
		write_text(col,j,text_str_load_separator);

		if (text_dir_files[i].d_type==4)
		{
			r.w=110+64+64;
			visibleLen = 29;
		}
		else
		{
			r.w=110+64+64+40;
			visibleLen = 34;
		}

		len = strlen(text_dir_files[i].d_name);
		if ((text_dir_num_files_index==i) && (len > visibleLen))
		{
			if(lastSelected != text_dir_num_files_index)
			{
				lastSelected = text_dir_num_files_index;
				scroll = 0;
				pauseScrollTimer = 15;
			}

			if(!pauseScrollTimer)
			{
				if(updateScroll)
					scroll++;
				if(scroll > len - visibleLen)
					pauseScrollTimer = 60;
			}
			else
			{
				pauseScrollTimer--;

				if(pauseScrollTimer == 0)
					scroll = 0;
			}

			padding = col + 1 - scroll;
			r.x += 16;
			r.w -= 16;
		}
		else
		{
			if(lastSelected != text_dir_num_files_index)
			{
				lastSelected = text_dir_num_files_index;
				scroll = 0;
				pauseScrollTimer = 15;
			}

			padding = col + 1;
			r.x=56;
		}

		SDL_SetClipRect(text_screen,&r);
		if ((text_dir_num_files_index==i)&&(bb))
			write_text_inv(padding,j+1,(char *)&text_dir_files[i].d_name);
		else
			write_text(padding,j+1,(char *)&text_dir_files[i].d_name);

		SDL_SetClipRect(text_screen,NULL);

		if (text_dir_files[i].d_type==4)
			write_text(col+29,j+1,text_str_load_dir);
	}
	write_text(col,j,text_str_load_separator);
	text_flip();
	b++;
}

static int key_loadMenu(int *c)
{
	int end=0;
	int left=0, right=0, up=0, down=0, hit0=0, hit1=0, hit2=0;
	SDL_Event event;

	while (SDL_PollEvent(&event) > 0)
	{
		if (uib_handle_event(&event)) continue;
		if (event.type == SDL_KEYDOWN)
		{
			uae4all_play_click();
			switch(event.key.keysym.sym)
			{
				case DS_RIGHT1:
				case DS_RIGHT2:
				case DS_RIGHT3:
				case AK_RT: right = 1; break;
				case DS_LEFT1:
				case DS_LEFT2:
				case DS_LEFT3:
				case AK_LF: left = 1; break;
				case DS_UP1:
				case DS_UP2:
				case DS_UP3:
				case AK_UP: up = 1; break;
				case DS_DOWN1:
				case DS_DOWN2:
				case DS_DOWN3:
				case AK_DN: down = 1; break;
				case AK_Y:
				case DS_Y: 
				case AK_X:
				case DS_X: hit2=1; break;
				case DS_A:
				case DS_START:
				case AK_RET:
				case AK_SPC: hit0=1; break;
				case AK_ESC:
				case DS_B: hit1=1; break;
				case DS_R:
				case AK_R:
						if (text_dir_num_files)
							text_dir_num_files_index=text_dir_num_files-1;
						break;
				case AK_L:
				case DS_L:
						text_dir_num_files_index=0;
						break;

			}
			if ((hit0)||(hit2))
			{
				if ((text_dir_files[text_dir_num_files_index].d_type==4)||(!strcmp((char *)&text_dir_files[text_dir_num_files_index].d_name,"."))||(!strcmp((char *)&text_dir_files[text_dir_num_files_index].d_name,"..")))
				{
					char *tmp=(char *)calloc(1,512);
					strcpy(tmp,text_dir_files[text_dir_num_files_index].d_name);
					if (getFiles(tmp))
						end=-1;
					free(tmp);
				}
				else
				{
					if (hit0)
					{
						if(df_num == DF_0)
							copyCompleteName(uae4all_image_file,text_dir_num_files_index);
						else if(df_num == DF_1)
							copyCompleteName(uae4all_image_file2,text_dir_num_files_index);
						//menu_addFavImage(text_dir_files[text_dir_num_files_index].d_name);
					}
					end=1;
				}
			}
			else if (hit2)
			{
			}
			else if (hit1)
				end=-1;
			else if ((up)&&(text_dir_num_files_index>0))
				text_dir_num_files_index--;
			else if ((down)&&(text_dir_num_files_index+1!=text_dir_num_files))
				text_dir_num_files_index++;
			else if (left)
			{
				text_dir_num_files_index-=SHOW_MAX_FILES;
				if (text_dir_num_files_index<0)
					text_dir_num_files_index=0;
			}
			else if (right)
			{
				text_dir_num_files_index+=SHOW_MAX_FILES;
				if (text_dir_num_files_index+1>=text_dir_num_files)
					text_dir_num_files_index=text_dir_num_files-1;
			}
		}
	}

	return end;
}

static void raise_loadMenu()
{
	int i;

	text_draw_background();
	text_flip();
	for(i=0;i<10;i++)
	{
		text_draw_background();
		text_draw_window(80-24,(10-i)*24,160+64+64,220,text_str_load_title);
		text_flip();
	}
}

static void unraise_loadMenu()
{
	int i;

	for(i=9;i>=0;i--)
	{
		text_draw_background();
		text_draw_window(80-24,(10-i)*24,160+64+64,220,text_str_load_title);
		text_flip();
	}
	text_draw_background();
	text_flip();
}


int getDefaultFiles(void)
{
#ifdef DREAMCAST
	strcpy(actual_dir,MENU_DIR_DEFAULT);
#endif
	if(last_directory[0])
		return(getFiles(last_directory));
	else
		return(getFiles(MENU_DIR_DEFAULT));
}

int run_menuLoad(enum DiskOrder new_df_num)
{
	int end=0,c=0;
#ifdef DREAMCAST
	extern void reinit_sdcard(void);
	reinit_sdcard();
#endif

	df_num = new_df_num;

	if (text_dir_files==NULL)
		end=getDefaultFiles();
	else
		if (checkFiles())
			end=getDefaultFiles();

//	text_dir_num_files_index=0;

	raise_loadMenu();
	while(!end)
	{
		draw_loadMenu(c);
		SDL_Delay(10);
		end=key_loadMenu(&c);
	}
	unraise_loadMenu();

	return end;
}
