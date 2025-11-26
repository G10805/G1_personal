//*************************************************************************************************
// Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//*************************************************************************************************

#ifndef _CAMERA_GBM_BUFFER_H_
#define _CAMERA_GBM_BUFFER_H_

#include <hardware/gralloc.h>
#include "gbm.h"
#include "gbm_priv.h"
#include "qcarcam.h"

class GBMBuffer
{
private:
    struct PlaneInfo
    {
        uint32_t    _uiStride;
        uint32_t    _uiSize;
    };

    enum { MAX_PLANE_NUM = 3 };
public:
    GBMBuffer(uint32_t uiW, uint32_t uiH, uint32_t uiFormat, uint32_t uiReserve);
    ~GBMBuffer();

    native_handle_t* getNativeHandle(native_handle_t***);

    uint32_t getStride(uint32_t plane_idx=0) const    { return m_planes[plane_idx]._uiStride; }
    uint32_t getPlaneSize(uint32_t plane_idx=0) const       { return m_planes[plane_idx]._uiSize; }
    uint32_t getSize() const { return m_uiLen; }

    void* getData();

    uint32_t getHeight() const    { return m_uiHeight; }
    uint32_t getWidth() const     { return m_uiWidth;  }
    uint32_t getFormat() const    { return m_uiFormat; }
    uint32_t getUsage() const      { return m_uiUsage;  }

    int32_t lock(uint32_t inUsage, void** vaddr);
    void    unlock();

    void fill_qcarcam_buffer(qcarcam_buffer_t& qcarcam_buf, bool is_align, uint32 format);

    static void destory_gbm_device();
    static uint32_t get_gbm_format(uint32_t fmt);
    static uint32_t get_bytesperline(uint32 format, uint32 w);

private:
    uint32_t create_gbm_buffer(uint32_t uiW, uint32_t uiH, uint32_t uiFormat, uint32_t);
    void     destory_gbm_buffer(struct gbm_bo* bo, void* pData);

    static int32_t create_gbm_device();

private:
    native_handle_t*    m_pNativeHandle;

    uint32_t            m_uiHeight;
    uint32_t            m_uiWidth;

    uint32_t            m_uiLen;
    uint32_t            m_uiFormat;
    uint32_t            m_uiUsage;
    struct gbm_bo*      m_pbo;
    int                 m_fd;
    void*               m_pData;

    PlaneInfo           m_planes[MAX_PLANE_NUM];
    uint32_t            m_numPlanes;

    static int             s_drm_fd;
    static struct gbm_device *    s_gbm_dev;

};


#endif
