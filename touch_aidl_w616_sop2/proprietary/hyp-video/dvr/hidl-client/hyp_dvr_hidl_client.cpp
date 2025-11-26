/*===========================================================================

*//** @file hyp_dvr_hidl_client.cpp
This file renders frames to SurfaceFlinger received from HIDL service

Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/

/*===========================================================================
                             Edit History

$Header:  $

when(mm/dd/yy)     who         what, where, why
-------------      --------    -------------------------------------------------------
11/29/21           nd          Enable DVR on LA S GVM and fix compilation issues
07/12/20           sa          Bringup DVR on LA GVM without muxer
03/24/20           sh          Add P010 format
01/15/20           sh          Bringup DVR on LA GVM on Hana
============================================================================*/

#include <vendor/qti/dvr/1.0/IDvrDisplay.h>
#include <vendor/qti/dvr/1.0/types.h>
#include "hyp_dvr.h"
#include "hyp_debug.h"
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#ifdef ANDROID_R_AOSP
#include <ui/DisplayConfig.h>
#else
#include <ui/DisplayMode.h>
#endif
#include <gui/ISurfaceComposer.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <gralloc_priv.h>

#define MAX_BUFFERS             6
#define FENCE_WAIT_TIMEOUT      60
#define MAX_RETRIES             10

using namespace android;
using android::hardware::Return;
using ::android::hardware::Void;
using namespace vendor::qti::dvr::V1_0;
using ::android::hardware::hidl_handle;

struct win_buf {
    ANativeWindowBuffer* native_buf;
    void *va;
};

int debug_level = 0x1;
static sp<IDvrDisplay> gDvrDisplayHal = NULL;
static DvrDisplayInfo dvr_display_info = {0, 0, 0, 0};
#ifdef ANDROID_R_AOSP
static DisplayConfig display_config;
#else
static ui::DisplayMode display_mode;
#endif
static sp<SurfaceComposerClient> composer_client = NULL;
static sp<SurfaceControl> control = NULL;
static sp<Surface> surface = NULL;
static SurfaceComposerClient::Transaction transaction;
static ANativeWindow* native_window = NULL;
static uint32_t early_preview_frame_cnt = 0;
NATIVE_HANDLE_DECLARE_STORAGE(dvr_handle, MAX_BUFFERS, 0);
struct native_handle* native_fd_handle = NULL;
static struct win_buf window_bufs[MAX_BUFFERS] = {{0, 0}};
static int fence_fd_list[MAX_BUFFERS] = {0};
static boolean preview_enable = FALSE;

class dvr_hidl_client : public IDvrDisplayStreamCB
{
    public:
    Return<int32_t> display_init();
    Return<int32_t> display_buffer(uint32_t index);
    Return<uint32_t> get_display_buf_index();
    Return<void> display_deinit();
};

Return<int32_t> dvr_hidl_client::display_buffer(uint32_t index)
{
    int32_t ret = 0;
    HYP_VIDEO_MSG_LOW("Displaying buffer on index %d", index);
    ret = native_window->queueBuffer(native_window, window_bufs[index].native_buf, fence_fd_list[index]);

    return ret;
}

Return<uint32_t> dvr_hidl_client::get_display_buf_index()
{
    int32_t fence_fd = -1;
    ANativeWindowBuffer* buf;
    int32_t i = 0;
    uint32_t buf_index = 0;

    if (MAX_BUFFERS > early_preview_frame_cnt)
    {
        buf_index = early_preview_frame_cnt++;
    }
    else
    {
        native_window->dequeueBuffer(native_window, &buf, &fence_fd);
        for (i = 0; i < MAX_BUFFERS; i++)
        {
            if (window_bufs[i].native_buf == buf)
            {
                buf_index = i;
                break;
            }
        }
        if (MAX_BUFFERS == i)
        {
            HYP_VIDEO_MSG_ERROR("failed to find buffer index. default to 0. buf addr %p", buf);
        }
    }

    fence_fd_list[buf_index] = fence_fd;
    return buf_index;
}

Return<void> dvr_hidl_client::display_deinit()
{
    HYP_VIDEO_MSG_LOW("Display Deinitializing");
    preview_enable = FALSE;
    composer_client->dispose();
    native_window_api_disconnect(native_window, NATIVE_WINDOW_API_MEDIA);
    early_preview_frame_cnt = 0;

    return Void();
}

