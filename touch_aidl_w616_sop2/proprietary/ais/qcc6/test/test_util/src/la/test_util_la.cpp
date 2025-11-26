/* ===========================================================================
 * Copyright (c) 2017-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>

#include <sys/mman.h>

#ifdef TESTUTIL_VENDOR_LIB
#include <binder/ProcessState.h>
#endif

#include "test_util.h"
#include "test_util_la.h"

#if !defined(ANDROID_Q_AOSP)
#include <android/hardware/graphics/mapper/4.0/IMapper.h>
using Error_v4       = ::android::hardware::graphics::mapper::V4_0::Error;
using IMapper_v4     = ::android::hardware::graphics::mapper::V4_0::IMapper;
#endif

/**
 * @NOTE: QCarCam require minimum 16byte alignment. C2D requires 64byte.
 *        Setting the stride alignment to 64.
 */
#define STRIDE_ALIGN 64

#define KGSL_USER_MEM_TYPE_ION 3

//max time can be taken for displaying each frame initial value 33ms
//TODO:Set this value based on sensor's FPS
static int max_time_to_display = 33;

/*For now force to use HAL_PIXEL_FORMAT_YCRCB_420_SP */
//#define USE_NV12_HEIGHT_ALIGNED


/**
 * @brief Pattern definitions to fill buffers
 */
typedef enum
{
    TEST_UTIL_PATTERN_BLACK = 0
}test_util_pattern_t;

//power manager resource
typedef struct
{
    void* event_client_data;
    power_event_callable p_power_event_callback;
}test_util_pm_handle_t;
static test_util_pm_handle_t g_pm_handle;

/**
 * get_display_info
 *
 * @brief fills display information for the test util context
 *
 * @param ctxt      test_util context
 *
 * @return QCarCamRet_e
 */
static QCarCamRet_e get_display_info(test_util_ctxt_t *ctxt)
{
    int err = 0;
#if defined(ANDROID_Q_AOSP) || !defined(TESTUTIL_VENDOR_LIB)
    // Get main display parameters.
    //ANDROID_Q_AOSP or above
    sp<IBinder> displayToken = SurfaceComposerClient::getInternalDisplayToken();

    if (displayToken == nullptr) {
        QCARCAM_ERRORMSG("ERROR: no internal display");
        return QCARCAM_RET_FAILED;
    }
#ifdef ANDROID_Q_AOSP
    err = SurfaceComposerClient::getDisplayInfo(displayToken, &ctxt->m_displayInfo);
    if (err != NO_ERROR) {
        QCARCAM_ERRORMSG("ERROR: unable to get display characteristics");
        return QCARCAM_RET_FAILED;
    }
    uint32_t width, height, orientation;
    if (ctxt->m_displayInfo.orientation != DISPLAY_ORIENTATION_0 &&
            ctxt->m_displayInfo.orientation != DISPLAY_ORIENTATION_180) {
        // rotated
        width = ctxt->m_displayInfo.h;
        height = ctxt->m_displayInfo.w;
    } else {
        width = ctxt->m_displayInfo.w;
        height = ctxt->m_displayInfo.h;
    }
    orientation = ctxt->m_displayInfo.orientation;
#else //ANDROID_R_AOSP or above
#ifdef ANDROID_R_AOSP
    err = SurfaceComposerClient::getActiveDisplayConfig(displayToken, &ctxt->m_displayConfig);
#else
    err = SurfaceComposerClient::getActiveDisplayMode(displayToken, &ctxt->m_displayConfig);
#endif
    if (err != NO_ERROR) {
        QCARCAM_ERRORMSG("ERROR: unable to get getActiveDisplayConfig\n");
        return QCARCAM_RET_FAILED;
    }
    err = SurfaceComposerClient::getDisplayState(displayToken, &ctxt->m_displayState);
    if (err != NO_ERROR) {
        QCARCAM_ERRORMSG("ERROR: unable to get getDisplayState\n");
        return QCARCAM_RET_FAILED;
    }
    const ui::Size& resolution = ctxt->m_displayConfig.resolution;
    auto width = resolution.getWidth();
    auto height = resolution.getHeight();
    auto orientation = ctxt->m_displayState.orientation;
    if (ctxt->m_displayState.orientation != ui::ROTATION_0 &&
            ctxt->m_displayState.orientation != ui::ROTATION_180) {
        // flip width/height
        std::swap(width, height);
    }
#endif
    ctxt->m_surfaceComposer = new SurfaceComposerClient();
    if (0 == ctxt->m_surfaceComposer.get())
    {
        QCARCAM_ERRORMSG("Failed to create SurfaceComposerClient");
        return QCARCAM_RET_FAILED;
    }

    // update display projection info
    android::Rect destRect(width, height);

    SurfaceComposerClient::Transaction t;
    t.setDisplayProjection(displayToken, orientation, destRect, destRect);
#endif
    return QCARCAM_RET_OK;
}

/**
 * test_util_fill_planes
 *
 * @brief fills buffer plane information based on fmt and width/height of plane0
 *
 * @param pBuffer    buffer
 * @param fmt        qcarcam format
 *
 * @return void
 */
static void test_util_fill_planes(QCarCamBuffer_t* pBuffer, QCarCamColorFmt_e fmt)
{
    switch (fmt)
    {
    case QCARCAM_FMT_RGB_888:
        pBuffer->planes[0].stride = TESTUTIL_ALIGN(pBuffer->planes[0].width * 3, STRIDE_ALIGN);
        pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        break;
    case QCARCAM_FMT_MIPIRAW_8:
        pBuffer->planes[0].stride = pBuffer->planes[0].width;
        pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        break;
    case QCARCAM_FMT_MIPIRAW_10:
        if (0 == (pBuffer->planes[0].width % 4))
        {
            pBuffer->planes[0].stride = TESTUTIL_ALIGN(pBuffer->planes[0].width * 5 / 4, STRIDE_ALIGN);
            pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        }
        break;
    case QCARCAM_FMT_MIPIRAW_12:
        if (0 == (pBuffer->planes[0].width % 2))
        {
            pBuffer->planes[0].stride = TESTUTIL_ALIGN(pBuffer->planes[0].width * 3 / 2, STRIDE_ALIGN);
            pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        }
        break;
    case QCARCAM_FMT_MIPIRAW_16:
    case QCARCAM_FMT_PLAIN16_10:
    case QCARCAM_FMT_PLAIN16_12:
    case QCARCAM_FMT_PLAIN16_14:
    case QCARCAM_FMT_PLAIN16_16:
    case QCARCAM_FMT_UYVY_8:
        pBuffer->planes[0].stride = TESTUTIL_ALIGN(pBuffer->planes[0].width * 2, STRIDE_ALIGN);
        pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        break;
    case QCARCAM_FMT_UYVY_10:
        pBuffer->planes[0].stride = TESTUTIL_ALIGN(pBuffer->planes[0].width * 5 / 4, STRIDE_ALIGN);
        pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        break;
    case QCARCAM_FMT_UYVY_12:
        if (0 == (pBuffer->planes[0].width % 2))
        {
            pBuffer->planes[0].stride = TESTUTIL_ALIGN(pBuffer->planes[0].width * 2 * 3 / 2, STRIDE_ALIGN);
        }
        pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        break;
    case QCARCAM_FMT_NV12:
    case QCARCAM_FMT_NV21:
    {
#ifdef USE_NV12_HEIGHT_ALIGNED
        uint32 width_align = 64;
        uint32 height_align = 64;
#else
        uint32 width_align = 64;
        uint32 height_align = 32;
#endif
        pBuffer->numPlanes = 2;

        pBuffer->planes[0].stride = TESTUTIL_ALIGN(pBuffer->planes[0].width, width_align);

        //plane 2
        pBuffer->planes[1].width = pBuffer->planes[0].width;
        pBuffer->planes[1].height = pBuffer->planes[0].height / 2;
        pBuffer->planes[1].stride = pBuffer->planes[0].stride;

        pBuffer->planes[0].size = pBuffer->planes[0].stride * TESTUTIL_ALIGN(pBuffer->planes[0].height, height_align);
        pBuffer->planes[1].size = pBuffer->planes[1].stride * pBuffer->planes[1].height;
        pBuffer->planes[1].offset = pBuffer->planes[0].size;

#ifndef USE_NV12_HEIGHT_ALIGNED
        //plane 0 is 4k aligned
        pBuffer->planes[0].size = TESTUTIL_ALIGN(pBuffer->planes[0].size, 4096);
#endif
        break;
    }
    case QCARCAM_FMT_P010:
    {
        // Using venus alignment as isp_chi_cdk_test/src/chibuffermanager.cpp
        // Venus requirements from msm_media_info.h
        // Y_Stride : Width * 2 aligned to 256
        // UV_Stride : Width * 2 aligned to 256
        // Y_Scanlines : Height aligned to 32
        // UV_Scanlines : Height / 2 aligned to 16
        // Total size = align((Y_Stride * Y_Scanlines
        //
        uint32 width_align = 256;
        uint32 height_align = 32;

        //plane 2
        pBuffer->numPlanes = 2;
        pBuffer->planes[1].height = pBuffer->planes[0].height / 2;
        pBuffer->planes[1].width = pBuffer->planes[0].width;
        pBuffer->planes[0].stride = TESTUTIL_ALIGN(pBuffer->planes[0].width*2, width_align);
        pBuffer->planes[1].stride = pBuffer->planes[0].stride;

        pBuffer->planes[0].size = pBuffer->planes[0].stride * TESTUTIL_ALIGN(pBuffer->planes[0].height, height_align);
        pBuffer->planes[1].size = pBuffer->planes[1].stride * pBuffer->planes[1].height;
        pBuffer->planes[0].size = TESTUTIL_ALIGN(pBuffer->planes[0].size, 4096);
        break;
    }
    default:
        QCARCAM_ERRORMSG("Unknown format 0x%x", fmt);
        pBuffer->planes[0].size = 0;
        break;
    }
}

