#ifndef _AIS_BUFFER_MANAGER_H_
#define _AIS_BUFFER_MANAGER_H_

/*!
 * Copyright (c) 2016-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ais_engine.h"

class AisBufferManager
{
public:
    /**
     * MapBuffers
     *
     * @brief Map buffers to a pUsrCtxt buffer list
     *
     * @param pBufferList - buffer list to hold mapping
     * @param pQcarcamBuffers - buffers to map
     *
     * @return CameraResult
     */
    static CameraResult MapBuffers(AisBufferList* pBufferList, qcarcam_buffers_t* pBuffers);

    /**
     * MapBuffers
     *
     * @brief Map buffers to a pUsrCtxt buffer list
     *
     * @param pBufferList - buffer list to hold mapping
     * @param pQcarcamBuffers - buffers to map
     *
     * @return CameraResult
     */
    static CameraResult MapBuffersV2(AisBufferList* pBufferList, const qcarcam_bufferlist_t* pBuffers);

    /**
     * UnmapBuffers
     *
     * @brief UnMap buffers from a buffer list
     *
     * @param pBufferList
     *
     * @return CameraResult
     */
    static CameraResult UnmapBuffers(AisBufferList* pBufferList);
};


#endif /* _AIS_BUFFER_MANAGER_H_ */
