/* ===========================================================================
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 =========================================================================== */
/*
 * Copyright (C) 2015 Advanced Driver Information Technology Joint Venture GmbH
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  The copyright holders make
 * no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <linux/types.h>
#include <linux/ion.h>
#include <linux/msm_ion.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/un.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm.h>
#include <drm_mode.h>
#include <drm_fourcc.h>
#include <ilm/ilm_control.h>
#include <linux/version.h>
#include "qcarcam.h"
#include "gbm.h"
#include "gbm_priv.h"
#include <wayland-client.h>
#include "ivi-application-client-protocol.h"
#include "gbm-buffer-backend-client-protocol.h"
#include "linux-dmabuf-client-protocol.h"

#define QCARCAM_TEST_DEFAULT_GET_FRAME_TIMEOUT 500000000
#define DEFAULT_PRINT_DELAY_SEC 10
#define SERVER_SOCKET_PATH "/dev/shm/ais_socket_0"
#define DEFAULT_DUMP_LOCATION "/dev/shm/"
#define IVI_SURFACE_ID 9000
#define GBM_BUFFER_PARAMS_FLAGS_EARLY_DISPLAY 0x8

#ifdef _DEBUG_
#define CVBS_INFO(_fmt_, ...) \
{ \
    fprintf(stderr, _fmt_ "\n", ##__VA_ARGS__); \
}
#else
#define CVBS_INFO(_fmt_, ...) \
{ \
}
#endif

struct display_wl
{
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_shell *shell;
    uint32_t formats;

    struct ivi_application *ivi_application;
    struct gbm_buffer_backend* gbmbuf;
    struct zlinux_dmabuf *dmabuf;
};

struct gbm_buffer
{
    struct gbm_bo *bo;
    int bo_fd;
    int meta_fd;
    struct ion_fd_data fd_data;
    void *p_data;
    unsigned long length;
    struct wl_buffer *wl_buf;
    int is_busy;
};

struct window_wl
{
    struct display_wl *display;
    int width, height;
    struct wl_surface *surface;
    struct wl_shell_surface *shell_surface;
    struct wl_callback *callback;

    struct ivi_surface *ivi_surface;

};

typedef unsigned long uintptr_t;

typedef struct
{
    struct window_wl *wl_window;
    struct gbm_buffer *gbm_buf;
    struct gbm_buffer *gbm_buf_wl;

    int buffer_size[2];
    int n_buffers;
    int stride;
    int format;

    bool wl_buf_ready;
    bool gbm_buf_ready;
} cvbs_buf_t;

typedef struct
{
    pthread_t thread_id;
    int idx;

    /*qcarcam context*/
    qcarcam_hndl_t qcarcam_context;
    qcarcam_input_desc_t qcarcam_input_id;

    qcarcam_buffers_t p_buffers;
    qcarcam_buffers_t p_buffers_disp;
    qcarcam_res_t resolution;
    qcarcam_field_t field_info;

    unsigned long long int frame_timeout;
    int use_event_callback;

    cvbs_buf_t qcarcam_window;
    cvbs_buf_t display_window;

    int buf_idx_qcarcam;
    int prev_buf_idx_qcarcam;

    /*diag*/
    int frameCnt;
    int is_running;

    int n_buffers_qcarcam;
    int n_buffers_disp;

    bool is_buf_dequeued[QCARCAM_MAX_NUM_BUFFERS];
} cvbs_input_t;

enum use_protocol
{
    USE_GBM = 0,
    USE_DMA = 1,
};

enum render
{
    RENDER_NOTHING = 0,
    RENDER_DRM     = 1,
    RENDER_WESTON  = 2,
};

enum deinterlace
{
    DEINTERLACE_DISABLE = 0,
    DEINTERLACE_BOB     = 1,
    DEINTERLACE_WEAVE   = 2,
};

///////////////////////////////
/// STATICS
///////////////////////////////
static enum use_protocol protocol = USE_GBM;
static int dumpFrame = 0;
static int first_display = 1;
static bool first_start = true;
static bool gpio_preview = false;
static bool weston_op = false;
static bool dump_first_frame = false;
static bool skip_gpio = true;
static qcarcam_input_desc_t input_id = QCARCAM_INPUT_TYPE_ANALOG_MEDIA;
static enum render enable_display = RENDER_NOTHING;
static enum deinterlace enable_deinterlace = DEINTERLACE_DISABLE;
static int try_ais = 1000;
static int try_weston = 1000;
static int try_buffers = 1000;
static int gpio109 = 0;
static int gpio_interval = 100 * 1000; //100ms
static char gpiovalue[128] = "/sys/class/gpio/gpio109/value";
static struct gbm_device *gbm = NULL;
static int per_frame_byte = 0;
static int drm_fd = -1;
static int crtc_id = -1;
static int plane_id = 0;
static bool gbm_created = false;
static bool drm_ready = false;
static drmModeConnector *connector;
static drmModeRes *resources;
static drmModePlaneRes *plane_res;
static drmModeCrtc *crtc;
static struct gbm_bo *crtc_bo = NULL;
static uint32_t fbid_current;
static int ionfd;
static t_ilm_uint screenWidth;
static t_ilm_uint screenHeight;
static t_ilm_uint layer;
static int number_of_surfaces;
static uint32_t id_ivisurf;

static cvbs_input_t *test_input;
static enum wl_shm_format support_format = WL_SHM_FORMAT_XRGB8888;
static struct display_wl *wl_display;
static char filename[128] = "qcarcam_config.xml";
static pthread_t pthread_helper;

/*abort condition*/
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_handle = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_abort = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_abort = PTHREAD_COND_INITIALIZER;
static pthread_cond_t  waiterVariable = PTHREAD_COND_INITIALIZER;
static int abort_disp = 0;
static int abort_ais = 0;
static int quit = 0;

static const int exceptsigs[] = {
    SIGCHLD, SIGIO, SIGURG, SIGWINCH,
    SIGSTOP, SIGTSTP, SIGTTIN, SIGTTOU, SIGCONT,
    -1,
};

static void weston_close_display(cvbs_input_t *input_ctxt);
static void place_marker(const char *name)
{
#ifdef ANDROID_U_AND_ABOVE
    int fd=open("/debug/bootkpi/kpi_values", O_WRONLY);
    if (fd > 0)
    {
        write(fd, name, strlen(name));
        close(fd);
        CVBS_INFO("marker: %s", name);
    }
    else
    {
        CVBS_INFO("marker open failed");
    }
#else
   /* debug info, hence boot_kpi string is not added.
    */
   ALOGE("%s", name);
#endif
}

static void dmabuf_format(void *data, struct zlinux_dmabuf *zlinux_dmabuf, uint32_t format)
{
    struct display_wl *d = (struct display_wl *)data;
}

static const struct zlinux_dmabuf_listener dmabuf_listener = {
    dmabuf_format
};

static void buffer_release(void *data, struct wl_buffer *buffer)
{
    struct gbm_buffer *buf = (struct gbm_buffer *)data;
    buf->is_busy = 0;
}

static const struct wl_buffer_listener buffer_listener = {
    buffer_release};

static void gbm_create_succeeded(void *data, struct gbm_buffer_params *params, struct wl_buffer *new_buffer)
{
    struct gbm_buffer *buffer = (struct gbm_buffer *)data;

    buffer->wl_buf = new_buffer;
    wl_buffer_add_listener(buffer->wl_buf, &buffer_listener, buffer);
    buffer->is_busy = 0;

    gbm_buffer_params_destroy(params);
}

static void gbm_create_failed(void *data, struct gbm_buffer_params *params)
{
    struct gbm_buffer *buffer = (struct gbm_buffer *)data;

    buffer->wl_buf = NULL;
    gbm_buffer_params_destroy(params);

    CVBS_INFO("Error: gbm_buffer_params_create.create failed.");
}

static const struct gbm_buffer_params_listener gbm_params_listener = {
    gbm_create_succeeded,
    gbm_create_failed
};

static void zlinux_create_succeeded(void *data, struct zlinux_buffer_params *params, struct wl_buffer *new_buffer)
{
    struct gbm_buffer *buffer = (struct gbm_buffer *)data;

    buffer->wl_buf = new_buffer;
    wl_buffer_add_listener(buffer->wl_buf, &buffer_listener, buffer);
    buffer->is_busy = 0;

    zlinux_buffer_params_destroy(params);
}

static void zlinux_create_failed(void *data, struct zlinux_buffer_params *params)
{
    struct gbm_buffer *buffer = (struct gbm_buffer *)data;

    buffer->wl_buf = NULL;
    zlinux_buffer_params_destroy(params);

    CVBS_INFO("Error: zlinux_buffer_params_create.create failed.");
}

static const struct zlinux_buffer_params_listener zlinux_params_listener = {
    zlinux_create_succeeded,
    zlinux_create_failed
};

static void wl_registry_handle(void *display_data, struct wl_registry *reg, uint32_t id, const char *intf, uint32_t ver)
{
    struct display_wl *d = (display_wl*)display_data;

    if (strcmp(intf, "wl_compositor") == 0)
    {
        d->compositor = (wl_compositor*)wl_registry_bind(reg, id, &wl_compositor_interface, 1);
    }
    else if (strcmp(intf, "wl_shell") == 0)
    {
        d->shell = (wl_shell*)wl_registry_bind(reg, id, &wl_shell_interface, 1);
    }
    else if (strcmp(intf, "ivi_application") == 0)
    {
        d->ivi_application = (ivi_application*)wl_registry_bind(reg, id, &ivi_application_interface, 1);
    }
    else if (strcmp(intf, "gbm_buffer_backend") == 0)
    {
        d->gbmbuf = (gbm_buffer_backend*)wl_registry_bind(reg, id, &gbm_buffer_backend_interface, 1);
    }
    else if (strcmp(intf, "zlinux_dmabuf") == 0)
    {
        d->dmabuf = (struct zlinux_dmabuf*)wl_registry_bind(reg, id, &zlinux_dmabuf_interface, 1);
        zlinux_dmabuf_add_listener(d->dmabuf, &dmabuf_listener, d);
    }
}

