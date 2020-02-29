#include <curl/curl.h>

#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000
#define HTTP_ERRBUFSIZE 256

typedef enum {
	MODE_FILE,
	MODE_MEMORY,
	MODE_CALLBACK,
	MODE_HEAD
} http_dlmode;

typedef struct {
	size_t (*write_callback)(char *ptr, size_t, size_t, void *userdata);
	void *userdata;
} downloadFile_callback;

typedef struct {
	long mtime;
} http_info;

extern int downloadFile(char *url,
	void *arg,
	int (*progress_callback)(void*, curl_off_t, curl_off_t, curl_off_t, curl_off_t),
	http_dlmode mode);

extern http_info http_last_req_info;
extern char http_errbuf[HTTP_ERRBUFSIZE];