#if !defined(ANDROID_Q_AOSP)
/**
 * get_window_buffer_fd
 *
 * @brief retrieves file descriptor for ANativeWindowBuffer
 *
 * @param window_buf : pointer to ANativeWindowBuffer
 *
 * @return int
*/
static int get_window_buffer_fd(ANativeWindowBuffer *window_buf)
{
    sp<IMapper_v4> mapper = IMapper_v4::getService();
    int fd = -1;
    mapper->get(const_cast<native_handle_t*>(window_buf->handle), qtigralloc::MetadataType_FD,
        [&](const auto &_error, const auto &_bytestream) {
                if (_error !=  Error_v4::NONE)
                    QCARCAM_ERRORMSG("Failed to get FD %d", _error);
                else {
                    android::gralloc4::decodeInt32(qtigralloc::MetadataType_FD,
                            _bytestream, &fd);
                }
    });
    return fd;
}

/**
 * get_window_buffer_flags
 *
 * @brief retrieves flags for ANativeWindowBuffer
 *
 * @param window_buf : pointer to ANativeWindowBuffer
 *
 * @return int
*/
static int get_window_buffer_flags(ANativeWindowBuffer *window_buf)
{
    sp<IMapper_v4> mapper = IMapper_v4::getService();
    int flags = -1;
    mapper->get(const_cast<native_handle_t*>(window_buf->handle), qtigralloc::MetadataType_PrivateFlags,
        [&](const auto &_error, const auto &_bytestream) {
                if (_error !=  Error_v4::NONE)
                    QCARCAM_ERRORMSG("Failed to get flags %d", _error);
                else {
                    android::gralloc4::decodeInt32(
                        qtigralloc::MetadataType_PrivateFlags, _bytestream, &flags);
                }
    });
    return flags;
}

/**
 * get_window_buffer_size
 *
 * @brief retrieves size of ANativeWindowBuffer
 *
 * @param window_buf : pointer to ANativeWindowBuffer
 *
 * @return uint64_t
*/
static uint64_t get_window_buffer_size(ANativeWindowBuffer *window_buf)
{
    sp<IMapper_v4> mapper = IMapper_v4::getService();
    uint64_t size;
    mapper->get(const_cast<native_handle_t*>(window_buf->handle), android::gralloc4::MetadataType_AllocationSize,
        [&](const auto &_error, const auto &_bytestream) {
                if (_error !=  Error_v4::NONE)
                    QCARCAM_ERRORMSG("Failed to get size %d", _error);
                else {
                    android::gralloc4::decodeAllocationSize(_bytestream, &size);
                }
    });
    return size;
}

/**
 * get_window_buffer_alignedWidth
 *
 * @brief retrieves aligned width of ANativeWindowBuffer
 *
 * @param window_buf : pointer to ANativeWindowBuffer
 *
 * @return int
*/
static int get_window_buffer_alignedWidth(ANativeWindowBuffer *window_buf)
{
    sp<IMapper_v4> mapper = IMapper_v4::getService();
    int width;
    mapper->get(const_cast<native_handle_t*>(window_buf->handle), qtigralloc::MetadataType_AlignedWidthInPixels,
        [&](const auto &_error, const auto &_bytestream) {
                if (_error !=  Error_v4::NONE)
                    QCARCAM_ERRORMSG("Failed to get aligned width %d", _error);
                else {
                    android::gralloc4::decodeInt32(qtigralloc::MetadataType_AlignedWidthInPixels,
                            _bytestream, &width);
                }
    });
    return width;
}


/**
 * test_util_fill_native_planes
 *
 * @brief fills buffer plane information based on natively allocated buffer and format
 *
 * @param p_window    pointer to window
 * @param pCtxtBuffer pointer to allocated native buffer from window
 * @param pBuffer     pointer to qcarcam buffer whose plane information will be filled
 *
 * @return void
 */
