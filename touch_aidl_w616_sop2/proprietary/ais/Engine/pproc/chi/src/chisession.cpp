//******************************************************************************************************************************
// Copyright (c) 2019, 2021 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//******************************************************************************************************************************

#include <string.h>

#include "chimodule.h"
#include "chipipeline.h"
#include "chisession.h"
#include "ais_log.h"

#define ISP_LOG(lvl, fmt...) AIS_LOG(PPROC_ISP, lvl, fmt)

ChiSession::ChiSession(ChiModule* pChiModule) :
    m_pChiModule(pChiModule), m_sessionHandle(NULL)
{
    memset(&m_sessionCreateData, 0, sizeof(m_sessionCreateData));
}

ChiSession::~ChiSession()
{
}

void ChiSession::Flush()
{
   m_pChiModule->GetChiOps()->pFlushSession(m_pChiModule->GetContext(),m_sessionHandle);
}

ChiSession* ChiSession::Create(ChiPipeline**    ppPipelines,
                                 int            numPipelines,
                                 CHICALLBACKS*  pCallbacks,
                                 void*          pPrivData,
                                 ChiModule*     pChiModule)
{
    // Sanity check input params
    if ((NULL == ppPipelines) || (NULL == pCallbacks) || (NULL == pChiModule))
    {
        ISP_LOG(ERROR,
            "Invalid params: ppPipelines=%p, pCallbacks=%p, pChiModule=%p",
            ppPipelines, pCallbacks, pChiModule);
        return NULL;
    }

    ChiSession* pChiSession = new ChiSession(pChiModule);
    if (pChiSession != NULL)
    {
        if (pChiSession->Initialize(
                ppPipelines, numPipelines,pCallbacks,pPrivData) != CAMERA_SUCCESS)
        {
                ISP_LOG(ERROR,
                    "Failed to initialize ChiSession object.");
                delete pChiSession;
                return NULL;
        }
    }
    return pChiSession;
}

CameraResult ChiSession::Initialize(ChiPipeline**       ppPipelines,
                                       int              numPipelines,
                                       CHICALLBACKS*    pCallbacks,
                                       void*            pPrivData)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (numPipelines > MaxPipelinesPerSession)
    {
        ISP_LOG(ERROR,
            "Session cannot have more than %d pipelines.", MaxPipelinesPerSession);
        return CAMERA_EBADPARM;
    }

    for (int i = 0; i < numPipelines; i++)
    {
        rc = ppPipelines[i]->GetPipelineInfo(&(m_sessionCreateData.pPipelinesInfo[i]));
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR,
                "Failed to get pipeline info for pipeline num %d.", i);
            return CAMERA_EFAILED;
        }
    }
    m_sessionCreateData.numPipelines        = numPipelines;
    m_sessionCreateData.pCallbacks          = pCallbacks;
    m_sessionCreateData.pPrivCallbackData   = pPrivData;

    // Call camx chi api to create session
    //
    CHISESSIONFLAGS flags = {};
    flags.u.isNativeChi = 1;
    m_sessionHandle = m_pChiModule->GetChiOps()->pCreateSession(
        m_pChiModule->GetContext(),
        m_sessionCreateData.numPipelines,
        m_sessionCreateData.pPipelinesInfo,
        m_sessionCreateData.pCallbacks,
        m_sessionCreateData.pPrivCallbackData,
        flags);
    if (NULL == m_sessionHandle)
    {
        ISP_LOG(ERROR, "Failed to create session.");
        return CAMERA_EFAILED;
    }
    ISP_LOG(DBG,
        "Succeeded to create session, m_sessionHandle=%p.", m_sessionHandle);

    return rc;
}

void ChiSession::DestroySession()
{
    if (m_sessionHandle)
    {
        m_pChiModule->GetChiOps()->pDestroySession(
            m_pChiModule->GetContext(), m_sessionHandle, false);
        ISP_LOG(DBG,
            "Successfully destroyed session, m_sessionHandle=%p.", m_sessionHandle);
        m_sessionHandle = NULL;
    }
    delete this;
}
