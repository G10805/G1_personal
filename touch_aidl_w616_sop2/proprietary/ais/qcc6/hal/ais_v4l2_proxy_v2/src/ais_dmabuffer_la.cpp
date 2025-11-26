/* ===========================================================================
 * Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 =========================================================================== */

#include <sys/mman.h>

#include "ais_log.h"
#include "qcarcam.h"
#include "ais_v4l2_proxy_util.h"

#include "video_color_fmt.h"

#include <QtiGralloc.h>
#include <ui/GraphicBuffer.h>
#include <EGL/egl.h>
#include <android/hardware/graphics/mapper/4.0/IMapper.h>
using Error_v4       = ::android::hardware::graphics::mapper::V4_0::Error;
using IMapper_v4     = ::android::hardware::graphics::mapper::V4_0::IMapper;

using namespace android;
sp<IMapper_v4> mapper = IMapper_v4::getService();

typedef struct
{
    int fd;
    void* ptr;
    int size;
    ANativeWindowBuffer* window_buf;
}ais_buffer_t;


struct frame_buffer_t
{
    int n_buffers;
    int format;
    bool is_align;  // need align for display, if not used for display, doesn't need it. Default is 1
    uint32_t uilen;
    uint32_t uiw;
    uint32_t uih;
    sp<GraphicBuffer> *gfx_bufs;

    ais_buffer_t* buffers;
};

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

static bool is_support_config_align(uint32 format)
{
    if (QCARCAM_FMT_UYVY_8 == format ||
        QCARCAM_FMT_RGB_888 == format ||
        QCARCAM_FMT_PLAIN16_12 == format ||
        QCARCAM_FMT_MIPIRAW_12 == format ||
        QCARCAM_FMT_MIPIRAW_8 == format)
    {
        return true;
    }

    return false;
}

uint32 get_stride(uint32 format, uint32 w)
{
    uint32 ret = 2 * w;
    switch (format)
    {
        case QCARCAM_FMT_UYVY_8: ret = 2 * w;
                                 break;
        case QCARCAM_FMT_RGB_888: ret = 3 * w;
                                  break;
        case QCARCAM_FMT_PLAIN16_12: ret = 2 * w;
                                    break;
        case QCARCAM_FMT_MIPIRAW_12: ret = 3 * w/2;
                                     break;
        case QCARCAM_FMT_MIPIRAW_8:  ret = w;
                                    break;
        case QCARCAM_FMT_NV12:
        case QCARCAM_FMT_NV21:
            ret = w;
            break;
        default:
            CAM_MSG(ERROR,"unsupported format %x, use UYVY", format);
            break;
    }
    return ret;
}

static void fill_qcarcam_buffer_params(QCarCamBuffer_t* pBuffer, QCarCamColorFmt_e fmt)
{
    switch (fmt)
    {
    case QCARCAM_FMT_RGB_888:
        pBuffer->planes[0].stride = pBuffer->planes[0].width * 3;
        pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        break;
    case QCARCAM_FMT_MIPIRAW_8:
        pBuffer->planes[0].stride = pBuffer->planes[0].width;
        pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        break;
    case QCARCAM_FMT_MIPIRAW_10:
        if (0 == (pBuffer->planes[0].width % 4))
        {
            pBuffer->planes[0].stride = pBuffer->planes[0].width * 5 / 4;
            pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        }
        break;
    case QCARCAM_FMT_MIPIRAW_12:
        if (0 == (pBuffer->planes[0].width % 2))
        {
            pBuffer->planes[0].stride = pBuffer->planes[0].width * 3 / 2;
            pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        }
        break;
    case QCARCAM_FMT_UYVY_8:
        pBuffer->planes[0].stride = pBuffer->planes[0].width * 2;
        pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        break;
    case QCARCAM_FMT_UYVY_10:
        pBuffer->planes[0].stride = pBuffer->planes[0].width * 5 / 4;
        pBuffer->planes[0].size = pBuffer->planes[0].stride * pBuffer->planes[0].height;
        break;
    case QCARCAM_FMT_UYVY_12:
        if (0 == (pBuffer->planes[0].width % 2))
        {
            pBuffer->planes[0].stride = pBuffer->planes[0].width * 2 * 3 / 2;
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

#ifndef USE_NV12_HEIGHT_ALIGNED
        //plane 0 is 4k aligned
        pBuffer->planes[0].size = TESTUTIL_ALIGN(pBuffer->planes[0].size, 4096);
#endif
        break;
    }
    default:
        QCARCAM_ERRORMSG("Unknown format 0x%x", fmt);
        pBuffer->planes[0].size = 0;
        break;
    }
}


