/* ===========================================================================
 * Copyright (c) 2017-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */
#ifndef _TEST_UTIL_EDRM_H_
#define _TEST_UTIL_EDRM_H_

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <poll.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <vector>
#include <thread>
#include <utils/Log.h>
#include <cstdio>

/* edrm headers */
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>
#include <drm.h>
#include <linux/ion.h>
#include <linux/msm_ion.h>
#include <libdrm_macros.h>
#include <drm/msm_drm.h>

/* Graphics headers */
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#include <assert.h>
#include <malloc.h>

#define MAX_BUFFER 4

typedef struct
{
    void*  ptr[2];  //plane va
    uint32 size[2]; //plane size
    uint32 stride[2]; //plane size
    unsigned int c2d_surface_id;
    int ion_map_fd;
    int ion_fd;
    void *gpuaddr;
    int is_dequeud;
    struct ion_allocation_data ion_alloc_data;
} test_util_buffer_t;

/* Graphics types */
struct EglStruct
{
    EGLint      width;
    EGLint      height;
    EGLDisplay  dsp;
    EGLConfig   cfg;
    EGLContext  cxt;
    EGLSurface  surf;
};

/* Display types */
struct bo
{
    int fd;
    void *ptr;
    size_t size;
    size_t offset;
    size_t pitch;
    unsigned handle;
    void *plane[2];
    unsigned ihdl;
    int ion_fd;
    int ion_map_fd;
    void *gpuaddr;
    uint32_t fbid;
#ifndef C2D_DISABLED
    unsigned int c2d_surface_id;
#endif
};

struct connector_config {
    uint32_t connector_id;
    uint32_t crtc_id;
    uint32_t crtc_idx;
    uint32_t width;
    uint32_t height;
    uint32_t mode_id;
    drmModeModeInfo mode;
    uint32_t crtc_id_pid;
    uint32_t active_pid;
    uint32_t mode_id_pid;
};

struct plane_config {
    uint32_t plane_id;
    uint32_t crtc_id;
    uint32_t connector_id;
    bo bo[MAX_BUFFER];
    int x_offset;
    int x_increment;
    bool first_run;
    bool handoff_init;
    uint32_t possible_crtcs;

    int x;
    int y;
    int w;
    int h;
    int zpos;
    int format;

    /* optional */
    int handoff;
    bool handoff_set;

    /* properties */
    uint32_t crtc_pid;
    uint32_t fb_pid;
    uint32_t zpos_pid;
    uint32_t crtc_x_pid;
    uint32_t crtc_y_pid;
    uint32_t crtc_w_pid;
    uint32_t crtc_h_pid;
    uint32_t src_x_pid;
    uint32_t src_y_pid;
    uint32_t src_w_pid;
    uint32_t src_h_pid;
    uint32_t handoff_pid;
};

struct esplash_dev {
    int fd;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    int format;
    int active_fbid;
    unsigned int FramebufferName;
    unsigned int textureId;

    std::mutex plane_mutex;
    std::vector<plane_config> plane_cfg;
    std::vector<connector_config> connector_cfg;
    drmModeAtomicReqPtr req;
};

struct test_util_ctxt_t
{
    test_util_ctxt_params_t params;
    struct EglStruct m_egl;
    struct esplash_dev *pesplash_dev;
#ifndef C2D_DISABLED
    pthread_mutex_t mutex_c2d;
#endif
};

typedef struct
{
    int size[2];
    int pos[2];
    int ssize[2];
    int spos[2];
    int visibility;
} test_util_window_param_cpy_t;

struct test_util_window_t
{
    char winid[64];
    test_util_window_param_cpy_t params;

    /*buffers*/
    test_util_buffer_t* buffers;
    int n_buffers;

    int width;
    int height;

    test_util_color_fmt_t format;
    int native_format;

    test_util_diag_t* diag;
#ifdef C2D_DISABLED
    // TODO::Remove this RGB buffer once GFX YUV pipeline fixed
    uint32_t* rgbBuffer;
#endif
    int ion_fd;
};

#endif //_TEST_UTIL_EDRM_H_
