/* ===========================================================================
 * Copyright (c) 2017-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */
#ifndef _TEST_UTIL_LA_H_
#define _TEST_UTIL_LA_H_

#include <map>
#include <fcntl.h>
#include <ui/Fence.h>
#ifndef ANDROID_Q_AOSP
#ifdef ANDROID_R_AOSP
#include <ui/DisplayConfig.h>
#else
#include <ui/DisplayMode.h>
#endif
#include <ui/DisplayState.h>
#else
#include <ui/DisplayInfo.h>
#endif
#include <ui/Rect.h>

#if defined(ANDROID_Q_AOSP) || !defined(TESTUTIL_VENDOR_LIB)
#include <gui/ISurfaceComposer.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#endif

#ifdef ANDROID_Q_AOSP
#include <gralloc_priv.h>
#else
#include <QtiGralloc.h>
#include <ui/GraphicBuffer.h>
#include <EGL/egl.h>
#endif
#include <errno.h>
#include "qcarcam.h"

#ifndef C2D_DISABLED
#include "c2d2.h"
#endif

#define NUM_MAX_DISP_BUFS 3


using namespace android;

typedef struct
{
    int display_id;
    int size[2];
} test_util_display_prop_t;

typedef struct
{
    ANativeWindowBuffer* window_buf;
#ifdef ANDROID_Q_AOSP
    struct private_handle_t * private_hndl;
#endif
    void*  ptr[2];  //plane va
    uint32 size[2]; //plane size
    uint32 stride[2]; //plane size
    unsigned int c2d_surface_id;
    int is_dequeud;
} test_util_buffer_t;

struct test_util_ctxt_t
{
    test_util_ctxt_params_t params;

    test_util_display_prop_t *display_property;
    int screen_ndisplays;
#if defined(ANDROID_Q_AOSP) || !defined(TESTUTIL_VENDOR_LIB)
    sp<SurfaceComposerClient> m_surfaceComposer;
#ifndef ANDROID_Q_AOSP
#ifdef ANDROID_R_AOSP
    DisplayConfig m_displayConfig;
#else
    ui::DisplayMode m_displayConfig;
#endif
    ui::DisplayState m_displayState;
#else
    DisplayInfo m_displayInfo;
#endif
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
    ANativeWindow* nativeWindow;
#if !defined(TESTUTIL_VENDOR_LIB)
    sp<SurfaceControl> m_surfaceControl;
#endif
    test_util_window_param_cpy_t params;

    int is_offscreen;

    /*buffers*/
    test_util_buffer_t* buffers;
    int n_buffers;

    int width;
    int height;

    test_util_color_fmt_t format;
    int native_format;
    unsigned int flags;

    int min_num_buffers;
    int num_queued_buf;

    sp<GraphicBuffer> *gfx_bufs;

    test_util_diag_t* diag;
};

#endif