static uint32 get_display_buffer_format(uint32 color_format)
{
    uint32 format = HAL_PIXEL_FORMAT_YCbCr_420_SP_VENUS;

    if (HDVR_PIXEL_FORMAT_TYPE_NV12 == color_format)
    {
        format = HAL_PIXEL_FORMAT_YCbCr_420_SP_VENUS;
    }
    else if (HDVR_PIXEL_FORMAT_TYPE_RGB888 == color_format)
    {
        format = HAL_PIXEL_FORMAT_RGB_888;
    }
    else if (HDVR_PIXEL_FORMAT_TYPE_P010 == color_format)
    {
        format = HAL_PIXEL_FORMAT_YCbCr_420_P010;
    }

    return format;
}

hdvr_status_type isDisplayReady()
{
    sp<IBinder> binder;
    sp<IServiceManager> sm = defaultServiceManager();
    int32_t retries = 0;
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;

    while (MAX_RETRIES > retries)
    {
        binder = sm->checkService(String16("SurfaceFlinger"));
        if (binder == 0)
        {
            hdvr_status = HDVR_STATUS_FAIL;
            HYP_VIDEO_MSG_ERROR("Failed to find SurfaceFlinger.. retrying(iter = %d)", retries);
            sleep(1);
        }
        else
        {
            hdvr_status = HDVR_STATUS_SUCCESS;
            HYP_VIDEO_MSG_LOW("SurfaceFlinger is up");
            break;
        }
    }

    return hdvr_status;
}

