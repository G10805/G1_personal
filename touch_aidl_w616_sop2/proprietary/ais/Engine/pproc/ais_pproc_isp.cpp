/*!
 * Copyright (c) 2016-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ais_pproc_isp.h"


#include <stdio.h>
#if defined(__ANDROID__)|| defined(__AGL__)
#include <cutils/native_handle.h>
#endif

#include "ais_log.h"
#include "ais_engine.h"

#include "CameraPlatform.h"

#include "camxcdktypes.h"
#include "chituningmodeparam.h"
#include "chipipeline.h"
#include "chimodule.h"
#include "chisession.h"
#include "chistatsproperty.h"

//////////////////////////////////////////////////////////////////////////////////
/// MACRO DEFINITIONS
//////////////////////////////////////////////////////////////////////////////////
#define EVENT_ISP_QUEUE_MAX_SIZE 32
#define MAX_CHISESSION_NUM       4

#define ISP_LOG(lvl, fmt...) AIS_LOG(PPROC_ISP, lvl, fmt)

//////////////////////////////////////////////////////////////////////////////////
/// TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
/// FORWARD DECLARE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
/// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////////
AisPProcIsp* AisPProcIsp::m_pIspSchedulerInstance = nullptr;

//////////////////////////////////////////////////////////////////////////////////
/// FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////
AisPProcIsp* AisPProcIsp::GetInstance()
{
    return m_pIspSchedulerInstance;
}

void AisPProcIsp::DestroyInstance()
{
    if(m_pIspSchedulerInstance != nullptr)
    {
        delete m_pIspSchedulerInstance;
        m_pIspSchedulerInstance = nullptr;
    }
}

AisPProcIsp::AisPProcIsp() :
    m_state(AIS_PPROC_ISP_STATE_INVALID),
    m_ispEventHandlerSignal(NULL), m_ispEventHandlerTid(NULL),
    m_bIspEventHandlerExit(FALSE), m_ispEventQ(NULL),
    m_ispMutex(NULL), m_pChiModule(NULL), m_pProcThreadSignal(NULL),
    m_pCaptureResQMutex(NULL), m_procThreadHandle(NULL), m_procThreadExit(FALSE),
    m_pCaptureResultsQ(NULL)
{

}

/**
 * Create
 *
 * @brief Create ISP scheduler
 *
 * @return AisNodeIsp
 */
AisPProcIsp* AisPProcIsp::Create()
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 i = 0;

    AIS_LOG(PPROC_ISP, HIGH, "Create isp scheduler");

    AisPProcIsp* pPProcIsp = new AisPProcIsp;

    if (pPProcIsp)
    {
        CameraQueueCreateParamType sCreateParams = {};
        sCreateParams.nCapacity = EVENT_ISP_QUEUE_MAX_SIZE;
        sCreateParams.nDataSizeInBytes = sizeof(AisIspEvent);
        sCreateParams.eLockType = CAMERAQUEUE_LOCK_THREAD;

        rc = CameraQueueCreate(&pPProcIsp->m_ispEventQ, &sCreateParams);
        AIS_LOG_ON_ERR(PPROC_ISP, rc, "Failed to create event queue for isp %d: %d",i, rc);

        if (CAMERA_SUCCESS == rc)
        {
            rc = CameraCreateSignal(&pPProcIsp->m_ispEventHandlerSignal);
            AIS_LOG_ON_ERR(PPROC_ISP, rc, "Failed to create isp handler signal: %d", rc);
        }

        if (CAMERA_SUCCESS == rc)
        {
            rc = CameraCreateMutex(&pPProcIsp->m_ispMutex);
            AIS_LOG_ON_ERR(PPROC_ISP, rc, "Failed to create isp mutex: %d", rc);
        }

        if (CAMERA_SUCCESS == rc)
        {
            char name[64];
            /*create the thread for the isp handler*/
            snprintf(name, sizeof(name), "engine_isp_hndlr_%d", i);

            pPProcIsp->m_bIspEventHandlerExit = false;

            rc = CameraCreateThread(CAMERA_THREAD_PRIO_HIGH_REALTIME,
                    0,
                    IspEventHandler,
                    pPProcIsp,
                    0x10000,
                    name,
                    &pPProcIsp->m_ispEventHandlerTid);
            AIS_LOG_ON_ERR(PPROC_ISP, rc, "CameraCreateThread failed: %d", rc);
        }

        if (CAMERA_SUCCESS == rc)
        {
            pPProcIsp->m_state = AIS_PPROC_ISP_STATE_INIT;
            m_pIspSchedulerInstance = pPProcIsp;
        }
        else
        {
            pPProcIsp->Deinitialize();
            delete pPProcIsp;
            return NULL;
        }
    }

    return m_pIspSchedulerInstance;
}


