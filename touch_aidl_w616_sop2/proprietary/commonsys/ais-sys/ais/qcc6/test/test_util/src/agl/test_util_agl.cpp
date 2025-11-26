/* ===========================================================================
 * Copyright (c) 2017-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <linux/msm_ion.h>
#include <linux/ion.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "gbm.h"
#include "gbm_priv.h"

#include "c2d2.h"
#include "qcarcam.h"
#include "test_util.h"
#include "test_util_agl.h"
#include <wayland-client.h>
#include "ivi-application-client-protocol.h"




#define BACKGROUND_COLOR_GRAY 0x77
/**
 * The parent surface is used by display for the setposition, has no matter with camera, use the hardcode format
 */
#define PARENT_SURFACE_COLOR_FOMAT    QCARCAM_FMT_UYVY_8
/**
* In theroy if post 2 buffers to display, display should release 1 buffer back.
* In the running time, in fact post more than 2 buffers to display, 1 buffer is released back.
* When submit MAX_COMMIT_FRAME_NUM buffers to display, we need to wait util 1 buffer is released back
* Camera app's buffer num is >= 4 (qcarcam_rcv is 4, qcarcam_test is default 5)
* MAX_COMMIT_FRAME_NUM should be <= camera app's buffer num, if too small, the FPS maybe low.
*/
#define MAX_COMMIT_FRAME_NUM    4

//power manager resource
typedef struct
{
    void* event_client_data;
    power_event_callable p_power_event_callback;
}test_util_pm_handle_t;
static test_util_pm_handle_t g_pm_handle;

static bool display_buffer = 0;
static struct display_wl *display_pointer;
static enum wl_shm_format support_format = WL_SHM_FORMAT_XRGB8888;
static int input_num = 0;
static volatile int aborted = 0;
static bool b_post = false;

static int wl_buf_render(unsigned int stream_idx, int buf_idx);

static int test_util_create_uyvy_wlbuffer(test_util_ctxt_t *ctxt, test_util_window_t *p_window, unsigned int idx, int w, int h);
static int test_util_create_uyvy_wlbuffer_gbm(test_util_ctxt_t *ctxt, test_util_window_t *p_window, unsigned int idx, int w, int h);

static void notify_frame_done(test_util_window_t *p_window);

static inline uint64_t raw_read_ctr_el0(void)
{
    uint64_t value;
    asm volatile("mrs %0, ctr_el0\n" : "=r" (value) : : "memory");
    return value;
}

static inline void dccvac(uint64_t cvac)
{
    asm volatile("dc cvac, %0\n\t" : : "r" (cvac) : "memory");
}

static inline void dccivac(uint64_t cvac)
{
    asm volatile("dc civac, %0\n\t" : : "r" (cvac) : "memory");
}

size_t dcache_line_bytes(void)
{
    uint32_t ctr_el0;
    static size_t line_bytes = 0;

    if (line_bytes)
        return line_bytes;

    ctr_el0 = raw_read_ctr_el0();
    /* [19:16] - Indicates (Log2(number of words in cache line) */
    line_bytes = 1 << ((ctr_el0 >> 16) & 0xf);
    /* Bytes in a word (32-bit) */
    line_bytes *= sizeof(uint32_t);

    return line_bytes;
}

static void dcache_flush_dma(uint8 *addr, size_t len)
{
    uint64_t linesize = dcache_line_bytes();
    uint64_t line = (uint64_t) addr & ~(linesize - 1);

    asm volatile("dsb sy" : : : "memory");
    while (/*(void *)*/line < ((uint64_t)addr + len)) {
        dccivac(line);
        line += linesize;
    }
    asm volatile("isb" : : : "memory");
}


static void handle_xdg_surface_configure(void *data, struct xdg_surface *surface,
                             uint32_t serial)
{
    struct test_util_ctxt_t *ctxt = (struct test_util_ctxt_t*)data;

    xdg_surface_ack_configure(surface, serial);

    if (ctxt->wait_for_configure == true)
    {
        ctxt->wait_for_configure = false;
    }
}

static void handle_ivi_surface_configure(void *data, struct ivi_surface *ivi_surface, int32_t width, int32_t height)
{
}

static void handle_toplevel_configure(void *data, struct xdg_toplevel *toplevel,
                            int32_t width, int32_t height, struct wl_array *states)
{
}

static void handle_toplevel_close(void *data, struct xdg_toplevel *xdg_toplevel)
{
}


static const struct ivi_surface_listener ivi_surface_listener = {
    handle_ivi_surface_configure,
};

static const struct xdg_surface_listener xdg_surface_listener = {
    handle_xdg_surface_configure,
};

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    handle_toplevel_configure,
    handle_toplevel_close,
    NULL
};


static void
output_handle_geometry(void *data, struct wl_output *wl_output,
                       int32_t x, int32_t y,
                       int32_t physical_width, int32_t physical_height,
                       int32_t subpixel,
                       const char *make, const char *model,
                       int32_t output_transform)
{
}

static void
output_handle_mode(void *data, struct wl_output *wl_output,
                   uint32_t flags, int32_t width, int32_t height,
                   int32_t refresh)
{
    struct display_wl *d = (display_wl*)data;

    // Only the first output configure takes effect
    if (d->output_width == 0 && width != 0)
        d->output_width = width;
    if (d->output_height == 0 && height != 0)
        d->output_height = height;

    QCARCAM_INFOMSG("output w=%u h=%u, recv w=%d h=%d", d->output_width, d->output_height, width, height);
}

static const struct wl_output_listener output_listener = {
    output_handle_geometry,
    output_handle_mode,
    NULL, NULL, NULL, NULL
};

static void
frame_done(void *data, struct wl_callback *callback, uint32_t time)
{
    if (callback)
        wl_callback_destroy(callback);
}

static const struct wl_callback_listener frame_listener = {
    frame_done
};

static void wl_shm_fmt(void *dis, struct wl_shm *s, uint32_t f)
{
    struct display_wl *d = (display_wl*)dis;

    if (f == support_format)
        d->formats = support_format;
}

static struct wl_shm_listener qcarcam_shm_lsn = {
    wl_shm_fmt};

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *shell, uint32_t serial)
{
    xdg_wm_base_pong(shell, serial);
}

static const struct xdg_wm_base_listener wm_base_listener = {
    xdg_wm_base_ping,
};

static void wl_registry_handle(void *display_data, struct wl_registry *reg,
                               uint32_t id, const char *intf, uint32_t ver)
{
    struct display_wl *d = (display_wl*)display_data;

    if (strcmp(intf, "wl_compositor") == 0)
    {
        d->compositor = (wl_compositor*)wl_registry_bind(reg, id, &wl_compositor_interface, 1);
    }
    else if (strcmp(intf, "wl_shm") == 0)
    {
        d->shm = (wl_shm*)wl_registry_bind(reg, id, &wl_shm_interface, 1);
        if (d->shm)
        {
            wl_shm_add_listener(d->shm, &qcarcam_shm_lsn, d);
        }
    }
    else if (strcmp(intf, "xdg_wm_base") == 0)
    {
        d->wm_base = (xdg_wm_base*)wl_registry_bind(reg, id, &xdg_wm_base_interface, 1);
        if (d->wm_base)
        {
            xdg_wm_base_add_listener(d->wm_base, &wm_base_listener, d);
        }
    }
    else if (strcmp(intf, "ivi_application") == 0)
    {
        d->ivi_application = (ivi_application*)wl_registry_bind(reg, id, &ivi_application_interface, 1);
    }
    else if (strcmp(intf, "wl_scaler") == 0)
    {
        d->scaler_version = ver < 2 ? ver : 2;
        d->scaler = (wl_scaler*)wl_registry_bind(reg, id,
                                   &wl_scaler_interface,
                                   d->scaler_version);
    }
    else if (strcmp(intf, "wl_output") == 0)
    {
        d->output = (wl_output*)wl_registry_bind(reg, id, &wl_output_interface, 1);
        if (d->output)
        {
            wl_output_add_listener(d->output, &output_listener, d);
        }
    }
    else if (strcmp(intf, "gbm_buffer_backend") == 0)
    {
        d->gbmbuf = (gbm_buffer_backend*)wl_registry_bind(reg, id, &gbm_buffer_backend_interface, 1);
    }
    else if (strcmp(intf, "wp_viewporter") == 0)
    {
        d->viewporter = (wp_viewporter*)wl_registry_bind(reg, id, &wp_viewporter_interface, 1);
    }
    else if (strcmp(intf, "wl_subcompositor") == 0)
    {
        d->subcompositor = (wl_subcompositor*)wl_registry_bind(reg, id, &wl_subcompositor_interface, 1);
    }
}
static void wl_registry_handle_remove(void *display_data, struct wl_registry *reg, uint32_t n)
{
}

static const struct wl_registry_listener wl_registry_lsn = {
    wl_registry_handle,
    wl_registry_handle_remove};

