/*!
 * Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ais_pproc_usrdone.h"

#include "ais_engine.h"
#include "CameraPlatform.h"

AisPProcUsrDone* AisPProcUsrDone::m_pNodeInstance = nullptr;

/**
 * AisBuflistEnqueueUser
 *
 * @brief Enqueue buffer done to user
 *
 * @return CameraResult
 */
CameraResult AisBuflistEnqueueUser(AisUsrCtxt* pUsrCtxt, AisBufferList* pBufferList, qcarcam_frame_info_v2_t* pFrameDone)
{
    CameraResult result = CAMERA_SUCCESS;
    int rc = EOK;
    uint32 buffer_done_q_length = 0;

    pthread_mutex_lock(&pBufferList->m_bufferDoneQMutex);

    /*Check latency and reduce if needed*/
    CameraQueueGetLength(pBufferList->m_bufferDoneQ, &buffer_done_q_length);
    if (buffer_done_q_length > pUsrCtxt->m_usrSettings.n_latency_max)
    {
        uint32 i;

        for (i = 0; i < pUsrCtxt->m_usrSettings.n_latency_reduce_rate; i++)
        {
            qcarcam_frame_info_v2_t frame_info;
            result = CameraQueueDequeue(pBufferList->m_bufferDoneQ, (CameraQueueDataType)&frame_info);
            if (CAMERA_SUCCESS == result)
            {
                AIS_LOG(ENGINE, WARN, "Drop buffer %d from bufferDoneQ and requeue", frame_info.idx);
                pBufferList->SetBufferState(frame_info.idx, AIS_BUFFER_INITIALIZED);
                pBufferList->ReturnBuffer(pUsrCtxt, frame_info.idx);
            }
            else
            {
                AIS_LOG(ENGINE, ERROR, "ctxt %p failed (%d) to dequeue frame to requeue", pUsrCtxt, result);
            }
        }
    }

    /*Queue new frame and signal condition*/
    result = CameraQueueEnqueue(pBufferList->m_bufferDoneQ, (CameraQueueDataType)pFrameDone);
    pBufferList->SetBufferState(pFrameDone->idx, AIS_DELIVERY_QUEUE);

    if (EOK != (rc = pthread_cond_signal(&pBufferList->m_bufferDoneQCond)))
    {
        AIS_LOG(ENGINE, ERROR, "pthread_cond_signal failed: %s", strerror(rc));
        result = ERRNO_TO_RESULT(rc);
    }

    pthread_mutex_unlock(&pBufferList->m_bufferDoneQMutex);

    /*send callback if needed*/
    if (pUsrCtxt->m_eventCbFcn && (pUsrCtxt->m_eventMask & QCARCAM_EVENT_FRAME_READY))
    {
        qcarcam_event_payload_t payload = {};
        payload.frame_info = *pFrameDone;

        if (AisEngine::GetInstance()->GetLatencyMeasurementMode() == CAMERA_LM_MODE_ALL_STEPS)
        {
            uint64 ptimestamp = 0;

            //set flags to indicate latency measurement is enabled
            payload.frame_info.flags = 0x1;

            CameraGetTime(&ptimestamp);
            AIS_LOG(ENGINE, HIGH, "LATENCY| input %d  qcarcamHndl %lu buff list enqueue latency from frame done %llu us",
                    pUsrCtxt->m_inputId,
                    pUsrCtxt->m_qcarcamHndl,
                    (ptimestamp - pFrameDone->timestamp_system) / 1000);
        }

        pUsrCtxt->m_eventCbFcn(pUsrCtxt->m_qcarcamHndl, QCARCAM_EVENT_FRAME_READY, (qcarcam_event_payload_t *)&payload);
    }

    return result;
}

/**
 * ProcessEvent
 *
 * @brief Process Event by queuing frame buffer to client buffer done Q.
 *        If the buffer done Q exceeds latency max limit, the head of the Q is returned
 *        to be processed by the engine.
 *
 * @param pUsrCtxt
 * @param pEvent
 *
 * @return CameraResult
 */
CameraResult AisPProcUsrDone::ProcessEvent(AisUsrCtxt* pUsrCtxt, AisEventMsgType* pEvent)
{
    CameraResult result = CAMERA_SUCCESS;
    AisEventPProcJobType* pJob = &pEvent->payload.pprocJob;
    uint bufferlistId = pJob->pProcChain->inBuflistId[pJob->streamIdx];

    if (bufferlistId >= AIS_BUFLIST_MAX)
    {
        AIS_LOG(ENGINE, ERROR, "Context %p invalid bufferlist id %d", pUsrCtxt, bufferlistId);
        return CAMERA_EBADSTATE;
    }

    AisBufferList* pBufferList = pUsrCtxt->m_bufferList[bufferlistId];

    pUsrCtxt->Lock();

    //context must be streaming
    if (pUsrCtxt->m_state != AIS_USR_STATE_STREAMING)
    {
        AIS_LOG(ENGINE, ERROR, "Context %p in bad state %d", pUsrCtxt, pUsrCtxt->m_state);
        pUsrCtxt->Unlock();
        return CAMERA_EBADSTATE;
    }

    if (pUsrCtxt->m_isPendingStart)
    {
        pUsrCtxt->m_isPendingStart = FALSE;
    }

    AisBuffer* pBuffer = pBufferList->GetReadyBuffer(pJob->jobId);
    if (!pBuffer)
    {
        AIS_LOG(ENGINE, ERROR, "Context %p cannot find input buffer (%d) for jobId 0x%llx", pUsrCtxt, bufferlistId, pJob->jobId);
        pUsrCtxt->Unlock();
        return CAMERA_ENOREADYBUFFER;
    }

    if (pJob->frameInfo.timestamp < pUsrCtxt->m_startTime)
    {
        AIS_LOG(ENGINE, ERROR, "Drop buffer %d due to frame timestamp (%llu) <= starttime (%llu)",
                pBuffer->idx, pJob->frameInfo.timestamp, pUsrCtxt->m_startTime);

        pBufferList->SetBufferState(pBuffer->idx, AIS_BUFFER_INITIALIZED);
        pBufferList->ReturnBuffer(pUsrCtxt, pBuffer->idx);

        pUsrCtxt->Unlock();
        return CAMERA_EFAILED;
    }

    pJob->frameInfo.id = bufferlistId;
    pJob->frameInfo.idx = pBuffer->idx;


    //Only invalidate if buffer is cached
    if (CAMERA_BUFFER_FLAG_CACHED & pBuffer->flags)
    {
        result = CameraBufferCacheInvalidate(pBuffer);
        if (CAMERA_SUCCESS != result)
        {
            AIS_LOG(ENGINE, ERROR, "buffer%d cache invalidate failed %d",
                    pBuffer->idx, result);
        }
    }

    AisBuflistEnqueueUser(pUsrCtxt, pBufferList, &pJob->frameInfo);

    pUsrCtxt->Unlock();

    pEvent->eventId = AIS_EVENT_PPROC_JOB_DONE;
    AisEngine::GetInstance()->QueueEvent(pEvent);

    return result;
};
