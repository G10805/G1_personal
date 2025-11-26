/*!
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *
 * This proc chain module dynamically loads an external library using the
 * post processing API. This module handles the AIS buffer management and assumes
 * 1 input and 1 output buffer for a frame to process for now.
 *
 */

#include "ais_pproc_ext.h"

#include "ais_engine.h"
#include "CameraPlatform.h"
#include "post_process.h"

#include <dlfcn.h>

#define PPROC_LOG(lvl, fmt...) AIS_LOG(POST_PROCESS, lvl, fmt)

class AisPProcNodePrivate
{
public:
    AisPProcNodePrivate()
    {
        m_hPPLib = NULL;
        m_pGetPPInterface = NULL;
        m_pInterface = NULL;
        m_pp_ctxt = NULL;
    };

    ~AisPProcNodePrivate()
    {
        if (m_pp_ctxt)
        {
            m_pInterface->qcarcam_post_process_close(m_pp_ctxt);
            m_pp_ctxt = NULL;
        }

        if (m_hPPLib)
        {
            dlclose(m_hPPLib);
            m_hPPLib = NULL;
        }
    };

    CameraResult LoadLibrary(const char* pLibName);
    CameraResult UnloadLibarary();
    CameraResult Init(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain);
    CameraResult Deinit();
    CameraResult ProcessFrame(pp_job_t* pJob);

private:
    void*  m_hPPLib;
    GetPostProcessingInterfaceType m_pGetPPInterface;
    IPostProcessing const* m_pInterface;
    pp_ctxt_t m_pp_ctxt;
};


CameraResult AisPProcNodePrivate::LoadLibrary(const char* pLibName)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (0 == (m_hPPLib = dlopen(pLibName, RTLD_NOW|RTLD_GLOBAL)))
    {
        PPROC_LOG(ERROR, "'%s' Loading failed '%s'", pLibName, dlerror());
        rc = CAMERA_EFAILED;
    }
    else if (0 == (m_pGetPPInterface = (GetPostProcessingInterfaceType)dlsym(m_hPPLib, GET_POST_PROCESSING_INTERFACE)))
    {
        PPROC_LOG(ERROR, "'%s' Interface not found '%s'", pLibName, dlerror());
        rc = CAMERA_EFAILED;
    }
    else
    {
        m_pInterface = m_pGetPPInterface();
        if (!m_pInterface)
        {
            PPROC_LOG(ERROR, "'%s' invalid interface", pLibName);
            rc = CAMERA_EFAILED;
        }
        else if (POST_PROCESS_VERSION != m_pInterface->version ||
                 !m_pInterface->qcarcam_post_process_open ||
                 !m_pInterface->qcarcam_post_process_close ||
                 !m_pInterface->qcarcam_post_process_init ||
                 !m_pInterface->qcarcam_post_process_deinit ||
                 !m_pInterface->qcarcam_post_process_frame)
        {
            PPROC_LOG(ERROR, "'%s' invalid interfaces", pLibName);
            rc = CAMERA_EFAILED;
        }
        else
        {
            m_pp_ctxt = m_pInterface->qcarcam_post_process_open();
            if (!m_pp_ctxt)
            {
                PPROC_LOG(ERROR, "'%s' failed to open", pLibName);
                rc = CAMERA_EFAILED;
            }
        }
    }


    if (CAMERA_SUCCESS != rc)
    {
        if (m_hPPLib)
        {
            dlclose(m_hPPLib);
            m_hPPLib = NULL;
        }
    }

    return rc;
}

