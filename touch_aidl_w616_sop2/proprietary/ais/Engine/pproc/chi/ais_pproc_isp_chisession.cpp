/*!
* Copyright (c) 2016-2021, 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "ais_pproc_isp_chisession.h"
#include "ais_pproc_isp.h"


#include <stdio.h>
#include <math.h>
#if defined(__ANDROID__)|| defined(__AGL__)
#include <cutils/native_handle.h>
#include <video_color_fmt.h>
#endif
#include <system/camera_metadata.h>
#include <cstdlib>

#include "ais_log.h"
#include "ais_engine.h"
#include "ais_ife_configurer.h"

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

#ifdef __AGL__
#define YCBCR420_888_WIDTH_ALIGNMENT     128
#define YCBCR420_888_HEIGHT_ALIGNMENT    32
#else
#define YCBCR420_888_WIDTH_ALIGNMENT     64
#define YCBCR420_888_HEIGHT_ALIGNMENT    32
#endif // endif __AGL__


#define ISP_LOG(lvl, fmt...) AIS_LOG(PPROC_ISP, lvl, fmt)
#define CHECK_BIT(num, pos) ((num) & (0x1<<(pos)))

//////////////////////////////////////////////////////////////////////////////////
/// TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
/// FORWARD DECLARE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
/// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////////

const char* AisPProcIspChiSession::IQ_TAG_SECTION   = "org.quic.camera2.iqsettings";
const char* AisPProcIspChiSession::BPS_TAG_NAME      = "OEMBPSIQSetting";
const char* AisPProcIspChiSession::IPE_TAG_NAME      = "OEMIPEIQSetting";

//For BPS BG stats use case
const char* AisPProcIspChiSession::STATS_CONFIG_TAG_SECTION    = "org.quic.camera2.statsconfigs";
const char* AisPProcIspChiSession::AWB_STATS_CTL_TAG_NAME     = "AWBStatsControl";
const char* AisPProcIspChiSession::AEC_FRAME_CTL_TAG_NAME     = "AECFrameControl";
const int BG_REGION_NUM_H = 64;
const int BG_REGION_NUM_V = 48;

// For SHDR use case
const char* AisPProcIspChiSession::SHDR_TAG_NAME       = "OEMSHDRIQSetting";

//////////////////////////////////////////////////////////////////////////////////
/// FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////
#if defined(__QNXNTO__) || defined(CAMERA_UNITTEST)
void destory_pmem_buffer(AisBufferHandle* AisBufHndl)
{
    if (AisBufHndl)
    {
        free(AisBufHndl);
    }
}
#endif

AisPProcIspChiSession::AisPProcIspChiSession() :
    m_pRequiredStreams(NULL), m_numStreams(0),
    m_usecaseId(IspUsecaseMax), m_pChiPipeline(NULL), m_pChiSession(NULL),
    m_pReqsCompleteSig(NULL), m_pInputMetabufPool(NULL), m_InputMetabufPoolSize(0),
    m_pOutputMetabufPool(NULL), m_OutputMetabufPoolSize(0),
    m_pFrame2NumProcBufsMapMutex(NULL), m_pFrame2pAisEventPProcJobTypeMutex(NULL)
{
    m_aectagId = 0;
    memset(&m_callbacks, 0, sizeof(m_callbacks));
    m_callbacks.ChiProcessCaptureResult         = AisPProcIspChiSession::ChiProcCaptureResult;
    m_callbacks.ChiNotify                        = AisPProcIspChiSession::ChiProcNotify;
    m_callbacks.ChiProcessPartialCaptureResult    = AisPProcIspChiSession::ChiProcPartialCaptureResult;
    memset(&m_MetadataOps, 0, sizeof(m_MetadataOps));
    memset(&m_TagOps, 0, sizeof(m_TagOps));
    memset(m_OfflineRequests, 0, sizeof(m_OfflineRequests[0]) * maxRequests);
    memset(&m_requestPvtData, 0, sizeof(m_requestPvtData));
    memset(&m_submitRequest, 0, sizeof(m_submitRequest));
    memset(m_InputBufferMap, 0, sizeof(m_InputBufferMap));
    memset(m_OutputBufferMap, 0, sizeof(m_OutputBufferMap));
    memset(m_JpegOutputBufferMap, 0, sizeof(m_JpegOutputBufferMap));

    memset(m_SubmitReq, 0, sizeof(m_SubmitReq));
    memset(m_ProcRes, 0, sizeof(m_ProcRes));
    m_ProcFrameMin = 0;
    m_ProcFrameMax = 0;
    m_ProcFrameCum = 0;
    m_ProcFrameNSamples = 0;
    m_ProcFrameAvg = 0;

    m_pUsrCtxt = NULL;
}

AisPProcIspChiSession::~AisPProcIspChiSession()
{
}

CameraResult AisPProcIspChiSession::Flush()
{
    CameraResult rc = CAMERA_SUCCESS;

    m_pChiSession->Flush();

    return rc;
}


/**
 * Initialize
 *
 * @brief Initialize ISP chisession
 *
 * @return CameraResult
 */
CameraResult AisPProcIspChiSession::Initialize(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain)
{
    CameraResult rc = CAMERA_SUCCESS;

    rc = CameraCreateMutex(&m_pFrame2NumProcBufsMapMutex);
    if (NULL == m_pFrame2NumProcBufsMapMutex)
    {
        ISP_LOG(ERROR,
            "Failed to create Frame2NumProcBufsMapMutex mutex.");
        return CAMERA_EFAILED;
    }
    ISP_LOG(DBG,
        "Successfully created Frame2NumProcBufsMapMutex mutex, m_pFrame2NumProcBufsMapMutex=%p.",
        m_pFrame2NumProcBufsMapMutex);

    rc = CameraCreateMutex(&m_ApplySettingsMutex);
    if (NULL == m_ApplySettingsMutex)
    {
        ISP_LOG(ERROR,
            "Failed to create m_ApplySettingsMutex mutex.");
        return CAMERA_EFAILED;
    }
    ISP_LOG(DBG,
        "Successfully created m_ApplySettingsMutex mutex, m_ApplySettingsMutex=%p.",
        m_pFrame2NumProcBufsMapMutex);

    rc = CameraCreateSignal(&m_pReqsCompleteSig);
    if (NULL == m_pReqsCompleteSig)
    {
        ISP_LOG(ERROR,
            "Failed to create requests completed signal.");
        return CAMERA_EFAILED;
    }
    ISP_LOG(DBG,
        "Successfully created requests completed signal, m_pReqsCompleteSig=%p.",
        m_pReqsCompleteSig);

    rc = CameraCreateMutex(&m_pFrame2pAisEventPProcJobTypeMutex);
    if (NULL == m_pFrame2pAisEventPProcJobTypeMutex)
    {
        ISP_LOG(ERROR,
            "Failed to create m_pFrame2pAisEventPProcJobTypeMutex mutex.");
        return CAMERA_EFAILED;
    }
    ISP_LOG(DBG,
        "Successfully created m_pFrame2pAisEventPProcJobTypeMutex mutex, m_pFrame2pAisEventPProcJobTypeMutex=%p.",
        m_pFrame2pAisEventPProcJobTypeMutex);

    AisPProcIsp::GetInstance()->GetMetadataOp(&m_MetadataOps);
    AisPProcIsp::GetInstance()->GetTagOp(&m_TagOps);

    // Query AEC Frame controls
    if (m_TagOps.pQueryVendorTagLocation(
                STATS_CONFIG_TAG_SECTION,
                AEC_FRAME_CTL_TAG_NAME,
                &m_aectagId) != CDKResultSuccess)
    {
        ISP_LOG(ERROR,
                "Failed to get tag id for %s.%s", STATS_CONFIG_TAG_SECTION, AEC_FRAME_CTL_TAG_NAME);
    }

    m_pUsrCtxt = pUsrCtxt;
    AisBufferList* pInBufferList = m_pUsrCtxt->m_bufferList[pProcChain->inBuflistId[0]];
    AisBufferList* pOutBufferList = m_pUsrCtxt->m_bufferList[pProcChain->outBuflistId[0]];
    AisBufferList* pJpegOutBufferList = NULL;
#ifdef AIS_ISP_ENABLE_JPEG
    pJpegOutBufferList = m_pUsrCtxt->m_bufferList[pProcChain->outBuflistId[1]];
#endif

    if (pProcChain->instanceId >= AIS_USER_CTXT_MAX_ISP_INSTANCES)
    {
        ISP_LOG(ERROR,
            "ISP instance num exceeds maximum ISP instrance number %u", AIS_USER_CTXT_MAX_ISP_INSTANCES);
        return CAMERA_EFAILED;
    }

    if (AIS_ISP_CAMERA_ID_DEFAULT != m_pUsrCtxt->m_ispInstance[pProcChain->instanceId].cameraId)
    {
        m_cameraId = m_pUsrCtxt->m_ispInstance[pProcChain->instanceId].cameraId;
    }
    else if (pProcChain->instanceId == 0)
    {
        //if cameraId is not configured correctly, use ais input id as isp camera id for first instance.
        m_cameraId = pUsrCtxt->m_inputId;
        ISP_LOG(WARN, "ISP camera id is not configured correctly for instance %u, defaulting to ais input id %u",
           pProcChain->instanceId, pUsrCtxt->m_inputId);
    }
    else
    {
        ISP_LOG(ERROR,
            "ISP camera id is not configured correctly for instance %u", pProcChain->instanceId);
        return CAMERA_EFAILED;
    }

    // ISP usecase can be explicitly set for ISP or RGBIR operational mode only.
    // The legacy sHDR operational mode always uses UsecaseShdrBpsIpeAECAWB.
    if ((QCARCAM_OPMODE_ISP == m_pUsrCtxt->m_opMode) ||
        (QCARCAM_OPMODE_RGBIR == m_pUsrCtxt->m_opMode) ||
        (QCARCAM_OPMODE_INJECT == m_pUsrCtxt->m_opMode))
    {
        rc = QcarcamToChiIspUsecase(m_pUsrCtxt->m_ispInstance[pProcChain->instanceId].useCase, &m_usecaseId);
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(WARN, "Failed to convert ISP usecase for instance %u, defaulting to full sHDR pipeline",
                pProcChain->instanceId);
            m_usecaseId = UsecaseShdrBpsIpeAECAWB;
        }
    }
    else
    {
        m_usecaseId = UsecaseShdrBpsIpeAECAWB;
    }

    ISP_LOG(WARN, "Running ISP Pipeline = %d CameraId = %u for ISP instance %u",
        m_usecaseId, m_cameraId, pProcChain->instanceId);

    // 1. Create chi stream
    //
    rc = SetupStreams(pProcChain->inBuflistId, pProcChain->outBuflistId);
    if (rc != CAMERA_SUCCESS)
    {
        ISP_LOG(ERROR, "Failed to setup streams.");
        return rc;
    }

    // 2. Create pipelines
    //
    rc = CreatePipelines(m_cameraId);
    if (rc != CAMERA_SUCCESS)
    {
        ISP_LOG(ERROR, "Failed to create pipelines.");
        return rc;
    }

    // 3. Create sessions
    rc = CreateSessions();
    if (rc != CAMERA_SUCCESS)
    {
        ISP_LOG(ERROR, "Failed to create sessions.");
        return rc;
    }

    // 4. Allocate metadata pools
    //
    rc = CreateInputMetabufPool(maxRequests);
    if (rc != CAMERA_SUCCESS)
    {
        ISP_LOG(ERROR, "Failed to create input metadata pool.");
        return rc;
    }
    rc = CreateOutputMetabufPool(maxRequests);
    if (rc != CAMERA_SUCCESS)
    {
        ISP_LOG(ERROR, "Failed to create output metadata pool.");
        return rc;
    }

    // 5. Create buffer maps
    //
    rc = PrepareRDIBuffers(&m_pRequiredStreams[0], pInBufferList, m_InputBufferMap);
    if (rc != CAMERA_SUCCESS)
    {
        ISP_LOG(ERROR, "Failed to prepare input buffers.");
        return rc;
    }

    rc = PrepareRDIBuffers(&m_pRequiredStreams[1], pOutBufferList, m_OutputBufferMap);
    if (rc != CAMERA_SUCCESS)
    {
        ISP_LOG(ERROR, "Failed to prepare output buffers.");
        return rc;
    }

    if (pJpegOutBufferList != NULL)
    {
        rc = PrepareRDIBuffers(&m_pRequiredStreams[2], pJpegOutBufferList, m_JpegOutputBufferMap);
        if (rc != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "Failed to prepare output buffers.");
            return rc;
        }
    }

    return rc;
}

/**
 * Deinitialize
 *
 * @brief Initialize ISP scheduler
 *
 * @return None
 */
CameraResult AisPProcIspChiSession::Deinitialize(const AisProcChainType* pProcChain)
{
    CameraResult rc = CAMERA_SUCCESS;

    AisBufferList* pInBufferList = m_pUsrCtxt->m_bufferList[pProcChain->inBuflistId[0]];
    AisBufferList* pOutBufferList = m_pUsrCtxt->m_bufferList[pProcChain->outBuflistId[0]];
    AisBufferList* pJpegOutBufferList = m_pUsrCtxt->m_bufferList[pProcChain->outBuflistId[1]];

    if (m_pChiPipeline)
    {
        m_pChiPipeline->DeactivatePipeline(m_pChiSession);
        ISP_LOG(WARN,
            "Successfully deactivated pipeline = %p for session = %p.",
            m_pChiPipeline, m_pChiSession);
    }

    if (m_pChiSession)
    {
        ISP_LOG(WARN,
            "Destroying m_pChiSession = %p.", m_pChiSession);
        m_pChiSession->DestroySession();
        m_pChiSession = NULL;
    }

    if (m_pChiPipeline)
    {
        ISP_LOG(WARN,
            "Destroying pipeline = %p.", m_pChiPipeline);
        m_pChiPipeline->DestroyPipeline();
        m_pChiPipeline = NULL;
    }

    DestroyMetabufPools();

    if (m_pRequiredStreams)
    {
        for (uint32 i = 0; i < pInBufferList->m_nBuffers; i++)
        {
            if (m_InputBufferMap[i].pChiBuffer)
            {
                delete m_InputBufferMap[i].pChiBuffer;
                m_InputBufferMap[i].pChiBuffer = NULL;
            }

            if (m_InputBufferMap[i].pPrivateHndl)
            {
#if defined(__ANDROID__)|| defined(__AGL__)
                native_handle_delete((native_handle_t*)m_InputBufferMap[i].pPrivateHndl);
#elif defined(__QNXNTO__) || defined(CAMERA_UNITTEST)
                destory_pmem_buffer((AisBufferHandle*)m_InputBufferMap[i].pPrivateHndl);
#endif
                m_InputBufferMap[i].pPrivateHndl = NULL;
            }
        }

        for (uint32 i = 0; i < pOutBufferList->m_nBuffers; i++)
        {
            if (m_OutputBufferMap[i].pChiBuffer)
            {
                delete m_OutputBufferMap[i].pChiBuffer;
                m_OutputBufferMap[i].pChiBuffer = NULL;
            }

            if (m_OutputBufferMap[i].pPrivateHndl)
            {
#if defined(__ANDROID__)|| defined(__AGL__)
                native_handle_delete((native_handle_t*)m_OutputBufferMap[i].pPrivateHndl);
#elif defined(__QNXNTO__) || defined(CAMERA_UNITTEST)
                destory_pmem_buffer((AisBufferHandle*)m_OutputBufferMap[i].pPrivateHndl);
#endif
                m_OutputBufferMap[i].pPrivateHndl = NULL;
            }
        }

        if (pJpegOutBufferList != NULL)
        {
            for (uint32 i = 0; i < pJpegOutBufferList->m_nBuffers; i++)
            {
                if (m_JpegOutputBufferMap[i].pChiBuffer)
                {
                    delete m_JpegOutputBufferMap[i].pChiBuffer;
                    m_JpegOutputBufferMap[i].pChiBuffer = NULL;
                }

                if (m_JpegOutputBufferMap[i].pPrivateHndl)
                {
#if defined(__ANDROID__)|| defined(__AGL__)
                native_handle_delete((native_handle_t*)m_JpegOutputBufferMap[i].pPrivateHndl);
#elif defined(__QNXNTO__) || defined(CAMERA_UNITTEST)
                destory_pmem_buffer((AisBufferHandle*)m_JpegOutputBufferMap[i].pPrivateHndl);
#endif
                    m_JpegOutputBufferMap[i].pPrivateHndl = NULL;
                }
            }
        }

        delete[] m_pRequiredStreams;
        m_pRequiredStreams = NULL;
    }

    CameraLockMutex(m_pFrame2pAisEventPProcJobTypeMutex);
    if (!m_pFrame2pAisEventPProcJobType.empty())
    {
        std::map<UINT32, AisEventPProcJobType*>::iterator itr = m_pFrame2pAisEventPProcJobType.begin();
        while (itr != m_pFrame2pAisEventPProcJobType.end())
        {
            AisEventPProcJobType* pIspJob = itr->second;
            if (pIspJob)
            {
                ISP_LOG(MED, "Job 0x%x aborted %p", pIspJob->jobId, pIspJob);
                CameraFree(CAMERA_ALLOCATE_ID_UNASSIGNED, pIspJob);
            }
            itr = m_pFrame2pAisEventPProcJobType.erase(itr);
        }
    }
    CameraUnlockMutex(m_pFrame2pAisEventPProcJobTypeMutex);

    if (m_pReqsCompleteSig)
    {
        CameraDestroySignal(m_pReqsCompleteSig);
        m_pReqsCompleteSig = NULL;
    }

    if (m_pFrame2NumProcBufsMapMutex)
    {
        CameraDestroyMutex(m_pFrame2NumProcBufsMapMutex);
        m_pFrame2NumProcBufsMapMutex = NULL;
    }

    if (m_pFrame2pAisEventPProcJobTypeMutex)
    {
        CameraDestroyMutex(m_pFrame2pAisEventPProcJobTypeMutex);
        m_pFrame2pAisEventPProcJobTypeMutex = NULL;
    }

    if (m_ApplySettingsMutex)
    {
        CameraDestroyMutex(m_ApplySettingsMutex);
        m_ApplySettingsMutex = NULL;
    }

    return rc;
}