/**
 * InitializeChi
 *
 * @brief Initialize CHI
 *
 * @return None
 */
CameraResult AisPProcIsp::InitializeChi()
{
    CameraResult rc = CAMERA_SUCCESS;

    m_pChiModule = ChiModule::CreateInstance();
    if (NULL == m_pChiModule)
    {
        ISP_LOG(ERROR, "Failed to create ChiModule.");
        return CAMERA_EFAILED;
    }
    ISP_LOG(DBG,
        "Successfully created ChiModule=%p.", m_pChiModule);

    rc = CameraCreateSignal(&m_pProcThreadSignal);
    if (NULL == m_pProcThreadSignal)
    {
        ISP_LOG(ERROR,
            "Failed to create processing thread signal.");
        return CAMERA_EFAILED;
    }
    ISP_LOG(DBG,
        "Successfully created processing thread signal, m_pProcThreadSignal=%p.",
        m_pProcThreadSignal);

    rc = CameraCreateMutex(&m_pCaptureResQMutex);
    if (NULL == m_pCaptureResQMutex)
    {
        ISP_LOG(ERROR,
            "Failed to create capture result queue mutex.");
        return CAMERA_EFAILED;
    }
    ISP_LOG(DBG,
        "Successfully created capture result mutex, m_pCaptureResQMutex=%p.",
        m_pCaptureResQMutex);

    m_pCaptureResultsQ = new std::queue<AisIspCaptureResult>();
    if (NULL == m_pCaptureResultsQ)
    {
        ISP_LOG(ERROR,
            "Failed to create capture results queue.");
        return CAMERA_EFAILED;
    }
    ISP_LOG(DBG,
        "Successfully created capture results queue, m_pCaptureResultsQ=%p.",
        m_pCaptureResultsQ);

    char name[64];
    /*create the thread for the isp handler*/
    snprintf(name, sizeof(name), "ais_isp_procthread");
    m_procThreadExit = false;
    rc = CameraCreateThread(CAMERA_THREAD_PRIO_HIGH_REALTIME,
            0,
            ProcThreadEntry,
            this,
            0,
            name,
            &m_procThreadHandle);
    if (rc != CAMERA_SUCCESS)
    {
        ISP_LOG(ERROR,
            "Failed create capture processing thread.");
        return rc;
    }
    ISP_LOG(DBG,
        "Successfully created capture processing thread, m_procThreadHandle=%p.",
        m_procThreadHandle);

    return rc;
}

/**
 * Deinitialize
 *
 * @brief Initialize ISP scheduler
 *
 * @return None
 */