static void AisBufferlistToPProcBufferlist(AisBufferList* pAisBufferlist, pp_bufferlist_t* pPProcBufferlist)
{
    pPProcBufferlist->color_format = pAisBufferlist->GetColorFmt();
    pPProcBufferlist->num_planes = pAisBufferlist->m_pBuffers[0].bufferInfo.n_planes;
    pPProcBufferlist->width = pAisBufferlist->GetWidth();
    pPProcBufferlist->height = pAisBufferlist->GetHeight();
    pPProcBufferlist->n_buffers = pAisBufferlist->m_nBuffers;
    pPProcBufferlist->stride[0] = pAisBufferlist->m_pBuffers[0].bufferInfo.planes[0].stride;
    pPProcBufferlist->stride[1] = pAisBufferlist->m_pBuffers[0].bufferInfo.planes[1].stride;
    pPProcBufferlist->stride[2] = pAisBufferlist->m_pBuffers[0].bufferInfo.planes[2].stride;
    pPProcBufferlist->offset[0] = pAisBufferlist->m_pBuffers[0].bufferInfo.planes[0].offset;
    pPProcBufferlist->offset[1] = pAisBufferlist->m_pBuffers[0].bufferInfo.planes[1].offset;
    pPProcBufferlist->offset[2] = pAisBufferlist->m_pBuffers[0].bufferInfo.planes[2].offset;
    pPProcBufferlist->size[0] = pAisBufferlist->m_pBuffers[0].bufferInfo.planes[0].size;
    pPProcBufferlist->size[1] = pAisBufferlist->m_pBuffers[0].bufferInfo.planes[1].size;
    pPProcBufferlist->size[2] = pAisBufferlist->m_pBuffers[0].bufferInfo.planes[2].size;

    for (uint32 i = 0; i < pAisBufferlist->m_nBuffers; i++)
    {
        pPProcBufferlist->buffers[i].mem_handle = (unsigned long long)pAisBufferlist->m_pBuffers[i].pMemHndl;
        pPProcBufferlist->buffers[i].ptr = pAisBufferlist->m_pBuffers[i].pVa;
    }
}

CameraResult AisPProcNodePrivate::Init(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain)
{
    CameraResult rc = CAMERA_SUCCESS;
    pp_init_t pp_init = {};
    int ret = 0;

    pp_init.num_src = pProcChain->numIn;
    for (uint32 i = 0; i < pProcChain->numIn; i++)
    {
        AisBufferList* pInBufferlist = pUsrCtxt->m_bufferList[pProcChain->inBuflistId[i]];
        if (!pInBufferlist)
        {
            PPROC_LOG(ERROR, "Invalid in bufferlist %d", pProcChain->inBuflistId[i]);
            rc = CAMERA_EFAILED;
            break;
        }
        AisBufferlistToPProcBufferlist(pInBufferlist, &pp_init.src_bufferlists[i]);
    }

    pp_init.num_tgt = pProcChain->numOut;
    for (uint32 i = 0; i < pProcChain->numOut; i++)
    {
        AisBufferList* pOutBufferlist = pUsrCtxt->m_bufferList[pProcChain->outBuflistId[i]];
        if (!pOutBufferlist)
        {
            PPROC_LOG(ERROR, "Invalid out bufferlist %d", pProcChain->inBuflistId[i]);
            rc = CAMERA_EFAILED;
            break;
        }
        AisBufferlistToPProcBufferlist(pOutBufferlist, &pp_init.tgt_bufferlists[i]);
    }

    if (CAMERA_SUCCESS == rc)
    {
        ret = m_pInterface->qcarcam_post_process_init(m_pp_ctxt, &pp_init);

        if (0 != ret)
        {
            PPROC_LOG(ERROR, "Failed to init pproc %d", ret);
            rc = CAMERA_EFAILED;
        }
    }

    return rc;
}

CameraResult AisPProcNodePrivate::Deinit()
{
    CameraResult rc = CAMERA_SUCCESS;

    if (m_pp_ctxt)
    {
        if (0 != m_pInterface->qcarcam_post_process_deinit(m_pp_ctxt))
        {
            rc = CAMERA_EFAILED;
        }
    }
    return rc;
}