static void wl_registry_handle_remove(void *display_data, struct wl_registry *reg, uint32_t n)
{
}

static const struct wl_registry_listener wl_registry_lsn = {
    wl_registry_handle,
    wl_registry_handle_remove};

static void handle_ivi_surface_configure(void *data, struct ivi_surface *ivi_surface, int32_t width, int32_t height)
{
}

static void handle_ping(void *data, struct wl_shell_surface *shell_surface, uint32_t serial)
{
    wl_shell_surface_pong(shell_surface, serial);
}

static void handle_configure(void *data, struct wl_shell_surface *shell_surface, uint32_t edges, int32_t width, int32_t height)
{
}

static void handle_popup_done(void *data, struct wl_shell_surface *shell_surface)
{
}

static const struct wl_shell_surface_listener shell_surface_listener = {
    handle_ping,
    handle_configure,
    handle_popup_done};

static const struct ivi_surface_listener ivi_surface_listener = {
    handle_ivi_surface_configure,
};

static struct display_wl *create_wl_display(void)
{
    struct display_wl *display;

    display = (display_wl*)calloc(1, sizeof(*display));
    if (display == NULL)
    {
        CVBS_INFO("out of memory");
        return NULL;
    }

    display->display = wl_display_connect(NULL);
    while (!display->display && try_weston)
    {
        CVBS_INFO("Wait 5ms and retry display_connect; %d attempts left", try_weston);
        usleep(5*1000);
        display->display = wl_display_connect(NULL);
        try_weston--;
        if(try_weston == 0)
        {
            CVBS_INFO("wl_display_connect failed.");
            return NULL;
        }
    }

    display->formats = 0;
    display->registry = wl_display_get_registry(display->display);
    wl_registry_add_listener(display->registry, &wl_registry_lsn, display);
    wl_display_dispatch(display->display);
    wl_display_roundtrip(display->display);

    wl_display_get_fd(display->display);

    return display;
}

static void abort_qcarcam(void)
{
    pthread_mutex_lock(&mutex_abort);
    abort_ais = 1;
    pthread_cond_broadcast(&cond_abort);
    pthread_mutex_unlock(&mutex_abort);
}

static void abort_display(void)
{
    __sync_fetch_and_add(&abort_disp, 1);
}

static void *signal_thread(void *arg)
{
    sigset_t sigset;
    int sig;
    int i;

    pthread_setname_np(pthread_self(), "signal_thread");
    pthread_detach(pthread_self());
    sigfillset(&sigset);
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    for (i = 0; exceptsigs[i] != -1; i++)
    {
        sigdelset(&sigset, exceptsigs[i]);
    }

    for (;;)
    {
        if (sigwait(&sigset, &sig) == 0)
        {
            CVBS_INFO("Receive signal %d then quit", sig);
            quit = 1;
            abort_qcarcam();
            abort_display();
            break;
        }
    }
    return NULL;
}

static void *helper_thread(void *arg)
{
    int ret;
    pthread_setname_np(pthread_self(), "helper_thread");
    pthread_detach(pthread_self());

    while (wl_display)
    {
        __sync_fetch_and_add(&abort_disp, 0);
        if (abort_disp)
        {
            CVBS_INFO("exit helper_thread");
            pthread_exit(NULL);
        }

        ret = wl_display_dispatch(wl_display->display);
        if (ret == -1)
        {
            CVBS_INFO("wl_display_dispatch failed");
            quit = 1;
            abort_qcarcam();
            abort_display();
            pthread_exit(NULL);
        }
    }

    pthread_exit(NULL);
}

static unsigned int cvbs_get_stride(qcarcam_color_fmt_t fmt, unsigned int width, unsigned int height)
{
    unsigned int stride = 0;

    switch (fmt)
    {
    case QCARCAM_FMT_RGB_888:
        stride = width * 3;
        break;
    case QCARCAM_FMT_MIPIRAW_8:
        stride = width;
        break;
    case QCARCAM_FMT_MIPIRAW_10:
        if (0 == (width % 4))
            stride = width * 5 / 4;
        break;
    case QCARCAM_FMT_MIPIRAW_12:
        if (0 == (width % 2))
            stride = width * 3 / 2;
        break;
    case QCARCAM_FMT_UYVY_8:
        return width * 2;
    case QCARCAM_FMT_UYVY_10:
        if (0 == (width % 4))
            stride = width * 2 * 5 / 4;
        break;
    case QCARCAM_FMT_UYVY_12:
        if (0 == (width % 2))
            stride = width * 2 * 3 / 2;
        break;
    default:
        break;
    }

    return stride;
}

static qcarcam_ret_t cvbs_deinterlace(cvbs_input_t *input_ctxt)
{
    unsigned char *buf_odd_ptr = NULL;
    unsigned char *buf_even_ptr = NULL;
    unsigned char *buf_ptr = (unsigned char*)input_ctxt->display_window.gbm_buf[input_ctxt->buf_idx_qcarcam].p_data;
    uint32_t line = input_ctxt->qcarcam_window.stride;
    static qcarcam_field_t field_info_prev;

    if (__sync_fetch_and_sub(&first_display, 0) == 1)
    {
        CVBS_INFO("Frame %d, field info %d", input_ctxt->frameCnt, input_ctxt->field_info);
    }
    else
    {
        if (input_ctxt->field_info != field_info_prev)
            CVBS_INFO("Frame %d, field info changed from %d to %d", input_ctxt->frameCnt, field_info_prev, input_ctxt->field_info);
    }

    if ((input_ctxt->field_info == QCARCAM_FIELD_UNKNOWN && field_info_prev == QCARCAM_FIELD_EVEN_ODD) || (input_ctxt->field_info == QCARCAM_FIELD_EVEN_ODD))
    {
        buf_even_ptr = (unsigned char*)input_ctxt->qcarcam_window.gbm_buf[input_ctxt->buf_idx_qcarcam].p_data + line * 14;
        buf_odd_ptr = (unsigned char*)input_ctxt->qcarcam_window.gbm_buf[input_ctxt->buf_idx_qcarcam].p_data + line * 267;
    }
    else
    {
        buf_odd_ptr = (unsigned char*)input_ctxt->qcarcam_window.gbm_buf[input_ctxt->buf_idx_qcarcam].p_data + line * 13;
        buf_even_ptr = (unsigned char*)input_ctxt->qcarcam_window.gbm_buf[input_ctxt->buf_idx_qcarcam].p_data + line * 267;
    }

    if(enable_deinterlace == DEINTERLACE_WEAVE)
    {
        for(int i = 0; i < 240; i++)
        {
            memcpy(buf_ptr, buf_odd_ptr, line);
            buf_ptr += line;
            buf_odd_ptr += line;
            memcpy(buf_ptr, buf_even_ptr, line);
            buf_ptr += line;
            buf_even_ptr += line;
        }
    }
    else if(enable_deinterlace == DEINTERLACE_BOB)
    {
        for(int i = 0; i < 240; i++)
        {
            memcpy(buf_ptr, buf_odd_ptr, line);
            buf_ptr += line;
            memcpy(buf_ptr, buf_odd_ptr, line);
            buf_ptr += line;
            buf_odd_ptr += line;
        }
    }

    if (__sync_fetch_and_sub(&first_display, 0) == 1)
        place_marker("# CVBS leave deinterlace");

    if(input_ctxt->field_info != QCARCAM_FIELD_UNKNOWN)
        field_info_prev = input_ctxt->field_info;

    return QCARCAM_RET_OK;
}

static qcarcam_ret_t cvbs_dump_buffer(cvbs_buf_t *user_ctxt, unsigned int idx, const char *filename)
{
    FILE *fp;
    size_t numBytesWritten = 0;
    size_t numByteToWrite = 0;
    unsigned char *buf_ptr = NULL;

    fp = fopen(filename, "w+b");
    CVBS_INFO("dumping qcarcam frame %s", filename);

    if (0 != fp)
    {
        if (user_ctxt->buffer_size[0] == user_ctxt->stride)
        {
            numByteToWrite = user_ctxt->stride * user_ctxt->buffer_size[1];
            buf_ptr = (unsigned char*)user_ctxt->gbm_buf[idx].p_data;
            numBytesWritten = fwrite((void *)buf_ptr, numByteToWrite, 1, fp);
        }
        else
        {
            buf_ptr = (unsigned char*)user_ctxt->gbm_buf[idx].p_data;
            for(int i = 0; i < user_ctxt->buffer_size[1]; i++)
            {
                numBytesWritten += fwrite((void *)buf_ptr, user_ctxt->buffer_size[0]*2, 1, fp);
                buf_ptr += user_ctxt->stride;
            }
        }

        fclose(fp);
    }
    else
    {
        CVBS_INFO("failed to open file");
        return QCARCAM_RET_FAILED;
    }

    return QCARCAM_RET_OK;
}

/**
 * Function to retrieve frame from qcarcam and increase frame_counter
 * @param input_ctxt
 * @return 0 on success, -1 on failure
 */