void AisPProcIsp::Deinitialize(void)
{
    m_bIspEventHandlerExit = true;

    if (m_ispEventHandlerTid)
    {
        CameraSetSignal(m_ispEventHandlerSignal);

        CameraJoinThread(m_ispEventHandlerTid, NULL);
        CameraReleaseThread(m_ispEventHandlerTid);
    }

    if (m_ispMutex)
    {
        CameraDestroyMutex(m_ispMutex);
        m_ispMutex = NULL;
    }

    if (m_ispEventHandlerSignal)
    {
        CameraDestroySignal(m_ispEventHandlerSignal);
    }

    if (m_ispEventQ)
    {
        CameraQueueDestroy(m_ispEventQ);
    }

    /*Deinit chi cdk*/
    if (m_pProcThreadSignal)
    {
        m_procThreadExit = true;
        CameraSetSignal(m_pProcThreadSignal);
    }
    if (m_procThreadHandle)
    {
        CameraJoinThread(m_procThreadHandle, NULL);
        CameraReleaseThread(m_procThreadHandle);
    }

    if (m_pCaptureResultsQ != NULL)
    {
        CameraLockMutex(m_pCaptureResQMutex);
        while (!m_pCaptureResultsQ->empty())
        {
            AisIspCaptureResult& rCaptureRes = m_pCaptureResultsQ->front();
            if (rCaptureRes.pChiCaptureResult)
            {
                delete rCaptureRes.pChiCaptureResult;
            }
            m_pCaptureResultsQ->pop();
        }
        delete m_pCaptureResultsQ;
        m_pCaptureResultsQ = NULL;
        CameraUnlockMutex(m_pCaptureResQMutex);
    }

    if (m_pCaptureResQMutex)
    {
        CameraDestroyMutex(m_pCaptureResQMutex);
        m_pCaptureResQMutex = NULL;
    }

    if (m_pProcThreadSignal)
    {
        CameraDestroySignal(m_pProcThreadSignal);
        m_pProcThreadSignal = NULL;
    }

    if (m_pChiModule)
    {
        delete m_pChiModule;
        m_pChiModule = NULL;
    }
}

/**
 * Destroy
 *
 * @brief Destroy ISP scheduler
 *
 * @return None
 */
void AisPProcIsp::Destroy(void)
{
    AIS_LOG(PPROC_ISP, HIGH, "Destroy isp scheduler");

    AisPProcIsp* pIspCtxt = m_pIspSchedulerInstance;

    if (pIspCtxt)
    {
        pIspCtxt->Deinitialize();

        DestroyInstance();
    }
}

/**
 * ProcessIspEvent
 *
 * @brief Dequeues event from event Q and processes it
 *
 * @param pMsg
 *
 * @return int
 */