static void test_util_fill_native_planes(test_util_window_t *p_window, test_util_buffer_t* pCtxtBuffer, QCarCamBuffer_t* pBuffer)
{
    int native_buffer_width = get_window_buffer_alignedWidth(pCtxtBuffer->window_buf);
    unsigned int native_buffer_size = get_window_buffer_size(pCtxtBuffer->window_buf);

    switch(p_window->format)
    {
    case TESTUTIL_FMT_RGB_565:
        pBuffer->planes[0].stride = native_buffer_width * 2;
        pBuffer->planes[0].size = native_buffer_size;
        break;
    case TESTUTIL_FMT_RGB_888:
        pBuffer->planes[0].stride = native_buffer_width * 3;
        pBuffer->planes[0].size = native_buffer_size;
        break;
    case TESTUTIL_FMT_RGBX_8888:
        pBuffer->planes[0].stride = native_buffer_width * 4;
        pBuffer->planes[0].size = native_buffer_size;
        break;
    case TESTUTIL_FMT_UYVY_8:
        pBuffer->planes[0].stride = native_buffer_width * 2;
        pBuffer->planes[0].size = native_buffer_size;
        break;
    case TESTUTIL_FMT_NV12:
    {
#ifdef USE_NV12_HEIGHT_ALIGNED
        //HAL_PIXEL_FORMAT_YCBCR_420_888 width,height 64 aligned
        uint32 width_align = 64;
        uint32 height_align = 64;
#else
        //HAL_PIXEL_FORMAT_YCRCB_420_SP width,height 32 aligned. UV offset 4k aligned
        uint32 width_align = 64;
        uint32 height_align = 32;
#endif

        pBuffer->numPlanes = 2;
        pBuffer->planes[0].stride = TESTUTIL_ALIGN(pBuffer->planes[0].width, width_align);
        pBuffer->planes[0].size = pBuffer->planes[0].stride *
            TESTUTIL_ALIGN(pBuffer->planes[0].height, height_align);
        pBuffer->planes[0].size = TESTUTIL_ALIGN(pBuffer->planes[0].size, 4096);

        pBuffer->planes[1].width = pBuffer->planes[0].width;
        pBuffer->planes[1].height = pBuffer->planes[0].height / 2;
        pBuffer->planes[1].stride = pBuffer->planes[0].stride;
        pBuffer->planes[1].size = pBuffer->planes[1].stride *
            TESTUTIL_ALIGN(pBuffer->planes[0].height, height_align) / 2;

        QCARCAM_ERRORMSG("NV12 allocation %dx%d [%d,%d] (%d vs %d)",
            pBuffer->planes[0].width, pBuffer->planes[0].height,
            pBuffer->planes[0].stride, pBuffer->planes[0].size,
            (pBuffer->planes[0].size + pBuffer->planes[1].size), native_buffer_size);
        break;
    }
    case TESTUTIL_FMT_P010:
    {
        // Using venus alignment as isp_chi_cdk_test/src/chibuffermanager.cpp
        // Venus requirements from msm_media_info.h
        // Y_Stride : Width * 2 aligned to 256
        // UV_Stride : Width * 2 aligned to 256
        // Y_Scanlines : Height aligned to 32
        // UV_Scanlines : Height / 2 aligned to 16
        // Total size = align((Y_Stride * Y_Scanlines
        //
        uint32 width_align = 256;
        uint32 height0_align = 32;
        uint32 height1_align = 16;

        //plane 2
        pBuffer->numPlanes = 2;
        pBuffer->planes[1].height = pBuffer->planes[0].height / 2;
        pBuffer->planes[1].width = pBuffer->planes[0].width;
        pBuffer->planes[0].stride = TESTUTIL_ALIGN(pBuffer->planes[0].width*2, width_align);
        pBuffer->planes[1].stride = pBuffer->planes[0].stride;

        pBuffer->planes[0].size = pBuffer->planes[0].stride * TESTUTIL_ALIGN(pBuffer->planes[0].height, height0_align);
        pBuffer->planes[1].size = pBuffer->planes[1].stride * TESTUTIL_ALIGN(pBuffer->planes[1].height, height1_align);
        pBuffer->planes[0].size = TESTUTIL_ALIGN(pBuffer->planes[0].size, 4096);
        QCARCAM_ERRORMSG("P010 allocation %dx%d [%d,%d] (%d vs %d)",
            pBuffer->planes[0].width, pBuffer->planes[0].height,
            pBuffer->planes[0].stride, pBuffer->planes[0].size,
            (pBuffer->planes[0].size + pBuffer->planes[1].size), native_buffer_size);
        break;
    }
    case TESTUTIL_FMT_MIPIRAW_8:
    default:
        pBuffer->planes[0].stride = native_buffer_width;
        pBuffer->planes[0].size = native_buffer_size;
    }

    pCtxtBuffer->size[0] = pBuffer->planes[0].size;
    pCtxtBuffer->size[1] = pBuffer->planes[1].size;
}

#elif defined (ANDROID_Q_AOSP)
/**
 * test_util_fill_native_planes
 *
 * @brief fills buffer plane information based on natively allocated buffer and format
 *
 * @param p_window    pointer to window
 * @param pCtxtBuffer pointer to allocated native buffer from window
 * @param pBuffer     pointer to qcarcam buffer whose plane information will be filled
 *
 * @return void
 */
static void test_util_fill_native_planes(test_util_window_t *p_window, test_util_buffer_t* pCtxtBuffer, QCarCamBuffer_t* pBuffer)
{
    switch(p_window->format)
    {
    case TESTUTIL_FMT_RGB_565:
        pBuffer->planes[0].stride = pCtxtBuffer->private_hndl->width * 2;
        pBuffer->planes[0].size = pCtxtBuffer->private_hndl->size;
        break;
    case TESTUTIL_FMT_RGB_888:
        pBuffer->planes[0].stride = pCtxtBuffer->private_hndl->width * 3;
        pBuffer->planes[0].size = pCtxtBuffer->private_hndl->size;
        break;
    case TESTUTIL_FMT_RGBX_8888:
        pBuffer->planes[0].stride = pCtxtBuffer->private_hndl->width * 4;
        pBuffer->planes[0].size = pCtxtBuffer->private_hndl->size;
        break;
    case TESTUTIL_FMT_UYVY_8:
        pBuffer->planes[0].stride = pCtxtBuffer->private_hndl->width * 2;
        pBuffer->planes[0].size = pCtxtBuffer->private_hndl->size;
        break;
    case TESTUTIL_FMT_NV12:
    {
#ifdef USE_NV12_HEIGHT_ALIGNED
        //HAL_PIXEL_FORMAT_YCBCR_420_888 width,height 64 aligned
        uint32 width_align = 64;
        uint32 height_align = 64;
#else
        //HAL_PIXEL_FORMAT_YCRCB_420_SP width,height 32 aligned. UV offset 4k aligned
        uint32 width_align = 64;
        uint32 height_align = 32;
#endif

        pBuffer->numPlanes = 2;
        pBuffer->planes[0].stride = TESTUTIL_ALIGN(pBuffer->planes[0].width, width_align);
        pBuffer->planes[0].size = pBuffer->planes[0].stride *
            TESTUTIL_ALIGN(pBuffer->planes[0].height, height_align);
        pBuffer->planes[0].size = TESTUTIL_ALIGN(pBuffer->planes[0].size, 4096);

        pBuffer->planes[1].width = pBuffer->planes[0].width;
        pBuffer->planes[1].height = pBuffer->planes[0].height / 2;
        pBuffer->planes[1].stride = pBuffer->planes[0].stride;
        pBuffer->planes[1].size = pBuffer->planes[1].stride *
            TESTUTIL_ALIGN(pBuffer->planes[0].height, height_align) / 2;

        QCARCAM_ERRORMSG("NV12 allocation %dx%d [%d,%d] (%d vs %d)",
            pBuffer->planes[0].width, pBuffer->planes[0].height,
            pBuffer->planes[0].stride, pBuffer->planes[0].size,
            (pBuffer->planes[0].size + pBuffer->planes[1].size), pCtxtBuffer->private_hndl->size);
        break;
    }
    case TESTUTIL_FMT_P010:
    {
        // Using venus alignment as isp_chi_cdk_test/src/chibuffermanager.cpp
        // Venus requirements from msm_media_info.h
        // Y_Stride : Width * 2 aligned to 256
        // UV_Stride : Width * 2 aligned to 256
        // Y_Scanlines : Height aligned to 32
        // UV_Scanlines : Height / 2 aligned to 16
        // Total size = align((Y_Stride * Y_Scanlines
        //
        uint32 width_align = 256;
        uint32 height0_align = 32;
        uint32 height1_align = 16;

        //plane 2
        pBuffer->numPlanes = 2;
        pBuffer->planes[1].height = pBuffer->planes[0].height / 2;
        pBuffer->planes[1].width = pBuffer->planes[0].width;
        pBuffer->planes[0].stride = TESTUTIL_ALIGN(pBuffer->planes[0].width*2, width_align);
        pBuffer->planes[1].stride = pBuffer->planes[0].stride;

        pBuffer->planes[0].size = pBuffer->planes[0].stride * TESTUTIL_ALIGN(pBuffer->planes[0].height, height0_align);
        pBuffer->planes[1].size = pBuffer->planes[1].stride * TESTUTIL_ALIGN(pBuffer->planes[1].height, height1_align);
        pBuffer->planes[0].size = TESTUTIL_ALIGN(pBuffer->planes[0].size, 4096);
        QCARCAM_ERRORMSG("P010 allocation %dx%d [%d,%d] (%d vs %d)",
            pBuffer->planes[0].width, pBuffer->planes[0].height,
            pBuffer->planes[0].stride, pBuffer->planes[0].size,
            (pBuffer->planes[0].size + pBuffer->planes[1].size), native_buffer_size);
        break;
    }
    case TESTUTIL_FMT_MIPIRAW_8:
    default:
        pBuffer->planes[0].stride = pCtxtBuffer->private_hndl->width;
        pBuffer->planes[0].size = pCtxtBuffer->private_hndl->size;
    }

    pCtxtBuffer->size[0] = pBuffer->planes[0].size;
    pCtxtBuffer->size[1] = pBuffer->planes[1].size;
}
#endif