static int cvbs_get_frame(cvbs_input_t *input_ctxt)
{
    qcarcam_ret_t ret;
    qcarcam_frame_info_t frame_info;

    ret = qcarcam_get_frame(input_ctxt->qcarcam_context, &frame_info, input_ctxt->frame_timeout, 0);

    if (QCARCAM_RET_OK != ret)
    {
        CVBS_INFO("Get frame context 0x%p ret %d\n", input_ctxt->qcarcam_context, ret);
        return -1;
    }

    if (frame_info.idx >= input_ctxt->n_buffers_qcarcam)
    {
        CVBS_INFO("Get frame context 0x%p ret invalid idx %d\n", input_ctxt->qcarcam_context, frame_info.idx);
        return -1;
    }

    if (input_ctxt->frameCnt == 0)
    {
        place_marker("# CVBS First Frame");
        CVBS_INFO("Deinterlace: %d", enable_deinterlace);

        if (dump_first_frame)
        {
            snprintf(filename, sizeof(filename), DEFAULT_DUMP_LOCATION "frame_%d_%i", input_ctxt->idx, input_ctxt->frameCnt);
            cvbs_dump_buffer(&input_ctxt->qcarcam_window, input_ctxt->buf_idx_qcarcam, filename);
        }
    }

    input_ctxt->buf_idx_qcarcam = frame_info.idx;
    input_ctxt->field_info = frame_info.field_type;

    input_ctxt->is_buf_dequeued[input_ctxt->buf_idx_qcarcam] = 1;

    return 0;
}

/**
 * Release frame back to qcarcam
 * @param input_ctxt
 * @return 0 on success, -1 on failure
 */
static int cvbs_release_frame(cvbs_input_t *input_ctxt)
{
    qcarcam_ret_t ret;

    if (input_ctxt->prev_buf_idx_qcarcam >= 0 && input_ctxt->prev_buf_idx_qcarcam < input_ctxt->n_buffers_qcarcam)
    {
        if (input_ctxt->is_buf_dequeued[input_ctxt->prev_buf_idx_qcarcam])
        {
            ret = qcarcam_release_frame(input_ctxt->qcarcam_context, input_ctxt->prev_buf_idx_qcarcam);
            if (QCARCAM_RET_OK != ret)
            {
                CVBS_INFO("qcarcam_release_frame() %d failed", input_ctxt->prev_buf_idx_qcarcam);
                return -1;
            }
            input_ctxt->is_buf_dequeued[input_ctxt->prev_buf_idx_qcarcam] = 0;
        }
        else
        {
            CVBS_INFO("qcarcam_release_frame() skipped since buffer %d not dequeued", input_ctxt->prev_buf_idx_qcarcam);
        }
    }
    else
    {
        CVBS_INFO("qcarcam_release_frame() skipped");
    }

    input_ctxt->prev_buf_idx_qcarcam = input_ctxt->buf_idx_qcarcam;

    return 0;
}

qcarcam_ret_t cvbs_post_buffer(cvbs_buf_t *user_ctxt, unsigned int idx, int *rel_idx)
{
    if (gpio_preview && !weston_op)
    {
        user_ctxt->gbm_buf_wl[idx].is_busy = 1;
        wl_surface_damage(user_ctxt->wl_window->surface, 0, 0, user_ctxt->wl_window->width, user_ctxt->wl_window->height);
        wl_surface_attach(user_ctxt->wl_window->surface, user_ctxt->gbm_buf_wl[idx].wl_buf, 0, 0);
        wl_surface_commit(user_ctxt->wl_window->surface);
        user_ctxt->wl_window->callback = wl_display_sync(wl_display->display);
        wl_callback_destroy(user_ctxt->wl_window->callback);
        wl_display_flush(wl_display->display);
    }

    return QCARCAM_RET_OK;
}

static int cvbs_post_to_display(cvbs_input_t *input_ctxt)
{
    if (__sync_fetch_and_sub(&first_display, 0) == 1)
    {
        __sync_fetch_and_sub(&first_display, 1);
        place_marker("# CVBS First Frame to display");
    }

    qcarcam_ret_t ret;
    if (0 != dumpFrame)
    {
        if (0 == input_ctxt->frameCnt % dumpFrame)
        {
            snprintf(filename, sizeof(filename), DEFAULT_DUMP_LOCATION "frame_%i_720x507_UYVY.yuv", input_ctxt->frameCnt);
            cvbs_dump_buffer(&input_ctxt->qcarcam_window, input_ctxt->buf_idx_qcarcam, filename);
            if(enable_deinterlace)
            {
                snprintf(filename, sizeof(filename), DEFAULT_DUMP_LOCATION "frame_di_%i_720x480_UYVY.yuv", input_ctxt->frameCnt);
                cvbs_dump_buffer(&input_ctxt->display_window, input_ctxt->buf_idx_qcarcam, filename);
            }
        }
    }

    if (enable_display == RENDER_WESTON)
    {
        if(enable_deinterlace)
            ret = cvbs_post_buffer(&input_ctxt->display_window, input_ctxt->buf_idx_qcarcam, &input_ctxt->prev_buf_idx_qcarcam);
        else
            ret = cvbs_post_buffer(&input_ctxt->qcarcam_window, input_ctxt->buf_idx_qcarcam, &input_ctxt->prev_buf_idx_qcarcam);

        if (ret != QCARCAM_RET_OK)
        {
            CVBS_INFO("cvbs_post_window_buffer failed");
        }
    }

    return 0;
}

static qcarcam_ret_t cvbs_drm_render_frame(cvbs_input_t *input_ctxt)
{
    union gbm_bo_handle bo_handle;
    struct gbm_import_fd_data buf_data;
    struct gbm_bo *bo = NULL;
    uint32_t handles[4] = {0};
    uint32_t pitches[4] = {0};
    uint32_t offsets[4] = {0};
    uint32_t flags = 0;
    uint32_t fbid;
    struct drm_mode_fb_cmd2 args;
    unsigned int dst_w, dst_h;
    unsigned int width, height;

    /* create bo */
    if (enable_deinterlace)
    {
        buf_data.fd = input_ctxt->display_window.gbm_buf[input_ctxt->buf_idx_qcarcam].bo_fd;
        buf_data.width = input_ctxt->display_window.buffer_size[0];
        buf_data.height = input_ctxt->display_window.buffer_size[1];
    }
    else
    {
        buf_data.fd = input_ctxt->qcarcam_window.gbm_buf[input_ctxt->buf_idx_qcarcam].bo_fd;
        buf_data.width = input_ctxt->qcarcam_window.buffer_size[0];
        buf_data.height = input_ctxt->qcarcam_window.buffer_size[1];
    }

    buf_data.format = GBM_FORMAT_UYVY;
    bo = gbm_bo_import(gbm, GBM_BO_IMPORT_FD, &buf_data, GBM_BO_USE_RENDERING);

    /*get the fb of display from bo */
    width = gbm_bo_get_width(bo);
    height = gbm_bo_get_height(bo);
    bo_handle = gbm_bo_get_handle(bo);
    handles[0] = bo_handle.u32;
    pitches[0] = gbm_bo_get_stride(bo);
    offsets[0] = 0;

    if (drmModeAddFB2(drm_fd, width, height, DRM_FORMAT_UYVY, handles, pitches, offsets, &fbid, 0))
    {
        CVBS_INFO("failed to add fb: %s", strerror(errno));
        goto failed_with_bo;
    }

    dst_w = connector->modes[0].hdisplay;
    dst_h = connector->modes[0].vdisplay;

    if (drmModeSetPlane(drm_fd, plane_id, crtc_id, fbid, 0, 0, 0, dst_w, dst_h, 0, 0, width << 16, height << 16))
    {
        CVBS_INFO("drm_fd=%d, crtc_id=%d, fbid=%d, failed to set plane.errno=%s", drm_fd, crtc_id, fbid, strerror(errno));
        goto failed_with_bo;
    }

    if (fbid_current)
        drmModeRmFB(drm_fd, fbid_current);
    fbid_current = fbid;

    if (__sync_fetch_and_sub(&first_display, 0) == 1)
    {
        __sync_fetch_and_sub(&first_display, 1);
        place_marker("# CVBS First Frame on Screen");
    }

    if(bo)
        gbm_bo_destroy(bo);
    return QCARCAM_RET_OK;

failed_with_bo:
    if(bo)
        gbm_bo_destroy(bo);
    return QCARCAM_RET_FAILED;
}

/**
 * Function to handle routine of fetching, displaying, and releasing frames when one is available
 * @param input_ctxt
 * @return 0 on success, -1 on failure
 */