int AisPProcIsp::ProcessIspEvent(AisIspEvent* pIspEvent)
{
    CameraResult rc;

    rc = CameraQueueDequeue(m_ispEventQ, pIspEvent);
    if (CAMERA_SUCCESS != rc)
    {
        if (CAMERA_ENOMORE != rc)
        {
            AIS_LOG(PPROC_ISP, ERROR, "Failed to dequeue event (%d)", rc);
        }
        return 0;
    }

    switch (pIspEvent->id)
    {
    case ISP_EVENT_PROCESS_FRAME:
    {
        AisEventPProcJobType* pIspJob = pIspEvent->payload.pIspJob;
        AisPProcIspChiSession* pSession = GetSession(pIspJob->pUsrCtxt, pIspJob->pProcChain->instanceId);
        uint32 jobId = pIspJob->jobId;

        AIS_LOG(PPROC_ISP, MED, "ISP process job id 0x%x %d",
                pIspJob->jobId,
                pIspJob->pProcChain->instanceId);

        if (pSession)
        {
            //Process Buffer here
            rc = pSession->ProcessFrame(pIspJob);
            if (CAMERA_SUCCESS != rc)
            {
                AIS_LOG(PPROC_ISP, ERROR, "ISP job id fail to process Isp Frame (rc = %d)", rc);
            }
        }
        else
        {
            rc = CAMERA_EFAILED;
            AIS_LOG(PPROC_ISP, ERROR, "can't find IspChiSession by UsrCtxt %p", pIspJob->pUsrCtxt);
        }

        if (CAMERA_SUCCESS != rc)
        {
            AisBufferList* pInBufferList = pIspJob->pUsrCtxt->m_bufferList[pIspJob->pProcChain->inBuflistId[0]];
            AisBufferList* pOutBufferList = pIspJob->pUsrCtxt->m_bufferList[pIspJob->pProcChain->outBuflistId[0]];
            AisBufferList* pJpegOutBufferList = pIspJob->pUsrCtxt->m_bufferList[pIspJob->pProcChain->outBuflistId[1]];

            pInBufferList->SetBufferState(pIspJob->bufInIdx[0], AIS_BUFFER_INITIALIZED);
            pInBufferList->ReturnBuffer(pIspJob->pUsrCtxt, pIspJob->bufInIdx[0]);
            pOutBufferList->SetBufferState(pIspJob->bufOutIdx[0], AIS_BUFFER_INITIALIZED);
            pOutBufferList->ReturnBuffer(pIspJob->pUsrCtxt, pIspJob->bufOutIdx[0]);

            if (pJpegOutBufferList != NULL)
            {
                pJpegOutBufferList->SetBufferState(pIspJob->bufOutIdx[1], AIS_BUFFER_INITIALIZED);
                pJpegOutBufferList->ReturnBuffer(pIspJob->pUsrCtxt, pIspJob->bufOutIdx[1]);
            }

            AisEventMsgType msg = {};
            pIspJob->status = rc;
            msg.eventId = AIS_EVENT_PPROC_JOB_DONE;
            memcpy(&msg.payload.pprocJob, pIspJob, sizeof(AisEventPProcJobType));
            AisEngine::GetInstance()->QueueEvent(&msg);
            CameraFree(CAMERA_ALLOCATE_ID_ENGINE_EVENT, pIspJob);
        }
        else
        {
            AIS_LOG(PPROC_ISP, DBG, "ISP job id submitted 0x%x(rc = %d)", jobId, rc);
        }

        break;
    }
    default:
        AIS_LOG(PPROC_ISP, ERROR, "Unsupported event (%p %d)", pIspEvent, pIspEvent->id);
        break;
    }

    return 1;
}

/**
 * IspEventHandler
 *
 * @brief ISP event handler thread to process events
 *
 * @param AisNodeIsp*
 *
 * @return int
 */
int AisPProcIsp::IspEventHandler(void *pArg)
{
    AisPProcIsp* pIspCtxt = (AisPProcIsp*)pArg;

    if (pIspCtxt)
    {
        CameraResult rc = CAMERA_SUCCESS;

        AIS_LOG(PPROC_ISP, HIGH, "Starting IspEventHandler");

        CameraLockMutex(pIspCtxt->m_ispMutex);
#if !defined(AIS_WITH_HAL_CAMERA) &&  defined(AIS_WITH_CAMX)
        rc = pIspCtxt->InitializeChi();
#else
        rc = CAMERA_EFAILED;
#endif
        if (CAMERA_SUCCESS != rc)
        {
            pIspCtxt->m_state = AIS_PPROC_ISP_STATE_ERROR;
            CameraUnlockMutex(pIspCtxt->m_ispMutex);
            return -1;
        }

        pIspCtxt->m_state = AIS_PPROC_ISP_STATE_READY;
        CameraUnlockMutex(pIspCtxt->m_ispMutex);

        AisIspEvent* pIspEvent = (AisIspEvent*)CameraAllocate(CAMERA_ALLOCATE_ID_ISP_EVENT, sizeof(AisIspEvent));

        if (pIspEvent)
        {
            while (!pIspCtxt->m_bIspEventHandlerExit)
            {
                AIS_LOG(PPROC_ISP, LOW, "Awake; ready to work.");

                (void)pIspCtxt->ProcessIspEvent(pIspEvent);

                AIS_LOG(PPROC_ISP, LOW, "Nothing to do, going to sleep");

                CameraWaitOnSignal(pIspCtxt->m_ispEventHandlerSignal,
                    CAM_SIGNAL_WAIT_NO_TIMEOUT);
            }

            CameraFree(CAMERA_ALLOCATE_ID_ISP_EVENT, pIspEvent);
        }
        else
        {
            rc = CAMERA_ENOMEMORY;
        }

        AIS_LOG(PPROC_ISP, HIGH, "Terminating ...");
    }

    // Don't use MM_Thread_Exit.
    // - Depending on scheduler's thread execution,
    //   MM_Thread_Exit may just do "return 1",
    //   which is not what we want.

    return 0;
}