hdvr_status_type initializeDisplay()
{
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;
    int32 rc = 0;
    status_t err = NO_ERROR;
    uint32 format = HAL_PIXEL_FORMAT_YCbCr_420_SP_VENUS;
    int32 width, height;

    native_fd_handle = native_handle_init(dvr_handle, MAX_BUFFERS, 0);
    if (NULL == native_fd_handle)
    {
        HYP_VIDEO_MSG_ERROR("Failed to initialize native handle");
        hdvr_status = HDVR_STATUS_FAIL;
    }

    if (HDVR_STATUS_SUCCESS == hdvr_status)
    {
        composer_client = new SurfaceComposerClient;
        composer_client->initCheck();
        sp<IBinder> disp_token = SurfaceComposerClient::getInternalDisplayToken();
        if (disp_token == nullptr)
        {
            HYP_VIDEO_MSG_ERROR("No internal display\n");
            hdvr_status = HDVR_STATUS_FAIL;
        }
        else
        {
            format = get_display_buffer_format(dvr_display_info.format);
#ifdef ANDROID_R_AOSP
            err = SurfaceComposerClient::getActiveDisplayConfig(disp_token, &display_config);
#else
            err = SurfaceComposerClient::getActiveDisplayMode(disp_token, &display_mode);
#endif
            if (err != NO_ERROR)
            {
                HYP_VIDEO_MSG_ERROR("unable to get getActiveDisplayConfig\n");
                hdvr_status = HDVR_STATUS_FAIL;
            }
#ifdef ANDROID_R_AOSP
            const ui::Size& resolution = display_config.resolution;
#else
            const ui::Size& resolution = display_mode.resolution;
#endif
            width = resolution.getWidth();
            height = resolution.getHeight();
            control = composer_client->createSurface(String8("HYP_DVR"),
                                                    width, height,
                                                    format, 0);

            if ((control == NULL) || (!control->isValid()))
            {
                HYP_VIDEO_MSG_ERROR("createSurface failed\n");
                hdvr_status = HDVR_STATUS_FAIL;
            }
        }
    }

    if (HDVR_STATUS_SUCCESS == hdvr_status)
    {
        HYP_VIDEO_MSG_LOW("init display with width %d height %d format %d",
                           width, height, format);

        transaction.setLayer(control, INT_MAX);
        transaction.setPosition(control, 0, 0);
        transaction.setSize(control, width, height);
        transaction.apply();
        sp<Surface> anw = control->getSurface();
        native_window = anw.get();
        native_window_set_usage(native_window, GRALLOC_USAGE_SW_READ_OFTEN |
                                GRALLOC_USAGE_SW_WRITE_OFTEN);
        native_window_set_buffer_count(native_window, MAX_BUFFERS);
        native_window_set_buffers_dimensions(native_window, dvr_display_info.width, dvr_display_info.height);
        native_window_set_scaling_mode(native_window,
               NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
        native_window_api_connect(native_window, NATIVE_WINDOW_API_MEDIA);

        for (int32 i = 0; i < MAX_BUFFERS; i++)
        {
            ANativeWindowBuffer* buf;

            rc = native_window->dequeueBuffer(native_window, &buf, &fence_fd_list[i]);
            if (0 != rc)
            {
                HYP_VIDEO_MSG_ERROR("Dequeue buffer failed for fence =%d, rc = %d\n",
                       fence_fd_list[i], rc);
                rc = -1;
                hdvr_status = HDVR_STATUS_FAIL;
                break;
            }
            else
            {
                window_bufs[i].native_buf = buf;
                struct private_handle_t * private_hndl = (struct private_handle_t *)buf->handle;
                native_fd_handle->version = sizeof(struct native_handle);
                native_fd_handle->numFds = MAX_BUFFERS;
                native_fd_handle->numInts = 0;
                native_fd_handle->data[i] = private_hndl->fd;
                HYP_VIDEO_MSG_INFO("index = %d fd = %d", i, private_hndl->fd);
            }
        }

        if (0 != rc)
        {
            composer_client->dispose();
        }
    }

    early_preview_frame_cnt = 0;

    return hdvr_status;
}

Return<int32_t> dvr_hidl_client::display_init()
{
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;
    if (FALSE == preview_enable)
    {
        gDvrDisplayHal->getDisplayInfo([&](int32_t error, DvrDisplayInfo t)
                                        {
                                            if (0 == error)
                                            {
                                                dvr_display_info = t;
                                                HYP_VIDEO_MSG_LOW("Got Display Info width %d height %d", dvr_display_info.width, dvr_display_info.height);
                                            }
                                            else
                                            {
                                                HYP_VIDEO_MSG_ERROR("Failed to get display info");
                                                hdvr_status = HDVR_STATUS_FAIL;
                                            }
                                        }
                                      );
        if (HDVR_STATUS_SUCCESS == hdvr_status)
        {
            hdvr_status = initializeDisplay();
        }

        if (HDVR_STATUS_SUCCESS == hdvr_status)
        {
            gDvrDisplayHal->setDisplayBuffers(native_fd_handle);
            preview_enable = TRUE;
        }
    }

    return hdvr_status;
}

int main()
{
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;
    dvr_hidl_client* clientObj = NULL;

    if (gDvrDisplayHal == NULL)
    {
        gDvrDisplayHal = IDvrDisplay::tryGetService();
    }

    if (gDvrDisplayHal != NULL)
    {
        HYP_VIDEO_MSG_LOW("Got DVR HIDL Service handle");
    }
    else
    {
        hdvr_status = HDVR_STATUS_FAIL;
        HYP_VIDEO_MSG_ERROR("DVR hal gDvrDisplayHal is NULL");
    }

    if (HDVR_STATUS_SUCCESS == hdvr_status)
    {
        hdvr_status = isDisplayReady();
    }

    if (HDVR_STATUS_SUCCESS == hdvr_status)
    {
        gDvrDisplayHal->getDisplayInfo([&](int32_t error, DvrDisplayInfo t)
                                        {
                                            if (0 == error)
                                            {
                                                dvr_display_info = t;
                                                HYP_VIDEO_MSG_LOW("Got Display Info width %d height %d", dvr_display_info.width, dvr_display_info.height);
                                            }
                                            else
                                            {
                                                HYP_VIDEO_MSG_ERROR("Failed to get display info");
                                                hdvr_status = HDVR_STATUS_FAIL;
                                            }
                                        }
                                      );

        if (HDVR_STATUS_SUCCESS == hdvr_status)
        {
            clientObj = new dvr_hidl_client;
            if (NULL != clientObj)
            {
                Return<int32_t> ret = gDvrDisplayHal->registerCallback(clientObj);
                if (!ret.isOk())
                {
                    HYP_VIDEO_MSG_ERROR("Failed to register callback object");
                    hdvr_status = HDVR_STATUS_FAIL;
                }
            }
            else
            {
                HYP_VIDEO_MSG_ERROR("Failed to create callback object");
                hdvr_status = HDVR_STATUS_FAIL;
            }
        }
    }

    if (HDVR_STATUS_SUCCESS == hdvr_status)
    {
        ProcessState::self()->startThreadPool();
        IPCThreadState::self()->joinThreadPool();
    }

    if (clientObj)
    {
        delete clientObj;
    }

    return 0;
}