static int cvbs_handle_new_frame(cvbs_input_t *input_ctxt)
{
    pthread_mutex_lock(&mutex_handle);
    qcarcam_ret_t ret;

    if (cvbs_get_frame(input_ctxt))
    {
        pthread_mutex_unlock(&mutex_handle);
        return 0;
    }

    if (enable_display == RENDER_WESTON && ((enable_deinterlace && input_ctxt->display_window.wl_buf_ready) || (!enable_deinterlace && input_ctxt->qcarcam_window.wl_buf_ready)) && gpio_preview && !weston_op)
    {
        if(enable_deinterlace)
        {
            if(__sync_fetch_and_sub(&first_display, 0) && input_ctxt->field_info == QCARCAM_FIELD_UNKNOWN)
            {
                CVBS_INFO("Unknown field info & first display frame. SKIP DISPLAY");
                input_ctxt->field_info == QCARCAM_FIELD_EVEN_ODD;
            }

            if (cvbs_deinterlace(input_ctxt))
            {
                pthread_mutex_unlock(&mutex_handle);
                return 0;
            }
        }

        cvbs_post_to_display(input_ctxt);
    }
    else if (enable_display == RENDER_DRM && drm_ready &&  ((enable_deinterlace && input_ctxt->display_window.gbm_buf_ready) || (!enable_deinterlace && input_ctxt->qcarcam_window.gbm_buf_ready)) && gpio_preview && !weston_op)
    {
        if(enable_deinterlace)
        {
            if(__sync_fetch_and_sub(&first_display, 0) && input_ctxt->field_info == QCARCAM_FIELD_UNKNOWN)
            {
                CVBS_INFO("Unknown field info & first display frame. SKIP DISPLAY");
                input_ctxt->field_info == QCARCAM_FIELD_EVEN_ODD;
            }

            if (cvbs_deinterlace(input_ctxt))
            {
                pthread_mutex_unlock(&mutex_handle);
                return 0;
            }
        }

        cvbs_drm_render_frame(input_ctxt);
    }
    else
        CVBS_INFO("SKIP DISPLAY. enable_display: %d, enable_deinterlace: %d, wl_buf_ready: %s, drm_ready: %s, gpio_preview: %s, abort_disp: %d", enable_display, enable_deinterlace, input_ctxt->display_window.wl_buf_ready?"true":"false", drm_ready?"true":"false", gpio_preview?"true":"false", abort_disp);

    input_ctxt->frameCnt++;

    if (cvbs_release_frame(input_ctxt))
    {
        pthread_mutex_unlock(&mutex_handle);
        return -1;
    }

    pthread_mutex_unlock(&mutex_handle);
    return 0;
}

/**
 * Qcarcam event callback function
 * @param hndl
 * @param event_id
 * @param p_payload
 */
static void cvbs_event_cb(qcarcam_hndl_t hndl, qcarcam_event_t event_id, qcarcam_event_payload_t *p_payload)
{
    if (!test_input || ! test_input->qcarcam_context)
    {
        CVBS_INFO("event_cb called with invalid qcarcam handle");
        return;
    }

    switch (event_id)
    {
    case QCARCAM_EVENT_FRAME_READY:
        cvbs_handle_new_frame(test_input);
        break;
    case QCARCAM_EVENT_ERROR:
        CVBS_INFO("%d received QCARCAM_EVENT_ERROR", test_input->idx);
        if (0)
        {
            qcarcam_stop(test_input->qcarcam_context);
            qcarcam_start(test_input->qcarcam_context);
        }
        else
        {
            CVBS_INFO("%d ABORT TEST!!!", test_input->idx);
            abort_qcarcam();
        }
        break;
    default:
        CVBS_INFO("%d received unsupported event %d", test_input->idx, event_id);
        break;
    }
}

static qcarcam_ret_t cvbs_init_gbm_buffers(cvbs_buf_t *user_ctxt, qcarcam_buffers_t *buffers)
{
    struct ion_allocation_data ion_alloc;
    struct ion_fd_data ion_info_fd;
    struct gbm_buffer *gbm_buf;
    unsigned long length;
    int rc = -1;

    if (test_input->qcarcam_input_id == QCARCAM_INPUT_TYPE_ANALOG_MEDIA)
        user_ctxt->stride = 736 * 2;
    else
        user_ctxt->stride = 1280 * 2;

    user_ctxt->format = buffers->color_fmt;
    user_ctxt->n_buffers = buffers->n_buffers;

    gbm_buf = (gbm_buffer*)calloc(buffers->n_buffers, sizeof(*gbm_buf));
    if (!gbm_buf)
    {
        CVBS_INFO("Failed to allocate ion buffers structure");
        return QCARCAM_RET_FAILED;
    }

    for (int i = 0; i < buffers->n_buffers; i++)
    {
        buffers->buffers[i].n_planes = 1;
        if (test_input->qcarcam_input_id == QCARCAM_INPUT_TYPE_ANALOG_MEDIA)
            buffers->buffers[i].planes[0].stride = 736 * 2;
        else
            buffers->buffers[i].planes[0].stride = 1280 * 2;
        buffers->buffers[i].planes[0].size = buffers->buffers[i].planes[0].height * buffers->buffers[i].planes[0].stride;

        // pad length for alignment
        length = buffers->buffers[i].planes[0].size;
        length = (length + 4096 - 1) & ~(4096 - 1);

        if (protocol == USE_GBM)
        {
            gbm_buf[i].bo = gbm_bo_create(gbm, buffers->buffers[i].planes[0].width, buffers->buffers[i].planes[0].height, GBM_FORMAT_UYVY, GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
            if (gbm_buf[i].bo == NULL)
            {
                CVBS_INFO("Failed to create bo");
                return QCARCAM_RET_FAILED;
            }

            gbm_buf[i].bo_fd = gbm_bo_get_fd(gbm_buf[i].bo);
            if (gbm_buf[i].bo_fd < 0)
            {
                CVBS_INFO("Failed to get bo fd");
                return QCARCAM_RET_FAILED;
            }

            gbm_perform(GBM_PERFORM_GET_METADATA_ION_FD, gbm_buf[i].bo, &gbm_buf[i].meta_fd);
            if (gbm_buf[i].meta_fd < 0)
            {
                CVBS_INFO("Failed to get meta bo fd");
                return QCARCAM_RET_FAILED;
            }

            buffers->buffers[i].planes[0].p_buf = (void *)(uintptr_t)gbm_buf[i].bo_fd;
            gbm_buf[i].p_data = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, gbm_buf[i].bo_fd, 0);
        }

        if (protocol == USE_DMA)
        {
            memset(&ion_alloc, 0, sizeof(ion_alloc));
            ion_alloc.len = length;
            ion_alloc.align = 4096;
            ion_alloc.heap_id_mask = 0x1 << ION_IOMMU_HEAP_ID;
            ion_alloc.handle = 0;

            rc = ioctl(ionfd, ION_IOC_ALLOC, &ion_alloc);
            if (rc < 0)
            {
                CVBS_INFO("Failed to allocate buffer (len=%lu), %s", length, strerror(errno));
                return QCARCAM_RET_FAILED;
            }
            else
            {
                ion_info_fd.handle = ion_alloc.handle;
                rc = ioctl(ionfd, ION_IOC_SHARE, &ion_info_fd);
                if (rc < 0)
                    CVBS_INFO("ION share buffer failed, %d", rc);

            }

            buffers->buffers[i].planes[0].p_buf = (void *)(uintptr_t)ion_info_fd.fd;
            gbm_buf[i].fd_data = ion_info_fd;
            gbm_buf[i].p_data = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, ion_info_fd.fd, 0);
        }

        gbm_buf[i].length = length;
    }

    user_ctxt->gbm_buf = gbm_buf;
    user_ctxt->gbm_buf_ready = true;

    return QCARCAM_RET_OK;
}

/**
 * Thread to setup and run qcarcam based on test input context
 *
 * @note For single threaded operation, this function only sets up qcarcam context.
 *      qcarcam_start and handling of frames is not executed.
 *
 * @param arg cvbs_input_t* input_ctxt
 */
static void *cvbs_setup_input_ctxt_thread(void *arg)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    const char *env_dir;
    char socket_dir[UNIX_PATH_MAX];
    cvbs_input_t *input_ctxt = (cvbs_input_t *)arg;
    int try_drm = 100;

    if (!input_ctxt)
        return (void *)-1;

    pthread_setname_np(pthread_self(), "setup_input_ctxt_thread");
    pthread_detach(pthread_self());

    if(protocol == USE_DMA)
    {
        ionfd = open("/dev/ion", O_RDONLY);
        if (ionfd < 0)
        {
            CVBS_INFO("Failed to open ION handle, %d", ionfd);
            return (void *)-1;
        }
    }

    if (protocol == USE_GBM)
    {
        if (enable_display == RENDER_DRM)
        {
            if (drm_fd < 0)
                drm_fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
            while (drm_fd < 0 && try_drm)
            {
                CVBS_INFO("Open drm_fd failed wait 3ms, %d attempts left", try_drm);
                usleep(3000);
                try_drm--;
                if (try_drm == 0)
                {
                    CVBS_INFO("/dev/dri/card0 open failed: %d", drm_fd);
                    return (void *)-1;
                }
                drm_fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
            }
            CVBS_INFO("/dev/dri/card0 open: %d", drm_fd);
        }

        if (enable_display == RENDER_WESTON)
        {
            if (drm_fd < 0)
                drm_fd = open("/dev/dri/renderD128", O_RDWR | O_CLOEXEC);
            while (drm_fd < 0 && try_drm)
            {
                CVBS_INFO("Open drm_fd failed wait 3ms, %d attempts left", try_drm);
                usleep(3000);
                try_drm--;
                if (try_drm == 0)
                {
                    CVBS_INFO("/dev/dri/renderD128 open failed: %d", drm_fd);
                    return (void *)-1;
                }
                drm_fd = open("/dev/dri/renderD128", O_RDWR | O_CLOEXEC);
            }
            CVBS_INFO("/dev/dri/renderD128 open: %d", drm_fd);
        }

        if (gbm == NULL)
            gbm = gbm_create_device(drm_fd);
        if (gbm == NULL)
        {
            CVBS_INFO("gbm_create_device failed");
            return (void *)-1;
        }
        CVBS_INFO("GBM device created: %p", gbm);
        gbm_created = true;
    }

    ret = cvbs_init_gbm_buffers(&input_ctxt->qcarcam_window, &input_ctxt->p_buffers);
    if(ret != QCARCAM_RET_OK)
        return (void *)-1;

    if(enable_deinterlace)
    {
        ret = cvbs_init_gbm_buffers(&input_ctxt->display_window, &input_ctxt->p_buffers_disp);
        if(ret != QCARCAM_RET_OK)
            return (void *)-1;
    }

    while (0 != access(SERVER_SOCKET_PATH, F_OK) && try_ais)
    {
        CVBS_INFO("Wait 3ms and retry ais; %d attempts left", try_ais);
        usleep(3*1000);
        try_ais--;
        if(try_ais == 0)
            return (void *)-1;
    }

    input_ctxt->qcarcam_context = qcarcam_open(input_ctxt->qcarcam_input_id);
    if (input_ctxt->qcarcam_context == 0)
    {
        CVBS_INFO("open() failed, exit");
        goto qcarcam_thread_fail;
    }

    ret = qcarcam_s_buffers(input_ctxt->qcarcam_context, &input_ctxt->p_buffers);
    if (ret != QCARCAM_RET_OK)
    {
        CVBS_INFO("qcarcam_s_buffers() failed");
        goto qcarcam_thread_fail;
    }

    if (input_ctxt->use_event_callback)
    {
        qcarcam_param_value_t param;

        param.ptr_value = (void *)cvbs_event_cb;
        ret = qcarcam_s_param(input_ctxt->qcarcam_context, QCARCAM_PARAM_EVENT_CB, &param);

        param.uint_value = QCARCAM_EVENT_FRAME_READY | QCARCAM_EVENT_INPUT_SIGNAL | QCARCAM_EVENT_ERROR;
        ret = qcarcam_s_param(input_ctxt->qcarcam_context, QCARCAM_PARAM_EVENT_MASK, &param);
    }

    while (!input_ctxt->qcarcam_window.gbm_buf_ready && try_buffers)
    {
        CVBS_INFO("Wait 5ms and try if buffer ready; %d attempts left", try_buffers);
        usleep(5*1000);
        try_buffers--;
        if(try_buffers == 0)
            return (void *)-1;
    }

    ret = qcarcam_start(input_ctxt->qcarcam_context);
    if (ret != QCARCAM_RET_OK)
    {
        CVBS_INFO("qcarcam_start() failed");
        goto qcarcam_thread_fail;
    }

    input_ctxt->is_running = 1;

    if (!input_ctxt->use_event_callback)
    {
        while (!abort_ais)
        {
            if (cvbs_handle_new_frame(input_ctxt))
                break;
        }
    }
    else
    {
        pthread_mutex_lock(&mutex_abort);
        if (!abort_ais)
        {
            pthread_cond_wait(&cond_abort, &mutex_abort);
        }
        pthread_mutex_unlock(&mutex_abort);
    }

    return NULL;