CameraResult AisPProcIspChiSession::ProcessFrame(AisEventPProcJobType* pIspJob)
{
    CameraResult rc = CAMERA_SUCCESS;

    CameraLockMutex(m_pFrame2pAisEventPProcJobTypeMutex);
    FramePPorcJobMap::iterator itr = m_pFrame2pAisEventPProcJobType.find(pIspJob->frameInfo.seq_no[0]);
    if (itr != m_pFrame2pAisEventPProcJobType.end())
    {
        CameraUnlockMutex(m_pFrame2pAisEventPProcJobTypeMutex);
        ISP_LOG(ERROR,
            "seq_no %d exists in pprocjob map", pIspJob->frameInfo.seq_no[0]);
        return CAMERA_EFAILED;
    }
    m_pFrame2pAisEventPProcJobType[pIspJob->frameInfo.seq_no[0]] = pIspJob;
    CameraUnlockMutex(m_pFrame2pAisEventPProcJobTypeMutex);

    rc = GenerateOfflineCaptureRequest(pIspJob);

   if(CAMERA_SUCCESS != rc)
   {
       CameraLockMutex(m_pFrame2pAisEventPProcJobTypeMutex);
       m_pFrame2pAisEventPProcJobType.erase(pIspJob->frameInfo.seq_no[0]);
       CameraUnlockMutex(m_pFrame2pAisEventPProcJobTypeMutex);
   }

   return rc;

}

CameraResult AisPProcIspChiSession::PrepareRDIBuffers(CHISTREAM* pChiStream,
    AisBufferList* pBufferList,
    ChiBufferMap* pBufferMap)
{
    CameraResult result = CAMERA_SUCCESS;
    CHISTREAMBUFFER* pChiBuffer = NULL;

    for (uint32 i = 0; i < pBufferList->m_nBuffers; i++)
    {
        pChiBuffer = new CHISTREAMBUFFER();
#if defined(__ANDROID__)|| defined(__AGL__)
        native_handle_t* pPrivateHndl = native_handle_create(1, 0);
        if (!pPrivateHndl) {
            delete pChiBuffer;
            pChiBuffer = NULL;
            result = CAMERA_EFAILED;
            break;
        }

        pPrivateHndl->data[0] = static_cast<int>(reinterpret_cast <uintptr_t>(pBufferList->m_pBuffers[i].pMemHndl));
#elif defined(__QNXNTO__) || defined(CAMERA_UNITTEST)
        AisBufferHandle *pPrivateHndl = (AisBufferHandle *)calloc(1, sizeof(*pPrivateHndl));
        if (!pPrivateHndl) {
            delete pChiBuffer;
            pChiBuffer = NULL;
            result = CAMERA_EFAILED;
            break;
        }

        pPrivateHndl->base = &pBufferList->m_pBuffers[i];
#endif
        pBufferMap[i].pPrivateHndl = pPrivateHndl;
        pBufferMap[i].pChiBuffer = pChiBuffer;

        pChiBuffer->size = sizeof(*pChiBuffer);
        pChiBuffer->pStream = pChiStream;

        pChiBuffer->bufferInfo.phBuffer = &pBufferMap[i].pPrivateHndl;
        pChiBuffer->bufferInfo.bufferType = CHIBUFFERTYPE::ChiGralloc;

        pChiBuffer->acquireFence.valid = FALSE;
        pChiBuffer->releaseFence.valid = FALSE;

        ISP_LOG(DBG,
            "RDI map buffer %i %p streambuffer %p",
            i, pChiBuffer->bufferInfo.phBuffer, pChiBuffer);
    }

    return result;
}

CameraResult AisPProcIspChiSession::SetupStreams(const AisBuflistIdType* inBuflistId, const AisBuflistIdType* outBuflistId)
{
    CameraResult rc = CAMERA_SUCCESS;

    AisBufferList* pInBufferList = m_pUsrCtxt->m_bufferList[inBuflistId[0]];
    AisBufferList* pOutBufferList = m_pUsrCtxt->m_bufferList[outBuflistId[0]];

    if (UsecaseBPSInputMipiRaw10OutFullP010 == m_usecaseId)
    {
        m_numStreams = 2;
        m_pRequiredStreams = new ChiStream[m_numStreams];
        if (NULL == m_pRequiredStreams)
        {
            ISP_LOG(ERROR,
                "Failed to alloc memory for %lu streams.", m_numStreams);
            return CAMERA_ENOMEMORY;
        }
        memset(m_pRequiredStreams, 0, sizeof(m_pRequiredStreams[0]) * m_numStreams);

        // Setup input stream
        //
        m_pRequiredStreams[0].streamType                = ChiStreamTypeInput;
        m_pRequiredStreams[0].width                     = pInBufferList->GetWidth();
        m_pRequiredStreams[0].height                    = pInBufferList->GetHeight();
        m_pRequiredStreams[0].format                    = ChiStreamFormatRaw10;
        m_pRequiredStreams[0].grallocUsage              = GrallocUsageSwReadOften | GrallocUsageSwWriteOften;
        m_pRequiredStreams[0].maxNumBuffers             = 1;
        m_pRequiredStreams[0].dataspace                 = DataspaceArbitrary;
        m_pRequiredStreams[0].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[0].pHalStream                = NULL;

        // Setup output stream
        //
        m_pRequiredStreams[1].streamType                = ChiStreamTypeOutput;
        m_pRequiredStreams[1].width                     = pOutBufferList->GetWidth();
        m_pRequiredStreams[1].height                    = pOutBufferList->GetHeight();
        m_pRequiredStreams[1].format                    = ChiStreamFormatP010; // ChiStreamFormatUBWCTP10
        m_pRequiredStreams[1].grallocUsage              = GrallocUsageSwReadOften | GrallocUsageSwWriteOften;
        m_pRequiredStreams[1].maxNumBuffers             = 1;
        m_pRequiredStreams[1].dataspace                 = DataspaceUnknown;
        m_pRequiredStreams[1].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[1].pHalStream                = NULL;
    }
    else if (UsecaseSHDRInputRaw == m_usecaseId)
    {
        m_numStreams = 2;
        m_pRequiredStreams = new ChiStream[m_numStreams];
        if (NULL == m_pRequiredStreams)
        {
            ISP_LOG(ERROR,
                "Failed to alloc memory for %lu streams.", m_numStreams);
            return CAMERA_ENOMEMORY;
        }
        memset(m_pRequiredStreams, 0, sizeof(m_pRequiredStreams[0]) * m_numStreams);

        // Setup input stream
        //
        m_pRequiredStreams[0].streamType                = ChiStreamTypeInput;
        m_pRequiredStreams[0].width                     = pInBufferList->GetWidth();
        m_pRequiredStreams[0].height                    = pInBufferList->GetHeight();
        m_pRequiredStreams[0].format                    = ChiStreamFormatRaw16;
        m_pRequiredStreams[0].grallocUsage              = 0;
        m_pRequiredStreams[0].maxNumBuffers             = 1;
        m_pRequiredStreams[0].dataspace                 = DataspaceArbitrary;
        m_pRequiredStreams[0].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[0].pHalStream                = NULL;

        // Setup output stream
        //
        m_pRequiredStreams[1].streamType                = ChiStreamTypeOutput;
        m_pRequiredStreams[1].width                     = pOutBufferList->GetWidth();
        m_pRequiredStreams[1].height                    = pOutBufferList->GetHeight();
        m_pRequiredStreams[1].format                    = ChiStreamFormatRaw16;
        m_pRequiredStreams[1].grallocUsage              = GrallocUsageSwReadOften | GrallocUsageSwWriteOften;
        m_pRequiredStreams[1].maxNumBuffers             = 1;
        m_pRequiredStreams[1].dataspace                 = DataspaceUnknown;
        m_pRequiredStreams[1].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[1].pHalStream                = NULL;
    }
    else if (UsecaseBPSInputMipiRaw10OutNV12 == m_usecaseId)
    {
        m_numStreams = 2;
        m_pRequiredStreams = new ChiStream[m_numStreams];
        if (NULL == m_pRequiredStreams)
        {
            ISP_LOG(ERROR,
                "Failed to alloc memory for %lu streams.", m_numStreams);
            return CAMERA_ENOMEMORY;
        }
        memset(m_pRequiredStreams, 0, sizeof(m_pRequiredStreams[0]) * m_numStreams);

        // Setup input stream
        //
        m_pRequiredStreams[0].streamType                = ChiStreamTypeInput;
        m_pRequiredStreams[0].width                     = pInBufferList->GetWidth();
        m_pRequiredStreams[0].height                    = pInBufferList->GetHeight();
        m_pRequiredStreams[0].format                    = ChiStreamFormatRaw10;
        m_pRequiredStreams[0].grallocUsage              = GrallocUsageSwReadOften | GrallocUsageSwWriteOften;
        m_pRequiredStreams[0].maxNumBuffers             = 1;
        m_pRequiredStreams[0].dataspace                 = DataspaceArbitrary;
        m_pRequiredStreams[0].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[0].pHalStream                = NULL;

        // Setup output stream
        //
        m_pRequiredStreams[1].streamType                = ChiStreamTypeOutput;
        m_pRequiredStreams[1].width                     = pOutBufferList->GetWidth();
        m_pRequiredStreams[1].height                    = pOutBufferList->GetHeight();
        m_pRequiredStreams[1].format                    = ChiStreamFormatYCbCr420_888; // ChiStreamFormatUBWCTP10
        m_pRequiredStreams[1].grallocUsage              = GrallocUsageSwWriteOften;
        m_pRequiredStreams[1].maxNumBuffers             = 1;
        m_pRequiredStreams[1].dataspace                 = DataspaceUnknown;
        m_pRequiredStreams[1].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[1].pHalStream                = NULL;

        m_pRequiredStreams[1].streamParams.planeStride =
            pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].stride;
        m_pRequiredStreams[1].streamParams.sliceHeight =
            pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].size / pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].stride;
    }
    else if (UsecaseBPSInputRaw16OutNV12 == m_usecaseId)
    {
        m_numStreams = 2;
        m_pRequiredStreams = new ChiStream[m_numStreams];
        if (NULL == m_pRequiredStreams)
        {
            ISP_LOG(ERROR,
                "Failed to alloc memory for %lu streams.", m_numStreams);
            return CAMERA_ENOMEMORY;
        }
        memset(m_pRequiredStreams, 0, sizeof(m_pRequiredStreams[0]) * m_numStreams);

        // Setup input stream
        //
        m_pRequiredStreams[0].streamType                = ChiStreamTypeInput;
        m_pRequiredStreams[0].width                     = pInBufferList->GetWidth();
        m_pRequiredStreams[0].height                    = pInBufferList->GetHeight();
        m_pRequiredStreams[0].format                    = ChiStreamFormatRaw16;
        m_pRequiredStreams[0].grallocUsage              = GrallocUsageSwReadOften | GrallocUsageSwWriteOften;
        m_pRequiredStreams[0].maxNumBuffers             = 1;
        m_pRequiredStreams[0].dataspace                 = DataspaceArbitrary;
        m_pRequiredStreams[0].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[0].pHalStream                = NULL;

        // Setup output stream
        //
        m_pRequiredStreams[1].streamType                = ChiStreamTypeOutput;
        m_pRequiredStreams[1].width                     = pOutBufferList->GetWidth();
        m_pRequiredStreams[1].height                    = pOutBufferList->GetHeight();
        m_pRequiredStreams[1].format                    = ChiStreamFormatYCbCr420_888;
        m_pRequiredStreams[1].grallocUsage              = GrallocUsageSwWriteOften;
        m_pRequiredStreams[1].maxNumBuffers             = 1;
        m_pRequiredStreams[1].dataspace                 = DataspaceUnknown;
        m_pRequiredStreams[1].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[1].pHalStream                = NULL;

        m_pRequiredStreams[1].streamParams.planeStride =
            pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].stride;
        m_pRequiredStreams[1].streamParams.sliceHeight =
            pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].size / pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].stride;
    }
