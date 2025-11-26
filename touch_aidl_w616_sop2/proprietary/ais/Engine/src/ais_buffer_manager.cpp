/*!
 * Copyright (c) 2016-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ais_buffer_manager.h"
#include "CameraPlatform.h"

static uint32 GetBufferFlags(uint32 qcarcamFlags)
{
    uint32 flags = 0x0;

    if (qcarcamFlags & QCARCAM_BUFFER_FLAG_SECURE)
    {
        flags |= CAMERA_BUFFER_FLAG_SECURE;
    }
    if (qcarcamFlags & QCARCAM_BUFFER_FLAG_CACHE)
    {
        flags |= CAMERA_BUFFER_FLAG_CACHED;
    }
    if (qcarcamFlags & QCARCAM_BUFFER_FLAG_OS_HNDL)
    {
        flags |= CAMERA_BUFFER_FLAG_HNDL;
    }

    return flags;
}

/**
 * MapBuffers
 *
 * @brief Map buffers to a buffer_list
 *
 * @param pBufferList - buffer list to hold mapping
 * @param pQcarcamBuffers - buffers to map
 *
 * @return CameraResult
 */
CameraResult AisBufferManager::MapBuffers(AisBufferList* pBufferList,
        qcarcam_buffers_t* pQcarcamBuffers)
{
    CameraHwBlockType hwBlock = CAMERA_HWBLOCK_IFE;
    CameraResult rc = CAMERA_SUCCESS;
    int i = 0;

    for (i = 0; i < (int)pQcarcamBuffers->n_buffers; i++)
    {
        AisBuffer* pBuffer = &pBufferList->m_pBuffers[i];

        pBuffer->idx = i;
        pBuffer->pMemHndl = pQcarcamBuffers->buffers[i].planes[0].p_buf;
        pBuffer->colorFmt = pQcarcamBuffers->color_fmt;

        pBuffer->flags = GetBufferFlags(pQcarcamBuffers->flags);

        if (pQcarcamBuffers->buffers[i].n_planes <= QCARCAM_MAX_NUM_PLANES)
        {
            pBuffer->size = 0;
            pBuffer->bufferInfo.n_planes = pQcarcamBuffers->buffers[i].n_planes;
            for (uint32 j = 0; j < pQcarcamBuffers->buffers[i].n_planes; ++j)
            {
                pBuffer->bufferInfo.planes[j].width = pQcarcamBuffers->buffers[i].planes[j].width;
                pBuffer->bufferInfo.planes[j].height = pQcarcamBuffers->buffers[i].planes[j].height;
                pBuffer->bufferInfo.planes[j].stride = pQcarcamBuffers->buffers[i].planes[j].stride;
                pBuffer->bufferInfo.planes[j].size = pQcarcamBuffers->buffers[i].planes[j].size;
                pBuffer->bufferInfo.planes[j].hndl = (unsigned long long)pQcarcamBuffers->buffers[i].planes[j].p_buf;
                pBuffer->bufferInfo.planes[j].offset = pBuffer->size;

                pBuffer->size += pQcarcamBuffers->buffers[i].planes[j].size;
            }
        }

        AIS_LOG(ENGINE, MED, "Mapping %d (%p %d)", i, pBuffer->pMemHndl, pBuffer->size);

        rc = CameraBufferMap(pBuffer, pBuffer->flags, 0x0, &hwBlock, 1);
        if (CAMERA_SUCCESS != rc)
        {
            AIS_LOG(ENGINE, ERROR, "Failed to map buffer idx %d (%p %d)", i, pBuffer->pMemHndl, pBuffer->size);

            /*cleanup by unmapping previously mapped buffers*/
            while (i > 0)
            {
                i--;
                CameraBufferUnmap(&pBufferList->m_pBuffers[i]);
            }
            std_memset(pBufferList->m_pBuffers, 0x0, sizeof(pBufferList->m_pBuffers));
            rc = CAMERA_EFAILED;
            break;
        }

        pBuffer->bufferInfo.n_planes = pQcarcamBuffers->buffers[i].n_planes;
        pBuffer->bufferInfo.n_planes = pQcarcamBuffers->buffers[i].n_planes;
        pBuffer->pDa = CameraBufferGetDeviceAddress(pBuffer, hwBlock);
        pBuffer->isInternal = FALSE;
        pBuffer->state = AIS_BUFFER_INITIALIZED;
    }

    if (CAMERA_SUCCESS == rc)
    {
        pBufferList->SetProperties(pQcarcamBuffers->n_buffers,
                pQcarcamBuffers->buffers[0].planes[0].width,
                pQcarcamBuffers->buffers[0].planes[0].height,
                pQcarcamBuffers->color_fmt);
        AIS_LOG(ENGINE, HIGH, "Successfully mapped %d buffers", pBufferList->m_nBuffers);
    }

    return rc;
}