qcarcam_thread_fail:
    if (input_ctxt->qcarcam_context)
        qcarcam_close(input_ctxt->qcarcam_context);

    return (void *)-1;
}

static int cvbs_create_uyvy_wlbuffer(cvbs_buf_t *user_ctxt, unsigned int idx, int w, int h)
{
    uint64_t modifier = 0;
    uint32_t flags = 1;

    if (protocol == USE_GBM)
    {
        struct gbm_buffer_params *params;

        params = gbm_buffer_backend_create_params(wl_display->gbmbuf);
        flags |= GBM_BUFFER_PARAMS_FLAGS_EARLY_DISPLAY;
        gbm_buffer_params_add_listener(params, &gbm_params_listener, &user_ctxt->gbm_buf_wl[idx]);
        gbm_buffer_params_create(params,
                                 user_ctxt->gbm_buf[idx].bo_fd,
                                 user_ctxt->gbm_buf[idx].meta_fd,
                                 w,
                                 h,
                                 DRM_FORMAT_UYVY,
                                 flags);
    }

    if (protocol == USE_DMA)
    {
        int offset = 0;
        struct zlinux_buffer_params *params;

        struct ion_fd_data sFdData = user_ctxt->gbm_buf[idx].fd_data;

        params = zlinux_dmabuf_create_params(wl_display->dmabuf);
        zlinux_buffer_params_add(params,
                                 sFdData.fd,
                                 0, /* plane_idx */
                                 0, /* offset */
                                 w * 2,
                                 modifier >> 32,
                                 modifier & 0xffffffff);

        zlinux_buffer_params_add_listener(params, &zlinux_params_listener, &user_ctxt->gbm_buf_wl[idx]);
        zlinux_buffer_params_create(params,
                                    w,
                                    h,
                                    DRM_FORMAT_UYVY,
                                    flags);
    }

    return QCARCAM_RET_OK;
}

static qcarcam_ret_t cvbs_init_buffers(cvbs_buf_t *user_ctxt, qcarcam_buffers_t *buffers)
{
    if(enable_display == RENDER_WESTON)
    {
        struct window_wl *window;

        window = (window_wl *)calloc(1, sizeof(*window));
        if (!window)
        {
            return QCARCAM_RET_FAILED;
        }

        user_ctxt->wl_window = window;

        struct wl_region *region;

        user_ctxt->wl_window->callback = NULL;
        user_ctxt->wl_window->display = wl_display;
        if(enable_deinterlace)
        {
            user_ctxt->wl_window->width = 720;
            user_ctxt->wl_window->height = 480;
        }
        else
        {
            user_ctxt->wl_window->width = user_ctxt->buffer_size[0];
            user_ctxt->wl_window->height = user_ctxt->buffer_size[1];
        }
        user_ctxt->wl_window->surface = wl_compositor_create_surface(wl_display->compositor);

        if (wl_display->ivi_application)
        {
            id_ivisurf = IVI_SURFACE_ID + (uint32_t)getpid();

            user_ctxt->wl_window->ivi_surface = ivi_application_surface_create(wl_display->ivi_application, id_ivisurf, user_ctxt->wl_window->surface);

            if (user_ctxt->wl_window->ivi_surface == NULL)
            {
                CVBS_INFO("Failed to create ivi_client_surface");
                return QCARCAM_RET_FAILED;
            }
            ivi_surface_add_listener(user_ctxt->wl_window->ivi_surface, &ivi_surface_listener, user_ctxt->wl_window);
        }
        else if (wl_display->shell)
        {
            user_ctxt->wl_window->shell_surface = wl_shell_get_shell_surface(wl_display->shell, user_ctxt->wl_window->surface);
            if (!user_ctxt->wl_window->shell_surface)
            {
                CVBS_INFO("wl_shell_get_shell_surface failed.");
            }

            wl_shell_surface_add_listener(user_ctxt->wl_window->shell_surface, &shell_surface_listener, NULL);
            wl_shell_surface_set_fullscreen(user_ctxt->wl_window->shell_surface, WL_SHELL_SURFACE_FULLSCREEN_METHOD_SCALE, 0, NULL);
            wl_surface_damage(user_ctxt->wl_window->surface, 0, 0, user_ctxt->wl_window->width, user_ctxt->wl_window->height);
            region = wl_compositor_create_region(user_ctxt->wl_window->display->compositor);
            wl_region_add(region, 0, 0, user_ctxt->wl_window->width, user_ctxt->wl_window->height);
            wl_surface_set_opaque_region(user_ctxt->wl_window->surface, region);
            wl_region_destroy(region);

            user_ctxt->wl_window->callback = wl_surface_frame(user_ctxt->wl_window->surface);
        }
        else
        {
            CVBS_INFO("display->shell NULL.");
        }

        struct gbm_buffer *gbm_buf_wl = (gbm_buffer *)calloc(buffers->n_buffers, sizeof(*gbm_buf_wl));

        while (!user_ctxt->gbm_buf_ready)
        {
            CVBS_INFO("Waiting 3ms for ionbuf ready");
            usleep(3*1000);
        }

        user_ctxt->gbm_buf_wl = gbm_buf_wl;

        int rc = -1;
        for (int i = 0; i < buffers->n_buffers; i++)
        {
            rc = cvbs_create_uyvy_wlbuffer(user_ctxt, i, buffers->buffers[0].planes[0].width, buffers->buffers[0].planes[0].height);
            if (rc)
            {
                CVBS_INFO("cvbs_create_uyvy_wlbuffer failed");
            }
        }

        wl_display_roundtrip(wl_display->display);
        user_ctxt->wl_buf_ready = true;
    }

    return QCARCAM_RET_OK;
}

static void cvbs_ivi_application_deinit()
{
    if (enable_display == RENDER_WESTON && wl_display)
    {
        if (wl_display->ivi_application)
        {
            ilm_layerSetVisibility(layer, ILM_FALSE);
            ilm_layerRemoveSurface(layer, id_ivisurf);
            ilm_layerRemove(layer);
            ilm_unregisterNotification();
            ilm_destroy();
        }
    }
}

static qcarcam_ret_t cvbs_deinit_weston_buffers(cvbs_buf_t *user_ctxt)
{
    user_ctxt->wl_buf_ready = false;

    if (user_ctxt->gbm_buf_wl)
    {
        for (int i = 0; i < user_ctxt->n_buffers; i++){
            if ( user_ctxt->gbm_buf_wl[i].wl_buf )
            {
                wl_buffer_destroy(user_ctxt->gbm_buf_wl[i].wl_buf);
            }
        }
        free(user_ctxt->gbm_buf_wl);
        user_ctxt->gbm_buf_wl = NULL;
    }

    if (enable_display == RENDER_WESTON && user_ctxt->wl_window)
    {
        if (user_ctxt->wl_window->shell_surface)
        {
            wl_shell_surface_destroy(user_ctxt->wl_window->shell_surface);
        }

        if (user_ctxt->wl_window->ivi_surface)
        {
            ivi_surface_destroy(user_ctxt->wl_window->ivi_surface);
        }

        if (user_ctxt->wl_window->surface)
        {
            wl_surface_destroy(user_ctxt->wl_window->surface);
        }

        if (user_ctxt->wl_window)
        {
            if (protocol != USE_GBM)
                wl_callback_destroy(user_ctxt->wl_window->callback);
            free(user_ctxt->wl_window);
            user_ctxt->wl_window = NULL;
        }

        if (wl_display->shell)
        {
            wl_shell_destroy(wl_display->shell);
        }

        if (wl_display->ivi_application)
        {
            ivi_application_destroy(wl_display->ivi_application);
        }

        if (wl_display->dmabuf)
        {
            zlinux_dmabuf_destroy(wl_display->dmabuf);
        }

        if (wl_display->compositor)
        {
            wl_compositor_destroy(wl_display->compositor);
        }

        wl_registry_destroy(wl_display->registry);
        wl_display_flush(wl_display->display);
        wl_display_disconnect(wl_display->display);
        free(wl_display);
        wl_display = NULL;
    }

    return QCARCAM_RET_OK;
}

