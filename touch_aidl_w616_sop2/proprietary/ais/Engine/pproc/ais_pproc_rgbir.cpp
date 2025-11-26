/*!
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * @brief  RGB-IR extraction node
 *      Takes in an input RGB-IR interleaved frame and outputs.
 *      1) BAYER stream
 *      2) IR stream (1/4 size)
 *      3) IR upscaled stream
 */

#include "ais_pproc_rgbir.h"

#include "ais_engine.h"
#include "CameraPlatform.h"

#define PPROC_LOG(lvl, fmt...) AIS_LOG(PPROC_RGBIR, lvl, fmt)

AisPProcRgbIR* AisPProcRgbIR::m_pNodeInstance = nullptr;

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
CameraResult AisPProcRgbIR::ProcessEvent(AisUsrCtxt* pUsrCtxt, AisEventMsgType* pEvent)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisEventPProcJobType* pJob = &pEvent->payload.pprocJob;
    AisBufferList* pBufferListIn = pUsrCtxt->m_bufferList[pJob->pProcChain->inBuflistId[0]];
    AisBufferList* pBufferListOut = pUsrCtxt->m_bufferList[pJob->pProcChain->outBuflistId[0]];
    AisBuffer* pOutBuffer = NULL;

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

    //@TODO: add better logic here but this can be just a temporary solution for sample
    // input0 - RGB
    // input1 - IR 1/4 size
    // input2 - IR FULL size
    //
    // For this sample, skip every other frame.
    // Solution can parse buffer (pInBuffer->pVa) to find out if RGB or IR frame
    if (pUsrCtxt->m_inputId == 0)
    {
        //can parse buffer ((pInBuffer->pVa)) to find out if RGB or IR frame
        //for this sample just split it as odd/even frames
        if (pJob->frameInfo.seq_no[0] % 2)
        {
            // skip frame and end pproc job by returning ENEEDMORE
            rc = CAMERA_ENEEDMORE;
        }
    }
    else if ((pJob->frameInfo.seq_no[0] % 2) == 0)
    {
        // skip frame and end pproc job by returning ENEEDMORE
        rc = CAMERA_ENEEDMORE;
    }
    else
    {
        //grab an output buffer
        pOutBuffer = pBufferListOut->GetFreeBuffer(pUsrCtxt);
        if (pOutBuffer)
        {
            //for now just do simple memcpy...
            if (pUsrCtxt->m_inputId == 0 || pUsrCtxt->m_inputId == 2)
            {
                memcpy(pOutBuffer->pVa, pInBuffer->pVa, STD_MIN(pOutBuffer->size, pInBuffer->size));
            }
            else
            {
                //IR 1/4 size
                for (uint32 y = 0; y < pInBuffer->bufferInfo.planes[0].height; y+=2)
                {
                    uint16* pSrc = (uint16*)pInBuffer->pVa + (y * (pInBuffer->bufferInfo.planes[0].stride / 2));
                    uint16* pDst = (uint16*)pOutBuffer->pVa + (y * (pOutBuffer->bufferInfo.planes[0].stride / 2));

                    for (uint32 x = 0; x < pInBuffer->bufferInfo.planes[0].width; x+=2)
                    {
                        *pDst = *pSrc;
                        pDst++;
                        pSrc+=2;
                    }
                }
            }

            pBufferListOut->QueueReadyBuffer(pJob->jobId, pOutBuffer);
        }
        else
        {
            PPROC_LOG(ERROR, "No free buffers (error %d), dropped frame!", rc);
            rc = CAMERA_EFAILED;
        }
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
