#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "homedir.h"
#include "config.h"
#include "uae3ds.h"

#define MKDIRMOD 0644
#define PATH_SEP_CHAR '/'

char home_dir[]="/3ds/uae3DS";
char *config_dir=home_dir;

int mkpath(char* file_path, int complete) {
	char* p;

	for (p=strchr(file_path+1, PATH_SEP_CHAR); p != NULL; p=strchr(p+1, PATH_SEP_CHAR)) {
		*p='\0';
		if (mkdir(file_path, MKDIRMOD)==-1) {
			if (errno!=EEXIST) { *p=PATH_SEP_CHAR; goto mkpath_err; }
		}
		*p=PATH_SEP_CHAR;
	}
	if (complete) {
		mkdir(file_path, MKDIRMOD);
	}
	return 0;
mkpath_err:
	return 1;
}

void get_config_dir()
{
	// create the directory if doesn't exist
	mkpath(config_dir, 1);
	char *s=concat(SAVESTATE_PREFIX,"x",NULL);
	mkpath(s, 0);
	free(s);
}