static qcarcam_ret_t cvbs_deinit_buffers(cvbs_buf_t *user_ctxt)
{
    abort_ais = 1;

    if (user_ctxt->gbm_buf)
    {
        for (int i = 0; i < user_ctxt->n_buffers; i++)
        {
            if (user_ctxt->gbm_buf[i].p_data)
            {
                munmap(user_ctxt->gbm_buf[i].p_data, user_ctxt->gbm_buf[i].length);
                user_ctxt->gbm_buf[i].p_data = NULL;
            }
        }
        free(user_ctxt->gbm_buf);
        user_ctxt->gbm_buf = NULL;
    }

    cvbs_deinit_weston_buffers(user_ctxt);

    return QCARCAM_RET_OK;
}

static void configure_ilm_surface(t_ilm_uint id, t_ilm_uint width, t_ilm_uint height)
{
    ilm_surfaceSetDestinationRectangle(id, 0, 0, screenWidth, screenHeight);
    CVBS_INFO("SetDestinationRectangle: surface ID (%d), Width (%u), Height (%u)", id, screenWidth, screenHeight);
    ilm_surfaceSetSourceRectangle(id, 0, 0, width, height);
    CVBS_INFO("SetSourceRectangle     : surface ID (%d), Width (%u), Height (%u)", id, width, height);
    ilm_surfaceSetVisibility(id, ILM_TRUE);
    CVBS_INFO("SetVisibility          : surface ID (%d), ILM_TRUE", id);
    ilm_layerAddSurface(layer,id);
    CVBS_INFO("layerAddSurface        : surface ID (%d) is added to layer ID (%d)", id, layer);
    ilm_commitChanges();
    pthread_cond_signal( &waiterVariable );
}

static void surfaceCallbackFunction(t_ilm_uint id, struct ilmSurfaceProperties* sp, t_ilm_notification_mask m)
{
    if ((unsigned)m & ILM_NOTIFICATION_CONFIGURED)
    {
        configure_ilm_surface(id, sp->origSourceWidth, sp->origSourceHeight);
    }
}

static void callbackFunction(ilmObjectType object, t_ilm_uint id, t_ilm_bool created, void *user_data)
{
    (void)user_data;
    struct ilmSurfaceProperties sp;

    if (object == ILM_SURFACE)
    {
        if (created)
        {
            if (number_of_surfaces > 0)
            {
                number_of_surfaces--;
                CVBS_INFO("surface                : %d created",id);
                ilm_getPropertiesOfSurface(id, &sp);

                if ((sp.origSourceWidth != 0) && (sp.origSourceHeight !=0))
                {   // surface is already configured
                    configure_ilm_surface(id, sp.origSourceWidth, sp.origSourceHeight);
                }
                else
                {
                    // wait for configured event
                    ilm_surfaceAddNotification(id,&surfaceCallbackFunction);
                    ilm_commitChanges();
                }
            }
        }
        else
        {
            if(!created)
            {
                CVBS_INFO("surface: %d destroyed",id);
            }
        }
    }
    else
    {
        if (object == ILM_LAYER)
        {
            if (created)
            {
                CVBS_INFO("layer: %d created",id);
            }
            else
            {
                if(!created)
                {
                    CVBS_INFO("layer: %d destroyed",id);
                }
            }
        }
    }
}

static void drm_close_display()
{
    gpio_preview = false;
    if (drmModeSetPlane(drm_fd, plane_id, crtc_id, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0))
    {
        CVBS_INFO("destroy plane failed\n");
    }
    else
    {
        CVBS_INFO("plane %d destroied\n", plane_id);
    }
    if (drmDropMaster(drm_fd))
    {
        CVBS_INFO("drop master failed\n");
    }
    else
    {
        CVBS_INFO("drop master success\n");
    }
    usleep(100000);
    if (crtc)
        drmModeFreeCrtc(crtc);
    if (connector)
        drmModeFreeConnector(connector);
    if (resources)
        drmModeFreeResources(resources);
    if (plane_res)
        drmModeFreePlaneResources(plane_res);
    if (fbid_current)
        drmModeRmFB(drm_fd, fbid_current);
    if (crtc_bo)
        gbm_bo_destroy(crtc_bo);
    crtc_bo = NULL;
    if (gbm)
        gbm_device_destroy(gbm);
    gbm = NULL;
    gbm_created = false;
    if (drm_fd)
        close(drm_fd);
    drm_fd = -1;
    drm_ready = false;
}

static void cvbs_clean(cvbs_input_t *input_ctxt)
{
    // cleanup
    CVBS_INFO("clean and quit");
    if (input_ctxt)
    {
        if (input_ctxt->qcarcam_context != NULL)
        {
            (void)qcarcam_stop(input_ctxt->qcarcam_context);
            (void)qcarcam_close(input_ctxt->qcarcam_context);
            input_ctxt->qcarcam_context = NULL;
        }

        cvbs_deinit_buffers(&input_ctxt->qcarcam_window);
        cvbs_deinit_buffers(&input_ctxt->display_window);

        if (enable_display == RENDER_DRM)
            drm_close_display();
        else
            weston_close_display(input_ctxt);

        if (input_ctxt->p_buffers.buffers)
        {
            free(input_ctxt->p_buffers.buffers);
            input_ctxt->p_buffers.buffers = NULL;
        }

        if (input_ctxt->p_buffers_disp.buffers)
        {
            free(input_ctxt->p_buffers_disp.buffers);
            input_ctxt->p_buffers_disp.buffers = NULL;
        }

        free(input_ctxt);
    }

    if (enable_display == RENDER_WESTON)
    {
        if (ionfd > 0 && protocol == USE_DMA)
            close(ionfd);

        if (drm_fd > 0 && protocol == USE_GBM)
            close(drm_fd);
    }
}

static qcarcam_ret_t set_crtc(unsigned int color_format)
{
    union gbm_bo_handle bo_handle;
    uint32_t handles[4] = {0};
    uint32_t pitches[4] = {0};
    uint32_t offsets[4] = {0};
    uint32_t flags = 0;
    uint32_t fbid;
    struct drm_mode_fb_cmd2 args;
    unsigned char *back_buffer;
    unsigned int buffer_size = 0;
    unsigned int width, height;

    width = connector->modes[0].hdisplay;
    height = connector->modes[0].vdisplay;
    /* create bo */
    crtc_bo = gbm_bo_create(gbm, width, height, color_format, GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING | GBM_BO_USE_WRITE);
    if (crtc_bo == NULL)
    {
        CVBS_INFO("Failed to create GBM bo");
        goto failed;
    }

    /*get the fb of display from bo */
    width = gbm_bo_get_width(crtc_bo);
    height = gbm_bo_get_height(crtc_bo);
    bo_handle = gbm_bo_get_handle(crtc_bo);
    handles[0] = bo_handle.u32;
    pitches[0] = gbm_bo_get_stride(crtc_bo);
    offsets[0] = 0;

    buffer_size = pitches[0] * height;
    back_buffer = (unsigned char*)malloc(buffer_size);
    CVBS_INFO("buffer_size = %d", buffer_size);
    if (!back_buffer)
    {
        goto failed_with_bo;
    }

    memset(back_buffer, 0xff, buffer_size);

    if (gbm_bo_write(crtc_bo, back_buffer, per_frame_byte))
    {
        CVBS_INFO("gbm_bo_write calling failed");
        goto failed_with_buffer;
    }

    free(back_buffer);
    back_buffer = NULL;

    if (drmModeAddFB2(drm_fd, width, height, color_format, handles, pitches, offsets, &fbid, 0))
    {
        CVBS_INFO("failed to add fb: %s", strerror(errno));
        goto failed_with_bo;
    }

    /* set fb to display */
    if(drmModeSetCrtc(drm_fd, crtc_id, fbid, 0, 0, &connector->connector_id, 1, &connector->modes[0]))
    {
        CVBS_INFO("drm_fd=%d, crtc_id=%d, fbid=%d, connector_id=%d,failed to set mode.errno=%s", drm_fd, crtc_id, fbid, connector->connector_id, strerror(errno));
        goto failed_with_bo;
    }

    fbid_current = fbid;
    return QCARCAM_RET_OK;

failed_with_buffer:
    free(back_buffer);
    back_buffer = NULL;

failed_with_bo:
    if (crtc_bo)
        gbm_bo_destroy(crtc_bo);

failed:
    return QCARCAM_RET_FAILED;
}

static bool format_support(const drmModePlanePtr ovr, uint32_t fmt)
{
    unsigned int i;

    for (i = 0; i < ovr->count_formats; ++i)
    {
        if (ovr->formats[i] == fmt)
            return true;
    }

    return false;
}