#ifdef __QNXNTO__
    else if (UsecaseBPSInputRaw16OutP010 == m_usecaseId)
    {
        m_numStreams = 2;
        m_pRequiredStreams = new ChiStream[m_numStreams];
        if (NULL == m_pRequiredStreams)
        {
            ISP_LOG(ERROR,
                "Failed to alloc memory for %lu streams.", m_numStreams);
            return CAMERA_ENOMEMORY;
        }
        memset(m_pRequiredStreams, 0, sizeof(m_pRequiredStreams[0]) * m_numStreams);

        // Setup input stream
        //
        m_pRequiredStreams[0].streamType                = ChiStreamTypeInput;
        m_pRequiredStreams[0].width                     = pInBufferList->GetWidth();
        m_pRequiredStreams[0].height                    = pInBufferList->GetHeight();
        m_pRequiredStreams[0].format                    = ChiStreamFormatRaw16;
        m_pRequiredStreams[0].grallocUsage              = GrallocUsageSwReadOften | GrallocUsageSwWriteOften;
        m_pRequiredStreams[0].maxNumBuffers             = 1;
        m_pRequiredStreams[0].dataspace                 = DataspaceArbitrary;
        m_pRequiredStreams[0].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[0].pHalStream                = NULL;

        // Setup output stream
        //
        m_pRequiredStreams[1].streamType                = ChiStreamTypeOutput;
        m_pRequiredStreams[1].width                     = pOutBufferList->GetWidth();
        m_pRequiredStreams[1].height                    = pOutBufferList->GetHeight();
        m_pRequiredStreams[1].format                    = ChiStreamFormatP010;
        m_pRequiredStreams[1].grallocUsage              = GrallocUsageSwWriteOften;
        m_pRequiredStreams[1].maxNumBuffers             = 1;
        m_pRequiredStreams[1].dataspace                 = DataspaceUnknown;
        m_pRequiredStreams[1].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[1].pHalStream                = NULL;
    }
#endif
    else if(UsecaseBPSInputMipiRaw10OutFullP010Stats == m_usecaseId)
    {
        m_numStreams = 4;
        m_pRequiredStreams = new ChiStream[m_numStreams];
        if (NULL == m_pRequiredStreams)
        {
            ISP_LOG(ERROR,
                "Failed to alloc memory for %lu streams.", m_numStreams);
            return CAMERA_ENOMEMORY;
        }
        memset(m_pRequiredStreams, 0, sizeof(m_pRequiredStreams[0]) * m_numStreams);
        int stream_index = 0;
        // Setup input stream
        //
        m_pRequiredStreams[stream_index].streamType                = ChiStreamTypeInput;
        m_pRequiredStreams[stream_index].width                     = pInBufferList->GetWidth();
        m_pRequiredStreams[stream_index].height                    = pInBufferList->GetHeight();
        m_pRequiredStreams[stream_index].format                    = ChiStreamFormatRaw10;
        m_pRequiredStreams[stream_index].grallocUsage              = GrallocUsageSwReadOften | GrallocUsageSwWriteOften;
        m_pRequiredStreams[stream_index].maxNumBuffers             = 1;
        m_pRequiredStreams[stream_index].dataspace                 = DataspaceArbitrary;
        m_pRequiredStreams[stream_index].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[stream_index].pHalStream                = NULL;

        // Setup output stream
        //
        stream_index++;
        m_pRequiredStreams[stream_index].streamType                = ChiStreamTypeOutput;
        m_pRequiredStreams[stream_index].width                     = pOutBufferList->GetWidth();
        m_pRequiredStreams[stream_index].height                    = pOutBufferList->GetHeight();
        m_pRequiredStreams[stream_index].format                    = ChiStreamFormatP010; // ChiStreamFormatUBWCTP10
        m_pRequiredStreams[stream_index].grallocUsage              = GrallocUsageSwReadOften | GrallocUsageSwWriteOften;
        m_pRequiredStreams[stream_index].maxNumBuffers             = 1;
        m_pRequiredStreams[stream_index].dataspace                 = DataspaceUnknown;
        m_pRequiredStreams[stream_index].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[stream_index].pHalStream                = NULL;

        // Setup BG stats output stream
        stream_index++;
        m_pRequiredStreams[stream_index].streamType                = ChiStreamTypeOutput;
        m_pRequiredStreams[stream_index].width                     = 691200;//0x59000;
        m_pRequiredStreams[stream_index].height                    = 1;
        m_pRequiredStreams[stream_index].format                    = ChiStreamFormatBlob; // ChiStreamFormatUBWCTP10
        m_pRequiredStreams[stream_index].grallocUsage              =  GrallocUsageSwReadOften | GrallocUsageSwWriteOften | GrallocUsageHwCameraWrite;
        m_pRequiredStreams[stream_index].maxNumBuffers             = 1;
        m_pRequiredStreams[stream_index].dataspace                 = DataspaceUnknown;
        m_pRequiredStreams[stream_index].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[stream_index].pHalStream                = NULL;

        // Setup hdr hist stats output stream
        stream_index++;
        m_pRequiredStreams[stream_index].streamType                = ChiStreamTypeOutput;
        m_pRequiredStreams[stream_index].width                     = 3072;
        m_pRequiredStreams[stream_index].height                    = 1;
        m_pRequiredStreams[stream_index].format                    = ChiStreamFormatBlob; // ChiStreamFormatUBWCTP10
        m_pRequiredStreams[stream_index].grallocUsage              =  GrallocUsageSwReadOften | GrallocUsageSwWriteOften | GrallocUsageHwCameraWrite;
        m_pRequiredStreams[stream_index].maxNumBuffers             = 1;
        m_pRequiredStreams[stream_index].dataspace                 = DataspaceUnknown;
        m_pRequiredStreams[stream_index].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[stream_index].pHalStream                = NULL;
    }
    else if(UsecaseBPSInputRaw16OutNV12Stats == m_usecaseId)
    {
        m_numStreams = 4;
        m_pRequiredStreams = new ChiStream[m_numStreams];
        if (NULL == m_pRequiredStreams)
        {
            ISP_LOG(ERROR,
                "Failed to alloc memory for %lu streams.", m_numStreams);
            return CAMERA_ENOMEMORY;
        }
        memset(m_pRequiredStreams, 0, sizeof(m_pRequiredStreams[0]) * m_numStreams);
        int stream_index = 0;
        // Setup input stream
        //
        m_pRequiredStreams[stream_index].streamType                = ChiStreamTypeInput;
        m_pRequiredStreams[stream_index].width                     = pInBufferList->GetWidth();
        m_pRequiredStreams[stream_index].height                    = pInBufferList->GetHeight();
        m_pRequiredStreams[stream_index].format                    = ChiStreamFormatRaw16;
        m_pRequiredStreams[stream_index].grallocUsage              = GrallocUsageSwReadOften | GrallocUsageSwWriteOften;
        m_pRequiredStreams[stream_index].maxNumBuffers             = 1;
        m_pRequiredStreams[stream_index].dataspace                 = DataspaceArbitrary;
        m_pRequiredStreams[stream_index].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[stream_index].pHalStream                = NULL;

        // Setup output stream
        //
        stream_index++;
        m_pRequiredStreams[stream_index].streamType                = ChiStreamTypeOutput;
        m_pRequiredStreams[stream_index].width                     = pOutBufferList->GetWidth();
        m_pRequiredStreams[stream_index].height                    = pOutBufferList->GetHeight();
        m_pRequiredStreams[stream_index].format                    = ChiStreamFormatYCbCr420_888;
        m_pRequiredStreams[stream_index].grallocUsage              = GrallocUsageSwReadOften | GrallocUsageSwWriteOften;
        m_pRequiredStreams[stream_index].maxNumBuffers             = 1;
        m_pRequiredStreams[stream_index].dataspace                 = DataspaceUnknown;
        m_pRequiredStreams[stream_index].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[stream_index].pHalStream                = NULL;

        m_pRequiredStreams[1].streamParams.planeStride =
            pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].stride;
        m_pRequiredStreams[1].streamParams.sliceHeight =
            pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].size / pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].stride;

        // Setup BG stats output stream
        stream_index++;
        m_pRequiredStreams[stream_index].streamType                = ChiStreamTypeOutput;
        m_pRequiredStreams[stream_index].width                     = 691200;//0x59000;
        m_pRequiredStreams[stream_index].height                    = 1;
        m_pRequiredStreams[stream_index].format                    = ChiStreamFormatBlob; // ChiStreamFormatUBWCTP10
        m_pRequiredStreams[stream_index].grallocUsage              =  GrallocUsageSwReadOften | GrallocUsageSwWriteOften | GrallocUsageHwCameraWrite;
        m_pRequiredStreams[stream_index].maxNumBuffers             = 1;
        m_pRequiredStreams[stream_index].dataspace                 = DataspaceUnknown;
        m_pRequiredStreams[stream_index].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[stream_index].pHalStream                = NULL;

        // Setup hdr hist stats output stream
        stream_index++;
        m_pRequiredStreams[stream_index].streamType                = ChiStreamTypeOutput;
        m_pRequiredStreams[stream_index].width                     = 3072;
        m_pRequiredStreams[stream_index].height                    = 1;
        m_pRequiredStreams[stream_index].format                    = ChiStreamFormatBlob; // ChiStreamFormatUBWCTP10
        m_pRequiredStreams[stream_index].grallocUsage              =  GrallocUsageSwReadOften | GrallocUsageSwWriteOften | GrallocUsageHwCameraWrite;
        m_pRequiredStreams[stream_index].maxNumBuffers             = 1;
        m_pRequiredStreams[stream_index].dataspace                 = DataspaceUnknown;
        m_pRequiredStreams[stream_index].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[stream_index].pHalStream                = NULL;
    }
#ifdef AIS_ISP_ENABLE_JPEG
    else if(UsecaseShdrBpsIpeAECAWB == m_usecaseId) {
          m_numStreams = 3;
          m_pRequiredStreams = new ChiStream[m_numStreams];
          if (NULL == m_pRequiredStreams)
          {
              ISP_LOG(ERROR,
                  "Failed to alloc memory for %lu streams.", m_numStreams);
              return CAMERA_ENOMEMORY;
          }
        memset(m_pRequiredStreams, 0, sizeof(m_pRequiredStreams[0]) * m_numStreams);
        int stream_index=0;
        AisBufferList* pJpegOutBufferList = m_pUsrCtxt->m_bufferList[outBuflistId[1]];
        // Setup input stream
        //
        m_pRequiredStreams[stream_index].streamType                = ChiStreamTypeInput;
        m_pRequiredStreams[stream_index].width                     = pInBufferList->GetWidth();
        m_pRequiredStreams[stream_index].height                    = pInBufferList->GetHeight();
        m_pRequiredStreams[stream_index].format                    = ChiStreamFormatRaw16;
        m_pRequiredStreams[stream_index].grallocUsage              = GrallocUsageSwReadOften | GrallocUsageSwWriteOften;
        m_pRequiredStreams[stream_index].maxNumBuffers             = 1;
        m_pRequiredStreams[stream_index].dataspace                 = DataspaceArbitrary;
        m_pRequiredStreams[stream_index].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[stream_index].pHalStream                = NULL;

        stream_index++;
        m_pRequiredStreams[stream_index].streamType                = ChiStreamTypeOutput;
        m_pRequiredStreams[stream_index].width                     = pOutBufferList->GetWidth();
        m_pRequiredStreams[stream_index].height                    = pOutBufferList->GetHeight();
        m_pRequiredStreams[stream_index].format                    = ChiStreamFormatYCbCr420_888;
        m_pRequiredStreams[stream_index].grallocUsage              = GrallocUsageSwWriteOften;
        m_pRequiredStreams[stream_index].maxNumBuffers             = 1;
        m_pRequiredStreams[stream_index].dataspace                 = DataspaceUnknown;
        m_pRequiredStreams[stream_index].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[stream_index].pHalStream                = NULL;

        m_pRequiredStreams[1].streamParams.planeStride =
            pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].stride;
        m_pRequiredStreams[1].streamParams.sliceHeight =
            pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].size / pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].stride;

        stream_index++;
        m_pRequiredStreams[stream_index].streamType                = ChiStreamTypeOutput;
        m_pRequiredStreams[stream_index].width                     = pJpegOutBufferList->GetWidth()/3;
        m_pRequiredStreams[stream_index].height                    = pJpegOutBufferList->GetHeight();
        m_pRequiredStreams[stream_index].format                    = ChiStreamFormatBlob;
        m_pRequiredStreams[stream_index].grallocUsage              = GrallocUsageSwReadOften | GrallocUsageSwWriteOften;
        m_pRequiredStreams[stream_index].maxNumBuffers             = 1;
        m_pRequiredStreams[stream_index].dataspace                 = DataspaceJFIF;
        m_pRequiredStreams[stream_index].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[stream_index].pHalStream                = NULL;
    }
#else
    // Pipeline without JPEG
    else if(UsecaseShdrBpsIpeAECAWB == m_usecaseId){
          m_numStreams = 2;
          m_pRequiredStreams = new ChiStream[m_numStreams];
          if (NULL == m_pRequiredStreams)
          {
              ISP_LOG(ERROR,
                  "Failed to alloc memory for %lu streams.", m_numStreams);
              return CAMERA_ENOMEMORY;
          }
        memset(m_pRequiredStreams, 0, sizeof(m_pRequiredStreams[0]) * m_numStreams);
        int stream_index=0;
        // Setup input stream
        //
        m_pRequiredStreams[stream_index].streamType                = ChiStreamTypeInput;
        m_pRequiredStreams[stream_index].width                     = pInBufferList->GetWidth();
        m_pRequiredStreams[stream_index].height                    = pInBufferList->GetHeight();
        m_pRequiredStreams[stream_index].format                    = ChiStreamFormatRaw16;
        m_pRequiredStreams[stream_index].grallocUsage              = GrallocUsageSwReadOften | GrallocUsageSwWriteOften;
        m_pRequiredStreams[stream_index].maxNumBuffers             = 1;
        m_pRequiredStreams[stream_index].dataspace                 = DataspaceArbitrary;
        m_pRequiredStreams[stream_index].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[stream_index].pHalStream                = NULL;

        stream_index++;
        m_pRequiredStreams[stream_index].streamType                = ChiStreamTypeOutput;
        m_pRequiredStreams[stream_index].width                     = pOutBufferList->GetWidth();
        m_pRequiredStreams[stream_index].height                    = pOutBufferList->GetHeight();
        m_pRequiredStreams[stream_index].format                    = ChiStreamFormatYCbCr420_888;
        m_pRequiredStreams[stream_index].grallocUsage              = GrallocUsageSwWriteOften;
        m_pRequiredStreams[stream_index].maxNumBuffers             = 1;
        m_pRequiredStreams[stream_index].dataspace                 = DataspaceUnknown;
        m_pRequiredStreams[stream_index].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[stream_index].pHalStream                = NULL;

        m_pRequiredStreams[1].streamParams.planeStride =
            pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].stride;
        m_pRequiredStreams[1].streamParams.sliceHeight =
            pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].size / pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].stride;

    }
