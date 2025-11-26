//*************************************************************************************************
// Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//*************************************************************************************************


#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>



#include <sys/time.h>
#include "ais_log.h"
#include "CameraResult.h"
#include "qcarcam_types.h"
#include "gbmbuffer.h"
#include <sys/mman.h>

uint32_t GBMBuffer::get_gbm_format(uint32_t fmt)
{
    uint32_t gbmfmt = GBM_FORMAT_UYVY;
    switch (fmt)
    {
    case QCARCAM_FMT_UYVY_8:
        gbmfmt = GBM_FORMAT_UYVY;
           break;
    case QCARCAM_FMT_NV12:
    case QCARCAM_FMT_MIPIRAW_12:
        gbmfmt = GBM_FORMAT_NV12;
        break;
    case QCARCAM_FMT_NV21:
        gbmfmt = GBM_FORMAT_NV21;    // GBM doesn't  support NV21 actually
        break;
    case QCARCAM_FMT_PLAIN16_12:
        gbmfmt = GBM_FORMAT_UYVY;    // 2bytes per pixel
        break;
    case QCARCAM_FMT_RGB_888:
        gbmfmt = GBM_FORMAT_BGR888;  // //TODO: can only support BRG888, need check
        break;
    case QCARCAM_FMT_MIPIRAW_8:
        gbmfmt = GBM_FORMAT_RAW8;
        CAM_MSG(HIGH, "format %x is relative to GBM_FORMAT_RAW8", fmt);
        break;
    default:
        CAM_MSG(ERROR,"unsupported format %x, use default UYVY", fmt);
        break;
    }

    return gbmfmt;
}

int GBMBuffer::s_drm_fd = -1;
struct gbm_device * GBMBuffer::s_gbm_dev = NULL;

GBMBuffer::GBMBuffer(uint32_t uiW, uint32_t uiH, uint32_t uiFormat, uint32_t uiUsage)
: m_pNativeHandle(NULL)
, m_uiHeight(0)
, m_uiWidth(0)
, m_uiLen(0)
, m_uiFormat(0)
, m_uiUsage(0)
, m_pbo(NULL)
, m_fd(-1)
, m_pData(NULL)
{
    memset(m_planes, 0, sizeof(m_planes));
    create_gbm_buffer(uiW, uiH, uiFormat, uiUsage);

}

GBMBuffer::~GBMBuffer()
{
    destory_gbm_buffer(m_pbo, m_pData);
    if (m_pNativeHandle)
    {
        free(m_pNativeHandle);
        m_pNativeHandle = NULL;
    }
}

native_handle_t* GBMBuffer::getNativeHandle(native_handle_t*** ppp)
{
    if (NULL == m_pNativeHandle)
    {
        m_pNativeHandle = (native_handle_t*)malloc(sizeof(native_handle_t) + sizeof(int));
        if (NULL == m_pNativeHandle)
        {
            CAM_MSG(ERROR, "Failed to malloc memory for m_pNativeHandle");
            return m_pNativeHandle;
        }

        m_pNativeHandle->numFds = 1;
        m_pNativeHandle->numInts = 0;
        m_pNativeHandle->data[0] = m_fd;
    }

    if (ppp)
    {
        *ppp = &m_pNativeHandle;
    }

    return m_pNativeHandle;
}

void* GBMBuffer::getData()
{
    if (m_pData == NULL)
    {
        lock(0, NULL);
    }

    return m_pData;
}