/**
 * test_util_get_native_format
 *
 * @brief translates test_util format to native format
 *
 * @param fmt        tes_util format
 *
 * @return int - native format
 */
static int test_util_get_native_format(test_util_color_fmt_t fmt)
{
    switch(fmt)
    {
    case TESTUTIL_FMT_RGB_565:
        return HAL_PIXEL_FORMAT_RGB_565;
    case TESTUTIL_FMT_RGB_888:
        return HAL_PIXEL_FORMAT_RGB_888;
    case TESTUTIL_FMT_RGBX_8888:
        return HAL_PIXEL_FORMAT_RGBA_8888;
    case TESTUTIL_FMT_UYVY_8:
        return HAL_PIXEL_FORMAT_CbYCrY_422_I;
    case TESTUTIL_FMT_NV12:
#ifdef USE_NV12_HEIGHT_ALIGNED
        return HAL_PIXEL_FORMAT_YCBCR_420_888;
#else
        //return HAL_PIXEL_FORMAT_YCRCB_420_SP;
        return HAL_PIXEL_FORMAT_YCbCr_420_SP;
#endif
    case TESTUTIL_FMT_P010:
        return HAL_PIXEL_FORMAT_YCbCr_420_P010_VENUS;

    case TESTUTIL_FMT_MIPIRAW_8:
    default:
        return HAL_PIXEL_FORMAT_Y8;
    }
    return 0;
}

/**
 * test_util_fill_buffer
 *
 * @brief Fill buffer with pattern based on its format
 *
 * @param buffer        buffer to fill
 * @param pattern
 * @param format
 *
 * @return void
 */
static void test_util_fill_buffer(test_util_buffer_t* buffer, test_util_pattern_t pattern, test_util_color_fmt_t format)
{
    if (format == TESTUTIL_FMT_UYVY_8)
    {
        //grey
        memset(buffer->ptr[0], 0x80, buffer->size[0]);
    }
    else if ((format == TESTUTIL_FMT_NV12) || (format == TESTUTIL_FMT_P010))
    {
        //black
#if 0
        memset(buffer->ptr[0], 0x0, buffer->size[0]);
        memset(buffer->ptr[1], 0x80, buffer->size[1]);
#else
        memset(buffer->ptr[0], 0x80, buffer->size[0]);
        memset(buffer->ptr[1], 0xFF, buffer->size[1]);
#endif
    }
    else
    {
        memset(buffer->ptr[0], 0x0, buffer->size[0]);
    }
}

/**
 * alloc_graphic_buffers
 *
 * @brief allocate window buffers through gralloc and fill information into passed in structure
 *
 * @param p_window        pointer to window
 * @param buffers         buffers structure to be filled with allocated buffers
  *
 * @return QCarCamRet_e
 */
static QCarCamRet_e alloc_graphic_buffers(test_util_window_t *p_window, QCarCamBufferList_t *buffers)
{
    QCarCamRet_e rc = QCARCAM_RET_OK;
    const uint32_t SIZE_4KB = 0x1000;
    const uint32_t SIZE_2M = 0x200000;
    unsigned int is_secure = 0;
    uint64_t graphicsUsage  = android::GraphicBuffer::USAGE_SW_READ_OFTEN |
            android::GraphicBuffer::USAGE_SW_WRITE_OFTEN;
#ifdef QCC4x
    if (p_window->flags & QCARCAM_INPUT_FLAG_CONTENT_PROTECTED)
    {
        is_secure = 1;
        graphicsUsage = GRALLOC_USAGE_PROTECTED;
    }
#endif
    p_window->gfx_bufs = (sp<GraphicBuffer>*)calloc(p_window->n_buffers, sizeof(sp<GraphicBuffer>));
    if(!p_window->gfx_bufs)
    {
        QCARCAM_ERRORMSG("Failed to allocate GraphicBuffer lists");
        return QCARCAM_RET_NOMEM;
    }

    for (int i = 0; i < p_window->n_buffers; i++)
    {
        uint32 allocSize;

        QCARCAM_DBGMSG("%dx%d 0x%x",
            p_window->width, p_window->height, buffers->colorFmt);

        test_util_fill_planes(&buffers->pBuffers[i], buffers->colorFmt);

        p_window->buffers[i].stride[0] = buffers->pBuffers[i].planes[0].stride;
        p_window->buffers[i].stride[1] = buffers->pBuffers[i].planes[1].stride;
        p_window->buffers[i].size[0] = buffers->pBuffers[i].planes[0].size;
        p_window->buffers[i].size[1] = buffers->pBuffers[i].planes[1].size;
        if (is_secure)
        {
            allocSize = TESTUTIL_ALIGN(buffers->pBuffers[i].planes[0].size + buffers->pBuffers[i].planes[1].size, SIZE_2M);
        }
        else
        {
            allocSize = TESTUTIL_ALIGN(buffers->pBuffers[i].planes[0].size + buffers->pBuffers[i].planes[1].size, SIZE_4KB);
        }

        QCARCAM_DBGMSG("%dx%d %d %d 0x%x",
            p_window->width, p_window->height, p_window->buffers[i].stride[0], allocSize, p_window->flags);

        p_window->gfx_bufs[i] = new GraphicBuffer(allocSize,
                1,
                HAL_PIXEL_FORMAT_BLOB,
                1,
                graphicsUsage);

        if (!p_window->gfx_bufs[i])
        {
            QCARCAM_ERRORMSG("Failed to allocate buffer %dx%d (fmt=0x%x) %d",
                p_window->width, p_window->height, buffers->colorFmt, allocSize);
            rc = QCARCAM_RET_FAILED;

            while (i)
            {
                i--;
                if (!is_secure)
                {
#if defined (ANDROID_Q_AOSP)
                    munmap(p_window->buffers[i].ptr[0], p_window->buffers[i].private_hndl->size);
#else
                    munmap(p_window->buffers[i].ptr[0], (unsigned int)get_window_buffer_size(p_window->buffers[i].window_buf));
#endif
                }
            }

            goto EXIT;
        }
#if defined (ANDROID_Q_AOSP)
        p_window->buffers[i].private_hndl = (struct private_handle_t *)(p_window->gfx_bufs[i]->getNativeBuffer()->handle);
        buffers->pBuffers[i].planes[0].memHndl = (uint64_t)(uintptr_t)(p_window->buffers[i].private_hndl->fd);
        p_window->buffers[i].is_dequeud = 1;

        QCARCAM_DBGMSG("%d %dx%d %d (%d|%d) 0x%x fd=%d",
            i,
            p_window->width,
            p_window->height,
            p_window->buffers[i].stride[0],
            allocSize,
            p_window->buffers[i].private_hndl->size,
            p_window->buffers[i].private_hndl->flags,
            p_window->buffers[i].private_hndl->fd);

        if (!is_secure)
        {
            p_window->buffers[i].ptr[0] =
                    mmap(NULL, p_window->buffers[i].private_hndl->size,
                            PROT_READ | PROT_WRITE, MAP_SHARED,
                            p_window->buffers[i].private_hndl->fd, 0);
            if (p_window->buffers[i].ptr[0] == MAP_FAILED)
            {
                QCARCAM_ERRORMSG("Failed to map buffer %d (size=%d)", i, p_window->buffers[i].private_hndl->size);
                rc = QCARCAM_RET_FAILED;

                while (i)
                {
                    i--;
                    munmap(p_window->buffers[i].ptr[0], p_window->buffers[i].private_hndl->size);
                }

                goto EXIT;
            }

            p_window->buffers[i].ptr[1] = (void*)((uintptr_t)(p_window->buffers[i].ptr[0]) + p_window->buffers[i].size[0]);
        }

#else
        p_window->buffers[i].window_buf = p_window->gfx_bufs[i]->getNativeBuffer();
        buffers->pBuffers[i].planes[0].memHndl = (uint64_t)(uintptr_t)get_window_buffer_fd(p_window->buffers[i].window_buf);
        p_window->buffers[i].is_dequeud = 1;

        unsigned int size = get_window_buffer_size(p_window->buffers[i].window_buf);
        QCARCAM_DBGMSG("%d %dx%d %d (%d|%d) 0x%x fd=%d",
            i,
            p_window->width,
            p_window->height,
            p_window->buffers[i].stride[0],
            allocSize,
            size,
            get_window_buffer_flags(p_window->buffers[i].window_buf),
            get_window_buffer_fd(p_window->buffers[i].window_buf));

        if (!is_secure)
        {
            p_window->buffers[i].ptr[0] =
                    mmap(NULL, size,
                            PROT_READ | PROT_WRITE, MAP_SHARED,
                            get_window_buffer_fd(p_window->buffers[i].window_buf), 0);
            if (p_window->buffers[i].ptr[0] == MAP_FAILED)
            {
                QCARCAM_ERRORMSG("Failed to map buffer %d (size=%d)", i, size);
                rc = QCARCAM_RET_FAILED;

                while (i)
                {
                    i--;
                    munmap(p_window->buffers[i].ptr[0], size);
                }

                goto EXIT;
            }

            p_window->buffers[i].ptr[1] = (void*)((uintptr_t)(p_window->buffers[i].ptr[0]) + p_window->buffers[i].size[0]);
        }
#endif
    }

#ifdef QCC4x
    // Prefill all buffers as black
    if (!(p_window->flags & QCARCAM_INPUT_FLAG_CONTENT_PROTECTED))
#endif
    {
        for (int i = 0; i < p_window->n_buffers; i++)
        {
            test_util_fill_buffer(&p_window->buffers[i], TEST_UTIL_PATTERN_BLACK, p_window->format);
        }
    }

EXIT:
    return rc;
}