#endif
    else if (UsecaseShdrBpsInputRaw16OutNV12 == m_usecaseId ||
             UsecaseShdrBpsStatsProcInputRaw16OutNV12 == m_usecaseId ||
             UsecaseShdrBpsAECAWB == m_usecaseId )
    {
        m_numStreams = 2;
        m_pRequiredStreams = new ChiStream[m_numStreams];
        if (NULL == m_pRequiredStreams)
        {
            ISP_LOG(ERROR,
                "Failed to alloc memory for %lu streams.", m_numStreams);
            return CAMERA_ENOMEMORY;
        }
        memset(m_pRequiredStreams, 0, sizeof(m_pRequiredStreams[0]) * m_numStreams);

        // Setup input stream
        //
        m_pRequiredStreams[0].streamType                = ChiStreamTypeInput;
        m_pRequiredStreams[0].width                     = pInBufferList->GetWidth();
        m_pRequiredStreams[0].height                    = pInBufferList->GetHeight();
        m_pRequiredStreams[0].format                    = ChiStreamFormatRaw16;
        m_pRequiredStreams[0].grallocUsage              = GrallocUsageSwReadOften | GrallocUsageSwWriteOften;
        m_pRequiredStreams[0].maxNumBuffers             = 1;
        m_pRequiredStreams[0].dataspace                 = DataspaceArbitrary;
        m_pRequiredStreams[0].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[0].pHalStream                = NULL;

        // Setup output stream
        //
        m_pRequiredStreams[1].streamType                = ChiStreamTypeOutput;
        m_pRequiredStreams[1].width                     = pOutBufferList->GetWidth();
        m_pRequiredStreams[1].height                    = pOutBufferList->GetHeight();
        m_pRequiredStreams[1].format                    = ChiStreamFormatYCbCr420_888;
        m_pRequiredStreams[1].grallocUsage              = GrallocUsageSwWriteOften;
        m_pRequiredStreams[1].maxNumBuffers             = 1;
        m_pRequiredStreams[1].dataspace                 = DataspaceUnknown;
        m_pRequiredStreams[1].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[1].pHalStream                = NULL;

        m_pRequiredStreams[1].streamParams.planeStride =
            pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].stride;
        m_pRequiredStreams[1].streamParams.sliceHeight =
            pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].size / pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].stride;

    }
    else if (UsecaseBpsAWBBGStats == m_usecaseId ||
             UsecaseBPSBGBHISTAWB == m_usecaseId ||
             UsecaseBPSAECAWB == m_usecaseId     ||
             UsecaseBPSAECAWBIPEInputMipiRaw16OutNV12 == m_usecaseId)
    {
        m_numStreams = 2;
        m_pRequiredStreams = new ChiStream[m_numStreams];
        if (NULL == m_pRequiredStreams)
        {
            ISP_LOG(ERROR,
                "Failed to alloc memory for %lu streams.", m_numStreams);
            return CAMERA_ENOMEMORY;
        }
        memset(m_pRequiredStreams, 0, sizeof(m_pRequiredStreams[0]) * m_numStreams);

        // Setup input stream
        //
        m_pRequiredStreams[0].streamType                 = ChiStreamTypeInput;
        m_pRequiredStreams[0].width                      = pInBufferList->GetWidth();
        m_pRequiredStreams[0].height                     = pInBufferList->GetHeight();
        m_pRequiredStreams[0].format                     = ChiStreamFormatRaw16;
        m_pRequiredStreams[0].grallocUsage               = GrallocUsageSwReadOften | GrallocUsageSwWriteOften;
        m_pRequiredStreams[0].maxNumBuffers              = 1;
        m_pRequiredStreams[0].dataspace                  = DataspaceArbitrary;
        m_pRequiredStreams[0].rotation                   = StreamRotationCCW0;
        m_pRequiredStreams[0].pHalStream                 = NULL;

        // Setup output stream
        //
        m_pRequiredStreams[1].streamType                 = ChiStreamTypeOutput;
        m_pRequiredStreams[1].width                      = pOutBufferList->GetWidth();
        m_pRequiredStreams[1].height                     = pOutBufferList->GetHeight();
        m_pRequiredStreams[1].format                     = ChiStreamFormatYCbCr420_888;
        m_pRequiredStreams[1].grallocUsage               = GrallocUsageSwWriteOften;
        m_pRequiredStreams[1].maxNumBuffers              = 1;
        m_pRequiredStreams[1].dataspace                  = DataspaceUnknown;
        m_pRequiredStreams[1].rotation                   = StreamRotationCCW0;
        m_pRequiredStreams[1].pHalStream                 = NULL;

        m_pRequiredStreams[1].streamParams.planeStride =
            pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].stride;
        m_pRequiredStreams[1].streamParams.sliceHeight =
            pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].size / pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].stride;
    }
    else if ((UsecaseIPEInputNV12OutUBWCNV12 == m_usecaseId) ||
        (UsecaseIPEInputNV12OutNV12 == m_usecaseId))
    {
        m_numStreams = 2;
        m_pRequiredStreams = new ChiStream[m_numStreams];
        if (NULL == m_pRequiredStreams)
        {
            ISP_LOG(ERROR,
                    "Failed to alloc memory for %lu streams.", m_numStreams);
            return CAMERA_ENOMEMORY;
        }
        memset(m_pRequiredStreams, 0, sizeof(m_pRequiredStreams[0]) * m_numStreams);

        //Setup input stream
        m_pRequiredStreams[0].streamType                = ChiStreamTypeInput;
        m_pRequiredStreams[0].width                     = pInBufferList->GetWidth();
        m_pRequiredStreams[0].height                    = pInBufferList->GetHeight();
        m_pRequiredStreams[0].format                    = ChiStreamFormatYCbCr420_888;
        m_pRequiredStreams[0].grallocUsage              = GrallocUsageSwReadOften | GrallocUsageSwWriteOften;
        m_pRequiredStreams[0].maxNumBuffers             = 1;
        m_pRequiredStreams[1].dataspace                 = DataspaceUnknown;
        m_pRequiredStreams[1].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[1].pHalStream                = NULL;

        //setup output stream
        m_pRequiredStreams[1].streamType                = ChiStreamTypeOutput;
        m_pRequiredStreams[1].width                     = pOutBufferList->GetWidth();
        m_pRequiredStreams[1].height                    = pOutBufferList->GetHeight();

        if (UsecaseIPEInputNV12OutUBWCNV12 == m_usecaseId)
        {
            m_pRequiredStreams[1].format                = ChiStreamFormatUBWCNV12;
        }
        else if (UsecaseIPEInputNV12OutNV12 == m_usecaseId)
        {
            m_pRequiredStreams[1].format                = ChiStreamFormatYCbCr420_888;
        }

        m_pRequiredStreams[1].grallocUsage              = GrallocUsageSwWriteOften; // | GRALLOC_USAGE_PRIVATE_0 | PRIVATE_GRALLOC_USAGE_TP10;
        m_pRequiredStreams[1].maxNumBuffers             = 1;
        m_pRequiredStreams[1].dataspace                 = DataspaceUnknown;
        m_pRequiredStreams[1].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[1].pHalStream                = NULL;
    }
    else if (UsecaseBPSIPEInputMipiRaw16OutNV12 == m_usecaseId)
    {
        m_numStreams = 2;
        m_pRequiredStreams = new ChiStream[m_numStreams];
        if (NULL == m_pRequiredStreams)
        {
            ISP_LOG(ERROR,
                    "Failed to alloc memory for %lu streams.", m_numStreams);
            return CAMERA_ENOMEMORY;
        }
        memset(m_pRequiredStreams, 0, sizeof(m_pRequiredStreams[0]) * m_numStreams);

        // Setup input stream
        //
        m_pRequiredStreams[0].streamType                = ChiStreamTypeInput;
        m_pRequiredStreams[0].width                     = pInBufferList->GetWidth();
        m_pRequiredStreams[0].height                    = pInBufferList->GetHeight();
        m_pRequiredStreams[0].format                    = ChiStreamFormatRaw16;
        m_pRequiredStreams[0].grallocUsage              = GrallocUsageSwReadOften | GrallocUsageSwWriteOften;
        m_pRequiredStreams[0].maxNumBuffers             = 1;
        m_pRequiredStreams[0].dataspace                 = DataspaceArbitrary;
        m_pRequiredStreams[0].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[0].pHalStream                = NULL;

        //setup output stream
        m_pRequiredStreams[1].streamType                = ChiStreamTypeOutput;
        m_pRequiredStreams[1].width                     = pOutBufferList->GetWidth();
        m_pRequiredStreams[1].height                    = pOutBufferList->GetHeight();
        m_pRequiredStreams[1].format                    = ChiStreamFormatYCbCr420_888;
        m_pRequiredStreams[1].grallocUsage              = GrallocUsageSwWriteOften; // | GRALLOC_USAGE_PRIVATE_0 | PRIVATE_GRALLOC_USAGE_TP10;
        m_pRequiredStreams[1].maxNumBuffers             = 1;
        m_pRequiredStreams[1].dataspace                 = DataspaceUnknown;
        m_pRequiredStreams[1].rotation                  = StreamRotationCCW0;
        m_pRequiredStreams[1].pHalStream                = NULL;

        m_pRequiredStreams[1].streamParams.planeStride =
            pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].stride;
        m_pRequiredStreams[1].streamParams.sliceHeight =
            pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].size / pOutBufferList->m_pBuffers[0].bufferInfo.planes[0].stride;

    }
    return rc;
}

CameraResult AisPProcIspChiSession::CreatePipelines(uint32 cameraId)
{
    CameraResult rc = CAMERA_SUCCESS;

    PipelineType type = PipelineTypeMax;

    switch (m_usecaseId)
    {
        case UsecaseBPSInputMipiRaw10OutFullP010:
        case UsecaseBPSInputMipiRaw10OutNV12:
        case UsecaseBPSInputRaw16OutNV12:
#ifdef __QNXNTO__
        case UsecaseBPSInputRaw16OutP010:
#endif
            type = OfflineBPS;
            break;
        case UsecaseBPSInputMipiRaw10OutFullP010Stats:
        case UsecaseBPSInputRaw16OutNV12Stats:
            type = OfflineBPSStats;
            break;
        case UsecaseSHDRInputRaw:
            type = OfflineSHDR;
            break;
        case UsecaseShdrBpsInputRaw16OutNV12:
            type = OfflineShdrBps;
            break;
        case UsecaseShdrBpsStatsProcInputRaw16OutNV12:
            type = OfflineShdrBpsStatsProc;
            break;
        case UsecaseBpsAWBBGStats:
            type = OfflineBPSAWBBGStats;
            break;
        case UsecaseIPEInputNV12OutUBWCNV12:
        case UsecaseIPEInputNV12OutNV12:
            type = OfflineIPE;
            break;
        case UsecaseBPSBGBHISTAWB:
            type = OfflineBPSBGBHISTAWB;
            break;
        case UsecaseShdrBpsAECAWB:
            type = OfflineShdrBpsAECAWB;
            break;
        case UsecaseBPSAECAWB:
            type = OfflineBPSAECAWB;
            break;
        case UsecaseBPSIPEInputMipiRaw16OutNV12:
            type = OfflineBpsIpe;
            break;
        case UsecaseBPSAECAWBIPEInputMipiRaw16OutNV12:
            type = OfflineBpsIpeAECAWB;
            break;
        case UsecaseShdrBpsIpeAECAWB:
            type = OfflineShdrBpsIpeAECAWB;
            break;
        default:
            ISP_LOG(ERROR, "Unsupported usecaseId %d", m_usecaseId);
            return CAMERA_EFAILED;
    }

    m_pChiPipeline = ChiPipeline::Create(m_pRequiredStreams,
                                             m_numStreams, cameraId,
                                             AisPProcIsp::GetInstance()->GetChiModule(),
                                             type);
    if (NULL == m_pChiPipeline)
    {
        ISP_LOG(ERROR, "Failed to create pipeline.");
        return CAMERA_EFAILED;
    }
    ISP_LOG(DBG,
            "Successfully created pipeline = %p.", m_pChiPipeline);

    return rc;
}

CameraResult AisPProcIspChiSession::CreateSessions()
{
    CameraResult rc = CAMERA_SUCCESS;

    m_pChiSession = ChiSession::Create(
        &m_pChiPipeline, 1, &m_callbacks, this, AisPProcIsp::GetInstance()->GetChiModule());
    if (NULL == m_pChiSession)
    {
        ISP_LOG(ERROR, "Failed to create session.");
        return CAMERA_EFAILED;
    }
    ISP_LOG(DBG,
        "Successfully created session = %p.", m_pChiSession);

    // Activate pipeline
    //
    rc = m_pChiPipeline->ActivatePipeline(m_pChiSession);
    if (rc != CAMERA_SUCCESS)
    {
        ISP_LOG(ERROR,
            "Failed to activate pipeline = %p for session = %p.",
            m_pChiPipeline, m_pChiSession);
        return CAMERA_EFAILED;
    }
    ISP_LOG(DBG,
        "Successfully activated pipeline = %p for session = %p.",
        m_pChiPipeline, m_pChiSession);

    return rc;
}

void AisPProcIspChiSession::ChiProcNotify(
    const CHIMESSAGEDESCRIPTOR* pMessageDescriptor,
    VOID*                        pPrivateCallbackData)
{
    CAM_UNUSED(pPrivateCallbackData);
    ISP_LOG(DBG, "Received notify from driver.");

    switch(pMessageDescriptor->messageType)
    {
    case ChiMessageTypeError:
    {
        switch (pMessageDescriptor->message.errorMessage.errorMessageCode)
        {
        case MessageCodeDevice:
            ISP_LOG(ERROR,
                "Notify ERROR [DEVICE]: serious failure occurred and no"
                " further frames will be produced by the device");
            break;
        case MessageCodeRequest:
            ISP_LOG(ERROR,
                "Notify ERROR [REQUEST]: error has occurred in processing "
                "a request and no output will be produced for this request");
            break;
        case MessageCodeResult:
            ISP_LOG(ERROR,
                "Notify ERROR [RESULT]: error has occurred in producing "
                "an output result metadata buffer for a request");
            break;
        case MessageCodeBuffer:
            ISP_LOG(ERROR,
                "Notify ERROR [BUFFER]: error has occurred in placing "
                "an output buffer into a stream for a request");
            break;
        case MessageCodeTriggerRecovery:
            ISP_LOG(ERROR,
                "Notify ERROR [RECOVERY]: error has occurred and we need to trigger recovery");
            break;
        default:
            ISP_LOG(ERROR,
                "Unsupported error code = %d.",
                pMessageDescriptor->message.errorMessage.errorMessageCode);
            break;
        }
        break;
    }
    case ChiMessageTypeSof:
    {
        const CHISOFMESSAGE& sofMessage = pMessageDescriptor->message.sofMessage;
        // SOF notifications are not sent to the HAL3 application
        ISP_LOG(DBG,
            "Notify SOF frameNum %u framework frameNum %u, timestamp %ull",
            sofMessage.sofId,
            sofMessage.frameworkFrameNum,
            sofMessage.timestamp);
        break;
    }
    case ChiMessageTypeShutter:
    {
        const CHISHUTTERMESSAGE& shutterMessage = pMessageDescriptor->message.shutterMessage;

        ISP_LOG(DBG,
            "Notify Shutter framework frameNum %u, timestamp %llu",
            shutterMessage.frameworkFrameNum,
            shutterMessage.timestamp);
        break;
    }
    case ChiMessageTypeMetaBufferDone:
    {
        const CHIMETABUFFERDONEMESSAGE metaBufferDoneMessage = pMessageDescriptor->message.metaBufferDoneMessage;
        ISP_LOG(DBG,
            "Notify Meta buffer done frameNum %u", metaBufferDoneMessage.frameworkFrameNum);
        break;
    }
    case ChiMessageTypeTriggerRecovery:
    {
        ISP_LOG(DBG, "Notify message trigger recovery");
        break;
    }
    default:
    {
        ISP_LOG(ERROR,
            "Unsupported message type = %d.", pMessageDescriptor->messageType);
        break;
    }
    }
}

void AisPProcIspChiSession::ChiProcPartialCaptureResult(
     CHIPARTIALCAPTURERESULT* pCaptureResult,
     VOID*                      pPrivateCallbackData)
{
    CAM_UNUSED(pCaptureResult);
    CAM_UNUSED(pPrivateCallbackData);
}