CameraResult AisPProcIsp::Flush(AisUsrCtxt* pUsrCtxt,const AisProcChainType* pProcChain)
{
    CameraResult rc = CAMERA_SUCCESS;

    AisPProcIspChiSession* pSession = GetSession(pUsrCtxt, pProcChain->instanceId);
    if (NULL != pSession)
    {
        rc = pSession->Flush();
    } else
    {
        rc = CAMERA_EFAILED;
        AIS_LOG(PPROC_ISP, ERROR, "can't get session for ctxt: %p", pUsrCtxt);
    }

    return rc;
}




/**
 * ProcessEvent
 *
 * @brief Process ISP Frame
 *
 * @param pUsrCtxt
 * @param pEvent
 *
 * @return CameraResult
 */
CameraResult AisPProcIsp::ProcessEvent(AisUsrCtxt* pUsrCtxt, AisEventMsgType* pEvent)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisIspEvent sIspEvent = {};
    AisEventPProcJobType* pIspJob = (AisEventPProcJobType*)CameraAllocate(CAMERA_ALLOCATE_ID_ENGINE_EVENT, sizeof(AisEventPProcJobType));

    if (pIspJob == NULL)
    {
        AIS_LOG(PPROC_ISP, ERROR, "allocation failed");
        rc = CAMERA_EFAILED;
        return rc;
    }
    else
    {
        memcpy(pIspJob, &pEvent->payload.pprocJob, sizeof(AisEventPProcJobType));
    }

    sIspEvent.id = ISP_EVENT_PROCESS_FRAME;
    sIspEvent.payload.pIspJob = pIspJob;

    AisBufferList* pInBufferList = pUsrCtxt->m_bufferList[pIspJob->pProcChain->inBuflistId[0]];
    AisBufferList* pOutBufferList = pUsrCtxt->m_bufferList[pIspJob->pProcChain->outBuflistId[0]];
    AisBufferList* pJpegBufferList = (pIspJob->pProcChain->numOut > 1) ?
        pUsrCtxt->m_bufferList[pIspJob->pProcChain->outBuflistId[1]] : NULL;

    AisBuffer* pInBuffer = pInBufferList->GetReadyBuffer(pIspJob->jobId);
    if (!pInBuffer)
    {
        AIS_LOG(ENGINE, ERROR, "Context %p cannot find input buffer for jobId 0x%llx",
                pIspJob->pUsrCtxt, pIspJob->jobId);
        CameraFree(CAMERA_ALLOCATE_ID_ENGINE_EVENT, pIspJob);
        return CAMERA_ENOREADYBUFFER;
    }
    pIspJob->bufInIdx[0] = pInBuffer->idx;

    /*get available output buffer*/
    AisBuffer* pOutBuffer = pOutBufferList->GetFreeBuffer(pIspJob->pUsrCtxt);
    if (pOutBuffer)
    {
        pIspJob->bufOutIdx[0] = pOutBuffer->idx;

        if (pJpegBufferList != NULL)
        {
            AisBuffer* pJpegBuffer = pJpegBufferList->GetFreeBuffer(pIspJob->pUsrCtxt);

            if (pJpegBuffer)
            {
                pIspJob->bufOutIdx[1] = pJpegBuffer->idx;
            }
            else
            {
                rc = CAMERA_EFAILED;

                AIS_LOG(PPROC_ISP, ERROR, "no free jpeg buffers!");
                pOutBufferList->SetBufferState(pOutBuffer->idx, AIS_BUFFER_INITIALIZED);
                pOutBufferList->ReturnBuffer(pIspJob->pUsrCtxt, pOutBuffer->idx);
            }
        }
    }
    else
    {
        rc = CAMERA_EFAILED;
    }

    if (CAMERA_SUCCESS != rc)
    {
        AIS_LOG(PPROC_ISP, ERROR, "no free buffers (error %d), dropped frame!", rc);
        pInBufferList->SetBufferState(pInBuffer->idx, AIS_BUFFER_INITIALIZED);
        pInBufferList->ReturnBuffer(pIspJob->pUsrCtxt, pInBuffer->idx);
        CameraFree(CAMERA_ALLOCATE_ID_ENGINE_EVENT, pIspJob);
        rc = CAMERA_EFAILED;
    }
    else
    {
        pInBufferList->SetBufferState(pInBuffer->idx, AIS_ISP_SCHEDULER_INPUT_AVAIL);

        AIS_LOG(PPROC_ISP, MED, "ISP enqueue job id 0x%x (%d, %d) %d",
            pIspJob->jobId, pInBuffer->idx, pOutBuffer->idx,
            pIspJob->pProcChain->instanceId);

        rc = EnqueueEvent(&sIspEvent);
        if (CAMERA_SUCCESS != rc)
        {
            pInBufferList->SetBufferState(pIspJob->bufInIdx[0], AIS_BUFFER_INITIALIZED);
            pInBufferList->ReturnBuffer(pIspJob->pUsrCtxt, pIspJob->bufInIdx[0]);
            pOutBufferList->SetBufferState(pIspJob->bufOutIdx[0], AIS_BUFFER_INITIALIZED);
            pOutBufferList->ReturnBuffer(pIspJob->pUsrCtxt, pIspJob->bufOutIdx[0]);
            if (pJpegBufferList != NULL)
            {
                pJpegBufferList->SetBufferState(pIspJob->bufOutIdx[1], AIS_BUFFER_INITIALIZED);
                pJpegBufferList->ReturnBuffer(pIspJob->pUsrCtxt, pIspJob->bufOutIdx[1]);
            }
            CameraFree(CAMERA_ALLOCATE_ID_ENGINE_EVENT, pIspJob);
        }
    }

    return rc;
}