/**
 * alloc_native_window_buffers
 *
 * @brief allocate window buffers natively and fills information into passed in structure
 *
 * @param p_window        pointer to window
 * @param buffers         buffers structure to be filled with allocated buffers
  *
 * @return QCarCamRet_e
 */
static QCarCamRet_e alloc_native_window_buffers(test_util_window_t *p_window, QCarCamBufferList_t *buffers)
{
    QCarCamRet_e rc = QCARCAM_RET_OK;
#if defined(ANDROID_Q_AOSP) || !defined(TESTUTIL_VENDOR_LIB)
    unsigned int is_secure = 0;

#ifdef QCC4x
    if (p_window->flags & QCARCAM_INPUT_FLAG_CONTENT_PROTECTED)
    {
        is_secure = 1;
    }
#endif
    p_window->nativeWindow->query(p_window->nativeWindow,
            NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &p_window->min_num_buffers);
    QCARCAM_DBGMSG("NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS %d", p_window->min_num_buffers);

    native_window_set_buffer_count(p_window->nativeWindow, p_window->n_buffers);

#ifndef USE_NV12_HEIGHT_ALIGNED
    //If non aligned NV12 format, force height to 32 aligned
    if (p_window->format == TESTUTIL_FMT_NV12)
    {
        p_window->height = TESTUTIL_ALIGN(p_window->height, 32);
    }
#endif
    if (p_window->format == TESTUTIL_FMT_P010)
    {
        p_window->height = TESTUTIL_ALIGN(p_window->height, 32);
        QCARCAM_DBGMSG("Buffers %d width = %d height = %d", p_window->n_buffers, p_window->width, p_window->height);
    }

    native_window_set_buffers_dimensions(p_window->nativeWindow, p_window->width, p_window->height);

    native_window_set_scaling_mode(p_window->nativeWindow,
            NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);

    native_window_api_connect(p_window->nativeWindow, NATIVE_WINDOW_API_CAMERA);

    for (int i = 0; i < p_window->n_buffers; i++)
    {
        int err = 0;
        ANativeWindowBuffer* window_buf;
        int fenceFd;

        err = p_window->nativeWindow->dequeueBuffer(p_window->nativeWindow,
                &window_buf, &fenceFd);
        QCARCAM_DBGMSG("window_buf[%d]=%p", i, window_buf);
        if(err)
        {
            QCARCAM_ERRORMSG("Dequeue buffer failed for %d, fence = %d, err = %d",
                    i, fenceFd, err);
            rc = QCARCAM_RET_FAILED;

            while (i)
            {
                i--;
                if (!is_secure)
                {
#if defined (ANDROID_Q_AOSP)
                    munmap(p_window->buffers[i].ptr[0], p_window->buffers[i].private_hndl->size);
#else
                    munmap(p_window->buffers[i].ptr[0], (unsigned int)get_window_buffer_size(p_window->buffers[i].window_buf));
#endif
                }
            }

            goto EXIT;
        }

        p_window->buffers[i].window_buf = window_buf;
#if defined (ANDROID_Q_AOSP)
        p_window->buffers[i].private_hndl = (struct private_handle_t *)window_buf->handle;
        buffers->buffers[i].planes[0].memHndl = (uint64_t)(uintptr_t)(p_window->buffers[i].private_hndl->fd);
        p_window->buffers[i].is_dequeud = 1;

        test_util_fill_native_planes(p_window, &p_window->buffers[i], &buffers->buffers[i]);

        QCARCAM_ERRORMSG("%dx%d %d [%d] %d %d",
            window_buf->width, window_buf->height,
            buffers->buffers[i].planes[0].stride,
            window_buf->stride,
            buffers->buffers[i].planes[0].size,
            p_window->buffers[i].private_hndl->size);


        if (!is_secure)
        {
            p_window->buffers[i].ptr[0] =
                mmap(NULL, p_window->buffers[i].private_hndl->size,
                        PROT_READ | PROT_WRITE, MAP_SHARED,
                        p_window->buffers[i].private_hndl->fd, 0);
            if (p_window->buffers[i].ptr[0] == MAP_FAILED)
            {
                QCARCAM_ERRORMSG("Failed to map buffer %d (size=%d)", i, p_window->buffers[i].private_hndl->size);
                rc = QCARCAM_RET_FAILED;

                while (i)
                {
                    i--;
                    munmap(p_window->buffers[i].ptr[0], p_window->buffers[i].private_hndl->size);
                }

                goto EXIT;
            }

            p_window->buffers[i].ptr[1] = (void*)((uintptr_t)(p_window->buffers[i].ptr[0]) + p_window->buffers[i].size[0]);
        }
#else
        unsigned int size = get_window_buffer_size(p_window->buffers[i].window_buf);

        buffers->pBuffers[i].planes[0].memHndl = (uint64_t)(uintptr_t)(get_window_buffer_fd(p_window->buffers[i].window_buf));
        p_window->buffers[i].is_dequeud = 1;

        test_util_fill_native_planes(p_window, &p_window->buffers[i], &buffers->pBuffers[i]);

        QCARCAM_ERRORMSG("%dx%d %d [%d] %d %d",
            window_buf->width, window_buf->height,
            buffers->pBuffers[i].planes[0].stride,
            window_buf->stride,
            buffers->pBuffers[i].planes[0].size,
            size);


        if (!is_secure)
        {
            p_window->buffers[i].ptr[0] =
                mmap(NULL, size,
                        PROT_READ | PROT_WRITE, MAP_SHARED,
                        get_window_buffer_fd(p_window->buffers[i].window_buf), 0);
            if (p_window->buffers[i].ptr[0] == MAP_FAILED)
            {
                QCARCAM_ERRORMSG("Failed to map buffer %d (size=%d)", i, size);
                rc = QCARCAM_RET_FAILED;

                while (i)
                {
                    i--;
                    munmap(p_window->buffers[i].ptr[0], size);
                }

                goto EXIT;
            }

            p_window->buffers[i].ptr[1] = (void*)((uintptr_t)(p_window->buffers[i].ptr[0]) + p_window->buffers[i].size[0]);
        }
#endif
    }

    for (int i = 0; i < p_window->min_num_buffers; i++)
    {
        int err = 0;
        err = p_window->nativeWindow->cancelBuffer(p_window->nativeWindow,
                p_window->buffers[i].window_buf, -1);
        if(err)
        {
            QCARCAM_ERRORMSG("Cancel buffer failed. Error = %d",err);

        }

        p_window->buffers[i].is_dequeud = 0;
    }

    // Fill last buffer with 0 pattern. Some applications allocate one extra buffer
    // to send it constantly to display in the event of loss of signal from the sensor
    // instead of the last captured frame.
    if (!is_secure)
    {
        test_util_fill_buffer(&p_window->buffers[p_window->n_buffers-1], TEST_UTIL_PATTERN_BLACK, p_window->format);
    }
#endif
EXIT:
    return rc;
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
    QCarCamRet_e rc;
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

    pCtxt->params = *params;
#if !defined(ANDROID_Q_AOSP) && defined (TESTUTIL_VENDOR_LIB)
//libgui display is not supported on vendor lib from Android R onwords
    pCtxt->params.disable_display = true;
#endif
    if (!pCtxt->params.disable_display)
    {
#ifdef TESTUTIL_VENDOR_LIB
        ProcessState::initWithDriver("/dev/binder");
#endif

        rc = get_display_info(pCtxt);
        if (rc != QCARCAM_RET_OK)
        {
            goto fail;
        }
    }

    *ctxt = pCtxt;

    return QCARCAM_RET_OK;

fail:
    free(pCtxt);
    return QCARCAM_RET_FAILED;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_init_window
///
/// @brief Initialize window for display
///
/// @param ctxt             Pointer to test_util context
/// @param pp_window        Pointer to window pointer to be filled
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_init_window(test_util_ctxt_t *ctxt, test_util_window_t **pp_window)
{
    *pp_window = (test_util_window_t*)calloc(1, sizeof(struct test_util_window_t));
    if (!*pp_window)
    {
        return QCARCAM_RET_NOMEM;
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
    QCarCamRet_e rc;

    p_window->n_buffers = buffers->nBuffers;
    p_window->buffers = (test_util_buffer_t*)calloc(p_window->n_buffers, sizeof(*p_window->buffers));
    if(NULL == p_window->buffers)
    {
        QCARCAM_ERRORMSG("Failed to allocate buffer list");
        return QCARCAM_RET_NOMEM;
    }

    p_window->width = buffers->pBuffers[0].planes[0].width;
    p_window->height = buffers->pBuffers[0].planes[0].height;

    QCARCAM_DBGMSG("Buffers %d width = %d height = %d",
            p_window->n_buffers, p_window->width, p_window->height);

    if (!ctxt->params.disable_display && !p_window->is_offscreen)
    {
        rc = alloc_native_window_buffers(p_window, buffers);
    }
    else
    {
        rc = alloc_graphic_buffers(p_window, buffers);
    }

    if (rc != QCARCAM_RET_OK)
    {
        free(p_window->buffers);
        p_window->buffers = NULL;

        if (p_window->gfx_bufs)
        {
            free(p_window->gfx_bufs);
            p_window->gfx_bufs = NULL;
        }
    }

    return rc;
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
QCarCamRet_e test_util_post_window_buffer(test_util_ctxt_t *ctxt, test_util_window_t *p_window, unsigned int idx,
        std::list<uint32>* p_rel_buf_idx, QCarCamInterlaceField_e field_type)
{
    unsigned long long timer_before = 0;
    unsigned long long timer_after = 0;
    unsigned long long timer_diff = 0;
    if (!ctxt->params.disable_display && !p_window->is_offscreen)
    {
        int err = 0;
        int fenceFd = 0;
#if defined(ANDROID_Q_AOSP) || !defined(TESTUTIL_VENDOR_LIB)
        ANativeWindowBuffer* window_buf = NULL;

        if (p_window->buffers[idx].is_dequeud)
        {
            if (ctxt->params.enable_di == TESTUTIL_DEINTERLACE_SW_BOB)
            {
                if (field_type == QCARCAM_INTERLACE_FIELD_ODD)
                {
                    android_native_rect_t rect = {0, DEINTERLACE_ODD_HEADER_HEIGHT, p_window->width, DEINTERLACE_ODD_HEIGHT};
                    native_window_set_crop(p_window->nativeWindow, &rect);

                    err = p_window->nativeWindow->queueBuffer(p_window->nativeWindow,
                            p_window->buffers[idx].window_buf, -1);
                    if(err)
                    {
                        QCARCAM_ERRORMSG("Queuebuffer %d returned with err = %d", idx, err);
                    }
                    else
                    {
                        p_window->buffers[idx].is_dequeud = 0;
                        p_window->num_queued_buf++;
                    }
                }
                else if (field_type == QCARCAM_INTERLACE_FIELD_EVEN)
                {
                    android_native_rect_t rect = {0, DEINTERLACE_EVEN_HEADER_HEIGHT, p_window->width, DEINTERLACE_EVEN_HEIGHT};
                    native_window_set_crop(p_window->nativeWindow, &rect);

                    err = p_window->nativeWindow->queueBuffer(p_window->nativeWindow,
                            p_window->buffers[idx].window_buf, -1);
                    if(err)
                    {
                        QCARCAM_ERRORMSG("Queuebuffer %d returned with err = %d", idx, err);
                    }
                    else
                    {
                        p_window->buffers[idx].is_dequeud = 0;
                        p_window->num_queued_buf++;
                    }
                }
                else if (field_type == QCARCAM_INTERLACE_FIELD_ODD_EVEN)
                {
                    android_native_rect_t rect = {0, DEINTERLACE_ODD_HEADER_HEIGHT, p_window->width, DEINTERLACE_ODD_HEIGHT};
                    native_window_set_crop(p_window->nativeWindow, &rect);

                    err = p_window->nativeWindow->queueBuffer(p_window->nativeWindow,
                            p_window->buffers[idx].window_buf, -1);
                    if(err)
                    {
                        QCARCAM_ERRORMSG("Queuebuffer %d returned with err = %d", idx, err);
                    }

                    err = p_window->nativeWindow->dequeueBuffer(p_window->nativeWindow,
                            &window_buf, &fenceFd);
                    if(err)
                    {
                        QCARCAM_ERRORMSG("Dequeuebuffer %d returned with err = %d", idx, err);
                    }
                    p_window->nativeWindow->cancelBuffer(p_window->nativeWindow, window_buf, fenceFd);

                    rect = {0, DEINTERLACE_ODD_HEIGHT + DEINTERLACE_EVEN_HEADER_HEIGHT,p_window->width, DEINTERLACE_ODD_HEIGHT+DEINTERLACE_EVEN_HEIGHT};
                    native_window_set_crop(p_window->nativeWindow, &rect);

                    err = p_window->nativeWindow->queueBuffer(p_window->nativeWindow,
                            p_window->buffers[idx].window_buf, -1);
                    if(err)
                    {
                        QCARCAM_ERRORMSG("Queuebuffer %d returned with err = %d", idx, err);
                    }
                    else
                    {
                        p_window->buffers[idx].is_dequeud = 0;
                        p_window->num_queued_buf++;
                    }
                }
                else if (field_type == QCARCAM_INTERLACE_FIELD_EVEN_ODD)
                {
                    android_native_rect_t rect = {0, DEINTERLACE_EVEN_HEADER_HEIGHT, p_window->width, DEINTERLACE_EVEN_HEIGHT};
                    native_window_set_crop(p_window->nativeWindow, &rect);

                    err = p_window->nativeWindow->queueBuffer(p_window->nativeWindow,
                            p_window->buffers[idx].window_buf, -1);
                    if(err)
                    {
                        QCARCAM_ERRORMSG("Queuebuffer %d returned with err = %d", idx, err);
                    }

                    err = p_window->nativeWindow->dequeueBuffer(p_window->nativeWindow,
                            &window_buf, &fenceFd);
                    if(err)
                    {
                        QCARCAM_ERRORMSG("Dequeuebuffer %d returned with err = %d", idx, err);
                    }
                    p_window->nativeWindow->cancelBuffer(p_window->nativeWindow, window_buf, fenceFd);

                    rect = {0, DEINTERLACE_EVEN_HEIGHT + DEINTERLACE_ODD_HEADER_HEIGHT,p_window->width, DEINTERLACE_ODD_HEIGHT+DEINTERLACE_EVEN_HEIGHT};
                    native_window_set_crop(p_window->nativeWindow, &rect);

                    err = p_window->nativeWindow->queueBuffer(p_window->nativeWindow,
                            p_window->buffers[idx].window_buf, -1);
                    if(err)
                    {
                        QCARCAM_ERRORMSG("Queuebuffer %d returned with err = %d", idx, err);
                    }
                    else
                    {
                        p_window->buffers[idx].is_dequeud = 0;
                        p_window->num_queued_buf++;
                    }
                }
            }
            else
            {
                (void)test_util_get_time(&timer_before);
                err = p_window->nativeWindow->queueBuffer(p_window->nativeWindow,
                        p_window->buffers[idx].window_buf, -1);
                (void)test_util_get_time(&timer_after);
                if(err)
                {
                    QCARCAM_ERRORMSG("Queuebuffer %d returned with err = %d", idx, err);
                }
                else
                {
                    p_window->buffers[idx].is_dequeud = 0;
                    p_window->num_queued_buf++;
                    timer_diff = timer_after-timer_before;
                    if (max_time_to_display < timer_diff)
                    {
                        QCARCAM_ERRORMSG("Frame drop found in display!! Time taken for Queuebuffer = %llu", timer_diff);
                    }
                }
            }
        }
        else
        {
            QCARCAM_ERRORMSG("Trying to post a non-dequeued buffer %u", idx);
        }

        while (p_window->num_queued_buf > p_window->min_num_buffers)
        {
            (void)test_util_get_time(&timer_before);
            err = p_window->nativeWindow->dequeueBuffer(p_window->nativeWindow, &window_buf, &fenceFd);
            (void)test_util_get_time(&timer_after);
            if (err)
            {
                QCARCAM_ERRORMSG("Dequeuebuffer returned with err = %d", err);
                usleep(10000);
            }
            else
            {
                timer_diff = timer_after-timer_before;
                if (max_time_to_display < timer_diff)
                {
                    QCARCAM_ERRORMSG("Frame drop found in display!! Time taken for Dequeuebuffer = %llu", timer_diff);
                }
                // wait for the buffer
                sp<Fence> fence(new Fence(fenceFd));
                status_t status = fence->wait(Fence::TIMEOUT_NEVER);
                if (status != NO_ERROR)
                {
                    QCARCAM_ERRORMSG("fence wait err = %d", status);
                    p_window->nativeWindow->cancelBuffer(p_window->nativeWindow, window_buf, fenceFd);
                    break;
                }

                p_window->num_queued_buf--;

                for (int i = 0; i < p_window->n_buffers; i++)
                {
                    if (p_window->buffers[i].window_buf == window_buf)
                    {
                        p_window->buffers[i].is_dequeud = 1;

                        if (p_rel_buf_idx)
                        {
                            p_rel_buf_idx->push_back(i);
                        }

                        break;
                    }
                }
            }
        }
#endif
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
/// @param idx              Buffer idx
/// @param filename         Char pointer to file name to be dumped
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_dump_window_buffer(test_util_ctxt_t *ctxt, test_util_window_t *p_window, unsigned int idx, const char *filename)
{
    FILE *fp;
    size_t numBytesWritten = 0;
    size_t numByteToWrite = 0;

#ifdef QCC4x
    if (p_window->flags & QCARCAM_INPUT_FLAG_CONTENT_PROTECTED)
    {
        QCARCAM_ERRORMSG("secure buffer cannot be dumped");
        return QCARCAM_RET_FAILED;
    }
#endif
    if (!p_window->buffers[idx].ptr[0])
    {
        QCARCAM_ERRORMSG("buffer is not mapped");
        return QCARCAM_RET_FAILED;
    }

    fp = fopen(filename, "w+");

    QCARCAM_ERRORMSG("dumping qcarcam frame %s", filename);

    if (0 != fp)
    {
        test_util_buffer_t* buffer = &p_window->buffers[idx];

        numByteToWrite = buffer->size[0];
        numBytesWritten = fwrite(buffer->ptr[0], 1, buffer->size[0], fp);

        if ((p_window->format == TESTUTIL_FMT_NV12) || (p_window->format == TESTUTIL_FMT_P010))
        {
            numByteToWrite += buffer->size[1];
            numBytesWritten += fwrite(buffer->ptr[1], 1, buffer->size[1], fp);
        }

        if (numBytesWritten != numByteToWrite)
        {
            QCARCAM_ERRORMSG("error no data written to file");
        }

        fclose(fp);
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

    p_buf->p_va[0] = (unsigned char *)p_window->buffers[idx].ptr[0];
    p_buf->stride[0] = p_window->buffers[idx].stride[0];
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
    // Deinit window/surface if any
    free(p_window);

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
    int i = 0;

    if (p_window->buffers)
    {
#if defined(ANDROID_Q_AOSP)
            for (i = 0; i < p_window->n_buffers; i++)
            {
                if (p_window->buffers[i].private_hndl)
                {
                    munmap(p_window->buffers[i].ptr[0],
                            p_window->buffers[i].private_hndl->size);
                }
            }

#else
            for (i = 0; i < p_window->n_buffers; i++)
            {
                if (p_window->buffers[i].window_buf->handle)
                {
                    munmap(p_window->buffers[i].ptr[0],
                            get_window_buffer_size(p_window->buffers[i].window_buf));
                }
            }
#endif

        free(p_window->buffers);
    }

    if (p_window->gfx_bufs)
    {
        free(p_window->gfx_bufs);
    }

    memset(p_window, 0x0, sizeof(*p_window));

    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_deinit
///
/// @brief Destroy context and free memory.
///
/// @param ctxt   Pointer test_util context to be destroyed
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_deinit(test_util_ctxt_t *ctxt)
{
    return QCARCAM_RET_OK;
}

#ifndef C2D_DISABLED
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
        stride = width * 3/2;
        break;
    default:
        break;
    }

    return stride;
}
#endif

///////////////////////////////////////////////////////////////////////////////
/// test_util_create_source_c2d_surface
///
/// @brief Create a Source C2D surface
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
/// @param idx              Buffer index
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_create_source_c2d_surface(test_util_ctxt_t *ctxt, test_util_window_t *p_window, unsigned int idx)
{
#ifndef C2D_DISABLED
    void *gpuaddr = NULL;
    int stride = qcarcam_get_stride(QCARCAM_FMT_UYVY_8, p_window->width);
    int length = stride * p_window->height;
    length = (length + 4096 - 1) & ~(4096 - 1);

    C2D_STATUS c2d_status;
    C2D_YUV_SURFACE_DEF c2d_yuv_surface_def;
    c2d_yuv_surface_def.format = C2D_COLOR_FORMAT_422_UYVY;
    c2d_yuv_surface_def.width = p_window->width;
    c2d_yuv_surface_def.height = p_window->height;
    c2d_yuv_surface_def.stride0 = stride;
    c2d_yuv_surface_def.plane0 = p_window->buffers[idx].ptr[0];
#if defined (ANDROID_Q_AOSP)
    c2d_status = c2dMapAddr(p_window->buffers[idx].fd, p_window->buffers[idx].ptr[0], length, 0, KGSL_USER_MEM_TYPE_ION, &gpuaddr);
#else
    c2d_status = c2dMapAddr(get_window_buffer_fd(p_window->buffers[idx].window_buf), p_window->buffers[idx].ptr[0], length, 0, KGSL_USER_MEM_TYPE_ION, &gpuaddr);
#endif
    c2d_yuv_surface_def.phys0 = gpuaddr;
    C2D_SURFACE_TYPE surface_type = (C2D_SURFACE_TYPE)(C2D_SURFACE_YUV_HOST | C2D_SURFACE_WITH_PHYS);

    c2d_status = c2dCreateSurface(&p_window->buffers[idx].c2d_surface_id,
            C2D_SOURCE,
            surface_type,
            &c2d_yuv_surface_def);

    if (c2d_status != C2D_STATUS_OK)
    {
        QCARCAM_ERRORMSG("c2dCreateSurface %d buf %d failed %d", 0, idx, c2d_status);
        return QCARCAM_RET_FAILED;
    }
#endif
    return QCARCAM_RET_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_create_target_c2d_surface
///
/// @brief Create a Target C2D surface
///
/// @param ctxt             Pointer to test_util context
/// @param p_window         Pointer to window
/// @param idx              Buffer index
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
QCarCamRet_e test_util_create_target_c2d_surface(test_util_ctxt_t *ctxt, test_util_window_t *p_window, unsigned int idx)
{
#ifndef C2D_DISABLED
    void *gpuaddr = NULL;
    int stride = qcarcam_get_stride(QCARCAM_FMT_RGB_888, p_window->width);
    int length = stride * p_window->height;
    length = (length + 4096 - 1) & ~(4096 - 1);

    C2D_STATUS c2d_status;
    C2D_RGB_SURFACE_DEF c2d_rgb_surface_def;
    c2d_rgb_surface_def.format = C2D_COLOR_FORMAT_888_RGB | C2D_FORMAT_SWAP_RB;
    c2d_rgb_surface_def.width = p_window->width;
    c2d_rgb_surface_def.height = p_window->height;
    c2d_rgb_surface_def.stride = stride;
    c2d_rgb_surface_def.buffer = p_window->buffers[idx].ptr[0];
#if defined (ANDROID_Q_AOSP)
    c2d_status = c2dMapAddr(p_window->buffers[idx].fd, p_window->buffers[idx].ptr[0], length, 0, KGSL_USER_MEM_TYPE_ION, &gpuaddr);
#else
    c2d_status = c2dMapAddr(get_window_buffer_fd(p_window->buffers[idx].window_buf), p_window->buffers[idx].ptr[0], length, 0, KGSL_USER_MEM_TYPE_ION, &gpuaddr);
#endif
    c2d_rgb_surface_def.phys = gpuaddr;
    C2D_SURFACE_TYPE surface_type = (C2D_SURFACE_TYPE)(C2D_SURFACE_RGB_HOST | C2D_SURFACE_WITH_PHYS);

    c2d_status = c2dCreateSurface(&p_window->buffers[idx].c2d_surface_id,
            C2D_TARGET,
            surface_type,
            &c2d_rgb_surface_def);

    if (c2d_status != C2D_STATUS_OK)
    {
        QCARCAM_ERRORMSG("c2dCreateSurface %d buf %d failed %d", 0, idx, c2d_status);
        return QCARCAM_RET_FAILED;
    }
#endif
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
QCarCamRet_e test_util_get_c2d_surface_id(test_util_ctxt_t *ctxt, test_util_window_t *p_window, unsigned int idx, unsigned int *surface_id)
{
    if (!surface_id)
        return QCARCAM_RET_BADPARAM;

    *surface_id = p_window->buffers[idx].c2d_surface_id;

    return QCARCAM_RET_OK;
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
    p_window->format = window_params->format;
    p_window->native_format = test_util_get_native_format(window_params->format);
    p_window->flags = window_params->flags;
#if !defined(ANDROID_P_AOSP) && !defined(ANDROID_Q_AOSP) && defined (TESTUTIL_VENDOR_LIB)
    //libgui display is not supported on vendor lib from Android R onwords so
    //force offscreen by default
    window_params->is_offscreen = 1;
#endif
    p_window->is_offscreen = window_params->is_offscreen;

    if (!ctxt->params.disable_display && !p_window->is_offscreen)
    {
#if defined(ANDROID_Q_AOSP) || !defined(TESTUTIL_VENDOR_LIB)
#ifdef ANDROID_Q_AOSP
        unsigned int disp_w = ctxt->m_displayInfo.w;
        unsigned int disp_h = ctxt->m_displayInfo.h;
#else //ANDROID_R_AOSP and above
        unsigned int disp_w = ctxt->m_displayConfig.resolution.getWidth();
        unsigned int disp_h = ctxt->m_displayConfig.resolution.getHeight();
        if (ctxt->m_displayState.orientation != ui::ROTATION_0 &&
                ctxt->m_displayState.orientation != ui::ROTATION_180) {
            std::swap(disp_w, disp_h);
        }
#endif
        unsigned int w = disp_w * window_params->window_size[0];
        unsigned int h = disp_h * window_params->window_size[1];
        int x          = disp_w * window_params->window_pos[0];
        int y          = disp_h * window_params->window_pos[1];
        int src_width  = window_params->buffer_size[0] * window_params->window_source_size[0];
        int src_height = window_params->buffer_size[1] * window_params->window_source_size[1];
        int src_x      = window_params->buffer_size[0] * window_params->window_source_pos[0];
        int src_y      = window_params->buffer_size[1] * window_params->window_source_pos[1];

        unsigned int flags = 0;

#ifdef QCC4x
        if (p_window->flags & QCARCAM_INPUT_FLAG_CONTENT_PROTECTED)
        {
            flags |= ISurfaceComposerClient::eSecure;
        }
#endif
        QCARCAM_DBGMSG("w = %u h = %u", w, h);
        QCARCAM_DBGMSG("x = %d y= %d", x, y);

        if (ctxt->m_surfaceComposer.get())
        {
#ifdef ANDROID_Q_AOSP
            p_window->m_surfaceControl =
                ctxt->m_surfaceComposer->createSurface(android::String8(window_params->debug_name),
                    ctxt->m_displayInfo.w,
                    ctxt->m_displayInfo.h,
                    p_window->native_format,
                    flags);
#else //ANDROID_R_AOSP and above
            p_window->m_surfaceControl =
                ctxt->m_surfaceComposer->createSurface(android::String8(window_params->debug_name),
                        w,
                        h,
                        p_window->native_format,
                        flags);
#endif
            if (0 == p_window->m_surfaceControl.get())
            {
                QCARCAM_ERRORMSG("createSurface() failed");
                return QCARCAM_RET_FAILED;
            }
        }
        else
        {
            QCARCAM_ERRORMSG("ctxt->m_surfaceComposer.get() failed");
            return QCARCAM_RET_FAILED;
        }

        android::SurfaceComposerClient::Transaction{}
        .setSize(p_window->m_surfaceControl, w, h)
            .setLayer(p_window->m_surfaceControl, 0x40000000)
            .setPosition(p_window->m_surfaceControl, x, y)
            .show(p_window->m_surfaceControl)
            .apply();

        sp<ANativeWindow> anw = p_window->m_surfaceControl->getSurface();
        p_window->nativeWindow = anw.get();

#ifdef QCC4x
        if (p_window->flags & QCARCAM_INPUT_FLAG_CONTENT_PROTECTED)
        {
            native_window_set_usage(p_window->nativeWindow, GRALLOC_USAGE_PROTECTED);
        }
#endif
        android_native_rect_t rect = {src_x, src_y, src_width, src_height};
        native_window_set_crop(p_window->nativeWindow, &rect);
#endif
    }

    p_window->width = window_params->buffer_size[0];
    p_window->height = window_params->buffer_size[1];

    return QCARCAM_RET_OK;
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
    if(p_window)
    {
        p_window->diag = diag;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_gpio_interrupt_config
///
/// @brief enable IO privileges, configure the gpio and set it up for interrupts
///
/// @param intr             Pointer for the IRQ to be stored
/// @param gpio_number      Specific gpio that is being utilized
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
/// @brief create a thread to handle the interrupt
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
/// @param p_window         window we want to use
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
/// @param p_window         window we want to use
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
