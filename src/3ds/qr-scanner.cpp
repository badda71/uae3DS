#include "sysconfig.h"
#include "menu.h"
#include "sysdeps.h"
#include "keyboard.h"
#include "uibottom.h"

#include <malloc.h>
#include <string.h>

#include <3ds.h>
#include <SDL/SDL.h>

#include "quirc.h"

typedef enum capture_cam_camera_e {
    CAMERA_OUTER,
    CAMERA_INNER
} capture_cam_camera;

typedef struct capture_cam_data_s {
    u16* buffer;
    s16 width;
    s16 height;
    capture_cam_camera camera;

    Handle mutex;

    volatile bool finished;
    Result result;
    Handle cancelEvent;
} capture_cam_data;

#define EVENT_CANCEL 0
#define EVENT_RECV 1
#define EVENT_BUFFER_ERROR 2

#define EVENT_COUNT 3

#define QR_IMAGE_WIDTH 400
#define QR_IMAGE_HEIGHT 240

typedef struct {
    struct quirc* qrContext;

    bool capturing;
    capture_cam_data captureInfo;
} qr_data;

static qr_data* installData = NULL;
static SDL_Surface *qr_screen = NULL;
static SDL_Surface *qr_im = NULL;
static SDL_Surface *qr_bw = NULL;
static SDL_Surface *qr_msg = NULL;

static void ui_error(const char *s)
{
	text_messagebox("Download Error", (char*)s, MB_OK);
}

static void capture_cam_thread(void* arg) {
    capture_cam_data* data = (capture_cam_data*) arg;

    Handle events[EVENT_COUNT] = {0};
    events[EVENT_CANCEL] = data->cancelEvent;

    Result res = 0;

    u32 bufferSize = data->width * data->height * sizeof(u16);
    u16* buffer = (u16*) calloc(1, bufferSize);
    if(buffer != NULL) {
        if(R_SUCCEEDED(res = camInit())) {
            u32 cam = data->camera == CAMERA_OUTER ? SELECT_OUT1 : SELECT_IN1;

            if(R_SUCCEEDED(res = CAMU_SetSize(cam, SIZE_CTR_TOP_LCD, CONTEXT_A))
               && R_SUCCEEDED(res = CAMU_SetOutputFormat(cam, OUTPUT_RGB_565, CONTEXT_A))
               && R_SUCCEEDED(res = CAMU_SetFrameRate(cam, FRAME_RATE_30))
               && R_SUCCEEDED(res = CAMU_SetNoiseFilter(cam, true))
               && R_SUCCEEDED(res = CAMU_SetAutoExposure(cam, true))
               && R_SUCCEEDED(res = CAMU_SetAutoWhiteBalance(cam, true))
               && R_SUCCEEDED(res = CAMU_Activate(cam))) {
                u32 transferUnit = 0;

                if(R_SUCCEEDED(res = CAMU_GetBufferErrorInterruptEvent(&events[EVENT_BUFFER_ERROR], PORT_CAM1))
                   && R_SUCCEEDED(res = CAMU_SetTrimming(PORT_CAM1, true))
                   && R_SUCCEEDED(res = CAMU_SetTrimmingParamsCenter(PORT_CAM1, data->width, data->height, 400, 240))
                   && R_SUCCEEDED(res = CAMU_GetMaxBytes(&transferUnit, data->width, data->height))
                   && R_SUCCEEDED(res = CAMU_SetTransferBytes(PORT_CAM1, transferUnit, data->width, data->height))
                   && R_SUCCEEDED(res = CAMU_ClearBuffer(PORT_CAM1))
                   && R_SUCCEEDED(res = CAMU_SetReceiving(&events[EVENT_RECV], buffer, PORT_CAM1, bufferSize, (s16) transferUnit))
                   && R_SUCCEEDED(res = CAMU_StartCapture(PORT_CAM1))) {
                    bool cancelRequested = false;
                    while(!cancelRequested && R_SUCCEEDED(res)) {
                        s32 index = 0;
                        if(R_SUCCEEDED(res = svcWaitSynchronizationN(&index, events, EVENT_COUNT, false, U64_MAX))) {
                            switch(index) {
                                case EVENT_CANCEL:
                                    cancelRequested = true;
                                    break;
                                case EVENT_RECV:
                                    svcCloseHandle(events[EVENT_RECV]);
                                    events[EVENT_RECV] = 0;

                                    svcWaitSynchronization(data->mutex, U64_MAX);
                                    memcpy(data->buffer, buffer, bufferSize);
                                    GSPGPU_FlushDataCache(data->buffer, bufferSize);
                                    svcReleaseMutex(data->mutex);

                                    res = CAMU_SetReceiving(&events[EVENT_RECV], buffer, PORT_CAM1, bufferSize, (s16) transferUnit);
                                    break;
                                case EVENT_BUFFER_ERROR:
                                    svcCloseHandle(events[EVENT_RECV]);
                                    events[EVENT_RECV] = 0;

                                    if(R_SUCCEEDED(res = CAMU_ClearBuffer(PORT_CAM1))
                                       && R_SUCCEEDED(res = CAMU_SetReceiving(&events[EVENT_RECV], buffer, PORT_CAM1, bufferSize, (s16) transferUnit))) {
                                        res = CAMU_StartCapture(PORT_CAM1);
                                    }

                                    break;
                                default:
                                    break;
                            }
                        }
                    }

                    CAMU_StopCapture(PORT_CAM1);

                    bool busy = false;
					int count = 0;
                    while(R_SUCCEEDED(CAMU_IsBusy(&busy, PORT_CAM1)) && busy && ++count < 2000) {
                        svcSleepThread(1000000);
                    }

                    CAMU_ClearBuffer(PORT_CAM1);
                }

                CAMU_Activate(SELECT_NONE);
            }

            camExit();
        }

        free(buffer);
    } else {
        res = -2; //R_APP_OUT_OF_MEMORY;
    }

    for(int i = 0; i < EVENT_COUNT; i++) {
        if(events[i] != 0) {
            svcCloseHandle(events[i]);
            events[i] = 0;
        }
    }

    svcCloseHandle(data->mutex);

    data->result = res;
    data->finished = true;
}