void AisPProcIsp::GetMetadataOp(CHIMETADATAOPS* pMetaDataOps)
{
    if (pMetaDataOps)
    {
        m_pChiModule->GetChiOps()->pMetadataOps(pMetaDataOps);
    }
}

void AisPProcIsp::GetTagOp(CHITAGSOPS* pTagOps)
{
    if (pTagOps)
    {
        m_pChiModule->GetChiOps()->pTagOps(pTagOps);
    }
}

CameraResult AisPProcIsp::SubmitPipelineRequest(CHIPIPELINEREQUEST* pRequest)
{
    CameraResult rc = CAMERA_SUCCESS;
    if (pRequest)
    {
        if (m_pChiModule->GetChiOps()->pSubmitPipelineRequest(
            m_pChiModule->GetContext(),
            pRequest) != CDKResultSuccess)
        {
            rc = CAMERA_EFAILED;
        }
    }

    return rc;
}

CameraResult AisPProcIsp::EnqueueEvent(AisIspEvent* pEvent)
{
    CameraResult rc;

    AIS_LOG(PPROC_ISP, LOW, "Enqueue (%p %d)", pEvent, pEvent->id);

    rc = CameraQueueEnqueue(m_ispEventQ, pEvent);
    if (CAMERA_SUCCESS == rc)
    {
        rc = CameraSetSignal(m_ispEventHandlerSignal);
    }

    if (CAMERA_SUCCESS != rc)
    {
        AIS_LOG(PPROC_ISP, ERROR, "Event %p %d Failed - %d", pEvent, pEvent->id, rc);
    }

    return rc;
}

