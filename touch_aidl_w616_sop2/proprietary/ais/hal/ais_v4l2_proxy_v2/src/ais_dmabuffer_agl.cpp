/* ===========================================================================
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 =========================================================================== */

#include <sys/mman.h>

#include "ais_log.h"
#include "qcarcam.h"
#include "ais_v4l2_proxy_util.h"

#include <hardware/gralloc.h>
#include "gbm.h"
#include "gbm_priv.h"
#include "gbmbuffer.h"

struct frame_buffer_t
{
    int n_buffers;
    int format;
    bool is_align;  // need align for display, if not used for display, doesn't need it. Default is 1
    uint32_t uilen;
    uint32_t uiw;
    uint32_t uih;
    uint32_t align_w;
    uint32_t align_h;

    GBMBuffer* gfx_bufs[QCARCAM_MAX_NUM_BUFFERS];
};

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

int v4l2_get_buf_info(frame_buffer_handle_t hndl, buf_info_t& info)
{
    frame_buffer_t* framebufs_ctxt = static_cast<frame_buffer_t*>(hndl);
    if (framebufs_ctxt == NULL)
    {
        return -1;
    }
    else
    {
        uint32 stride = framebufs_ctxt->uiw;
        info.w = framebufs_ctxt->uiw;
        info.h = framebufs_ctxt->uih;
        info.size = framebufs_ctxt->uilen;
        if (framebufs_ctxt->is_align)
        {
            stride = framebufs_ctxt->align_w;
        }

        info.bytesperline = GBMBuffer::get_bytesperline(framebufs_ctxt->format, stride);
    }

    return 0;
}

int init_gfx_buffers(frame_buffer_handle_t *phndl, uint32 w, uint32 h, uint32 format, bool is_align)
{
    struct gbm_buf_info bufInfo;
    int rc;
    frame_buffer_t* framebufs_ctxt;

    if (phndl == NULL)
    {
        return -1;
    }

    framebufs_ctxt = (frame_buffer_t*)malloc(sizeof(frame_buffer_t));
    if (NULL == framebufs_ctxt)
    {
        CAM_MSG(ERROR, "Failed to malloc memory for framebufs_ctxt");
        return -1;
    }

    memset(framebufs_ctxt, 0, sizeof(frame_buffer_t));
    *phndl = framebufs_ctxt;

    bufInfo.width = w;
    bufInfo.height = h;
    bufInfo.format = GBMBuffer::get_gbm_format(format);
    rc = gbm_perform(GBM_PERFORM_GET_BUFFER_SIZE_DIMENSIONS, &bufInfo, 0,
        &framebufs_ctxt->align_w, &framebufs_ctxt->align_h, &framebufs_ctxt->uilen);
    if (GBM_ERROR_NONE != rc)
    {
        free(framebufs_ctxt);
        CAM_MSG(ERROR,"Failed to GBM_PERFORM_GET_BUFFER_SIZE_DIMENSIONS");
        return -1;
    }

    framebufs_ctxt->uiw = w;
    framebufs_ctxt->uih = h;
    framebufs_ctxt->format = format;
    framebufs_ctxt->is_align = is_align;

    CAM_MSG(HIGH, "w %u h %u alignw %u alignh %u format %x align %d size %u",
        framebufs_ctxt->uiw, framebufs_ctxt->uih,
        framebufs_ctxt->align_w, framebufs_ctxt->align_h,
        format, is_align, framebufs_ctxt->uilen);

    return 0;
}