static void buffer_release(void *data, struct wl_buffer *buffer)
{
    struct gbm_buffer *gbm_buf = (struct gbm_buffer *)data;
    static int first_frame = 1;

    if (gbm_buf) {
        struct test_util_window_t* p_window = gbm_buf->p_window;

        if (!p_window)
            return;

        if (p_window->diag != NULL)
            test_util_get_time(&p_window->diag->buf_rel_time[TEST_CUR_BUFFER]);

        if (first_frame)
        {
            ais_log_kpi(AIS_EVENT_KPI_CLIENT_FIRST_FRAME_POST_END);
            first_frame = 0;
        }
        QCARCAM_DBGMSG("buffer release window %p, idx %d", p_window, gbm_buf->idx);

        pthread_mutex_lock(&p_window->m_mutex);
        p_window->commit_frames--;
        p_window->rel_buf_list->push_back(gbm_buf->idx);
        pthread_mutex_unlock(&p_window->m_mutex);

        //TODO: maintain commit/release timestamp for every posted buffer
        if (p_window->diag != NULL)
        {
            if (p_window->diag->bprint)
            {
                QCARCAM_ERRORMSG("buffer release commit_interval %lu draw %lu transfer %lu total %lu",
                        (p_window->diag->buf_commit_time[TEST_CUR_BUFFER] - p_window->diag->buf_commit_time[TEST_PREV_BUFFER] ),
                        (p_window->diag->buf_rel_time[TEST_CUR_BUFFER] - p_window->diag->buf_commit_time[TEST_PREV_BUFFER]),
                        (p_window->diag->buf_commit_time[TEST_PREV_BUFFER] - p_window->diag->frame_generate_time[TEST_PREV_BUFFER] ),
                        (p_window->diag->buf_rel_time[TEST_CUR_BUFFER] - p_window->diag->frame_generate_time[TEST_PREV_BUFFER] )
                        );
            }
        }
    }

}

static const struct wl_buffer_listener buffer_listener = {
    buffer_release};


static void
gbmcreate_buffer_succeeded(void *data, struct gbm_buffer_params *params,
                                  struct wl_buffer *new_buffer)
{
    struct gbm_buffer *gbm_buf = (struct gbm_buffer *)data;
    if (gbm_buf) {
        gbm_buf->wl_buf = new_buffer;
        gbm_buf->state = GBM_BUF_STATE_INIT_WL;
    }

    gbm_buffer_params_destroy(params);
    QCARCAM_INFOMSG( "gbm_buffer_params_create.create succeed.");
}

static void
gbmcreate_buffer_failed(void *data, struct gbm_buffer_params *params)
{
    struct gbm_buffer *gbm_buf = (struct gbm_buffer *)data;
    if (gbm_buf) {
        gbm_buf->wl_buf = NULL;
    }

    gbm_buffer_params_destroy(params);

    QCARCAM_ERRORMSG( "Error:gbm_buffer_params_create.create failed.");
}

static const struct gbm_buffer_params_listener gbm_params_listener = {
    gbmcreate_buffer_succeeded,
    gbmcreate_buffer_failed
};

static void *helper_thread(void *arg)
{
    int ret;
    pthread_setname_np(pthread_self(), "helper_thread");
    pthread_detach(pthread_self());

    while (!b_post)
    {
        usleep(3000);
    }

    while (!aborted)
    {
        ret = wl_display_dispatch(display_pointer->display);
        if (ret == -1)
        {
            QCARCAM_ERRORMSG("wl_display_dispatch failed");
        }
    }
    return NULL;
}

static unsigned int qcarcam_get_stride(QCarCamColorFmt_e fmt, unsigned int width)
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
    case QCARCAM_FMT_PLAIN16_10:
    case QCARCAM_FMT_PLAIN16_12:
    case QCARCAM_FMT_PLAIN16_14:
    case QCARCAM_FMT_PLAIN16_16:
        return width * 2;
    case QCARCAM_FMT_UYVY_10:
        if (0 == (width % 4))
            stride = width * 2 * 5 / 4;
        break;
    case QCARCAM_FMT_UYVY_12:
        if (0 == (width % 2))
            stride = width * 2 * 3 / 2;
        break;
    case QCARCAM_FMT_NV12:
    case QCARCAM_FMT_NV21:
    case QCARCAM_FMT_YU12:
    case QCARCAM_FMT_YV12:
        stride = width * 3/2;
        break;
    default:
        break;
    }

    return stride;
}

static uint32_t get_gbm_format(uint32_t fmt)
{
    uint32_t gbmfmt = GBM_FORMAT_UYVY;
    switch (fmt)
    {
    case QCARCAM_FMT_UYVY_8:
        gbmfmt = GBM_FORMAT_UYVY;
           break;
    case QCARCAM_FMT_NV12:
        gbmfmt = GBM_FORMAT_NV12;
        break;
    case QCARCAM_FMT_NV21:
        gbmfmt = GBM_FORMAT_NV21;    // GBM doesn't  support NV21 actually
        break;
    case QCARCAM_FMT_YU12:
        gbmfmt = GBM_FORMAT_YUV420;
        break;
    case QCARCAM_FMT_YV12:
        gbmfmt = GBM_FORMAT_YVU420;
        break;
    case QCARCAM_FMT_PLAIN16_12:
        gbmfmt = GBM_FORMAT_UYVY;    // 2bytes per pixel
        break;
    case QCARCAM_FMT_RGB_888:
        gbmfmt = GBM_FORMAT_RGB888;
        break;
    case QCARCAM_FMT_MIPIRAW_8:
        gbmfmt = GBM_FORMAT_RAW8;
        break;
    default:
        QCARCAM_ERRORMSG("unsupported format");
        break;
    }

    return gbmfmt;
}

static uint32_t get_drm_format(uint32_t fmt)
{
    uint32_t drmfmt = DRM_FORMAT_UYVY;
    switch (fmt)
    {
    case QCARCAM_FMT_UYVY_8:
        drmfmt = DRM_FORMAT_UYVY;
        break;
    case QCARCAM_FMT_NV12:
        drmfmt = DRM_FORMAT_NV12;
        break;
    case QCARCAM_FMT_NV21:
        drmfmt = DRM_FORMAT_NV21;   // GBM doesn't  support NV21 actually
        break;
    case QCARCAM_FMT_RGB_888:
        drmfmt = DRM_FORMAT_RGB888;
        break;
    default:
        QCARCAM_ERRORMSG("unsupported format");
        break;
    }

    return drmfmt;
}

static struct display_wl *create_wl_display(void)
{
    struct display_wl *display;

    display = (display_wl*)calloc(1, sizeof(*display));
    if (display == NULL)
    {
        QCARCAM_ERRORMSG("out of memory");
        return NULL;
    }
    display->display = wl_display_connect(NULL);
    if (display->display == NULL)
    {
        QCARCAM_ERRORMSG("wl_display_connect failed.");
        free(display);
        return NULL;
    }

    display->formats = 0;
    display->output_width = 0;
    display->output_height = 0;
    display->registry = wl_display_get_registry(display->display);
    wl_registry_add_listener(display->registry, &wl_registry_lsn, display);
    wl_display_dispatch(display->display);
    wl_display_roundtrip(display->display);
    if (display->shm == NULL)
    {
        QCARCAM_ERRORMSG("No wl_shm global");
        free(display);
        return NULL;
    }

    wl_display_get_fd(display->display);

    return display;
}

static void post_window(test_util_ctxt_t *ctxt, test_util_window_t *p_window, unsigned int idx, bool has_buf_rel_cb)
{
    struct wl_callback *callback =NULL;
    wl_surface_damage(p_window->wl_window->surface, 0, 0, p_window->wl_window->width, p_window->wl_window->height);
    wl_surface_attach(p_window->wl_window->surface, p_window->gbm_buf_wl[idx].wl_buf, 0, 0);

    if (p_window->diag != NULL)
    {
        p_window->diag->buf_commit_time[TEST_PREV_BUFFER] = p_window->diag->buf_commit_time[TEST_CUR_BUFFER];
        test_util_get_time(&p_window->diag->buf_commit_time[TEST_CUR_BUFFER]);
    }

#ifndef USE_GBM_BACKEND
    p_window->wl_window->callback = wl_surface_frame(p_window->wl_window->surface);
    wl_callback_add_listener(p_window->wl_window->callback, &frame_listener, p_window);
#endif

    wl_surface_commit(p_window->wl_window->surface);
    wl_display_flush(ctxt->wl_display->display);

    pthread_mutex_lock(&p_window->m_mutex);
    p_window->commit_frames++;
    pthread_mutex_unlock(&p_window->m_mutex);

    p_window->wl_window->buffer_index = idx;
    p_window->buffers[idx].isdequeue = 0;
    QCARCAM_DBGMSG("Post display for window %p idx %d", p_window, idx);
}

static int set_parent_surface(test_util_ctxt_t *ctxt)
{
    if (ctxt->wl_display->ivi_application)
    {
        uint32_t id_ivisurf = getpid() + IVI_SURFACE_ID + input_num;

        ctxt->parent_ivi_surface = ivi_application_surface_create(
            ctxt->wl_display->ivi_application,
            id_ivisurf, ctxt->parent_surface);

        if (ctxt->parent_ivi_surface == NULL)
        {
            QCARCAM_ERRORMSG("Failed to create ivi_client_surface");
            return QCARCAM_RET_FAILED;
        }
        ivi_surface_add_listener(ctxt->parent_ivi_surface, &ivi_surface_listener, NULL);
        input_num++;
    }
    else if (ctxt->wl_display->wm_base)
    {
        ctxt->parent_xdg_surface = xdg_wm_base_get_xdg_surface(ctxt->wl_display->wm_base, ctxt->parent_surface);
        if (ctxt->parent_xdg_surface == NULL)
        {
            QCARCAM_ERRORMSG("xdg_wm_base_get_xdg_surface failed.");
            return QCARCAM_RET_FAILED;
        }
        xdg_surface_add_listener(ctxt->parent_xdg_surface, &xdg_surface_listener, ctxt);
        ctxt->wait_for_configure = true;
        ctxt->parent_xdg_toplevel = xdg_surface_get_toplevel(ctxt->parent_xdg_surface);
        xdg_toplevel_add_listener(ctxt->parent_xdg_toplevel, &xdg_toplevel_listener, ctxt);
        xdg_toplevel_set_fullscreen(ctxt->parent_xdg_toplevel, NULL);
        if (ctxt->wait_for_configure)
        {
            wl_display_roundtrip(ctxt->wl_display->display);
        }
        ctxt->parent_callback = wl_surface_frame(ctxt->parent_surface);
    }
    else
    {
        QCARCAM_ERRORMSG("display->shell NULL.");
        return QCARCAM_RET_FAILED;
    }

    ctxt->parent_set = 1;

    return QCARCAM_RET_OK;
}