void AisPProcIspChiSession::ChiProcCaptureResult(
    CHICAPTURERESULT*            pCaptureResult,
    VOID*                        pPrivateCallbackData)
{
    if ((NULL == pCaptureResult) || (NULL == pPrivateCallbackData))
    {
        ISP_LOG(ERROR,
            "Invalid input params: pCaptureResult=%p, pPrivateCallbackData=%p.",
            pCaptureResult, pPrivateCallbackData);
        return;
    }

    ISP_LOG(DBG,
        "Received capture result: pCaptureResult=%p, pPrivateCallbackData=%p. Enqueue it for processing...",
        pCaptureResult, pPrivateCallbackData);

    AisPProcIspChiSession* pNode = reinterpret_cast<AisPProcIspChiSession*>(pPrivateCallbackData);
    if (pNode != NULL)
    {
        CHICAPTURERESULT* pSavedResult = pNode->DeepCopyCaptureResult(pCaptureResult);
        if (pSavedResult != NULL)
        {
            CameraResult rc = AisPProcIsp::GetInstance()->EnqCaptureResult(pNode, pSavedResult);
            if (CAMERA_SUCCESS != rc)
            {
                pNode->DeepDestroyCaptureResult(pSavedResult);
            }
        }
    }
}

// This method processes all queued capture results.
//
// returns: CameraResult indicating success or failure of this method.
//
CameraResult AisPProcIspChiSession::ProcCaptureResult(CHICAPTURERESULT*        pCaptureResult)
{
    bool procBuf = false;
    ProcFrameResult(pCaptureResult, &procBuf);
    CameraLockMutex(m_pFrame2NumProcBufsMapMutex);
    if (procBuf)
    {
        m_Frame2NumProcBufsMap[pCaptureResult->frameworkFrameNum] -= pCaptureResult->numOutputBuffers;
        ISP_LOG(DBG,
            "Received capture result and output buffers, frameNum=%lu, buffersToProcess=%d",
            pCaptureResult->frameworkFrameNum, m_Frame2NumProcBufsMap[pCaptureResult->frameworkFrameNum]);
        if (0 == m_Frame2NumProcBufsMap[pCaptureResult->frameworkFrameNum])
        {
            m_Frame2NumProcBufsMap.erase(pCaptureResult->frameworkFrameNum);
        }
    }
    else
    {
        ISP_LOG(DBG,
            "Received capture result but it didn't have buffers to process, frameNum=%lu, buffersToProcess=%d",
            pCaptureResult->frameworkFrameNum, m_Frame2NumProcBufsMap[pCaptureResult->frameworkFrameNum]);
        if (0 == m_Frame2NumProcBufsMap[pCaptureResult->frameworkFrameNum])
        {
            m_Frame2NumProcBufsMap.erase(pCaptureResult->frameworkFrameNum);
        }
    }

    if (m_Frame2NumProcBufsMap.empty())
    {
        ISP_LOG(DBG,
            "Signal main thread that request processing completed");
        CameraSetSignal(m_pReqsCompleteSig);
    }

    CameraUnlockMutex(m_pFrame2NumProcBufsMapMutex);

    DeepDestroyCaptureResult(pCaptureResult);

    return CAMERA_SUCCESS;

}

void AisPProcIspChiSession::ProcFrameResult(
    CHICAPTURERESULT*            pCaptureResult,
    bool*                        pFrameProcessed)
{
    CameraResult rc = CAMERA_SUCCESS;

    *pFrameProcessed = false;

    CaptureResultPrint(pCaptureResult);

    if (pCaptureResult->numOutputBuffers > 0)
    {
        for (uint32_t i = 0; i < pCaptureResult->numOutputBuffers; i++)
        {
            CHISTREAMBUFFER* pChiBuf = (CHISTREAMBUFFER*)(&(pCaptureResult->pOutputBuffers[i]));
            if (pChiBuf != NULL)
            {
                for (uint32 j = 0; j < QCARCAM_MAX_NUM_BUFFERS; j++)
                {
                    if (m_OutputBufferMap[j].pChiBuffer != NULL && m_OutputBufferMap[j].pChiBuffer->bufferInfo.phBuffer == pChiBuf->bufferInfo.phBuffer)
                    {
                        //@TODO: ensure buffer idx same as job that is done

                        *pFrameProcessed = true;

                        FramePPorcJobMap::iterator itr = m_pFrame2pAisEventPProcJobType.find(pCaptureResult->frameworkFrameNum);
                        if (itr != m_pFrame2pAisEventPProcJobType.end())
                        {
                            AisEventMsgType msg = {};
                            AisEventPProcJobType* pIspJob = itr->second;
                            AisBufferList* pInBufferList = pIspJob->pUsrCtxt->m_bufferList[pIspJob->pProcChain->inBuflistId[0]];
                            AisBufferList* pOutBufferList = pIspJob->pUsrCtxt->m_bufferList[pIspJob->pProcChain->outBuflistId[0]];
                            AisBufferList* pJpegBufferList = pIspJob->pUsrCtxt->m_bufferList[pIspJob->pProcChain->outBuflistId[1]];
                            AIS_LOG(PPROC_ISP, DBG, "InputId %d ISP job done 0x%x (rc = %d), FrameNum %d, seq_no %d, bufInIdx %d, bufOutIdx %d",
                                pIspJob->pUsrCtxt->m_inputId, pIspJob->jobId, pIspJob->status, pCaptureResult->frameworkFrameNum, pIspJob->frameInfo.seq_no[0],
                                pIspJob->bufInIdx[0], pIspJob->bufOutIdx[0]);

                            pIspJob->status = rc;
                            pInBufferList->SetBufferState(pIspJob->bufInIdx[0], AIS_BUFFER_INITIALIZED);
                            pInBufferList->ReturnBuffer(pIspJob->pUsrCtxt, pIspJob->bufInIdx[0]);

                            //Dump buffers if need
                            if (pIspJob->bDumpBuffers)
                            {
                                pInBufferList->DumpBuffer(pIspJob->bufInIdx[0], pIspJob->frameInfo.seq_no[0]);
                                pOutBufferList->DumpBuffer(pIspJob->bufOutIdx[0], pIspJob->frameInfo.seq_no[0]);
#ifdef AIS_ISP_ENABLE_JPEG
                                if (pJpegBufferList)
                                {
                                    pJpegBufferList->DumpBuffer(pIspJob->bufOutIdx[1], pIspJob->frameInfo.seq_no[0]);
                                }
#endif
                            }

                            if (pJpegBufferList)
                            {
                                pJpegBufferList->SetBufferState(pIspJob->bufOutIdx[1], AIS_BUFFER_INITIALIZED);
                                pJpegBufferList->ReturnBuffer(pIspJob->pUsrCtxt, pIspJob->bufOutIdx[1]);
                            }

                            CameraLockMutex(m_pFrame2pAisEventPProcJobTypeMutex);
                            m_pFrame2pAisEventPProcJobType.erase(pCaptureResult->frameworkFrameNum);
                            CameraUnlockMutex(m_pFrame2pAisEventPProcJobTypeMutex);

                            ISP_LOG(MED, "Job done (0x%llx rc:%d), %p",
                                    pIspJob->jobId, pIspJob->status, pIspJob);

                            pOutBufferList->QueueReadyBuffer(pIspJob->jobId, pOutBufferList->GetBuffer(pIspJob->bufOutIdx[0]));

                            msg.eventId = AIS_EVENT_PPROC_JOB_DONE;
                            memcpy(&msg.payload.pprocJob, pIspJob, sizeof(AisEventPProcJobType));
                            AisEngine::GetInstance()->QueueEvent(&msg);
                            CameraFree(CAMERA_ALLOCATE_ID_ENGINE_EVENT, pIspJob);
                        }
                        else
                        {
                            ISP_LOG(ERROR, "Unmapped FrameNum %d", pCaptureResult->frameworkFrameNum);
                        }

                        break;
                    }
                }
            }
        }
    }

    if (*pFrameProcessed && m_pChiPipeline->PipelineHasPopulatedTag(m_aectagId) && m_pUsrCtxt->m_isMaster)
    {
        if (m_MetadataOps.pGetVendorTagEntry(
            m_pOutputMetabufPool[pCaptureResult->frameworkFrameNum % m_OutputMetabufPoolSize], /* CHIMETAHANDLE */
            STATS_CONFIG_TAG_SECTION, /* pTagSectionName */
            AEC_FRAME_CTL_TAG_NAME, /* pTagName */
            &m_metadataEntry /* CHIMETADATAENTRY */) == CDKResultSuccess)
        {
            AECFrameControl*  paecframecontrol = (AECFrameControl*)m_metadataEntry.pTagData;
            uint32 j;

            for (j = 0; j < ExposureIndexCount; j++)
            {
                if (0 == paecframecontrol->exposureInfo[j].exposureTime || 0.0f == paecframecontrol->exposureInfo[j].linearGain)
                {
                    ISP_LOG(ERROR, "invalid exposure control exposure time: %llu, linearGain: %f",
                                paecframecontrol->exposureInfo[j].exposureTime,
                                paecframecontrol->exposureInfo[j].linearGain);
                    break;
                }
            }

            if (j == ExposureIndexCount)
            {
                AisEventMsgType msg = {};

                rc = m_pUsrCtxt->IncRefCnt();

                if (rc != CAMERA_SUCCESS)
                {
                    ISP_LOG(WARN, "pUsrCtxt %p may be closed", m_pUsrCtxt);
                }
                else if (UsecaseShdrBpsIpeAECAWB == m_usecaseId)
                {
                    msg.eventId = AIS_EVENT_APPLY_PARAM;
                    msg.payload.applyParam.pUsrCtxt = m_pUsrCtxt;
                    msg.payload.applyParam.param = QCARCAM_PARAM_HDR_EXPOSURE;
                    msg.payload.applyParam.val.hdr_exposure_config.exposure_mode_type = QCARCAM_EXPOSURE_AUTO;
                    msg.payload.applyParam.val.hdr_exposure_config.num_exposures = ExposureIndexCount;

                    /* To create HDR image sensor can use up to 4 exposures. T1 exposure is a longest and
                    * each successive exposure being shorter (T1 > T2 > T3 > T4). The indexes in exposure_time
                    * array 0,1,2,3 correspond to T1, T2, T3, T4 of the sensor. AEC indexes of ExposureIndexLong,
                    * ExposureIndexSafe and ExposureIndexShort should be used to program T1, T2, T3 and T4.
                    */
                    msg.payload.applyParam.val.hdr_exposure_config.exposure_time[0] = (float)paecframecontrol->exposureInfo[ExposureIndexLong].exposureTime / 1000000;
                    msg.payload.applyParam.val.hdr_exposure_config.gain[0] = paecframecontrol->exposureInfo[ExposureIndexLong].linearGain;
                    msg.payload.applyParam.val.hdr_exposure_config.exposure_time[1] = (float)paecframecontrol->exposureInfo[ExposureIndexSafe].exposureTime / 1000000;
                    msg.payload.applyParam.val.hdr_exposure_config.gain[1] = paecframecontrol->exposureInfo[ExposureIndexSafe].linearGain;
                    msg.payload.applyParam.val.hdr_exposure_config.exposure_time[2] = (float)paecframecontrol->exposureInfo[ExposureIndexShort].exposureTime / 1000000;
                    msg.payload.applyParam.val.hdr_exposure_config.gain[2] = paecframecontrol->exposureInfo[ExposureIndexShort].linearGain;
                    ISP_LOG(DBG,"Set exposure: exposure_mode_type %d exposure_time (%f,%f,%f) gain (%f,%f,%f)",
                        msg.payload.applyParam.val.hdr_exposure_config.exposure_mode_type,
                        msg.payload.applyParam.val.hdr_exposure_config.exposure_time[0],
                        msg.payload.applyParam.val.hdr_exposure_config.exposure_time[1],
                        msg.payload.applyParam.val.hdr_exposure_config.exposure_time[2],
                        msg.payload.applyParam.val.hdr_exposure_config.gain[0],
                        msg.payload.applyParam.val.hdr_exposure_config.gain[1],
                        msg.payload.applyParam.val.hdr_exposure_config.gain[2]);
                }
                else if (UsecaseBPSAECAWBIPEInputMipiRaw16OutNV12 == m_usecaseId)
                {
                    msg.eventId = AIS_EVENT_APPLY_PARAM;
                    msg.payload.applyParam.pUsrCtxt = m_pUsrCtxt;
                    msg.payload.applyParam.param = QCARCAM_PARAM_EXPOSURE;
                    msg.payload.applyParam.val.exposure_config.exposure_mode_type = QCARCAM_EXPOSURE_AUTO;
                    msg.payload.applyParam.val.exposure_config.exposure_time = (float)paecframecontrol->exposureInfo[ExposureIndexSafe].exposureTime / 1000000;
                    msg.payload.applyParam.val.exposure_config.gain = paecframecontrol->exposureInfo[ExposureIndexSafe].linearGain;
                    ISP_LOG(DBG,"Set exposure: exposure_mode_type %d exposure_time %f gain %f",
                        msg.payload.applyParam.val.exposure_config.exposure_mode_type,
                        msg.payload.applyParam.val.exposure_config.exposure_time,
                        msg.payload.applyParam.val.exposure_config.gain);
                }
                else
                {
                    ISP_LOG(ERROR, "unsupported AIS_EVENT_APPLY_PARAM for usecase %d", m_usecaseId);
                    rc = CAMERA_EFAILED;
                }

                if (CAMERA_SUCCESS == rc)
                {
                    AisEngine::GetInstance()->QueueEvent(&msg);
                }
            }
        }
    }
}

CameraResult AisPProcIspChiSession::CreateDefaultMetada(CHIMETAHANDLE* pChiMetaHandle)
{
    CameraResult rc = CAMERA_SUCCESS;
    CHIMETAHANDLE  h_AndroidDefaultMetaHandle;

    if (NULL == pChiMetaHandle)
    {
        ISP_LOG(ERROR,
            "Invalid input params, pChiMetaHandle=%p.", pChiMetaHandle);
        return CAMERA_EBADPARM;
    }

    if (m_MetadataOps.pGetDefaultAndroidMeta(0, (const VOID**)&h_AndroidDefaultMetaHandle) != CDKResultSuccess)
    {
        ISP_LOG(ERROR, "Failed to get default Android metadata");
        return CAMERA_EFAILED;
    }

    ISP_LOG(DBG,
            "Succeeded to get default Android metadata handle, pChiMetaHandle=%p", h_AndroidDefaultMetaHandle);
    if (m_MetadataOps.pCreateWithAndroidMetadata(h_AndroidDefaultMetaHandle, pChiMetaHandle, NULL))
    {
        ISP_LOG(ERROR, "Failed to create android metadata");
        return CAMERA_EFAILED;
    }
    else
        ISP_LOG(DBG, "Successfully created android metadata");

    if (m_MetadataOps.pSetAndroidMetadata(*pChiMetaHandle, h_AndroidDefaultMetaHandle))
    {
        ISP_LOG(ERROR, "Failed to set default Android android metadata");
        return CAMERA_EFAILED;
    }
    else
        ISP_LOG(DBG, "Successfully set default Android metadata");

    ChiTuningModeParameter tagValue;
    memset(&tagValue, 0, sizeof(tagValue));
    if (m_MetadataOps.pSetVendorTag(
                *pChiMetaHandle,
                "org.quic.camera2.tuning.mode",
                "TuningMode",
                &tagValue,
                sizeof(ChiTuningModeParameter)) != CDKResultSuccess)
    {
        ISP_LOG(ERROR, "Failed to set vendor tag");
        return CAMERA_EFAILED;
    }

    return rc;
}