int set_gfx_buf_size(frame_buffer_handle_t hndl, uint32 w, uint32 h)
{
    int rc = 0;
    struct gbm_buf_info bufInfo;
    frame_buffer_t* framebufs_ctxt = static_cast<frame_buffer_t*>(hndl);
    if (framebufs_ctxt == NULL)
    {
        CAM_MSG(ERROR, "hndl is NULL");
        return -1;
    }

    bufInfo.width = w;
    bufInfo.height = h;
    bufInfo.format = GBMBuffer::get_gbm_format(framebufs_ctxt->format);
    rc = gbm_perform(GBM_PERFORM_GET_BUFFER_SIZE_DIMENSIONS, &bufInfo, 0,
        &framebufs_ctxt->align_w, &framebufs_ctxt->align_h, &framebufs_ctxt->uilen);
    if (GBM_ERROR_NONE != rc)
    {
        free(framebufs_ctxt);
        CAM_MSG(ERROR,"Failed to GBM_PERFORM_GET_BUFFER_SIZE_DIMENSIONS");
        return -1;
    }

    framebufs_ctxt->uiw = w;
    framebufs_ctxt->uih = h;

    CAM_MSG(HIGH, "w %u h %u alignw %u alignh %u format %x size %u",
        framebufs_ctxt->uiw, framebufs_ctxt->uih,
        framebufs_ctxt->align_w, framebufs_ctxt->align_h,
        framebufs_ctxt->format, framebufs_ctxt->uilen);

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
            framebufs_ctxt->uilen = GBMBuffer::get_bytesperline(framebufs_ctxt->format, framebufs_ctxt->uiw)
                * framebufs_ctxt->uih;
        }
        else
        {
           CAM_MSG(ERROR,"this format %x can't support config align", framebufs_ctxt->format);
           return -1;
        }
    }

    return 0;
}

int alloc_gfx_buffers(frame_buffer_handle_t hndl, qcarcam_buffers_t *qcarcam_buffers)
{
    frame_buffer_t* framebufs_ctxt = static_cast<frame_buffer_t*>(hndl);
    if (framebufs_ctxt == NULL)
    {
        CAM_MSG(ERROR, "hndl is NULL");
        return -1;
    }

    if (qcarcam_buffers)
    {
        framebufs_ctxt->n_buffers = qcarcam_buffers->n_buffers;

        for (int i = 0; i < framebufs_ctxt->n_buffers; i++)
        {
            if (framebufs_ctxt->gfx_bufs[i] == NULL)
            {
            // todo: when align = false, should use IonBuffer to allocate to avoid memory waste
                GBMBuffer* gbm_buf = new GBMBuffer(framebufs_ctxt->uiw,
                        framebufs_ctxt->uih,
                        framebufs_ctxt->format,
                        0);

                framebufs_ctxt->gfx_bufs[i] = gbm_buf;
                gbm_buf->fill_qcarcam_buffer(qcarcam_buffers->buffers[i], framebufs_ctxt->is_align, framebufs_ctxt->format);
            }
        }
    }
    else
    {
        CAM_MSG(ERROR, "qcarcam_buffers is NULL");
        return -1;
    }

    return 0;
}

int free_gfx_buffers(frame_buffer_handle_t hndl)
{
    frame_buffer_t* framebufs_ctxt = static_cast<frame_buffer_t*>(hndl);
    if (framebufs_ctxt)
    {
        for (int i = 0; i < framebufs_ctxt->n_buffers; i++)
        {
            if (framebufs_ctxt->gfx_bufs[i])
            {
                delete framebufs_ctxt->gfx_bufs[i];
                framebufs_ctxt->gfx_bufs[i] = NULL;
            }
        }
    }
    else
    {
        return -1;
    }

    return 0;
}

int deinit_gfx_buffers(frame_buffer_handle_t hndl)
{
    free(hndl);
    return 0;
}

void deinit_gfx_device()
{
    GBMBuffer::destory_gbm_device();
}


///////////////////////////////////////////////////////////////////////////////
/// qcarcam_dump_frame_buffer
///
/// @brief Dump frame to a file
///
/// @param ctxt             Pointer to display context
/// @param user_ctxt        Pointer to structure containing window parameters
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
        unsigned char *pBuf = (unsigned char*)framebufs_ctxt->gfx_bufs[idx]->getData();

        if (!pBuf)
        {
            VPROXY_ERROR("buffer is not mapped");
            return -1;
        }

        fp = fopen(filename, "w+");
        VPROXY_ERROR("dumping qcarcam frame %s numbytes %d", filename,
                framebufs_ctxt->gfx_bufs[idx]->getSize());

        if (0 != fp)
        {
            numByteToWrite = framebufs_ctxt->gfx_bufs[idx]->getSize();
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
            return -1;
        }
    }

    return 0;
}