static int  post_parent_surface(test_util_ctxt_t *ctxt)
{
    wl_surface_attach(ctxt->parent_surface, ctxt->parent_gbm_buf->wl_buf, 0, 0);
    wl_surface_damage(ctxt->parent_surface, 0, 0, ctxt->parent_size[0], ctxt->parent_size[1]);
    wl_surface_commit(ctxt->parent_surface);

    return QCARCAM_RET_OK;
}

static int create_parent_buffer(test_util_ctxt_t *ctxt, QCarCamColorFmt_e format, uint32 window_flags)
{
    int stride = qcarcam_get_stride(format, ctxt->wl_display->output_width);
    unsigned int length = stride * ctxt->wl_display->output_height;
    int fd;
    int rc = -1;
    struct wl_shm_pool *pool;
    struct gbm_bo *bo = NULL;
    uint32_t gbmfmt = get_gbm_format(format);
    uint32 flags = GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING;
    struct gbm_buffer *parent_gbm_buf = (gbm_buffer *)calloc(1, sizeof(*parent_gbm_buf));
    if (!parent_gbm_buf)
    {
        QCARCAM_ERRORMSG("Failed to allocate parent_gbm_buf");
        return QCARCAM_RET_NOMEM;
    }
#if 0 //@todo
    if (window_flags & QCARCAM_INPUT_FLAG_CONTENT_PROTECTED)
    {
        flags |= GBM_BO_USAGE_PROTECTED_QTI;
        parent_gbm_buf->is_secure = 1;
    }
#endif
    bo = gbm_bo_create(ctxt->gbm, ctxt->wl_display->output_width, ctxt->wl_display->output_height, gbmfmt, flags);
    if (bo == NULL)
    {
        QCARCAM_ERRORMSG("Failed to gbm_bo_create");
        return QCARCAM_RET_FAILED;
    }
    parent_gbm_buf->bo = bo;

#ifdef USE_GBM_BACKEND
    fd = gbm_bo_get_fd(bo);
#else
    fd = bo->ion_fd;
#endif
    QCARCAM_INFOMSG("fd %d", fd);
    if (fd < 0)
    {
        QCARCAM_ERRORMSG("Failed to gbm_bo_get_fd");
        return QCARCAM_RET_FAILED;
    }
    parent_gbm_buf->fd = fd;

    parent_gbm_buf->buflen = length;
    if (!parent_gbm_buf->is_secure)
    {
        parent_gbm_buf->p_data = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        memset(parent_gbm_buf->p_data, BACKGROUND_COLOR_GRAY, parent_gbm_buf->buflen);
    }
    parent_gbm_buf->state = GBM_BUF_STATE_INIT_BUF;

    ctxt->parent_gbm_buf = parent_gbm_buf;

    ctxt->parent_size[0] = ctxt->wl_display->output_width;
    ctxt->parent_size[1] = ctxt->wl_display->output_height;

#if USE_GBM_BACKEND
    rc = test_util_create_uyvy_wlbuffer_gbm(ctxt, NULL, 0, ctxt->wl_display->output_width, ctxt->wl_display->output_height);
#else
    rc = test_util_create_uyvy_wlbuffer(ctxt, NULL, 0, ctxt->wl_display->output_width, ctxt->wl_display->output_height);
#endif
    if (rc){
        QCARCAM_ERRORMSG("test_util_create_uyvy_wlbuffer(parent) failed");
        return QCARCAM_RET_FAILED;
    }

    return QCARCAM_RET_OK;
}

static boolean isYUVformat(QCarCamColorFmt_e fmt)
{
    QCarCamColorPattern_e pattern = QCARCAM_COLOR_GET_PATTERN(fmt);
    if (pattern >= QCARCAM_YUV_YUYV && pattern < QCARCAM_BAYER_GBRG)
    {
        return TRUE;
    }

    return FALSE;
}