static int find_crtc_for_connector(int fd, drmModeRes *resources, drmModeConnector *connector)
{
    drmModeEncoder *encoder;
    uint32_t possible_crtcs;
    int i, j;

    for (j = 0; j < connector->count_encoders; j++)
    {
        encoder = drmModeGetEncoder(fd, connector->encoders[j]);
        if (encoder == NULL)
        {
            CVBS_INFO("Failed to get encoder.\n");
            return -1;
        }
        possible_crtcs = encoder->possible_crtcs;
        drmModeFreeEncoder(encoder);

        for (i = 0; i < resources->count_crtcs; i++)
        {
            if (possible_crtcs & (1 << i))
                return i;
        }
    }

    return -1;
}

static qcarcam_ret_t drm_open_display(cvbs_input_t *input_ctxt)
{
    int idx = -1, i = 0;
    drmModePlane *ovr;
    int try_master = 100;

    drm_ready = false;
    if (input_ctxt->qcarcam_input_id == QCARCAM_INPUT_TYPE_ANALOG_MEDIA)
    {
        if (enable_deinterlace)
        {
            per_frame_byte = 736 * 2 * 480;
        }
        else
        {
            per_frame_byte = 736 * 2 * 507;
        }
    }
    else
    {
        per_frame_byte = 1280 * 2 * 720;
    }

    while (!gbm_created)
    {
        CVBS_INFO("gbm device not created, wait 5ms");
        usleep(5000);
    }

    resources = drmModeGetResources(drm_fd);
    if (!resources)
    {
        CVBS_INFO("drmModeGetResources failed");
        goto failed_created;
    }

    for (i = 0; i < resources->count_connectors; i++)
    {
        connector = drmModeGetConnector(drm_fd, resources->connectors[i]);
        if (!connector)
        {
            CVBS_INFO("failed to get connector");
            goto failed_resourced;
        }

        if (connector->connector_type == DRM_MODE_CONNECTOR_HDMIA)
        {
            CVBS_INFO("get HDMI connctor succeed!");
            break;
        }
    }

    if (i == resources->count_connectors)
    {
        CVBS_INFO("failed to detect HDMI connctor");
        goto failed_resourced;
    }

    idx = find_crtc_for_connector(drm_fd, resources, connector);
    if (idx < 0)
    {
        CVBS_INFO("No usable crtc/encoder pair for connector");
        goto failed_with_connector;
    }

    crtc =  drmModeGetCrtc(drm_fd, resources->crtcs[idx]);

    if (!crtc)
    {
        CVBS_INFO("failed to get crtc");
        goto failed_with_connector;
    }

    crtc_id = crtc->crtc_id;
    if (crtc_id < 0)
    {
        CVBS_INFO("failed to get crtc id");
        goto failed_with_crtc;
    }
    CVBS_INFO("crtc id is %d for connector %d", crtc_id, i);

    drmSetClientCap(drm_fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

    plane_res = drmModeGetPlaneResources(drm_fd);
    if (!plane_res)
    {
        CVBS_INFO("drmModeGetPlaneResources failed");
        goto failed_created;
    }

    for (i = 0; i < plane_res->count_planes; i++)
    {
        ovr = drmModeGetPlane(drm_fd, plane_res->planes[i]);
        if (!ovr)
        {
            CVBS_INFO("drmModeGetPlane failed");
            continue;
        }

        if (!format_support(ovr, DRM_FORMAT_UYVY))
            continue;

        if (ovr->possible_crtcs & (1 << idx))
        {
            plane_id = ovr->plane_id;
            break;
        }
    }

    while (drmSetMaster(drm_fd) && try_master)
    {
        usleep(3000);
        try_master--;
        CVBS_INFO("set master failed");
        if (!try_master)
            goto failed;
    }

    CVBS_INFO("plane id is %d", plane_id);
    if(set_crtc(DRM_FORMAT_UYVY))
    {
        CVBS_INFO("set crtc failed");
        goto failed_with_crtc;
    }
    else
        CVBS_INFO("set crtc succeeds");

    drm_ready = true;
    gpio_preview = true;
    place_marker("# CVBS open drm done");

    return QCARCAM_RET_OK;

failed_with_crtc:
    drmModeFreeCrtc(crtc);

failed_with_connector:
    drmModeFreeConnector(connector);

failed_resourced:
    drmModeFreeResources(resources);
    drmModeFreePlaneResources(plane_res);

failed_created:
    if(gbm)
        gbm_device_destroy(gbm);

failed:
    close(drm_fd);
    drm_fd = -1;

    return QCARCAM_RET_FAILED;
}

static void weston_open_display(cvbs_input_t *input_ctxt)
{
    qcarcam_ret_t ret;
    int rc = -1;
    pthread_attr_t pthread_attr;

    weston_op = true;
    abort_disp = 0;

    wl_display = create_wl_display();
    if (!wl_display)
    {
        CVBS_INFO("create_wl_display failed");
        goto fail;
    }

    rc = pthread_attr_init(&pthread_attr);
    if (rc)
    {
        CVBS_INFO("pthread_attr_init failed");
        goto fail;
    }

    if(enable_deinterlace)
    {
        ret = cvbs_init_buffers(&input_ctxt->display_window, &input_ctxt->p_buffers_disp);
        if (ret != QCARCAM_RET_OK)
        {
            CVBS_INFO("cvbs_init_wlbuf failed");
        }
    }
    else
    {
        ret = cvbs_init_buffers(&input_ctxt->qcarcam_window, &input_ctxt->p_buffers);

        if (ret != QCARCAM_RET_OK)
        {
            CVBS_INFO("cvbs_init_wlbuf failed");
        }
    }

    gpio_preview = true;
    weston_op = false;

    if(__sync_fetch_and_sub(&first_display, 0 ) == 1)
    {
        CVBS_INFO("cvbs_init_wlbuf ready, render");
        cvbs_handle_new_frame(input_ctxt);
    }

    if (wl_display->ivi_application)
    {
        layer = 1;
        number_of_surfaces = 1;

        pthread_mutexattr_t a;
        if (pthread_mutexattr_init(&a) != 0)
            goto fail;

        if (pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE) != 0)
        {
            pthread_mutexattr_destroy(&a);
            goto fail;
        }

        if (pthread_mutex_init(&mutex, &a) != 0)
        {
            pthread_mutexattr_destroy(&a);
            CVBS_INFO("failed to initialize pthread_mutex");
            goto fail;
        }

        pthread_mutexattr_destroy(&a);

        struct ilmScreenProperties screenProperties;
        t_ilm_layer renderOrder[1];
        renderOrder[0] = layer;
        ilm_init();
        ilm_getPropertiesOfScreen(0, &screenProperties);
        screenWidth = screenProperties.screenWidth;
        screenHeight = screenProperties.screenHeight;
        ilm_layerCreateWithDimension(&layer, screenWidth, screenHeight);
        CVBS_INFO("CreateWithDimension: layer ID (%d), Width (%u), Height (%u)", layer, screenWidth, screenHeight);
        ilm_layerSetVisibility(layer,ILM_TRUE);
        CVBS_INFO("SetVisibility      : layer ID (%d), ILM_TRUE", layer);
        ilm_displaySetRenderOrder(0,renderOrder,1);
        ilm_commitChanges();
        ilm_registerNotification(callbackFunction, NULL);

        while (number_of_surfaces > 0) {
            pthread_mutex_lock(&mutex);
            pthread_cond_wait( &waiterVariable, &mutex);
        }
    }

    /*signal handler thread*/
    rc = pthread_create(&pthread_helper, &pthread_attr, &helper_thread, NULL);
    if (rc)
    {
        CVBS_INFO("pthread_create failed");
        goto fail;
    }

    return;

fail:
    quit = 1;
    abort_qcarcam();
    abort_display();
    return;
}

static void weston_close_display(cvbs_input_t *input_ctxt)
{
    weston_op = true;
    gpio_preview = false;

    abort_display();

    cvbs_ivi_application_deinit();

    if(enable_deinterlace)
    {
        cvbs_deinit_weston_buffers(&input_ctxt->display_window);
    }
    else
    {
        cvbs_deinit_weston_buffers(&input_ctxt->qcarcam_window);
    }

    weston_op = false;
}

