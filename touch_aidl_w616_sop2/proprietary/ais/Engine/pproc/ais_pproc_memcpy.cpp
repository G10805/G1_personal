/*!
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ais_pproc_memcpy.h"

#include "ais_engine.h"
#include "CameraPlatform.h"

#define PPROC_LOG(lvl, fmt...) AIS_LOG(PPROC_MEMCPY, lvl, fmt)

AisPProcMemcpy* AisPProcMemcpy::m_pNodeInstance = nullptr;

/**
 * ProcessEvent
 *
 * @brief Process Event
 *
 * @param pUsrCtxt
 * @param pEvent
 *
 * @return CameraResult
 */
CameraResult AisPProcMemcpy::ProcessEvent(AisUsrCtxt* pUsrCtxt, AisEventMsgType* pEvent)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisEventPProcJobType* pJob = &pEvent->payload.pprocJob;
    AisBufferList* pBufferListIn = pUsrCtxt->m_bufferList[pJob->pProcChain->inBuflistId[0]];
    AisBufferList* pBufferListOut = pUsrCtxt->m_bufferList[pJob->pProcChain->outBuflistId[0]];

    pUsrCtxt->Lock();

    //context must be streaming
    if (pUsrCtxt->m_state != AIS_USR_STATE_STREAMING)
    {
        AIS_LOG(ENGINE, ERROR, "Context %p in bad state %d", pUsrCtxt, pUsrCtxt->m_state);
        pUsrCtxt->Unlock();
        return CAMERA_EBADSTATE;
    }

    AisBuffer* pInBuffer = pBufferListIn->GetReadyBuffer(pJob->jobId);
    if (!pInBuffer)
    {
        AIS_LOG(ENGINE, ERROR, "Context %p cannot find input buffer for jobId 0x%llx", pUsrCtxt, pJob->jobId);
        pUsrCtxt->Unlock();
        return CAMERA_ENOREADYBUFFER;
    }

    AisBuffer* pOutBuffer = pBufferListOut->GetFreeBuffer(pUsrCtxt);
    if (pOutBuffer)
    {
        memcpy(pOutBuffer->pVa, pInBuffer->pVa, STD_MIN(pOutBuffer->size, pInBuffer->size));

        pBufferListOut->QueueReadyBuffer(pJob->jobId, pOutBuffer);
    }
    else
    {
        PPROC_LOG(ERROR, "No free buffers (error %d), dropped frame!", rc);
        rc = CAMERA_EFAILED;
    }

    pBufferListIn->SetBufferState(pInBuffer->idx, AIS_BUFFER_INITIALIZED);
    pBufferListIn->ReturnBuffer(pUsrCtxt, pInBuffer->idx);

    pUsrCtxt->Unlock();

    if (CAMERA_SUCCESS == rc)
    {
        pJob->status = rc;
        pEvent->eventId = AIS_EVENT_PPROC_JOB_DONE;
        AisEngine::GetInstance()->QueueEvent(pEvent);
    }

    return rc;
};