CameraResult AisPProcIspChiSession::CreateInputMetabufPool(int poolSize)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (NULL == m_pInputMetabufPool)
    {
        m_pInputMetabufPool = new CHIMETAHANDLE[poolSize];
        if(NULL == m_pInputMetabufPool)
        {
            ISP_LOG(ERROR,
                "Failed to allocate input metadata pool");
            return CAMERA_ENOMEMORY;
        }
        ISP_LOG(DBG,
            "Successfully allocated input metadata pool, m_pInputMetabufPool=%p",
            m_pInputMetabufPool);
        m_InputMetabufPoolSize = poolSize;

        for (int index = 0; index < m_InputMetabufPoolSize; index++)
        {
            rc = CreateDefaultMetada(&(m_pInputMetabufPool[index]));
            if (rc != CAMERA_SUCCESS)
            {
                ISP_LOG(ERROR,
                    "Failed to allocate metadata buffer for idex=%d", index);
                return CAMERA_EFAILED;
            }
        }
    }
    return rc;
}

CameraResult AisPProcIspChiSession::CreateOutputMetabufPool(int poolSize)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (NULL == m_pOutputMetabufPool)
    {
        m_pOutputMetabufPool = new CHIMETAHANDLE[poolSize];
        if(NULL == m_pOutputMetabufPool)
        {
            ISP_LOG(ERROR,
                "Failed to allocate output metadata pool");
            return CAMERA_ENOMEMORY;
        }
        ISP_LOG(DBG,
            "Successfully allocated output metadata pool, m_pOutputMetabufPool=%p",
            m_pOutputMetabufPool);
        m_OutputMetabufPoolSize = poolSize;

        for (int index = 0; index < m_OutputMetabufPoolSize; index++)
        {
            rc = CreateDefaultMetada(&(m_pOutputMetabufPool[index]));
            if (rc != CAMERA_SUCCESS)
            {
                ISP_LOG(ERROR,
                    "Failed to allocate metadata buffer for idex=%d", index);
                return CAMERA_EFAILED;
            }
        }
    }
    return rc;
}

CameraResult AisPProcIspChiSession::DestroyMetabufPools()
{
    CameraResult rc = CAMERA_SUCCESS;

    if (m_pInputMetabufPool)
    {
        for (int index = 0; index < m_InputMetabufPoolSize; index++)
        {
            if (m_MetadataOps.pDestroy(
                    m_pInputMetabufPool[index], true) != CDKResultSuccess)
            {
                ISP_LOG(ERROR,
                    "Failed to destroy metadata buffer, pChiMetaHandle=%p",
                    m_pInputMetabufPool[index]);
                return CAMERA_EFAILED;
            }
            ISP_LOG(DBG,
                "Successfully destroyed input metadata buffer, pChiMetaHandle=%p",
                m_pInputMetabufPool[index]);
        }
        delete[] m_pInputMetabufPool;
        m_pInputMetabufPool = NULL;
    }

    if (m_pOutputMetabufPool)
    {
        for (int index = 0; index < m_OutputMetabufPoolSize; index++)
        {
            if (m_MetadataOps.pDestroy(
                    m_pOutputMetabufPool[index], true) != CDKResultSuccess)
            {
                ISP_LOG(ERROR,
                    "Failed to destroy metadata buffer, pChiMetaHandle=%p",
                    m_pOutputMetabufPool[index]);
                return CAMERA_EFAILED;
            }
            ISP_LOG(DBG,
                "Successfully destroyed output metadata buffer, pChiMetaHandle=%p",
                m_pOutputMetabufPool[index]);
        }
        delete[] m_pOutputMetabufPool;
        m_pOutputMetabufPool = NULL;
    }

    return rc;
}

CHICAPTURERESULT* AisPProcIspChiSession::DeepCopyCaptureResult(CHICAPTURERESULT* pCaptureResult)
{
    CameraResult rc = CAMERA_SUCCESS;

    CHICAPTURERESULT* pCopiedResult = new CHICAPTURERESULT();
    if (NULL == pCopiedResult)
    {
        ISP_LOG(ERROR,
            "Failed to alloc buffer for result copy.");
        rc = CAMERA_ENOMEMORY;
        goto exit;
    }
    memcpy(pCopiedResult, pCaptureResult, sizeof(CHICAPTURERESULT)); // shallow copy of result buffer

    pCopiedResult->pOutputBuffers = new CHISTREAMBUFFER[pCaptureResult->numOutputBuffers];
    pCopiedResult->pInputBuffer = new CHISTREAMBUFFER();
    pCopiedResult->pPrivData = new CHIPRIVDATA();
    if ((NULL == pCopiedResult->pOutputBuffers) ||
        (NULL == pCopiedResult->pInputBuffer) ||
        (NULL == pCopiedResult->pPrivData))
    {
        ISP_LOG(ERROR,
            "Failed to alloc buffer for deep copy.");
        rc = CAMERA_ENOMEMORY;
        goto exit;
    }

    if (pCaptureResult->pOutputBuffers)
    {
        memcpy((void*)(pCopiedResult->pOutputBuffers), pCaptureResult->pOutputBuffers,
            sizeof(CHISTREAMBUFFER) * pCaptureResult->numOutputBuffers);
    }
    if (pCaptureResult->pInputBuffer != NULL)
    {
        memcpy((void*)(pCopiedResult->pInputBuffer), pCaptureResult->pInputBuffer, sizeof(CHISTREAMBUFFER));
    }
    if (pCaptureResult->pPrivData != NULL)
    {
        memcpy(pCopiedResult->pPrivData, pCaptureResult->pPrivData, sizeof(CHIPRIVDATA));
    }
    pCopiedResult->pInputMetadata = pCaptureResult->pInputMetadata;
    pCopiedResult->pOutputMetadata = pCaptureResult->pOutputMetadata;
exit:
    if ((rc != CAMERA_SUCCESS) && pCopiedResult)
    {
        if (pCopiedResult->pOutputBuffers)
            delete [] pCopiedResult->pOutputBuffers;
        if (pCopiedResult->pInputBuffer)
            delete pCopiedResult->pInputBuffer;
        if (pCopiedResult->pPrivData)
            delete pCopiedResult->pPrivData;
        if (pCopiedResult)
            delete pCopiedResult;
        pCopiedResult = NULL;
    }
    return pCopiedResult;
}

void AisPProcIspChiSession::DeepDestroyCaptureResult(CHICAPTURERESULT* pCaptureResult)
{
    if (pCaptureResult != NULL)
    {
        if (pCaptureResult->pOutputBuffers != NULL)
        {
            delete [] pCaptureResult->pOutputBuffers;
        }
        if (pCaptureResult->pInputBuffer)
        {
            delete pCaptureResult->pInputBuffer;
        }
        if (pCaptureResult->pPrivData)
        {
            delete pCaptureResult->pPrivData;
        }
        delete pCaptureResult;
    }
}

CameraResult AisPProcIspChiSession::GenerateOfflineCaptureRequest(AisEventPProcJobType* pIspJob)
{
    CameraResult rc = CAMERA_SUCCESS;

    uint32_t frameNum = pIspJob->frameInfo.seq_no[0];
    uint32_t requestId = frameNum % maxRequests;
    static const uint32_t NumOutputBuffers = m_numStreams - 1; // not considering the input stream
#ifdef AIS_ISP_ENABLE_JPEG
    CHISTREAMBUFFER outbuf[2];
#endif

    m_OfflineRequests[requestId].frameNumber        = frameNum;
    m_OfflineRequests[requestId].hPipelineHandle    = m_pChiPipeline->GetPipelineHandle();
    m_OfflineRequests[requestId].numInputs          = 1;
    m_OfflineRequests[requestId].numOutputs         = NumOutputBuffers;
    m_OfflineRequests[requestId].pInputBuffers      = m_InputBufferMap[pIspJob->bufInIdx[0]].pChiBuffer;

#ifdef AIS_ISP_ENABLE_JPEG
    if (m_JpegOutputBufferMap[0].pChiBuffer != NULL && pIspJob->bDumpBuffers)
    {
        memcpy(&outbuf[0], m_OutputBufferMap[pIspJob->bufOutIdx[0]].pChiBuffer, sizeof(CHISTREAMBUFFER));
        memcpy(&outbuf[1], m_JpegOutputBufferMap[pIspJob->bufOutIdx[1]].pChiBuffer, sizeof(CHISTREAMBUFFER));
        m_OfflineRequests[requestId].pOutputBuffers     = outbuf;
    }
    else
#endif
    {
        m_OfflineRequests[requestId].pOutputBuffers     = m_OutputBufferMap[pIspJob->bufOutIdx[0]].pChiBuffer;
        m_OfflineRequests[requestId].numOutputs  = 1;
    }

    m_OfflineRequests[requestId].pInputMetadata     = m_pInputMetabufPool[(requestId % m_InputMetabufPoolSize)];
    m_OfflineRequests[requestId].pOutputMetadata    = m_pOutputMetabufPool[(requestId % m_OutputMetabufPoolSize)];
    m_OfflineRequests[requestId].pPrivData          = &m_requestPvtData;

    CameraLockMutex(m_ApplySettingsMutex);
    if (m_ApplySettingsReqCnt < maxRequests)
    {
        m_ApplySettingsReqCnt++;
        SetAndroidTags(m_OfflineRequests[requestId].pInputMetadata, &m_BayerCtrls);
        SetVendorTags(m_OfflineRequests[requestId].pInputMetadata, &m_BayerCtrls);
    }
    // Store frame number for querying pipeline settings
    m_CurrentFrameNum = frameNum;
    CameraUnlockMutex(m_ApplySettingsMutex);
    m_MetadataOps.pSetVendorTag(m_OfflineRequests[requestId].pInputMetadata,"org.quic.camera.tuningdata","SHDRDumpRaw" ,&pIspJob->bDumpBuffers, 1);

#if 0
    if ((UsecaseBPSIPEInputMipiRaw16OutNV12 == m_usecaseId) ||
        (UsecaseBPSAECAWBIPEInputMipiRaw16OutNV12 == m_usecaseId))
    {
        rc = SetupBpsIqSettings(m_OfflineRequests[requestId].pInputMetadata, &(m_IspIqSettings[m_usecaseId].oemBpsIqSettings));
        if (rc == CAMERA_SUCCESS)
        {
            rc = SetupIpeIqSettings(m_OfflineRequests[requestId].pInputMetadata, &(m_IspIqSettings[m_usecaseId].oemIpeIqSettings));
        }
    }
    if ((UsecaseIPEInputNV12OutUBWCNV12 == m_usecaseId) ||
        (UsecaseIPEInputNV12OutNV12 == m_usecaseId))
    {
         rc = SetupIpeIqSettings(m_OfflineRequests[requestId].pInputMetadata, &(m_IspIqSettings[m_usecaseId].oemIpeIqSettings));
    }
    else
    {
         rc = SetupBpsIqSettings(m_OfflineRequests[requestId].pInputMetadata, &(m_IspIqSettings[m_usecaseId].oemBpsIqSettings));
    }

    if (rc != CAMERA_SUCCESS)
    {
        ISP_LOG(ERROR,
            "Failed set IQ settings in request metadata");
        goto exit;
    }
    ISP_LOG(DBG,
        "Successfully set IQ settings in request metadata");
#endif

    if (UsecaseBPSInputMipiRaw10OutFullP010Stats == m_usecaseId ||
        UsecaseBPSInputRaw16OutNV12Stats == m_usecaseId ||
        UsecaseBpsAWBBGStats == m_usecaseId ||
        UsecaseBPSBGBHISTAWB == m_usecaseId ||
        UsecaseShdrBpsAECAWB == m_usecaseId ||
        UsecaseBPSAECAWB == m_usecaseId ||
        UsecaseBPSAECAWBIPEInputMipiRaw16OutNV12 == m_usecaseId ||
        UsecaseShdrBpsIpeAECAWB == m_usecaseId)
    {
        rc = SetupBpsStatsControl(m_OfflineRequests[requestId].pInputMetadata);
        if(rc)
        {
            ISP_LOG(ERROR,
                "Failed set BG stats vendor tag");
            goto exit;
        }
    }

    memset(&m_submitRequest, 0, sizeof(m_submitRequest));
    m_submitRequest.pSessionHandle = m_pChiSession->GetSessionHandle();
    m_submitRequest.numRequests = 1;
    m_submitRequest.pCaptureRequests = &(m_OfflineRequests[requestId]);

    CameraLockMutex(m_pFrame2NumProcBufsMapMutex);
    m_Frame2NumProcBufsMap[frameNum] = NumOutputBuffers;
    CameraUnlockMutex(m_pFrame2NumProcBufsMapMutex);

    ISP_LOG(DBG,
        "Sending pipeline request for frame: %d.", frameNum);
    if (AisPProcIsp::GetInstance()->SubmitPipelineRequest(&m_submitRequest) != CAMERA_SUCCESS)
    {
        ISP_LOG(ERROR,
            "Failed to send pipeline request for frame: %d.", frameNum);
        rc = CAMERA_EFAILED;
        goto exit;
    }

    // Query and print pipeline metadata tags
    m_pChiPipeline->QueryPipelineMetadataInfo(m_pChiSession);

    for (uint32_t reqNum = 0; reqNum < m_submitRequest.numRequests; reqNum++)
    {
        CaptureRequestPrint(&(m_submitRequest.pCaptureRequests[reqNum]));
    }

exit:
    return rc;
}

CameraResult AisPProcIspChiSession::WaitForResults(uint32_t timeoutMs)
{
    CameraResult rc = CAMERA_SUCCESS;
    rc = CameraWaitOnSignal(m_pReqsCompleteSig, timeoutMs);
    if (rc != CAMERA_SUCCESS)
    {
        ISP_LOG(ERROR,
            "Failed waiting for completion of capture results processing");
    }
    else
    {
        ISP_LOG(DBG,
            "Received signal that results processing completed");
    }
    return rc;
}

CameraResult AisPProcIspChiSession::SetupBpsIqSettings(CHIMETADATAHANDLE              pMetadata,
                                                    const OEMBPSIQSetting*    pBpsIqSettings)
{
    if (m_MetadataOps.pSetVendorTag(
            pMetadata,
            IQ_TAG_SECTION,
            BPS_TAG_NAME,
            (void*)(pBpsIqSettings),
            sizeof(OEMBPSIQSetting)) != CDKResultSuccess)
    {
        ISP_LOG(ERROR,
            "Failed to set vendor tag=%s in section=%s", BPS_TAG_NAME, IQ_TAG_SECTION);
        return CAMERA_EFAILED;
    }
    ISP_LOG(DBG,
        "Successfully set vendor tag=%s in section=%s", BPS_TAG_NAME, IQ_TAG_SECTION);

    return CAMERA_SUCCESS;
}

CameraResult AisPProcIspChiSession::SetupIpeIqSettings(CHIMETADATAHANDLE              pMetadata,
                                                    const OEMIPEIQSetting*    pIpeIqSettings)
{
    if (m_MetadataOps.pSetVendorTag(
            pMetadata,
            IQ_TAG_SECTION,
            IPE_TAG_NAME,
            (void*)(pIpeIqSettings),
            sizeof(OEMIPEIQSetting)) != CDKResultSuccess)
    {
        ISP_LOG(ERROR,
            "Failed to set vendor tag=%s in section=%s", IPE_TAG_NAME, IQ_TAG_SECTION);
        return CAMERA_EFAILED;
    }
    ISP_LOG(DBG,
        "Successfully set vendor tag=%s in section=%s", IPE_TAG_NAME, IQ_TAG_SECTION);

    return CAMERA_SUCCESS;
}