static boolean isSupportedFormat(QCarCamColorFmt_e fmt)
{
    switch (fmt)
    {
    case QCARCAM_FMT_UYVY_8:
    case QCARCAM_FMT_PLAIN16_12:
    case QCARCAM_FMT_MIPIRAW_12:
    case QCARCAM_FMT_NV12:
    case QCARCAM_FMT_NV21:
    case QCARCAM_FMT_YU12:
    case QCARCAM_FMT_YV12:
    case QCARCAM_FMT_RGB_888:
    case QCARCAM_FMT_MIPIRAW_8:
        return TRUE;
    default:
        break;
    }

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_init
///
/// @brief Initialize context that is to be used to display content on the screen.
///
/// @param ctxt   Pointer to context to be initialized
/// @param params Parameters to init ctxt
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_init(test_util_ctxt_t **ctxt, test_util_ctxt_params_t *params)
{
    test_util_ctxt_t* pCtxt;

    *ctxt = NULL;

    if (!params)
    {
        return QCARCAM_RET_BADPARAM;
    }

    pCtxt = (test_util_ctxt_t*)calloc(1, sizeof(struct test_util_ctxt_t));
    if (!pCtxt)
    {
        return QCARCAM_RET_NOMEM;
    }

    pCtxt->parent_set = 0;
    pCtxt->params = *params;

    if (!pCtxt->params.disable_display)
    {
        int rc = 0;
        pthread_t pthread_object;
        pthread_attr_t pthread_attr;
        int major = 0, minor = 0;

        pCtxt->wl_display = create_wl_display();
        display_pointer = pCtxt->wl_display;

        if (!pCtxt->wl_display)
        {
            QCARCAM_ERRORMSG("create_wl_display failed");
            return QCARCAM_RET_FAILED;
        }

#ifndef USE_GBM_BACKEND
        pCtxt->egldpy = eglGetDisplay((EGLNativeDisplayType) display_pointer->display);
        if (!pCtxt->egldpy) {
            QCARCAM_ERRORMSG("eglGetDisplay failed\n");
            return QCARCAM_RET_FAILED;
        }

        if (!eglInitialize(pCtxt->egldpy, &major, &minor)) {
            QCARCAM_ERRORMSG("eglInitialize failed\n");
            pCtxt->egldpy = EGL_NO_DISPLAY;
            return QCARCAM_RET_FAILED;
        }
        QCARCAM_ERRORMSG("egl dsp %p\n", pCtxt->egldpy);
#endif

        pCtxt->parent_surface = wl_compositor_create_surface(pCtxt->wl_display->compositor);
        if (pCtxt->parent_surface == NULL)
        {
            QCARCAM_ERRORMSG("create parent_surface failed\n");
            return QCARCAM_RET_FAILED;
        }

        rc = pthread_attr_init(&pthread_attr);
        if (rc)
        {
            QCARCAM_ERRORMSG("pthread_attr_init failed");
            return QCARCAM_RET_FAILED;
        }

        /*signal handler thread*/
        rc = pthread_create(&pthread_object, &pthread_attr, &helper_thread, NULL);
        if (rc)
        {
            QCARCAM_ERRORMSG("pthread_create failed");
            return QCARCAM_RET_FAILED;
        }
    }

    pCtxt->drm_fd = open("/dev/dri/renderD128", O_RDWR | O_CLOEXEC);
    if (pCtxt->drm_fd < 0) {
        QCARCAM_ERRORMSG("/dev/dri/renderD128 open failed: %d\n", pCtxt->drm_fd);
        return QCARCAM_RET_FAILED;
    }

    pCtxt->gbm = gbm_create_device(pCtxt->drm_fd);
    if (pCtxt->gbm == NULL)
    {
        QCARCAM_ERRORMSG("gbm_create_device failed\n");
        return QCARCAM_RET_FAILED;
    }

    *ctxt = pCtxt;

    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_deinit
///
/// @brief Destroy context and free memory.
///
/// @param ctxt   Pointer to context to be destroyed
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_deinit(test_util_ctxt_t *ctxt)
{
    if (ctxt)
    {
        if (ctxt->parent_gbm_buf)
        {
            if (ctxt->parent_gbm_buf->wl_buf)
            {
                wl_buffer_destroy(ctxt->parent_gbm_buf->wl_buf);
                ctxt->parent_gbm_buf->wl_buf = NULL;
            }
            if (ctxt->parent_gbm_buf->p_data)
            {
                munmap(ctxt->parent_gbm_buf->p_data, ctxt->parent_gbm_buf->buflen);
                ctxt->parent_gbm_buf->p_data = NULL;
            }
            if (-1 < ctxt->parent_gbm_buf->fd)
            {
#ifdef USE_GBM_BACKEND
                close(ctxt->parent_gbm_buf->fd);
#endif
            }
            if (ctxt->parent_gbm_buf->bo)
            {
                gbm_bo_destroy(ctxt->parent_gbm_buf->bo);
                ctxt->parent_gbm_buf->bo = NULL;
            }
            free(ctxt->parent_gbm_buf);
            ctxt->parent_gbm_buf = NULL;
        }

        if (ctxt->parent_callback)
        {
            wl_callback_destroy(ctxt->parent_callback);
        }
        if (ctxt->parent_xdg_toplevel)
        {
            xdg_toplevel_destroy(ctxt->parent_xdg_toplevel);
            ctxt->parent_xdg_toplevel  = NULL;
        }
        if (ctxt->parent_xdg_surface)
        {
            xdg_surface_destroy(ctxt->parent_xdg_surface);
            ctxt->parent_xdg_surface = NULL;
        }
        if (ctxt->parent_surface)
        {
            wl_surface_destroy(ctxt->parent_surface);
        }

        if (ctxt->gbm ){
            gbm_device_destroy(ctxt->gbm);
            ctxt->gbm = NULL;
        }
        close(ctxt->drm_fd);
        if (!ctxt->params.disable_display)
        {
#ifndef USE_GBM_BACKEND
            eglTerminate(ctxt->egldpy);
#endif
            if (ctxt->wl_display->shm)
            {
                wl_shm_destroy(ctxt->wl_display->shm);
            }

            if (ctxt->wl_display->ivi_application)
            {
                ivi_application_destroy(ctxt->wl_display->ivi_application);
                ctxt->wl_display->ivi_application = NULL;
            }

            if (ctxt->wl_display->compositor)
            {
                wl_compositor_destroy(ctxt->wl_display->compositor);
            }

            if (ctxt->wl_display->viewport)
            {
                wl_viewport_destroy(ctxt->wl_display->viewport);
                ctxt->wl_display->viewport = NULL;
            }

            if (ctxt->wl_display->scaler)
            {
                wl_scaler_destroy(ctxt->wl_display->scaler);
                ctxt->wl_display->scaler = NULL;
            }

            if (ctxt->wl_display->viewporter)
            {
                wp_viewporter_destroy(ctxt->wl_display->viewporter);
                ctxt->wl_display->viewporter = NULL;
            }

            if (ctxt->wl_display->subcompositor)
            {
                wl_subcompositor_destroy(ctxt->wl_display->subcompositor);
            }

            wl_registry_destroy(ctxt->wl_display->registry);
            wl_display_flush(ctxt->wl_display->display);
            wl_display_disconnect(ctxt->wl_display->display);
            free(ctxt->wl_display);
            ctxt->wl_display = NULL;
        }

        free(ctxt);
    }

    aborted = 1;
    b_post = true;

    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_init_window
///
/// @brief Initialize new window
///
/// @param ctxt             Pointer to util context
/// @param pp_window        Pointer to new window to be initialized
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_init_window(test_util_ctxt_t *ctxt, test_util_window_t **pp_window)
{
    int rc = 0;
    *pp_window = NULL;
    test_util_window_t* p_window = NULL;

    p_window = (test_util_window_t*)calloc(1, sizeof(struct test_util_window_t));
    if (!p_window)
    {
        return QCARCAM_RET_NOMEM;
    }

    if (!ctxt->params.disable_display)
    {
        p_window->wl_window = (window_wl *)calloc(1, sizeof(*p_window->wl_window));
        if (!p_window->wl_window)
        {
            free(p_window);
            return QCARCAM_RET_FAILED;
        }
        p_window->commit_frames = 0;
        p_window->wl_window->buffer_index = -1;

        p_window->rel_buf_list = new std::list<uint32>();
        if (!p_window->rel_buf_list)
        {
            free(p_window->wl_window);
            free(p_window);
            return QCARCAM_RET_FAILED;
        }

        if (pthread_mutex_init(&p_window->m_mutex, 0))
        {
            delete p_window->rel_buf_list;
            free(p_window->wl_window);
            free(p_window);
            return QCARCAM_RET_FAILED;
        }
    }

    *pp_window = p_window;

    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_deinit_window
///
/// @brief Destroy window
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_deinit_window(test_util_ctxt_t *ctxt, test_util_window_t *p_window)
{
    int rc = 0;

    if (!ctxt->params.disable_display && p_window)
    {
        if (p_window->rel_buf_list)
            delete p_window->rel_buf_list;

        if (p_window->wl_window)
        {
            if (p_window->wl_window->xdg_toplevel)
            {
                xdg_toplevel_destroy(p_window->wl_window->xdg_toplevel);
                p_window->wl_window->xdg_toplevel = NULL;
            }

            if (p_window->wl_window->xdg_surface)
            {
                xdg_surface_destroy(p_window->wl_window->xdg_surface);
                p_window->wl_window->xdg_surface = NULL;
            }

            if (p_window->wl_window->ivi_surface)
            {
                ivi_surface_destroy(p_window->wl_window->ivi_surface);
                p_window->wl_window->ivi_surface = NULL;
            }

            if (p_window->wl_window->viewport_wp)
            {
                wp_viewport_destroy(p_window->wl_window->viewport_wp);
                p_window->wl_window->viewport_wp = NULL;
            }

            if (p_window->wl_window->subsurface)
            {
                wl_subsurface_destroy(p_window->wl_window->subsurface);
            }

            if (p_window->wl_window->surface)
            {
                wl_surface_destroy(p_window->wl_window->surface);
            }

            if (p_window->wl_window->subviewport)
            {
                wl_viewport_destroy(p_window->wl_window->subviewport);
                p_window->wl_window->subviewport = NULL;
            }

            free(p_window->wl_window);
            p_window->wl_window = NULL;
        }

        rc = pthread_mutex_destroy(&p_window->m_mutex);
    }

    if (p_window)
        free(p_window);

    return (rc == 0) ? QCARCAM_RET_OK : QCARCAM_RET_FAILED;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_set_window_param
///
/// @brief Send window parameters to display
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
/// @param window_params    Pointer to structure with window properties
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_set_window_param(test_util_ctxt_t *ctxt, test_util_window_t *p_window, test_util_window_param_t *window_params)
{
    p_window->flags = window_params->flags;
    p_window->is_imported_buffer = window_params->is_imported_buffer;

    if (!ctxt->params.disable_display)
    {
        struct wl_region *region;

        if ((ctxt->params.enable_c2d && p_window->format != QCARCAM_FMT_UYVY_8) ||
                !ctxt->params.enable_c2d)
        {
            p_window->buffer_size[0] = window_params->buffer_size[0];
            p_window->buffer_size[1] = window_params->buffer_size[1];
            p_window->src_rec.src_width  = p_window->buffer_size[0] * window_params->window_source_size[0];
            p_window->src_rec.src_height = p_window->buffer_size[1] * window_params->window_source_size[1];
            p_window->src_rec.src_x = p_window->buffer_size[0] * window_params->window_source_pos[0];
            p_window->src_rec.src_y = p_window->buffer_size[1] * window_params->window_source_pos[1];
            p_window->wl_window->callback = NULL;
            p_window->wl_window->display = ctxt->wl_display;
#ifndef USE_GBM_BACKEND
            p_window->wl_window->egldpy = ctxt->egldpy;
#endif
            p_window->wl_window->width = ctxt->wl_display->output_width * window_params->window_size[0];
            p_window->wl_window->height = ctxt->wl_display->output_height * window_params->window_size[1];
            p_window->wl_window->pos_x = ctxt->wl_display->output_width * window_params->window_pos[0];
            p_window->wl_window->pos_y = ctxt->wl_display->output_height * window_params->window_pos[1];
            p_window->wl_window->fullscreen = 0;
            p_window->wl_window->surface = wl_compositor_create_surface(ctxt->wl_display->compositor);
            p_window->wl_window->subsurface = wl_subcompositor_get_subsurface(ctxt->wl_display->subcompositor,
                                                                                p_window->wl_window->surface,
                                                                                ctxt->parent_surface);
            if (p_window->wl_window->subsurface == NULL)
            {
                QCARCAM_ERRORMSG("wl_subcompositor_get_subsurface failed.");
                return QCARCAM_RET_FAILED;
            }
            wl_subsurface_set_position(p_window->wl_window->subsurface, p_window->wl_window->pos_x, p_window->wl_window->pos_y);
            wl_subsurface_set_desync(p_window->wl_window->subsurface);

            if (ctxt->wl_display->viewporter)
            {
                p_window->wl_window->viewport_wp = wp_viewporter_get_viewport(ctxt->wl_display->viewporter, p_window->wl_window->surface);
                if (p_window->wl_window->width != p_window->buffer_size[0] || p_window->wl_window->height != p_window->buffer_size[1])
                {
                    wp_viewport_set_destination(p_window->wl_window->viewport_wp, p_window->wl_window->width, p_window->wl_window->height);
                }
                /* we have to scale to fullscreen manually because of the ratio of one field resolution on BOB 60fps case */
                if (ctxt->params.enable_di == TESTUTIL_DEINTERLACE_SW_BOB)
                {
                    wp_viewport_set_destination(p_window->wl_window->viewport_wp, ctxt->wl_display->output_width, ctxt->wl_display->output_height);
                }
            }
            else if (ctxt->wl_display->scaler)
            {
                p_window->wl_window->subviewport = wl_scaler_get_viewport(ctxt->wl_display->scaler, p_window->wl_window->surface);
                if (p_window->wl_window->width != p_window->buffer_size[0] || p_window->wl_window->height != p_window->buffer_size[1])
                {
                    wl_viewport_set_destination(p_window->wl_window->subviewport, p_window->wl_window->width, p_window->wl_window->height);
                }

                /* we have to scale to fullscreen manually because of the ratio of one field resolution on BOB 60fps case */
                if (ctxt->params.enable_di == TESTUTIL_DEINTERLACE_SW_BOB)
                {// TODO the viewport is not be initialized, need to change it to subviewport ???
                    wl_viewport_set_destination(ctxt->wl_display->viewport, ctxt->wl_display->output_width, ctxt->wl_display->output_height);
                }
            }

            wl_surface_damage(p_window->wl_window->surface, 0, 0, p_window->wl_window->width, p_window->wl_window->height);
            region = wl_compositor_create_region(p_window->wl_window->display->compositor);
            wl_region_add(region, 0, 0, p_window->wl_window->width, p_window->wl_window->height);
            wl_surface_set_opaque_region(p_window->wl_window->surface, region);
            wl_region_destroy(region);
        }
    }

    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_init_window_buffers
///
/// @brief Initialize buffers for display
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
/// @param buffers          Pointer to qcarcam buffers
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_init_window_buffers(test_util_ctxt_t *ctxt, test_util_window_t *p_window, QCarCamBufferList_t *buffers)
{
    unsigned int length = 0;
    int fd;
    struct gbm_bo *bo = NULL;
    int rc = -1;
    int bpp = 4;
    uint32_t align_w;
    uint32_t align_h;
    struct gbm_buf_info bufInfo;
    uint32_t gbmfmt = GBM_FORMAT_UYVY;

    struct wl_shm_pool *pool;

    p_window->buffer_size[0] = buffers->pBuffers[0].planes[0].width;
    p_window->buffer_size[1] = buffers->pBuffers[0].planes[0].height;

    p_window->format = buffers->colorFmt;
    gbmfmt = get_gbm_format(buffers->colorFmt);

    p_window->n_buffers = buffers->nBuffers;

    p_window->buffers = (test_util_buffer_t*)calloc(p_window->n_buffers, sizeof(*p_window->buffers));
    if (!p_window->buffers)
    {
        QCARCAM_ERRORMSG("Failed to allocate buffers structure");
        return QCARCAM_RET_NOMEM;
    }

    if (isSupportedFormat(buffers->colorFmt))
    {
        uint32 flags = GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING;
        struct gbm_buffer *gbm_buf = (gbm_buffer*)calloc(buffers->nBuffers, sizeof(*gbm_buf));
        if (!gbm_buf)
        {
            QCARCAM_ERRORMSG("Failed to allocate gbm_buf");
            return QCARCAM_RET_NOMEM;
        }
#if 0 //@todo
        if (p_window->flags & QCARCAM_INPUT_FLAG_CONTENT_PROTECTED)
        {
            flags |= GBM_BO_USAGE_PROTECTED_QTI;
            gbm_buf->is_secure = 1;
        }
#endif
        for (int i = 0; i < buffers->nBuffers; i++)
        {
            if (p_window->is_imported_buffer && buffers->pBuffers[i].planes[0].memHndl != 0)
            {
                //Request GBM to import buffer
                fd = (int)buffers->pBuffers[i].planes[0].memHndl;

                struct gbm_import_fd_data buf_data;
                buf_data.fd = fd;
                buf_data.width = buffers->pBuffers[i].planes[0].width;
                buf_data.height = buffers->pBuffers[i].planes[0].height;
                buf_data.format = gbmfmt;

                bo = gbm_bo_import(ctxt->gbm, GBM_BO_IMPORT_FD_ALLOC_META, &buf_data, flags);
                if (NULL == bo)
                {
                    QCARCAM_ERRORMSG("Failed to import gbm bo");
                    return QCARCAM_RET_FAILED;
                }
            }
            else
            {
                //Need GBM to allocate buffer
                bo = gbm_bo_create(ctxt->gbm, buffers->pBuffers[i].planes[0].width, buffers->pBuffers[i].planes[0].height, gbmfmt, flags);
                if (bo == NULL)
                {
                    QCARCAM_ERRORMSG("Failed to gbm_bo_create %u", gbmfmt);
                    return QCARCAM_RET_FAILED;
                }

#ifdef USE_GBM_BACKEND
                fd = gbm_bo_get_fd(bo);
#else
                fd = bo->ion_fd;
#endif
                QCARCAM_INFOMSG("fd %d", fd);
                if (fd < 0)
                {
                    QCARCAM_ERRORMSG("Failed to gbm_bo_get_fd");
                    return QCARCAM_RET_FAILED;
                }

                buffers->pBuffers[i].planes[0].memHndl = (uint64_t)fd;
            }

            gbm_perform(GBM_PERFORM_GET_BO_ALIGNED_WIDTH, bo, &align_w);
            gbm_perform(GBM_PERFORM_GET_BO_ALIGNED_HEIGHT, bo, &align_h);

            p_window->stride = qcarcam_get_stride(buffers->colorFmt, align_w);

            buffers->pBuffers[i].numPlanes = 1;
            buffers->pBuffers[i].planes[0].stride = qcarcam_get_stride((QCarCamColorFmt_e)p_window->format, align_w);
            buffers->pBuffers[i].planes[0].size = buffers->pBuffers[i].planes[0].height * buffers->pBuffers[i].planes[0].stride;

            bufInfo.width = p_window->buffer_size[0];
            bufInfo.height = p_window->buffer_size[1];
            bufInfo.format = gbmfmt;

            rc = gbm_perform(GBM_PERFORM_GET_BUFFER_SIZE_DIMENSIONS, &bufInfo, 0, &align_w, &align_h, &length);
            if (GBM_ERROR_NONE != rc)
            {
                QCARCAM_ERRORMSG("Failed to GBM_PERFORM_GET_BUFFER_SIZE_DIMENSIONS");
                return QCARCAM_RET_FAILED;
            }

            if (isYUVformat(buffers->colorFmt))
            { // get the YUV format information by gbm
                generic_buf_layout_t layout = {};
                rc = gbm_perform(GBM_PERFORM_GET_YUV_PLANE_INFO, bo, &layout);
                if (GBM_ERROR_NONE != rc)
                {
                    QCARCAM_ERRORMSG("Failed to get plane info");
                    return QCARCAM_RET_FAILED;
                }
                else
                {
                     buffers->pBuffers[i].numPlanes = layout.num_planes;
                }

                switch (layout.num_planes)
                {
                case 3:
                    buffers->pBuffers[i].planes[2].stride = layout.planes[2].v_increment;
                    buffers->pBuffers[i].planes[1].stride = layout.planes[1].v_increment;
                    buffers->pBuffers[i].planes[0].stride = layout.planes[0].v_increment;

                    buffers->pBuffers[i].planes[2].size = length - layout.planes[2].offset;
                    buffers->pBuffers[i].planes[1].size = layout.planes[2].offset - layout.planes[1].offset;
                    buffers->pBuffers[i].planes[0].size = layout.planes[1].offset - layout.planes[0].offset;
                    QCARCAM_INFOMSG("plane2 size2 %u stride2 %u size1 %u stride1 %u size0 %u stride0 %u",
                        buffers->pBuffers[i].planes[2].size, buffers->pBuffers[i].planes[2].stride,
                        buffers->pBuffers[i].planes[1].size, buffers->pBuffers[i].planes[1].stride,
                        buffers->pBuffers[i].planes[0].size, buffers->pBuffers[i].planes[0].stride);
                    break;
                case 2:
                    buffers->pBuffers[i].planes[1].stride = layout.planes[1].v_increment;
                    buffers->pBuffers[i].planes[0].stride = layout.planes[0].v_increment;

                    buffers->pBuffers[i].planes[1].size = length - layout.planes[1].offset;
                    buffers->pBuffers[i].planes[0].size = layout.planes[1].offset - layout.planes[0].offset;

                    QCARCAM_INFOMSG("size1 %u stride1 %u size0 %u stride0 %u",
                        buffers->pBuffers[i].planes[1].size, buffers->pBuffers[i].planes[1].stride,
                        buffers->pBuffers[i].planes[0].size, buffers->pBuffers[i].planes[0].stride);
                    break;
                case 1:
                    buffers->pBuffers[i].planes[0].stride = layout.planes[0].v_increment;
                    buffers->pBuffers[i].planes[0].size = length;
                    QCARCAM_INFOMSG("size0 %u stride0 %u",
                        buffers->pBuffers[i].planes[0].size, buffers->pBuffers[i].planes[0].stride);
                    break;
                default:
                    QCARCAM_ERRORMSG("plane number %d, should be 1/2/3", layout.num_planes);
                    return QCARCAM_RET_FAILED;
                }
            }


            QCARCAM_INFOMSG("plane_num %u w %u h %u aign_w %u align_h %u stride %u length %u size %u",
                    buffers->pBuffers[i].numPlanes,
                    buffers->pBuffers[i].planes[0].width,
                    buffers->pBuffers[i].planes[0].height,
                    align_w, align_h,
                    buffers->pBuffers[i].planes[0].stride, length, buffers->pBuffers[i].planes[0].size);

            gbm_buf[i].bo = bo;
            gbm_buf[i].fd = fd;
            gbm_buf[i].buflen = length;

            if (!gbm_buf->is_secure)
            {
                gbm_buf[i].p_data = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            }

            gbm_buf[i].state = GBM_BUF_STATE_INIT_BUF;

            p_window->buffers[i].ptr = (void *)(uintptr_t)buffers->pBuffers[i].planes[0].memHndl;
            p_window->buffers[i].isdequeue = 1;
        }
        p_window->gbm_buf = gbm_buf;

        if (!ctxt->params.enable_c2d && !ctxt->params.disable_display)
        {
            struct gbm_buffer *gbm_buf_wl = (gbm_buffer *)calloc(buffers->nBuffers, sizeof(*gbm_buf_wl));
            if (!gbm_buf_wl)
            {
                QCARCAM_ERRORMSG("Failed to allocate gbmbuf_wl");
                return QCARCAM_RET_NOMEM;
            }
            memset(gbm_buf_wl, 0, sizeof(gbm_buffer) * buffers->nBuffers);
            for (int i = 0; i < buffers->nBuffers; i++)
            {
                gbm_buf_wl[i].fd = -1;
            }

            p_window->gbm_buf_wl = gbm_buf_wl;

            if ((ctxt->params.enable_di == 0) ||
                (ctxt->params.enable_di == TESTUTIL_DEINTERLACE_SW_BOB))
            {
                for (int i = 0; i < buffers->nBuffers; i++)
                {
#if USE_GBM_BACKEND
                    rc = test_util_create_uyvy_wlbuffer_gbm(ctxt, p_window, i,
                            buffers->pBuffers[0].planes[0].width,
                            buffers->pBuffers[0].planes[0].height);
#else
                    rc = test_util_create_uyvy_wlbuffer(ctxt, p_window, i,
                            buffers->pBuffers[0].planes[0].width,
                            buffers->pBuffers[0].planes[0].height);
#endif
                    if (rc)
                    {
                        QCARCAM_ERRORMSG("test_util_create_uyvy_wlbuffer failed");
                    }
                }

                QCARCAM_INFOMSG("window buffer %p", p_window);
            }
        }
    }
    else
    {
        if (!ctxt->params.disable_display)
        {
            uint32 flags = GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING;
            struct gbm_buffer *gbm_buf_wl = (gbm_buffer *)calloc(buffers->nBuffers, sizeof(*gbm_buf_wl));
            if (!gbm_buf_wl)
            {
                QCARCAM_ERRORMSG("Failed to allocate gbm_buf_wl");
                return QCARCAM_RET_NOMEM;
            }
#if 0 //@todo
            if (p_window->flags & QCARCAM_INPUT_FLAG_CONTENT_PROTECTED)
            {
                flags |= GBM_BO_USAGE_PROTECTED_QTI;
                gbm_buf_wl->is_secure = 1;
            }
#endif
            for (int i = 0; i < buffers->nBuffers; i++)
            {
                length = p_window->wl_window->width * p_window->wl_window->height * bpp;

                if (p_window->is_imported_buffer && buffers->pBuffers[i].planes[0].memHndl != 0)
                {
                    //Request GBM to import buffer
                    fd = (int)buffers->pBuffers[i].planes[0].memHndl;

                    struct gbm_import_fd_data buf_data;
                    buf_data.fd = fd;
                    buf_data.width = buffers->pBuffers[i].planes[0].width;
                    buf_data.height = buffers->pBuffers[i].planes[0].height;
                    buf_data.format = GBM_FORMAT_UYVY;

                    bo = gbm_bo_import(ctxt->gbm, GBM_BO_IMPORT_FD_ALLOC_META, &buf_data, flags);
                    if (NULL == bo)
                    {
                        QCARCAM_ERRORMSG("Failed to import gbm bo");
                        return QCARCAM_RET_FAILED;
                    }
                }
                else
                {
                    //Need GBM to allocate buffer
                    bo = gbm_bo_create(ctxt->gbm, buffers->pBuffers[i].planes[0].width, buffers->pBuffers[i].planes[0].height, GBM_FORMAT_UYVY, flags);
                    if (bo == NULL)
                    {
                        QCARCAM_ERRORMSG("Failed to gbm_bo_create %u", gbmfmt);
                        return QCARCAM_RET_FAILED;
                    }

#ifdef USE_GBM_BACKEND
                    fd = gbm_bo_get_fd(bo);
#else
                    fd = bo->ion_fd;
#endif
                    QCARCAM_INFOMSG("fd %d", fd);
                    if (fd < 0)
                    {
                        QCARCAM_ERRORMSG("Failed to gbm_bo_get_fd");
                        return QCARCAM_RET_FAILED;
                    }

                    buffers->pBuffers[i].planes[0].memHndl = (uint64_t)fd;
                }


                gbm_buf_wl[i].bo = bo;
                gbm_buf_wl[i].buflen = length;
                gbm_buf_wl[i].fd = fd;

                if (!gbm_buf_wl->is_secure)
                {
                   gbm_buf_wl[i].p_data = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                }

                pool = wl_shm_create_pool(p_window->wl_window->display->shm, fd, length);
                gbm_buf_wl[i].wl_buf = wl_shm_pool_create_buffer(pool, 0, p_window->wl_window->width, p_window->wl_window->height, p_window->wl_window->width * bpp, support_format);
                wl_buffer_add_listener(gbm_buf_wl[i].wl_buf, &buffer_listener, &gbm_buf_wl[i]);
                wl_shm_pool_destroy(pool);

                p_window->buffers[i].wl_ptr = (void *)(uintptr_t)fd;
            }
            p_window->gbm_buf_wl = gbm_buf_wl;
        }
    }

    if (!ctxt->params.disable_display)
    {
        if (!ctxt->parent_set)
        {
            rc = create_parent_buffer(ctxt, PARENT_SURFACE_COLOR_FOMAT,p_window->flags);
            if (rc)
            {
                QCARCAM_ERRORMSG("create_parent_buffer failed");
                return QCARCAM_RET_FAILED;
            }

            rc = set_parent_surface(ctxt);
            if (rc)
            {
                QCARCAM_ERRORMSG("set_parent_surface failed");
                return QCARCAM_RET_FAILED;
            }
        }
        rc = post_parent_surface(ctxt);
        if (rc)
        {
            QCARCAM_ERRORMSG("post_parent_surface failed");
            return QCARCAM_RET_FAILED;
        }
    }

    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_deinit_window_buffer
///
/// @brief Destroy window buffers
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_deinit_window_buffer(test_util_ctxt_t *ctxt, test_util_window_t *p_window)
{
    if (p_window->buffers)
    {
        free(p_window->buffers);
        p_window->buffers = NULL;
    }

    if (p_window->gbm_buf_wl)
    {
        for (int i = 0; i < p_window->n_buffers; i++)
        {
            if (p_window->gbm_buf_wl[i].wl_buf)
            {
                wl_buffer_destroy(p_window->gbm_buf_wl[i].wl_buf);
                p_window->gbm_buf_wl[i].wl_buf = NULL;
            }

            if (p_window->gbm_buf_wl[i].p_data)
            {
                munmap(p_window->gbm_buf_wl[i].p_data,p_window->gbm_buf_wl[i].buflen);
                p_window->gbm_buf_wl[i].p_data = NULL;
            }

            if (!p_window->is_imported_buffer && -1 < p_window->gbm_buf_wl[i].fd)
            {
#ifdef USE_GBM_BACKEND
                close(p_window->gbm_buf_wl[i].fd);
#endif
            }

            if (p_window->gbm_buf_wl[i].bo)
            {
                gbm_bo_destroy(p_window->gbm_buf_wl[i].bo);
                p_window->gbm_buf_wl[i].bo = NULL;
            }
        }
        free(p_window->gbm_buf_wl);
        p_window->gbm_buf_wl = NULL;
    }

    if (p_window->gbm_buf)
    {
        for (int i = 0; i < p_window->n_buffers; i++)
        {
            if (p_window->gbm_buf[i].p_data)
            {
                munmap(p_window->gbm_buf[i].p_data, p_window->gbm_buf[i].buflen);
                p_window->gbm_buf[i].p_data = NULL;
            }

            if (!p_window->is_imported_buffer && -1 < p_window->gbm_buf[i].fd)
            {
#ifdef USE_GBM_BACKEND
                close(p_window->gbm_buf[i].fd);
#endif
            }

            if (p_window->gbm_buf[i].bo)
            {
                gbm_bo_destroy(p_window->gbm_buf[i].bo);
                p_window->gbm_buf[i].bo = NULL;
            }

        }
        free(p_window->gbm_buf);
        p_window->gbm_buf = NULL;
    }


    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_post_window_buffer
///
/// @brief Send frame to display
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
/// @param idx              Buffer index
/// @param p_rel_buf_idx    List to fill with buffers ready to release
/// @param field_type       Field type in current frame buffer if interlaced
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_post_window_buffer(test_util_ctxt_t *ctxt, test_util_window_t *p_window,
    unsigned int idx, std::list<uint32>* p_rel_buf_idx, QCarCamInterlaceField_e field_type)
{
    b_post = true;

    if (!ctxt->params.disable_display)
    {
        QCarCamRet_e ret = QCARCAM_RET_OK;
        uint32 prev_buf_idx = p_window->wl_window->buffer_index;

        if (!p_window->buffers[idx].isdequeue)
        {
            QCARCAM_ALWZMSG("Have submitted this buffer %u", idx);
        }
        else
        {
            struct wp_viewport* p_viewport = p_window->wl_window->viewport_wp;

            if (ctxt->params.enable_di == TESTUTIL_DEINTERLACE_SW_BOB)
            {
                if (field_type == QCARCAM_INTERLACE_FIELD_ODD)
                {
                    // offset to odd field
                    wp_viewport_set_source(p_viewport,
                            wl_fixed_from_int(0),
                            wl_fixed_from_int(DEINTERLACE_ODD_HEADER_HEIGHT),
                            wl_fixed_from_int(720),
                            wl_fixed_from_int(DEINTERLACE_FIELD_HEIGHT));

                    post_window(ctxt, p_window, idx, 1);
                }
                else if (field_type == QCARCAM_INTERLACE_FIELD_EVEN)
                {
                    // offset to even field
                    wp_viewport_set_source(p_viewport,
                            wl_fixed_from_int(0),
                            wl_fixed_from_int(DEINTERLACE_EVEN_HEADER_HEIGHT),
                            wl_fixed_from_int(720),
                            wl_fixed_from_int(DEINTERLACE_FIELD_HEIGHT));

                    post_window(ctxt, p_window, idx, 1);
                }
                else if (field_type == QCARCAM_INTERLACE_FIELD_ODD_EVEN)
                {
                    // offset to odd field
                    wp_viewport_set_source(p_viewport,
                            wl_fixed_from_int(0),
                            wl_fixed_from_int(DEINTERLACE_ODD_HEADER_HEIGHT),
                            wl_fixed_from_int(720),
                            wl_fixed_from_int(DEINTERLACE_FIELD_HEIGHT));
                    post_window(ctxt, p_window, idx, 1);

                    // offset to even field
                    wp_viewport_set_source(p_viewport,
                            wl_fixed_from_int(0),
                            wl_fixed_from_int(DEINTERLACE_ODD_HEIGHT + DEINTERLACE_EVEN_HEADER_HEIGHT),
                            wl_fixed_from_int(720),
                            wl_fixed_from_int(DEINTERLACE_FIELD_HEIGHT));
                    post_window(ctxt, p_window, idx, 0);
                }
                else if (field_type == QCARCAM_INTERLACE_FIELD_EVEN_ODD)
                {
                    // offset to even field
                    wp_viewport_set_source(p_viewport,
                            wl_fixed_from_int(0),
                            wl_fixed_from_int(DEINTERLACE_EVEN_HEADER_HEIGHT),
                            wl_fixed_from_int(720),
                            wl_fixed_from_int(DEINTERLACE_FIELD_HEIGHT));
                    post_window(ctxt, p_window, idx, 1);

                    // offset to odd field
                    wp_viewport_set_source(p_viewport,
                            wl_fixed_from_int(0),
                            wl_fixed_from_int(DEINTERLACE_EVEN_HEIGHT + DEINTERLACE_ODD_HEADER_HEIGHT),
                            wl_fixed_from_int(720),
                            wl_fixed_from_int(DEINTERLACE_FIELD_HEIGHT));
                    post_window(ctxt, p_window, idx, 0);
                }
                else
                {
                    QCARCAM_ERRORMSG("Unknown field type %d", field_type);
                }
            }
            else
            {
                wp_viewport_set_source(p_viewport,
                        wl_fixed_from_int(p_window->src_rec.src_x),
                        wl_fixed_from_int(p_window->src_rec.src_y),
                        wl_fixed_from_int(p_window->src_rec.src_width),
                        wl_fixed_from_int(p_window->src_rec.src_height));
                post_window(ctxt, p_window, idx, 1);
            }
        }

        pthread_mutex_lock(&p_window->m_mutex);

        //Committed frames to display achieves max value, wait for display to release buffer to go on
        while (p_window->commit_frames >= MAX_COMMIT_FRAME_NUM)
        {
            QCARCAM_ALWZMSG("Committed buffer number to window user ctxt(%p) achieves maxium number (%d)",
                p_window, MAX_COMMIT_FRAME_NUM);

            pthread_mutex_unlock(&p_window->m_mutex);
            usleep(5000);
            pthread_mutex_lock(&p_window->m_mutex);
        }

        while(!p_window->rel_buf_list->empty())
        {
            uint32 idx = p_window->rel_buf_list->front();
            p_window->rel_buf_list->pop_front();

            if (p_rel_buf_idx)
            {
                p_rel_buf_idx->push_back(idx);
            }
            p_window->buffers[idx].isdequeue = 1;
        }
        pthread_mutex_unlock(&p_window->m_mutex);

    }

    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_dump_window_buffer
///
/// @brief Dump frame to a file
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
/// @param idx              Buffer index
/// @param filename         Char pointer to file name to be dumped
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_dump_window_buffer(test_util_ctxt_t *ctxt, test_util_window_t *p_window, unsigned int idx, const char *filename)
{
    FILE *fp;
    size_t numBytesWritten = 0;
    size_t numByteToWrite = 0;
    unsigned char *buf_ptr = NULL;

    if (p_window->gbm_buf[idx].is_secure)
    {
        QCARCAM_ERRORMSG("secure buffer cannot be dumped");
        return QCARCAM_RET_FAILED;
    }

    numByteToWrite = p_window->gbm_buf[0].buflen;

    fp = fopen(filename, "w+b");
    QCARCAM_ERRORMSG("dumping qcarcam frame %s numByteToWrite : %u ", filename, numByteToWrite);

    if (0 != fp)
    {
        buf_ptr = (unsigned char*)p_window->gbm_buf[idx].p_data;
        numBytesWritten = fwrite((void *)buf_ptr, sizeof(byte), numByteToWrite, fp);
        fclose(fp);

        if (numBytesWritten != numByteToWrite )
        {
             QCARCAM_ERRORMSG("numByteToWrite %lu, numBytesWritten %lu, failed to write file",numByteToWrite,numBytesWritten);
             return QCARCAM_RET_FAILED;
        }
    }
    else
    {
        QCARCAM_ERRORMSG("failed to open file");
        return QCARCAM_RET_FAILED;
    }

    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_get_buf_ptr
///
/// @brief Get buffer virtual address
///
/// @param p_window       window
/// @param p_buf          pointer to buffer structure to be filled
///
/// @return Void
///////////////////////////////////////////////////////////////////////////////
void test_util_get_buf_ptr(test_util_window_t *p_window, test_util_buf_ptr_t *p_buf)
{
    int idx = p_buf->buf_idx % p_window->n_buffers;

    if (p_window->gbm_buf)
    {
        p_buf->p_va[0] = (unsigned char *)p_window->gbm_buf[idx].p_data;
        p_buf->stride[0] = p_window->stride;
    }
    else if (p_window->gbm_buf_wl)
    {
        p_buf->p_va[0] = (unsigned char *)p_window->gbm_buf_wl[idx].p_data;
        p_buf->stride[0] = p_window->stride;
    }

    if(p_buf->p_va[0])
    {
        // need to flush cache before cpu access the DMA buffer on LV
        dcache_flush_dma(p_buf->p_va[0], p_buf->stride[0] * p_window->buffer_size[1]);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_set_diag
///
/// @brief set the diagnostic structure to test_util_window_t
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
/// @param diag             diagnostic structure
///
/// @return Void
///////////////////////////////////////////////////////////////////////////////
void test_util_set_diag(test_util_ctxt_t *ctxt, test_util_window_t *p_window, test_util_diag_t* diag)
{
    if (p_window)
    {
        p_window->diag = diag;
    }
}

///////////////////////////////////////////////////////////////////////////////

/// test_util_create_c2d_surface
///
/// @brief Create a C2D surface
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
/// @param idx              Buffer index
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_create_c2d_surface(test_util_ctxt_t *ctxt, test_util_window_t *p_window, unsigned int idx)
{
    void *gpuaddr = NULL;

    if (QCARCAM_FMT_UYVY_8 == p_window->format)
    {
        int stride = qcarcam_get_stride(QCARCAM_FMT_UYVY_8, p_window->buffer_size[0]);
        int length = stride * p_window->buffer_size[1];
        length = (length + 4096 - 1) & ~(4096 - 1);


        C2D_STATUS c2d_status;
        C2D_YUV_SURFACE_DEF c2d_yuv_surface_def;
        c2d_yuv_surface_def.format = C2D_COLOR_FORMAT_422_UYVY;
        c2d_yuv_surface_def.width = p_window->buffer_size[0];
        c2d_yuv_surface_def.height = p_window->buffer_size[1];
        c2d_yuv_surface_def.stride0 = stride;
        c2d_yuv_surface_def.plane0 = p_window->gbm_buf[idx].p_data;
        c2d_status = c2dMapAddr((intptr_t)p_window->buffers[idx].ptr, p_window->gbm_buf[idx].p_data, length, 0, KGSL_USER_MEM_TYPE_ION, &gpuaddr);
        c2d_yuv_surface_def.phys0 = gpuaddr;

        c2d_status = c2dCreateSurface(&p_window->buffers[idx].c2d_surface_id,
                                      C2D_SOURCE | C2D_TARGET,
                                      (C2D_SURFACE_TYPE)(C2D_SURFACE_YUV_HOST | C2D_SURFACE_WITH_PHYS),
                                      &c2d_yuv_surface_def);

        if (c2d_status != C2D_STATUS_OK)
        {
            QCARCAM_ERRORMSG("c2dCreateSurface %d buf %d failed %d", 0, idx, c2d_status);
            return QCARCAM_RET_FAILED;
        }
    }
    else
    {
        if (!ctxt->params.disable_display)
        {
            int length = p_window->wl_window->width * p_window->wl_window->height * 4;
            int stride = p_window->buffer_size[0] * 4;

            C2D_STATUS c2d_status;
            C2D_YUV_SURFACE_DEF c2d_yuv_surface_def;
            c2d_yuv_surface_def.format = C2D_COLOR_FORMAT_8888_ARGB;
            c2d_yuv_surface_def.width = p_window->buffer_size[0];
            c2d_yuv_surface_def.height = p_window->buffer_size[1];
            c2d_yuv_surface_def.stride0 = stride;
            c2d_yuv_surface_def.plane0 = p_window->gbm_buf_wl[idx].p_data;
            c2d_status = c2dMapAddr((intptr_t)p_window->buffers[idx].wl_ptr, p_window->gbm_buf_wl[idx].p_data, length, 0, KGSL_USER_MEM_TYPE_ION, &gpuaddr);
            c2d_yuv_surface_def.phys0 = gpuaddr;

            c2d_status = c2dCreateSurface(&p_window->buffers[idx].c2d_surface_id,
                                          C2D_SOURCE | C2D_TARGET,
                                          (C2D_SURFACE_TYPE)(C2D_SURFACE_RGB_HOST | C2D_SURFACE_WITH_PHYS),
                                          &c2d_yuv_surface_def);

            if (c2d_status != C2D_STATUS_OK)
            {
                QCARCAM_ERRORMSG("c2dCreateSurface %d buf %d failed %d", 0, idx, c2d_status);
                return QCARCAM_RET_FAILED;
            }
        }
    }
    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_get_c2d_surface_id
///
/// @brief Get the ID from a C2D surface
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
/// @param idx              Buffer index
/// @param surface_id       Pointer to C2D sruface ID number
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_get_c2d_surface_id(test_util_ctxt_t *ctxt, test_util_window_t *p_window, unsigned int idx, uint32 *surface_id)
{
    if (!surface_id)
    {
        return QCARCAM_RET_BADPARAM;
    }

    *surface_id = p_window->buffers[idx].c2d_surface_id;

    return QCARCAM_RET_OK;
}

static int test_util_create_uyvy_wlbuffer(test_util_ctxt_t *ctxt, test_util_window_t *p_window, unsigned int idx, int w, int h)
{
    struct gbm_buffer *gbm_buf = p_window ? &p_window->gbm_buf[idx] : ctxt->parent_gbm_buf;
    struct gbm_buffer *gbm_buf_wl = p_window ? &p_window->gbm_buf_wl[idx] : ctxt->parent_gbm_buf;
    uint32_t fmt = p_window ? p_window->format : PARENT_SURFACE_COLOR_FOMAT;
    int bufferFd = gbm_buf->fd;
    int offset = 0;
    struct wl_buffer* buffer = NULL;

    EGLImageKHR eglimg;
    PFNEGLCREATEIMAGEKHRPROC create_image;
    PFNEGLCREATEWAYLANDBUFFERFROMIMAGEWL create_wlbuf;
    PFNEGLDESTROYIMAGEKHRPROC destroy_image;

    EGLint attribs[] = {
                 EGL_WIDTH, 0,
                 EGL_HEIGHT, 0,
                 EGL_LINUX_DRM_FOURCC_EXT, 0,
                 EGL_DMA_BUF_PLANE0_FD_EXT, 0,
                 EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
                 EGL_NONE
    };

    attribs[1] = w;
    attribs[3] = h;
    attribs[5] = get_drm_format(fmt);
    attribs[7] = bufferFd;
    attribs[9] = offset;

    create_image = (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress("eglCreateImageKHR");
    if (create_image == NULL)
    {
        QCARCAM_ERRORMSG("can't find eglCreateImageKHR");
        return 1;
    }

    create_wlbuf = (PFNEGLCREATEWAYLANDBUFFERFROMIMAGEWL) eglGetProcAddress("eglCreateWaylandBufferFromImageWL");
    if (create_wlbuf == NULL)
    {
        QCARCAM_ERRORMSG("can't find eglCreateWaylandBufferFromImageWL");
        return 1;
    }

    destroy_image = (PFNEGLDESTROYIMAGEKHRPROC) eglGetProcAddress("eglDestroyImageKHR");
    if (destroy_image == NULL)
    {
        QCARCAM_ERRORMSG("can't find eglDestroyImageKHR");
        return 1;
    }

    eglimg = create_image(ctxt->egldpy, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attribs);
    if (eglimg == NULL)
    {
        QCARCAM_ERRORMSG("can't create_image %p fd=%d", ctxt->egldpy, bufferFd);
        return 1;
    }

    buffer = create_wlbuf(ctxt->egldpy, eglimg);
    if (buffer == NULL)
    {
        QCARCAM_ERRORMSG("can't create_wlbuf egldpy %p img %p", ctxt->egldpy, eglimg);
        return 1;
    }

    if (p_window)
    { // parent surface will only commit 1 time, won't receive the buffer_release, don't need it
        p_window->gbm_buf_wl[idx].wl_buf = buffer;
        p_window->gbm_buf_wl[idx].idx = idx;
        p_window->gbm_buf_wl[idx].p_window = p_window;
        wl_buffer_add_listener(buffer, &buffer_listener, &p_window->gbm_buf_wl[idx]);
    }
    else
    {
        gbm_buf_wl->wl_buf = buffer;
    }
    destroy_image(ctxt->egldpy, eglimg);

    return 0;
}

static int test_util_create_uyvy_wlbuffer_gbm(test_util_ctxt_t *ctxt, test_util_window_t *p_window, unsigned int idx, int w, int h)
{
#if USE_GBM_BACKEND

    /* set window as NULL to create wl_buffer for parent_surface */
    struct gbm_buffer *gbm_buf = p_window ? &p_window->gbm_buf[idx] : ctxt->parent_gbm_buf;
    struct gbm_buffer *gbm_buf_wl = p_window ? &p_window->gbm_buf_wl[idx] : ctxt->parent_gbm_buf;
    uint32_t fmt = p_window ? p_window->format : PARENT_SURFACE_COLOR_FOMAT;

    int bufferFd = gbm_buf->fd;
    struct gbm_bo *bo = gbm_buf->bo;

    int offset = 0;
    struct wl_buffer* buffer = NULL;
    int meta_fd;
    struct gbm_buffer_params *params = NULL;
    uint32_t flags = GBM_BUFFER_PARAMS_FLAGS_EARLY_DISPLAY;
    uint32_t wait_cb_times = 10;
    uint32_t i = 0;
    uint32_t drmfmt = DRM_FORMAT_UYVY;

    drmfmt = get_gbm_format(fmt);

    QCARCAM_INFOMSG("use gbm backend bo %p FMT= %u flags = 0x%x", bo, fmt, flags);
    gbm_perform(GBM_PERFORM_GET_METADATA_ION_FD, bo, &meta_fd);
    if (meta_fd < 0){
        QCARCAM_ERRORMSG("Get bo meta fd failed \n");
        return 1;
    }

    params = gbm_buffer_backend_create_params(ctxt->wl_display->gbmbuf);
    if (params == NULL)
    {
        QCARCAM_ERRORMSG("gbm_buffer_backend_create_params fail");
        return 1;
    }

    gbm_buffer_params_add_listener(params, &gbm_params_listener, (void*)gbm_buf_wl);
    gbm_buffer_params_create(params,
                                    bufferFd,
                                     meta_fd,
                                     w,
                                     h,
                                     drmfmt,
                                     flags);

    wl_display_roundtrip(ctxt->wl_display->display);

    while (gbm_buf_wl->state != GBM_BUF_STATE_INIT_WL &&
            i < wait_cb_times)
    {
        usleep(2000);
        ++i;
    }
    if (gbm_buf_wl->state == GBM_BUF_STATE_INIT_WL &&
        gbm_buf_wl->wl_buf != NULL)
    {
        gbm_buf_wl->idx = idx;
        gbm_buf_wl->p_window = p_window;
        wl_buffer_add_listener(gbm_buf_wl->wl_buf, &buffer_listener, gbm_buf_wl);
    }
    else
    {
        QCARCAM_ERRORMSG("test_util_create_uyvy_wlbuffer_gbm fail");
        return 1;
    }
    QCARCAM_INFOMSG("use gbm backend end wl_buf = %p", gbm_buf_wl->wl_buf);
#endif
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_gpio_config
///
/// @brief enable IO privileges, configure the gpio and set it up for interrupts
///
/// @param intr             Pointer for the IRQ to be stored
/// @param gpio_number      Specific gpio that is being utilised
/// @param trigger          Instance of the signal which shall causes the interrupt
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_gpio_interrupt_config(uint32_t *intr, int gpio_number, test_util_trigger_type_t trigger)
{
    return QCARCAM_RET_UNSUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_interrupt_attach
///
/// @brief attach the interrupt event to an interrupt id
///
/// @param arguments    arguments to pass to the newly created thread
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_interrupt_attach(test_util_intr_thrd_args_t *arguments)
{
    return QCARCAM_RET_UNSUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_interrupt_wait_and_unmask
///
/// @brief wait for a GPIO interrupt and then unmask it
///
/// @param irq              IRQ to unmask
/// @param interrupt_id     interrupt id to unmask
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_interrupt_wait_and_unmask(uint32_t irq, int interrupt_id)
{
    return QCARCAM_RET_UNSUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_get_param
///
/// @brief get the value of the window parameter of the window
///
/// @param p_window         Pointer to window
/// @param param            window parameter you are trying to access
/// @param value            value of parameter will be stored here
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_get_param(test_util_window_t *p_window, test_util_params_t param, int *value)
{
    return QCARCAM_RET_UNSUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_set_param
///
/// @brief set the value of the window parameter
///
/// @param p_window         Pointer to window
/// @param param            window parameter you want to change
/// @param value            value you want to set the param to
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_set_param(test_util_window_t *p_window, test_util_params_t param, int value)
{
    return QCARCAM_RET_UNSUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_set_power_callback
///
/// @brief set power event callback
///
/// @return NULL
///////////////////////////////////////////////////////////////////////////////
void test_util_set_power_callback(power_event_callable pm_event_callback, void* p_usr_ctxt)
{
    g_pm_handle.p_power_event_callback = pm_event_callback;
    g_pm_handle.event_client_data      = p_usr_ctxt;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_allocate_input_buffers
///
/// @brief Allocate buffers for injection as input to qcarcam
///
/// @param p_window         Pointer to window
/// @param p_buffer_list    Pointer to buffer list
/// @param size             size to allocate
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_allocate_input_buffers(test_util_window_t*                  p_window,
                                                           QCarCamBufferList_t*    p_buffer_list,
                                                           unsigned int             size)
{
    return QCARCAM_RET_UNSUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_free_input_buffers
///
/// @brief Free buffers allocated for injection as input to qcarcam
///
/// @param p_window         Pointer to window
/// @param p_buffer_list    Pointer to buffer list
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_free_input_buffers(test_util_window_t*              p_window,
                                                     QCarCamBufferList_t*  p_buffer_list)
{
    return QCARCAM_RET_UNSUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_read_input_data
///
/// @brief Read input data into buffer list
///
/// @param p_window         Pointer to window
/// @param nframes          Number of frames stored in the file
/// @param filename         Path to data file to be read
/// @param size             Size of each frame
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_read_input_data(test_util_window_t*                 p_window,
                                                 const char*                filename,
                                                 int                        nframes,
                                                 size_t                     size)
{
    return QCARCAM_RET_UNSUPPORTED;
}