/**
 * CreateSession
 *
 * @brief Create ISP session for UsrCtxt
 *
 * @param pUsrCtxt
 * @param pProcChain
 *
 * @return CameraResult
 */
CameraResult AisPProcIsp::CreateSession(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain)
{
    CameraResult rc = CAMERA_SUCCESS;

    CameraLockMutex(m_ispMutex);
    if (m_state != AIS_PPROC_ISP_STATE_READY)
    {
        rc = CAMERA_EUNSUPPORTED;
    }

    if (m_pChiSessionMap.size() >= MAX_CHISESSION_NUM)
    {
        AIS_LOG(PPROC_ISP, ERROR, "beyond the max supported chisession num %u ", MAX_CHISESSION_NUM);
        rc = CAMERA_EFAILED;
    }

    CameraUnlockMutex(m_ispMutex);

    if (rc == CAMERA_SUCCESS)
    {
        AisPProcIspChiSession* pSession = new AisPProcIspChiSession();
        rc = pSession->Initialize(pUsrCtxt, pProcChain);

        if (rc == CAMERA_SUCCESS)
        {
            rc = AddSession(pUsrCtxt, pProcChain->instanceId, pSession);
        }

        if (rc != CAMERA_SUCCESS)
        {
            delete pSession;
            rc = CAMERA_EFAILED;
        }
    }

    return rc;
}

/**
 * DestroySession
 *
 * @brief Destroy ISP session for UsrCtxt
 *
 * @param pUsrCtxt
 * @param pProcChain
 *
 * @return CameraResult
 */
CameraResult AisPProcIsp::DestroySession(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisPProcIspChiSession* pSession = GetSession(pUsrCtxt, pProcChain->instanceId);
    if (pSession)
    {
        RemoveSession(pUsrCtxt, pProcChain->instanceId);
        pSession->Deinitialize(pProcChain);

        delete pSession;
    }

    return rc;
}

int AisPProcIsp::ProcThreadEntry(void *pArg)
{
    if (NULL == pArg)
    {
        ISP_LOG(ERROR,
            "No AisNodeIsp instance provided.");
        return 0;
    }

    AisPProcIsp* pNode = reinterpret_cast<AisPProcIsp*>(pArg);

    while (!pNode->m_procThreadExit)
    {
        CameraWaitOnSignal(pNode->m_pProcThreadSignal, CAM_SIGNAL_WAIT_NO_TIMEOUT);

        ISP_LOG(DBG,
            "Processing capture results.");
        pNode->ProcCaptureResults();
    }

    return 0;
}

CameraResult AisPProcIsp::ProcCaptureResults()
{
    CameraResult rc = CAMERA_SUCCESS;

    CameraLockMutex(m_pCaptureResQMutex);
    if (!m_pCaptureResultsQ->empty())
    {
        AisIspCaptureResult rResult = m_pCaptureResultsQ->front();
        m_pCaptureResultsQ->pop();
        CameraUnlockMutex(m_pCaptureResQMutex);

        AisPProcIspChiSession* pSession = rResult.pSession;
        CHICAPTURERESULT* pCaptureResult = rResult.pChiCaptureResult;

        if (pSession && pCaptureResult)
        {
            rc = pSession->ProcCaptureResult(pCaptureResult);
        }
        else
        {
            AIS_LOG(PPROC_ISP, ERROR, "capture result dequeued is invalid pSession = %p, pCaptureResult = %p",
                pSession, pCaptureResult);
            rc = CAMERA_EBADPARM;
            return rc;
        }

    }
    else
    {
        CameraUnlockMutex(m_pCaptureResQMutex);
        AIS_LOG(PPROC_ISP, ERROR, "Capture results queue is empty!");
    }

    return rc;
}

