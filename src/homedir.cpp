#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "homedir.h"

char *home_dir;
char *config_dir;

void get_config_dir()
{
	home_dir = getenv("HOME");

	if(home_dir != NULL)
	{
		config_dir = (char *)malloc(strlen(home_dir) + strlen("/.uae4all") + 1);
		if(config_dir != NULL)
		{
			sprintf(config_dir, "%s/.uae4all", home_dir);
			mkdir(config_dir, 0755); // create the directory if doesn't exist
		}
	}
}