int32_t GBMBuffer::lock(uint32_t inUsage, void** vaddr)
{
    int32_t ret = 0;
    if (m_fd < 0)
    {
        ret = 1;
    }
    else
    {
        if (m_pData == NULL)
        {
            m_pData = mmap(NULL, m_uiLen, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
            if (m_pData == NULL)
            {
                CAM_MSG(ERROR, "Failed to mmap");
                ret = 1;
            }
        }

        if (vaddr)
        {
            *vaddr = m_pData;
        }
    }

    return ret;
}

void  GBMBuffer::unlock()
{
    if (m_pData)
    {
        munmap(m_pData, m_uiLen);
        m_pData = NULL;
    }
}

uint32 GBMBuffer::get_bytesperline(uint32 format, uint32 w)
{
    uint32 ret = 2 * w;
    switch (format)
    {
    case QCARCAM_FMT_UYVY_8:
        ret = 2 * w;
        break;
    case QCARCAM_FMT_RGB_888:
        ret = 3 * w;
        break;
    case QCARCAM_FMT_PLAIN16_12:
        ret = 2 * w;
        break;
    case QCARCAM_FMT_MIPIRAW_12:
        ret = 3 * w/2;
        break;
    case QCARCAM_FMT_MIPIRAW_8:
        ret = w;
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

void GBMBuffer::fill_qcarcam_buffer(QCarCamBuffer_t& qcarcam_buf, bool is_align, uint32 format)
{
    switch (format)
    {
    case QCARCAM_FMT_UYVY_8:
    case QCARCAM_FMT_RGB_888:
    case QCARCAM_FMT_PLAIN16_12:
    case QCARCAM_FMT_MIPIRAW_12:
    case QCARCAM_FMT_MIPIRAW_8:
        qcarcam_buf.numPlanes = 1;
        qcarcam_buf.planes[0].width = m_uiWidth;
        qcarcam_buf.planes[0].height = m_uiHeight;
        qcarcam_buf.planes[0].memHndl = (uint64_t)(uintptr_t)m_fd;
        qcarcam_buf.planes[0].stride = m_planes[0]._uiStride;
        qcarcam_buf.planes[0].size = m_uiLen;
        if (!is_align)
        {
            qcarcam_buf.planes[0].stride = get_bytesperline(format, m_uiWidth);
            qcarcam_buf.planes[0].size = qcarcam_buf.planes[0].stride * m_uiHeight;
            CAM_MSG(HIGH, "set stride=width for format %x %u %u", format, qcarcam_buf.planes[0].stride,
                qcarcam_buf.planes[0].size);
        }
        break;

    case QCARCAM_FMT_NV12:
    case QCARCAM_FMT_NV21:
        {
            qcarcam_buf.numPlanes = m_numPlanes;
            for (uint32_t i = 0; i < m_numPlanes; ++i)
            {
                qcarcam_buf.planes[i].width = m_uiWidth;
                qcarcam_buf.planes[i].height = m_uiHeight;
                qcarcam_buf.planes[i].memHndl = (uint64_t)(uintptr_t)m_fd;
                qcarcam_buf.planes[i].stride = m_planes[i]._uiStride;
                qcarcam_buf.planes[i].size = m_planes[i]._uiSize;
            }

            /* for the NV12 format, camera ISP HW fill the NV12 format, must follow some alignment
             * so don't support the align=false for NV12 format */
            if (!is_align)
            {
                CAM_MSG(ERROR, "NV12 must follow the alignment");
            }
        }
        break;
    default:
        CAM_MSG(ERROR,"unsupported format %x", format);
        break;

    }
}

uint32_t GBMBuffer::create_gbm_buffer(uint32_t uiW, uint32_t uiH, uint32_t uiFormat, uint32_t uiUsage)
{
    int rc = -1;
    struct gbm_bo* bo = NULL;
    int fd = -1;
    uint32_t align_w = 0;
    uint32_t align_h = 0;
    struct gbm_buf_info bufInfo;
    uint32_t uilen = 0;
    generic_buf_layout_t layout;

    if (s_gbm_dev == NULL)
    {
        if (create_gbm_device())
        {
            return 1;
        }
    }

    uint32_t gbmformat = get_gbm_format(uiFormat);

    bo = gbm_bo_create(s_gbm_dev, uiW, uiH, gbmformat, uiUsage);
    if (bo == NULL)
    {
        CAM_MSG(ERROR, "Failed to gbm_bo_create");
        return 1;
    }

    m_pbo = bo;
#ifdef USE_GBM_BACKEND
    fd = gbm_bo_get_fd(bo);
#else
    fd = bo->ion_fd;
#endif
    CAM_MSG(HIGH, "fd %d", fd);
    if (fd < 0)
    {
        CAM_MSG(ERROR, "Failed to gbm_bo_get_fd");
        return 1;
    }

    m_fd = fd;
    bufInfo.width = uiW;
    bufInfo.height = uiH;
    bufInfo.format = gbmformat;
    rc = gbm_perform(GBM_PERFORM_GET_BUFFER_SIZE_DIMENSIONS, &bufInfo, 0, &align_w, &align_h, &uilen);
    if (GBM_ERROR_NONE != rc)
    {
        CAM_MSG(ERROR,"Failed to GBM_PERFORM_GET_BUFFER_SIZE_DIMENSIONS");
        return 1;
    }

    rc = gbm_perform(GBM_PERFORM_GET_PLANE_INFO, bo, &layout);
    if (GBM_ERROR_NONE != rc)
    {
        CAM_MSG(ERROR,"Failed to get plane info");
        return 1;
    }
    else
    {
        m_numPlanes = layout.num_planes;
    }

    switch (m_numPlanes)
    {
    case 3:
        m_planes[2]._uiStride = layout.planes[2].v_increment;
        m_planes[1]._uiStride = layout.planes[1].v_increment;
        m_planes[0]._uiStride = layout.planes[0].v_increment;

        m_planes[2]._uiSize = uilen - layout.planes[2].offset;
        m_planes[1]._uiSize = layout.planes[2].offset - layout.planes[1].offset;
        m_planes[0]._uiSize = layout.planes[1].offset - layout.planes[0].offset;
        CAM_MSG(HIGH,"plane2 size2 %u stride2 %u size1 %u stride1 %u size0 %u stride0 %u",
            m_planes[2]._uiSize, m_planes[2]._uiStride,
            m_planes[1]._uiSize, m_planes[1]._uiStride,
            m_planes[0]._uiSize, m_planes[0]._uiStride);
        break;
    case 2:
        m_planes[1]._uiStride  = layout.planes[1].v_increment;
        m_planes[0]._uiStride = layout.planes[0].v_increment;

        m_planes[1]._uiSize = uilen - layout.planes[1].offset;
        m_planes[0]._uiSize = layout.planes[1].offset - layout.planes[0].offset;

        CAM_MSG(HIGH,"size1 %u stride1 %u size0 %u stride0 %u",
            m_planes[1]._uiSize, m_planes[1]._uiStride,
            m_planes[0]._uiSize, m_planes[0]._uiStride);
        break;
    case 1:
        m_planes[0]._uiStride = layout.planes[0].v_increment;
        m_planes[0]._uiSize = uilen;
        CAM_MSG(HIGH, "size0 %u stride0 %u",
            m_planes[0]._uiSize, m_planes[0]._uiStride);
        break;
    default:

        CAM_MSG(ERROR, "plane number %d, should be 1/2/3", layout.num_planes);
        return 1;
    }

    m_uiWidth = uiW;
    m_uiHeight = uiH;
    m_uiFormat = gbmformat;
    m_uiUsage = uiUsage;
    m_uiLen = uilen;

    return 0;
}

void GBMBuffer::destory_gbm_buffer(struct gbm_bo* bo, void* pData)
{
    if (pData)
    {
        munmap(pData, m_uiLen);
    }

    if (-1 < m_fd)
    {
#ifdef USE_GBM_BACKEND
        close(m_fd);
#endif
    }

    if (bo)
    {
        gbm_bo_destroy(bo);
    }
}

int32_t GBMBuffer::create_gbm_device()
{
    s_drm_fd = open("/dev/dri/renderD128", O_RDWR | O_CLOEXEC);
    if (s_drm_fd < 0)
    {
        CAM_MSG(ERROR, "/dev/dri/renderD128 open failed: %d", s_drm_fd);
        return 1;
    }

    s_gbm_dev = gbm_create_device(s_drm_fd);
    if (s_gbm_dev == NULL)
    {
        CAM_MSG(ERROR, "gbm_create_device failed");
        return 1;
    }

    return 0;
}


void GBMBuffer::destory_gbm_device()
{
    if (s_gbm_dev)
    {
        gbm_device_destroy(s_gbm_dev);
        s_gbm_dev = NULL;
    }

    close(s_drm_fd);
}


