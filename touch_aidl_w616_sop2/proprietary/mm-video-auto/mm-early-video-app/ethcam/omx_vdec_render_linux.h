/*--------------------------------------------------------------------------
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Copyright (c) 2010 - 2013, 2016 - 2017, The Linux Foundation. All rights reserved.
 * Copyright (c) 2011 Benjamin Franzke
 * Copyright (c) 2010 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
--------------------------------------------------------------------------*/

#ifndef __OMX_VDEC_RENDER_LINUX_H__
#define __OMX_VDEC_RENDER_LINUX_H__

#define PROPERTY_VALUE_MAX 92
#include <glib.h>
#define strlcpy g_strlcpy

#define ALOGE(fmt, args...) fprintf(stderr, fmt, ##args)
#define DEBUG_PRINT printf
#define DEBUG_PRINT_ERROR printf

#define USE_GBM
#define WL_DISPLAY
#define ALLOCATE_BUFFER 1

#include "omx_vdec_render.h"

enum {
   PRIO_ERROR=0x1,
   PRIO_INFO=0x1,
   PRIO_HIGH=0x2,
   PRIO_LOW=0x4
};

#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#define getpid() syscall(SYS_getpid)
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define DEBUG_PRINT_CTL(level, fmt, args...)   \
  do {                                        \
    char *ptr = getenv("OMXT_DEBUG_LEVEL");    \
    if (level & (ptr?atoi(ptr):0) )           \
       printf("[%ld:%ld]:[%s:%d] "fmt" \n", getpid(), \
       gettid(), __FILENAME__, __LINE__, ##args); \
  } while(0)

#define DEBUGT_PRINT(fmt, args...) DEBUG_PRINT_CTL(PRIO_LOW, fmt, ##args)
#define DEBUGT_PRINT_INFO(fmt, args...) DEBUG_PRINT_CTL(PRIO_HIGH, fmt, ##args)
#define DEBUGT_PRINT_ERROR(fmt,args...) DEBUG_PRINT_CTL(PRIO_ERROR, fmt, ##args)
#define ALOGE DEBUGT_PRINT_ERROR

#define WL_DISPLAY
#include <inttypes.h>
#include <linux/msm_mdp.h>
#ifdef WL_DISPLAY
#include <poll.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include "ivi-application-client-protocol.h"
#include <drm_fourcc.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm.h>
#include <drm_mode.h>

#include <gl2.h>
#include "linux-dmabuf-client-protocol.h"
#include "gbm.h"
#include "gbm_priv.h"
#include "msm_drm.h"
#include <ilm/ilm_control.h>
#include "gbm-buffer-backend-client-protocol.h"
#endif


#ifdef USE_ION
#include <linux/msm_ion.h>
#endif

#ifdef USE_ION
#define MEM_DEVICE "/dev/ion"
#define GBM_MEM_DEVICE "/dev/dri/renderD128"
#define MEM_HEAP_ID ION_CP_MM_HEAP_ID
struct vdec_ion {
    int ion_device_fd;
    struct ion_fd_data fd_ion_data;
    struct ion_allocation_data ion_alloc_data;
    struct gbm_bo *bo;
    int bo_fd;
    int meta_fd;
};

extern bool first_start;

int alloc_map_ion_memory(OMX_U32 buffer_size,struct vdec_ion *vdec, OMX_U32 alignment, int flag);
void free_ion_memory(struct vdec_ion *buf_ion_info);
#endif

extern GQueue queue_buffer_list;

#ifdef WL_DISPLAY
struct display {
    struct wl_display *display;
    struct wl_event_queue *queue;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct xdg_shell *shell;
    struct wl_shell *wlshell;
    struct zlinux_dmabuf *dmabuf;
    struct wl_shm *shm;
    uint32_t formats;
    struct ivi_application *ivi_application;
    GThread *thread;
    struct gbm_buffer_backend* gbmbuf;
};
struct window {
    struct display *display;
    int width, height;
    struct wl_surface *surface;
    struct wl_shell_surface *shell_surface;
    struct xdg_surface *xdg_surface;
    struct ivi_surface *ivi_surface;
    struct wl_callback *callback;
};
struct listen_buffer {
    struct wl_buffer *wlbuf;
    void *shm_data;
    int busy;
};

struct queue_buffer {
  uint8 *buffer;
  int len;
};

static int wl_buf_render(struct OMX_BUFFERHEADERTYPE *pBufHdr);
#endif

int drm_buf_render(struct OMX_BUFFERHEADERTYPE *pBufHdr);
int drm_open_display();
void drm_close_display();

static void configure_ilm_surface(t_ilm_uint id, t_ilm_uint width, t_ilm_uint height);
static void surfaceCallbackFunction(t_ilm_uint id, struct ilmSurfaceProperties* sp, t_ilm_notification_mask m);
static void callbackFunction(ilmObjectType object, t_ilm_uint id, t_ilm_bool created, void *user_data);
static struct display *create_display(void);
static void destroy_buffer(void *data);
static struct window *create_window(struct display *display, int width, int height);
static void destroy_display(struct display *display);
static void weston_close_display();
static void destroy_window(struct window *window);

void *create_gbm_thread(void *arg);
void *check_gpio_thread(void *arg);

OMX_ERRORTYPE use_output_buffer_multiple_fd ( OMX_COMPONENTTYPE *dec_handle,
                                  OMX_BUFFERHEADERTYPE  ***pBufHdrs,
                                  OMX_U32 nPortIndex,
                                  OMX_U32 bufSize,
                                  long bufCntMin);

#endif