CameraResult AisPProcIsp::EnqCaptureResult(AisPProcIspChiSession* pSession, CHICAPTURERESULT* pCaptureResult)
{
    CameraResult rc = CAMERA_SUCCESS;

    CameraLockMutex(m_pCaptureResQMutex);
    AisIspCaptureResult result = {pSession, pCaptureResult};
    m_pCaptureResultsQ->push(result);
    CameraUnlockMutex(m_pCaptureResQMutex);

    rc = CameraSetSignal(m_pProcThreadSignal);
    return rc;
}

AisPProcIspChiSession* AisPProcIsp::GetSession(AisUsrCtxt* pUsrCtxt, uint32 instanceId)
{
    AisPProcIspChiSession* pSession = NULL;
    std::list<AisPProcIspChiSessionListType>::iterator  it;

    CameraLockMutex(m_ispMutex);
    for(it = m_pChiSessionMap.begin(); it != m_pChiSessionMap.end(); ++it)
    {
        if (it->pUsrCtxt == pUsrCtxt && it->instanceId == instanceId)
        {
            pSession = it->pSession;
            break;
        }
    }
    CameraUnlockMutex(m_ispMutex);

    if (NULL == pSession)
    {
        AIS_LOG(PPROC_ISP, ERROR, "can't find the session %p %d", pUsrCtxt, instanceId);
    }

    return pSession;
}

void AisPProcIsp::RemoveSession(AisUsrCtxt* pUsrCtxt, uint32 instanceId)
{
    std::list<AisPProcIspChiSessionListType>::iterator  it;

    CameraLockMutex(m_ispMutex);
    for(it = m_pChiSessionMap.begin(); it != m_pChiSessionMap.end(); ++it)
    {
        if (it->pUsrCtxt == pUsrCtxt && it->instanceId == instanceId)
        {
            m_pChiSessionMap.erase(it);
            break;
        }
    }
    CameraUnlockMutex(m_ispMutex);
}

CameraResult AisPProcIsp::AddSession(AisUsrCtxt* pUsrCtxt, uint32 instanceId, AisPProcIspChiSession* pSession)
{
    CameraResult rc = CAMERA_SUCCESS;

    AisPProcIspChiSessionListType newEntry = {
            .pSession = pSession,
            .pUsrCtxt = pUsrCtxt,
            .instanceId = instanceId
    };

    CameraLockMutex(m_ispMutex);
    m_pChiSessionMap.push_front(newEntry);
    CameraUnlockMutex(m_ispMutex);

    return rc;
}

/**
 * SetParams
 *
 * @brief Apply ISP Params for UsrCtxt
 *
 * @param pUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisPProcIsp::SetParams(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;
    //@todo for now hardcoded to instance id 0
    AisPProcIspChiSession* pSession = GetSession(pUsrCtxt, 0);
    if (pSession)
    {
        rc = pSession->UpdateFrameParams(&pUsrCtxt->m_usrSettings.isp_ctrls);
    }
    else
    {
        AIS_LOG(PPROC_ISP, ERROR, "Unabled to find ISP session for User context %p", pUsrCtxt);
        rc =  CAMERA_EFAILED;
    }

    return rc;
}

/**
 * GetParams
 *
 * @brief Get ISP Params for UsrCtxt
 *
 * @param pUsrCtxt
 * @param pParamOut
 *
 * @return CameraResult
 */
CameraResult AisPProcIsp::GetParams(AisUsrCtxt* pUsrCtxt, void *pParamOut)
{
    CameraResult rc = CAMERA_SUCCESS;
    qcarcam_param_isp_ctrls_t *pBayerCtrls = (qcarcam_param_isp_ctrls_t *)pParamOut;
    //@todo for now hardcoded to instance id 0
    AisPProcIspChiSession* pSession = GetSession(pUsrCtxt, 0);
    if (pSession)
    {
        rc = pSession->GetFrameParams(pBayerCtrls);
    }
    else
    {
        AIS_LOG(PPROC_ISP, ERROR, "Unabled to find ISP session for User context %p", pUsrCtxt);
        rc =  CAMERA_EFAILED;
    }

    return rc;
}