CameraResult AisPProcNodePrivate::ProcessFrame(pp_job_t* pJob)
{
    CameraResult rc = CAMERA_SUCCESS;
    int ret = 0;

    ret = m_pInterface->qcarcam_post_process_frame(m_pp_ctxt, pJob);
    if (0 != ret)
    {
        PPROC_LOG(ERROR, "Failed to process frame %llu %d", pJob->job_id, ret);
        rc = CAMERA_EFAILED;
    }
    return rc;
}


AisPProcNode::AisPProcNode()
{
    m_pCtxt = NULL;
}

AisPProcNode::~AisPProcNode()
{
    AisPProcNodePrivate* pCtxt = (AisPProcNodePrivate*)m_pCtxt;
    if (pCtxt)
    {
        pCtxt->Deinit();
        delete pCtxt;
    }
}

/**
 * Create
 *
 * @brief Create Node
 *
 * @return AisPProcNode*
 */
AisPProcNode* AisPProcNode::Create()
{
    return new AisPProcNode;
}

/**
 * Destroy
 *
 * @brief Destroy Node
 *
 * @return None
 */
void AisPProcNode::Destroy(void)
{
    delete this;
}


/**
 * CreateSession
 *
 * @brief Create session for UsrCtxt
 *
 * @param pUsrCtxt
 * @param pProcChain
 *
 * @return CameraResult
 */
CameraResult AisPProcNode::CreateSession(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain)
{
    CameraResult rc = CAMERA_SUCCESS;

    AisPProcNodePrivate* pCtxt = new AisPProcNodePrivate();
    m_pCtxt = pCtxt;

    rc = pCtxt->LoadLibrary(pProcChain->pLibName);
    if (CAMERA_SUCCESS == rc)
    {
        rc = pCtxt->Init(pUsrCtxt, pProcChain);
    }

    return rc;
};

/**
 * DestroySession
 *
 * @brief Destroy session for UsrCtxt
 *
 * @param pUsrCtxt
 * @param pProcChain
 *
 * @return CameraResult
 */