CameraResult AisPProcIspChiSession::SetupBpsStatsControl(CHIMETADATAHANDLE            pMetadata)
{
    AWBStatsControl StatsCntl;
    BGBEConfig*     pBGConfig = &StatsCntl.statsConfig.BGConfig;

    if (NULL != pBGConfig)
    {
        pBGConfig->channelGainThreshold[ChannelIndexR]    = (1 << 14) - 1;
        pBGConfig->channelGainThreshold[ChannelIndexGR] = (1 << 14) - 1; //BPSPipelineBitWidth =14
        pBGConfig->channelGainThreshold[ChannelIndexB]    = (1 << 14) - 1;
        pBGConfig->channelGainThreshold[ChannelIndexGB] = (1 << 14) - 1;
        pBGConfig->horizontalNum                        = BG_REGION_NUM_H;
        pBGConfig->verticalNum                            = BG_REGION_NUM_V;
        pBGConfig->ROI.left                             = 0;
        pBGConfig->ROI.top                                = 0;


        pBGConfig->ROI.width = m_pRequiredStreams[0].width;
        pBGConfig->ROI.height = m_pRequiredStreams[0].height;

        pBGConfig->outputBitDepth                        = 0;
        pBGConfig->outputMode                            = BGBERegular;
        pBGConfig->YStatsWeights[0]                     = static_cast<FLOAT>(0.2f);
        pBGConfig->YStatsWeights[0]                     = static_cast<FLOAT>(0.3);
        pBGConfig->YStatsWeights[0]                     = static_cast<FLOAT>(0.4);
        pBGConfig->greenType                            = Gr;
    }

    if (m_MetadataOps.pSetVendorTag(
            pMetadata,
            STATS_CONFIG_TAG_SECTION,
            AWB_STATS_CTL_TAG_NAME,
            (void*)(&StatsCntl),
            sizeof(AWBStatsControl)) != CDKResultSuccess)
    {
        ISP_LOG(ERROR,
            "Failed to set vendor tag=%s in section=%s", AWB_STATS_CTL_TAG_NAME, STATS_CONFIG_TAG_SECTION);
        return CAMERA_EFAILED;
    }

    ISP_LOG(DBG,
        "Successfully set vendor tag=%s in section=%s", AWB_STATS_CTL_TAG_NAME, STATS_CONFIG_TAG_SECTION);

    return CAMERA_SUCCESS;
}

void AisPProcIspChiSession::CaptureRequestPrint(const CHICAPTUREREQUEST* pCaptureRequest)
{
    if (pCaptureRequest)
    {
        ISP_LOG(DBG,
            "pCaptureRequest=%p: frameNumber=%llu, hPipelineHandle=%p, numInputs=%d, pInputBuffers=%p, "
            "numOutputs=%d, pOutputBuffers=%p, pInputMetadata=%p, pOutputMetadata=%p, pPrivData=%p",
            pCaptureRequest, pCaptureRequest->frameNumber, pCaptureRequest->hPipelineHandle,
            pCaptureRequest->numInputs, pCaptureRequest->pInputBuffers, pCaptureRequest->numOutputs,
            pCaptureRequest->pOutputBuffers, pCaptureRequest->pInputMetadata,
            pCaptureRequest->pOutputMetadata, pCaptureRequest->pPrivData);
        for (uint32_t inputNum = 0; inputNum < pCaptureRequest->numInputs; inputNum++)
        {
            if (pCaptureRequest->pInputBuffers)
            {
                ISP_LOG(DBG,
                    "pCaptureRequest=%p: inputNum=%d, pChiBuffer=%p, pStream=%p, bufferInfo.bufferType=%d, bufferInfo.phBuffer=%p",
                    pCaptureRequest, inputNum, &(pCaptureRequest->pInputBuffers[inputNum]),
                    pCaptureRequest->pInputBuffers[inputNum].pStream,
                    pCaptureRequest->pInputBuffers[inputNum].bufferInfo.bufferType,
                    pCaptureRequest->pInputBuffers[inputNum].bufferInfo.phBuffer);
            }
        }
        for (uint32_t outputNum = 0; outputNum < pCaptureRequest->numOutputs; outputNum++)
        {
            if (pCaptureRequest->pOutputBuffers)
            {
                ISP_LOG(DBG,
                    "pCaptureRequest=%p: outputNum=%d, pChiBuffer=%p, pStream=%p, bufferInfo.bufferType=%d, bufferInfo.phBuffer=%p",
                    pCaptureRequest, outputNum, &(pCaptureRequest->pOutputBuffers[outputNum]),
                    pCaptureRequest->pOutputBuffers[outputNum].pStream,
                    pCaptureRequest->pOutputBuffers[outputNum].bufferInfo.bufferType,
                    pCaptureRequest->pOutputBuffers[outputNum].bufferInfo.phBuffer);
            }
        }
    }

}

void AisPProcIspChiSession::CaptureResultPrint(const CHICAPTURERESULT* pCaptureResult)
{
    if (pCaptureResult)
    {
        ISP_LOG(DBG,
            "pCaptureResult=%p: frameworkFrameNum=%llu, pResultMetadata=%p, pInputBuffer=%p, "
            "numOutputBuffers=%d, pOutputBuffers=%p, numPartialMetadata=%lu, pInputMetadata=%p, "
            "pOutputMetadata=%p, pPrivData=%p",
            pCaptureResult, pCaptureResult->frameworkFrameNum, pCaptureResult->pResultMetadata,
            pCaptureResult->pInputBuffer, pCaptureResult->numOutputBuffers, pCaptureResult->pOutputBuffers,
            pCaptureResult->numPartialMetadata, pCaptureResult->pInputMetadata, pCaptureResult->pOutputMetadata,
            pCaptureResult->pPrivData);

        if (pCaptureResult->pInputBuffer)
        {
            ISP_LOG(DBG,
                "pCaptureResult=%p: inputNum=0, pChiBuffer=%p, pStream=%p, bufferInfo.bufferType=%d, bufferInfo.phBuffer=%p",
                pCaptureResult, pCaptureResult->pInputBuffer,
                pCaptureResult->pInputBuffer->pStream,
                pCaptureResult->pInputBuffer->bufferInfo.bufferType,
                pCaptureResult->pInputBuffer->bufferInfo.phBuffer);
        }

        for (uint32_t outputNum = 0; outputNum < pCaptureResult->numOutputBuffers; outputNum++)
        {
            if (pCaptureResult->pOutputBuffers)
            {
            ISP_LOG(DBG,
                "pCaptureResult=%p: outputNum=%d, pChiBuffer=%p, pStream=%p, bufferInfo.bufferType=%d, bufferInfo.phBuffer=%p",
                pCaptureResult, outputNum, &(pCaptureResult->pOutputBuffers[outputNum]),
                pCaptureResult->pOutputBuffers[outputNum].pStream,
                pCaptureResult->pOutputBuffers[outputNum].bufferInfo.bufferType,
                pCaptureResult->pOutputBuffers[outputNum].bufferInfo.phBuffer);
            }
        }


        if (m_pChiPipeline->PipelineHasPopulatedTag(m_aectagId))
        {
            if (m_MetadataOps.pGetVendorTagEntry(
                        m_pOutputMetabufPool[pCaptureResult->frameworkFrameNum % m_OutputMetabufPoolSize], /* CHIMETAHANDLE */
                        STATS_CONFIG_TAG_SECTION, /* pTagSectionName */
                        AEC_FRAME_CTL_TAG_NAME, /* pTagName */
                        &m_metadataEntry /* CHIMETADATAENTRY */) != CDKResultSuccess)
            {
                ISP_LOG(ERROR,
                        "Failed to get metadata Entry for AECFrameControl for metadata buffer %p",
                        m_pOutputMetabufPool[pCaptureResult->frameworkFrameNum % m_OutputMetabufPoolSize]);
            }
            else
            {
                ISP_LOG(DBG, "AECFrameControl Received");
                ISP_LOG(DBG, "AECFrameControl tag id = %x",m_metadataEntry.tagID);
                ISP_LOG(DBG, "AECFrameControl count = %u",m_metadataEntry.count);
                ISP_LOG(DBG, "AECFrameControl size = %u",m_metadataEntry.size);
                ISP_LOG(DBG, "AECFrameControl type = %u",m_metadataEntry.type);

                AECFrameControl*  paecframecontrol = (AECFrameControl*)m_metadataEntry.pTagData;
                ISP_LOG(DBG, "AECFrameControl luxIndex = %f",paecframecontrol->luxIndex);
                ISP_LOG(DBG, "AECFrameControl predictiveGain = %f",paecframecontrol->predictiveGain);
                for (int i = 0; i < ExposureIndexCount; i++)
                {
                    ISP_LOG(DBG, "AECFrameControl exposureInfo[%d].deltaEVFromTarget = %f",i, paecframecontrol->exposureInfo[i].deltaEVFromTarget);
                    ISP_LOG(DBG, "AECFrameControl exposureInfo[%d].exposureTime = %llu",i, paecframecontrol->exposureInfo[i].exposureTime);
                    ISP_LOG(DBG, "AECFrameControl exposureInfo[%d].sensitivity = %f",i, paecframecontrol->exposureInfo[i].sensitivity);
                }
            }
        }
    }
}

void AisPProcIspChiSession::SetAndroidTags(CHIMETADATAHANDLE pMetadata,
        qcarcam_param_isp_ctrls_t *pBayerCtrls)
{
    UINT32 tagID = 0, tagData = 0, count = 1;

    // AE_MODE
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_AE_MODE))
    {
        tagID = ANDROID_CONTROL_AE_MODE;
        tagData = (camera_metadata_enum_android_control_ae_mode)pBayerCtrls->ae_mode;
        if (m_MetadataOps.pSetTag(pMetadata, tagID, &tagData, count))
            ISP_LOG(ERROR, "Failed to set Android android metadata AE_MODE");
        else
            ISP_LOG(DBG, "Successfully set Android android metadata AE_MODE");
    }
    // AE_LOCK
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_AE_LOCK))
    {
        tagID = ANDROID_CONTROL_AE_LOCK;
        tagData = (camera_metadata_enum_android_control_ae_lock_t)pBayerCtrls->ae_lock;
        if (m_MetadataOps.pSetTag(pMetadata, tagID, &tagData, count))
            ISP_LOG(ERROR, "Failed to set Android android metadata AE_LOCK");
        else
            ISP_LOG(DBG, "Successfully set Android android metadata AE_LOCK");
    }
    // AWB_MODE
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_AWB_MODE))
    {
        tagID = ANDROID_CONTROL_AWB_MODE;
        tagData = (camera_metadata_enum_android_control_awb_mode_t)pBayerCtrls->awb_mode;
        if (m_MetadataOps.pSetTag(pMetadata, tagID, &tagData, count))
            ISP_LOG(ERROR, "Failed to set Android android metadata AWB_MODE");
        else
            ISP_LOG(DBG, "Successfully set Android android metadata AWB_MODE");
    }
    // AWB_LOCK
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_AWB_LOCK))
    {
        tagID = ANDROID_CONTROL_AWB_LOCK;
        tagData = (camera_metadata_enum_android_control_awb_lock_t)pBayerCtrls->awb_lock;
        if (m_MetadataOps.pSetTag(pMetadata, tagID, &tagData, count))
            ISP_LOG(ERROR, "Failed to set Android android metadata AWB_LOCK");
        else
            ISP_LOG(DBG, "Successfully set Android android metadata AWB_LOCK");
    }
    // EFFECT_MODE
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_EFFECT_MODE))
    {
        tagID = ANDROID_CONTROL_EFFECT_MODE;
        tagData = (camera_metadata_enum_android_control_effect_mode_t)pBayerCtrls->effect_mode;
        if (m_MetadataOps.pSetTag(pMetadata, tagID, &tagData, count))
            ISP_LOG(ERROR, "Failed to set Android android metadata EFFECT_MODE");
        else
            ISP_LOG(DBG, "Successfully set Android android metadata EFFECT_MODE");
    }
    // CONTROL_MODE
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_MODE))
    {
        tagID = ANDROID_CONTROL_MODE;
        tagData = (camera_metadata_enum_android_control_mode_t)pBayerCtrls->ctrl_mode;
        if (m_MetadataOps.pSetTag(pMetadata, tagID, &tagData, count))
            ISP_LOG(ERROR, "Failed to set Android android metadata CONTROL_MODE");
        else
            ISP_LOG(DBG, "Successfully set Android android metadata CONTROL_MODE");
    }
    // SCENE_MODE
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_SCENE_MODE))
    {
        tagID = ANDROID_CONTROL_SCENE_MODE;
        tagData = (camera_metadata_enum_android_control_scene_mode_t)pBayerCtrls->scene_mode;
        if (m_MetadataOps.pSetTag(pMetadata, tagID, &tagData, count))
            ISP_LOG(ERROR, "Failed to set Android android metadata SCENE_MODE");
        else
            ISP_LOG(DBG, "Successfully set Android android metadata SCENE_MODE");
    }
    // AE_ANTIBANDING_MODE
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_AE_ANTIBANDING_MODE))
    {
        tagID = ANDROID_CONTROL_AE_ANTIBANDING_MODE;
        tagData = (camera_metadata_enum_android_control_ae_antibanding_mode_t)pBayerCtrls->ae_antibanding_mode;
        if (m_MetadataOps.pSetTag(pMetadata, tagID, &tagData, count))
            ISP_LOG(ERROR, "Failed to set Android android metadata AE_ANTIBANDING_MODE");
        else
            ISP_LOG(DBG, "Successfully set Android android metadata AE_ANTIBANDING_MODE");
    }
    // CONTROL_AE_EXPOSURE_COMPENSATION
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_AE_COMPENSATION))
    {
        tagID = ANDROID_CONTROL_AE_EXPOSURE_COMPENSATION;
        //[-1.0, 1.0] map to [-12, 12]

        // client API ae compensation range [-1.0, 1.0]
        const float minClientAECompensation = -1.0;
        const float maxClientAECompensation = 1.0;

        // Tags ae compensation range [-12, 12]
        const int minAECompensationTableValue = -12;
        const int maxAECompensationTableValue = 12;

        float mapAeCompensation = (pBayerCtrls->ae_compensation - minClientAECompensation)
                                    / (maxClientAECompensation - minClientAECompensation)
                                    * (maxAECompensationTableValue - minAECompensationTableValue)
                                    + minAECompensationTableValue;

        int32_t aeCompensation = round(mapAeCompensation);
        if (m_MetadataOps.pSetTag(pMetadata, tagID, &aeCompensation, count))
            ISP_LOG(ERROR, "Failed to set Android android metadata ANDROID_CONTROL_AE_EXPOSURE_COMPENSATION");
        else
            ISP_LOG(DBG, "Successfully set Android android metadata ANDROID_CONTROL_AE_EXPOSURE_COMPENSATION = %d, %f",
            aeCompensation, pBayerCtrls->ae_compensation);
    }
    // CONTROL_AE_REGIONS
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_AE_REGIONS))
    {
        tagID = ANDROID_CONTROL_AE_REGIONS;
        qcarcam_ctrl_ae_regions_t* aeRegions = (qcarcam_ctrl_ae_regions_t *)&pBayerCtrls->ae_regions;
        int32_t regions[5] = {};
        regions[0] = aeRegions->xMin;
        regions[1] = aeRegions->yMin;
        regions[2] = aeRegions->xMax;
        regions[3] = aeRegions->yMax;
        regions[4] = aeRegions->weight;

        if (m_MetadataOps.pSetTag(pMetadata, tagID, (void **)&regions, 5))
            ISP_LOG(ERROR, "Failed to set Android android metadata ANDROID_CONTROL_AE_REGIONS");
        else
            ISP_LOG(DBG, "Successfully set Android android metadata ANDROID_CONTROL_AE_REGIONS");
    }
}