static int qcarcam_to_native_color_format(QCarCamColorFmt_e fmt)
{
    switch(fmt)
    {
        case QCARCAM_FMT_UYVY_8:
        case QCARCAM_FMT_UYVY_10:
        case QCARCAM_FMT_UYVY_12:
            return HAL_PIXEL_FORMAT_CbYCrY_422_I;
        case QCARCAM_FMT_YUYV_8:
        case QCARCAM_FMT_YUYV_10:
        case QCARCAM_FMT_YUYV_12:
            return HAL_PIXEL_FORMAT_YCBCR_422_I;
        case QCARCAM_FMT_RGB_888:
            return HAL_PIXEL_FORMAT_RGB_888;
        case QCARCAM_FMT_MIPIRAW_8:
            return HAL_PIXEL_FORMAT_RAW8;
        case QCARCAM_FMT_MIPIRAW_10:
            return HAL_PIXEL_FORMAT_RAW10;
        case QCARCAM_FMT_MIPIRAW_12:
            return HAL_PIXEL_FORMAT_RAW12;
        case QCARCAM_FMT_NV12:
            return HAL_PIXEL_FORMAT_YCbCr_420_SP;
        default:
            return HAL_PIXEL_FORMAT_CbYCrY_422_I;
            break;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// v4l2_get_size
///
/// @brief Get Buffer size
///
/// @param hndl             Pointer to structure containing buffer parameters
///
/// @return size if successful
///////////////////////////////////////////////////////////////////////////////
unsigned int v4l2_get_size(frame_buffer_handle_t hndl)
{
    frame_buffer_t* framebufs_ctxt = static_cast<frame_buffer_t*>(hndl);
    if (framebufs_ctxt == NULL)
    {
        return -1;
    }
    else
    {
        return framebufs_ctxt->uilen;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// v4l2_get_buf_info
///
/// @brief Update buffer info into buf_info_t
///
/// @param hndl             Pointer to structure containing buffer parameters
/// @param info             buf_info_t struct
///
/// @return 0 if successful
///////////////////////////////////////////////////////////////////////////////
int v4l2_get_buf_info(frame_buffer_handle_t hndl, buf_info_t& info)
{
    frame_buffer_t* framebufs_ctxt = static_cast<frame_buffer_t*>(hndl);
    if (framebufs_ctxt == NULL)
    {
        return -1;
    }
    else
    {
        info.w = framebufs_ctxt->uiw;
        info.h = framebufs_ctxt->uih;
        info.size = framebufs_ctxt->uilen;
        info.bytesperline = get_stride(framebufs_ctxt->format, framebufs_ctxt->uiw);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// init_gfx_buffers
///
/// @brief init function for gfx buffers in LA
///
/// @param phndl             Pointer to structure containing buffer parameters
///
/// @return 0 if successful
///////////////////////////////////////////////////////////////////////////////
int init_gfx_buffers(frame_buffer_handle_t *phndl, uint32 w, uint32 h, uint32 format, bool is_align)
{
    int rc;
    frame_buffer_t* framebufs_ctxt;

    if (phndl == NULL)
    {
        return -1;
    }

    framebufs_ctxt = (frame_buffer_t*)malloc(sizeof(frame_buffer_t));
    if (!framebufs_ctxt)
    {
        QCARCAM_ERRORMSG("framebufs_ctxt is null");
        return -1;
    }
    memset(framebufs_ctxt, 0, sizeof(frame_buffer_t));
    *phndl = framebufs_ctxt;

    QCarCamBuffer_t* buffer = (QCarCamBuffer_t*)malloc(sizeof(QCarCamBuffer_t));

    if (!buffer)
    {
        QCARCAM_ERRORMSG("buffer is null");
        return -1;
    }
    memset(buffer, 0, sizeof(QCarCamBuffer_t));
    buffer->planes[0].width = w;
    buffer->planes[0].height = h;
    fill_qcarcam_buffer_params(buffer, (QCarCamColorFmt_e)format);

    framebufs_ctxt->uiw = w;
    framebufs_ctxt->uih = h;
    framebufs_ctxt->format = format;
    framebufs_ctxt->is_align = is_align;
    framebufs_ctxt->uilen = buffer->planes[0].size;

    CAM_MSG(HIGH, "w %u h %u format %x align %d size %u",
        framebufs_ctxt->uiw, framebufs_ctxt->uih,
        format, is_align, framebufs_ctxt->uilen);
    free(buffer);

    return 0;
}

int set_buf_align(frame_buffer_handle_t hndl, bool is_buf_align)
{
    frame_buffer_t* framebufs_ctxt = static_cast<frame_buffer_t*>(hndl);
    if (framebufs_ctxt == NULL)
    {
        CAM_MSG(ERROR, "hndl is NULL");
        return -1;
    }

    if (!is_buf_align)
    {
        if (is_support_config_align(framebufs_ctxt->format))
        {
            framebufs_ctxt->is_align = false;
            framebufs_ctxt->uilen = get_stride(framebufs_ctxt->format, framebufs_ctxt->uiw) * framebufs_ctxt->uih;
        }
        else
        {
           CAM_MSG(ERROR,"this format %x can't support config align", framebufs_ctxt->format);
           return -1;
        }
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// alloc_gfx_buffers
///
/// @brief Allocates gfx buffers
///
/// @param hndl             Pointer to structure containing buffer parameters
/// @param qcarcam_buffers  Pointer to QCarCamBufferList_t
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
int alloc_gfx_buffers(frame_buffer_handle_t hndl, QCarCamBufferList_t *qcarcam_buffers)
{
    int rc = 0, fd, i;
    uint64_t graphicsUsage;
    uint32_t allocSize, size;

    frame_buffer_t* framebufs_ctxt = static_cast<frame_buffer_t*>(hndl);
    if (framebufs_ctxt == NULL)
    {
        CAM_MSG(ERROR, "hndl is NULL");
        return -1;
    }

    if (qcarcam_buffers)
    {

        framebufs_ctxt->n_buffers = qcarcam_buffers->nBuffers;
        framebufs_ctxt->buffers = (ais_buffer_t*)calloc(framebufs_ctxt->n_buffers, sizeof(*framebufs_ctxt->buffers));
        if(NULL == framebufs_ctxt->buffers)
        {
            QCARCAM_ERRORMSG("Value of framebufs_ctxt->buffers is NULL");
            return QCARCAM_RET_NOMEM;
        }
        graphicsUsage  = android::GraphicBuffer::USAGE_SW_READ_OFTEN |
                         android::GraphicBuffer::USAGE_SW_WRITE_OFTEN |
                         android::GraphicBuffer::USAGE_HW_TEXTURE;
        const int nativeBufferFormat = qcarcam_to_native_color_format((QCarCamColorFmt_e)qcarcam_buffers->colorFmt);
        framebufs_ctxt->gfx_bufs = (sp<GraphicBuffer>*)calloc(framebufs_ctxt->n_buffers, sizeof(sp<GraphicBuffer>));
        if(!framebufs_ctxt->gfx_bufs)
        {
            QCARCAM_ERRORMSG("Failed to allocate GraphicBuffer lists");
            return QCARCAM_RET_NOMEM;
        }

        for (i = 0; i < framebufs_ctxt->n_buffers; i++)
        {
            fill_qcarcam_buffer_params(&qcarcam_buffers->pBuffers[i], qcarcam_buffers->colorFmt);
            framebufs_ctxt->buffers[i].size = qcarcam_buffers->pBuffers[i].planes[0].size;
            allocSize = framebufs_ctxt->buffers[i].size;
            framebufs_ctxt->uilen = framebufs_ctxt->buffers[i].size;

            framebufs_ctxt->gfx_bufs[i] = new GraphicBuffer(allocSize,
                    1,
                    nativeBufferFormat,
                    1,
                    graphicsUsage);

            if (!framebufs_ctxt->gfx_bufs[i])
            {
                QCARCAM_ERRORMSG("Failed to allocate buffer %dx%d (fmt=0x%x) %d",
                    framebufs_ctxt->uiw, framebufs_ctxt->uih, qcarcam_buffers->colorFmt, allocSize);
                rc = QCARCAM_RET_FAILED;

                while (i)
                {
                    i--;
                    munmap(framebufs_ctxt->buffers[i].ptr, (unsigned int)get_window_buffer_size(framebufs_ctxt->buffers[i].window_buf));
                }

                goto EXIT;
            }

            framebufs_ctxt->buffers[i].window_buf = framebufs_ctxt->gfx_bufs[i]->getNativeBuffer();
            fd = get_window_buffer_fd(framebufs_ctxt->buffers[i].window_buf);
            qcarcam_buffers->pBuffers[i].planes[0].memHndl = (uint64_t)(uintptr_t)fd;
            framebufs_ctxt->buffers[i].fd = fd;

            size = get_window_buffer_size(framebufs_ctxt->buffers[i].window_buf);
            QCARCAM_DBGMSG("%d %dx%d (%d|%d) fd=%d (fmt=0x%x)",
                i,
                framebufs_ctxt->uiw,
                framebufs_ctxt->uih,
                allocSize,
                size,
                fd,
                qcarcam_buffers->colorFmt);

            framebufs_ctxt->buffers[i].ptr = mmap(NULL, size,
                                PROT_READ | PROT_WRITE, MAP_SHARED,
                                fd, 0);
            if (framebufs_ctxt->buffers[i].ptr == MAP_FAILED)
            {
                QCARCAM_ERRORMSG("Failed to map buffer %d (size=%d)", i, size);
                rc = QCARCAM_RET_FAILED;

                while (i)
                {
                    i--;
                    munmap(framebufs_ctxt->buffers[i].ptr, size);
                }

                goto EXIT;
            }

        }
    }
    else
    {
        CAM_MSG(ERROR, "qcarcam_buffers is NULL");
        rc = -1;
    }
EXIT:
    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// free_gfx_buffers
///
/// @brief frees allocation gfx buffers
///
/// @param hndl             Pointer to structure containing buffer parameters
///
/// @return 0 if successful
///////////////////////////////////////////////////////////////////////////////
int free_gfx_buffers(frame_buffer_handle_t hndl)
{
    frame_buffer_t* framebufs_ctxt = static_cast<frame_buffer_t*>(hndl);
    if (framebufs_ctxt)
    {
        for (int i = 0; i < framebufs_ctxt->n_buffers; i++)
        {
            if (framebufs_ctxt->buffers[i].window_buf->handle)
            {
                munmap(framebufs_ctxt->buffers[i].ptr,
                        get_window_buffer_size(framebufs_ctxt->buffers[i].window_buf));
                if (framebufs_ctxt->buffers[i].fd >= 0)
                {
                    close(framebufs_ctxt->buffers[i].fd);
                    framebufs_ctxt->buffers[i].fd = -1;
                }
                if (framebufs_ctxt->gfx_bufs && framebufs_ctxt->gfx_bufs[i])
                {
                    framebufs_ctxt->gfx_bufs[i] = nullptr;
                }
            }
        }
        return 0;
    }
    else
    {
        return -1;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// deinit_gfx_buffers
///
/// @brief deinit gfx buffer
///
/// @param hndl             Pointer to structure containing buffer parameters
///
/// @return 0 if successful
///////////////////////////////////////////////////////////////////////////////
int deinit_gfx_buffers(frame_buffer_handle_t hndl)
{
    frame_buffer_t* framebufs_ctxt = static_cast<frame_buffer_t*>(hndl);
    if (framebufs_ctxt)
    {
        if (framebufs_ctxt->gfx_bufs)
        {
            free(framebufs_ctxt->gfx_bufs);
        }
    }
    free(hndl);
    return 0;
}

void deinit_gfx_device()
{
    return;
}


///////////////////////////////////////////////////////////////////////////////
/// qcarcam_dump_frame_buffer
///
/// @brief Dump frame to a file
///
/// @param hndl             Pointer to structure containing buffer parameters
/// @param idx              Frame ID number
/// @param filename         Char pointer to file name to be dumped
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
int qcarcam_dump_frame_buffer(frame_buffer_handle_t hndl, unsigned int idx, const char *filename)
{
    frame_buffer_t* framebufs_ctxt = static_cast<frame_buffer_t*>(hndl);
    if (framebufs_ctxt)
    {
        FILE *fp;
        size_t numBytesWritten = 0;
        size_t numByteToWrite = 0;
        unsigned char *pBuf = 0;
        if (!framebufs_ctxt->buffers[idx].ptr)
        {
            QCARCAM_ERRORMSG("buffer is not mapped");
            return QCARCAM_RET_FAILED;
        }

        fp = fopen(filename, "w+");
        VPROXY_ERROR("dumping qcarcam frame %s numbytes %d", filename,
                framebufs_ctxt->buffers[idx].size);

        if (0 != fp)
        {
            pBuf = (unsigned char *)framebufs_ctxt->buffers[idx].ptr;
            numByteToWrite = framebufs_ctxt->buffers[idx].size;
            numBytesWritten = fwrite(pBuf, 1, numByteToWrite, fp);
            if (numBytesWritten != numByteToWrite)
            {
                VPROXY_ERROR("error no data written to file");
            }
            fclose(fp);
        }
        else
        {
            VPROXY_ERROR("failed to open file");
            return QCARCAM_RET_FAILED;
        }
    }

    return QCARCAM_RET_OK;
}