static void *cvbs_thread(void *arg)
{
    qcarcam_ret_t ret;
    int i, rc;
    const char *tok;
    pthread_attr_t pthread_attr;

    place_marker("# CVBS thread");
    CVBS_INFO("enable_display: %d", enable_display);

    pthread_setname_np(pthread_self(), "cvbs_thread");

    abort_ais = 0;
    abort_disp = 0;
    weston_op = false;

    test_input =(cvbs_input_t *) calloc(1, sizeof *test_input);

    if(!test_input)
        goto fail;

    test_input->idx = 0;
    test_input->qcarcam_input_id = input_id;
    test_input->use_event_callback = 1;
    test_input->frame_timeout = QCARCAM_TEST_DEFAULT_GET_FRAME_TIMEOUT;
    test_input->n_buffers_qcarcam = 5;
    if (test_input->qcarcam_input_id == QCARCAM_INPUT_TYPE_ANALOG_MEDIA)
    {
        if(enable_deinterlace)
            test_input->n_buffers_disp = 5;
        test_input->qcarcam_window.buffer_size[0] = 720;
        test_input->qcarcam_window.buffer_size[1] = 507;
        test_input->display_window.buffer_size[0] = 720;
        if (enable_deinterlace)
            test_input->display_window.buffer_size[1] = 480;
        else
            test_input->display_window.buffer_size[1] = 507;
    }
    else
    {
        test_input->qcarcam_window.buffer_size[0] = 1280;
        test_input->qcarcam_window.buffer_size[1] = 720;
    }
    test_input->display_window.gbm_buf_ready = false;
    test_input->display_window.wl_buf_ready = false;
    test_input->qcarcam_window.gbm_buf_ready = false;
    test_input->qcarcam_window.wl_buf_ready = false;
    test_input->prev_buf_idx_qcarcam = -1;
    test_input->p_buffers.n_buffers = test_input->n_buffers_qcarcam;
    test_input->p_buffers.color_fmt = QCARCAM_FMT_UYVY_8;
    test_input->p_buffers.buffers = (qcarcam_buffer_t *)calloc(test_input->p_buffers.n_buffers, sizeof(*test_input->p_buffers.buffers));
    if (test_input->p_buffers.buffers == 0)
    {
        CVBS_INFO("alloc qcarcam_buffer failed");
        goto fail;
    }

    for (i = 0; i < test_input->n_buffers_qcarcam; ++i)
    {
        test_input->p_buffers.buffers[i].n_planes = 1;
        test_input->p_buffers.buffers[i].planes[0].width = test_input->qcarcam_window.buffer_size[0];
        test_input->p_buffers.buffers[i].planes[0].height = test_input->qcarcam_window.buffer_size[1];
    }

    if(enable_deinterlace)
    {
        test_input->p_buffers_disp.n_buffers = test_input->n_buffers_disp;
        test_input->p_buffers_disp.color_fmt = QCARCAM_FMT_UYVY_8;
        test_input->p_buffers_disp.buffers = (qcarcam_buffer_t *)calloc(test_input->p_buffers_disp.n_buffers, sizeof(*test_input->p_buffers_disp.buffers));
        if (test_input->p_buffers_disp.buffers == 0)
        {
            CVBS_INFO("alloc disp_buffer failed");
            goto fail;
        }

        for (i = 0; i < test_input->n_buffers_disp; ++i)
        {
            test_input->p_buffers_disp.buffers[i].n_planes = 1;
            test_input->p_buffers_disp.buffers[i].planes[0].width = test_input->display_window.buffer_size[0];
            test_input->p_buffers_disp.buffers[i].planes[0].height = test_input->display_window.buffer_size[1];
        }
    }

    /*launch threads to do the work*/
    rc = pthread_create(&test_input->thread_id, NULL, cvbs_setup_input_ctxt_thread, test_input);
    if (rc)
    {
        CVBS_INFO("pthread_create failed");
        goto fail;
    }

    if(enable_display == RENDER_WESTON)
    {
        weston_open_display(test_input);
    }

    if (enable_display == RENDER_DRM)
    {
        drm_open_display(test_input);
    }

fail:
    return NULL;
}

int main(int argc, char **argv)
{
    int i, rc;
    qcarcam_ret_t ret;
    const char *tok;
    sigset_t sigset;
    pthread_t signal_thread_id, pthread_cvbs;
    pthread_attr_t pthread_attr;

    bool is_started = false;
    first_start = true;
    gpio_preview = false;
    CVBS_INFO("Enter cvbs app!");
    place_marker("# CVBS main");

    for (i = 1; i < argc; i++)
    {
        if (!strncmp(argv[i], "-dumpFirst", strlen("-dumpFirst")))
        {
            dump_first_frame = true;
        }
        if (!strncmp(argv[i], "-display", strlen("-display")))
        {
            char di[8] = "drm";
            enable_display = RENDER_DRM;
            if (!strncmp(argv[i], "-display=", strlen("-display=")))
            {
                tok = argv[i] + strlen("-display=");
                snprintf(di, sizeof(di), "%s", tok);

                if(!strncmp(di, "weston", strlen("weston")))
                    enable_display = RENDER_WESTON;

                if(strncmp(di, "drm", strlen("drm")) && (strncmp(di, "weston", strlen("weston"))))
                {
                    CVBS_INFO("Only \"drm\" or \"weston\" is supported!");
                    return 0;
                }
            }
        }
        if (!strncmp(argv[i], "-deinterlace", strlen("-deinterlace")))
        {
            char di_method[16] = "bob";
            enable_deinterlace = DEINTERLACE_BOB;
            if (!strncmp(argv[i], "-deinterlace=", strlen("-deinterlace=")))
            {
                tok = argv[i] + strlen("-deinterlace=");
                snprintf(di_method, sizeof(di_method), "%s", tok);

                if(!strncmp(di_method, "bob", strlen("bob")))
                    enable_deinterlace = DEINTERLACE_BOB;
                else if (!strncmp(di_method, "weave", strlen("weave")))
                    enable_deinterlace = DEINTERLACE_WEAVE;
                else
                {
                    CVBS_INFO("Only \"drm\" or \"weston\" is supported!");
                    return 0;
                }
            }
        }
        if (!strncmp(argv[i], "-protocol", strlen("-protocol")))
        {
            char prot[16] = "gbm";

            tok = argv[i] + strlen("-protocol=");
            snprintf(prot, sizeof(prot), "%s", tok);

            if(!strncmp(prot, "dma", strlen("dma")))
                protocol = USE_DMA; //dma
            else
                protocol = USE_GBM; //gbm
        }
        if (!strncmp(argv[i], "-dumpFrame=", strlen("-dumpFrame=")))
        {
            tok = argv[i] + strlen("-dumpFrame=");
            dumpFrame = atoi(tok);
        }
        if (!strncmp(argv[i], "-input=", strlen("-input=")))
        {
            int id = 0;
            tok = argv[i] + strlen("-input=");
            id = atoi(tok);

            if (id < 0 || id > 7)
            {
                CVBS_INFO("Wrong input_id %d, should be 0-7", id);
                goto fail;
            }
            else if (id == 0)
            {
                input_id = QCARCAM_INPUT_TYPE_EXT_REAR;
            }
            else if (id == 1)
            {
                input_id = QCARCAM_INPUT_TYPE_EXT_FRONT;
            }
            else if (id == 2)
            {
                input_id = QCARCAM_INPUT_TYPE_EXT_LEFT;
            }
            else if (id == 3)
            {
                input_id = QCARCAM_INPUT_TYPE_EXT_RIGHT;
            }
            else if (id == 4)
            {
                input_id = QCARCAM_INPUT_TYPE_DRIVER;
            }
            else if (id == 5)
            {
                input_id = QCARCAM_INPUT_TYPE_LANE_WATCH;
            }
            else if (id == 6)
            {
                input_id = QCARCAM_INPUT_TYPE_DIGITAL_MEDIA;
            }
            else if (id == 7)
            {
                input_id = QCARCAM_INPUT_TYPE_ANALOG_MEDIA;
            }
        }
        if (!strncmp(argv[i], "-gpioValue=", strlen("-gpioValue=")))
        {
            tok = argv[i] + strlen("-gpioValue=");
            skip_gpio = false;
            snprintf(gpiovalue, sizeof(gpiovalue), "%s", tok);
        }
    }

    if (input_id != QCARCAM_INPUT_TYPE_ANALOG_MEDIA)
    {
        enable_deinterlace =  DEINTERLACE_DISABLE;
    }

    sigfillset(&sigset);
    for (i = 0; exceptsigs[i] != -1; i++)
    {
        sigdelset(&sigset, exceptsigs[i]);
    }
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    /*signal handler thread*/
    rc = pthread_create(&signal_thread_id, NULL, signal_thread, NULL);
    if (rc)
    {
        CVBS_INFO("pthread_create failed");
        goto fail;
    }

    if(!open(gpiovalue, O_RDONLY) || skip_gpio)
    {
        quit = 1;
        CVBS_INFO("invoke cvbs_thread");

        rc = pthread_attr_init(&pthread_attr);
        if (rc)
        {
            CVBS_INFO("pthread_attr_init failed");
            goto fail;
        }

        rc = pthread_create(&pthread_cvbs, &pthread_attr, &cvbs_thread, NULL);
        if (rc)
        {
            CVBS_INFO("pthread_create failed");
            goto fail;
        }

        pthread_mutex_lock(&mutex_abort);
        if (!abort_ais)
        {
            pthread_cond_wait(&cond_abort, &mutex_abort);
        }
        pthread_mutex_unlock(&mutex_abort);
    }

    while(!quit)
    {
        int src_fd = open(gpiovalue, O_RDONLY);
        if(src_fd)
        {
            read(src_fd, &gpio109, 1);
            switch(gpio109)
            {
            case '0':
                if(is_started)
                {
                    is_started = false;

                    CVBS_INFO("stop preview");

                    if(enable_display == RENDER_DRM)
                    {
                        drm_close_display();
                    }

                    if(enable_display == RENDER_WESTON)
                    {
                        weston_close_display(test_input);
                    }
                }

                if(enable_display == RENDER_DRM)
                {
                    enable_display = RENDER_WESTON;
                }
                break;
            case '1':
                if(!is_started)
                {
                    is_started = true;

                    CVBS_INFO("start preview");
                    if(!first_start)
                    {
                        if(enable_display == RENDER_WESTON)
                        {
                            weston_open_display(test_input);
                        }
                    }
                    else
                    {
                        first_start = false;
                        CVBS_INFO("invoke cvbs_thread");

                        rc = pthread_attr_init(&pthread_attr);
                        if (rc)
                        {
                            CVBS_INFO("pthread_attr_init failed");
                            goto fail;
                        }

                        /*signal handler thread*/
                        rc = pthread_create(&pthread_cvbs, &pthread_attr, &cvbs_thread, NULL);
                        if (rc)
                        {
                            CVBS_INFO("pthread_create failed");
                            goto fail;
                        }
                    }
                }
                break;
            default:
                break;
            }

            close(src_fd);
        }
        usleep(gpio_interval);
    }

fail:
    cvbs_clean(test_input);
    drm_fd = -1;
    gbm = NULL;
    return EXIT_SUCCESS;
}