void AisPProcIspChiSession::GetAndroidTags(CHIMETADATAHANDLE pMetadata,
        qcarcam_param_isp_ctrls_t *pBayerCtrls)
{
    UINT32 tagID = 0;
    void *pData = NULL;
    int *ptagData = NULL;

    // AE_MODE
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_AE_MODE))
    {
        tagID = ANDROID_CONTROL_AE_MODE;
        if (!m_MetadataOps.pGetTag(pMetadata, tagID, &pData))
        {
            ptagData = (int*)pData;
            ISP_LOG(DBG, "Successfully got android metadata AE_MODE = %d", *ptagData);
            pBayerCtrls->ae_mode = (qcarcam_ctrl_ae_mode_t)*ptagData;
        }
        else
            ISP_LOG(ERROR, "Failed to get android metadata AE_MODE");
    }
    // AE_LOCK
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_AE_LOCK))
    {
        tagID = ANDROID_CONTROL_AE_LOCK;
        if (!m_MetadataOps.pGetTag(pMetadata, tagID, &pData))
        {
            ptagData = (int*)pData;
            ISP_LOG(DBG, "Successfully got android metadata AE_LOCK = %d", *ptagData);
            pBayerCtrls->ae_lock = (qcarcam_ctrl_ae_lock_t)*ptagData;
        }
        else
            ISP_LOG(ERROR, "Failed to get android metadata AE_LOCK");
    }
    // AWB_MODE
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_AWB_MODE))
    {
        tagID = ANDROID_CONTROL_AWB_MODE;
        if (!m_MetadataOps.pGetTag(pMetadata, tagID, &pData))
        {
            ptagData = (int*)pData;
            ISP_LOG(DBG, "Successfully got android metadata AWB_MODE = %d", *ptagData);
            pBayerCtrls->awb_mode = (qcarcam_ctrl_awb_mode_t)*ptagData;
        }
        else
            ISP_LOG(ERROR, "Failed to get android metadata AWB_MODE");
    }
    // AWB_LOCK
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_AWB_LOCK))
    {
        tagID = ANDROID_CONTROL_AWB_LOCK;
        if (!m_MetadataOps.pGetTag(pMetadata, tagID, &pData))
        {
            ptagData = (int*)pData;
            ISP_LOG(DBG, "Successfully got android metadata AWB_LOCK = %d", *ptagData);
            pBayerCtrls->awb_lock = (qcarcam_ctrl_awb_lock_t)*ptagData;
        }
        else
            ISP_LOG(ERROR, "Failed to get android metadata AWB_LOCK");
    }
    // EFFECT_MODE
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_EFFECT_MODE))
    {
        tagID = ANDROID_CONTROL_EFFECT_MODE;
        if (!m_MetadataOps.pGetTag(pMetadata, tagID, &pData))
        {
            ptagData = (int*)pData;
            ISP_LOG(DBG, "Successfully got android metadata EFFECT_MODE = %d", *ptagData);
            pBayerCtrls->effect_mode = (qcarcam_ctrl_control_effect_mode_t)*ptagData;
        }
        else
            ISP_LOG(ERROR, "Failed to get android metadata EFFECT_MODE");
    }
    // CONTROL_MODE
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_MODE))
    {
        tagID = ANDROID_CONTROL_MODE;
        if (!m_MetadataOps.pGetTag(pMetadata, tagID, &pData))
        {
            ptagData = (int*)pData;
            ISP_LOG(DBG, "Successfully got android metadata CONTROL_MODE = %d", *ptagData);
            pBayerCtrls->ctrl_mode = (qcarcam_ctrl_control_mode_t)*ptagData;
        }
        else
            ISP_LOG(ERROR, "Failed to get android metadata CONTROL_MODE");
    }
    // SCENE_MODE
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_SCENE_MODE))
    {
        tagID = ANDROID_CONTROL_SCENE_MODE;
        if (!m_MetadataOps.pGetTag(pMetadata, tagID, &pData))
        {
            ptagData = (int*)pData;
            ISP_LOG(DBG, "Successfully got android metadata SCENE_MODE = %d", *ptagData);
            pBayerCtrls->scene_mode = (qcarcam_ctrl_control_scene_mode_t)*ptagData;
        }
        else
            ISP_LOG(ERROR, "Failed to get android metadata SCENE_MODE");
    }
    // AE_ANTIBANDING_MODE
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_AE_ANTIBANDING_MODE))
    {
        tagID = ANDROID_CONTROL_AE_ANTIBANDING_MODE;
        if (!m_MetadataOps.pGetTag(pMetadata, tagID, &pData))
        {
            ptagData = (int*)pData;
            ISP_LOG(DBG, "Successfully got android metadata AE_ANTIBANDING_MODE = %d", *ptagData);
            pBayerCtrls->ae_antibanding_mode = (qcarcam_ctrl_ae_antibanding_mode_t)*ptagData;
        }
        else
            ISP_LOG(ERROR, "Failed to get android metadata AE_ANTIBANDING_MODE");
    }

    // CONTROL_AE_EXPOSURE_COMPENSATION
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_AE_COMPENSATION))
    {
        tagID = ANDROID_CONTROL_AE_EXPOSURE_COMPENSATION;
        if (!m_MetadataOps.pGetTag(pMetadata, tagID, &pData))
        {
            ptagData = (int*)pData;

            // client API ae compensation range [-1.0, 1.0]
            const float minClientAECompensation = -1.0;
            const float maxClientAECompensation = 1.0;

            // Tags ae compensation range [-12, 12]
            const int minAECompensationTableValue = -12;
            const int maxAECompensationTableValue = 12;

            pBayerCtrls->ae_compensation = ((float)(*ptagData) - minAECompensationTableValue)
                                            / (maxAECompensationTableValue - minAECompensationTableValue)
                                            * (maxClientAECompensation- minClientAECompensation)
                                            + minClientAECompensation;
            ISP_LOG(DBG, "Successfully got android metadata AE_EXPOSURE_COMPENSATION = %d, pBayerCtrls->ae_compensation %f",
                *ptagData, pBayerCtrls->ae_compensation);
        }
        else
            ISP_LOG(ERROR, "Failed to get android metadata AE_EXPOSURE_COMPENSATION");
    }

    // CONTROL_AE_REGIONS
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_AE_REGIONS))
    {
        tagID = ANDROID_CONTROL_AE_REGIONS;

        if (!m_MetadataOps.pGetTag(pMetadata, tagID, &pData))
        {
            if (pData)
            {
                ISP_LOG(DBG, "Successfully get tag aeRegions[%d, %d, %d, %d], weight %d",
                    ((int *)pData)[0],  ((int *)pData)[1],  ((int *)pData)[2],  ((int *)pData)[3], ((int *)pData)[4]);

                pBayerCtrls->ae_regions.xMin = ((int *)pData)[0];
                pBayerCtrls->ae_regions.yMin = ((int *)pData)[1];
                pBayerCtrls->ae_regions.xMax = ((int *)pData)[2];
                pBayerCtrls->ae_regions.yMax = ((int *)pData)[3];
                pBayerCtrls->ae_regions.weight = ((int *)pData)[4];
            }
        }
    }
}

void AisPProcIspChiSession::SetVendorTags(CHIMETADATAHANDLE pMetadata,
        qcarcam_param_isp_ctrls_t *pBayerCtrls)
{
    UINT32 tagData = 0;

    // contrast_level
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_CONTRAST_LEVEL))
    {
        // client API contrast range [-1.0, 1.0]
        const float minClientContrastLevel = -1.0;
        const float maxClientContrastLevel = 1.0;

        // Tags contrast range [1, 11], IPE node will do appLevel-1
        const int minContrastTableValue = 1;
        const int maxContrastTableValue = 11;

        float mapContrast = (pBayerCtrls->contrast_level - minClientContrastLevel) * (maxContrastTableValue - minContrastTableValue)
                    / (maxClientContrastLevel - minClientContrastLevel)
                    + minContrastTableValue;
        // convert float to int
        tagData = ceil(mapContrast);
        if (!m_MetadataOps.pSetVendorTag(pMetadata, "org.codeaurora.qcamera3.contrast", "level", &tagData, 1))
        {
            ISP_LOG(DBG, "pSetTag Successfully set vendor tag contrast_level %d, temp %f", tagData, mapContrast);
        }
        else
            ISP_LOG(ERROR, "Failed to set vendor tag contrast");
    }

    // saturation
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_SATURATION))
    {

        // client API saturation range [-1.0, 1.0]
        const float minClientSaturationValue = -1.0;
        const float maxClientSaturationValue = 1.0;

        // Tags saturation range [0, 10], sync with PlatformStaticCaps saturationRange
        int minSaturationTableValue = 0;
        int maxSaturationTableValue = 10;
        int defaultValue            = 5;
        int step                    = 1;

        int availableSaturationRange[4];
        availableSaturationRange[0] = minSaturationTableValue;
        availableSaturationRange[1] = maxSaturationTableValue;
        availableSaturationRange[2] = defaultValue;
        availableSaturationRange[3] = step;

        if (!m_MetadataOps.pSetVendorTag(pMetadata, "org.codeaurora.qcamera3.saturation", "range", (void **)&availableSaturationRange, 4))
        {
            ISP_LOG(DBG, "Successfully set vendor tag aturation range[%d, %d, %d, %d]",
                availableSaturationRange[0], availableSaturationRange[1],
                availableSaturationRange[2], availableSaturationRange[3]);
        }
        else
            ISP_LOG(ERROR, "Failed to set vendor tag saturation range");

        float saturationValue = pBayerCtrls->saturation;

        float mapSaturation = ((saturationValue - minClientSaturationValue) * (maxSaturationTableValue - minSaturationTableValue)
                    / (maxClientSaturationValue - minClientSaturationValue)
                    + minSaturationTableValue);
        //convert float to int
        tagData = round(mapSaturation);

        if (!m_MetadataOps.pSetVendorTag(pMetadata, "org.codeaurora.qcamera3.saturation", "use_saturation", &tagData, 1))
        {
            ISP_LOG(DBG, "pMetadata %p, Successfully set vendor tag saturation %d, %f", pMetadata, tagData, saturationValue);
        }
        else
            ISP_LOG(ERROR, "Failed to set vendor tag saturation use_saturation");

    }
}

void AisPProcIspChiSession::GetVendorTags(CHIMETADATAHANDLE pMetadata,
        qcarcam_param_isp_ctrls_t *pBayerCtrls)
{
    void *pData = NULL;
    int *ptagData = NULL;

    // contrast_level
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_CONTRAST_LEVEL))
    {
        if (!m_MetadataOps.pGetVendorTag(pMetadata, "org.codeaurora.qcamera3.contrast", "level", &pData))
        {
            ptagData = (int*)pData;
            ISP_LOG(ERROR, "Successfully get vendor tag contrast pData %p, ptagData %p", pData, ptagData);
            if (ptagData)
            {
                // client API contrast range [-1.0, 1.0]
                const float minClientContrastLevel = -1.0;
                const float maxClientContrastLevel = 1.0;

                // Tags contrast range [0, 10]
                const int minContrastTableValue = 0;
                const int maxContrastTableValue = 10;

                pBayerCtrls->contrast_level = (float)((*ptagData - minContrastTableValue) * (maxClientContrastLevel- minClientContrastLevel)
                                            / (maxContrastTableValue - minContrastTableValue) + minClientContrastLevel);

                ISP_LOG(DBG, "Successfully get vendor tag contrast %d, contrast_level %f", *ptagData, pBayerCtrls->contrast_level);
            }
        }
        else
        {
            ISP_LOG(ERROR, "Failed to get vendor tag contrast ");
        }
    }

    // saturation
    if (CHECK_BIT(pBayerCtrls->param_mask, QCARCAM_CONTROL_SATURATION))
    {
        if (!m_MetadataOps.pGetVendorTag(pMetadata, "org.codeaurora.qcamera3.saturation", "range", &pData))
        {
            // client saturation range [-1.0, 1.0]
            const float minClientSaturationValue = -1.0;
            const float maxClientSaturationValue = 1.0;

            int minSaturationTableValue = 0;
            int maxSaturationTableValue = 10;
            if (pData)
            {
                ISP_LOG(DBG, "Successfully get vendor tag aturation range[%d, %d, %d, %d]",
                    ((int *)pData)[0],  ((int *)pData)[1],  ((int *)pData)[2],  ((int *)pData)[3]);

                minSaturationTableValue = ((int *)pData)[0];
                maxSaturationTableValue = ((int *)pData)[1];
            }

            if (!m_MetadataOps.pGetVendorTag(pMetadata, "org.codeaurora.qcamera3.saturation", "use_saturation", &pData))
            {
                ptagData = (int*)pData;
                if (ptagData)
                {
                    pBayerCtrls->saturation = (float)((*ptagData - minSaturationTableValue) * (maxClientSaturationValue - minClientSaturationValue)
                                                / (maxSaturationTableValue - minSaturationTableValue) + minClientSaturationValue);
                    ISP_LOG(DBG, "Successfully get vendor tag saturation use_saturation %p, %d, %f", ptagData, *ptagData, pBayerCtrls->saturation);
                }
                else
                {
                    ISP_LOG(ERROR, "Failed to get vendor tag saturation use_saturation ptagData is NULL");
                }
            }
            else
            {
                ISP_LOG(ERROR, "Failed to get vendor tag saturation use_saturation");
            }
        }
        else
        {
            ISP_LOG(ERROR, "Failed to get vendor tag saturation range");
        }
    }
}

CameraResult AisPProcIspChiSession::UpdateFrameParams(qcarcam_param_isp_ctrls_t *pBayer_ctrls)
{
    CameraLockMutex(m_ApplySettingsMutex);
    memcpy(&m_BayerCtrls, pBayer_ctrls, sizeof(m_BayerCtrls));
    m_ApplySettingsReqCnt = 0;
    CameraUnlockMutex(m_ApplySettingsMutex);
    return CAMERA_SUCCESS;
}

CameraResult AisPProcIspChiSession::GetFrameParams(qcarcam_param_isp_ctrls_t *pBayer_ctrls)
{
    CameraResult rc = CAMERA_SUCCESS;
    if (pBayer_ctrls)
    {
        CameraLockMutex(m_ApplySettingsMutex);
        uint32_t requestId = m_CurrentFrameNum % maxRequests;
        CameraUnlockMutex(m_ApplySettingsMutex);
        GetAndroidTags(m_OfflineRequests[requestId].pOutputMetadata, pBayer_ctrls);
        GetVendorTags(m_OfflineRequests[requestId].pOutputMetadata, pBayer_ctrls);
    }
    else
        rc = CAMERA_EFAILED;

    return rc;
}

CameraResult AisPProcIspChiSession::QcarcamToChiIspUsecase(
    qcarcam_isp_usecase_t qcarcamIspUsecase,
    IspUsecaseId* pChiIspUsecase)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (NULL == pChiIspUsecase)
    {
        ISP_LOG(ERROR, "pChiIspUsecase is NULL");
        return CAMERA_EBADPARM;
    }

    switch (qcarcamIspUsecase)
    {
    case QCARCAM_ISP_USECASE_SHDR_BPS_IPE_AEC_AWB:
        *pChiIspUsecase = UsecaseShdrBpsIpeAECAWB;
        break;
    case QCARCAM_ISP_USECASE_BPS_IPE_AEC_AWB:
        *pChiIspUsecase = UsecaseBPSAECAWBIPEInputMipiRaw16OutNV12;
        break;
    default:
        ISP_LOG(ERROR, "qcarcamIspUsecase %d isn't supported",
            qcarcamIspUsecase);
        return CAMERA_EUNSUPPORTED;
    }

    return rc;
}

