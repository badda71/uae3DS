#define _GNU_SOURCE
#include "sysconfig.h"
#include "sysdeps.h"
#include <ctype.h>
#include <3ds.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "http.h"
#include "homedir.h" // mkpath

#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000

static u32 *SOC_buffer = NULL;
char http_errbuf[HTTP_ERRBUFSIZE];
http_info http_last_req_info = {0};

static int user_err = 0;

typedef struct {
    char        dnld_remote_fname[4096];
    char        dnld_url[4096]; 
	char		dnld_dir[4096];
    FILE        *dnld_stream;
	char		*memory;
	char		**ext;
    unsigned long dnld_file_sz;
} dnld_params_t;

static int get_oname_from_cd(char *cd, char *oname)
{
	const char *key = "filename=";
	char *val;
	char end = ';';

	// skip whitespace
	while (*cd && isspace(*cd)) ++cd;

	val = strcasestr(cd, key);
	// If filename is present
	if (val) {
		val += strlen(key);

		if (*val == '"')
			end = *(val++);

		// Copy value as oname
		int count=0;
		while (*val && *val != '\n' && *val != '\r' && *val != end && count < 4095) {
			if (*val=='\\') ++val; // escape sequence
			if (*val=='/') { // discard any path
				++val;
				count = 0;
				continue;
			}
			*(oname + count++) = *(val++);
		}
		*(oname + count) = '\0';
		return 0;
	}
	return -1;
}

static int get_oname_from_url(char *url, char *oname)
{
	char *u,*p;

	// skip whitespace
	while (*url && isspace(*url)) ++url;

	// discard http://
	p = strstr(url, "://");
	u = p ? p + 3 : url;

	// get part after last '/' url, urldecode, stop if we encounter a '?' or '#'
	p = strrchr(u, '/');
	u = p ? p + 1 : u;
	char a, b;
	int count=0;
	while (*u && *u != '\n' && *u != '\r' && *u != '?' && *u != '#' && count < 4095) {
		if ((*u == '%') &&
			((a = u[1]) && (b = u[2])) &&
			(isxdigit(a) && isxdigit(b)))
		{
			if (a >= 'a')
					a -= 'a'-'A';
			if (a >= 'A')
					a -= ('A' - 10);
			else
					a -= '0';
			if (b >= 'a')
					b -= 'a'-'A';
			if (b >= 'A')
					b -= ('A' - 10);
			else
					b -= '0';
			u+=3;
			if (16*a+b=='/') 
				count = 0;
			else
				oname[count++] = 16*a+b;
		}
		else if (*u == '+')
		{
			oname[count++] = ' ';
			u++;
		}
		else
		{
			oname[count++] = *u++;
		}
	}
	oname[count++] = 0;
	return 0;
}

static size_t dnld_header_parse(void *hdr, size_t size, size_t nmemb, void *userdata)
{
	size_t cb = size * nmemb;
	char *hdr_str = (char*)hdr;
	dnld_params_t *dnld_params = (dnld_params_t*)userdata;

	const char *cdtag = "Content-disposition:";
	const char *loctag = "Location:";

	if (!strncasecmp(hdr_str, cdtag, strlen(cdtag))) {
		get_oname_from_cd(hdr_str+strlen(cdtag), dnld_params->dnld_remote_fname);
	} else if (!strncasecmp(hdr_str, loctag, strlen(loctag))) {
		get_oname_from_url(hdr_str+strlen(loctag), dnld_params->dnld_remote_fname);
	}
	return cb;
}

static size_t write_cb(void *buffer, size_t sz, size_t nmemb, void *userdata)
{
   int ret = 0, i;
    dnld_params_t *dnld_params = (dnld_params_t*)userdata;

    if (!dnld_params->dnld_remote_fname[0]) {
        ret = get_oname_from_url(dnld_params->dnld_url, dnld_params->dnld_remote_fname);
    }

    if (!dnld_params->dnld_stream) {
	    char out[4096];
	    snprintf(out, sizeof(out), "%s/%s", dnld_params->dnld_dir, dnld_params->dnld_remote_fname);
		// check extension if given
		if (dnld_params->ext) {
			for(i=0; dnld_params->ext[i] != NULL; ++i) {
				if (strcasecmp(dnld_params->ext[i], out + strlen(out) - strlen(dnld_params->ext[i])) == 0) {
					break;
				}
			}
			if (dnld_params->ext[i] == NULL) {
				user_err = 1;
				return -1;
			}
		}

		dnld_params->dnld_stream = fopen(out, "wb");
		if (!dnld_params->dnld_stream) return -1;
    }

    ret = fwrite(buffer, 1, nmemb, dnld_params->dnld_stream);
    dnld_params->dnld_file_sz += ret;
    return ret;
}

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	dnld_params_t *mem = (dnld_params_t *)userp;

	mem->memory = (char*)realloc(mem->memory, mem->dnld_file_sz + realsize + 1);
	if(mem->memory == NULL) {
		/* out of memory! */
		snprintf(http_errbuf,HTTP_ERRBUFSIZE,"not enough memory (realloc returned NULL)");
		return 0;
	}
	memcpy(&(mem->memory[mem->dnld_file_sz]), contents, realsize);
	mem->dnld_file_sz += realsize;
	mem->memory[mem->dnld_file_sz] = 0;

	return realsize;
}