Result capture_cam(capture_cam_data* data) {
    if(data == NULL || data->buffer == NULL || data->width <= 0 || data->width > 640 || data->height <= 0 || data->height > 480) {
		return -1; // Invalid argument
    }

    data->mutex = 0;

    data->finished = false;
    data->result = 0;
    data->cancelEvent = 0;

    Result res = 0;

    if(R_SUCCEEDED(res = svcCreateEvent(&data->cancelEvent, RESET_STICKY)) && R_SUCCEEDED(res = svcCreateMutex(&data->mutex, false))) {
        if(threadCreate(capture_cam_thread, data, 0x10000, 0x1A, 0, true) == NULL) {
 			return -2; //R_APP_THREAD_CREATE_FAILED;
        }
    }

    if(R_FAILED(res)) {
        data->finished = true;

        if(data->cancelEvent != 0) {
            svcCloseHandle(data->cancelEvent);
            data->cancelEvent = 0;
        }

        if(data->mutex != 0) {
            svcCloseHandle(data->mutex);
            data->mutex = 0;
        }
    }

    return res;
}

static void qr_stop_capture() {
    if(!installData->captureInfo.finished) {
        svcSignalEvent(installData->captureInfo.cancelEvent);
        while(!installData->captureInfo.finished) {
            svcSleepThread(1000000);
        }
    }

    installData->capturing = false;

    if(installData->captureInfo.buffer != NULL) {
        memset(installData->captureInfo.buffer, 0, QR_IMAGE_WIDTH * QR_IMAGE_HEIGHT * sizeof(u16));
    }
}

static void qr_free_data() {
	if (qr_im) {
		SDL_FreeSurface(qr_im);
		qr_im = NULL;
	}
	if (qr_bw) {
		SDL_FreeSurface(qr_bw);
		qr_bw = NULL;
	}
	if (qr_msg) {
		SDL_FreeSurface(qr_msg);
		qr_msg = NULL;
	}

	if (installData) {
		qr_stop_capture();

		if(installData->qrContext != NULL) {
			quirc_destroy(installData->qrContext);
			installData->qrContext = NULL;
		}

		free(installData);
		installData = NULL;
	}
}

static void qr_draw_top() {
	svcWaitSynchronization(installData->captureInfo.mutex, U64_MAX);
	SDL_BlitSurface(qr_im, NULL, qr_screen, &(SDL_Rect){0,0,400,228});
	svcReleaseMutex(installData->captureInfo.mutex);	
	SDL_BlitSurface(qr_msg, NULL, qr_screen, &(SDL_Rect){0,228,400,12});
	SDL_Flip(qr_screen);
}

