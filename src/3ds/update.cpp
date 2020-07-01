/*
 * update.c - Update support
 *
 * Written by
 *  Sebastian Weber <me@badda.de>
 *
 * This file is part of VICE3DS
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"
#include <3ds.h>
#include <malloc.h>
#include <unistd.h>
#include <curl/curl.h>

#include "update.h"
#include "menu.h"
#include "homedir.h"
#include "3ds_cia.h"
#include "uae3ds.h"
#include "http.h"
#include "uibottom.h"
#include "keyboard.h"

extern int __system_argc;
extern char** __system_argv;
__attribute__((weak)) const char* __romfs_path = NULL;

#define ERRBUFSIZE 256
#define UPDATE_INFO_URL "https://api.github.com/repos/badda71/uae3DS/releases/latest"
//#define UPDATE_INFO_URL "http://badda.de/uae3DS/latest"


static char errbuf[ERRBUFSIZE];

// *******************************************************
// exposed functions
// *******************************************************

static char *prog_title=NULL;
static char *prog_text=NULL;
static int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
	// check events
	SDL_Event e;
	if (SDL_PollEvent(&e) && 
		!uib_handle_event(&e) &&
		e.type==SDL_KEYDOWN && (
		e.key.keysym.sym == AK_ESC ||
		e.key.keysym.sym == DS_B))
	{
		return -1;
	}
	
	// draw update message box
	char *bar="##############################";
	char buf[512];
	sprintf(buf, "%s\n\n%-30s\n%s / ",
		prog_text?prog_text:"Downloading file ...",
		bar+30-((dlnow * 30) / dltotal),
		humanSize(dlnow));
	strcat(buf,humanSize(dltotal));
	text_messagebox(
		prog_title?prog_title:"Download", buf, MB_NONE);
	return 0;
}

static int progress_callback2(u32 dltotal, u32 dlnow)
{
	return progress_callback(NULL, (curl_off_t)dltotal, (curl_off_t)dlnow, 0, 0);
}

int check_update()
{
	char buf[256];

	bool ishomebrew=0;

	if (!getWifiStatus()) {
		text_messagebox("Update","Error:\nWiFi not enabled",MB_OK);
		return 0;
	}
	// check if we need an update
	char *update_info, *p;
	prog_title="Update";
	prog_text="Checking for update ...";
	if (downloadFile(UPDATE_INFO_URL, &update_info, progress_callback, MODE_MEMORY, NULL)) {
		snprintf(buf,256,"Error: Could not check update\n%s,",http_errbuf);
		write_log("%s",buf);
		text_messagebox("Update",buf,MB_OK);
		return 0;
	}

	p = strstr(update_info, "tag_name");
	if (!p) {
		text_messagebox("Update","Error: Could not get\nlatest version info\nfrom github api", MB_OK);
		free(update_info);
		return 0;
	}
	p+=12;
	char *q=strchr(p,'"'); *q=0;
	if (strcasecmp(p, VERSION3DS)==0) {
		free(update_info);
		snprintf(buf,256,"No update available\n(latest = current = %s)",VERSION3DS);
		text_messagebox("Update",buf,MB_OK);
		return 0;
	}
	
	// ask user if we should update
	snprintf(buf,256,
		"Update available!\n"
		"Installed version: %-9s\n"
		"Latest version:    %-9s\n"
		"\n"
		">> Install update? <<",VERSION3DS,p);
	if (text_messagebox("Update", buf, MB_YESNO) != 0 ) {
		free(update_info);
		return 0;
	}

	*q='"';
	// get update URL from github
	ishomebrew = envIsHomebrew();
//ishomebrew = true;
	
	char *ext = ishomebrew? "3dsx" : "cia";
	char *update_url=update_info;
	while ((update_url=strstr(update_url, "browser_download_url"))!=NULL) {
		update_url+=24;
		char *q=strchr(update_url,'"'); *q=0;
		if (strcasecmp(q-strlen(ext),ext) == 0) break;
		*q='"';
	}
	if (!update_url) {
		text_messagebox("Update","Error: Could not get download\nurl from github api", MB_OK);
		free(update_info);
		return 0;
	}

	// download update
	char update_fname[256]={0};
	
	if (ishomebrew) {
		if (__romfs_path) strcpy(update_fname, __romfs_path);
		if (__system_argc > 0 && __system_argv[0])
			strcpy(update_fname, __system_argv[0]);
		if (update_fname[0]) {
			if (strncmp(update_fname, "sdmc:/", 6) == 0)
				sprintf(update_fname, "%s", update_fname + 5);
			else if (strncmp(update_fname, "3dslink:/", 9) == 0)
				sprintf(update_fname, "/3ds%s", update_fname + 8);
		}
	}
	if (update_fname[0] == 0) {
		sprintf(update_fname,"%s%s", SAVE_PREFIX, strrchr(update_url,'/')+1);
	}
	mkpath(update_fname, 0);

	prog_title="Update";
	prog_text="Downloading Update";
	if (downloadFile(update_url, update_fname, progress_callback, MODE_FILE, NULL)) {
		snprintf(buf,256,"Error: Could not download update\n%s,",http_errbuf);
		write_log("%s",buf);
		free(update_info);
		text_messagebox("Update",buf,MB_OK);
		return 0;
	}
	free(update_info);
//log_citra("downloaded update to %s",update_fname);

	// install update if necessary
	if (ishomebrew) {
		text_messagebox("Update", "Update downloaded.\nShutting down uae3DS.", MB_OK);
	} else {
		prog_title="Update";
		prog_text="Installing Update";
		CIA_SetErrorBuffer(errbuf);
		if (CIA_InstallTitle(update_fname, progress_callback2) != 0) {
			unlink(update_fname);
			snprintf(buf,256,"Error: Could not install update\n%s,",errbuf);
			write_log("%s",buf);
			text_messagebox("Update",buf,MB_OK);
			return 0;
		} else {
			unlink(update_fname);
			text_messagebox("Update","Update downloaded and\ninstalled. Restarting\nuae3DS.", MB_OK);
			if (CIA_LaunchLastTitle() != 0) {
				snprintf(buf,256,"Error: Could not launch\nupdated title, please\nstart manually:\n%s",errbuf);
				write_log("%s",buf);
				text_messagebox("Update",buf,MB_OK);
			}
		}
	}
	return 1;
}