static void socShutdown() {
	socExit();
	if (SOC_buffer) {
		free(SOC_buffer);
		SOC_buffer=NULL;
	}
}

int downloadFile(char *url,
	void *arg,
	int (*progress_callback)(void*, curl_off_t, curl_off_t, curl_off_t, curl_off_t),
	http_dlmode mode,
	char **valid_extensions)
{
	static int isInit=0;
	*http_errbuf=0;
	user_err = 0;
	if (!isInit) {
		int ret;
		SOC_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
		if(SOC_buffer == NULL) {
			snprintf(http_errbuf,HTTP_ERRBUFSIZE,"memalign failed to allocate");
			return -1;
		}
		if ((ret = socInit(SOC_buffer, SOC_BUFFERSIZE)) != 0) {
			free(SOC_buffer);
			SOC_buffer=NULL;
			snprintf(http_errbuf,HTTP_ERRBUFSIZE,"socInit failed 0x%08X", (unsigned int)ret);
			return -1;
		}
		atexit(socShutdown);
		isInit=1;
	}

	// get my file with curl
	CURL *curl;
	CURLcode res;
	struct curl_slist *list = NULL;
	dnld_params_t dnld_params = {0};
	char *p;

	dnld_params.ext = valid_extensions;

	curl = curl_easy_init();
	if(!curl) {
		snprintf(http_errbuf,HTTP_ERRBUFSIZE,"curl_easy_init failed");
		return -1;
	}
	write_log("downloading file %s",url);

	strcpy(dnld_params.dnld_dir, ".");
	strncpy(dnld_params.dnld_url, url, 4095);
	switch (mode) {
		case MODE_AUTOFILE:
			strncpy(dnld_params.dnld_dir, (char*)arg, 4095);
			while (dnld_params.dnld_dir[0] && dnld_params.dnld_dir[strlen(dnld_params.dnld_dir)-1]=='/')
				dnld_params.dnld_dir[strlen(dnld_params.dnld_dir)-1]=0;
			curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, dnld_header_parse);
			curl_easy_setopt(curl, CURLOPT_HEADERDATA, &dnld_params);
			goto mode_file;
		case MODE_FILE:
			p = strrchr((char*)arg, '/');
			if (!p) strncpy(dnld_params.dnld_remote_fname, (char*)arg, 4095);
			else {
				strncpy(dnld_params.dnld_remote_fname, p+1, 4095);
				strncpy(dnld_params.dnld_dir, (char*)arg, p-(char*)arg);
				dnld_params.dnld_dir[p-(char*)arg]=0;
			}
mode_file:
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dnld_params);
			break;
		case MODE_MEMORY:
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dnld_params);
			break;
		case MODE_HEAD:
			curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
			list = curl_slist_append(list, "Cache-Control: no-cache");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
			break;
	}

	curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 128 * 1024);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
	char buf[200];
	snprintf(buf,sizeof(buf),"curl/%s",curl_version_info(CURLVERSION_NOW)->version);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, buf);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(curl, CURLOPT_STDERR, stdout);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
	curl_easy_setopt(curl, CURLOPT_FILETIME, 1L);

	curl_easy_setopt(curl, CURLOPT_NOPROGRESS,
		progress_callback != NULL ? 0L : 1L );

	curl_easy_setopt(curl, CURLOPT_PROXY, "");
//log_citra("setting proxy");curl_easy_setopt(curl, CURLOPT_PROXY, "http://127.0.0.1:3128");

	if (progress_callback != NULL)
		curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);

	/* Perform the request, res will get the return code */
	res = curl_easy_perform(curl);
	curl_slist_free_all(list);

	/* always cleanup */
	switch (mode) {
		case MODE_FILE:
		case MODE_AUTOFILE:
			strcat(dnld_params.dnld_dir, "/");
			strcat(dnld_params.dnld_dir, dnld_params.dnld_remote_fname);
			if (dnld_params.dnld_stream) {
				fclose(dnld_params.dnld_stream);
				if(res != CURLE_OK) 
					unlink(dnld_params.dnld_dir);	
			}
	}

	// set info object
	curl_easy_getinfo(curl, CURLINFO_FILETIME, &(http_last_req_info.mtime));
	curl_easy_cleanup(curl);

	if(res != CURLE_OK) {
		if (res == CURLE_ABORTED_BY_CALLBACK)
			snprintf(http_errbuf,HTTP_ERRBUFSIZE,"Aborted by user");
		else if (!user_err)
			snprintf(http_errbuf,HTTP_ERRBUFSIZE,"curl_easy_perform failed: %s", curl_easy_strerror(res));
		else 
			snprintf(http_errbuf,HTTP_ERRBUFSIZE,"File type (%s) not supported",
				strrchr(dnld_params.dnld_dir, '.')?strrchr(dnld_params.dnld_dir, '.')+1:"unknown");
		if (mode == MODE_MEMORY && dnld_params.memory) {
			free(dnld_params.memory);
			*((char**)arg) = NULL;
		}
	} else {
		if (mode == MODE_MEMORY) {
			*((char**)arg) = dnld_params.memory;
		} else if (mode == MODE_AUTOFILE) {
			strcpy((char*)arg, dnld_params.dnld_dir);
		}
	}
	return res;
}