static int qr_update(char **payload) {
    if(!installData->capturing) {
        Result capRes = capture_cam(&installData->captureInfo);
        if(R_FAILED(capRes)) {
            ui_error("Failed to start camera capture.");
            qr_free_data();
            return -1;
        } else {
            installData->capturing = true;
        }
    }

    if(installData->captureInfo.finished) {
        if(R_FAILED(installData->captureInfo.result)) {
            ui_error("Error while capturing camera frames.");
        }
        qr_free_data();
        return -1;
    }

	qr_draw_top();

    int w = 0;
    int h = 0;
    uint8_t* qrBuf = quirc_begin(installData->qrContext, &w, &h);

    svcWaitSynchronization(installData->captureInfo.mutex, U64_MAX);

    for(int x = 0; x < w; x++) {
        for(int y = 0; y < h; y++) {
            u16 px = installData->captureInfo.buffer[y * QR_IMAGE_WIDTH + x];
            qrBuf[y * w + x] = (u8) (((((px >> 11) & 0x1F) << 3) + (((px >> 5) & 0x3F) << 2) + ((px & 0x1F) << 3)) / 3);
        }
    }

    svcReleaseMutex(installData->captureInfo.mutex);

    quirc_end(installData->qrContext);

	int num_codes = quirc_count(installData->qrContext);
	for (int i = 0; i < num_codes; i++) {
		struct quirc_code code;
		struct quirc_data data;
		quirc_decode_error_t err;

		quirc_extract(installData->qrContext, i, &code);
		err = quirc_decode(&code, &data);
		if (!err) {
			*payload=(char*)malloc(strlen((char*)data.payload)+1);
			strcpy(*payload, (char*)data.payload);
	        qr_free_data();
			return 0;
		}
	}

	return 1;
}

static int qr_init(SDL_Surface *s) {
    if (installData) {
		qr_free_data();
	}
	qr_screen = s;

	installData = (qr_data*) calloc(1, sizeof(qr_data));
    if(installData == NULL) {
        ui_error("Failed to allocate QR install data.");
        return -1;
    }

    installData->capturing = false;

    installData->captureInfo.width = QR_IMAGE_WIDTH;
    installData->captureInfo.height = QR_IMAGE_HEIGHT;

    installData->captureInfo.camera = CAMERA_OUTER;

    installData->captureInfo.finished = true;

    installData->qrContext = quirc_new();
    if(installData->qrContext == NULL) {
        ui_error("Failed to create QR context.");
        qr_free_data();
        return -1;
    }

    if(quirc_resize(installData->qrContext, QR_IMAGE_WIDTH, QR_IMAGE_HEIGHT) != 0) {
        ui_error("Failed to resize QR context.");
        qr_free_data();
        return -1;
    }

    qr_im = SDL_CreateRGBSurface(0, QR_IMAGE_WIDTH, QR_IMAGE_HEIGHT, 16, 0xF800, 0x07E0, 0x001F, 0);
    if(qr_im == NULL) {
        ui_error("Failed to create buffer image.");
        qr_free_data();
        return -1;
    }	
	installData->captureInfo.buffer = (u16*)qr_im->pixels;
	
	qr_msg=SDL_CreateRGBSurface(s->flags,s->w,12,s->format->BitsPerPixel,s->format->Rmask,s->format->Gmask,s->format->Bmask,s->format->Amask);
	write_text_full (qr_msg,"Scan QR Code (B: Cancel, X: Switch Camera)        ", 2, 2, 0, ALIGN_LEFT, FONT_NORMAL, menu_text_color, 1);

	return 0;
}

char *scan_qr_code(SDL_Surface *s)
{
	if (qr_init(s)) return NULL;
	char *url = NULL;

	SDL_Event e;
	while (1) {
		if (SDL_PollEvent(&e)) {
			if (uib_handle_event(&e)) continue;
			if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.sym == AK_ESC ||
					e.key.keysym.sym == DS_B) {
					qr_free_data();
					break;
				} else if (
					e.key.keysym.sym == AK_X ||
					e.key.keysym.sym == DS_X) {
					qr_stop_capture();
					installData->captureInfo.camera = installData->captureInfo.camera == CAMERA_OUTER ? CAMERA_INNER : CAMERA_OUTER;
				}
			}
		}
		uib_update();
		if (qr_update(&url)<1) break;
	}
	return url;
}