CameraResult AisPProcNode::DestroySession(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain)
{
    AisPProcNodePrivate* pCtxt = (AisPProcNodePrivate*)m_pCtxt;

    pCtxt->Deinit();
    delete pCtxt;
    m_pCtxt = NULL;

    return CAMERA_SUCCESS;
};


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
CameraResult AisPProcNode::ProcessEvent(AisUsrCtxt* pUsrCtxt, AisEventMsgType* pEvent)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisPProcNodePrivate* pCtxt = (AisPProcNodePrivate*)m_pCtxt;
    AisEventPProcJobType* pJob = &pEvent->payload.pprocJob;
    pp_job_t pp_job = {};
    AisBuffer* pOutBuffer[AIS_PPROC_MAX_BUFLISTS] = {};

    pUsrCtxt->Lock();

    //context must be streaming
    if (pUsrCtxt->m_state != AIS_USR_STATE_STREAMING)
    {
        AIS_LOG(ENGINE, ERROR, "Context %p in bad state %d", pUsrCtxt, pUsrCtxt->m_state);
        rc = CAMERA_EBADSTATE;
    }

    //get input buffers
    if (CAMERA_SUCCESS == rc)
    {
        for (uint32 i = 0; i < pJob->pProcChain->numIn; i++)
        {
            AisBuflistIdType bufferlistId = pJob->pProcChain->inBuflistId[i];
            AisBufferList* pBufferListIn = pUsrCtxt->m_bufferList[bufferlistId];

            if (NULL == pBufferListIn)
            {
                AIS_LOG(ENGINE, ERROR, "undefined bufferlist ID %d", i);
                rc = CAMERA_EFAILED;
                break;
            }

            AisBuffer* pInBuffer = pBufferListIn->GetReadyBuffer(pJob->jobId);
            if (!pInBuffer)
            {
                AIS_LOG(ENGINE, ERROR, "Context %p cannot find input buffer[%d] for jobId 0x%llx",
                        pUsrCtxt, bufferlistId, pJob->jobId);
                rc = CAMERA_ENOREADYBUFFER;
            }
            else
            {
                pp_job.src_buf_idx[i] = pInBuffer->idx;
            }
        }
    }

    //get output buffers
    if (CAMERA_SUCCESS == rc)
    {
        for (uint32 i = 0; i < pJob->pProcChain->numOut; i++)
        {
            AisBuflistIdType bufferlistId = pJob->pProcChain->outBuflistId[i];
            AisBufferList* pBufferListOut = pUsrCtxt->m_bufferList[bufferlistId];

            if (NULL == pBufferListOut)
            {
                AIS_LOG(ENGINE, ERROR, "undefined bufferlist ID %d", i);
                rc = CAMERA_EFAILED;
                break;
            }

            pOutBuffer[i] = pBufferListOut->GetFreeBuffer(pUsrCtxt);
            if (!pOutBuffer[i])
            {
                PPROC_LOG(ERROR, "No free buffers (error %d), dropped frame!", rc);
                rc = CAMERA_EFAILED;
            }

            pp_job.tgt_buf_idx[i] = pOutBuffer[i]->idx;
        }

        //process job
        if (CAMERA_SUCCESS == rc)
        {
            pp_job.job_id = pJob->jobId;
            pp_job.frame_id = pJob->frameInfo.seq_no[0];

            pUsrCtxt->Unlock();

            rc = pCtxt->ProcessFrame(&pp_job);

            pUsrCtxt->Lock();

            //queue ready buffers if success or recycle if failed
            for (uint32 i = 0; i < pJob->pProcChain->numOut; i++)
            {
                AisBuflistIdType bufferlistId = pJob->pProcChain->outBuflistId[i];
                AisBufferList* pBufferListOut = pUsrCtxt->m_bufferList[bufferlistId];

                if (CAMERA_SUCCESS == rc && PP_BUFFER_STATE_PROCESSED == pp_job.tgt_buf_state[i])
                {
                    pBufferListOut->QueueReadyBuffer(pJob->jobId, pOutBuffer[i]);
                }
                else
                {
                    AIS_LOG(ENGINE, DBG, "output buffer %d(%d) not processed...", i, pOutBuffer[i]->idx);
                    pBufferListOut->SetBufferState(pOutBuffer[i]->idx, AIS_BUFFER_INITIALIZED);
                    pBufferListOut->ReturnBuffer(pUsrCtxt, pOutBuffer[i]->idx);
                }
            }
        }

        //return input buffers
        for (uint32 i = 0; i < pJob->pProcChain->numIn; i++)
        {
            AisBuflistIdType bufferlistId = pJob->pProcChain->inBuflistId[i];
            AisBufferList* pBufferListIn = pUsrCtxt->m_bufferList[bufferlistId];

            pBufferListIn->SetBufferState(pp_job.src_buf_idx[i], AIS_BUFFER_INITIALIZED);
            pBufferListIn->ReturnBuffer(pUsrCtxt, pp_job.src_buf_idx[i]);
        }
    }


    pUsrCtxt->Unlock();

    if (CAMERA_SUCCESS == rc)
    {
        pJob->status = rc;
        pEvent->eventId = AIS_EVENT_PPROC_JOB_DONE;
        AisEngine::GetInstance()->QueueEvent(pEvent);
    }

    return rc;
};

/**
 * SetParams
 *
 * @brief Apply Params for UsrCtxt
 *
 * @param pUsrCtxt
 *
 * @return CameraResult
 */
CameraResult AisPProcNode::SetParams(AisUsrCtxt* pUsrCtxt)
{
    CAM_UNUSED(pUsrCtxt);

    return CAMERA_SUCCESS;
};

/**
 * GetParams
 *
 * @brief Get Params for UsrCtxt
 *
 * @param pUsrCtxt
 * @param pParamOut
 *
 * @return CameraResult
 */
CameraResult AisPProcNode::GetParams(AisUsrCtxt* pUsrCtxt, void *pParamOut)
{
    CAM_UNUSED(pUsrCtxt);
    CAM_UNUSED(pParamOut);

    return CAMERA_SUCCESS;
};