/**
 * MapBuffers
 *
 * @brief Map buffers to a buffer_list
 *
 * @param pBufferList - buffer list to hold mapping
 * @param pQcarcamBuffers - buffers to map
 *
 * @return CameraResult
 */
CameraResult AisBufferManager::MapBuffersV2(AisBufferList* pBufferList,
        const qcarcam_bufferlist_t* pQcarcamBuffers)
{
    CameraHwBlockType hwBlock = CAMERA_HWBLOCK_IFE;
    CameraResult rc = CAMERA_SUCCESS;
    int i = 0;

    for (i = 0; i < (int)pQcarcamBuffers->n_buffers; i++)
    {
        AisBuffer* pBuffer = &pBufferList->m_pBuffers[i];

        pBuffer->idx = i;
        pBuffer->pMemHndl = (void*)pQcarcamBuffers->buffers[i].planes[0].hndl;
        pBuffer->colorFmt = pQcarcamBuffers->color_fmt;

        pBuffer->flags = GetBufferFlags(pQcarcamBuffers->flags);

        pBuffer->size = pQcarcamBuffers->buffers[i].planes[0].size;
        if (pQcarcamBuffers->buffers[i].n_planes <= QCARCAM_MAX_NUM_PLANES)
        {
            for (uint32 j = 1; j < pQcarcamBuffers->buffers[i].n_planes; ++j)
            {
                pBuffer->size += pQcarcamBuffers->buffers[i].planes[j].size;
            }
        }

        AIS_LOG(ENGINE, MED, "Mapping %d (%p %d)", i, pBuffer->pMemHndl, pBuffer->size);

        rc = CameraBufferMap(pBuffer, pBuffer->flags, 0x0, &hwBlock, 1);

        if (CAMERA_SUCCESS != rc)
        {
            AIS_LOG(ENGINE, ERROR, "Failed to map buffer idx %d (%p %d)", i, pBuffer->pMemHndl, pBuffer->size);

            /*cleanup by unmapping previously mapped buffers*/
            while (i > 0)
            {
                i--;
                CameraBufferUnmap(&pBufferList->m_pBuffers[i]);
            }
            std_memset(pBufferList->m_pBuffers, 0x0, sizeof(pBufferList->m_pBuffers));
            rc = CAMERA_EFAILED;
            break;
        }

        pBuffer->bufferInfo = pQcarcamBuffers->buffers[i];
        pBuffer->pDa = CameraBufferGetDeviceAddress(pBuffer, hwBlock);
        pBuffer->isInternal = FALSE;
        pBuffer->state = AIS_BUFFER_INITIALIZED;
    }

    if (CAMERA_SUCCESS == rc)
    {
        pBufferList->SetProperties(pQcarcamBuffers->n_buffers,
                pQcarcamBuffers->buffers[0].planes[0].width,
                pQcarcamBuffers->buffers[0].planes[0].height,
                pQcarcamBuffers->color_fmt);
        AIS_LOG(ENGINE, HIGH, "Successfully mapped %d buffers", pBufferList->m_nBuffers);
    }

    return rc;
}

/**
 * UnmapBuffers
 *
 * @brief UnMap buffers from  buffer list
 *
 * @param pBufferList
 *
 * @return CameraResult
 */
CameraResult AisBufferManager::UnmapBuffers(AisBufferList* pBufferList)
{
    uint32 i = 0;

    for (i = 0; i < pBufferList->m_nBuffers; i++)
    {
        CameraBufferUnmap(&pBufferList->m_pBuffers[i]);
    }
    std_memset(pBufferList->m_pBuffers, 0x0, sizeof(pBufferList->m_pBuffers));
    pBufferList->m_nBuffers = 0;

    return CAMERA_SUCCESS;
}
