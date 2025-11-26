////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxusecasemc.cpp
/// @brief Usecases class implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "chxusecase.h"
#include "chxusecasemc.h"
#include "chistatspropertydefines.h"
#include "chxutils.h"
#include "chxmulticamcontroller.h"
#include "chistatsproperty.h"

#define ATRACE_TAG (ATRACE_TAG_CAMERA | ATRACE_TAG_HAL)


// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files
static const UINT32 maxPipelines = 16;
struct SessionPipelineMap
{
    UINT32 sessionIndex;                    ///< Session this pipeline belongs to
    UINT32 numPipeliens;
    UINT32 xmlPipeline[maxPipelines];       ///< Pipeline index from xml
};

struct usecaseMapInfo
{
    UINT32             numPhyCameras;       ///< Number of physical cameras
    UINT32             usecaseMode;         ///< Usecase mode like RTB/SAT
    SessionPipelineMap map[MAX_MULTI_CAMERA_SESSION];   ///< Pipeline to Session mapping
    ChiUsecase*        pUsecaseXML;         ///< pointer to usecase
};

struct usecaseMapInfo usecaseMap[] =
{
    {
        2, UsecaseSAT,
        {
            {REALTIME_SESSION,            2, {RealtimePreviewSAT0, RealtimePreviewSAT1}},
            {OFFLINE_YUV_SESSION,         1, {SATOfflinePreview }},
            {OFFLINE_RAW16_SESSION,       1, {SATOfflineRAW16 }},
            {OFFLINE_FUSION_SESSION,      1, {SATOfflineSnapshot }},
            {OFFLINE_JPEG_SESSION,        1, {SATJpegEncode }},
            {OFFLINE_RDI_SESSION0,        1, {SATSnapshotYUV0}},
            {OFFLINE_RDI_SESSION1,        1, {SATSnapshotYUV1}}
        },
        g_pUsecaseSAT,
    },
    {
        2, UsecaseRTB,
        {
            {REALTIME_SESSION,            2, {RealtimePreview0, RealtimePreview1}},
            {OFFLINE_YUV_SESSION,         1, {RTBOfflinePreview}},
            {OFFLINE_FUSION_SESSION,      1, {RTBOfflineSnapshot}},
            {OFFLINE_RDI_SESSION0,        1, {RTBSnapshotYUV0}},
            {OFFLINE_RDI_SESSION1,        1, {RTBSnapshotYUV1}},
            {OFFLINE_JPEG_SESSION,        1, {RTBJpegEncode}},
        },
        g_pUsecaseRTB,
    },
    {
        3, UsecaseSAT,
        {
            {REALTIME_SESSION,            3, {RealtimePreviewSAT0, RealtimePreviewSAT1, RealtimePreviewSAT2}},
            {OFFLINE_YUV_SESSION,         1, {SATOfflinePreview}},
            {OFFLINE_RAW16_SESSION,       1, {SATOfflineRAW16}},
            {OFFLINE_FUSION_SESSION,      1, {SATOfflineSnapshot}},
            {OFFLINE_JPEG_SESSION,        1, {SATJpegEncode}},
            {OFFLINE_RDI_SESSION0,        1, {SATSnapshotYUV0}},
            {OFFLINE_RDI_SESSION1,        1, {SATSnapshotYUV1}},
            {OFFLINE_RDI_SESSION2,        1, {SATSnapshotYUV2}},
        },
        g_pUsecaseSAT,
    },
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::~UsecaseMultiCamera
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UsecaseMultiCamera::~UsecaseMultiCamera()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::Create
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UsecaseMultiCamera* UsecaseMultiCamera::Create(
    LogicalCameraInfo*              cameraInfo,    ///< Camera info
    camera3_stream_configuration_t* pStreamConfig) ///< Stream configuration
{
    CDKResult             result                = CDKResultSuccess;
    UsecaseMultiCamera* pUsecaseMultiCamera = CHX_NEW UsecaseMultiCamera;

    if (NULL != pUsecaseMultiCamera)
    {
        result = pUsecaseMultiCamera->Initialize(cameraInfo, pStreamConfig);

        if (CDKResultSuccess != result)
        {
            pUsecaseMultiCamera->Destroy(FALSE);
            pUsecaseMultiCamera = NULL;
        }
    }
    else
    {
        result = CDKResultEFailed;
    }

    return pUsecaseMultiCamera;
}

UINT32 UsecaseMultiCamera::GetSessionIndex(
    UINT32              pipelineIndexFromUsecase,
    MultiCameraUsecase  usecase)
{
    UINT32 sessionIndex = MAX_MULTI_CAMERA_SESSION;

    for (UINT32 i = 0; i < (sizeof(usecaseMap)/sizeof(usecaseMap[0])); i++)
    {
        if (m_pLogicalCameraInfo->numPhysicalCameras == usecaseMap[i].numPhyCameras)
        {
            if (usecase == usecaseMap[i].usecaseMode)
            {
                for (UINT32 j = 0; j < MAX_MULTI_CAMERA_SESSION; j++)
                {
                    for (UINT32 id = 0; id < usecaseMap[i].map[j].numPipeliens; id++)
                    {
                        if (pipelineIndexFromUsecase == usecaseMap[i].map[j].xmlPipeline[id])
                        {
                            sessionIndex = usecaseMap[i].map[j].sessionIndex;
                            break;
                        }
                    }
                    if (sessionIndex != MAX_MULTI_CAMERA_SESSION)
                    {
                        break;
                    }
                }
            }
        }
        if (sessionIndex != MAX_MULTI_CAMERA_SESSION)
        {
            break;
        }
    }

    if (sessionIndex == MAX_MULTI_CAMERA_SESSION)
    {
        CHX_LOG_WARN("No Entry Found in table usecaseMap."
            "NumCam = %d, usecase = %d index = %d",
            m_pLogicalCameraInfo->numPhysicalCameras, usecase, pipelineIndexFromUsecase);
    }

    return sessionIndex;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::DestroyMultiCameraResource
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::DestroyMultiCameraResource()
{
    for (UINT i = 0 ; i < m_numOfPhysicalDevices; i++)
    {
        MultiCameraResource* pObj = &m_multiCamResource[i];
        if (NULL != pObj->m_pYUVInputStream)
        {
            ChxUtils::Free(pObj->m_pYUVInputStream);
            pObj->m_pYUVInputStream = NULL;
        }

        if (NULL != pObj->m_pDS4InputStream)
        {
            ChxUtils::Free(pObj->m_pDS4InputStream);
            pObj->m_pDS4InputStream = NULL;
        }

        if (NULL != pObj->m_pDS16InputStream)
        {
            ChxUtils::Free(pObj->m_pDS16InputStream);
            pObj->m_pDS16InputStream = NULL;
        }

        if (NULL != pObj->m_pRTOutputYUVStream)
        {
            ChxUtils::Free(pObj->m_pRTOutputYUVStream);
            pObj->m_pRTOutputYUVStream = NULL;
        }

        if (NULL != pObj->m_pRTOutputDS4Stream)
        {
            ChxUtils::Free(pObj->m_pRTOutputDS4Stream);
            pObj->m_pRTOutputDS4Stream = NULL;
        }

        if (NULL != pObj->m_pRTOutputDS16Stream)
        {
            ChxUtils::Free(pObj->m_pRTOutputDS16Stream);
            pObj->m_pRTOutputDS16Stream = NULL;
        }

        if (NULL != pObj->m_pRTOutputRDIStream)
        {
            ChxUtils::Free(pObj->m_pRTOutputRDIStream);
            pObj->m_pRTOutputRDIStream = NULL;
        }
        if (NULL != pObj->m_pRTOutputFDStream)
        {
            ChxUtils::Free(pObj->m_pRTOutputFDStream);
            pObj->m_pRTOutputFDStream = NULL;
        }
        if (NULL != pObj->m_pRTOutputRAW16Stream)
        {
            ChxUtils::Free(pObj->m_pRTOutputRAW16Stream);
            pObj->m_pRTOutputRAW16Stream = NULL;
        }
        if (NULL != pObj->m_pRAW16InputStream)
        {
            ChxUtils::Free(pObj->m_pRAW16InputStream);
            pObj->m_pRAW16InputStream = NULL;
        }
        if (NULL != pObj->m_pYUVOutputSnapshotStream)
        {
            ChxUtils::Free(pObj->m_pYUVOutputSnapshotStream);
            pObj->m_pYUVOutputSnapshotStream = NULL;
        }
        if (NULL != pObj->m_pJPEGInputStream)
        {
            ChxUtils::Free(pObj->m_pJPEGInputStream);
            pObj->m_pJPEGInputStream = NULL;
        }
        if (NULL != pObj->m_pDummyStream)
        {
            ChxUtils::Free(pObj->m_pDummyStream);
            pObj->m_pDummyStream = NULL;
        }
        if (NULL != pObj->m_pFusionStream)
        {
            ChxUtils::Free(pObj->m_pFusionStream);
            pObj->m_pFusionStream = NULL;
        }

        if (NULL != pObj->m_pRTYUVBufferManager)
        {
            pObj->m_pRTYUVBufferManager->Destroy();
            pObj->m_pRTYUVBufferManager = NULL;
        }
        if (NULL != pObj->m_pRAW16BufferManager)
        {
            pObj->m_pRAW16BufferManager->Destroy();
            pObj->m_pRAW16BufferManager = NULL;
        }
        if (NULL != pObj->m_pYUVSnapshotBufferManager)
        {
            pObj->m_pYUVSnapshotBufferManager->Destroy();
            pObj->m_pYUVSnapshotBufferManager = NULL;
        }
        if (NULL != pObj->m_pFusionSnapshotBufferManager)
        {
            pObj->m_pFusionSnapshotBufferManager->Destroy();
            pObj->m_pFusionSnapshotBufferManager = NULL;
        }

        if (NULL != pObj->m_pRTDS4BufferManager)
        {
            pObj->m_pRTDS4BufferManager->Destroy();
            pObj->m_pRTDS4BufferManager = NULL;
        }

        if (NULL != pObj->m_pRTDS16BufferManager)
        {
            pObj->m_pRTDS16BufferManager->Destroy();
            pObj->m_pRTDS16BufferManager = NULL;
        }

        if (NULL != pObj->m_pStickyMetadata)
        {
            pObj->m_pStickyMetadata->Destroy();
            pObj->m_pStickyMetadata = NULL;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::ExecuteFlush
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::ExecuteFlush()
{
    CDKResult result = CDKResultSuccess;

    result = AdvancedCameraUsecase::ExecuteFlush();

    Session* ppSessionsToFlush[MaxSessions];
    ChxUtils::Memset(ppSessionsToFlush, 0, sizeof(ppSessionsToFlush));

    for (UINT sessionIndex = 0; sessionIndex < MaxSessions; sessionIndex++)
    {
        ppSessionsToFlush[sessionIndex] = m_sessionInfo[sessionIndex].m_pSession;
    }
    FlushAllSessions(ppSessionsToFlush);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::Destroy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::Destroy(BOOL isForced)
{
    DestroyDeferResource();

    DestroyOfflineProcessResource();

    // First destroy the dual camera corresponding sessions
    for (UINT i = 0 ; i < MaxSessions; i++)
    {
        if (NULL != m_sessionInfo[i].m_pSession)
        {
            CHX_LOG_INFO("destroy Dual camera usecase %d session E", i);
            m_sessionInfo[i].m_pSession->Destroy(isForced);
            m_sessionInfo[i].m_pSession = NULL;
            CHX_LOG_INFO("destroy Dual camera usecase %d session X", i);
        }
        for (UINT32 pipelineIndex = 0 ; pipelineIndex < m_sessionInfo[i].m_numPipelines; pipelineIndex++)
        {
            DestroySyncInfo(&m_sessionInfo[i].m_waitUntilDrain[pipelineIndex]);
        }
    }

    // AdvancedCameraUsecase just initialize in SAT mode, so destroy it in SAT mode.
    if (canEnableAdvanceFeature())
    {
        CHX_LOG_INFO("destroy advanced usecase session's in DC usecase E");
        AdvancedCameraUsecase::Destroy(isForced);
        CHX_LOG_INFO("destroy advanced usecase session's in DC usecase X");
    }

    for (UINT i = 0; i < MaxSessions; i++)
    {
        for (UINT j = 0 ; j < m_sessionInfo[i].m_numPipelines; j++)
        {
            if (NULL != m_sessionInfo[i].m_pPipeline[j])
            {
                m_sessionInfo[i].m_pPipeline[j]->Destroy();
                m_sessionInfo[i].m_pPipeline[j] = NULL;
            }
        }
    }

    DestroyAllInternalBufferQueues();

    // Destroy multicamera controller only when camera is closed otherwise
    // retain important data during reconfiguration
    if (TRUE == ExtensionModule::GetInstance()->IsCameraClose())
    {
        MultiCamControllerManager::GetInstance()->Destroy();
        m_pMultiCamController = NULL;
    }

    DestroyMultiCameraResource();

    if (NULL != m_pDeferSnapMetadataMutex)
    {
        m_pDeferSnapMetadataMutex->Destroy();
        m_pDeferSnapMetadataMutex = NULL;
    }

    // Free the input metadata for the offline pipeline for preview
    for (UINT32 index = 0; index < MAX_EMPTY_RTMETA_COUNT; ++index)
    {
        if (NULL != m_pOfflinePipelineInputMetadataPreview[index])
        {
            m_pOfflinePipelineInputMetadataPreview[index]->Destroy();
            m_pOfflinePipelineInputMetadataPreview[index] = NULL;
        }
    }

    // Free the input metadata for the offline pipeline for snapshot
    for (UINT32 index = 0; index < MAX_EMPTY_OFFLINEMETA_COUNT; ++index)
    {
        if (NULL != m_pOfflinePipelineInputMetadataSnapshot[index])
        {
            m_pOfflinePipelineInputMetadataSnapshot[index]->Destroy();
            m_pOfflinePipelineInputMetadataSnapshot[index] = NULL;
        }
    }

    // Free the input metadata for the offline pipeline for RAW16
    if (NULL != m_pOfflinePipelineInputMetadataRAW16)
    {
        m_pOfflinePipelineInputMetadataRAW16->Destroy();
        m_pOfflinePipelineInputMetadataRAW16 = NULL;
    }

    // Free debug-data for holding real-time data
    if (NULL != m_debugDataRealTime.pBaseAllocation)
    {
        CHX_FREE(m_debugDataRealTime.pBaseAllocation);
        m_debugDataRealTime.pBaseAllocation = NULL;
        m_debugDataRealTime.fullSize        = 0;
        m_debugDataRealTime.maxQueue        = 0;
    }
    // Free debug-data for offline processing
    if (NULL != m_debugDataOffline.pData)
    {
        CHX_FREE(m_debugDataOffline.pData);
        m_debugDataOffline.pData    = NULL;
        m_debugDataOffline.size     = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::processRAW16Frame
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::processRAW16Frame(
    const CHICAPTURERESULT* pResult)
{
    CDKResult           result                  = CDKResultSuccess;
    UINT32              pipelineIndex           = pResult->pPrivData->requestIndex;
    UINT32              internalFrameNum        = pResult->frameworkFrameNum;
    UINT32              internalFrameNumIndex   = internalFrameNum %  MaxOutstandingRequests;
    BOOL                hasRAW16Stream          = FALSE;
    CHISTREAMBUFFER*    RAW16Buffer             = NULL;
    ChiMetadata*        pChiOutputMetadata      = NULL;
    SessionInfo*        pSessionInfo            = &m_sessionInfo[OFFLINE_RAW16_SESSION];

    if (m_numOfPhysicalDevices <= pipelineIndex)
    {
        CHX_LOG_ERROR("Result: Cannot Accept pipeline index of more than 1 frame = %d", pResult->frameworkFrameNum);
        // Early return
        return CDKResultEFailed;
    }

    for (UINT32 i = 0; i < pResult->numOutputBuffers; i++)
    {
        if (IsRdiRAW16Stream(pResult->pOutputBuffers[i].pStream))
        {
            hasRAW16Stream = TRUE;
            RAW16Buffer    = const_cast<CHISTREAMBUFFER*>(&pResult->pOutputBuffers[i]);
            break;
        }
    }

    pSessionInfo->m_bufferQueue[internalFrameNumIndex].frameNumber = pResult->frameworkFrameNum;
    if (NULL != pResult->pOutputMetadata)
    {
        pChiOutputMetadata = m_pMetadataManager->GetMetadataFromHandle(pResult->pOutputMetadata);

        // Receive Realtime pipeline result
        pSessionInfo->m_bufferQueue[internalFrameNumIndex].pMetadata[pipelineIndex] = pChiOutputMetadata;
        pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex]    |= META_READY_FLAG;

        CHX_LOG("pipeline idx:%d meta is ready,value:%x", pipelineIndex,
            pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex]);
    }

    if ((TRUE == hasRAW16Stream) && (NULL != RAW16Buffer))
    {
        // Receive Realtime pipeline result
        pSessionInfo->m_bufferQueue[internalFrameNumIndex].frameNumber = internalFrameNum;

        ChxUtils::Memcpy(&pSessionInfo->m_bufferQueue[internalFrameNumIndex].buffer[pipelineIndex],
            RAW16Buffer, sizeof(CHISTREAMBUFFER));
        pSessionInfo->m_bufferQueue[internalFrameNumIndex].buffer[pipelineIndex].pStream =
            m_multiCamResource[pipelineIndex].m_pRAW16InputStream;
        pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex]         |= BUF_READY_FLAG;

        CHX_LOG("RAW16 pipeline idx:%d buffer is ready,value:%x", pipelineIndex,
            pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex]);
    }

    // trigger reprocessing
    CHISTREAMBUFFER     outputBuffer[2];
    CHICAPTUREREQUEST   request     = {0};
    UINT32              buffercount = 0;

    ChxUtils::PopulateHALToChiStreamBuffer(&m_appRAW16Buffer[internalFrameNumIndex], &outputBuffer[buffercount]);

    buffercount++;

    request.frameNumber             = pSessionInfo->m_bufferQueue[internalFrameNumIndex].frameNumber;
    request.hPipelineHandle         = reinterpret_cast<CHIPIPELINEHANDLE>(
            pSessionInfo->m_pSession->GetPipelineHandle(0));
    request.numOutputs              = buffercount;
    request.pOutputBuffers          = outputBuffer;
    request.pPrivData               = &pSessionInfo->m_chiPrivData[0][internalFrameNumIndex];
    request.pPrivData->streamIndex  = 0;

    MulticamResultMetadata multiCamResultMetadata;
    multiCamResultMetadata.frameNum            = pResult->frameworkFrameNum;

    // check if the preview result is ready for this pipeline
    if ((pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex] & META_READY_FLAG) &&
        (pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex] & BUF_READY_FLAG))
    {
        pSessionInfo->m_bufferQueue[internalFrameNumIndex].m_resultMask =
            ChxUtils::BitSet(pSessionInfo->m_bufferQueue[internalFrameNumIndex].m_resultMask, pipelineIndex);
    }

    // if result map is equal with request map, it means all result for preview is ready, go to offline pipeline
    if (m_requestInfo[internalFrameNumIndex].m_rawMask ==
        pSessionInfo->m_bufferQueue[internalFrameNumIndex].m_resultMask)
    {
        UINT32            numOfInputs = 0;
        ChiMetadata*      pResultMetadata[MaxDevicePerLogicalCamera];
        CHISTREAMBUFFER   resultBuffer[MaxDevicePerLogicalCamera];
        CHIBufferManager* pBufferManager[MaxDevicePerLogicalCamera];
        ChxUtils::Memset(resultBuffer, 0, sizeof(resultBuffer));
        ChxUtils::Memset(pResultMetadata, 0, sizeof(pResultMetadata));

        ChiMetadata* pOutputMetadata = m_pMetadataManager->Get(
            pSessionInfo->m_pSession->GetMetadataClientId(), request.frameNumber);

        if (NULL == pOutputMetadata)
        {
            CHX_LOG_ERROR("Cannot get offline output metadata for frame %" PRId64, request.frameNumber);
            return CDKResultEFailed;
        }

        CHX_LOG("Metadata internalFrameNumIndex %d (%p %p %p)", internalFrameNumIndex,
               pSessionInfo->m_bufferQueue[internalFrameNumIndex].pMetadata[0],
               pSessionInfo->m_bufferQueue[internalFrameNumIndex].pMetadata[1],
               m_pOfflinePipelineInputMetadataRAW16);

        for (UINT32 i = 0 ; i < m_numOfPhysicalDevices ; i++)
        {
            if (ChxUtils::IsBitSet(pSessionInfo->m_bufferQueue[internalFrameNumIndex].m_resultMask, i))
            {
                pResultMetadata[numOfInputs] = pSessionInfo->m_bufferQueue[internalFrameNumIndex].pMetadata[i];
                resultBuffer[numOfInputs]    = pSessionInfo->m_bufferQueue[internalFrameNumIndex].buffer[i];
                pBufferManager[numOfInputs]  = m_multiCamResource[i].m_pRAW16BufferManager;
                numOfInputs ++;
                pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[i] = 0;
            }
        }

        request.numInputs     = numOfInputs;
        request.pInputBuffers = resultBuffer;

        // Save input buffers info for later releasing reference
        for (UINT32 i = 0 ; i < numOfInputs ; i++)
        {
            request.pPrivData->inputBuffers[i]    = resultBuffer[i].bufferInfo;
            request.pPrivData->bufferManagers[i]  = pBufferManager[i];
        }
        request.pPrivData->numInputBuffers        = numOfInputs;

        AppendOfflineMetadata(pResult->frameworkFrameNum,
                    pResultMetadata[0],
                    pResultMetadata[1],
                    m_pOfflinePipelineInputMetadataRAW16,
                    FALSE,
                    FALSE);

        request.pInputMetadata  = m_pOfflinePipelineInputMetadataRAW16->GetHandle();

        if (NULL != pChiOutputMetadata)
        {
            request.pOutputMetadata = pChiOutputMetadata->GetHandle();
        }

        CHX_LOG("OfflineRAW: reprocessing frameNum = %" PRIu64 " buffercount %d, requestmap:0x%x",
            request.frameNumber, buffercount, m_requestInfo[internalFrameNumIndex].m_rawMask);

        CHX_LOG_REQMAP("frame: %u  <==>  chiOverrideFrameNum: %u <==>  internalFrameNum: %u  <==>  chiFrameNum: %" PRIu64
                       " -- Offline RAW16 Request",
                       GetAppFrameNum(m_requestMapInfo[internalFrameNumIndex].frameNumber),
                       m_requestMapInfo[internalFrameNumIndex].frameNumber,
                       internalFrameNum,
                       request.frameNumber);

        processSessionRequest(OFFLINE_RAW16_SESSION, 1, &request);
        m_requestInfo[internalFrameNumIndex].m_rawMask  = 0;
    }

    return result;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::processRDIFrame
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::processRDIFrame(
    const ChiCaptureResult* pResult)
{
    CHICAPTUREREQUEST   request                 = {0};
    UINT32              pipelineIndex;
    UINT32              sessionIndex;
    BOOL                hasRDIStream            = FALSE;
    BOOL                hasRDIRaw16Stream       = FALSE;
    BOOL                hasFDStream             = FALSE;
    CHISTREAMBUFFER*    pRDIRaw16Buffer         = NULL;
    ChiStreamBuffer*    FDBuffer                = NULL;
    UINT32              internalFrameNum        = pResult->frameworkFrameNum;
    UINT32              internalFrameNumIndex   = internalFrameNum % MaxOutstandingRequests;

    if (NULL == pResult->pPrivData)
    {
        CHX_LOG_ERROR("Result: Cannot Accept NULL private data here for = %d", internalFrameNum);
        // Early return
        return;
    }
    else
    {
        pipelineIndex = pResult->pPrivData->streamIndex;
    }

    for (UINT32 i = 0; i < pResult->numOutputBuffers; i++)
    {
        if (IsRdiStream(pResult->pOutputBuffers[i].pStream))
        {
            hasRDIStream = TRUE;
            UpdateBufferReadyForRDIQueue(internalFrameNum, pipelineIndex, TRUE);

            CHX_LOG("[MultiCamera RDI] RDI MIPI RAW 10 found pipelineIndex %d", pipelineIndex);
        }

        if (IsFDStream(pResult->pOutputBuffers[i].pStream))
        {
            hasFDStream = TRUE;
            FDBuffer    = const_cast<ChiStreamBuffer*>(&pResult->pOutputBuffers[i]);
            UpdateBufferReadyForFDQueue(internalFrameNum, pipelineIndex, TRUE);
            CHX_LOG("[MultiCamera FD] FD found pipelineIndex %d", pipelineIndex);
        }

        if (IsRdiRAW16Stream(pResult->pOutputBuffers[i].pStream))
        {
            hasRDIRaw16Stream = TRUE;
            pRDIRaw16Buffer    = const_cast<CHISTREAMBUFFER*>(&pResult->pOutputBuffers[i]);
            CHX_LOG("[MultiCamera RDI] RDI RAW 16 found pipelineIndex %d", pipelineIndex);
        }
    }

    // RDI RAW16 request for getting offline RAW16
    if ((TRUE == hasRDIRaw16Stream) && (NULL != pRDIRaw16Buffer))
    {
        processRAW16Frame(pResult);
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::ProcessDriverPartialCaptureResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::ProcessDriverPartialCaptureResult(
    CHIPARTIALCAPTURERESULT*    pResult,
    UINT32                      sessionId)
{
    UINT32               pipelineIndex;
    UINT32               internalFrameNum             = pResult->frameworkFrameNum;
    UINT32               internalFrameNumIndex        = internalFrameNum %  MaxOutstandingRequests;
    PartialResultSender  sender                       = PartialResultSender::DriverPartialData;
    UINT32               applicationFrameNum          = m_requestMapInfo[internalFrameNumIndex].frameNumber;
    UINT32               applicationFrameNumIndex     = applicationFrameNum % MaxOutstandingRequests;
    ChiMetadata*         pChiOutputMetadata           = NULL;

    if (NULL == pResult->pPrivData)
    {
        CHX_LOG_ERROR("Result: Cannot Accept NULL private data here for = %d", pResult->frameworkFrameNum);
        // Early return
        return;
    }
    else
    {
        pipelineIndex = pResult->pPrivData->streamIndex;
    }

    if (pipelineIndex == m_requestInfo[applicationFrameNumIndex].masterPipelineIndex)
    {
        pChiOutputMetadata = m_pMetadataManager->GetMetadataFromHandle(pResult->pPartialResultMetadata);

        // This check is to ensure that we have not sent earlier
        if (TRUE == CheckIfPartialDataCanBeSent(sender, applicationFrameNumIndex))
        {
            if (PartialMetaSupport::CombinedPartialMeta == ExtensionModule::GetInstance()->EnableCHIPartialData())
            {
                ProcessCHIPartialData(pResult->frameworkFrameNum,0);
            }

            UpdateAppPartialResultMetadataFromDriver(pChiOutputMetadata,
                applicationFrameNumIndex,
                applicationFrameNum,
                m_sessionInfo[sessionId].m_pPipeline[pipelineIndex]->GetMetadataClientId());

            ProcessAndReturnPartialMetadataFinishedResults(sender);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::ProcessCHIPartialData
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::ProcessCHIPartialData(
    UINT32    frameNum,
    UINT32    sessionId)
{
    CAMX_UNREFERENCED_PARAM(frameNum);
    CAMX_UNREFERENCED_PARAM(sessionId);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::RealtimeCaptureResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::RealtimeCaptureResult(
    ChiCaptureResult* pResult)
{
    UINT32       pipelineIndex;
    UINT32       internalFrameNum      = pResult->frameworkFrameNum;
    UINT32       internalFrameNumIndex = internalFrameNum %  MaxOutstandingRequests;
    ChiMetadata* pChiOutputMetadata    = m_pMetadataManager->GetMetadataFromHandle(pResult->pOutputMetadata);

    if (NULL != pChiOutputMetadata)
    {
        AECFrameControl* pAECFrameControl = static_cast<AECFrameControl*>(pChiOutputMetadata->GetTag(
                                                    "org.quic.camera2.statsconfigs", "AECFrameControl"));

        if (NULL != pAECFrameControl)
        {
            CHX_LOG_VERBOSE("RealtimeCaptureResult: isSeamlessIHDRSupported=%u, IHDRTrigger=%u, previous result=%u",
                                            pAECFrameControl->IHDRTriggerOutputInfo.isSeamlessIHDRSupported,
                                            pAECFrameControl->isInsensorHDRSnapshot,
                                            m_isIHDRAECTrigger);

            // For multiple camera case, AEC will know whether each sensor can support seamless IHDR or not from tuning XML.
            // And if yes, AEC will do seamless IHDR trigger detection, calculate the corresponding exposure info and output the
            // result to meta.
            if (TRUE == pAECFrameControl->IHDRTriggerOutputInfo.isSeamlessIHDRSupported)
            {
                m_isIHDRAECTrigger = pAECFrameControl->isInsensorHDRSnapshot;
            }
        }
    }

    if (NULL == pResult->pPrivData)
    {
        CHX_LOG_ERROR("Result: Cannot Accept NULL private data here for = %d", pResult->frameworkFrameNum);
        // Early return
        return;
    }
    else
    {
        pipelineIndex = pResult->pPrivData->streamIndex;
    }

    if ((pipelineIndex == m_requestInfo[internalFrameNumIndex].masterPipelineIndex) &&
        (NULL != pResult->pOutputMetadata))
    {
        AdvancedCameraUsecase::ParseResultMetadata(m_pMetadataManager->GetMetadataFromHandle(pResult->pOutputMetadata));
    }

    NotifyResultsAvailable(pResult);

    processRDIFrame(pResult);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::AppendOfflineMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::AppendOfflineMetadata(
    UINT32       frameNum,
    ChiMetadata* pMetadata1,
    ChiMetadata* pMetadata2,
    ChiMetadata* pResultMetadata,
    BOOL         isSnapshotMeta,
    BOOL         bReleaseInputMeta)
{
    CDKResult result              = CDKResultSuccess;

    UINT metadataCount = 1;

    MulticamResultMetadata multiCamResultMetadata;
    multiCamResultMetadata.frameNum         = frameNum;

    if (NULL != pMetadata2)
    {
        metadataCount++;
    }

    multiCamResultMetadata.numResults       = metadataCount;
    multiCamResultMetadata.ppResultMetadata =
        static_cast<ChiMetadata**>(CHX_CALLOC(DualCamCount * sizeof(ChiMetadata*)));

    if (NULL != multiCamResultMetadata.ppResultMetadata)
    {
        multiCamResultMetadata.ppResultMetadata[0] = pMetadata1;
        multiCamResultMetadata.ppResultMetadata[1] = pMetadata2;
    }
    else
    {
        CHX_LOG_ERROR("CALLOC failed for ppResultMetadata");
        CHX_ASSERT(NULL != multiCamResultMetadata.ppResultMetadata);
        result = CDKResultEFailed;
    }

    if (CDKResultSuccess == result)
    {
        m_pMultiCamController->FillOfflinePipelineInputMetadata(&multiCamResultMetadata,
            pResultMetadata, isSnapshotMeta);

        // Release the result metadata if the caller no longer needs them
        // Output metadata holds the input references if FillOfflinePipelineInputMetadata
        // is successful
        if (TRUE == bReleaseInputMeta)
        {
            for (UINT index = 0; index < DualCamCount; ++index)
            {
                if (NULL != multiCamResultMetadata.ppResultMetadata[index])
                {
                    m_pMetadataManager->Release(multiCamResultMetadata.ppResultMetadata[index]);
                }
            }
        }

        if (NULL != multiCamResultMetadata.ppResultMetadata)
        {
            CHX_FREE(multiCamResultMetadata.ppResultMetadata);
            multiCamResultMetadata.ppResultMetadata = NULL;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::SubmitJpegRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::SubmitJpegRequest(
    UINT32              frameNum,
    CHISTREAMBUFFER*    inputBuffer,
    CHISTREAMBUFFER*    JpegBuffer,
    ChiMetadata*        pMetadata,
    CHIBufferManager*   pInputBufferManager)
{
    CDKResult      result        = CDKResultSuccess;
    SessionInfo*   pSessionInfo  = &m_sessionInfo[OFFLINE_FUSION_SESSION];

    UINT32 internalFrameNumIndex  = m_jpegRequestCount %  MaxOutstandingRequests;

    m_jpegRequestId[internalFrameNumIndex].requestId = frameNum;
    m_jpegRequestId[internalFrameNumIndex].stream    = JpegBuffer->pStream;

    JpegBuffer->pStream = m_pTargetSnapshotStream;

    ChiMetadata* pOutputMetadata = m_pMetadataManager->Get(
        m_sessionInfo[OFFLINE_JPEG_SESSION].m_pSession->GetMetadataClientId(), frameNum);

    if (NULL == pOutputMetadata)
    {
        CHX_LOG_ERROR("Cannot get metadata %p", pOutputMetadata);
        return result;
    }

    CHICAPTUREREQUEST request               = {0};
    request.frameNumber                     = m_jpegRequestCount;
    request.hPipelineHandle                 = reinterpret_cast<CHIPIPELINEHANDLE>(
            m_sessionInfo[OFFLINE_JPEG_SESSION].m_pSession->GetPipelineHandle(0));

    request.numOutputs                     = 1;
    request.pOutputBuffers                 = JpegBuffer;
    request.numInputs                      = 1;
    request.pInputBuffers                  = inputBuffer;

    request.pPrivData                        = &pSessionInfo->m_chiPrivData[0][internalFrameNumIndex];
    request.pPrivData->streamIndex           = 0;
    request.pInputMetadata                   = pMetadata->GetHandle();
    request.pOutputMetadata                  = pOutputMetadata->GetHandle();

    // Save input buffers info for later releasing reference
    request.pPrivData->inputBuffers[0]      = inputBuffer->bufferInfo;
    request.pPrivData->bufferManagers[0]    = pInputBufferManager;
    request.pPrivData->numInputBuffers      = 1;

    CHX_LOG("OfflineSnapshot: send JPEG request frame = %d m_jpegRequestCount = %" PRIu64,
        frameNum, m_jpegRequestCount);

    CHX_LOG_REQMAP("frame: %u  <==>  chiOverrideFrameNum: %u <==> internalFrameNum: %u <==> chiFrameNum: %" PRIu64
        " -- Offline JPEG Snapshot",
        GetAppFrameNum(m_requestMapInfo[internalFrameNumIndex].frameNumber),
        m_requestMapInfo[internalFrameNumIndex].frameNumber,
        frameNum,
        request.frameNumber);

    result = processSessionRequest(OFFLINE_JPEG_SESSION, 1, &request);
    m_jpegRequestCount++;

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::OfflineSnapshotResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::OfflineSnapshotResult(
    const ChiCaptureResult* pResult, UINT32 sessionIdx)
{
    UINT32                 pipelineIndex            = 0;
    const CHISTREAMBUFFER* YUVBuffer                = NULL;
    UINT32                 internalFrameNum         = pResult->frameworkFrameNum;
    UINT32                 internalFrameNumIndex    = internalFrameNum %  MaxOutstandingRequests;
    ChiMetadata*           pChiOutputMetadata       = NULL;
    SessionInfo*           pSessionInfo             = &m_sessionInfo[sessionIdx];
    UINT32                 applicationFrameNum      = m_requestMapInfo[internalFrameNumIndex].frameNumber;
    UINT32                 applicationFrameNumIndex = applicationFrameNum % MaxOutstandingRequests;

    if (NULL == pResult->pPrivData)
    {
        CHX_LOG_ERROR("Result: Cannot Accept NULL private data here for = %d", pResult->frameworkFrameNum);
        // Early return
        return;
    }
    else
    {
        pipelineIndex = pResult->pPrivData->streamIndex;
    }

    for (UINT i = 0; i < pResult->numOutputBuffers; i++)
    {
        if ((TRUE == m_isPostViewNeeded) &&
            (pResult->pOutputBuffers[i].pStream == m_pTargetPostviewStream))
        {
            camera3_capture_result_t *pAppResult   = GetCaptureResult(applicationFrameNumIndex);
            pAppResult->frame_number               = applicationFrameNum;

            m_pAppResultMutex->Lock();
            camera3_stream_buffer_t* pResultBuffer =
                    const_cast<camera3_stream_buffer_t*>(&pAppResult->output_buffers[pAppResult->num_output_buffers++]);

            ChxUtils::PopulateChiToHALStreamBuffer(&pResult->pOutputBuffers[i], pResultBuffer);
            m_pAppResultMutex->Unlock();

            ProcessAndReturnFinishedResults();
            CHX_LOG("Thumbnail Callback: Sent to application i =%d", i);
        }
        else
        {
           YUVBuffer = const_cast <CHISTREAMBUFFER *>(
                (CHISTREAMBUFFER *)&pResult->pOutputBuffers[i]);
        }
    }

    if (NULL != pResult->pOutputMetadata)
    {
        pChiOutputMetadata = m_pMetadataManager->GetMetadataFromHandle(pResult->pOutputMetadata);
        pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex]  |= META_READY_FLAG;

        CHX_LOG("OfflineSnapshotResult pipeline idx:%d meta is ready,value:%x",
                 pipelineIndex, pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex]);
        pSessionInfo->m_bufferQueue[internalFrameNumIndex].pMetadata[pipelineIndex] = pChiOutputMetadata;
    }

    ///< Receive Realtime pipeline result
    if (NULL != YUVBuffer)
    {
        pSessionInfo->m_bufferQueue[internalFrameNumIndex].frameNumber = internalFrameNum;

        if (pResult->numOutputBuffers > 0)
        {
            // Release reference to the input RDI buffers - regular snapshot without mfnr case
            ReleaseReferenceToInputBuffers(pResult->pPrivData);

            // Update valid buffer length when release input buffer handle
            // Feature input resource will be released in feature
            if (FeatureType::UNINIT == pResult->pPrivData->featureType)
            {
                UpdateValidRDIBufferLength(pipelineIndex, 1);
            }
            ChxUtils::Memcpy(&pSessionInfo->m_bufferQueue[internalFrameNumIndex].buffer[pipelineIndex],
                             const_cast<CHISTREAMBUFFER*>(YUVBuffer),
                             sizeof(CHISTREAMBUFFER));

            pSessionInfo->m_bufferQueue[internalFrameNumIndex].buffer[pipelineIndex].pStream =
                m_multiCamResource[pipelineIndex].m_pJPEGInputStream;
            pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex]         |= BUF_READY_FLAG;

            CHX_LOG_INFO("OfflineSnapshotResult pipeline idx:%d buffer is ready,stream:%p,valid:%x", pipelineIndex,
                pSessionInfo->m_bufferQueue[internalFrameNumIndex].buffer[pipelineIndex].pStream,
                pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex]);
        }
    }

    // check if the preview result is ready for this pipeline
    if ((pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex] & META_READY_FLAG) &&
        (pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex] & BUF_READY_FLAG))
    {
        pSessionInfo->m_bufferQueue[internalFrameNumIndex].m_resultMask =
            ChxUtils::BitSet(pSessionInfo->m_bufferQueue[internalFrameNumIndex].m_resultMask,
            pipelineIndex);
    }

    UINT32 resultMask = 0;
    for (UINT32 i = OFFLINE_RDI_SESSION0; i < (OFFLINE_RDI_SESSION0 + m_numOfPhysicalDevices); i++)
    {
        resultMask |= m_sessionInfo[i].m_bufferQueue[internalFrameNumIndex].m_resultMask;
    }
    CHX_LOG_INFO("m_snapshotMask:0x%x,m_resultMask:0x%x frame = %d", m_requestInfo[internalFrameNumIndex].m_snapshotMask,
        resultMask, internalFrameNum);

    // if request == result, go to next session
    if ((m_requestInfo[internalFrameNumIndex].m_snapshotMask != 0) &&
        (m_requestInfo[internalFrameNumIndex].m_snapshotMask == resultMask))
    {
        UINT32            numOfInputs = 0;
        ChiMetadata*      pResultMetadata[MaxDevicePerLogicalCamera];
        CHISTREAMBUFFER   resultBuffer[MaxDevicePerLogicalCamera];
        CHIBufferManager* pBufferManager[MaxDevicePerLogicalCamera];
        ChxUtils::Memset(resultBuffer, 0, sizeof(resultBuffer));
        ChxUtils::Memset(pResultMetadata, 0, sizeof(pResultMetadata));
        ChxUtils::Memset(pBufferManager, 0, sizeof(pBufferManager));
        // construct result buffer and metadata for request input
        for (UINT32 resultSession = OFFLINE_RDI_SESSION0; resultSession < (OFFLINE_RDI_SESSION0 + m_numOfPhysicalDevices);
              resultSession++)
        {
            /*Below Code needs to be inspected for Triple camera*/
            UINT32 pipelineIndex = 0;
            switch (resultSession)
            {
            case OFFLINE_RDI_SESSION0:
                pipelineIndex = 0;
                break;
            case OFFLINE_RDI_SESSION1:
                pipelineIndex = 1;
                break;
            case OFFLINE_RDI_SESSION2:
                pipelineIndex = 2;
                break;
            default:
                pipelineIndex = 0;
                CHX_LOG_ERROR("We have a new RDI for which we dont have a mapping. We will have issue!!!");
                break;
            }
            SessionInfo* pResultSessionInfo = &m_sessionInfo[resultSession];
            if (ChxUtils::IsBitSet(pResultSessionInfo->m_bufferQueue[internalFrameNumIndex].m_resultMask, pipelineIndex))
            {
                pResultMetadata[numOfInputs] =
                    pResultSessionInfo->m_bufferQueue[internalFrameNumIndex].pMetadata[pipelineIndex];
                resultBuffer[numOfInputs]    = pResultSessionInfo->m_bufferQueue[internalFrameNumIndex].buffer[pipelineIndex];
                pBufferManager[numOfInputs]  = m_multiCamResource[pipelineIndex].m_pYUVSnapshotBufferManager;

                numOfInputs++;

                pResultSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex] = 0;
                m_requestInfo[internalFrameNumIndex].m_snapshotMask =
                    ChxUtils::BitReset(m_requestInfo[internalFrameNumIndex].m_snapshotMask, pipelineIndex);
            }
        }

        ChiMetadata* pOfflineSnapshotInputMeta = m_pOfflinePipelineInputMetadataSnapshot[
            pResult->frameworkFrameNum % MAX_EMPTY_OFFLINEMETA_COUNT];

        // capture wide sensor output
        if ((m_bokehPrimaryJpeg) &&
            (pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[m_primaryCameraIndex] & META_READY_FLAG) &&
            (pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[m_primaryCameraIndex] & BUF_READY_FLAG) &&
            (NULL != m_appWideJpegBuffer[applicationFrameNumIndex].buffer))
        {
            CHISTREAMBUFFER jpegBuffer = { 0 };

            ChxUtils::PopulateHALToChiStreamBuffer(&m_appWideJpegBuffer[applicationFrameNumIndex], &jpegBuffer);

            jpegBuffer.acquireFence.valid   = FALSE;
            jpegBuffer.pStream              = m_pTargetPrimaryJpegStream;

            AppendOfflineMetadata(internalFrameNum,
                        pResultMetadata[0],
                        pResultMetadata[1],
                        pOfflineSnapshotInputMeta,
                        TRUE,
                        TRUE);

            // Increase Ref count as the same buffer would be submitted to Fusion pipeline as well.
            m_multiCamResource[m_primaryCameraIndex].m_pYUVSnapshotBufferManager->AddReference(
                &pSessionInfo->m_bufferQueue[internalFrameNumIndex].buffer[m_primaryCameraIndex].bufferInfo);

            SubmitJpegRequest(pResult->frameworkFrameNum,
                    &pSessionInfo->m_bufferQueue[internalFrameNumIndex].buffer[m_primaryCameraIndex],
                    &jpegBuffer,
                    pOfflineSnapshotInputMeta,
                    m_multiCamResource[m_primaryCameraIndex].m_pYUVSnapshotBufferManager);

        }

        CHICAPTUREREQUEST request               = {0};

        // fusion capture or bokeh capture to fusion pipeline
        if (numOfInputs > 1)
        {
            ChiMetadata* pOutputMetadata = m_pMetadataManager->Get(
                m_sessionInfo[OFFLINE_FUSION_SESSION].m_pSession->GetMetadataClientId(),
                pResult->frameworkFrameNum);

            request.frameNumber                     = pSessionInfo->m_bufferQueue[internalFrameNumIndex].frameNumber;
            request.hPipelineHandle                 = reinterpret_cast<CHIPIPELINEHANDLE>(
                m_sessionInfo[OFFLINE_FUSION_SESSION].m_pSession->GetPipelineHandle(0));

            request.numOutputs                      = 1;

            // Get buffer to hold fusion image
            CHISTREAMBUFFER   outputBuffer = { 0 };
            outputBuffer.size               = sizeof(CHISTREAMBUFFER);
            outputBuffer.acquireFence.valid = FALSE;
            outputBuffer.bufferInfo         =
                m_multiCamResource[0].m_pFusionSnapshotBufferManager->GetImageBufferInfo();
            outputBuffer.pStream            = m_multiCamResource[m_maxJpegCameraId].m_pFusionStream;
            request.pOutputBuffers          = &outputBuffer;

            request.numInputs               = numOfInputs;
            request.pInputBuffers           = resultBuffer;

            AppendOfflineMetadata(internalFrameNum,
                    pResultMetadata[0],
                    pResultMetadata[1],
                    pOfflineSnapshotInputMeta,
                    TRUE,
                    TRUE);

            request.pMetadata                      = pOfflineSnapshotInputMeta;
            request.pPrivData                      = &pSessionInfo->m_chiPrivData[0][internalFrameNumIndex];
            request.pPrivData->streamIndex         = 0;
            request.pInputMetadata                 = pOfflineSnapshotInputMeta->GetHandle();
            if (NULL != pOutputMetadata)
            {
                request.pOutputMetadata = pOutputMetadata->GetHandle();
            }

            // Save input buffers info for later releasing reference
            request.pPrivData->numInputBuffers = numOfInputs;
            for (UINT32 i = 0 ; i < numOfInputs ; i++)
            {
                request.pPrivData->inputBuffers[i]   = resultBuffer[i].bufferInfo;
                request.pPrivData->bufferManagers[i] = pBufferManager[i];
            }

            CHX_LOG("OfflineSnapshot: send capture fusion request in dual zone");

            CHX_LOG_REQMAP("frame: %u  <==>  chiOverrideFrameNum: %u <==> internalFrameNum: %u <==> chiFrameNum: %" PRIu64
                           " -- Offline Fusion Request",
                           GetAppFrameNum(applicationFrameNum),
                           applicationFrameNum,
                           internalFrameNum,
                           request.frameNumber);

            processSessionRequest(OFFLINE_FUSION_SESSION, 1, &request);
        }
        else//single capture to jpeg pipeline
        {
            AppendOfflineMetadata(internalFrameNum,
                    pResultMetadata[0],
                    NULL,
                    pOfflineSnapshotInputMeta,
                    TRUE,
                    TRUE);

            CHISTREAMBUFFER snapshotBuffer = { 0 };

            ChxUtils::PopulateHALToChiStreamBuffer(&m_appSnapshotBuffer[applicationFrameNumIndex], &snapshotBuffer);

            SubmitJpegRequest(pSessionInfo->m_bufferQueue[internalFrameNumIndex].frameNumber,
                              &resultBuffer[0],
                              &snapshotBuffer,
                              pOfflineSnapshotInputMeta,
                              pBufferManager[0]);

        }
        pSessionInfo->m_bufferQueue[internalFrameNumIndex] = { 0 } ;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::OfflineFusionSnapshotResult
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::OfflineFusionSnapshotResult(
    const ChiCaptureResult* pResult)
{
    UINT32            internalFrameNum         = pResult->frameworkFrameNum;
    UINT32            internalFrameNumIndex    = internalFrameNum % MaxOutstandingRequests;
    ChiMetadata*      pChiOutputMetadata       = NULL;
    SessionInfo*      pSessionInfo             = &m_sessionInfo[OFFLINE_FUSION_SESSION];
    UINT32            applicationFrameNum      = m_requestMapInfo[internalFrameNumIndex].frameNumber;
    UINT32            applicationFrameNumIndex = applicationFrameNum % MaxOutstandingRequests;

    if (NULL != pResult->pOutputMetadata)
    {
        pChiOutputMetadata = m_pMetadataManager->GetMetadataFromHandle(pResult->pOutputMetadata);
        pSessionInfo->m_bufferQueue[internalFrameNumIndex].pMetadata[0] = pChiOutputMetadata;
        pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[0] |= META_READY_FLAG;
    }

    if (0 < pResult->numOutputBuffers)
    {
        ReleaseReferenceToInputBuffers(pResult->pPrivData);
        ChxUtils::Memcpy(&(pSessionInfo->m_bufferQueue[internalFrameNumIndex].buffer[0]),
            pResult->pOutputBuffers, sizeof(CHISTREAMBUFFER));
        pSessionInfo->m_bufferQueue[internalFrameNumIndex].buffer[0].pStream =
            m_multiCamResource[m_maxJpegCameraId].m_pFusionStream;
        pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[0] |= BUF_READY_FLAG;
    }

    if ((pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[0] & BUF_READY_FLAG) &&
        (pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[0] & META_READY_FLAG))
    {
        CHISTREAMBUFFER   snapshotBuffer    = { 0 };

        ChxUtils::PopulateHALToChiStreamBuffer(&m_appSnapshotBuffer[applicationFrameNumIndex], &snapshotBuffer);

        CHX_LOG_INFO("Fusion result is coming, send to jpeg processing! internalFrameNumber:%d, appFrameNumber:%d!",
            pResult->frameworkFrameNum, applicationFrameNum);

        SubmitJpegRequest(pResult->frameworkFrameNum,
                          &(pSessionInfo->m_bufferQueue[internalFrameNumIndex].buffer[0]),
                          &snapshotBuffer,
                          pSessionInfo->m_bufferQueue[internalFrameNumIndex].pMetadata[0],
                          m_multiCamResource[0].m_pFusionSnapshotBufferManager);

        pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[0] = FALSE;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::OfflineJPEGResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::OfflineJPEGResult(
    ChiCaptureResult* pResult)
{
    UINT32 pipelineIndex;
    UINT32 internalFrameNum          = static_cast<UINT32>(
                                        m_jpegRequestId[pResult->frameworkFrameNum%MaxOutstandingRequests].requestId);
    UINT32 internalFrameNumIndex     = internalFrameNum %  MaxOutstandingRequests;
    UINT32 applicationFrameNum       = m_requestMapInfo[internalFrameNumIndex].frameNumber;
    UINT32 applicationFrameNumIndex  = applicationFrameNum % MaxOutstandingRequests;

    ChiMetadata* pChiOutputMetadata = NULL;

    if (NULL == pResult->pPrivData)
    {
        CHX_LOG_ERROR("Result: Cannot Accept NULL private data here for = %d", pResult->frameworkFrameNum);
        // Early return
        return;
    }
    else
    {
        pipelineIndex = pResult->pPrivData->streamIndex;
    }

    camera3_capture_result_t *pAppResult   = GetCaptureResult(applicationFrameNumIndex);

    if ((NULL != pResult->pOutputMetadata) &&
        (TRUE == m_requestMapInfo[internalFrameNumIndex].isMetaReturnNeeded) &&
        (FALSE == IsMetadataSent(applicationFrameNumIndex)))
    {
        pAppResult->frame_number   = applicationFrameNum;
        pAppResult->partial_result = pResult->numPartialMetadata;

        pChiOutputMetadata = m_pMetadataManager->GetMetadataFromHandle(pResult->pOutputMetadata);
        camera_metadata_t* pFrameworkOutput = m_pMetadataManager->GetAndroidFrameworkOutputMetadata();

        m_pDeferSnapMetadataMutex->Lock();
        UINT64             timestamp = m_shutterTimestamp[applicationFrameNumIndex];

        if (0 != timestamp)
        {
            m_pDeferSnapMetadataMutex->Unlock();
            ChxUtils::UpdateTimeStamp(pChiOutputMetadata, timestamp, applicationFrameNum);
            SetMetadataAvailable(applicationFrameNumIndex);

            if (NULL != pFrameworkOutput)
            {
                pChiOutputMetadata->TranslateToCameraMetadata(pFrameworkOutput);
                pAppResult->result = pFrameworkOutput;
                ProcessAndReturnFinishedResults();
            }
        }
        else
        {
            if (NULL != pFrameworkOutput)
            {
                pChiOutputMetadata->TranslateToCameraMetadata(pFrameworkOutput);
                pAppResult->result = pFrameworkOutput;
                CHX_LOG_WARN("Warning: timestamp is 0, update it when frame:%u is coming", applicationFrameNum);
            }
            m_pDeferSnapMetadataMutex->Unlock();
        }
    }

    // release output metadata
    if (NULL != pResult->pOutputMetadata)
    {
        pChiOutputMetadata = m_pMetadataManager->GetMetadataFromHandle(pResult->pOutputMetadata);
        m_pMetadataManager->Release(pChiOutputMetadata);
    }

    if (pResult->numOutputBuffers != 0)
    {
        ReleaseReferenceToInputBuffers(pResult->pPrivData);

        pAppResult->frame_number               = applicationFrameNum;
        camera3_stream_buffer_t* pResultBuffer = NULL;
        CHISTREAMBUFFER *outputBuffer          = NULL;

        m_pAppResultMutex->Lock();
        for (UINT32 j = 0; j < pResult->numOutputBuffers; j++)
        {
            pResultBuffer         =
                const_cast<camera3_stream_buffer_t*>(&pAppResult->output_buffers[pAppResult->num_output_buffers++]);

            outputBuffer          = const_cast<CHISTREAMBUFFER *>(&pResult->pOutputBuffers[j]);
            outputBuffer->pStream = m_jpegRequestId[pResult->frameworkFrameNum%MaxOutstandingRequests].stream;

            ChxUtils::PopulateChiToHALStreamBuffer(&pResult->pOutputBuffers[j], pResultBuffer);
        }
        m_pAppResultMutex->Unlock();

        ProcessAndReturnFinishedResults();
        CHX_LOG_INFO("Snapshot Callback: Send to application:%d",internalFrameNum);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::OfflinePreviewResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::OfflinePreviewResult(
    ChiCaptureResult* pResult)
{
    UINT32          pipelineIndex;
    UINT32          internalFrameNum          = pResult->frameworkFrameNum;
    UINT32          internalFrameNumIndex     = internalFrameNum %  MaxOutstandingRequests;
    UINT32          applicationFrameNum       = m_requestMapInfo[internalFrameNumIndex].frameNumber;
    UINT32          applicationFrameNumIndex  = applicationFrameNum % MaxOutstandingRequests;
    ChiMetadata*    pChiOutputMetadata        = NULL;

    if (NULL == pResult->pPrivData)
    {
        CHX_LOG_ERROR("Result: Cannot Accept NULL private data here for = %d", pResult->frameworkFrameNum);
        // Early return
        return;
    }
    else
    {
        pipelineIndex = pResult->pPrivData->streamIndex;
    }
    camera3_capture_result_t *pAppResult   = GetCaptureResult(applicationFrameNumIndex);

    if (NULL != pResult->pOutputMetadata)
    {
        pChiOutputMetadata = m_pMetadataManager->GetMetadataFromHandle(pResult->pOutputMetadata);

        // Process the result metadata with controller
        m_pMultiCamController->ProcessResultMetadata(pChiOutputMetadata);

        if ((TRUE == m_requestMapInfo[internalFrameNumIndex].isMetaReturnNeeded) &&
            (FALSE == m_requestMapInfo[internalFrameNumIndex].isSnapshotReturnNeeded))
        {
            pAppResult->frame_number   = applicationFrameNum;
            pAppResult->partial_result = pResult->numPartialMetadata;

            // update timestamp
            UINT64             timestamp = m_shutterTimestamp[applicationFrameNum % MaxOutstandingRequests];
            ChxUtils::UpdateTimeStamp(pChiOutputMetadata, timestamp, applicationFrameNum);

            camera_metadata_t* pFrameworkOutput = m_pMetadataManager->GetAndroidFrameworkOutputMetadata();
            if (NULL != pFrameworkOutput)
            {
                pChiOutputMetadata->TranslateToCameraMetadata(pFrameworkOutput);
                m_pMultiCamController->TranslateResultMetadata(pFrameworkOutput);

                pAppResult->result = pFrameworkOutput;

                SetMetadataAvailable(applicationFrameNumIndex);

                ProcessAndReturnFinishedResults();
            }
            else
            {
                CHX_LOG_ERROR("[CMB_ERROR] cannot get f/w output for frame %u ",applicationFrameNumIndex);
            }
        }

        m_pMetadataManager->Release(pChiOutputMetadata);
    }

    if (pResult->numOutputBuffers != 0)
    {
        ReleaseReferenceToInputBuffers(pResult->pPrivData);

        pAppResult->frame_number               = applicationFrameNum;
        camera3_stream_buffer_t* pResultBuffer = NULL;

        m_pAppResultMutex->Lock();
        for (UINT32 j = 0; j < pResult->numOutputBuffers; j++)
        {
            pResultBuffer =
                const_cast<camera3_stream_buffer_t*>(&pAppResult->output_buffers[pAppResult->num_output_buffers++]);

            ChxUtils::PopulateChiToHALStreamBuffer(&pResult->pOutputBuffers[j], pResultBuffer);

            if ((m_pTargetPreviewStream == reinterpret_cast<CHISTREAM*>(pResultBuffer->stream)) &&
                (TRUE == m_requestMapInfo[internalFrameNumIndex].isSkipPreview))
            {
                m_requestMapInfo[internalFrameNumIndex].isSkipPreview = FALSE;

                CHX_LOG("skip the frame for display! internalFrameNum:%d, AppFrameNum:%d",
                    internalFrameNum, applicationFrameNum);
                ChxUtils::SkipPreviewFrame(pResultBuffer);
            }
        }
        m_pAppResultMutex->Unlock();

        ProcessAndReturnFinishedResults();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::OfflineRAW16Result
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::OfflineRAW16Result(
    ChiCaptureResult* pResult)
{
    UINT32 internalFrameNum          = pResult->frameworkFrameNum;
    UINT32 internalFrameNumIndex     = internalFrameNum %  MaxOutstandingRequests;
    UINT32 applicationFrameNum       = m_requestMapInfo[internalFrameNumIndex].frameNumber;
    UINT32 applicationFrameNumIndex  = applicationFrameNum % MaxOutstandingRequests;
    if (NULL == pResult->pPrivData)
    {
        CHX_LOG_ERROR("Result: Cannot Accept NULL private data here for = %d", pResult->frameworkFrameNum);
        // Early return
        return;
    }

    if ((pResult->numOutputBuffers != 0) && (m_pTargetRAW16Stream == pResult->pOutputBuffers[0].pStream))
    {
        ReleaseReferenceToInputBuffers(pResult->pPrivData);

        CHX_LOG("RAW16 callback send to application FrameNum %d", applicationFrameNum);
        camera3_capture_result_t *pAppResult   = GetCaptureResult(applicationFrameNumIndex);
        pAppResult->frame_number               = applicationFrameNum;
        camera3_stream_buffer_t* pResultBuffer = NULL;

        m_pAppResultMutex->Lock();
        for (UINT32 j = 0; j < pResult->numOutputBuffers; j++)
        {
            pResultBuffer =
                const_cast<camera3_stream_buffer_t*>(&pAppResult->output_buffers[pAppResult->num_output_buffers++]);

            ChxUtils::PopulateChiToHALStreamBuffer(&pResult->pOutputBuffers[j], pResultBuffer);
        }
        m_pAppResultMutex->Unlock();

        ProcessAndReturnFinishedResults();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::ProcessNotifyMessage
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::ProcessRealTimeNotifyMessage(
    const ChiMessageDescriptor* pMessageDescriptor)
{
    UINT32 pipelineIndex         = pMessageDescriptor->pPrivData->requestIndex;
    UINT32 internalFrameNum      = pMessageDescriptor->message.shutterMessage.frameworkFrameNum;
    UINT32 internalFrameNumIndex = internalFrameNum %  MaxOutstandingRequests;

    // Real time shutter notification to framework based on master pipeline index
    if (pipelineIndex == m_requestInfo[internalFrameNumIndex].masterPipelineIndex &&
        (TRUE == m_requestMapInfo[internalFrameNumIndex].isShutterReturnNeeded))
    {
        m_requestMapInfo[internalFrameNumIndex].isShutterReturnNeeded = FALSE;
        UINT32 applicationFrameNum      = m_requestMapInfo[internalFrameNumIndex].frameNumber;
        UINT32 applicationFrameNumIndex = applicationFrameNum % MaxOutstandingRequests;
        ChiMessageDescriptor* pTempMessage = const_cast<ChiMessageDescriptor*>(pMessageDescriptor);
        pTempMessage->message.shutterMessage.frameworkFrameNum = applicationFrameNum;

        camera3_notify_msg_t message;
        message.type                         = pMessageDescriptor->messageType;
        message.message.shutter.frame_number = applicationFrameNum;
        message.message.shutter.timestamp    = pMessageDescriptor->message.shutterMessage.timestamp;

        Usecase::ReturnFrameworkMessage((camera3_notify_msg_t*)pMessageDescriptor, m_cameraId);

        m_pDeferSnapMetadataMutex->Lock();
        m_shutterTimestamp[applicationFrameNumIndex] = pMessageDescriptor->message.shutterMessage.timestamp;
        camera3_capture_result_t *pAppResult         = GetCaptureResult(applicationFrameNumIndex);
        BOOL bNeedUpdateMetadata                     = ((FALSE == IsMetadataSent(applicationFrameNumIndex)) &&
                                                       (NULL != pAppResult->result));
        m_pDeferSnapMetadataMutex->Unlock();

        if (TRUE == bNeedUpdateMetadata)
        {
            UINT32                  SensorTimestampTag = 0x000E0010;
            camera_metadata_entry_t entry              = { 0 };

            camera_metadata_t* pMetadata = const_cast<camera_metadata_t*>(pAppResult->result);
            UINT64             timestamp = m_shutterTimestamp[applicationFrameNumIndex];

            INT32 status = find_camera_metadata_entry(pMetadata, SensorTimestampTag, &entry);

            if (-ENOENT == status)
            {
                status = add_camera_metadata_entry(pMetadata, SensorTimestampTag, &timestamp, 1);
            }
            else if (0 == status)
            {
                status = update_camera_metadata_entry(pMetadata, entry.index, &timestamp, 1, NULL);
            }
            SetMetadataAvailable(applicationFrameNumIndex);

            ProcessAndReturnFinishedResults();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::SessionCbNotifyMessage
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::SessionCbNotifyMessage(
    const ChiMessageDescriptor* pMessageDescriptor,
    VOID*                       pPrivateCallbackData)
{
    SessionPrivateData* pSessionPrivateData = static_cast<SessionPrivateData*>(pPrivateCallbackData);
    UsecaseMultiCamera* pUsecase            = static_cast<UsecaseMultiCamera*>(pSessionPrivateData->pUsecase);

    if ((NULL == pMessageDescriptor) ||
        (ChiMessageTypeSof == pMessageDescriptor->messageType) ||
        (ChiMessageTypeMetaBufferDone == pMessageDescriptor->messageType) ||
        (NULL == pMessageDescriptor->pPrivData))
    {
        // Sof notifications are not sent to the HAL3 application and so ignore
        if ((NULL == pMessageDescriptor) ||
            ((ChiMessageTypeSof != pMessageDescriptor->messageType) &&
             (ChiMessageTypeMetaBufferDone != pMessageDescriptor->messageType) &&
             (NULL == pMessageDescriptor->pPrivData)))
        {
            CHX_LOG_ERROR("Result: Cannot Accept NULL private data for Notify");
        }
        else if (ChiMessageTypeSof == pMessageDescriptor->messageType)
        {
            CHX_LOG("Chi Notify SOF frameNum %u framework frameNum %u, timestamp %" PRIu64,
                pMessageDescriptor->message.sofMessage.sofId,
                pMessageDescriptor->message.sofMessage.frameworkFrameNum,
                pMessageDescriptor->message.sofMessage.timestamp);
        }

        return;
    }

    UINT32 internalFrameNum      = pMessageDescriptor->message.shutterMessage.frameworkFrameNum;

    CHX_LOG("Notify Session: %d Pipeline: %d frameNum: %d Type: %d Timestamp: %" PRIu64,
        pSessionPrivateData->sessionId,
        pMessageDescriptor->pPrivData->streamIndex,
        internalFrameNum,
        pMessageDescriptor->messageType, pMessageDescriptor->message.shutterMessage.timestamp);

    if ((ChiMessageTypeError == pMessageDescriptor->messageType) ||
        (ChiMessageTypeTriggerRecovery == pMessageDescriptor->messageType))
    {
        RequestMapInfo requestMapInfo = pUsecase->GetRequestMapInfo(internalFrameNum);
        ChiMessageDescriptor* pTempMessage = const_cast<ChiMessageDescriptor*>(pMessageDescriptor);
        pTempMessage->message.shutterMessage.frameworkFrameNum = requestMapInfo.frameNumber;
        CHX_LOG("InternalFrameNuber = %d, AppFrameNumber %d", internalFrameNum, requestMapInfo.frameNumber);
        pUsecase->ProcessErrorMessage(pMessageDescriptor);
    }
    else
    {
        // Send only the realtime pipeline 0 shutter message
        if (pSessionPrivateData->sessionId == REALTIME_SESSION)
        {
            pUsecase->ProcessRealTimeNotifyMessage(pMessageDescriptor);
        }
    }

    return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::SessionCbCaptureResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::SessionCbCaptureResult(
    ChiCaptureResult* pResult,
    VOID*             pPrivateCallbackData)
{
    SessionPrivateData* pSessionPrivateData = static_cast<SessionPrivateData*>(pPrivateCallbackData);
    UsecaseMultiCamera* pUsecase            = static_cast<UsecaseMultiCamera*>(pSessionPrivateData->pUsecase);
    BOOL                isDebugDataEnable   = FALSE;
    CHAR*               pData               = NULL;
    DebugData*          pDebug              = NULL;
    ChiMetadata*        pChiOutputMetadata      = NULL;

    UINT32 pipelineIndex;

    if (NULL == pResult->pPrivData)
    {
        CHX_LOG_ERROR("Result: Cannot Accept NULL private data here for = %d", pResult->frameworkFrameNum);
        // Early return
        return;
    }
    else
    {
        pipelineIndex = pResult->pPrivData->streamIndex;
    }

    if(FlushStatus::NotFlushing != pUsecase->GetFlushStatus())
    {
        //Don't process flushed results.
        CHX_LOG_INFO("Flush is in progress = %d", pSessionPrivateData->sessionId);
        if (0 != pResult->numOutputBuffers)
        {
            UINT32      pipelineIndex      = pResult->pPrivData->requestIndex;
            SessionInfo *pSessionInfo      = &pUsecase->m_sessionInfo[pSessionPrivateData->sessionId];

            pSessionInfo->m_waitUntilDrain[pipelineIndex].m_pSyncMutex->Lock();
            if (0 < pSessionInfo->m_waitUntilDrain[pipelineIndex].m_syncCounter)
            {
                pSessionInfo->m_waitUntilDrain[pipelineIndex].m_syncCounter -= pResult->numOutputBuffers;
            }
            pSessionInfo->m_waitUntilDrain[pipelineIndex].m_pSyncMutex->Unlock();

            pUsecase->SignalResult(pSessionPrivateData->sessionId, pipelineIndex);
        }

        return;
    }

    if (NULL != pResult->pInputMetadata)
    {
        ChiMetadata* pInputMeta = pUsecase->m_pMetadataManager->GetMetadataFromHandle(pResult->pInputMetadata);

        // if this metadata is created locally, do invalidate
        if (ChiMetadataManager::InvalidClientId == pInputMeta->GetClientId())
        {
            pInputMeta->Invalidate();
        }
        else
        {
            pUsecase->m_pMetadataManager->Release(pInputMeta);
        }
    }

    if ((1 <= ExtensionModule::GetInstance()->EnableDumpDebugData()) &&
       (NULL != pResult->pOutputMetadata))
    {
        pChiOutputMetadata = pUsecase->m_pMetadataManager->GetMetadataFromHandle(pResult->pOutputMetadata);
        isDebugDataEnable = TRUE;
        if (TRUE == ChxUtils::IsVendorTagPresent(pChiOutputMetadata, DebugDataTag))
        {
            ChxUtils::GetVendorTagValue(pChiOutputMetadata, DebugDataTag, (VOID**)&pData);
            pDebug = reinterpret_cast<DebugData*>(pData);
        }
    }

    CHX_LOG_INFO("Result: Session = %d Pipeline = %d FrameNum: %d pDbgData: %p buffer - type %d handle %p, status - %d "
            "numbuf = %d partial = %d input = %p",
            pSessionPrivateData->sessionId,
            pipelineIndex,
            pResult->frameworkFrameNum,
            pData != NULL ? pDebug->pData : NULL,
            pResult->pOutputBuffers[0].bufferInfo.bufferType,
            pResult->pOutputBuffers[0].bufferInfo.phBuffer,
            pResult->pOutputBuffers[0].bufferStatus,
            pResult->numOutputBuffers,
            pResult->numPartialMetadata,
            pResult->pInputBuffer);

    if (0 != pResult->numOutputBuffers)
    {
        UINT32      pipelineIndex      = pResult->pPrivData->requestIndex;
        SessionInfo *pSessionInfo      = &pUsecase->m_sessionInfo[pSessionPrivateData->sessionId];

        pSessionInfo->m_waitUntilDrain[pipelineIndex].m_pSyncMutex->Lock();
        if (0 < pSessionInfo->m_waitUntilDrain[pipelineIndex].m_syncCounter)
        {
            pSessionInfo->m_waitUntilDrain[pipelineIndex].m_syncCounter -= pResult->numOutputBuffers;
        }
        pSessionInfo->m_waitUntilDrain[pipelineIndex].m_pSyncMutex->Unlock();

        pUsecase->SignalResult(pSessionPrivateData->sessionId, pipelineIndex);
    }

    if (REALTIME_SESSION == pSessionPrivateData->sessionId)
    {
        pUsecase->RealtimeCaptureResult(pResult);
    }
    else if (OFFLINE_YUV_SESSION == pSessionPrivateData->sessionId)
    {
        if (TRUE == isDebugDataEnable)
        {
            pUsecase->SaveDebugData(pResult);
        }

        pUsecase->OfflinePreviewResult(pResult);
    }
    else if ((OFFLINE_RDI_SESSION0 == pSessionPrivateData->sessionId) ||
             (OFFLINE_RDI_SESSION1 == pSessionPrivateData->sessionId) ||
             (OFFLINE_RDI_SESSION2 == pSessionPrivateData->sessionId))
    {
        if (TRUE == isDebugDataEnable)
        {
            // Process debug-data only for offline processing
            pUsecase->ProcessDebugData(pResult, pPrivateCallbackData, pResult->frameworkFrameNum);
        }

        pUsecase->OfflineSnapshotResult(pResult, pSessionPrivateData->sessionId);
    }
    else if (OFFLINE_FUSION_SESSION == pSessionPrivateData->sessionId)
    {
        pUsecase->OfflineFusionSnapshotResult(pResult);
    }
    else if (OFFLINE_JPEG_SESSION == pSessionPrivateData->sessionId)
    {
        pUsecase->OfflineJPEGResult(pResult);
    }
    else if (OFFLINE_RAW16_SESSION == pSessionPrivateData->sessionId)
    {
        pUsecase->OfflineRAW16Result(pResult);
    }
    else
    {
        CHX_LOG_ERROR("Unknown session ID %d ", pSessionPrivateData->sessionId);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::SessionCbCaptureResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::SessionCbPartialCaptureResult(
    ChiPartialCaptureResult* pPartialCaptureResult,
    VOID*                    pPrivateCallbackData)
{
    SessionPrivateData* pSessionPrivateData = static_cast<SessionPrivateData*>(pPrivateCallbackData);
    UsecaseMultiCamera* pUsecase            = static_cast<UsecaseMultiCamera*>(pSessionPrivateData->pUsecase);

    UINT32 pipelineIndex;

    if (NULL == pPartialCaptureResult->pPrivData)
    {
        CHX_LOG_ERROR("Result: Cannot Accept NULL private data here for = %d", pPartialCaptureResult->frameworkFrameNum);
        // Early return
        return;
    }
    else
    {
        pipelineIndex = pPartialCaptureResult->pPrivData->streamIndex;
    }

    if (REALTIME_SESSION == pSessionPrivateData->sessionId)
    {
        pUsecase->ProcessDriverPartialCaptureResult(pPartialCaptureResult, pSessionPrivateData->sessionId);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::DeferSnapSessionThread
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID* UsecaseMultiCamera::DeferSnapSessionThread(VOID * pThreadData)
{
    PerThreadData*      pPerThreadData      = reinterpret_cast<PerThreadData*>(pThreadData);
    UsecaseMultiCamera* pDualCamZoomUsecase = reinterpret_cast<UsecaseMultiCamera*>(pPerThreadData->pPrivateData);

    pDualCamZoomUsecase->DeferSnapSession();

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::StartDeferThread
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::StartDeferThread()
{
    CDKResult result = CDKResultSuccess;
    m_pDeferSnapDoneMutex->Lock();
    m_deferSnapshotThreadCreateDone = FALSE;
    m_pDeferSnapDoneMutex->Unlock();

    /// it need to check if defer snapshot session is created done.
    if ((FALSE == m_deferSnapshotSessionDone) &&
            (NULL == m_deferSnapSessionThreadData.pPrivateData))
    {
        m_deferSnapSessionThreadData.pPrivateData = this;

        result = ChxUtils::ThreadCreate(UsecaseMultiCamera::DeferSnapSessionThread, &m_deferSnapSessionThreadData,
            &m_deferSnapSessionThreadData.hThreadHandle);

        if (CDKResultSuccess != result)
        {
            m_deferSnapSessionThreadData.pPrivateData  = NULL;
            m_deferSnapSessionThreadData.hThreadHandle = INVALID_THREAD_HANDLE;
            CHX_LOG_ERROR("Create defer thread failed!");
        }
        else
        {
            m_pDeferSnapDoneMutex->Lock();
            m_deferSnapshotThreadCreateDone = TRUE;
            m_pDeferSnapDoneMutex->Unlock();
        }
    }
    else
    {
        CHX_LOG_WARN("Warning:snapshot related session has been created!");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::InitializeDeferResouce
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::InitializeDeferResource()
{
    m_pDeferSnapDoneMutex                       = Mutex::Create();
    m_pDeferSnapDoneCondition                   = Condition::Create();
    m_pDeferSyncMutex                           = Mutex::Create();
    m_pOfflinePreviewSessionCreateDoneCondition = Condition::Create();
    m_pOfflinePreviewSessionCreateDoneMutex     = Mutex::Create();
    m_deferSnapshotSessionDone                  = FALSE;
    m_deferSnapshotThreadCreateDone             = FALSE;
    m_offlinePreviewSessionCreateDone           = FALSE;
    m_deferedCameraID                           = INVALID_CAMERAID;

    ChxUtils::Memset(&m_deferSnapSessionThreadData, 0, sizeof(m_deferSnapSessionThreadData));
    m_deferSnapSessionThreadData.hThreadHandle = INVALID_THREAD_HANDLE;

    for (UINT i = OFFLINE_YUV_SESSION; i < MAX_MULTI_CAMERA_SESSION; i++)
    {
        m_sessionInfo[i].m_pSession = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::DestroyDeferResource
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::DestroyDeferResource()
{
    CDKResult result = CDKResultSuccess;

    result = WaitForDeferSnapThread();

    for (UINT sessionIdx = 0 ; sessionIdx < MaxSessions; sessionIdx++)
    {
        if (NULL != m_sessionInfo[sessionIdx].m_pSession)
        {
            for (UINT32 pipelineIndex = 0; pipelineIndex < m_sessionInfo[sessionIdx].m_numPipelines;
                pipelineIndex++)
            {
                m_sessionInfo[sessionIdx].m_waitUntilDrain[pipelineIndex].m_pSyncMutex->Lock();
                m_sessionInfo[sessionIdx].m_waitUntilDrain[pipelineIndex].m_syncCounter = 0;
                m_sessionInfo[sessionIdx].m_waitUntilDrain[pipelineIndex].m_pSyncMutex->Unlock();
                SignalResult(sessionIdx, pipelineIndex);
            }
        }
    }

    // terminate snapshot deferred thread.
    if ((INVALID_THREAD_HANDLE != m_deferSnapSessionThreadData.hThreadHandle) &&
        (TRUE == m_deferSnapshotThreadCreateDone) &&
        (CDKResultSuccess == result ))
    {
        ChxUtils::ThreadTerminate(m_deferSnapSessionThreadData.hThreadHandle);
        m_deferSnapSessionThreadData.hThreadHandle = INVALID_THREAD_HANDLE;
    }

    if (NULL != m_pDeferSnapDoneMutex)
    {
        m_pDeferSnapDoneMutex->Destroy();
        m_pDeferSnapDoneMutex = NULL;
    }

    if (NULL != m_pDeferSnapDoneCondition)
    {
        m_pDeferSnapDoneCondition->Destroy();
        m_pDeferSnapDoneCondition = NULL;
    }

    if (NULL != m_pDeferSyncMutex)
    {
        m_pDeferSyncMutex->Destroy();
        m_pDeferSyncMutex = NULL;
    }

    if (NULL != m_pOfflinePreviewSessionCreateDoneCondition)
    {
        m_pOfflinePreviewSessionCreateDoneCondition->Destroy();
        m_pOfflinePreviewSessionCreateDoneCondition = NULL;
    }

    if (NULL != m_pOfflinePreviewSessionCreateDoneMutex)
    {
        m_pOfflinePreviewSessionCreateDoneMutex->Destroy();
        m_pOfflinePreviewSessionCreateDoneMutex = NULL;
    }


    m_deferSnapshotSessionDone        = FALSE;
    m_deferSnapshotThreadCreateDone   = FALSE;
    m_offlinePreviewSessionCreateDone = FALSE;

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::WaitForDeferThread
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::WaitForDeferSnapThread()
{
    CDKResult result = CDKResultSuccess;

    if(TRUE == m_deferSnapshotThreadCreateDone)
    {
        m_pDeferSnapDoneMutex->Lock();
        if ((FALSE == m_deferSnapshotSessionDone))
        {
            result = m_pDeferSnapDoneCondition->TimedWait(m_pDeferSnapDoneMutex->GetNativeHandle(), 1500);
            if(CDKResultSuccess != result)
            {
                CHX_LOG_ERROR("WaitForDeferSnapThread timed out!");
            }
        }
        m_pDeferSnapDoneMutex->Unlock();
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::DeferSnapSession
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::DeferSnapSession()
{
    ATRACE_BEGIN("DeferSnapSession");

    CHX_LOG_INFO("DeferSnapSession:enter");

    CDKResult result = CDKResultSuccess;

    m_pOfflinePreviewSessionCreateDoneMutex->Lock();

    result = CreateSession(OFFLINE_YUV_SESSION, m_sessionInfo[OFFLINE_YUV_SESSION].m_numPipelines,
        m_sessionInfo[OFFLINE_YUV_SESSION].m_pPipeline);
    if (CDKResultSuccess == result)
    {
        // Activate offlinepreview pipeline to speed up first preview result to application
        if (FALSE == m_sessionInfo[OFFLINE_YUV_SESSION].m_pSession->IsPipelineActive())
        {
            result = ExtensionModule::GetInstance()->ActivatePipeline(
                m_sessionInfo[OFFLINE_YUV_SESSION].m_pSession->GetSessionHandle(),
                m_sessionInfo[OFFLINE_YUV_SESSION].m_pSession->GetPipelineHandle());
        }

        m_pOfflinePreviewSessionCreateDoneCondition->Signal();
        m_offlinePreviewSessionCreateDone = TRUE;
    }
    else
    {
        CHX_LOG_ERROR("Create offline preview session failed!");
    }

    m_pOfflinePreviewSessionCreateDoneMutex->Unlock();

    if ((CDKResultSuccess == result) && (TRUE == m_isOfflineRequired))
    {
        m_pDeferSnapDoneMutex->Lock();

        // Create snapshot related offline session
        result = CreateOfflineSession();

        if (CDKResultSuccess == result)
        {
            // Allocate buffer for snapshot related session
            CHAR bufferManagerName[MaxStringLength64];
            for (UINT i = 0; i < m_numOfPhysicalDevices; i++)
            {
                MultiCameraResource* pObj = &m_multiCamResource[i];
                CHIBufferManagerCreateData  bufferCreateData;
                memset(&bufferCreateData, 0, sizeof(bufferCreateData));

                CdkUtils::SNPrintF(bufferManagerName, sizeof(bufferManagerName), "MCYUVBufferManager_%d", i);

                bufferCreateData.format                  = m_pTargetPreviewStream->format;
                bufferCreateData.width                   = pObj->m_pYUVOutputSnapshotStream->width;
                bufferCreateData.height                  = pObj->m_pYUVOutputSnapshotStream->height;
                bufferCreateData.consumerFlags           = ChiGralloc1ConsumerUsageCamera;
                bufferCreateData.producerFlags           = ChiGralloc1ProducerUsageCamera;
                bufferCreateData.bEnableLateBinding      = ExtensionModule::GetInstance()->EnableCHILateBinding();
                bufferCreateData.bufferHeap              = BufferHeapDefault;
                bufferCreateData.pChiStream              = pObj->m_pYUVOutputSnapshotStream;
                // maxBufferCount is increased from 2 to 5 because when OEMs add chinodes to the offline pipeline
                // 2 buffers are insufficient during burst snapshot as the pipeline takes longer to process
                // RTB will not support burst.
                if (UsecaseSAT == m_usecaseMode)
                {
                    bufferCreateData.maxBufferCount      = 5;
                }
                else
                {
                    bufferCreateData.maxBufferCount      = 2;
                }
                bufferCreateData.immediateBufferCount    = 1;

                pObj->m_pYUVSnapshotBufferManager       =
                    CHIBufferManager::Create(bufferManagerName, &bufferCreateData);

                // Allocate RAW16 buffers for SAT and RAW16 is needed
                if ((UsecaseSAT == m_usecaseMode) && (TRUE == m_isRaw16Needed))
                {
                    memset(&bufferCreateData, 0, sizeof(bufferCreateData));
                    // Allocate RAW16 buffers if RAW16 stream is present in configurestreams
                    bufferCreateData                         = {0};
                    bufferCreateData.format                  = HAL_PIXEL_FORMAT_RAW16;
                    bufferCreateData.width                   = pObj->m_RAWWidth;
                    bufferCreateData.height                  = pObj->m_RAWHeight;
                    bufferCreateData.consumerFlags           = ChiGralloc1ConsumerUsageCamera;
                    bufferCreateData.producerFlags           = ChiGralloc1ProducerUsageCamera;
                    bufferCreateData.maxBufferCount          = BufferQueueDepth;
                    bufferCreateData.immediateBufferCount    = CHIImmediateBufferCountZSL;
                    bufferCreateData.bEnableLateBinding      = ExtensionModule::GetInstance()->EnableCHILateBinding();
                    bufferCreateData.bufferHeap              = BufferHeapDefault;
                    bufferCreateData.pChiStream              = pObj->m_pRTOutputRAW16Stream;

                    CdkUtils::SNPrintF(bufferManagerName, sizeof(bufferManagerName), "MCRDI16BufferManager_%d", i);
                    pObj->m_pRAW16BufferManager = CHIBufferManager::Create(bufferManagerName, &bufferCreateData);
                }
            }

            CHIBufferManagerCreateData  bufferCreateData;
            memset(&bufferCreateData, 0, sizeof(bufferCreateData));
            bufferCreateData.format               = m_pTargetPreviewStream->format;
            bufferCreateData.width                = m_multiCamResource[m_maxJpegCameraId].m_pFusionStream->width;
            bufferCreateData.height               = m_multiCamResource[m_maxJpegCameraId].m_pFusionStream->height;
            bufferCreateData.consumerFlags        = ChiGralloc1ConsumerUsageCamera;
            bufferCreateData.producerFlags        = ChiGralloc1ProducerUsageCamera;
            bufferCreateData.maxBufferCount       = 2;
            bufferCreateData.immediateBufferCount = 1;
            bufferCreateData.bEnableLateBinding   = ExtensionModule::GetInstance()->EnableCHILateBinding();
            bufferCreateData.bufferHeap           = BufferHeapDefault;
            bufferCreateData.pChiStream           = m_multiCamResource[m_maxJpegCameraId].m_pFusionStream;

            m_multiCamResource[0].m_pFusionSnapshotBufferManager =
                CHIBufferManager::Create("MCFusionBufferManager", &bufferCreateData);

            m_deferSnapshotSessionDone = TRUE;
            m_pDeferSnapDoneMutex->Unlock();
            m_pDeferSnapDoneCondition->Signal();
        }
        else
        {
            m_pDeferSnapDoneMutex->Unlock();
            m_pDeferSnapDoneCondition->Signal();
            m_deferSnapshotSessionDone = FALSE;
            CHX_LOG_ERROR("Create offline pipeline failed!");
        }
    }
    else
    {
        m_pDeferSnapDoneMutex->Lock();
        m_deferSnapshotSessionDone = TRUE;
        m_pDeferSnapDoneMutex->Unlock();
        m_pDeferSnapDoneCondition->Signal();
    }
    ATRACE_END();
    CHX_LOG_INFO("DeferSnapSession:done");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::getPhysicalDeviceInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DeviceInfo *UsecaseMultiCamera::getPhysicalDeviceInfo(UINT32 cameraId)
{
    DeviceInfo *PhyDeviceInfo    = NULL;

    for (UINT32 i = 0; i < m_pLogicalCameraInfo->numPhysicalCameras; i++)
    {
        if (cameraId == m_pLogicalCameraInfo->ppDeviceInfo[i]->cameraId)
        {
            PhyDeviceInfo = m_pLogicalCameraInfo->ppDeviceInfo[i];
        }
    }

    return PhyDeviceInfo;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::SignalResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::SignalResult(
    UINT32 sessionIdx,
    UINT32 pipelineIndex)
{
    CDKResult     result       = CDKResultSuccess;
    SessionInfo  *pSessionInfo = &m_sessionInfo[sessionIdx];

    pSessionInfo->m_waitUntilDrain[pipelineIndex].m_pSyncMutex->Lock();
    if ((0 >= pSessionInfo->m_waitUntilDrain[pipelineIndex].m_syncCounter) &&
        (TRUE == pSessionInfo->m_waitUntilDrain[pipelineIndex].m_waitForSignal))
    {
        pSessionInfo->m_waitUntilDrain[pipelineIndex].m_pSyncCondition->Signal();
    }
    pSessionInfo->m_waitUntilDrain[pipelineIndex].m_pSyncMutex->Unlock();

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::WaitUntilResultDrain
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::WaitUntilResultDrain(
    UINT32 sessionIdx,
    UINT32 pipelineIndex)
{
    CDKResult     result       = CDKResultSuccess;
    SessionInfo*  pSession     = &m_sessionInfo[sessionIdx];

    if (NULL != pSession && pSession->m_numPipelines < pipelineIndex)
    {
        result = CDKResultEInvalidArg;
    }

    if (CDKResultSuccess == result)
    {
        INT32 pendingResult = 0;
        pSession->m_waitUntilDrain[pipelineIndex].m_pSyncMutex->Lock();
        pendingResult = pSession->m_waitUntilDrain[pipelineIndex].m_syncCounter;
        pSession->m_waitUntilDrain[pipelineIndex].m_pSyncMutex->Unlock();
        if (0 < pendingResult)
        {
            BOOL        waitUntilDrain    = FALSE;
            BOOL        enableFlush       = FALSE;
            UINT32      cameraId          = pSession->m_pSession->GetCameraId(pipelineIndex);
            DeviceInfo *PhyDeviceInfo     = getPhysicalDeviceInfo(cameraId);

            if (NULL != PhyDeviceInfo)
            {
                if (PhyDeviceInfo->pDeviceConfig->enableSmoothTransition == TRUE)
                {
                    /// Can Enable when we have flush impl
                    enableFlush               = FALSE;
                }
                waitUntilDrain            = TRUE;
            }

            if (TRUE == enableFlush)
            {
                CHX_LOG_INFO("Start flush!");
                ExtensionModule::GetInstance()->Flush(pSession->m_pSession->GetSessionHandle());
                CHX_LOG_INFO("Flush done!");
            }

            if (TRUE == waitUntilDrain)
            {
                pSession->m_waitUntilDrain[pipelineIndex].m_pSyncMutex->Lock();
                if (0 < pSession->m_waitUntilDrain[pipelineIndex].m_syncCounter)
                {
                    CHX_LOG_INFO("Waiting for %d results from session: %d, Pipeline = %d",
                        pSession->m_waitUntilDrain[pipelineIndex].m_syncCounter, sessionIdx, pipelineIndex);
                    pSession->m_waitUntilDrain[pipelineIndex].m_waitForSignal = TRUE;
                    result = pSession->m_waitUntilDrain[pipelineIndex].m_pSyncCondition->TimedWait(
                        pSession->m_waitUntilDrain[pipelineIndex].m_pSyncMutex->GetNativeHandle(), 3000);
                    if(CDKResultSuccess != result)
                    {
                        CHX_LOG_ERROR("WaitForDeferSnapThread timed out! session = %d  pipeline = %d",
                            sessionIdx, pipelineIndex);
                        ExtensionModule::GetInstance()->Flush(pSession->m_pSession->GetSessionHandle());
                    }
                    else
                    {
                        CHX_LOG_INFO("Received all result for session = %d  pipeline = %d",
                            sessionIdx, pipelineIndex);
                        pSession->m_waitUntilDrain[pipelineIndex].m_syncCounter    = 0;
                        pSession->m_waitUntilDrain[pipelineIndex].m_waitForSignal  = FALSE;
                    }
                }
                pSession->m_waitUntilDrain[pipelineIndex].m_pSyncMutex->Unlock();
            }
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::ActivateDeactivateRealtimePipeline
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::ActivateDeactivateRealtimePipeline(
    ControllerResult*           pMccResult,
    UINT64                      requestId,
    camera3_capture_request_t*  pRequest)
{
    CDKResult result = CDKResultSuccess;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// 1. check if flush is needed because of IFE resource limited
    /// 2. check which pipeline will be deactivated in the realtime session, and deactivate it
    /// 3. check which pipeline will be activated in the realtime session, and activate it
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // check if flush is needed. when two realtime pipeline are active, then need to active third realtime
    // pipeline, it needs to flush realtime pipeline request instantly to speed up switch performance
    UINT32 activeMap           = m_curActiveMap | pMccResult->activeMap;
    UINT32 resourceCount       = 0;

    while(activeMap)
    {
        resourceCount += (activeMap & 1);
        activeMap     >>= 1;
    }
    if (resourceCount > MAX_RESOURCE_AVAILABLE)
    {
        CHX_LOG_INFO("Resource Conflict. Wait for resource to available");
        ATRACE_BEGIN("Start flush");
        for (UINT32 pipelineIndex = 0; pipelineIndex < m_numOfPhysicalDevices; pipelineIndex++)
        {
            result = WaitUntilResultDrain(REALTIME_SESSION, pipelineIndex);
        }
        ATRACE_END();
    }

    if ((FALSE == IsUsecaseInBadState()) && (CDKResultSuccess == result))
    {
        //Deactive firstly
        for (UINT32 pipelineIndex = 0; pipelineIndex < m_numOfPhysicalDevices; pipelineIndex++)
        {
            if ((FALSE == pMccResult->activeCameras[pipelineIndex].isActive) &&
                (TRUE == m_sessionInfo[REALTIME_SESSION].m_pSession->IsPipelineActive(pipelineIndex)))
            {
                INT32 pendingResult = 0;
                m_sessionInfo[REALTIME_SESSION].m_waitUntilDrain[pipelineIndex].m_pSyncMutex->Lock();
                pendingResult = m_sessionInfo[REALTIME_SESSION].m_waitUntilDrain[pipelineIndex].m_syncCounter;
                m_sessionInfo[REALTIME_SESSION].m_waitUntilDrain[pipelineIndex].m_pSyncMutex->Unlock();

                if (0 >= pendingResult)
                {
                    UINT32 deactivateMode = CHIDeactivateModeRealTimeDevices;

                    CHX_LOG_INFO("Deactivating Pipeline %d deactivateMode = %d", pipelineIndex, deactivateMode);

                    result = ExtensionModule::GetInstance()->DeactivatePipeline(
                        m_sessionInfo[REALTIME_SESSION].m_pSession->GetSessionHandle(),
                        m_sessionInfo[REALTIME_SESSION].m_pSession->GetPipelineHandle(pipelineIndex),
                        deactivateMode);
                    if (CDKResultSuccess == result)
                    {
                        CHX_LOG_INFO("Deactivating success Pipeline %d", pipelineIndex);
                        m_sessionInfo[REALTIME_SESSION].m_pSession->SetPipelineDeactivate(pipelineIndex);
                        m_curActiveMap &= ~(1 << pipelineIndex);
                        m_multiCamResource[pipelineIndex].m_baseFrameNumber = INVALIDFRAMENUMBER;
                        FlushRDIQueue(requestId, pipelineIndex);
                    }
                    else
                    {
                        CHX_LOG_INFO("Deactivating Pipeline %d failed!", pipelineIndex);
                    }
                }
            }
        }

        //do active
        UINT numberOfPCRBeforeStreamOn = ExtensionModule::GetInstance()->GetNumPCRsBeforeStreamOn(
            const_cast<camera_metadata_t*>(pRequest->settings));

        m_pendingActiveMask = 0;
        for (UINT32 pipelineIndex = 0; pipelineIndex < m_numOfPhysicalDevices; pipelineIndex++)
        {
            if ((0 == (m_curActiveMap & (m_curActiveMap -1 ))) && (TRUE == pMccResult->activeCameras[pipelineIndex].isActive) &&
                (FALSE == m_sessionInfo[REALTIME_SESSION].m_pSession->IsPipelineActive(pipelineIndex)))
            {
                if (INVALIDFRAMENUMBER == m_multiCamResource[pipelineIndex].m_baseFrameNumber)
                {
                    m_multiCamResource[pipelineIndex].m_baseFrameNumber = pRequest->frame_number;
                }

                CHX_LOG_INFO("NumPCR:%d reqFrameno:%d base frame num:%d",
                    numberOfPCRBeforeStreamOn,
                    pRequest->frame_number,
                    m_multiCamResource[pipelineIndex].m_baseFrameNumber);
                if (numberOfPCRBeforeStreamOn == (pRequest->frame_number - m_multiCamResource[pipelineIndex].m_baseFrameNumber))
                {
                    // Activate pipeline if LPM decision is to activate by checking the current state
                    CHX_LOG_INFO("Activating Pipeline %d", pipelineIndex);

                    ATRACE_BEGIN("ActivatePipeline");
                    result = ExtensionModule::GetInstance()->ActivatePipeline(
                        m_sessionInfo[REALTIME_SESSION].m_pSession->GetSessionHandle(),
                        m_sessionInfo[REALTIME_SESSION].m_pSession->GetPipelineHandle(pipelineIndex));
                    ATRACE_END();

                    if (CDKResultSuccess == result)
                    {
                        CHX_LOG_INFO("Success activating for pipeline %d", pipelineIndex);

                        m_sessionInfo[REALTIME_SESSION].m_pSession->SetPipelineActivateFlag(pipelineIndex);
                        m_multiCamResource[pipelineIndex].m_stickyMetaNeeded = TRUE;
                        m_curActiveMap |= 1 << pipelineIndex;
                    }
                    else
                    {
                        CHX_LOG_INFO("Activating Pipeline %d failed", pipelineIndex);
                    }
                }
                else
                {
                    // only do it when enable early PCR
                    if (numberOfPCRBeforeStreamOn != 0)
                    {
                        m_pendingActiveMask |= 1 << pipelineIndex;
                    }
                }
            }
        }
    }
    else
    {
        CHX_LOG_ERROR("MC is in bad state");
        result = CDKResultEInvalidState;
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::Initialize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::CreateMultiController(
        LogicalCameraInfo*              pCameraInfo,
        camera3_stream_configuration_t* pStreamConfig)
{
    CDKResult result = CDKResultSuccess;

    // Get multi-cam controller object
    MccCreateData mccCreateData = { 0 };
    mccCreateData.primaryCameraId   = pCameraInfo->primaryCameraId;
    mccCreateData.numBundledCameras = pCameraInfo->numPhysicalCameras;

    CHX_ASSERT(MaxDevicePerLogicalCamera >= pCameraInfo->numPhysicalCameras);

    CamInfo camInfo[MaxDevicePerLogicalCamera];

    for (UINT32 i = 0; i < mccCreateData.numBundledCameras; ++i)
    {
        camInfo[i].camId         = pCameraInfo->ppDeviceInfo[i]->cameraId;
        camInfo[i].pChiCamInfo   = pCameraInfo->ppDeviceInfo[i]->m_pDeviceCaps;
        camInfo[i].pChiCamConfig = pCameraInfo->ppDeviceInfo[i]->pDeviceConfig;

        CHISENSORMODEINFO* selectedSensorMode = UsecaseSelector::GetSensorModeInfo(camInfo[i].camId, pStreamConfig, 1);

        camInfo[i].sensorOutDimension.width  = selectedSensorMode->frameDimension.width;
        camInfo[i].sensorOutDimension.height = selectedSensorMode->frameDimension.height;
        camInfo[i].horizontalBinning         = selectedSensorMode->horizontalBinning;
        camInfo[i].verticalBinning           = selectedSensorMode->verticalBinning;

        UsecaseSelector::CalculateFovRectIFE(&camInfo[i].fovRectIFE,
                                             selectedSensorMode->frameDimension,
                                             camInfo[i].pChiCamInfo->sensorCaps.activeArray);

        mccCreateData.pBundledCamInfo[i] = &camInfo[i];
    }

    StreamConfig streamConfig = { 0 };
    streamConfig.numStreams      = pStreamConfig->num_streams;
    streamConfig.pStreamInfo     =
        static_cast<StreamInfo*>(CHX_CALLOC(pStreamConfig->num_streams * sizeof(StreamInfo)));

    CHX_ASSERT(NULL != streamConfig.pStreamInfo);

    if (NULL != streamConfig.pStreamInfo)
    {
        for (UINT32 i = 0; i < streamConfig.numStreams; ++i)
        {
            streamConfig.pStreamInfo[i].streamType             = pStreamConfig->streams[i]->stream_type;
            streamConfig.pStreamInfo[i].usage                  = pStreamConfig->streams[i]->usage;
            streamConfig.pStreamInfo[i].streamDimension.width  = pStreamConfig->streams[i]->width;
            streamConfig.pStreamInfo[i].streamDimension.height = pStreamConfig->streams[i]->height;
        }

        mccCreateData.pStreamConfig      = &streamConfig;
        mccCreateData.logicalCameraId    = pCameraInfo->cameraId;
        mccCreateData.logicalCameraType  = pCameraInfo->logicalCameraType;

        m_pMultiCamController = MultiCamControllerManager::GetInstance()->GetController(&mccCreateData);

        if (NULL != m_pMultiCamController)
        {
            // Allocate input metadata buffer for the offline pipeline for preview
            for (UINT32 index = 0; index < MAX_EMPTY_RTMETA_COUNT; ++index)
            {
                m_pOfflinePipelineInputMetadataPreview[index] = ChiMetadata::Create(NULL, 0, true);
                if(NULL == m_pOfflinePipelineInputMetadataPreview[index])
                {
                    CHX_LOG_ERROR("ChiMetadata::Create fails for preview meta data");
                    result = CDKResultEFailed;
                }
            }

            // Allocate input metadata buffer for the offline pipeline for snapshot
            for (UINT32 index = 0; index < MAX_EMPTY_OFFLINEMETA_COUNT; ++index)
            {
                m_pOfflinePipelineInputMetadataSnapshot[index] = ChiMetadata::Create(NULL, 0, true);
                if(NULL == m_pOfflinePipelineInputMetadataSnapshot[index])
                {
                    CHX_LOG_ERROR("ChiMetadata::Create fails for snapshot meta data");
                    result = CDKResultEFailed;
                }
            }

            // Allocate input metadata buffer for the offline pipeline for RAW16
            m_pOfflinePipelineInputMetadataRAW16 = ChiMetadata::Create(NULL, 0, true);
            if (NULL == m_pOfflinePipelineInputMetadataRAW16)
            {
                CHX_LOG_ERROR("allocate_camera_metadata fails for RAW16 meta data");
                result = CDKResultEFailed;
            }

        }
        else
        {
            result = CDKResultEFailed;
        }

        CHX_FREE(streamConfig.pStreamInfo);
        streamConfig.pStreamInfo = NULL;
    }
    else
    {
        CHX_LOG_ERROR("Allocate stream configure buffer failed!");
        result = CDKResultEFailed;
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::InitializeSyncInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::InitializeSyncInfo(
    SyncInfo *syncInfo)
{
    CDKResult result = CDKResultSuccess;

    if (NULL != syncInfo)
    {
        syncInfo->m_pSyncMutex     = Mutex::Create();
        syncInfo->m_pSyncCondition = Condition::Create();
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::DestroySyncInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::DestroySyncInfo(
    SyncInfo *syncInfo)
{
    CDKResult result = CDKResultSuccess;

    if (NULL != syncInfo)
    {
        if (NULL != syncInfo->m_pSyncMutex)
        {
            syncInfo->m_pSyncMutex->Destroy();
            syncInfo->m_pSyncMutex = NULL;
        }
        if (NULL != syncInfo->m_pSyncCondition)
        {
            syncInfo->m_pSyncCondition->Destroy();
            syncInfo->m_pSyncCondition = NULL;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::CreateSession
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::CreateSession(
    UINT32     sessionId,
    UINT32     numOfPipelines,
    Pipeline** pPipelines)
{
    CDKResult result = CDKResultSuccess;

    ChiCallBacks callbacks = { 0 };
    callbacks.ChiNotify                       = SessionCbNotifyMessage;
    callbacks.ChiProcessCaptureResult         = SessionCbCaptureResult;
    callbacks.ChiProcessPartialCaptureResult  = SessionCbPartialCaptureResult;

    m_sessionInfo[sessionId].m_privateData.sessionId  = sessionId;
    m_sessionInfo[sessionId].m_privateData.pUsecase   = this;

    CHX_LOG_INFO("SessionId:%d, numOfpipeline:%d", sessionId, numOfPipelines);

    if (NULL == m_sessionInfo[sessionId].m_pSession)
    {
        m_sessionInfo[sessionId].m_pSession = Session::Create(pPipelines,
            numOfPipelines, &callbacks, &m_sessionInfo[sessionId].m_privateData);

        if (NULL != m_sessionInfo[sessionId].m_pSession)
        {
            UINT32 frameworkBufferCount = 0;

            for (UINT32 i = 0 ; i < numOfPipelines ; i++)
            {
                UINT32 clientid = m_pMetadataManager->RegisterClient(
                    pPipelines[i]->IsRealTime(),
                    pPipelines[i]->GetTagList(),
                    pPipelines[i]->GetTagCount(),
                    pPipelines[i]->GetPartialTagCount(),
                    pPipelines[i]->GetMetadataBufferCount() + BufferQueueDepth,
                    (pPipelines[i]->IsRealTime() ?
                    ChiMetadataUsage::RealtimeOutput : ChiMetadataUsage::OfflineOutput));

                pPipelines[i]->SetMetadataClientId(clientid);
                frameworkBufferCount += pPipelines[i]->GetMetadataBufferCount() + BufferQueueDepth;
            }

            if (TRUE == pPipelines[0]->IsRealTime())
            {
                m_pMetadataManager->InitializeFrameworkInputClient(frameworkBufferCount, true);
            }

            for (UINT32 pipelineIndex = 0 ; pipelineIndex < numOfPipelines; pipelineIndex++)
            {
                InitializeSyncInfo(&m_sessionInfo[sessionId].m_waitUntilDrain[pipelineIndex]);
            }
        }
        else
        {
            CHX_LOG_ERROR("Failed to create realtime Session with multiple pipeline");
            result = CDKResultEFailed;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GetDownscaleTargetBufferInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult GetDownscaleTargetBufferInfo(
    ChiTargetPortDescriptorInfo* pTargetDescInfo,
    DownscaleTargetBufferInfo&   targetBufferInfo)
{
    CDKResult                result                 = CDKResultSuccess;
    const CHAR*              pClassificationFailure = NULL;
    ChiTargetPortDescriptor* pFullInputNodeInfo     = NULL; // Assumption: Only one node will be associated with full target
    ChiTargetPortDescriptor* pDSInputNodeInfo       = NULL; // Assumption: Only one node will be associated with DS target
    BufferDimension*         pFullDimension         = NULL;

    // First Pass: Find nodes information associated with Full/Downscale targets
    for (UINT j = 0; j < pTargetDescInfo->numTargets; j++)
    {
        auto* pTargetInfo = &pTargetDescInfo->pTargetPortDesc[j];
        auto* pNodeInfo   = &pTargetInfo->pNodePort;

        switch (pTargetInfo->pTarget->pBufferFormats[0])
        {
            case ChiBufferFormat::ChiFormatRawMIPI:
            case ChiBufferFormat::ChiFormatRawPlain16:
            case ChiBufferFormat::ChiFormatYUV420NV12:
            case ChiBufferFormat::ChiFormatP010:
                if ((NULL == pFullInputNodeInfo) || (pFullDimension->maxWidth < pTargetInfo->pTarget->dimension.maxWidth))
                {
                    pFullInputNodeInfo = pTargetInfo;
                    pFullDimension     = &pFullInputNodeInfo->pTarget->dimension;
                }
                break;
            case ChiBufferFormat::ChiFormatPD10:
                pDSInputNodeInfo = pTargetInfo;
                break;
            default: break;
        }
    } // End of first pass

    // Validate whether a full size target exists
    // * Downscaled input is optional
    if (NULL == pFullInputNodeInfo)
    {
        pClassificationFailure = "No full size target buffer found!";
    }

    // Second Pass: Find indexes of Full/DS4/DS16 targets
    //   The generation of g_pipelines.h can order source streams in an ordering dependent on a pipeline's
    //   link declarations within the usecase xml. Find a mapping from source stream to input type
    for (UINT sourceStreamIndex = 0; sourceStreamIndex < pTargetDescInfo->numTargets; sourceStreamIndex++)
    {

        ChiTargetPortDescriptor* pTargetInfo = &pTargetDescInfo->pTargetPortDesc[sourceStreamIndex];
        ChiLinkNodeDescriptor*   pNodeInfo   = pTargetInfo->pNodePort;
        BufferDimension*         pDimension  = &pTargetInfo->pTarget->dimension;

        if (NULL != pClassificationFailure)
        {
            break;
        }
        else if ((0 == pDimension->maxHeight) || (0 == pDimension->maxWidth))
        {
            pClassificationFailure = "Target cannot have 0 as max width or height to avoid divide by zero";
            break;
        }

        const UINT&& downscaleFactor = pFullDimension->maxHeight / pDimension->maxHeight;

        switch (pTargetInfo->pTarget->pBufferFormats[0])
        {
            case ChiBufferFormat::ChiFormatRawMIPI:
            case ChiBufferFormat::ChiFormatRawPlain16:
            case ChiBufferFormat::ChiFormatYUV420NV12:
            case ChiBufferFormat::ChiFormatP010:
                if ((pNodeInfo->nodeInstanceId != pFullInputNodeInfo->pNodePort[0].nodeInstanceId) ||
                    (pDimension->maxHeight != pFullDimension->maxHeight) ||
                    (pDimension->maxWidth != pFullDimension->maxWidth))
                {
                    continue;
                }
                else
                {
                    targetBufferInfo.mapFull[targetBufferInfo.numFull++] = sourceStreamIndex;
                }
                break; // end case ChiBufferFormat::ChiFormatP010
            case ChiBufferFormat::ChiFormatPD10:
                if (NULL == pDSInputNodeInfo)
                {
                    continue; // If no downscale consumer exists, skip DS targets
                }
                if (downscaleFactor != (pFullDimension->maxWidth / pDimension->maxWidth))
                {
                    pClassificationFailure = "Mismatching downscale factors";
                }
                else if (pNodeInfo->nodeInstanceId != pDSInputNodeInfo->pNodePort[0].nodeInstanceId)
                {
                    pClassificationFailure = "Multiple nodes with target format PD10";
                }
                else
                {
                    switch (downscaleFactor)
                    {
                        case  4:
                            targetBufferInfo.mapDS4[targetBufferInfo.numDS4++] = sourceStreamIndex;
                            break;
                        case 16:
                            targetBufferInfo.mapDS16[targetBufferInfo.numDS16++] = sourceStreamIndex;
                            break;
                        default:
                            pClassificationFailure = "Unknown Downscale Factor";
                            break;
                    }
                }
                break; // end case ChiBufferFormat::ChiFormatPD10
            default:
                break;
        }
    } // End of second pass

    if (pClassificationFailure != NULL)
    {
        CHX_LOG_ERROR("Error finding downscale buffer info: %s\n"
                      "\tNumTargetssFull: %u NumTargetsDS4: %u NumTargetsDS16: %u\n"
                      "\tMax Full Dimension: %p (%u x %u)",
                      pClassificationFailure,
                      targetBufferInfo.numFull, targetBufferInfo.numDS4, targetBufferInfo.numDS16,
                      pFullDimension,
                      (pFullDimension ? pFullDimension->maxWidth : 0),
                      (pFullDimension ? pFullDimension->maxHeight : 0));
        result = CDKResultEFailed;
    }


    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::SetupOfflinePipelineMapping
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::SetupOfflinePipelineMapping(
    UINT                         sessionIdx,
    ChiTargetPortDescriptorInfo* pSrcTarget,
    ChiStream**                  ppSourceChiStream,
    UINT&                        sourceIndex,
    ChiPhysicalCameraConfig&     physicalCameraConfiguration)
{
    CDKResult                 result                 = CDKResultSuccess;
    const CHAR*               pClassificationFailure = NULL;
    DownscaleTargetBufferInfo targetBufferInfo       = {0};
    UINT&                     numTargetsFull         = targetBufferInfo.numFull;
    UINT&                     numTargetsDS4          = targetBufferInfo.numDS4;
    UINT&                     numTargetsDS16         = targetBufferInfo.numDS16;

    result = GetDownscaleTargetBufferInfo(pSrcTarget, targetBufferInfo);


    physicalCameraConfiguration.numConfigurations = 0;
    // Do final input validation
    if (CDKResultSuccess == result)
    {
        BOOL expectsDS4Input  = (numTargetsDS4 > 0)  ? TRUE : FALSE;
        BOOL expectsDS16Input = (numTargetsDS16 > 0) ? TRUE : FALSE;

        if (numTargetsFull < m_numOfPhysicalDevices)
        {
            pClassificationFailure = "number of full inputs < number of physical cameras";
        }
        else if ((TRUE == expectsDS4Input) && (numTargetsDS4 < numTargetsFull))
        {
            pClassificationFailure = "number of DS4 inputs < number of full inputs";
        }
        else if ((TRUE == expectsDS16Input) && (numTargetsDS16 < numTargetsFull))
        {
            pClassificationFailure = "number of DS16 inputs < number of full inputs";
        }
        else
        {
            if (TRUE == expectsDS4Input)
            {
                m_offlinePipelineExpectationMask |= DS4_BUF_READY_FLAG;
                numTargetsDS4 = numTargetsFull;
            }
            if (TRUE == expectsDS16Input)
            {
                m_offlinePipelineExpectationMask |= DS16_BUF_READY_FLAG;
                numTargetsDS16 = numTargetsFull;
            }

            // configIdx is used to densely pack physicalCameraConfiguration.configuration within streamMappingClosure
            UINT& configIdx = physicalCameraConfiguration.numConfigurations;
            // Create stream mappings
            for (UINT j = 0; j < numTargetsFull; j++)
            {
                UINT physicalCameraId = -1;
                if (j < m_numOfPhysicalDevices)
                {
                    physicalCameraId = m_pLogicalCameraInfo->ppDeviceInfo[j]->cameraId;
                }

                CHISTREAM* pFullStream      = NULL;
                switch (sessionIdx)
                {
                    case OFFLINE_FUSION_SESSION: pFullStream = m_multiCamResource[j].m_pJPEGInputStream;  break;
                    case OFFLINE_YUV_SESSION:    pFullStream = m_multiCamResource[j].m_pYUVInputStream;   break;
                    case OFFLINE_RAW16_SESSION:  pFullStream = m_multiCamResource[j].m_pRAW16InputStream; break;
                    default:
                        pClassificationFailure = "Unknown offline session type";
                        break;
                }

                if (NULL != pClassificationFailure)
                {
                    break;
                }

                auto streamMappingClosure = [&] (UINT idx, BufferDownscaleType type, CHISTREAM* pSourceStream)
                {
                    PhysicalCameraInputConfiguration* pInputConfig = &physicalCameraConfiguration.configuration[configIdx++];

                    ppSourceChiStream[idx]            = pSourceStream;
                    pInputConfig->physicalCameraId    = physicalCameraId;
                    pInputConfig->nodeDescriptor      = pSrcTarget->pTargetPortDesc[idx].pNodePort[0];
                    pInputConfig->bufferDownscaleType = type;
                };

                streamMappingClosure(targetBufferInfo.mapFull[j], BUFFER_FULL, pFullStream);

                if (numTargetsDS4 > 0)
                {
                    streamMappingClosure(targetBufferInfo.mapDS4[j], BUFFER_DS4, m_multiCamResource[j].m_pDS4InputStream);
                }
                if (numTargetsDS16 > 0)
                {
                    streamMappingClosure(targetBufferInfo.mapDS16[j], BUFFER_DS16, m_multiCamResource[j].m_pDS16InputStream);
                }
            }

            sourceIndex += (numTargetsFull + numTargetsDS4 + numTargetsDS16);
            // Configure dummy streams for unused target inputs
            for (UINT j = 0; j < sourceIndex; j++)
            {
                if (NULL == ppSourceChiStream[j])
                {
                    ppSourceChiStream[j] = m_multiCamResource[m_primaryCameraIndex].m_pDummyStream;
                    configIdx--;
                }
            }
        }
    }

    if (pClassificationFailure != NULL)
    {
        CHX_LOG_ERROR("Error Creating Offline Mapping: %s\n"
                      "\tNumPhysicalCameras: %u NumInputsFull: %u NumInputsDS4: %u NumInputsDS16: %u\n",
                      pClassificationFailure, m_numOfPhysicalDevices, numTargetsFull, numTargetsDS4, numTargetsDS4);
        result = CDKResultEFailed;
    }

    return CDKResult();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::CreateOfflineSession
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::CreateOfflineSession()
{
    CDKResult result   = CDKResultSuccess;
    UINT numOfPipeline = 1;

    for (UINT i = OFFLINE_RDI_SESSION0; i < MAX_MULTI_CAMERA_SESSION; i++)
    {
        if ((OFFLINE_RAW16_SESSION == i) &&
            ((UsecaseRTB == m_usecaseMode) || (FALSE == m_isRaw16Needed)))
        {
            // No need to create RAW16session if its RTB or RAW16 not needed cases
            continue;
        }

        if (0 != m_sessionInfo[i].m_numPipelines)
        {
            result = CreateSession(i, m_sessionInfo[i].m_numPipelines,
                m_sessionInfo[i].m_pPipeline);
        }

        if (CDKResultEFailed == result)
        {
            CHX_LOG_ERROR("CreateSession failed! Session = %d, numPipelines = %d", i, m_sessionInfo[i].m_numPipelines);
            break;
        }
    }

    if (CDKResultSuccess != result)
    {
        for (UINT i = OFFLINE_RDI_SESSION0; i < MAX_MULTI_CAMERA_SESSION; i++)
        {
            if (NULL != m_sessionInfo[i].m_pSession)
            {
                m_sessionInfo[i].m_pSession->Destroy(TRUE);
                m_sessionInfo[i].m_pSession = NULL;
            }
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::CreatePipelines
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::CreatePipelines(
        ChiUsecase*                     pChiUsecase,
        camera3_stream_configuration_t* pStreamConfig)
{
    CDKResult result                        = CDKResultSuccess;
    UINT32    m_numRealtimePipelines        = 0;
    UINT32    m_numOfflineYUVinputStreams   = 0;
    UINT32    sessionIdx;


    for (UINT index = 0; index < pChiUsecase->numPipelines; index++)
    {
        UINT32     cameraId;
        BOOL       bCreatePipeline           = TRUE;
        UINT32     sinkIndex                 = 0;
        UINT32     sourceIndex               = 0;
        BOOL       setupPhysicalCameraConfig = FALSE;
        BOOL       alwaysOn                  = FALSE;
        ChiStream* pSourceChiStream[MaxChiStreams];
        ChiStream* pSinkChiStream[MaxChiStreams];

        ChiPhysicalCameraConfig      physicalCameraConfiguration;
        ChiTargetPortDescriptorInfo* pSinkTarget  = &pChiUsecase->pPipelineTargetCreateDesc[index].sinkTarget;
        ChiTargetPortDescriptorInfo* pSrcTarget   = &pChiUsecase->pPipelineTargetCreateDesc[index].sourceTarget;
        UINT                         sessionIndex = GetSessionIndex(index, m_usecaseMode);

        if (sessionIndex >= MAX_MULTI_CAMERA_SESSION)
        {
            CHX_LOG_INFO("Pipeline creation skipped = %d mode = %d phyCamCnt = %d",
                index, m_usecaseMode, m_numOfPhysicalDevices);
            continue;
        }

        CHX_LOG_INFO("NumPhyCam = %d, mode = %d pipeline Index:%d, name: %s, targetNum:%d,sourceNums:%d",
            m_numOfPhysicalDevices, m_usecaseMode, index, pChiUsecase->pPipelineTargetCreateDesc[index].pPipelineName,
            pSinkTarget->numTargets, pSrcTarget->numTargets);

        if ((FALSE == m_isOfflineRequired) && (OFFLINE_YUV_SESSION < sessionIndex))
        {
            continue;
        }

        cameraId = m_pLogicalCameraInfo->ppDeviceInfo[m_primaryCameraIndex]->cameraId;
        switch (sessionIndex)
        {
            case REALTIME_SESSION:
                {
                    /// YUV for Preview and RDI for Snapshot
                    DownscaleTargetBufferInfo targetBufferInfo = {0};
                    result     = GetDownscaleTargetBufferInfo(pSinkTarget, targetBufferInfo);
                    cameraId   = m_pLogicalCameraInfo->ppDeviceInfo[m_numRealtimePipelines]->cameraId;
                    alwaysOn   = m_pLogicalCameraInfo->ppDeviceInfo[m_numRealtimePipelines]->pDeviceConfig->alwaysOn;
                    sessionIdx = REALTIME_SESSION;

                    pSinkChiStream[sinkIndex++] = m_multiCamResource[m_numRealtimePipelines].m_pRTOutputYUVStream;

                    if (targetBufferInfo.numDS4 > 0)
                    {
                        pSinkChiStream[sinkIndex++] = m_multiCamResource[m_numRealtimePipelines].m_pRTOutputDS4Stream;
                    }
                    if (targetBufferInfo.numDS16 > 0)
                    {
                        pSinkChiStream[sinkIndex++] = m_multiCamResource[m_numRealtimePipelines].m_pRTOutputDS16Stream;
                    }

                    pSinkChiStream[sinkIndex++] = m_multiCamResource[m_numRealtimePipelines].m_pRTOutputRDIStream;

                    //Real time RDI RAW16number
                    if (UsecaseSAT == m_usecaseMode)
                    {
                        pSinkChiStream[sinkIndex++] = m_multiCamResource[m_numRealtimePipelines].m_pRTOutputRAW16Stream;
                    }

                    // Real Time FD
                    UINT fdIndex = GetTargetIndex(pSinkTarget, "TARGET_BUFFER_FD");
                    if (CDKInvalidId != fdIndex)
                    {
                        pSinkChiStream[fdIndex] = m_multiCamResource[m_numRealtimePipelines].m_pRTOutputFDStream;
                        m_isFdStreamSupported = TRUE;
                        sinkIndex++;
                    }

                    m_numRealtimePipelines++;
                }
                break;

            case OFFLINE_YUV_SESSION:

                /// YUV Inpput and Target Preview frame
                sessionIdx = OFFLINE_YUV_SESSION;
                // Offline Preview Pipeline
                pSinkChiStream[sinkIndex++] = m_pTargetPreviewStream;

                if (UsecaseSAT == m_usecaseMode)
                {
                    if (TRUE == m_isVideoNeeded)
                    {
                        pSinkChiStream[sinkIndex++] = m_pTargetVideoStream;
                    }
                    else
                    {
                        pSinkChiStream[sinkIndex++] = m_multiCamResource[m_primaryCameraIndex].m_pDummyStream;
                    }
                }
                setupPhysicalCameraConfig = TRUE;
                result = SetupOfflinePipelineMapping(sessionIdx, pSrcTarget, pSourceChiStream,
                                                     sourceIndex, physicalCameraConfiguration);
                break;
            case OFFLINE_RDI_SESSION0:
            case OFFLINE_RDI_SESSION1:
            case OFFLINE_RDI_SESSION2:
                /// RDI Input and YUV Snapshot frame
                cameraId      = m_pLogicalCameraInfo->ppDeviceInfo[m_numOfflineYUVinputStreams]->cameraId;
                sessionIdx    = OFFLINE_RDI_SESSION0 + m_numOfflineYUVinputStreams;

                pSinkChiStream[sinkIndex++] = m_multiCamResource[m_numOfflineYUVinputStreams].m_pYUVOutputSnapshotStream;

                if (TRUE == m_isPostViewNeeded)
                {
                    pSinkChiStream[sinkIndex++] = m_pTargetPostviewStream;
                }
                else
                {
                    pSinkChiStream[sinkIndex++] = m_multiCamResource[m_numOfflineYUVinputStreams].m_pDummyStream;
                }

                pSourceChiStream[sourceIndex++] = m_multiCamResource[m_numOfflineYUVinputStreams].m_pRTOutputRDIStream;
                m_numOfflineYUVinputStreams++;

                break;

            case OFFLINE_FUSION_SESSION:
                /// YUV Input snapshot and Jpeg Output
                sessionIdx                  = OFFLINE_FUSION_SESSION;
                pSinkChiStream[sinkIndex++] = m_multiCamResource[m_maxJpegCameraId].m_pFusionStream;
                setupPhysicalCameraConfig   = TRUE;

                result = SetupOfflinePipelineMapping(sessionIdx, pSrcTarget, pSourceChiStream,
                                                     sourceIndex, physicalCameraConfiguration);
                break;

            case OFFLINE_JPEG_SESSION:
                // Offline Bokeh SNAPSHOT

                // Pass the CameraID corresponding to maximum resolution as logical camera contains the max jpeg size of both
                cameraId                        = UsecaseSelector::FindMaxResolutionCameraID(m_pLogicalCameraInfo);
                sessionIdx                      = OFFLINE_JPEG_SESSION;

                pSinkChiStream[sinkIndex++]     = m_pTargetSnapshotStream;
                pSourceChiStream[sourceIndex++] = m_multiCamResource[m_maxJpegCameraId].m_pFusionStream;
                break;

            case OFFLINE_RAW16_SESSION:
                sessionIdx                      = OFFLINE_RAW16_SESSION;
                setupPhysicalCameraConfig       = TRUE;
                // offline preview pipeline  sink stream
                if (TRUE == m_isRaw16Needed)
                {
                    pSinkChiStream[sinkIndex++] = m_pTargetRAW16Stream;
                }
                else
                {
                    // Add RAW16 dummy stream to avoid this pipeline creation failure if RAW stream is not there
                    pSinkChiStream[sinkIndex++] = m_multiCamResource[0].m_pRTOutputRAW16Stream;
                }

                setupPhysicalCameraConfig   = TRUE;
                result = SetupOfflinePipelineMapping(sessionIdx, pSrcTarget, pSourceChiStream,
                                                     sourceIndex, physicalCameraConfiguration);
                break;

            default:
                CHX_LOG_WARN("Pipeline index is more than expected = %d", sessionIndex);
                bCreatePipeline = FALSE;
                break;
        }

        if (CDKResultSuccess == result && TRUE == bCreatePipeline)
        {
            Pipeline* m_pPipeline = NULL;

            m_pPipeline = Pipeline::Create(cameraId, PipelineType::Default);
            if (NULL != m_pPipeline)
            {
                ChiPipelineCreateDescriptor* pPipelineDesc = &pChiUsecase->pPipelineTargetCreateDesc[index].pipelineCreateDesc;
                ChiPortBufferDescriptor      BufferDesc[MaxChiStreams];
                ChxUtils::Memset(BufferDesc, 0 , sizeof(BufferDesc));


                if (sinkIndex != pSinkTarget->numTargets)
                {
                    CHX_LOG_WARN("Create pipeline:%s mismatch between numOfSinkStreams:%d and numofSinkTarget:%d!",
                        pChiUsecase->pPipelineTargetCreateDesc[index].pPipelineName,
                        sinkIndex,
                        pSinkTarget->numTargets);
                }

                if(sourceIndex != pSrcTarget->numTargets)
                {
                    CHX_LOG_WARN("create pipeline:%s mismatch between numOfSourceStreams:%d and numofSourceTarget:%d!",
                        pChiUsecase->pPipelineTargetCreateDesc[index].pPipelineName,
                        sourceIndex,
                        pSrcTarget->numTargets);

                    pSrcTarget->numTargets = sourceIndex;
                    pPipelineDesc->pNodes->nodeAllPorts.numInputPorts = sourceIndex;
                }

                for (UINT i = 0 ; i < sourceIndex ; i++)
                {
                    BufferDesc[i].pNodePort    = pSrcTarget->pTargetPortDesc[i].pNodePort;
                    BufferDesc[i].numNodePorts = pSrcTarget->pTargetPortDesc[i].numNodePorts;
                    BufferDesc[i].pStream      = pSourceChiStream[i];
                    pSrcTarget->pTargetPortDesc[i].pTarget->pChiStream = pSourceChiStream[i];


                }
                m_pPipeline->SetInputBuffers(sourceIndex, &BufferDesc[0]);

                ChxUtils::Memset(BufferDesc, 0, sizeof(BufferDesc));

                for (UINT i = 0 ; i < sinkIndex ; i++)
                {
                    BufferDesc[i].pNodePort    = pSinkTarget->pTargetPortDesc[i].pNodePort;
                    BufferDesc[i].numNodePorts = pSinkTarget->pTargetPortDesc[i].numNodePorts;
                    BufferDesc[i].pStream      = pSinkChiStream[i];
                    pSinkTarget->pTargetPortDesc[i].pTarget->pChiStream = pSinkChiStream[i];
                }
                m_pPipeline->SetOutputBuffers(sinkIndex, &BufferDesc[0]);

                m_pPipeline->SetPipelineNodePorts(pPipelineDesc);
                m_pPipeline->SetPipelineName(pChiUsecase->pPipelineTargetCreateDesc[index].pPipelineName);

                // This is a multi-sensor use case, it needs more hardware resource for two realtime
                // pipeline, therefore, limit HW resource for each pipeline.
                m_pPipeline->SetPipelineResourcePolicy(HWResourceMinimal);

                // here we just set defer flag for SAT usecase
                if (UsecaseSAT == m_usecaseMode)
                {
                    if ((TRUE == alwaysOn) && (REALTIME_SESSION == sessionIndex))
                    {
                        m_pPipeline->SetDeferFinalizeFlag(FALSE);
                    }
                    else
                    {
                        m_pPipeline->SetDeferFinalizeFlag(TRUE);
                    }
                }

                updateSessionSettings(m_pPipeline, pStreamConfig);
                m_defaultSensorModePickHint.sensorModeCaps.value    = 0;
                m_defaultSensorModePickHint.postSensorUpscale       = FALSE;
                m_defaultSensorModePickHint.sensorModeCaps.u.Normal = TRUE;
                m_pPipeline->SetSensorModePickHint(&m_defaultSensorModePickHint);

                if (TRUE == setupPhysicalCameraConfig)
                {
                    ChiMetadata* pMetadata = m_pPipeline->GetDescriptorMetadata();
                    pMetadata->SetTag("com.qti.chi.cameraconfiguration",
                                      "PhysicalCameraInputConfig",
                                      &physicalCameraConfiguration,
                                      sizeof(ChiPhysicalCameraConfig));
                }

                result = m_pPipeline->CreateDescriptor();
                if (CDKResultSuccess != result)
                {
                    CHX_LOG_ERROR("Create pipeline %s descriptor failed!",
                        pChiUsecase->pPipelineTargetCreateDesc[index].pPipelineName);
                }
                else
                {
                    m_sessionInfo[sessionIdx].m_pPipeline[m_sessionInfo[sessionIdx].m_numPipelines++] = m_pPipeline;
                }
            }
            else
            {
                CHX_LOG_ERROR("Pipeline creation failed pipeline = %d cameraId = %d", index, cameraId);
                result = CDKResultEFailed;
            }
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// UsecaseMultiCamera::SelectUsecaseXML
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ChiUsecase* UsecaseMultiCamera::SelectUsecaseXML()
{
    CDKResult result             = CDKResultSuccess;
    ChiUsecase* pUsecase         = NULL;
    LogicalCameraType cameraType = ExtensionModule::GetInstance()->GetCameraType(m_cameraId);
    UINT32    usecaseMode        = 0;
    UINT8     found              = FALSE;

    switch(cameraType)
    {
        case LogicalCameraType_SAT:
            m_usecaseMode = UsecaseSAT;
            CHX_LOG("SAT/DUAL FOV Preview with frame sync mode %d", m_kernelFrameSyncEnable);
            break;
        case LogicalCameraType_RTB:
            m_usecaseMode = UsecaseRTB;
            CHX_LOG("RTB Preview with frame sync mode %d", m_kernelFrameSyncEnable);
            break;
        default:
            CHX_LOG_ERROR("Not supported type with this usecase");
            result = CDKResultEInvalidArg;
    }

    if (CDKResultSuccess == result)
    {
        for (UINT32 i = 0; i < (sizeof(usecaseMap)/sizeof(usecaseMap[0])); i++)
        {
            if (m_pLogicalCameraInfo->numPhysicalCameras == usecaseMap[i].numPhyCameras)
            {
                if (m_usecaseMode == usecaseMap[i].usecaseMode)
                {
                    pUsecase            = usecaseMap[i].pUsecaseXML;
                }
            }
        }
    }

    if (NULL == pUsecase)
    {
        CHX_LOG_WARN("ERROR: No usecase XML Entry Found in table usecaseMap."
            "NumCam = %d, usecase = %d", m_pLogicalCameraInfo->numPhysicalCameras, m_usecaseMode);
    }
    return pUsecase;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::ClassifyTargetStream
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::ClassifyTargetStream(camera3_stream_configuration_t* pStreamConfig)
{
    CDKResult result             = CDKResultSuccess;

    m_pTargetVideoStream         = NULL;
    m_pTargetPreviewStream       = NULL;
    m_pTargetSnapshotStream      = NULL;
    m_pTargetPrimaryJpegStream   = NULL;
    m_pTargetPostviewStream      = NULL;
    m_pTargetRAW16Stream         = NULL;
    m_pTargetRDIStream           = NULL;

    for (UINT stream = 0; stream < pStreamConfig->num_streams; stream++)
    {
        CHX_LOG("stream = %p streamType = %d streamFormat = %d streamWidth = %d streamHeight = %d usage = 0x%x",
            pStreamConfig->streams[stream],
            pStreamConfig->streams[stream]->stream_type,
            pStreamConfig->streams[stream]->format,
            pStreamConfig->streams[stream]->width,
            pStreamConfig->streams[stream]->height,
            pStreamConfig->streams[stream]->usage);
        if (HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED == pStreamConfig->streams[stream]->format)
        {
            if (UsecaseSelector::IsVideoStream(pStreamConfig->streams[stream]))
            {
                m_pTargetVideoStream = reinterpret_cast<CHISTREAM*>(pStreamConfig->streams[stream]);
                CHX_LOG("stream = %d m_pTargetVideoStream %p", stream, m_pTargetVideoStream);
                m_isVideoNeeded = TRUE;
            }
            else
            {
                m_pTargetPreviewStream = reinterpret_cast<CHISTREAM*>(pStreamConfig->streams[stream]);
                // Set SW_READ_OFTEN usage flag for NV12 output from SW CHI node(RTB/SAT)
                if (NULL != m_pTargetPreviewStream)
                {
                    m_pTargetPreviewStream->grallocUsage |= GRALLOC_USAGE_SW_READ_OFTEN;
                    CHX_LOG("stream = %d m_pTargetPreviewStream %p", stream, m_pTargetPreviewStream);
                }
                else
                {
                    result = CDKResultEFailed;
                    CHX_LOG("Error: m_pTargetPreviewStream is NULL");
                    break;
                }
            }
        }
        else if (HAL_PIXEL_FORMAT_YCbCr_420_888 == pStreamConfig->streams[stream]->format)
        {
            m_pTargetPostviewStream = reinterpret_cast<CHISTREAM*>(pStreamConfig->streams[stream]);
            m_isPostViewNeeded = TRUE;
            CHX_LOG("stream = %d m_pTargetPostviewStream %p", stream, m_pTargetPostviewStream);
        }
        else if (HAL_PIXEL_FORMAT_RAW16 == pStreamConfig->streams[stream]->format)
        {
            m_pTargetRAW16Stream = reinterpret_cast<CHISTREAM*>(pStreamConfig->streams[stream]);
            m_isRaw16Needed = TRUE;
            CHX_LOG("stream = %d m_pTargetRAW16Stream %p", stream, m_pTargetRAW16Stream);
        }
        else if (HAL_PIXEL_FORMAT_BLOB == pStreamConfig->streams[stream]->format)
        {
            if (NULL == m_pTargetSnapshotStream)
            {
                m_pTargetSnapshotStream = reinterpret_cast<CHISTREAM*>(pStreamConfig->streams[stream]);
            }
            else
            {
                m_pTargetPrimaryJpegStream = reinterpret_cast<CHISTREAM*>(pStreamConfig->streams[stream]);
                m_bokehPrimaryJpeg = TRUE;

                CHX_LOG("Wide Snapshot: stream = %d m_pTargetPrimaryJpegStream %p",
                    stream, m_pTargetSnapshotStream);
            }
        }
    }

    if (NULL == m_pTargetSnapshotStream)
    {
        m_isOfflineRequired = FALSE;
    }
    else
    {
        m_isOfflineRequired = TRUE;
    }

    if (m_pTargetPreviewStream == NULL)
    {
        CHX_LOG_ERROR("m_pTargetPreviewStream is NULL, returning result as failed");
        result = CDKResultEFailed;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::CreateMultiCameraResource
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::CreateMultiCameraResource(
    camera3_stream_configuration_t* pStreamConfig)
{
    CDKResult result             = CDKResultSuccess;

    UINT32 previewRes = 0;
    UINT32 videoRes   = 0;
    UINT32 maxJpegRes = 0;

    UINT32 RDIMaxWidth  = 0;
    UINT32 RDIMaxHeight = 0;

    FLOAT previewAR   = 0;
    m_maxJpegCameraId = 0;

    if (NULL != m_pTargetRAW16Stream)
    {
        for (UINT32 i = 0 ; i < m_numOfPhysicalDevices ; i++)
        {
            MultiCameraResource* pObj        = &m_multiCamResource[i];
            pObj->m_RAWWidth  = m_pTargetRAW16Stream->width;
            pObj->m_RAWHeight = m_pTargetRAW16Stream->height;
        }
    }
    else if (NULL != m_pTargetSnapshotStream)
    {
        for (UINT32 i = 0 ; i < m_numOfPhysicalDevices ; i++)
        {
            MultiCameraResource* pObj        = &m_multiCamResource[i];
            UsecaseSelector::getSensorDimension(m_pLogicalCameraInfo->ppDeviceInfo[i]->cameraId,
                                                pStreamConfig,
                                                &pObj->m_RAWWidth,
                                                &pObj->m_RAWHeight,
                                                1);
            if ((pObj->m_RAWWidth * pObj->m_RAWHeight) > (RDIMaxWidth * RDIMaxHeight))
            {
                RDIMaxWidth  = pObj->m_RAWWidth;
                RDIMaxHeight = pObj->m_RAWHeight;
            }
            CHX_LOG_INFO("RDI resolution:%dx%d,camera index:%d",
                pObj->m_RAWWidth, pObj->m_RAWHeight, i);
        }
    }
    else
    {
        CHX_LOG_INFO("RAW Stream not needed for this usecase");
    }

    previewAR = static_cast<FLOAT>(m_pTargetPreviewStream->width) /
        m_pTargetPreviewStream->height;
    // Default FD stream dimensions
    UINT32  fdwidth  = DUMMYWIDTH;
    UINT32  fdheight = DUMMYHEIGHT;
    FLOAT   fdAR     = static_cast<FLOAT>(fdwidth) / static_cast<FLOAT>(fdheight);

    if (fdAR < previewAR)
    {
        fdheight = static_cast<UINT32>(static_cast<FLOAT>(fdwidth) / previewAR);
    }
    else if (fdAR > previewAR)
    {
        fdwidth = static_cast<UINT32>(static_cast<FLOAT>(fdheight) * previewAR);
    }

    fdwidth  = ChxUtils::EvenCeilingUINT32(fdwidth);
    fdheight = ChxUtils::EvenCeilingUINT32(fdheight);

    if ((m_isVideoNeeded) && (NULL != m_pTargetVideoStream))
    {
        videoRes = (m_pTargetVideoStream->width * m_pTargetVideoStream->height);
    }
    previewRes   = (m_pTargetPreviewStream->width * m_pTargetPreviewStream->height);

    for (UINT32 i = 0; i < m_numOfPhysicalDevices; i++)
    {
        MultiCameraResource* pObj        = &m_multiCamResource[i];
        pObj->m_pRTOutputYUVStream       = static_cast<CHISTREAM*>(ChxUtils::Calloc(sizeof(CHISTREAM)));
        pObj->m_pRTOutputDS4Stream       = static_cast<CHISTREAM*>(ChxUtils::Calloc(sizeof(CHISTREAM)));
        pObj->m_pRTOutputDS16Stream      = static_cast<CHISTREAM*>(ChxUtils::Calloc(sizeof(CHISTREAM)));
        pObj->m_pRTOutputRDIStream       = static_cast<CHISTREAM*>(ChxUtils::Calloc(sizeof(CHISTREAM)));
        pObj->m_pRTOutputFDStream        = static_cast<CHISTREAM*>(ChxUtils::Calloc(sizeof(CHISTREAM)));
        pObj->m_pRTOutputRAW16Stream     = static_cast<CHISTREAM*>(ChxUtils::Calloc(sizeof(CHISTREAM)));
        pObj->m_pYUVInputStream          = static_cast<CHISTREAM*>(ChxUtils::Calloc(sizeof(CHISTREAM)));
        pObj->m_pDS4InputStream          = static_cast<CHISTREAM*>(ChxUtils::Calloc(sizeof(CHISTREAM)));
        pObj->m_pDS16InputStream         = static_cast<CHISTREAM*>(ChxUtils::Calloc(sizeof(CHISTREAM)));
        pObj->m_pRAW16InputStream        = static_cast<CHISTREAM*>(ChxUtils::Calloc(sizeof(CHISTREAM)));
        pObj->m_pYUVOutputSnapshotStream = static_cast<CHISTREAM*>(ChxUtils::Calloc(sizeof(CHISTREAM)));
        pObj->m_pJPEGInputStream         = static_cast<CHISTREAM*>(ChxUtils::Calloc(sizeof(CHISTREAM)));
        pObj->m_pDummyStream             = static_cast<CHISTREAM*>(ChxUtils::Calloc(sizeof(CHISTREAM)));

        if (TRUE == !((NULL != pObj->m_pRTOutputYUVStream) && (NULL != pObj->m_pRTOutputRDIStream) &&
            (NULL != pObj->m_pRTOutputFDStream) && (NULL != pObj->m_pRTOutputRAW16Stream) &&
            (NULL != pObj->m_pYUVInputStream) && (NULL != pObj->m_pRAW16InputStream) &&
            (NULL != pObj->m_pYUVOutputSnapshotStream) && (NULL != pObj->m_pJPEGInputStream) &&
            (NULL != pObj->m_pDummyStream) && (NULL != pObj->m_pRTOutputDS4Stream) &&
            (NULL != pObj->m_pRTOutputDS16Stream) && (NULL != pObj->m_pDS4InputStream) &&
            (NULL != pObj->m_pDS16InputStream)))
        {
            CHX_LOG_ERROR("Failed to alloc memory for Multi Camera Resources (Streams)");
            result = CDKResultENoMemory;
            break;
        }
        // if Video enable, it needes to check which stream size is bigger, and use bigger
        // stream size for realtime preview pipeline output stream.
        if (videoRes > previewRes)
        {
            ChxUtils::Memcpy(pObj->m_pYUVInputStream, m_pTargetVideoStream, sizeof(CHISTREAM));
            ChxUtils::Memcpy(pObj->m_pDS4InputStream, m_pTargetVideoStream, sizeof(CHISTREAM));
            ChxUtils::Memcpy(pObj->m_pDS16InputStream, m_pTargetVideoStream, sizeof(CHISTREAM));
            ChxUtils::Memcpy(pObj->m_pRTOutputYUVStream, m_pTargetVideoStream, sizeof(CHISTREAM));
            ChxUtils::Memcpy(pObj->m_pRTOutputDS4Stream, m_pTargetVideoStream, sizeof(CHISTREAM));
            ChxUtils::Memcpy(pObj->m_pRTOutputDS16Stream, m_pTargetVideoStream, sizeof(CHISTREAM));
            ChxUtils::Memcpy(pObj->m_pDummyStream, m_pTargetVideoStream, sizeof(CHISTREAM));
        }
        else
        {
            ChxUtils::Memcpy(pObj->m_pYUVInputStream, m_pTargetPreviewStream, sizeof(CHISTREAM));
            ChxUtils::Memcpy(pObj->m_pDS4InputStream, m_pTargetPreviewStream, sizeof(CHISTREAM));
            ChxUtils::Memcpy(pObj->m_pDS16InputStream, m_pTargetPreviewStream, sizeof(CHISTREAM));
            ChxUtils::Memcpy(pObj->m_pRTOutputYUVStream, m_pTargetPreviewStream, sizeof(CHISTREAM));
            ChxUtils::Memcpy(pObj->m_pRTOutputDS4Stream, m_pTargetPreviewStream, sizeof(CHISTREAM));
            ChxUtils::Memcpy(pObj->m_pRTOutputDS16Stream, m_pTargetPreviewStream, sizeof(CHISTREAM));
            ChxUtils::Memcpy(pObj->m_pDummyStream, m_pTargetPreviewStream, sizeof(CHISTREAM));
        }
        pObj->m_pRTOutputYUVStream->streamType      = ChiStreamTypeBidirectional;
        pObj->m_pRTOutputDS4Stream->streamType      = ChiStreamTypeBidirectional;
        pObj->m_pRTOutputDS16Stream->streamType     = ChiStreamTypeBidirectional;

        pObj->m_pRTOutputYUVStream->format          = pObj->m_RTOutputYUVFormat;
        pObj->m_pRTOutputDS4Stream->format          = pObj->m_RTOutputDS4Format;
        pObj->m_pRTOutputDS16Stream->format         = pObj->m_RTOutputDS16Format;

        pObj->m_pYUVInputStream->format             = pObj->m_RTOutputYUVFormat;
        pObj->m_pDS4InputStream->format             = pObj->m_RTOutputDS4Format;
        pObj->m_pDS16InputStream->format            = pObj->m_RTOutputDS16Format;

        pObj->m_pDS4InputStream->width              =
            ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(pObj->m_pYUVInputStream->width, 4) / 4);
        pObj->m_pRTOutputDS4Stream->width           =
            ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(pObj->m_pRTOutputYUVStream->width, 4) / 4);
        pObj->m_pDS16InputStream->width             =
            ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(pObj->m_pYUVInputStream->width, 16) / 16);
        pObj->m_pRTOutputDS16Stream->width          =
            ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(pObj->m_pRTOutputYUVStream->width, 16) / 16);

        pObj->m_pDS4InputStream->height             =
            ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(pObj->m_pYUVInputStream->height, 4) / 4);
        pObj->m_pRTOutputDS4Stream->height          =
            ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(pObj->m_pRTOutputYUVStream->height, 4) / 4);
        pObj->m_pDS16InputStream->height            =
            ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(pObj->m_pYUVInputStream->height, 16) / 16);
        pObj->m_pRTOutputDS16Stream->height         =
            ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(pObj->m_pRTOutputYUVStream->height, 16) / 16);
        pObj->m_pDummyStream->width  = DUMMYWIDTH;
        pObj->m_pDummyStream->height = DUMMYHEIGHT;
        pObj->m_pDummyStream->format = ChiStreamFormatYCbCr420_888;

        if (NULL != m_pTargetSnapshotStream)
        {
            ChxUtils::Memcpy(pObj->m_pYUVOutputSnapshotStream, m_pTargetSnapshotStream, sizeof(CHISTREAM));
        }
        pObj->m_pYUVOutputSnapshotStream->format        = ChiStreamFormatYCbCr420_888;
        pObj->m_pYUVOutputSnapshotStream->grallocUsage  = (ChiGralloc1ProducerUsageCamera |
                                                           ChiGralloc1ConsumerUsageCamera |
                                                           ChiGralloc1ConsumerUsageCpuRead);
        // Assign RDI dimensions to YUV output as BOKEH/SAT node needs the uncropped image
        // and also need to handle the asymmetric configuration
        // For Asymmetric W X H for MAIN & AUX assign max W X H so, that two YUV streams would be
        // of same size and thus SAT Node can handle it.
        pObj->m_pYUVOutputSnapshotStream->width  = RDIMaxWidth;
        pObj->m_pYUVOutputSnapshotStream->height = RDIMaxHeight;


        if (NULL != m_pTargetSnapshotStream)
        {
            FLOAT snapRatio = static_cast<FLOAT>(m_pTargetSnapshotStream->height) /
                              static_cast<FLOAT>(m_pTargetSnapshotStream->width);
            FLOAT rdiRatio  = static_cast<FLOAT>(RDIMaxHeight) / static_cast<FLOAT>(RDIMaxWidth);

            if (abs(rdiRatio - snapRatio) > 0.1)
            {

                pObj->m_pYUVOutputSnapshotStream->width = m_pTargetSnapshotStream->width;
                pObj->m_pYUVOutputSnapshotStream->height= m_pTargetSnapshotStream->height;
            }
        }
        ChxUtils::Memcpy(pObj->m_pJPEGInputStream, pObj->m_pYUVOutputSnapshotStream, sizeof(CHISTREAM));

        pObj->m_pRTOutputRDIStream->format        = ChiStreamFormatRawOpaque;
        pObj->m_pRTOutputRDIStream->width         = pObj->m_RAWWidth;
        pObj->m_pRTOutputRDIStream->height        = pObj->m_RAWHeight;
        pObj->m_pRTOutputRDIStream->maxNumBuffers = 0;
        pObj->m_pRTOutputRDIStream->rotation      = StreamRotationCCW0;
        pObj->m_pRTOutputRDIStream->streamType    = ChiStreamTypeBidirectional;
        pObj->m_pRTOutputRDIStream->grallocUsage  = 0;

        pObj->m_pRTOutputFDStream->format        = ChiStreamFormatYCbCr420_888;
        pObj->m_pRTOutputFDStream->width         = fdwidth;
        pObj->m_pRTOutputFDStream->height        = fdheight;
        pObj->m_pRTOutputFDStream->maxNumBuffers = 0;
        pObj->m_pRTOutputFDStream->rotation      = StreamRotationCCW0;
        pObj->m_pRTOutputFDStream->streamType    = ChiStreamTypeOutput;
        pObj->m_pRTOutputFDStream->grallocUsage  = 0;

        if (UsecaseSAT == m_usecaseMode)
        {
            pObj->m_pRTOutputRAW16Stream->format        = ChiStreamFormatRaw16;
            pObj->m_pRTOutputRAW16Stream->width         = pObj->m_RAWWidth;
            pObj->m_pRTOutputRAW16Stream->height        = pObj->m_RAWHeight;
            pObj->m_pRTOutputRAW16Stream->maxNumBuffers = 0;
            pObj->m_pRTOutputRAW16Stream->rotation      = StreamRotationCCW0;
            pObj->m_pRTOutputRAW16Stream->streamType    = ChiStreamTypeBidirectional;
            pObj->m_pRTOutputRAW16Stream->grallocUsage  = 0;

            ChxUtils::Memcpy(m_multiCamResource[i].m_pRAW16InputStream,
                pObj->m_pRTOutputRAW16Stream, sizeof(CHISTREAM));
        }

        if (maxJpegRes < (pObj->m_pJPEGInputStream->width * pObj->m_pJPEGInputStream->height))
        {
            maxJpegRes        = (pObj->m_pJPEGInputStream->width * pObj->m_pJPEGInputStream->height);
            m_maxJpegCameraId = i;
        }


        pObj->m_pYUVInputStream->streamType          = ChiStreamTypeInput;
        pObj->m_pDS4InputStream->streamType          = ChiStreamTypeInput;
        pObj->m_pDS16InputStream->streamType         = ChiStreamTypeInput;
        pObj->m_pDummyStream->streamType             = ChiStreamTypeOutput;
        pObj->m_pYUVOutputSnapshotStream->streamType = ChiStreamTypeOutput;
        pObj->m_pJPEGInputStream->streamType         = ChiStreamTypeInput;

        if (UsecaseSAT == m_usecaseMode)
        {
            pObj->m_pRTOutputRAW16Stream->streamType  = ChiStreamTypeBidirectional;
            pObj->m_pRAW16InputStream->streamType     = ChiStreamTypeInput;
        }

        pObj->m_pStickyMetadata = ChiMetadata::Create(NULL, 0, true);
        if(NULL == pObj->m_pStickyMetadata)
        {
            CHX_LOG_ERROR("ChiMetadata::Create fails for sticky meta data");
            result = CDKResultEFailed;
            break;
        }
        pObj->m_baseFrameNumber = INVALIDFRAMENUMBER;
    }

    if (result == CDKResultSuccess)
    {
        MultiCameraResource* pObj    = &m_multiCamResource[m_maxJpegCameraId];
        pObj->m_pFusionStream        = static_cast<CHISTREAM*>(ChxUtils::Calloc(sizeof(CHISTREAM)));
        if (NULL != pObj->m_pFusionStream)
        {
            ChxUtils::Memcpy(pObj->m_pFusionStream, pObj->m_pJPEGInputStream, sizeof(CHISTREAM));

            pObj->m_pFusionStream->grallocUsage  = (ChiGralloc1ProducerUsageCamera |
                                                    ChiGralloc1ConsumerUsageCamera |
                                                    ChiGralloc1ConsumerUsageCpuRead);
            pObj->m_pFusionStream->streamType    = ChiStreamTypeBidirectional;
        }
        else
        {
            CHX_LOG_ERROR("m_pSnapshotFusionStream allocation failure");
            result = CDKResultEFailed;
        }
    }

    if (result == CDKResultSuccess)
    {
        CHIBufferManagerCreateData rtYUVBufData  = { 0 };
        rtYUVBufData.format                      = m_pTargetPreviewStream->format;
        rtYUVBufData.width                       = m_pTargetPreviewStream->width;
        rtYUVBufData.height                      = m_pTargetPreviewStream->height;
        rtYUVBufData.consumerFlags               = ChiGralloc1ConsumerUsageCamera|ChiGralloc1ConsumerUsageCpuRead;
        rtYUVBufData.producerFlags               = ChiGralloc1ProducerUsageCamera;
        rtYUVBufData.maxBufferCount              = BufferQueueDepth;
        rtYUVBufData.immediateBufferCount        = CHIImmediateBufferCountZSL;
        rtYUVBufData.bEnableLateBinding          = ExtensionModule::GetInstance()->EnableCHILateBinding();
        rtYUVBufData.bufferHeap                  = BufferHeapDefault;

        CHIBufferManagerCreateData rtDS4BufData  = { 0 };
        rtDS4BufData.format                      = m_pTargetPreviewStream->format;
        rtDS4BufData.width                       =
            ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(m_pTargetPreviewStream->width, 4) / 4);
        rtDS4BufData.height                      =
            ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(m_pTargetPreviewStream->height, 4) / 4);
        rtDS4BufData.consumerFlags               = ChiGralloc1ConsumerUsageCamera;
        rtDS4BufData.producerFlags               = ChiGralloc1ProducerUsageCamera;
        rtDS4BufData.maxBufferCount              = BufferQueueDepth;
        rtDS4BufData.immediateBufferCount        = CHIImmediateBufferCountZSL;
        rtDS4BufData.bEnableLateBinding          = ExtensionModule::GetInstance()->EnableCHILateBinding();
        rtDS4BufData.bufferHeap                  = BufferHeapDefault;

        CHIBufferManagerCreateData rtDS16BufData = { 0 };
        rtDS16BufData.format                     = m_pTargetPreviewStream->format;
        rtDS16BufData.width                      =
            ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(m_pTargetPreviewStream->width, 16) / 16);
        rtDS16BufData.height                     =
            ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(m_pTargetPreviewStream->height, 16) / 16);
        rtDS16BufData.consumerFlags              = ChiGralloc1ConsumerUsageCamera;
        rtDS16BufData.producerFlags              = ChiGralloc1ProducerUsageCamera;
        rtDS16BufData.maxBufferCount             = BufferQueueDepth;
        rtDS16BufData.immediateBufferCount       = CHIImmediateBufferCountZSL;
        rtDS16BufData.bEnableLateBinding         = ExtensionModule::GetInstance()->EnableCHILateBinding();
        rtDS16BufData.bufferHeap                 = BufferHeapDefault;

        if (videoRes > previewRes)
        {
            rtYUVBufData.format     = m_pTargetVideoStream->format;
            rtYUVBufData.width      = m_pTargetVideoStream->width;
            rtYUVBufData.height     = m_pTargetVideoStream->height;

            rtDS4BufData.format  = m_pTargetVideoStream->format;
            rtDS4BufData.width   = ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(m_pTargetVideoStream->width, 4) / 4);
            rtDS4BufData.height  = ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(m_pTargetVideoStream->height, 4) / 4);

            rtDS16BufData.format = m_pTargetVideoStream->format;
            rtDS16BufData.width  =
                ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(m_pTargetVideoStream->width, 16) / 16);
            rtDS16BufData.height =
                ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(m_pTargetVideoStream->height, 16) / 16);
        }

        CHAR bufferManagerName[MaxStringLength64];
        for (UINT32 i = 0; i < m_numOfPhysicalDevices; i++)
        {
            MultiCameraResource*       pObj             = &m_multiCamResource[i];
            CHIBufferManagerCreateData bufCreateData    = {0};

            rtYUVBufData.format  = pObj->m_RTOutputYUVFormat;
            rtDS4BufData.format  = pObj->m_RTOutputDS4Format;
            rtDS16BufData.format = pObj->m_RTOutputDS16Format;

            rtYUVBufData.pChiStream  = pObj->m_pRTOutputYUVStream;
            rtDS4BufData.pChiStream  = pObj->m_pRTOutputDS4Stream;
            rtDS16BufData.pChiStream = pObj->m_pRTOutputDS16Stream;

            CdkUtils::SNPrintF(bufferManagerName, sizeof(bufferManagerName), "MCPreviewBufferManager_%d", i);
            pObj->m_pRTYUVBufferManager = CHIBufferManager::Create(bufferManagerName, &rtYUVBufData);

            CdkUtils::SNPrintF(bufferManagerName, sizeof(bufferManagerName), "MCPreviewDS4BufferManager_%d", i);
            pObj->m_pRTDS4BufferManager = CHIBufferManager::Create(bufferManagerName, &rtDS4BufData);

            CdkUtils::SNPrintF(bufferManagerName, sizeof(bufferManagerName), "MCPreviewDS16BufferManager_%d", i);
            pObj->m_pRTDS16BufferManager = CHIBufferManager::Create(bufferManagerName, &rtDS16BufData);

            if ((NULL == pObj->m_pRTYUVBufferManager) || (NULL == pObj->m_pRTDS4BufferManager) ||
                (NULL == pObj->m_pRTDS16BufferManager))
            {
                CHX_LOG_ERROR("Out of memory when creating RT buffer managers!");
                result = CDKResultENoMemory;
                break;
            }

            CHX_LOG_INFO("Realtime Pipeline %u Stream Info - Full Output [Fmt: %u][%u x %u] | "
                         "DS4 Output [Fmt: %u][%u x %u] | DS16 Output [Fmt: %u][%u x %u] ",
                         i, rtYUVBufData.format, rtYUVBufData.width, rtYUVBufData.height,
                         rtDS4BufData.format, rtDS4BufData.width, rtDS4BufData.height,
                         rtDS16BufData.format, rtDS16BufData.width, rtDS16BufData.height);

            if (TRUE == m_isOfflineRequired)
            {
                memset(&bufCreateData, 0, sizeof(bufCreateData));
                bufCreateData.format                  = HAL_PIXEL_FORMAT_RAW10;
                bufCreateData.width                   = pObj->m_RAWWidth;
                bufCreateData.height                  = pObj->m_RAWHeight;
                bufCreateData.consumerFlags           = ChiGralloc1ConsumerUsageCamera;
                bufCreateData.producerFlags           = ChiGralloc1ProducerUsageCamera;
                // Why ZSLQueueDepth * 2 + 6 for maxBufferCount:
                //  ZSL queue will be filled up with buffers and extra 6 buffers are needed to keep the preview running
                //  ZSLQueueDepth + 6 (maxHalRequests) is the minimum buffer count to support preview
                //  In case there is a MFNR snapshot request which taks all the RDIs from the zsl queue,
                //  we need to fill up the queue again with 'ZSLQueueDepth' RDIs buffers for the next snapshot.
                bufCreateData.maxBufferCount          = BufferQueueDepth;
                bufCreateData.immediateBufferCount    = CHIImmediateBufferCountZSL;
                bufCreateData.bEnableLateBinding      = ExtensionModule::GetInstance()->EnableCHILateBinding();
                bufCreateData.bufferHeap              = BufferHeapDefault;
                bufCreateData.pChiStream              = pObj->m_pRTOutputRDIStream;

                CdkUtils::SNPrintF(bufferManagerName, sizeof(bufferManagerName), "MCRDIBufferManager_%d", i);
                AddNewInternalBufferQueue(pObj->m_pRTOutputRDIStream, &bufCreateData, &m_RDIBufferID[i], bufferManagerName);

                // Only SAT w/o video mode can enable advance feature. So it will be decided during InitializeAdvanceFeatures().
                // And the rest of them are running UpdateValidRDIBufferLength with delta = 2
                if (FALSE == canEnableAdvanceFeature())
                {
                    UpdateValidRDIBufferLength(i, 2);
                }
            }

            memset(&bufCreateData, 0, sizeof(bufCreateData));
            bufCreateData                 = { 0 };
            bufCreateData.format          = ChiStreamFormatYCbCr420_888;
            bufCreateData.width           = pObj->m_pRTOutputFDStream->width;
            bufCreateData.height          = pObj->m_pRTOutputFDStream->height;
            bufCreateData.consumerFlags   = ChiGralloc1ConsumerUsageCamera;
            bufCreateData.producerFlags   = ChiGralloc1ProducerUsageCamera;
            // Why ZSLQueueDepth * 2 + 6 for maxBufferCount:
            //  ZSL queue will be filled up with buffers and extra 6 buffers are needed to keep the preview running
            //  ZSLQueueDepth + 6 (maxHalRequests) is the minimum buffer count to support preview
            //  In case there is a MFNR snapshot request which taks all the RDIs from the zsl queue,
            //  we need to fill up the queue again with 'ZSLQueueDepth' RDIs buffers for the next snapshot.
            bufCreateData.maxBufferCount       = BufferQueueDepth;
            bufCreateData.immediateBufferCount = CHIImmediateBufferCountZSL;
            bufCreateData.bEnableLateBinding   = ExtensionModule::GetInstance()->EnableCHILateBinding();
            bufCreateData.bufferHeap           = BufferHeapDefault;
            bufCreateData.pChiStream           = pObj->m_pRTOutputFDStream;

            CdkUtils::SNPrintF(bufferManagerName, sizeof(bufferManagerName), "MCFDBufferManager_%d", i);
            AddNewInternalBufferQueue(pObj->m_pRTOutputFDStream, &bufCreateData, &m_FDBufferID[i], bufferManagerName);

            // Only SAT w/o video mode can enable advance feature. So it will be decided during InitializeAdvanceFeatures().
            // And the rest of them are running UpdateValidFDBufferLength with delta = 2
            if (FALSE == canEnableAdvanceFeature())
            {
                UpdateValidFDBufferLength(i, 2);
            }
        }
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::Initialize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::Initialize(
    LogicalCameraInfo*              pCameraInfo,   ///< Camera info
    camera3_stream_configuration_t* pStreamConfig) ///< Stream configuration
{
    CDKResult result             = CDKResultSuccess;
    ChiUsecase* pChiUsecase      = NULL;
    LogicalCameraType cameraType = LogicalCameraType_Default;
    m_isPostViewNeeded           = FALSE;
    m_isVideoNeeded              = FALSE;
    UINT32 remappedPipelineIndex = 0;

    // Initialize the target streams to Null
    m_pTargetPreviewStream  = NULL;
    m_pTargetSnapshotStream = NULL;
    m_pTargetVideoStream    = NULL;
    m_pTargetPostviewStream = NULL;
    m_pTargetRAW16Stream    = NULL;
    m_pDeferSnapMetadataMutex = Mutex::Create();

    cameraType = ExtensionModule::GetInstance()->GetCameraType(pCameraInfo->cameraId);
    m_kernelFrameSyncEnable = ExtensionModule::GetInstance()->GetDCFrameSyncMode();
    ExtensionModule::GetInstance()->GetVendorTagOps(&m_vendorTagOps);

    ChiPortBufferDescriptor pipelineOutputBuffer[MaxChiStreams];
    ChiPortBufferDescriptor pipelineInputBuffer[MaxChiStreams];

    ChxUtils::Memset(pipelineOutputBuffer, 0, sizeof(pipelineOutputBuffer));
    ChxUtils::Memset(pipelineInputBuffer, 0, sizeof(pipelineInputBuffer));
    ChxUtils::Memset(m_jpegRequestId, 0, sizeof(m_jpegRequestId));
    ChxUtils::Memset(&m_prevMCCResult, 0, sizeof(m_prevMCCResult));
    ChxUtils::Memset(&m_MCCResult, 0, sizeof(m_MCCResult));
    ChxUtils::Memset(&m_multiCamResource, 0, sizeof(m_multiCamResource));

    m_bokehPrimaryJpeg               = FALSE;
    m_jpegRequestCount               = 0;
    m_primaryCameraIndex             = ExtensionModule::GetInstance()->GetPrimaryCameraIndex(pCameraInfo);
    m_offlinePipelineExpectationMask = 0;

    m_isIHDRSnapshotEnable = ExtensionModule::GetInstance()->EnableIHDRSnapshot();
    m_isIHDRAECTrigger     = FALSE;
    m_isIHDRSnapshot       = FALSE;
    m_stateIHDRSnapshot    = IHDR_NONE;
    m_numIHDRPreviewDelay  = 0;

    ChxUtils::Memset(&m_sessionInfo[0], 0, sizeof(m_sessionInfo));

    m_usecaseId             = UsecaseId::MultiCamera;
    m_cameraId              = pCameraInfo->cameraId;
    m_pLogicalCameraInfo    = pCameraInfo;
    m_numOfPhysicalDevices  = pCameraInfo->numPhysicalCameras;

    m_defaultSensorModePickHint.postSensorUpscale       = FALSE;
    m_defaultSensorModePickHint.sensorModeCaps.u.Normal = TRUE;

    CHX_LOG("%s, E.", __func__);

    /// initialize defer related resource.
    InitializeDeferResource();

    /// Work around. Accepting only preview
    for (UINT stream = 0; stream < pStreamConfig->num_streams; stream++)
    {
        pStreamConfig->streams[stream]->max_buffers = 8;
    }

    result = ClassifyTargetStream(pStreamConfig);
    if (CDKResultSuccess == result)
    {
        pChiUsecase = SelectUsecaseXML();
        if (NULL == pChiUsecase)
        {
            result = CDKResultEFailed;
        }
    }

    if (((TRUE == m_isRaw16Needed) || (TRUE == m_isVideoNeeded)) && (UsecaseRTB == m_usecaseMode))
    {
        CHX_LOG_ERROR("ERROR: RAW16 or Video recording is not supported in RTB usecase");
        result = CDKResultEFailed;
    }

    if (CDKResultSuccess == result)
    {
        CHX_LOG("[MultiCamera] usecase name = %s, numPipelines = %d numTargets = %d",
            pChiUsecase->pUsecaseName, pChiUsecase->numPipelines, pChiUsecase->numTargets);

        UINT32 rtPipelines = 0;
        for (UINT index = 0; index < pChiUsecase->numPipelines; index++)
        {
            ChiTargetPortDescriptorInfo* pSinkTarget = &pChiUsecase->pPipelineTargetCreateDesc[index].sinkTarget;
            ChiTargetPortDescriptorInfo* pSrcTarget  = &pChiUsecase->pPipelineTargetCreateDesc[index].sourceTarget;
            UINT sessionIndex = GetSessionIndex(index, m_usecaseMode);
            if (sessionIndex == REALTIME_SESSION)
            {
                m_multiCamResource[rtPipelines].m_RTOutputYUVFormat = ChiStreamFormatImplDefined;
                for (UINT32 target = 0; target < pSinkTarget->numTargets; target++)
                {
                    ChiBufferFormat targetFormat = pSinkTarget->pTargetPortDesc[target].pTarget->pBufferFormats[0];
                    if (ChiFormatP010 == targetFormat)
                    {
                        m_multiCamResource[rtPipelines].m_RTOutputYUVFormat = ChiStreamFormatP010;
                    }
                    else if (ChiFormatPD10 == targetFormat)
                    {
                        m_multiCamResource[rtPipelines].m_RTOutputDS4Format  = ChiStreamFormatPD10;
                        m_multiCamResource[rtPipelines].m_RTOutputDS16Format = ChiStreamFormatPD10;
                    }
                }
                rtPipelines++;
            }
        }

        result = CreateMultiCameraResource(pStreamConfig);
        if (CDKResultSuccess != result)
        {
            CHX_LOG_ERROR("Failed to Create Multi Camera Resource");
        }
    }
    else
    {
        CHX_LOG_ERROR("pChiUsecase is NULL or stream config is not supported");
        result = CDKResultEFailed;
    }

    if (CDKResultSuccess == result)
    {
        result = CreatePipelines(pChiUsecase, pStreamConfig);
        if (CDKResultSuccess != result)
        {
            CHX_LOG_ERROR("Failed to Create Pipelines");
        }
    }

    if (CDKResultSuccess == result)
    {
        // don't initialize advance features in video mode
        if (canEnableAdvanceFeature())
        {
            result = InitializeAdvanceFeatures(pCameraInfo, pStreamConfig);
            CHX_LOG("m_pVideo;%p,usecase:%d", m_pTargetVideoStream, m_usecaseMode);
        }
        else
        {
            result = Usecase::Initialize(false);

            if (CDKResultSuccess == result)
            {
                result = CreateMetadataManager(m_cameraId, false, NULL, true);
            }
        }

        result = CreateSession(REALTIME_SESSION, m_sessionInfo[REALTIME_SESSION].m_numPipelines,
            m_sessionInfo[REALTIME_SESSION].m_pPipeline);

        if (CDKResultSuccess == result)
        {
            // here start a new thread to do offline session create.
            StartDeferThread();

            result = CreateMultiController(pCameraInfo, pStreamConfig);
            if (CDKResultSuccess == result)
            {
                // create offline thread related resource
                if (TRUE == m_isOfflineRequired)
                {
                    CreateOfflineProcessResource();
                }
            }
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::InitializeAdvanceFeatures
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::InitializeAdvanceFeatures(
    LogicalCameraInfo*              pCameraInfo,
    camera3_stream_configuration_t* pStreamConfig)
{
    CDKResult result = CDKResultSuccess;
    camera3_stream_configuration_t* pAdvanceFeaturesStreamConfig;

    CHX_LOG("InitializeAdvanceFeatures: E.");

    // @todo: move to a funciton to pass CHIStreams to advance usecase
    // use camera id 0 first.
    for (UINT32 i = 0; i < m_numOfPhysicalDevices; i++)
    {
        m_pRdiStream[i] = m_multiCamResource[i].m_pRTOutputRDIStream;
        m_pFdStream[i]  = m_multiCamResource[i].m_pRTOutputFDStream;
    }

    pAdvanceFeaturesStreamConfig = static_cast<camera3_stream_configuration_t*>(
        CHX_CALLOC(sizeof(camera3_stream_configuration_t)));

    if (pAdvanceFeaturesStreamConfig)
    {
        pAdvanceFeaturesStreamConfig->streams = static_cast<camera3_stream_t**>(
            CHX_CALLOC(sizeof(camera3_stream_t*) * pStreamConfig->num_streams));
        pAdvanceFeaturesStreamConfig->num_streams = pStreamConfig->num_streams;
        pAdvanceFeaturesStreamConfig->operation_mode = pStreamConfig->operation_mode;
    }

    // Take a copy of original stream config and modify the stream config for YUV snapshot features.
    // For features, pass YUV stream as the target stream instead of JPEG stream.
    for (UINT32 i = 0; i < pStreamConfig->num_streams; i++)
    {
        if (pAdvanceFeaturesStreamConfig && pAdvanceFeaturesStreamConfig->streams)
        {
            if (UsecaseSelector::IsJPEGSnapshotStream(pStreamConfig->streams[i]))
            {
                pAdvanceFeaturesStreamConfig->streams[i] =
                    reinterpret_cast<camera3_stream_t*>(m_multiCamResource[m_maxJpegCameraId].m_pYUVOutputSnapshotStream);
            }
            else
            {
                pAdvanceFeaturesStreamConfig->streams[i] = pStreamConfig->streams[i];
            }
        }
    }

    result = AdvancedCameraUsecase::Initialize(
        pCameraInfo,
        pAdvanceFeaturesStreamConfig,
        UsecaseId::MultiCamera);

    if (NULL != pAdvanceFeaturesStreamConfig)
    {
        if (NULL != pAdvanceFeaturesStreamConfig->streams)
        {
            CHX_FREE(pAdvanceFeaturesStreamConfig->streams);
            pAdvanceFeaturesStreamConfig->streams = NULL;
        }
        CHX_FREE(pAdvanceFeaturesStreamConfig);
        pAdvanceFeaturesStreamConfig = NULL;
    }

    CHX_LOG("InitializeAdvanceFeatures: X.");
    return CDKResultSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::SkipPreviousFrame
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::SkipPreviousFrame(
    UINT32 anchorFrameNumber,
    UINT32 nSkipFrames)
{
    UINT32 internalFrameNum = anchorFrameNumber;
    for(UINT32 i = 0 ; i < nSkipFrames ; i++)
    {
        internalFrameNum = anchorFrameNumber - i - 1;
        CHX_LOG_INFO("IHDR skip preview result internalFrameNum  :%d", internalFrameNum);
        UINT32 internalFrameNumIndex = internalFrameNum %  MaxOutstandingRequests;
        m_requestMapInfo[internalFrameNumIndex].isSkipPreview            = TRUE;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::SendNopRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::SendNopRequest(
    UINT32           nFrames,
    ControllerResult mccResult)
{
    UINT32 internalFrameNum          = m_realtimeRequestID;
    UINT32 internalFrameNumIndex     = internalFrameNum %  MaxOutstandingRequests;
    CDKResult result = CDKResultSuccess;
    for (UINT32 i = 0 ; i < nFrames; i++)
    {
        camera3_capture_request_t request;
        camera3_stream_buffer_t   buffer;
        CHX_LOG_INFO("IHDR send nop request internalFrameNumber: %d", m_realtimeRequestID);

        internalFrameNum          = m_realtimeRequestID++;
        internalFrameNumIndex     = internalFrameNum %  MaxOutstandingRequests;
        request.frame_number      = internalFrameNum;
        request.output_buffers    = &buffer;
        request.num_output_buffers = 1;

        ChxUtils::Memset(&buffer, 0, sizeof(camera3_stream_buffer_t));
        buffer.stream = reinterpret_cast<camera3_stream_t*>(m_multiCamResource[
                GetActivePipelineIndex(mccResult.masterCameraId)].m_pRTOutputRDIStream);
        m_pTargetRDIStream = m_multiCamResource[
                GetActivePipelineIndex(mccResult.masterCameraId)].m_pRTOutputRDIStream;
        m_requestMapInfo[internalFrameNumIndex]                         = { 0 };
        m_requestMapInfo[internalFrameNumIndex].frameNumber             = 0;
        m_requestMapInfo[internalFrameNumIndex].masterCameraID          = mccResult.masterCameraId;
        m_requestMapInfo[internalFrameNumIndex].pFeature                = NULL;
        m_requestMapInfo[internalFrameNumIndex].triggerOfflineReprocess = FALSE;
        m_requestMapInfo[internalFrameNumIndex].isMetaReturnNeeded      = TRUE;
        m_requestMapInfo[internalFrameNumIndex].isPreviewReturnNeeded   = FALSE;
        m_requestMapInfo[internalFrameNumIndex].isShutterReturnNeeded   = TRUE;
        m_requestMapInfo[internalFrameNumIndex].isSnapshotReturnNeeded  = FALSE;
        m_requestMapInfo[internalFrameNumIndex].isSkipPreview           = FALSE;
        m_requestMapInfo[internalFrameNumIndex].snapshotRequestID       = INVALIDSEQUENCEID;
        m_requestMapInfo[internalFrameNumIndex].activePipelineID =
            GetActivePipelineIndex(mccResult.masterCameraId);

        result = GenerateRealtimeRequest(&request, NULL);

    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::ExecuteCaptureRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::ExecuteCaptureRequest(
    camera3_capture_request_t* pRequest)
{
    CDKResult result  = CDKResultSuccess;
    Feature* pFeature = NULL;

    UINT32 applicationFrameNum      = pRequest->frame_number;
    UINT32 applicationFrameNumIndex = applicationFrameNum % MaxOutstandingRequests;
    // Request to generate preview for now
    for (UINT i = 0; i < pRequest->num_output_buffers; i++)
    {
        CHX_LOG("Request : output buffers :%d frameNum :%d acquireFence: %d , ReleaseFence: %d Buffer: %p, status: %d",
            pRequest->num_output_buffers, pRequest->frame_number, pRequest->output_buffers[i].acquire_fence,
            pRequest->output_buffers[i].release_fence, pRequest->output_buffers[i].buffer,
            pRequest->output_buffers[i].status);
    }

    camera3_capture_result_t* pUsecaseResult = GetCaptureResult(applicationFrameNumIndex);

    pUsecaseResult->result                           = NULL;
    pUsecaseResult->frame_number                     = applicationFrameNum;
    pUsecaseResult->num_output_buffers               = 0;
    m_shutterTimestamp[applicationFrameNumIndex]     = 0;

    result = UpdateFeatureModeIndex(const_cast<camera_metadata_t*>(pRequest->settings));

    for (UINT32 rtIndex = 0; rtIndex < m_numOfPhysicalDevices; rtIndex++)
    {
        m_multiCamResource[rtIndex].m_pStickyMetadata->SetAndroidMetadata(pRequest->settings);
    }
    //Check if this is start of camera and update multicamera results wrt. zoom values.
    if ((m_prevMCCResult.activeMap == 0) && (NULL != pRequest->settings))
    {
        m_pMultiCamController->UpdateResults(m_multiCamResource[0].m_pStickyMetadata);
    }

    m_MCCResult = m_pMultiCamController->GetResult(m_multiCamResource[0].m_pStickyMetadata);

    //Don't enable feature in dual zone for SAT usecase.
    if (TRUE == canEnableAdvanceFeature())
    {
        // Check if the capture request is a snapshot request
        for (UINT32 stream = 0; stream < pRequest->num_output_buffers; stream++) {
            if ((CHISTREAM *)pRequest->output_buffers[stream].stream == m_pTargetSnapshotStream)
            {
                // Get the feature to support the snapshot
                pFeature = SelectFeatureToExecuteCaptureRequest(pRequest,
                    GetActivePipelineIndex(m_MCCResult.masterCameraId));

                // Currently only support the snapshot from HDR or MFNR feature.
                if ((NULL != pFeature) &&
                    (FeatureType::HDR  != pFeature->GetFeatureType()) &&
                    (FeatureType::MFNR != pFeature->GetFeatureType()) &&
                    (FeatureType::SWMF != pFeature->GetFeatureType()) &&
                    (FeatureType::MFSR != pFeature->GetFeatureType()))
                {
                    pFeature = NULL;
                }

                if (NULL != pFeature)
                {
                    UpdateAdvanceFeatureStatus(pFeature);
                }

                CHX_LOG("snapshot request, appFrameNum:%d, m_isLLSSnapshot:%d, m_isIHDRSnapshot:%d",
                                        applicationFrameNum, m_isLLSSnapshot, m_isIHDRSnapshot);

                break;
            }
        }
    }
    UINT32 internalFrameNum          = m_realtimeRequestID;
    UINT32 internalFrameNumIndex     = internalFrameNum %  MaxOutstandingRequests;
    UINT32 numOfSnapshotBuffers      = 0;

    // set feature status as busy for multiframe snapshot features
    if ((NULL != pFeature) && canEnableAdvanceFeature())
    {
        if ((FeatureType::MFNR == pFeature->GetFeatureType()) ||
            (FeatureType::SWMF == pFeature->GetFeatureType()) ||
            (FeatureType::HDR == pFeature->GetFeatureType()))
        {
            pFeature->SetFeatureStatus(FeatureStatus::BUSY);
        }
    }

    // Reject only snapshot request, let preview continue for that request.
    if (TRUE == m_rejectedSnapshotRequestList[pRequest->frame_number % MaxOutstandingRequests])
    {
        camera3_capture_request_t request;
        ChxUtils::Memcpy(&request, pRequest, sizeof(camera3_capture_request_t));
        internalFrameNum = m_realtimeRequestID++;
        internalFrameNumIndex = internalFrameNum %  MaxOutstandingRequests;
        request.frame_number = internalFrameNum;

        m_requestMapInfo[internalFrameNumIndex]                         = { 0 };
        m_requestMapInfo[internalFrameNumIndex].frameNumber             = applicationFrameNum;
        m_requestMapInfo[internalFrameNumIndex].masterCameraID          = m_MCCResult.masterCameraId;
        m_requestMapInfo[internalFrameNumIndex].pFeature                = NULL;
        m_requestMapInfo[internalFrameNumIndex].triggerOfflineReprocess = FALSE;
        m_requestMapInfo[internalFrameNumIndex].isMetaReturnNeeded      = TRUE;
        m_requestMapInfo[internalFrameNumIndex].isPreviewReturnNeeded   = hasPreviewStreamRequest(pRequest);;
        m_requestMapInfo[internalFrameNumIndex].isShutterReturnNeeded   = TRUE;
        m_requestMapInfo[internalFrameNumIndex].isSnapshotReturnNeeded  = hasSnapshotStreamRequest(pRequest);
        m_requestMapInfo[internalFrameNumIndex].isSkipPreview           = FALSE;
        m_requestMapInfo[internalFrameNumIndex].snapshotRequestID       = INVALIDSEQUENCEID;
        m_requestMapInfo[internalFrameNumIndex].numOfSnapshotBuffers    = numOfSnapshotBuffers;
        m_requestMapInfo[internalFrameNumIndex].activePipelineID =
            GetActivePipelineIndex(m_MCCResult.masterCameraId);

        result = GenerateRealtimeRequest(&request, NULL);
    }
    else
    {
        // @note: currently the feature only support wide sensor
        if ((NULL != pFeature) && canEnableAdvanceFeature())
        {
            m_featureAnchorIndex    = INVALID_INDEX;
            numOfSnapshotBuffers    = pFeature->GetRequiredFramesForSnapshot(NULL);

            if ((FeatureType::HDR == pFeature->GetFeatureType()) ||
                ((FeatureType::MFNR == pFeature->GetFeatureType()) && ((TRUE == m_isLLSSnapshot) ||
                                                                       (TRUE == m_isIHDRSnapshot))))
            {
                FeatureRequestType requestType = FeatureRequestType::COMMON;

                if (TRUE == m_isIHDRSnapshot)
                {
                    requestType = FeatureRequestType::IHDR;
                }
                else if (TRUE == m_isLLSSnapshot)
                {
                    requestType = FeatureRequestType::LLS;
                }

                FeatureRequestInfo requestInfo;
                requestInfo.numOfRequest = 0;

                pFeature->GetRequestInfo(pRequest, &requestInfo, requestType);

                for (UINT32 i = 0; i < requestInfo.numOfRequest; i++)
                {
                    internalFrameNum = m_realtimeRequestID++;
                    internalFrameNumIndex = internalFrameNum %  MaxOutstandingRequests;
                    requestInfo.request[i].frame_number = internalFrameNum;

                    m_requestMapInfo[internalFrameNumIndex]                         = { 0 };
                    m_requestMapInfo[internalFrameNumIndex].frameNumber             = applicationFrameNum;
                    m_requestMapInfo[internalFrameNumIndex].masterCameraID          = m_MCCResult.masterCameraId;
                    m_requestMapInfo[internalFrameNumIndex].pFeature                = pFeature;
                    m_requestMapInfo[internalFrameNumIndex].triggerOfflineReprocess = FALSE;
                    m_requestMapInfo[internalFrameNumIndex].isMetaReturnNeeded      = requestInfo.isReturnResult[i];
                    m_requestMapInfo[internalFrameNumIndex].isPreviewReturnNeeded   = requestInfo.isReturnResult[i];
                    m_requestMapInfo[internalFrameNumIndex].isShutterReturnNeeded   = FALSE;
                    m_requestMapInfo[internalFrameNumIndex].isSnapshotReturnNeeded  = requestInfo.isReturnResult[i];
                    m_requestMapInfo[internalFrameNumIndex].isSkipPreview           = FALSE;
                    m_requestMapInfo[internalFrameNumIndex].snapshotRequestID       = INVALIDSEQUENCEID;
                    m_requestMapInfo[internalFrameNumIndex].numOfSnapshotBuffers    = numOfSnapshotBuffers;
                    m_requestMapInfo[internalFrameNumIndex].activePipelineID        =
                        GetActivePipelineIndex(m_MCCResult.masterCameraId);

                    if (((FeatureType::HDR == pFeature->GetFeatureType()) || (TRUE == m_isIHDRSnapshot)) &&
                        (TRUE == requestInfo.isReturnResult[i]))
                    {
                        CHX_LOG("requestIdx:%d, skip preview frame for HDR/IHDR, AppFrameNum:%d, internalFrameNum:%d",
                            i, applicationFrameNum, internalFrameNum);
                        m_requestMapInfo[internalFrameNumIndex].isSkipPreview = TRUE;
                    }

                    if (0 == i)
                    {
                        m_requestMapInfo[internalFrameNumIndex].isShutterReturnNeeded = TRUE;

                        // IHDR state update and skip the preview for the bad frame during seamless transition
                        if (TRUE == m_isIHDRSnapshot)
                        {
                            SkipPreviousFrame(m_realtimeRequestID, 2);
                            CHX_LOG_INFO("Set m_stateIHDRSnapshot to IHDR_START, internalFrameNum %d", internalFrameNum);
                            m_stateIHDRSnapshot   = IHDR_START;
                            m_numIHDRPreviewDelay = NumOfIHDRPreviewDelay;
                        }
                    }

                    if (FeatureType::HDR == pFeature->GetFeatureType())
                    {
                        m_requestMapInfo[internalFrameNumIndex].triggerOfflineReprocess = TRUE;
                        CHX_LOG("send HDR subrequest:framenum:%d, requestID:%d, isreturn:%d",
                            applicationFrameNum, internalFrameNum, requestInfo.isReturnResult[i]);
                    }
                    else
                    {
                        if ((requestInfo.numOfRequest - 1) == i)
                        {
                            m_requestMapInfo[internalFrameNumIndex].triggerOfflineReprocess = TRUE;
                        }

                        CHX_LOG("send LLS/IHDR MFNR subrequest:framenum:%d, requestID:%d, isreturn:%d",
                            applicationFrameNum, internalFrameNum, requestInfo.isReturnResult[i]);
                    }

                    camera3_stream_buffer_t buffer;

                    if (FALSE == requestInfo.isReturnResult[i])
                    {
                        requestInfo.request[i].num_output_buffers = 1;
                        ChxUtils::Memset(&buffer, 0, sizeof(camera3_stream_buffer_t));

                        m_pTargetRDIStream =
                            m_multiCamResource[
                            m_requestMapInfo[internalFrameNumIndex].activePipelineID].m_pRTOutputRDIStream;

                        ///< @todo (CAMX-4113) Decouple CHISTREAM and camera3_stream
                        buffer.stream = reinterpret_cast<camera3_stream_t*>(m_pTargetRDIStream);
                        requestInfo.request[i].output_buffers = &buffer;
                    }

                    for (UINT32 rtIndex = 0; rtIndex < m_numOfPhysicalDevices; rtIndex++)
                    {
                        m_multiCamResource[rtIndex].m_pStickyMetadata->SetAndroidMetadata(pRequest->settings);
                    }

                    result = GenerateRealtimeRequest(&requestInfo.request[i], requestInfo.metadataInfo[i].pInputMetadata);
                    if (TRUE == isOfflineProcessRequired(pRequest, &m_requestMapInfo[internalFrameNumIndex]))
                    {
                        TriggerOfflineRequest(internalFrameNum, pRequest);
                    }
                }

            }
            else if (((FeatureType::MFNR == pFeature->GetFeatureType()) && (FALSE == m_isLLSSnapshot)) ||
                (FeatureType::SWMF == pFeature->GetFeatureType()) ||
                (FeatureType::MFSR == pFeature->GetFeatureType()))
            {
                CHX_LOG("Send MFNR/SWMF/MFSR request:%d", internalFrameNum);

                camera3_capture_request_t request;
                ChxUtils::Memcpy(&request, pRequest, sizeof(camera3_capture_request_t));
                internalFrameNum = m_realtimeRequestID++;
                internalFrameNumIndex = internalFrameNum %  MaxOutstandingRequests;
                request.frame_number = internalFrameNum;

                m_requestMapInfo[internalFrameNumIndex]                          = { 0 };
                m_requestMapInfo[internalFrameNumIndex].frameNumber              = applicationFrameNum;
                m_requestMapInfo[internalFrameNumIndex].masterCameraID           = m_MCCResult.masterCameraId;
                m_requestMapInfo[internalFrameNumIndex].pFeature                 = pFeature;
                m_requestMapInfo[internalFrameNumIndex].triggerOfflineReprocess  = TRUE;
                m_requestMapInfo[internalFrameNumIndex].isMetaReturnNeeded       = TRUE;
                m_requestMapInfo[internalFrameNumIndex].isPreviewReturnNeeded    = hasPreviewStreamRequest(pRequest);;
                m_requestMapInfo[internalFrameNumIndex].isShutterReturnNeeded    = TRUE;
                m_requestMapInfo[internalFrameNumIndex].isSnapshotReturnNeeded   = TRUE;
                m_requestMapInfo[internalFrameNumIndex].isSkipPreview            = FALSE;
                m_requestMapInfo[internalFrameNumIndex].snapshotRequestID        = INVALIDSEQUENCEID;
                m_requestMapInfo[internalFrameNumIndex].numOfSnapshotBuffers     = numOfSnapshotBuffers;
                m_requestMapInfo[internalFrameNumIndex].activePipelineID         =
                    GetActivePipelineIndex(m_MCCResult.masterCameraId);

                for (UINT32 stream = 0; stream < pRequest->num_output_buffers; stream++)
                {
                    if ((CHISTREAM *)pRequest->output_buffers[stream].stream == m_pTargetSnapshotStream)
                    {
                        ChxUtils::Memcpy(&m_appSnapshotBuffer[applicationFrameNumIndex],
                            &pRequest->output_buffers[stream], sizeof(camera3_stream_buffer_t));
                    }
                }

                result = GenerateRealtimeRequest(&request, NULL);
                if (TRUE == isOfflineProcessRequired(pRequest, &m_requestMapInfo[internalFrameNumIndex]))
                {
                    TriggerOfflineRequest(internalFrameNum, pRequest);
                }
            }
        }
        else
        {
            camera3_capture_request_t request;
            ChxUtils::Memcpy(&request, pRequest, sizeof(camera3_capture_request_t));
            internalFrameNum = m_realtimeRequestID++;
            internalFrameNumIndex = internalFrameNum %  MaxOutstandingRequests;
            request.frame_number = internalFrameNum;

            m_requestMapInfo[internalFrameNumIndex]                         = { 0 };
            m_requestMapInfo[internalFrameNumIndex].frameNumber             = applicationFrameNum;
            m_requestMapInfo[internalFrameNumIndex].masterCameraID          = m_MCCResult.masterCameraId;
            m_requestMapInfo[internalFrameNumIndex].pFeature                = NULL;
            m_requestMapInfo[internalFrameNumIndex].triggerOfflineReprocess = FALSE;
            m_requestMapInfo[internalFrameNumIndex].isMetaReturnNeeded      = TRUE;
            m_requestMapInfo[internalFrameNumIndex].isPreviewReturnNeeded   = hasPreviewStreamRequest(pRequest);;
            m_requestMapInfo[internalFrameNumIndex].isShutterReturnNeeded   = TRUE;
            m_requestMapInfo[internalFrameNumIndex].isSnapshotReturnNeeded  = hasSnapshotStreamRequest(pRequest);
            m_requestMapInfo[internalFrameNumIndex].isSkipPreview           = FALSE;
            m_requestMapInfo[internalFrameNumIndex].snapshotRequestID       = INVALIDSEQUENCEID;
            m_requestMapInfo[internalFrameNumIndex].numOfSnapshotBuffers    = numOfSnapshotBuffers;
            m_requestMapInfo[internalFrameNumIndex].activePipelineID        =
                GetActivePipelineIndex(m_MCCResult.masterCameraId);

            if (TRUE == m_isIHDRSnapshot)
            {
                if (0 == m_numIHDRPreviewDelay)
                {
                    // Set m_stateIHDRSnapshot to IHDR_STOP, trigger IHDR stop sequence and end the IHDR snapshot
                    m_stateIHDRSnapshot = IHDR_STOP;
                    m_isIHDRSnapshot    = FALSE;
                    CHX_LOG_INFO("Set m_stateIHDRSnapshot to IHDR_STOP, internalFrameNum = %u", internalFrameNum);
                }
                else
                {
                    CHX_LOG_VERBOSE("remaining m_numIHDRPreviewDelay = %u", m_numIHDRPreviewDelay);
                    m_requestMapInfo[internalFrameNumIndex].isSkipPreview = TRUE;
                    --m_numIHDRPreviewDelay;
                }
            }
            else
            {
                m_stateIHDRSnapshot = IHDR_NONE;
            }

            result = GenerateRealtimeRequest(&request, NULL);
            if (TRUE == isOfflineProcessRequired(pRequest, &m_requestMapInfo[internalFrameNumIndex]))
            {
                TriggerOfflineRequest(internalFrameNum, pRequest);
            }

        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::NotifyResultsAvailable
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::NotifyResultsAvailable(
    const CHICAPTURERESULT* pResult)
{
    // ashvind - this function submits the offline preview request.
    CDKResult           result                  = CDKResultSuccess;
    UINT32              pipelineIndex           = pResult->pPrivData->requestIndex;
    UINT32              internalFrameNum        = pResult->frameworkFrameNum;
    UINT32              internalFrameNumIndex   = internalFrameNum %  MaxOutstandingRequests;
    BOOL                hasPreviewStream        = FALSE;
    BOOL                hasDS4Stream            = FALSE;
    BOOL                hasDS16Stream           = FALSE;
    CHISTREAMBUFFER*    rtPreviewBuffer         = NULL;
    CHISTREAMBUFFER*    rtDS4Buffer             = NULL;
    CHISTREAMBUFFER*    rtDS16Buffer            = NULL;
    ChiMetadata*        pChiResultMetadata      = NULL;
    SessionInfo*        pSessionInfo            = &m_sessionInfo[REALTIME_SESSION];
    if (MaxRealTimePipelines <= pipelineIndex)
    {
        CHX_LOG_ERROR("Result: Cannot Accept pipeline index of more than 1 frame = %d", pResult->frameworkFrameNum);
        // Early return
        return CDKResultEFailed;
    }

    for (UINT32 i = 0; i < pResult->numOutputBuffers; i++)
    {
        if (IsRTPreviewStream(pResult->pOutputBuffers[i].pStream))
        {
            hasPreviewStream = TRUE;
            rtPreviewBuffer  = const_cast<CHISTREAMBUFFER*>(&pResult->pOutputBuffers[i]);
        }
        else if (IsRTPreviewDS4Stream(pResult->pOutputBuffers[i].pStream))
        {
            hasDS4Stream     = TRUE;
            rtDS4Buffer      = const_cast<CHISTREAMBUFFER*>(&pResult->pOutputBuffers[i]);
        }
        else if (IsRTPreviewDS16Stream(pResult->pOutputBuffers[i].pStream))
        {
            hasDS16Stream    = TRUE;
            rtDS16Buffer     = const_cast<CHISTREAMBUFFER*>(&pResult->pOutputBuffers[i]);
        }
    }

    if (NULL != pResult->pOutputMetadata)
    {
        pChiResultMetadata = m_pMetadataManager->GetMetadataFromHandle(pResult->pOutputMetadata);

        // Receive Realtime pipeline result
        if (TRUE == m_isOfflineRequired)
        {
            pSessionInfo->m_bufferQueue[internalFrameNumIndex].pMetadata[pipelineIndex] =
                FillMetadataForRDIQueue(internalFrameNum, pipelineIndex, pChiResultMetadata);
        }
        else
        {
            pSessionInfo->m_bufferQueue[internalFrameNumIndex].pMetadata[pipelineIndex] = pChiResultMetadata;
        }

        pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex]      |= META_READY_FLAG;

        CHX_LOG("Preview pipeline idx:%d meta is ready,value:%x", pipelineIndex,
            pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex]);
    }

    if ((TRUE == hasPreviewStream) && (NULL != rtPreviewBuffer))
    {
        // Receive Realtime pipeline result
        pSessionInfo->m_bufferQueue[internalFrameNumIndex].frameNumber = internalFrameNum;
        if (0 < pResult->numOutputBuffers)
        {
            ChxUtils::Memcpy(&pSessionInfo->m_bufferQueue[internalFrameNumIndex].buffer[pipelineIndex],
                rtPreviewBuffer,
                sizeof(CHISTREAMBUFFER));

            pSessionInfo->m_bufferQueue[internalFrameNumIndex].buffer[pipelineIndex].pStream =
                m_multiCamResource[pipelineIndex].m_pYUVInputStream;

            pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex]         |= BUF_READY_FLAG;
        }

        CHX_LOG("Preview pipeline idx:%d buffer is ready,value:%x", pipelineIndex,
            pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex]);
    }

    if ((TRUE == hasDS4Stream) && (NULL != rtDS4Buffer))
    {
        // Receive Realtime pipeline result
        pSessionInfo->m_bufferQueue[internalFrameNumIndex].frameNumber = internalFrameNum;
        if (pResult->numOutputBuffers > 0)
        {
            ChxUtils::Memcpy(&pSessionInfo->m_bufferQueue[internalFrameNumIndex].ds4Buffer[pipelineIndex],
                rtDS4Buffer,
                sizeof(CHISTREAMBUFFER));

            pSessionInfo->m_bufferQueue[internalFrameNumIndex].ds4Buffer[pipelineIndex].pStream =
                m_multiCamResource[pipelineIndex].m_pDS4InputStream;

            pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex] |= DS4_BUF_READY_FLAG;
        }

        CHX_LOG("DS4 pipeline idx:%d buffer is ready,value:%x", pipelineIndex,
            pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex]);
    }

    if ((TRUE == hasDS16Stream) && (NULL != rtDS16Buffer))
    {
        // Receive Realtime pipeline result
        pSessionInfo->m_bufferQueue[internalFrameNumIndex].frameNumber = internalFrameNum;
        if (pResult->numOutputBuffers > 0)
        {
            ChxUtils::Memcpy(&pSessionInfo->m_bufferQueue[internalFrameNumIndex].ds16Buffer[pipelineIndex],
                rtDS16Buffer,
                sizeof(CHISTREAMBUFFER));

            pSessionInfo->m_bufferQueue[internalFrameNumIndex].ds16Buffer[pipelineIndex].pStream =
                m_multiCamResource[pipelineIndex].m_pDS16InputStream;

            pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex] |= DS16_BUF_READY_FLAG;
        }

        CHX_LOG("DS16 pipeline idx:%d buffer is ready,value:%x", pipelineIndex,
            pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex]);
    }

    const UINT readyMask  = META_READY_FLAG | BUF_READY_FLAG | m_offlinePipelineExpectationMask;
    const UINT readyFlags = pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[pipelineIndex];

    // check if the preview result is ready for this pipeline
    if ((readyMask & readyFlags) == readyMask)
    {
        pSessionInfo->m_bufferQueue[internalFrameNumIndex].m_resultMask =
            ChxUtils::BitSet(pSessionInfo->m_bufferQueue[internalFrameNumIndex].m_resultMask, pipelineIndex);
    }

    // if result map is equal with request map, it means all result for preview is ready, go to offline pipeline
    if ((0 != m_requestInfo[internalFrameNumIndex].m_previewMask) &&
        (m_requestInfo[internalFrameNumIndex].m_previewMask ==
        pSessionInfo->m_bufferQueue[internalFrameNumIndex].m_resultMask))
    {
        UINT32            numOfInputs = 0;
        ChiMetadata*      pResultMetadata[MaxChiStreams];
        CHISTREAMBUFFER   resultBuffer[MaxChiStreams];
        CHIBufferManager* pBufferManager[MaxChiStreams];
        ChxUtils::Memset(resultBuffer, 0, sizeof(resultBuffer));
        ChxUtils::Memset(pResultMetadata, 0, sizeof(pResultMetadata));

        // trigger reprocessing
        CHISTREAMBUFFER     outputBuffers[2] = { {0}, {0} };
        CHICAPTUREREQUEST   request          = {0};
        UINT32              buffercount      = 0;

        // wait for offline preview session create done. it should be done in defer thread.
        if ((FALSE == m_offlinePreviewSessionCreateDone) &&
            (INVALID_THREAD_HANDLE != m_deferSnapSessionThreadData.hThreadHandle))
        {
            m_pOfflinePreviewSessionCreateDoneMutex->Lock();
            if (FALSE == m_offlinePreviewSessionCreateDone)
            {
                result = m_pOfflinePreviewSessionCreateDoneCondition->TimedWait(
                    m_pOfflinePreviewSessionCreateDoneMutex->GetNativeHandle(), 700);
                if (result != CDKResultSuccess)
                {
                    CHX_LOG_ERROR("Fatal error: offline preview pipeline resource is not ready!");
                }
            }
            m_pOfflinePreviewSessionCreateDoneMutex->Unlock();
        }

        if (CDKResultSuccess == result)
        {
            // TARGET_PREVIEW
            ChxUtils::PopulateHALToChiStreamBuffer(&m_appPreviewBuffer[internalFrameNumIndex], &outputBuffers[buffercount]);
            buffercount++;

            if (ChxUtils::IsBitSet(m_requestInfo[internalFrameNumIndex].m_videoMask, pipelineIndex))
            {
                // TARGET_VIDEO
                ChxUtils::PopulateHALToChiStreamBuffer(&m_appVideoBuffer[internalFrameNumIndex], &outputBuffers[buffercount]);
                buffercount++;
            }

            request.frameNumber             = pSessionInfo->m_bufferQueue[internalFrameNumIndex].frameNumber;
            request.hPipelineHandle         = reinterpret_cast<CHIPIPELINEHANDLE>(
                m_sessionInfo[OFFLINE_YUV_SESSION].m_pSession->GetPipelineHandle(0));
            request.numOutputs              = buffercount;
            request.pOutputBuffers          = outputBuffers;
            request.pPrivData               = &pSessionInfo->m_chiPrivData[0][internalFrameNumIndex];
            request.pPrivData->streamIndex  = 0;

            ChiMetadata* pChiOutputMetadata = m_pMetadataManager->Get(
                m_sessionInfo[OFFLINE_YUV_SESSION].m_pSession->GetMetadataClientId(),
                pResult->frameworkFrameNum);

            ChiMetadata* pOfflineRTInputMeta = m_pOfflinePipelineInputMetadataPreview[
                pResult->frameworkFrameNum % MAX_EMPTY_RTMETA_COUNT];

            if ((NULL == pChiOutputMetadata) || (NULL == pOfflineRTInputMeta))
            {
                CHX_LOG_ERROR("[CMB_ERROR] metadata (%p %p)", pOfflineRTInputMeta, pChiOutputMetadata);
                return CDKResultEFailed;
            }

            CHX_LOG("Metadata internalFrameNumIndex %d (%p %p %p)", internalFrameNumIndex,
                   pSessionInfo->m_bufferQueue[internalFrameNumIndex].pMetadata[0],
                   pSessionInfo->m_bufferQueue[internalFrameNumIndex].pMetadata[1],
                   pOfflineRTInputMeta);

#ifdef __MC_DUMP_META__
            CHAR metaFileName[MaxFileLen];
            for (UINT32 i = 0 ; i < m_numOfPhysicalDevices ; i++)
            {
                CdkUtils::SNPrintF(metaFileName, sizeof(metaFileName), "rtout_pipeline%d_%d.txt", i, internalFrameNum);
                pSessionInfo->m_bufferQueue[internalFrameNumIndex].pMetadata[i]->DumpDetailsToFile(metaFileName);
            }
#endif

            UINT numOfMetadata = 0;
            for (UINT32 i = 0 ; i < m_numOfPhysicalDevices ; i++)
            {
                if (ChxUtils::IsBitSet(pSessionInfo->m_bufferQueue[internalFrameNumIndex].m_resultMask, i))
                {
                    pResultMetadata[numOfMetadata] = pSessionInfo->m_bufferQueue[internalFrameNumIndex].pMetadata[i];
                    resultBuffer[numOfInputs]    = pSessionInfo->m_bufferQueue[internalFrameNumIndex].buffer[i];
                    pBufferManager[numOfInputs]  = m_multiCamResource[i].m_pRTYUVBufferManager;
                    numOfInputs++;
                    numOfMetadata++;

                    if (TRUE == IsDS4RequiredOffline())
                    {
                        resultBuffer[numOfInputs]   = pSessionInfo->m_bufferQueue[internalFrameNumIndex].ds4Buffer[i];
                        pBufferManager[numOfInputs] = m_multiCamResource[i].m_pRTDS4BufferManager;
                        numOfInputs++;
                    }

                    if (TRUE == IsDS16RequiredOffline())
                    {
                        resultBuffer[numOfInputs]   = pSessionInfo->m_bufferQueue[internalFrameNumIndex].ds16Buffer[i];
                        pBufferManager[numOfInputs] = m_multiCamResource[i].m_pRTDS16BufferManager;
                        numOfInputs++;
                    }

                    pSessionInfo->m_bufferQueue[internalFrameNumIndex].valid[i] = 0;
                }
            }

            request.numInputs     = numOfInputs;
            request.pInputBuffers = resultBuffer;

            // Save input buffers info for later releasing reference
            for (UINT32 i = 0 ; i < numOfInputs ; i++)
            {
                request.pPrivData->inputBuffers[i]    = resultBuffer[i].bufferInfo;
                request.pPrivData->bufferManagers[i]  = pBufferManager[i];
            }
            request.pPrivData->numInputBuffers        = numOfInputs;

            AppendOfflineMetadata(pResult->frameworkFrameNum,
                        pResultMetadata[0],
                        pResultMetadata[1],
                        pOfflineRTInputMeta,
                        FALSE,
                        FALSE);

            request.pInputMetadata  = pOfflineRTInputMeta->GetHandle();
            request.pOutputMetadata = pChiOutputMetadata->GetHandle();

            CHX_LOG_INFO("OfflinePreview: reprocessing frameNum = %" PRIu64 " buffercount %d, requestmap:0x%x",
                request.frameNumber, buffercount, m_requestInfo[internalFrameNumIndex].m_previewMask);

            CHX_LOG_REQMAP("frame: %u  <==>  chiOverrideFrameNum: %u <==>  internalFrameNum: %u  <==>  chiFrameNum: %" PRIu64
                            " -- Offline YUV Session",
                            GetAppFrameNum(m_requestMapInfo[internalFrameNumIndex].frameNumber),
                            m_requestMapInfo[internalFrameNumIndex].frameNumber,
                            internalFrameNum,
                            request.frameNumber);

            processSessionRequest(OFFLINE_YUV_SESSION, 1, &request);
            //clear result map
            m_requestInfo[internalFrameNumIndex].m_previewMask = 0;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::ProcessResults
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::ProcessResults()
{

}

CDKResult UsecaseMultiCamera::processSessionRequest(
   UINT32 sessionId, UINT numRequest, CHICAPTUREREQUEST* pRequest)
{
    CDKResult          result = CDKResultSuccess;

    CHIPIPELINEREQUEST submitRequest;
    SessionInfo *pSessionInfo = &m_sessionInfo[sessionId];


    for (UINT32 i = 0; i < numRequest; i++)
    {
       CHX_LOG("Request SessionID %d, i %d, PipelineIndex %d, frame %" PRIu64 ", numInputs %d, numOutputs %d",
               sessionId, i, pRequest[i].pPrivData->streamIndex,
               pRequest[i].frameNumber, pRequest[i].numInputs, pRequest[i].numOutputs);
    }
    UINT32 frameIndex = pRequest[0].frameNumber % MaxOutstandingRequests;
    ChxUtils::Memset(&pSessionInfo->m_bufferQueue[frameIndex], 0, sizeof(BufferQueue));
    submitRequest.pSessionHandle   = reinterpret_cast<CHIHANDLE>(pSessionInfo->m_pSession->GetSessionHandle());
    submitRequest.numRequests      = numRequest;
    submitRequest.pCaptureRequests = pRequest;
    result = SubmitRequest(&submitRequest);

    if (CDKResultSuccess == result)
    {
        for (UINT32 i = 0; i < numRequest; i++)
        {
            UINT32 pipelineIndex =
                pSessionInfo->m_pSession->GetPipelineIndex(pRequest[i].hPipelineHandle);
            pSessionInfo->m_waitUntilDrain[pipelineIndex].m_pSyncMutex->Lock();
            if (0 <= pSessionInfo->m_waitUntilDrain[pipelineIndex].m_syncCounter)
            {
                pSessionInfo->m_waitUntilDrain[pipelineIndex].m_syncCounter += pRequest[i].numOutputs;
            }
            pSessionInfo->m_waitUntilDrain[pipelineIndex].m_pSyncMutex->Unlock();
        }
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::ActivatePendingPipeline
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::ActivatePendingPipeline()
{
    CDKResult result = CDKResultSuccess;
    for (UINT32 pipelineIndex = 0; pipelineIndex < m_numOfPhysicalDevices; pipelineIndex++)
    {
        if ((TRUE == ChxUtils::IsBitSet(m_pendingActiveMask, pipelineIndex)) &&
            (FALSE == m_sessionInfo[REALTIME_SESSION].m_pSession->IsPipelineActive(pipelineIndex)))
        {
            CHX_LOG_INFO("Activating Pipeline %d", pipelineIndex);

            ATRACE_BEGIN("ActivatePipeline");
            result = ExtensionModule::GetInstance()->ActivatePipeline(
                m_sessionInfo[REALTIME_SESSION].m_pSession->GetSessionHandle(),
                m_sessionInfo[REALTIME_SESSION].m_pSession->GetPipelineHandle(pipelineIndex));
            ATRACE_END();

            if (CDKResultSuccess == result)
            {
                CHX_LOG_INFO("Success activating for pipeline %d", pipelineIndex);

                m_sessionInfo[REALTIME_SESSION].m_pSession->SetPipelineActivateFlag(pipelineIndex);
                m_multiCamResource[pipelineIndex].m_stickyMetaNeeded = TRUE;
                m_curActiveMap |= 1 << pipelineIndex;
            }
            else
            {
                CHX_LOG_INFO("Activating Pipeline %d failed", pipelineIndex);
            }
        }
    }
    m_pendingActiveMask = 0;
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::GeneratePreviewRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::GenerateRealtimeRequest(
   camera3_capture_request_t* pRequest,
   ChiMetadata*               pFeatureInputMetadata)
{
    CHISTREAMBUFFER     streamBuffer[MaxDevicePerLogicalCamera][6];
    CHICAPTUREREQUEST   chiRequest[MaxRealTimePipelines];
    CDKResult           result                   = CDKResultSuccess;
    CHISTREAMBUFFER*    pBuffer                  = NULL;
    BOOL                LPMDecisionChanged       = FALSE;
    CHICAPTUREREQUEST*  request                  = NULL;
    UINT32              internalFrameNum         = pRequest->frame_number;
    UINT32              internalFrameNumIndex    = internalFrameNum % MaxOutstandingRequests;
    SessionInfo*        pSessionInfo             = &m_sessionInfo[REALTIME_SESSION];
    UINT32              applicationFrameNum      = m_requestMapInfo[internalFrameNumIndex].frameNumber;
    UINT32              applicationFrameNumIndex = applicationFrameNum % MaxOutstandingRequests;
    UINT32              numOfRequest             = 0;
    UINT32              numOfBuffers[MaxDevicePerLogicalCamera];

    ChxUtils::Memset(chiRequest, 0, sizeof(chiRequest));
    ChxUtils::Memset(streamBuffer, 0 , sizeof(streamBuffer));
    ChxUtils::Memset(numOfBuffers, 0 , sizeof(numOfBuffers));
    ChxUtils::Memset(&m_requestInfo[internalFrameNumIndex], 0, sizeof(RequestInfo));

    if (CDKResultSuccess != ActivateDeactivateRealtimePipeline(&m_MCCResult, internalFrameNum, pRequest))
    {
        return result;
    }

    // Check whether previous and current mcc result has any change to send the DC vendor tag settings in metadata
    if ((m_prevMCCResult.activeMap != m_MCCResult.activeMap) ||
        (m_prevMCCResult.masterCameraId != m_MCCResult.masterCameraId))
    {
       LPMDecisionChanged = TRUE;

       CHX_LOG("LPMDecisionChanged previous state (%d, %d) current state (%d, %d) previous master %d, current master %d",
               m_prevMCCResult.activeCameras[0].isActive, m_prevMCCResult.activeCameras[1].isActive,
               m_MCCResult.activeCameras[0].isActive, m_MCCResult.activeCameras[1].isActive,
               m_prevMCCResult.masterCameraId, m_MCCResult.masterCameraId );
    }

    // Reset previous MCC result with latest result
    m_prevMCCResult = m_MCCResult;

    ChiMetadata* pChiRTInputMetadata[MaxRealTimePipelines];
    ChiMetadata* pChiRTOutputMetadata[MaxRealTimePipelines];

    for (UINT32 rtIndex = 0 ; rtIndex < m_numOfPhysicalDevices ; rtIndex++)
    {
        pChiRTInputMetadata[rtIndex]  = m_pMetadataManager->GetInput(
            NULL, pRequest->frame_number, false, false);

        pChiRTOutputMetadata[rtIndex] = m_pMetadataManager->Get(
            pSessionInfo->m_pSession->GetMetadataClientId(rtIndex),
            pRequest->frame_number );

        if ((NULL == pChiRTInputMetadata[rtIndex]) || (NULL == pChiRTOutputMetadata[rtIndex]))
        {
            CHX_LOG_ERROR("[CMB_ERROR] cannot generate real time request metadata for pipeline:%d,(%p,%p)",
                          rtIndex,
                          pChiRTInputMetadata[rtIndex],
                          pChiRTOutputMetadata[rtIndex]);

            return CDKResultEFailed;
        }

        UINT32 sensorModeIndex = pSessionInfo->m_pSession->GetSensorModeInfo(rtIndex)->modeIndex;
        ChxUtils::FillTuningModeData(m_multiCamResource[rtIndex].m_pStickyMetadata,
                                     pRequest,
                                     sensorModeIndex,
                                     &m_multiCamResource[rtIndex].m_effectModeValue,
                                     &m_multiCamResource[rtIndex].m_sceneModeValue,
                                     &m_multiCamResource[rtIndex].m_tuningFeature1Value,
                                     &m_multiCamResource[rtIndex].m_tuningFeature2Value);
    }

    if ((NULL != pRequest->settings) || (TRUE == LPMDecisionChanged))
    {
        CHX_LOG_INFO( "settings not NULL or LPMDecisionChanged, internalFrameNum %d", internalFrameNum );
        // Translate request settings
        MulticamReqSettings multiCamSettings;
        multiCamSettings.numSettingsBuffers       = m_numOfPhysicalDevices;
        multiCamSettings.kernelFrameSyncEnable    = m_kernelFrameSyncEnable;
        multiCamSettings.ppSettings               = static_cast<ChiMetadata**>( CHX_CALLOC(
          multiCamSettings.numSettingsBuffers * sizeof(ChiMetadata*)));

        multiCamSettings.frameNum = pRequest->frame_number;
        // Send current mcc result which ensures same master & active camera info throught out the life of this request
        multiCamSettings.currentRequestMCCResult = m_MCCResult;

        for (UINT32 rtIndex = 0; rtIndex < m_numOfPhysicalDevices; rtIndex++)
        {
            //Set metadata owner info
            UINT32 cameraId = pSessionInfo->m_pSession->GetCameraId(rtIndex);
            m_multiCamResource[rtIndex].m_pStickyMetadata->SetTag("com.qti.chi.metadataOwnerInfo", "MetadataOwner",
                                                                  &cameraId, sizeof(MetadataOwnerInfo));

            if (NULL != pFeatureInputMetadata)
            {
                m_multiCamResource[rtIndex].m_pStickyMetadata->Copy(*pFeatureInputMetadata);
            }

            if (NULL != multiCamSettings.ppSettings)
            {
                multiCamSettings.ppSettings[rtIndex] = m_multiCamResource[rtIndex].m_pStickyMetadata;
            }
        }

        multiCamSettings.frameNum = pRequest->frame_number;
        // Send current mcc result which ensures same master & active camera info throught out the life of this request
        multiCamSettings.currentRequestMCCResult = m_MCCResult;

        if (pRequest->settings != NULL)
        {
            if ( CDKResultSuccess != m_pMultiCamController->TranslateRequestSettings(&multiCamSettings))
            {
                CHX_LOG_ERROR( "Error in translating request settings" );
            }
        }
        else if(TRUE == LPMDecisionChanged)
        {
            m_pMultiCamController->UpdateVendorTags(&multiCamSettings);
        }
        if(NULL != multiCamSettings.ppSettings)
        {
            CHX_FREE(multiCamSettings.ppSettings);
            multiCamSettings.ppSettings = NULL;
        }
    }

    for (UINT32 rtIndex = 0; rtIndex < m_numOfPhysicalDevices; rtIndex++)
    {
        pChiRTInputMetadata[rtIndex]->Copy(*(m_multiCamResource[rtIndex].m_pStickyMetadata));
        //< Set IHDR tags if needed
        result = pChiRTInputMetadata[rtIndex]->SetTag("com.qti.ihdr_control", "state_ihdr_snapshot", &m_stateIHDRSnapshot, 1);
        CHX_ASSERT(CamxResultSuccess == result);

#ifdef __MC_DUMP_META__
        char metaFileName[MaxFileLen];
        CdkUtils::SNPrintF( metaFileName, sizeof( metaFileName ), "rtin_pipeline:%d_%d.txt",
            rtIndex, internalFrameNum );
        pChiRTInputMetadata[rtIndex]->DumpDetailsToFile( metaFileName );
#endif
    }

    if (IHDR_START == m_stateIHDRSnapshot)
    {
        CHX_LOG_INFO("Set m_stateIHDRSnapshot to IHDR_ENABLED, internalFrameNum %d", internalFrameNum);
        m_stateIHDRSnapshot = IHDR_ENABLED;
    }

    m_requestInfo[internalFrameNumIndex] = { 0 };

    m_requestInfo[internalFrameNumIndex].masterPipelineIndex =
        pSessionInfo->m_pSession->GetPipelineIndex(m_MCCResult.masterCameraId);
    pSessionInfo->m_bufferQueue[internalFrameNumIndex]          = { 0 };

    CHX_ASSERT(m_requestInfo[internalFrameNumIndex].masterPipelineIndex < m_numOfPhysicalDevices);



    for (UINT32 i = 0; i < m_numOfPhysicalDevices; i++)
    {
        if (pSessionInfo->m_pSession->GetCameraId(i) ==
            m_MCCResult.activeCameras[i].cameraId)
        {
            m_requestInfo[internalFrameNumIndex].m_activeMask =
                (m_MCCResult.activeCameras[i].isActive) ?
                ChxUtils::BitSet(m_requestInfo[internalFrameNumIndex].m_activeMask, i) :
                ChxUtils::BitReset(m_requestInfo[internalFrameNumIndex].m_activeMask, i);
        }

        if (TRUE == m_isFlashRequired)
        {
            m_MCCResult.snapshotFusion = FALSE;
        }

        CHX_LOG_INFO("camera id %d, is active %d, main id %d, internalFrameNumber:%d",
            m_MCCResult.activeCameras[i].cameraId, m_MCCResult.activeCameras[i].isActive,
            pSessionInfo->m_pSession->GetCameraId(i),
            internalFrameNum);
    }

    CHX_LOG_INFO("snapshotFusion %d, masterPipelineIndex %d",
        m_MCCResult.snapshotFusion, m_requestInfo[internalFrameNumIndex].masterPipelineIndex);

    // if snapshot session is not created or created is not done.
    if (FALSE == m_deferSnapshotSessionDone)
    {
        if (hasSnapshotStreamRequest(pRequest))
        {
            m_pDeferSnapDoneMutex->Lock();
            /// wait snapshot session create done
            while (FALSE == m_deferSnapshotSessionDone)
            {
                m_pDeferSnapDoneCondition->Wait(m_pDeferSnapDoneMutex->GetNativeHandle());
            }
            m_pDeferSnapDoneMutex->Unlock();
        }
    }

    m_appWideJpegBuffer[applicationFrameNumIndex].buffer = NULL;

    // construct reqeust for realtime session
    for(UINT32 pipelineIndex = 0 ; pipelineIndex < m_numOfPhysicalDevices ; pipelineIndex++)
    {
        BOOL requestRAW               = FALSE;
        BOOL requestFD                = FALSE;
        //if this pipeline is not active, skip it.
        if (!ChxUtils::IsBitSet(m_requestInfo[internalFrameNumIndex].m_activeMask, pipelineIndex))
        {
            m_pMetadataManager->Release(pChiRTInputMetadata[pipelineIndex]);
            m_pMetadataManager->Release(pChiRTOutputMetadata[pipelineIndex]);
            continue;
        }

        for( UINT32 stream = 0 ; stream < pRequest->num_output_buffers; stream++)
        {
            CHX_LOG("pRequest->output_buffers[stream].stream = %p stream = %d", pRequest->output_buffers[stream].stream, stream);
            if ((CHISTREAM *)pRequest->output_buffers[stream].stream == m_pTargetPreviewStream)
            {
                ChxUtils::Memcpy(&m_appPreviewBuffer[internalFrameNumIndex],
                    &pRequest->output_buffers[stream], sizeof(camera3_stream_buffer_t));

                CHX_LOG("Preview real time request for pipeline:%d = %p", pipelineIndex,
                    m_multiCamResource[pipelineIndex].m_pRTOutputYUVStream);

                pBuffer = &streamBuffer[pipelineIndex][numOfBuffers[pipelineIndex]];
                pBuffer->size               = sizeof(CHISTREAMBUFFER);
                pBuffer->acquireFence.valid = FALSE;

                pBuffer->bufferInfo         = m_multiCamResource[pipelineIndex].m_pRTYUVBufferManager->GetImageBufferInfo();
                pBuffer->pStream            = m_multiCamResource[pipelineIndex].m_pRTOutputYUVStream;
                numOfBuffers[pipelineIndex]++;


                auto* pMultiCameraResource = &m_multiCamResource[pipelineIndex];

                if (TRUE == IsDS4RequiredOffline())
                {
                    pBuffer                     = &streamBuffer[pipelineIndex][numOfBuffers[pipelineIndex]];
                    pBuffer->size               = sizeof(CHISTREAMBUFFER);
                    pBuffer->acquireFence.valid = FALSE;
                    pBuffer->bufferInfo         = pMultiCameraResource->m_pRTDS4BufferManager->GetImageBufferInfo();
                    pBuffer->pStream            = pMultiCameraResource->m_pRTOutputDS4Stream;
                    numOfBuffers[pipelineIndex]++;

                }

                if (TRUE == IsDS16RequiredOffline())
                {
                    pBuffer                     = &streamBuffer[pipelineIndex][numOfBuffers[pipelineIndex]];
                    pBuffer->size               = sizeof(CHISTREAMBUFFER);
                    pBuffer->acquireFence.valid = FALSE;
                    pBuffer->bufferInfo         = pMultiCameraResource->m_pRTDS16BufferManager->GetImageBufferInfo();
                    pBuffer->pStream            = pMultiCameraResource->m_pRTOutputDS16Stream;
                    numOfBuffers[pipelineIndex]++;
                }

                if (TRUE == m_isOfflineRequired)
                {
                    requestRAW = TRUE;
                }
                requestFD      = TRUE;

                m_requestInfo[internalFrameNumIndex].m_previewMask =
                    ChxUtils::BitSet(m_requestInfo[internalFrameNumIndex].m_previewMask, pipelineIndex);
            }
            else if ((CHISTREAM *)pRequest->output_buffers[stream].stream == m_pTargetSnapshotStream)
            {
                ChxUtils::Memcpy(&m_appSnapshotBuffer[applicationFrameNumIndex],
                    &pRequest->output_buffers[stream], sizeof(camera3_stream_buffer_t));

                if ((TRUE == m_MCCResult.snapshotFusion) ||
                    (pipelineIndex == m_requestInfo[internalFrameNumIndex].masterPipelineIndex))
                {
                    CHX_LOG("Snapshot acquire = %d release = %d", m_appSnapshotBuffer[applicationFrameNumIndex].acquire_fence,
                        m_appSnapshotBuffer[applicationFrameNumIndex].release_fence);

                    requestFD = TRUE;

                    m_requestInfo[internalFrameNumIndex].m_snapshotMask =
                        ChxUtils::BitSet(m_requestInfo[internalFrameNumIndex].m_snapshotMask, pipelineIndex);
                }
                requestRAW    = TRUE;
            }
            else if ((CHISTREAM *)pRequest->output_buffers[stream].stream == m_pTargetPrimaryJpegStream)
            {
                ChxUtils::Memcpy(&m_appWideJpegBuffer[applicationFrameNumIndex],
                    &pRequest->output_buffers[stream], sizeof(camera3_stream_buffer_t));
            }
            else if (((CHISTREAM *)pRequest->output_buffers[stream].stream == m_pTargetRAW16Stream))
            {
                ChxUtils::Memcpy(&m_appRAW16Buffer[internalFrameNumIndex],
                    &pRequest->output_buffers[stream], sizeof(camera3_stream_buffer_t));

                if(UsecaseSAT == m_usecaseMode)
                {
                    CHX_LOG("RDI 16 Real time request for Main = %p",
                        m_multiCamResource[pipelineIndex].m_pRTOutputRAW16Stream);
                    pBuffer = &streamBuffer[pipelineIndex][numOfBuffers[pipelineIndex]];

                    pBuffer->size               = sizeof(CHISTREAMBUFFER);
                    pBuffer->acquireFence.valid = FALSE;
                    pBuffer->bufferInfo         = m_multiCamResource[pipelineIndex].m_pRAW16BufferManager->GetImageBufferInfo();
                    pBuffer->pStream            = m_multiCamResource[pipelineIndex].m_pRTOutputRAW16Stream;

                    numOfBuffers[pipelineIndex]++;
                    m_requestInfo[internalFrameNumIndex].m_rawMask =
                        ChxUtils::BitSet(m_requestInfo[internalFrameNumIndex].m_rawMask, pipelineIndex);
                }
            }
            else if (((CHISTREAM *)pRequest->output_buffers[stream].stream == m_pTargetPostviewStream))
            {
                CHX_LOG("Thumbnail Requested = %p", m_pTargetPostviewStream);

                ChxUtils::Memcpy(&m_appPostviewBuffer[internalFrameNumIndex],
                    &pRequest->output_buffers[stream], sizeof(camera3_stream_buffer_t));

                CHX_LOG("Thumbnail acquire = %d release = %d", m_appPostviewBuffer[internalFrameNumIndex].acquire_fence,
                    m_appPostviewBuffer[internalFrameNumIndex].release_fence);

                m_requestInfo[internalFrameNumIndex].m_postviewMask =
                    ChxUtils::BitSet(m_requestInfo[internalFrameNumIndex].m_postviewMask, pipelineIndex);
            }
            else if ((CHISTREAM *)pRequest->output_buffers[stream].stream == m_pTargetVideoStream)
            {
                CHX_LOG("Video Requested = %p", m_pTargetVideoStream);

                ChxUtils::Memcpy(&m_appVideoBuffer[internalFrameNumIndex],
                    &pRequest->output_buffers[stream], sizeof(camera3_stream_buffer_t));

                CHX_LOG("Video acquire = %d release = %d", m_appVideoBuffer[internalFrameNumIndex].acquire_fence,
                    m_appVideoBuffer[internalFrameNumIndex].release_fence);

                m_requestInfo[internalFrameNumIndex].m_videoMask =
                    ChxUtils::BitSet(m_requestInfo[internalFrameNumIndex].m_videoMask, pipelineIndex);
            }
            else if (((CHISTREAM *)pRequest->output_buffers[stream].stream == m_pTargetRDIStream) ||
                (TRUE == m_MCCResult.snapshotFusion))
            {
                if ((TRUE == m_MCCResult.snapshotFusion) ||
                    (pipelineIndex == m_requestInfo[internalFrameNumIndex].masterPipelineIndex))
                {
                    if (TRUE == IsFDBuffersNeeded())
                    {
                        pBuffer = &streamBuffer[pipelineIndex][numOfBuffers[pipelineIndex]];
                        GetOutputBufferFromFDQueue(internalFrameNum, pipelineIndex, pBuffer);
                        numOfBuffers[pipelineIndex]++;
                    }

                    m_requestInfo[internalFrameNumIndex].m_snapshotMask =
                        ChxUtils::BitSet(m_requestInfo[internalFrameNumIndex].m_snapshotMask, pipelineIndex);
                }
                requestRAW = TRUE;
            }
            else
            {
                CHX_LOG_ERROR("New stream???");
            }
        }

        if (TRUE == requestRAW)
        {
            pBuffer = &streamBuffer[pipelineIndex][numOfBuffers[pipelineIndex]];
            GetOutputBufferFromRDIQueue(internalFrameNum, pipelineIndex, pBuffer);
            numOfBuffers[pipelineIndex]++;
            m_requestInfo[internalFrameNumIndex].m_rawMask =
                        ChxUtils::BitSet(m_requestInfo[internalFrameNumIndex].m_rawMask, pipelineIndex);
            requestRAW = FALSE;
        }

        if (TRUE == IsFDBuffersNeeded() && (TRUE == requestFD))
        {
            pBuffer = &streamBuffer[pipelineIndex][numOfBuffers[pipelineIndex]];
            GetOutputBufferFromFDQueue(internalFrameNum, pipelineIndex, pBuffer);
            numOfBuffers[pipelineIndex]++;
        }

        request                    = &chiRequest[numOfRequest];
        request->frameNumber       = internalFrameNum;
        request->hPipelineHandle   = reinterpret_cast<CHIPIPELINEHANDLE>(
            pSessionInfo->m_pSession->GetPipelineHandle(pipelineIndex));
        request->numOutputs        = numOfBuffers[pipelineIndex];
        request->pOutputBuffers    = streamBuffer[pipelineIndex];
        request->pInputMetadata    = pChiRTInputMetadata[pipelineIndex]->GetHandle();
        request->pOutputMetadata   = pChiRTOutputMetadata[pipelineIndex]->GetHandle();

        CHX_LOG_REQMAP("frame: %u  <==>  chiOverrideFrameNum: %u <==>  internalFrameNum: %u  <==>  chiFrameNum: %" PRIu64
            " -- Realtime Session Request",
            GetAppFrameNum(applicationFrameNum),
            applicationFrameNum,
            internalFrameNum,
            request->frameNumber);

        ///< Add sticky if app settings is not NULL or if its 1st request after wakeup or LPM decision changed
        if ((NULL != pRequest->settings) ||
            (TRUE == m_multiCamResource[pipelineIndex].m_stickyMetaNeeded) ||
            (TRUE == LPMDecisionChanged))
        {
            request->pMetadata             = m_multiCamResource[pipelineIndex].m_pStickyMetadata;
        }

        request->pPrivData                 = &pSessionInfo->m_chiPrivData[pipelineIndex][internalFrameNumIndex];
        request->pPrivData->streamIndex    = pipelineIndex;

        if (0 != numOfBuffers[pipelineIndex])
        {
            numOfRequest++;
        }
    }

    if (0 < numOfRequest)
    {
        CHX_LOG_INFO("NumReq:%d Active:0x%x, Preview:0x%x, Snapshot:0x%x,"
            "Raw:0x%x, FrameNum:%d",
            numOfRequest,
            m_requestInfo[internalFrameNumIndex].m_activeMask,
            m_requestInfo[internalFrameNumIndex].m_previewMask,
            m_requestInfo[internalFrameNumIndex].m_snapshotMask,
            m_requestInfo[internalFrameNumIndex].m_rawMask,
            internalFrameNum);

        result = processSessionRequest(REALTIME_SESSION, numOfRequest, &chiRequest[0]);
    }
    else
    {
        CHX_LOG_ERROR("something wrong in LPM!");
    }

    ///< Reset sticky metadata if its already sent in the request
    for (UINT32 pipelineIndex = 0 ; pipelineIndex < m_numberOfOfflineStreams ; pipelineIndex++)
    {
        if (0 < numOfBuffers[pipelineIndex])
        {
            m_multiCamResource[pipelineIndex].m_stickyMetaNeeded = FALSE;
        }
    }

    if (CDKResultSuccess == result)
    {
        ActivatePendingPipeline();
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::IsRdiStream
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL UsecaseMultiCamera::IsRdiStream(
        CHISTREAM* pStream) const                ///< Stream to check
{
    BOOL isRDI = FALSE;
    for (UINT32 i = 0; i < m_numOfPhysicalDevices; i++)
    {
        if (m_multiCamResource[i].m_pRTOutputRDIStream == pStream)
        {
            isRDI = TRUE;
            break;
        }
    }
    return isRDI;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::IsFDStream
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL UsecaseMultiCamera::IsFDStream(
    CHISTREAM* pStream) const                ///< Stream to check
{
    BOOL isFD = FALSE;
    for (UINT32 i = 0; i < m_numOfPhysicalDevices; i++)
    {
        if (m_multiCamResource[i].m_pRTOutputFDStream == pStream)
        {
            isFD = TRUE;
            break;
        }
    }
    return isFD;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::IsRdiRAW16Stream
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL UsecaseMultiCamera::IsRdiRAW16Stream(
        CHISTREAM* pStream) const                ///< Stream to check
{
    BOOL isRDI = FALSE;
    for (UINT32 i = 0; i < m_numOfPhysicalDevices; i++)
    {
        if (m_multiCamResource[i].m_pRTOutputRAW16Stream == pStream)
        {
            isRDI = TRUE;
            break;
        }
    }
    return isRDI;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::IsRTPreviewStream
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL UsecaseMultiCamera::IsRTPreviewStream(
        CHISTREAM* pStream) const                ///< Stream to check
{
    BOOL isRTPreview = FALSE;
    for (UINT32 i = 0; i < m_numOfPhysicalDevices; i++)
    {
        if (m_multiCamResource[i].m_pRTOutputYUVStream == pStream)
        {
            isRTPreview = TRUE;
            break;
        }
    }
    return isRTPreview;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::IsRTPreviewStream
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL UsecaseMultiCamera::IsRTPreviewDS4Stream(
    CHISTREAM* pStream) const                ///< Stream to check
{
    BOOL isRTPreviewDS4 = FALSE;
    for (UINT32 i = 0; i < m_numOfPhysicalDevices; i++)
    {
        if (m_multiCamResource[i].m_pRTOutputDS4Stream == pStream)
        {
            isRTPreviewDS4 = TRUE;
            break;
        }
    }
    return isRTPreviewDS4;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::IsRTPreviewStream
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL UsecaseMultiCamera::IsRTPreviewDS16Stream(
    CHISTREAM* pStream) const                ///< Stream to check
{
    BOOL isRTPreviewDS16 = FALSE;
    for (UINT32 i = 0; i < m_numOfPhysicalDevices; i++)
    {
        if (m_multiCamResource[i].m_pRTOutputDS16Stream == pStream)
        {
            isRTPreviewDS16 = TRUE;
            break;
        }
    }
    return isRTPreviewDS16;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::CreateOfflineProcessResource
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::CreateOfflineProcessResource()
{
    CDKResult result = CDKResultSuccess;
    if (NULL == m_offlineRequestProcessThread.pPrivateData)
    {
        m_offlineRequestProcessThread.pPrivateData = this;

        m_pOfflineRequestMutex           = Mutex::Create();
        m_pOfflineRequestAvailable       = Condition::Create();
        m_offlineRequestProcessTerminate = FALSE;

        result = ChxUtils::ThreadCreate(UsecaseMultiCamera::OfflineRequestThread,
                                        &m_offlineRequestProcessThread,
                                        &m_offlineRequestProcessThread.hThreadHandle);
        if (CDKResultSuccess != result)
        {
            m_offlineRequestProcessThread = {0};
            if (NULL != m_pOfflineRequestMutex)
            {
                m_pOfflineRequestMutex->Destroy();
                m_pOfflineRequestMutex = NULL;
            }

            if (NULL != m_pOfflineRequestAvailable)
            {
                m_pOfflineRequestAvailable->Destroy();
                m_pOfflineRequestAvailable = NULL;
            }
        }

        if (CDKResultSuccess == result)
        {
            m_pFeatureDataMutex     = Mutex::Create();
            m_pFeatureDataAvailable = Condition::Create();
            m_featureAnchorIndex    = INVALID_INDEX;
            m_waitForFeatureData    = FALSE;
        }
    }
    return result;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::DestroyOfflineProcessResource
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::DestroyOfflineProcessResource()
{
    if (NULL != m_offlineRequestProcessThread.pPrivateData)
    {
        m_pOfflineRequestMutex->Lock();
        m_offlineRequestProcessTerminate = TRUE;
        m_pOfflineRequestAvailable->Signal();
        m_pOfflineRequestMutex->Unlock();

        m_pFeatureDataMutex->Lock();
        m_waitForFeatureData = FALSE;
        m_pFeatureDataAvailable->Signal();
        m_pFeatureDataMutex->Unlock();

        while (m_maxSnapshotReqId > m_snapshotReqId)
        {
            UINT snapshotReqIdIndex = (m_snapshotReqId % MaxOutstandingRequests);

            if (NULL != m_snapshotInputInfo[snapshotReqIdIndex].pAppMetadata)
            {
                m_snapshotInputInfo[snapshotReqIdIndex].pAppMetadata->Destroy();
                m_snapshotInputInfo[snapshotReqIdIndex].pAppMetadata = NULL;
            }
            m_snapshotReqId ++;
        }

        ChxUtils::ThreadTerminate(m_offlineRequestProcessThread.hThreadHandle);
        m_offlineRequestProcessThread = {0};
    }

    if (NULL != m_pOfflineRequestMutex)
    {
        m_pOfflineRequestMutex->Destroy();
        m_pOfflineRequestMutex = NULL;
    }

    if (NULL != m_pOfflineRequestAvailable)
    {
        m_pOfflineRequestAvailable->Destroy();
        m_pOfflineRequestAvailable = NULL;
    }

    if (NULL != m_pFeatureDataAvailable)
    {
        m_pFeatureDataAvailable->Destroy();
        m_pFeatureDataAvailable = NULL;
    }

    if (NULL != m_pFeatureDataMutex )
    {
        m_pFeatureDataMutex ->Destroy();
        m_pFeatureDataMutex  = NULL;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UsecaseMultiCamera::RequestThread
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID* UsecaseMultiCamera::OfflineRequestThread(
    VOID* pThreadData)
{
    PerThreadData* pPerThreadData = reinterpret_cast<PerThreadData*>(pThreadData);

    UsecaseMultiCamera* pMultiCamera = reinterpret_cast<UsecaseMultiCamera*>(pPerThreadData->pPrivateData);

    pMultiCamera->OfflineRequestProcess();

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UsecaseMultiCamera::CreateInputResourceForOffline
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::CreateInputResourceForOffline(
    UINT32 snapshotReqId)
{
    CDKResult result              = CDKResultSuccess;
    UINT32 snapshotReqIndex       = snapshotReqId % MaxOutstandingRequests;
    UINT32 internalFrameNum       = m_snapshotInputInfo[snapshotReqIndex].frameNumber;
    UINT32 internalFrameNumIndex  = internalFrameNum % MaxOutstandingRequests;
    RequestMapInfo requestMapInfo = m_requestMapInfo[internalFrameNumIndex];
    UINT32   inputFrameNum        = m_snapshotInputInfo[snapshotReqIndex].inputFrameNumber;

    m_requestMapInfo[internalFrameNumIndex].snapshotRequestID = snapshotReqId;

    for (UINT32 rtIndex = 0 ; rtIndex < m_numOfPhysicalDevices; rtIndex++)
    {
        if (ChxUtils::IsBitSet(m_snapshotInputInfo[snapshotReqIndex].snapshotMask, rtIndex))
        {
            do
            {
                result = WaitForBufferMetaReady(inputFrameNum, rtIndex);

                if ((CDKResultETimeout == result) && ((TRUE == m_offlineRequestProcessTerminate) ||
                    (TRUE == m_flushLock)))
                {
                    CHX_LOG_ERROR("wait for buffer ready failed: inputFramenubmer:%d, pipelineIndex:%d",
                        inputFrameNum, rtIndex);
                    break;
                }
            } while (result != CDKResultSuccess);

            if (CDKResultSuccess == result)
            {
                CHX_LOG_INFO("wait for buffer ready:%d, pipelineIndex:%d,done",inputFrameNum, rtIndex);
                result = CreateOfflineInputResource(internalFrameNum, rtIndex, TRUE);

                if (CDKResultSuccess == result)
                {
                    UpdateSnapshotMetadataForFeature(internalFrameNum, rtIndex);
                }
                else
                {
                    CHX_LOG_ERROR("Create offline input resource failed:snapshotid:%d, internalFrame:%d, rtIdx:%d",
                        snapshotReqId, internalFrameNum, rtIndex);
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UsecaseMultiCamera::UpdateSnapshotMetadataForFeature
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::UpdateSnapshotMetadataForFeature(
    UINT32 requestFrameNum,
    UINT32 pipelineIndex)
{
    OfflineInputInfo* pSnapshotInputInfo = GetOfflineInputInfo(requestFrameNum);
    for(UINT32 i = 0; i < pSnapshotInputInfo->numOfBuffers; i++)
    {
        ChiMetadata* pRDIMetadata       = pSnapshotInputInfo->pRDIMetadata[pipelineIndex][i];
        m_pMultiCamController->UpdateScalerCropForSnapshot(pRDIMetadata);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UsecaseMultiCamera::PickInputFrameForSnapshot
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::PickInputFrameForSnapshot(
    UINT32 snapshotReqId)
{
    CDKResult result = CDKResultSuccess;
    UINT32 snapshotReqIndex       = snapshotReqId % MaxOutstandingRequests;
    UINT32 internalFrameNum       = m_snapshotInputInfo[snapshotReqIndex].frameNumber;
    UINT32 internalFrameNumIndex  = internalFrameNum % MaxOutstandingRequests;

    if (FALSE == m_snapshotInputInfo[snapshotReqIndex].manualMode)
    {
        m_snapshotInputInfo[snapshotReqIndex].inputFrameNumber =
            GetZSLInputFrameNumber(internalFrameNum,
                                    m_snapshotInputInfo[snapshotReqIndex].numOfBuffers,
                                    m_requestInfo[internalFrameNumIndex].m_snapshotMask);
    }
    else
    {
        m_snapshotInputInfo[snapshotReqIndex].inputFrameNumber = internalFrameNum;
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UsecaseMultiCamera::SendOfflineProcessRequest
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult UsecaseMultiCamera::SendOfflineProcessRequest(
    UINT32 snapshotReqId)
{
    CDKResult result = CDKResultSuccess;
    UINT32 snapshotReqIndex       = snapshotReqId % MaxOutstandingRequests;
    UINT32 internalFrameNum       = m_snapshotInputInfo[snapshotReqIndex].frameNumber;
    UINT32 internalFrameNumIndex  = internalFrameNum % MaxOutstandingRequests;
    RequestMapInfo requestMapInfo = m_requestMapInfo[internalFrameNumIndex];

    UINT32   pipelineIndex        = requestMapInfo.activePipelineID;
    UINT32   appFrameNum          = requestMapInfo.frameNumber;
    UINT32   appFrameNumIndex     = appFrameNum % MaxOutstandingRequests;
    UINT32   inputFrameNum        = m_snapshotInputInfo[snapshotReqIndex].inputFrameNumber;

    Feature*      pFeature             = requestMapInfo.pFeature;
    FeatureType   snapshotFeatureType  = FeatureType::UNINIT;

    // send feature request first
    for (UINT32 rtIndex = 0 ; rtIndex < m_numOfPhysicalDevices; rtIndex++)
    {
        if ((TRUE == ChxUtils::IsBitSet(m_snapshotInputInfo[snapshotReqIndex].snapshotMask, rtIndex)) &&
            (TRUE == ChxUtils::IsBitSet(m_snapshotInputInfo[snapshotReqIndex].featureMask, rtIndex)))
        {
            snapshotFeatureType = pFeature->GetFeatureType();

            camera3_capture_request_t requestForFeature = { 0 };
            requestForFeature.frame_number              = internalFrameNum;
            requestForFeature.input_buffer              = NULL;
            requestForFeature.num_output_buffers        = 1;
            requestForFeature.settings                  = NULL;

            // Get buffer to hold Internal snapshot YUV image
            camera3_stream_buffer_t   outputBuffer = { 0 };
            outputBuffer.acquire_fence = InvalidNativeFence;
            outputBuffer.release_fence = InvalidNativeFence;
            outputBuffer.stream        = reinterpret_cast<camera3_stream_t*>
                (m_multiCamResource[m_maxJpegCameraId].m_pYUVOutputSnapshotStream);

            // if no snapshot request returned, don't get buffer handle from buffer manager,
            // otherwise, it may result in some buffer reference can't release.
            if (TRUE == requestMapInfo.isSnapshotReturnNeeded)
            {
                outputBuffer.buffer =  reinterpret_cast<buffer_handle_t*>
                    ((m_multiCamResource[rtIndex].m_pYUVSnapshotBufferManager->GetImageBufferInfo()).phBuffer);
            }
            else
            {
                outputBuffer.buffer = NULL;
            }
            requestForFeature.output_buffers  = &outputBuffer;

            m_pFeatureDataMutex->Lock();
            m_waitForFeatureData = getFeatureDependency(pFeature);
            m_pFeatureDataMutex->Unlock();

            result = m_requestMapInfo[internalFrameNumIndex].pFeature->ExecuteProcessRequest(&requestForFeature);
            m_requestMapInfo[internalFrameNumIndex].pFeature                = NULL;
            m_requestMapInfo[internalFrameNumIndex].triggerOfflineReprocess = FALSE;

            if (CDKResultSuccess == result)
            {
                CHX_LOG_INFO("offline request:%d, snapshotReqId:%d is sent for pipeline = %d!",
                    internalFrameNum, m_snapshotReqId, rtIndex);
            }

        }
    }

    // send normal snapshot request
    for (UINT32 rtIndex = 0 ; rtIndex < m_numOfPhysicalDevices; rtIndex++)
    {
        if ((TRUE == ChxUtils::IsBitSet(m_snapshotInputInfo[snapshotReqIndex].snapshotMask, rtIndex)) &&
            (FALSE == ChxUtils::IsBitSet(m_snapshotInputInfo[snapshotReqIndex].featureMask, rtIndex)))
        {
            UINT32 bufferIndex = 0;

            // when no feature request, no need to run AnchorFrame waiting logic
            if (NULL != pFeature)
            {
                // if there is pending feature result, waiting for it
                m_pFeatureDataMutex->Lock();

                if (TRUE == m_waitForFeatureData)
                {
                    result = m_pFeatureDataAvailable->TimedWait(m_pFeatureDataMutex->GetNativeHandle(), 500);
                    if ((CDKResultETimeout == result) && ((TRUE == m_offlineRequestProcessTerminate) ||
                        (TRUE == m_flushLock)))
                    {
                        CHX_LOG_ERROR("wait for feature data failed = %d terminate = %d", result,
                            m_offlineRequestProcessTerminate);
                        result = CDKResultEFailed;
                    }
                    else
                    {
                        if (INVALID_INDEX != m_featureAnchorIndex)
                        {
                            bufferIndex          = m_featureAnchorIndex;
                            m_featureAnchorIndex = INVALID_INDEX;
                        }
                        else
                        {
                            CHX_LOG_ERROR("No valid anchor frame is fed from feature!");
                        }
                    }
                }
                else
                {
                    // if m_waitForFeatureData == FALSE, it can't prove there is no feature dependency,
                    // feature process data notify may happen in feature executeprocessrequest, if so,
                    // the waitForFeatureData will be FALSE.
                    if (INVALID_INDEX != m_featureAnchorIndex)
                    {
                        bufferIndex          = m_featureAnchorIndex;
                        m_featureAnchorIndex = INVALID_INDEX;
                    }
                }

                CHX_LOG_INFO("Anchor frame index :%d, bufferindex: %d pipelineIndex:%d",
                    m_featureAnchorIndex, bufferIndex, rtIndex);

                m_pFeatureDataMutex->Unlock();
            }

            if (CDKResultSuccess == result)
            {
                CHX_LOG_REQMAP("frame: %u  <==>  chiOverrideFrameNum: %u <==>  internalFrameNum: %u  <==>  chiFrameNum: %u "
                               " -- OfflineSnapshotReq",
                               GetAppFrameNum(appFrameNum),
                               appFrameNum,
                               internalFrameNum,
                               internalFrameNum);

                GenerateSnapshotRequest(internalFrameNum, rtIndex, bufferIndex);

                //release non-useful buffer handle
                if (FeatureType::MFNR == snapshotFeatureType || FeatureType::SWMF == snapshotFeatureType)
                {
                    // For MFNR snapshot featuretype, some RDI buffers were held to pick
                    // one among them as per the achor frame data.
                    // Release all RDI buffers excect anchor frame.
                    OfflineInputInfo*  pInputInfo       = &m_snapshotInputInfo[snapshotReqId];
                    for (UINT32 i = 0; i < pInputInfo->numOfBuffers; i++)
                    {
                        // Don't release anchor frame Index.
                        if (i != bufferIndex)
                        {
                            ReleaseSingleOffineInputResource(internalFrameNum, rtIndex, i);
                            CHX_LOG_ERROR("Releasing for frame :%d, pipelineIndex:%d",
                                inputFrameNum - i, rtIndex);
                        }

                        // Release all FD buffers for MFNR feature
                        if (FeatureType::MFNR == snapshotFeatureType)
                        {
                            ReleaseSingleOffineFDInputResource(internalFrameNum, rtIndex, i);
                        }
                    }
                }

            }
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UsecaseMultiCamera::OfflineRequestProcess
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::OfflineRequestProcess()
{
    CDKResult result = CDKResultSuccess;
    CHX_LOG_INFO("OfflineRequestProcess Entered");
    while (TRUE)
    {
        m_pOfflineRequestMutex->Lock();
        if (m_offlineRequestProcessTerminate == FALSE)
        {
            m_pOfflineRequestAvailable->Wait(m_pOfflineRequestMutex->GetNativeHandle());
        }
        m_pOfflineRequestMutex->Unlock();

        if (m_offlineRequestProcessTerminate == TRUE)
        {
            break;
        }

        while ((m_maxSnapshotReqId > m_snapshotReqId) && (FALSE == m_offlineRequestProcessTerminate))
        {
            UINT   snapshotReqIdIndex     = (m_snapshotReqId % MaxOutstandingRequests);
            UINT32 internalFrameNum       = m_snapshotInputInfo[snapshotReqIdIndex].frameNumber;
            UINT32 internalFrameNumIndex  = internalFrameNum % MaxOutstandingRequests;
            m_requestMapInfo[internalFrameNumIndex].snapshotRequestID = m_snapshotReqId;

            // clear snapshot session result mask to avoid residual result
            for (UINT32 i = OFFLINE_RDI_SESSION0; i < (OFFLINE_RDI_SESSION0 + m_numOfPhysicalDevices); i++)
            {
                m_sessionInfo[i].m_bufferQueue[internalFrameNumIndex].m_resultMask = 0;
            }

            result = PickInputFrameForSnapshot(m_snapshotReqId);

            CHX_LOG_INFO("Send offline request:%d, inputFrame:%d,snapshotReqId:%d featureMask = %d SnapshotMask = %d",
                internalFrameNum,
                m_snapshotInputInfo[snapshotReqIdIndex].inputFrameNumber,
                m_snapshotReqId,
                m_snapshotInputInfo[snapshotReqIdIndex].featureMask,
                m_snapshotInputInfo[snapshotReqIdIndex].snapshotMask);

            // Pick resource from RDI queue imediately to avoid result being overwrite
            if (CDKResultSuccess == result)
            {
                result = CreateInputResourceForOffline(m_snapshotReqId);
            }

            // Send request for offline process
            if (CDKResultSuccess == result)
            {
                result = SendOfflineProcessRequest(m_snapshotReqId);
            }

            if (CDKResultSuccess == result)
            {
                //if app metadata is consumed, release it.
                if (NULL != m_snapshotInputInfo[snapshotReqIdIndex].pAppMetadata)
                {
                    m_snapshotInputInfo[snapshotReqIdIndex].pAppMetadata->Destroy();
                    m_snapshotInputInfo[snapshotReqIdIndex].pAppMetadata = NULL;
                }
                m_snapshotReqId ++;
            }

            if ((CDKResultSuccess != result) && ((TRUE == m_offlineRequestProcessTerminate) || (TRUE == m_flushLock)))
            {
                CHX_LOG_INFO("Set Snap Req ID to Last Snap Req if its in terminating %d or flush is in progress %d",
                    m_offlineRequestProcessTerminate, m_flushLock);
                m_snapshotReqId = m_maxSnapshotReqId;
            }
        }
    }
    CHX_LOG_INFO("OfflineRequestProcess Exited");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UsecaseMultiCamera::GetZSLInputFrameNumber
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 UsecaseMultiCamera::GetZSLInputFrameNumber(
    UINT32 requestFrameNumber,
    UINT32 requiredBufferCnt,
    UINT32 activeMask)
{
    UINT32 requestFrameIndex    = requestFrameNumber % MaxOutstandingRequests;
    UINT32 zslInputFrame        = INVALIDSEQUENCEID;
    UINT32 minimumZSLInputFrame = 0;
    UINT32 minimumLastReadyRDI  = INVALIDSEQUENCEID;

    /*****************************************************************************************/
    /* 1. calcuate minimum zslInputFramenumber to avoid no enough valid buffer in the queue  */
    /* 2. calcuate minimum lastReadyRDI to reduce capture latency                            */
    /* 3. zslInputFrame = minimumZSLInputFrame > minimumLastReadyRDI                         */
    /* minimumZSLInputFrame : minimumLastReadyRDI                                            */
    /*****************************************************************************************/
    for (UINT32 rtIndex = 0 ; rtIndex < m_numOfPhysicalDevices; rtIndex++)
    {
        if ((ChxUtils::IsBitSet(activeMask, rtIndex)))
        {
            UINT32 firstValidSequenceId = GetFirstReadyBufferFrameNum(rtIndex);
            UINT32 lastReadySequenceId  = GetLastReadyFrameNum(rtIndex);
            UINT32 tmpMinimumZSLInput   = lastReadySequenceId;

            CHX_LOG_INFO("first:%d,last:%d,requred:%d,rtindex:%d",
                firstValidSequenceId, lastReadySequenceId, requiredBufferCnt, rtIndex);

            if ((firstValidSequenceId != INVALIDSEQUENCEID) && (lastReadySequenceId != INVALIDSEQUENCEID) &&
                (requestFrameNumber - lastReadySequenceId < 6))
            {
                // if go into below condition, it shows there is no enough buffer in buffer queue
                // otherwise lastReadyFrameNum is best choise
                if ((firstValidSequenceId + requiredBufferCnt - 1) > lastReadySequenceId)
                {
                    tmpMinimumZSLInput = firstValidSequenceId + requiredBufferCnt - 1;
                }

                if (minimumLastReadyRDI > lastReadySequenceId)
                {
                    minimumLastReadyRDI = lastReadySequenceId;
                }
            }
            else
            {
                tmpMinimumZSLInput  = requestFrameNumber + requiredBufferCnt - 1;
            }

            if (minimumZSLInputFrame < tmpMinimumZSLInput)
            {
                minimumZSLInputFrame = tmpMinimumZSLInput;
            }

        }
    }

    // if there is no valid resource in RDI queue
    if (minimumLastReadyRDI == INVALIDSEQUENCEID)
    {
        minimumLastReadyRDI = minimumZSLInputFrame;
    }

    zslInputFrame = (minimumZSLInputFrame > minimumLastReadyRDI) ? minimumZSLInputFrame : minimumLastReadyRDI;

    CHX_LOG_INFO("requestFrameNumber = %d requiredBufferCnt = %d activeMask = %d zslInputFrame:%d",
        requestFrameNumber, requiredBufferCnt, activeMask, zslInputFrame);
    return zslInputFrame;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UsecaseMultiCamera::TriggerOfflineRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::TriggerOfflineRequest(UINT32 requestFrameNumber,
    camera3_capture_request_t* pRequest)
{
    CHX_LOG_INFO("TriggerOffineRequest :%d Entered", requestFrameNumber);

    if (FALSE == IsUsecaseInBadState())
    {
        UINT32          snapshotReqIdIndex      = (m_maxSnapshotReqId % MaxOutstandingRequests);
        UINT32          internalFrameNumIndex   = requestFrameNumber % MaxOutstandingRequests;
        SessionInfo*    pSessionInfo            = &m_sessionInfo[REALTIME_SESSION];
        UINT32          rtIndex                 = pSessionInfo->m_pSession->GetPipelineIndex(
                                                    m_requestMapInfo[internalFrameNumIndex].masterCameraID);
        ChiMetadata*    pAppSettings            = ChiMetadata::Create(NULL, 0, true);

        if (NULL != pAppSettings)
        {
            pAppSettings->Copy(*m_multiCamResource[rtIndex].m_pStickyMetadata);
        }
        else
        {
            CHX_LOG_ERROR("Create metadata for app settings failed!");
        }

        m_pOfflineRequestMutex->Lock();

        m_snapshotInputInfo[snapshotReqIdIndex].frameNumber  = requestFrameNumber;
        m_snapshotInputInfo[snapshotReqIdIndex].numOfBuffers = 1;
        m_snapshotInputInfo[snapshotReqIdIndex].featureMask  = 0;
        m_snapshotInputInfo[snapshotReqIdIndex].snapshotMask = 0;
        m_snapshotInputInfo[snapshotReqIdIndex].pAppMetadata = pAppSettings;

        /// snapshot metadata is used for ZSL feature.
        /// Advance Feature will reserve snapshot metadata by themselves.
        if (NULL == m_requestMapInfo[internalFrameNumIndex].pFeature)
        {
            UINT32 isZSLMode = 0;
            isZSLMode = ChxUtils::AndroidMetadata::GetZSLMode(const_cast<camera_metadata_t*>(pRequest->settings));

            if ((0 != isZSLMode) && (FALSE == m_isFlashRequired))
            {
                m_snapshotInputInfo[snapshotReqIdIndex].manualMode = FALSE;
            }
            else
            {
                m_snapshotInputInfo[snapshotReqIdIndex].manualMode = TRUE;
            }
            m_snapshotInputInfo[snapshotReqIdIndex].featureMask  = 0;
            m_snapshotInputInfo[snapshotReqIdIndex].snapshotMask =
                m_requestInfo[internalFrameNumIndex].m_snapshotMask;
        }
        else
        {
           if (TRUE == m_requestMapInfo[internalFrameNumIndex].triggerOfflineReprocess)
           {
                m_snapshotInputInfo[snapshotReqIdIndex].inputFrameNumber= requestFrameNumber;

                Feature* pFeature       = m_requestMapInfo[internalFrameNumIndex].pFeature;
                UINT32 requiredFrameCnt = m_requestMapInfo[internalFrameNumIndex].numOfSnapshotBuffers;

                for (UINT32 rtIndex = 0 ; rtIndex < m_numOfPhysicalDevices; rtIndex++)
                {
                    // Enable feature mask for the master pipeline of this particular request.
                    m_snapshotInputInfo[snapshotReqIdIndex].featureMask =
                        ((canEnableAdvanceFeature()) &&
                        (rtIndex == m_requestMapInfo[internalFrameNumIndex].activePipelineID)) ?
                        ChxUtils::BitSet(m_snapshotInputInfo[snapshotReqIdIndex].featureMask, rtIndex) :
                        ChxUtils::BitReset(m_snapshotInputInfo[snapshotReqIdIndex].featureMask, rtIndex);
                }
                {
                    m_snapshotInputInfo[snapshotReqIdIndex].snapshotMask =
                        m_requestInfo[internalFrameNumIndex].m_snapshotMask;
                }

                switch(pFeature->GetFeatureType())
                {
                    case FeatureType::MFSR:
                    case FeatureType::MFNR:
                        m_snapshotInputInfo[snapshotReqIdIndex].numOfBuffers   = requiredFrameCnt;

                        if ((TRUE == m_isLLSNeeded) || (TRUE == m_isIHDRSnapshot))
                        {
                            m_snapshotInputInfo[snapshotReqIdIndex].manualMode = TRUE;
                        }
                        else
                        {
                            m_snapshotInputInfo[snapshotReqIdIndex].manualMode = FALSE;
                        }

                        break;
                    case FeatureType::SWMF:
                        m_snapshotInputInfo[snapshotReqIdIndex].numOfBuffers   = requiredFrameCnt;
                        m_snapshotInputInfo[snapshotReqIdIndex].manualMode     = FALSE;
                        break;
                    case FeatureType::HDR:
                        m_snapshotInputInfo[snapshotReqIdIndex].numOfBuffers   = 1;
                        m_snapshotInputInfo[snapshotReqIdIndex].manualMode     = TRUE;
                        if (FALSE == m_requestMapInfo[internalFrameNumIndex].isSnapshotReturnNeeded)
                        {
                            for (UINT32 rtIndex = 0 ; rtIndex < m_numOfPhysicalDevices; rtIndex++)
                            {
                                if (rtIndex != m_requestMapInfo[internalFrameNumIndex].activePipelineID)
                                {
                                    m_snapshotInputInfo[snapshotReqIdIndex].snapshotMask =
                                        ChxUtils::BitReset(m_snapshotInputInfo[snapshotReqIdIndex].snapshotMask, rtIndex);
                                }
                            }
                        }

                        break;
                    default:
                        CHX_LOG_ERROR("non existence feature type!");
                        break;
                }

           }
        }

        m_maxSnapshotReqId++;
        m_pOfflineRequestAvailable->Signal();
        m_pOfflineRequestMutex->Unlock();
    }
    CHX_LOG_INFO("TriggerOffineRequest Exited: maxSnapshotReqId:%d", m_maxSnapshotReqId);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UsecaseMultiCamera::GenerateSnapshotRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::GenerateSnapshotRequest(
        UINT32 frameNumber,
        UINT32 pipelineIndex,
        UINT32 bufferIndex)
{
    UINT32              internalFrameNumIndex   = frameNumber % MaxOutstandingRequests;
    CHISTREAMBUFFER     RDIInputBuffer          = { 0 };
    CHICAPTUREREQUEST   request                 = {0};
    UINT32              sessionIndex            = pipelineIndex + OFFLINE_RDI_SESSION0;
    UINT32              numOutBuffer            = 0;
    CHISTREAMBUFFER     outputBuffers[2]        = { {0}, {0} };

    request.frameNumber              = frameNumber;
    request.hPipelineHandle          = reinterpret_cast<CHIPIPELINEHANDLE>(
        m_sessionInfo[sessionIndex].m_pSession->GetPipelineHandle(0));
    request.numInputs                = 1;

    numOutBuffer = 1;
    if (ChxUtils::IsBitSet(m_requestInfo[internalFrameNumIndex].m_postviewMask, pipelineIndex) &&
        (pipelineIndex == m_requestInfo[internalFrameNumIndex].masterPipelineIndex))
    {
        numOutBuffer = 2;
    }

    request.numOutputs                  = numOutBuffer;

    outputBuffers[0].size               = sizeof(CHISTREAMBUFFER);
    outputBuffers[0].acquireFence.valid = FALSE;
    outputBuffers[0].bufferInfo         =
        m_multiCamResource[pipelineIndex].m_pYUVSnapshotBufferManager->GetImageBufferInfo();
    outputBuffers[0].pStream            = m_multiCamResource[pipelineIndex].m_pYUVOutputSnapshotStream;

    if (1 < numOutBuffer)
    {
        ChxUtils::PopulateHALToChiStreamBuffer(&m_appPostviewBuffer[internalFrameNumIndex], &outputBuffers[1]);
    }

    request.pOutputBuffers = outputBuffers;

    for (UINT i = 0; i < request.numOutputs; i++)
    {
        CHX_LOG("[MultiCamera RDI] Request : frameNum= %" PRIu64 " acquireFence: (valid=%d type=%d nativeFence=%d chiFence=%p) "
                "ReleaseFence: (valid=%d type=%d nativeFence=%d chiFence=%p) Buffer: (type=%d handle=%p), status=%d",
                request.frameNumber,
                request.pOutputBuffers[i].acquireFence.valid,
                request.pOutputBuffers[i].acquireFence.type,
                request.pOutputBuffers[i].acquireFence.nativeFenceFD,
                request.pOutputBuffers[i].acquireFence.hChiFence,
                request.pOutputBuffers[i].releaseFence.valid,
                request.pOutputBuffers[i].releaseFence.type,
                request.pOutputBuffers[i].releaseFence.nativeFenceFD,
                request.pOutputBuffers[i].releaseFence.hChiFence,
                request.pOutputBuffers[i].bufferInfo.bufferType,
                request.pOutputBuffers[i].bufferInfo.phBuffer,
                request.pOutputBuffers[i].bufferStatus);
    }

    ChiMetadata* pRDIMetadata       = NULL;
    ChiMetadata* pChiOutputMetadata = m_pMetadataManager->Get(
        m_sessionInfo[sessionIndex].m_pSession->GetMetadataClientId(),
        frameNumber);

    GetInputBufferFromRDIQueue(frameNumber, pipelineIndex, bufferIndex, &RDIInputBuffer, &pRDIMetadata, TRUE);

    if ((NULL != pRDIMetadata) && (NULL != pChiOutputMetadata))
    {
        request.pInputBuffers     = &RDIInputBuffer;
        request.pInputMetadata    = pRDIMetadata->GetHandle();
        request.pOutputMetadata   = pChiOutputMetadata->GetHandle();
        request.pPrivData         = &m_sessionInfo[sessionIndex].m_chiPrivData[pipelineIndex][internalFrameNumIndex];
        request.pPrivData->streamIndex          = pipelineIndex;
        request.pPrivData->numInputBuffers      = 1;
        request.pPrivData->inputBuffers[0]      = RDIInputBuffer.bufferInfo;
        request.pPrivData->bufferManagers[0]    = GetBufferManager(pipelineIndex);

        CHX_LOG_INFO("[MultiCamera RDI] Send normal request:pipelineindex:%d, sessionIndex %d, pInputMetadata %p, pOutputMetadata:%p",
                pipelineIndex, sessionIndex, request.pInputMetadata, request.pOutputMetadata);

        if (1 <= ExtensionModule::GetInstance()->EnableDumpDebugData())
        {
            BOOL isMaster = (m_requestMapInfo[internalFrameNumIndex].masterCameraID ==
                m_sessionInfo[REALTIME_SESSION].m_pSession->GetCameraId(pipelineIndex) ? TRUE : FALSE);
            MergeDebugData(&request, isMaster);
        }

        processSessionRequest(sessionIndex, 1, &request);
    }
    else
    {
        CHX_LOG_ERROR("fatal error for snapshot request: pipeline:%d, session:%d, Cannot get %s ",
            pipelineIndex, sessionIndex, (NULL == pRDIMetadata)? "RDIMetadata":"ChiOutputMetadata");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UsecaseMultiCamera::isOfflineProcessRequired
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL UsecaseMultiCamera::isOfflineProcessRequired(
    camera3_capture_request_t* pRequest,
    RequestMapInfo*            pRequestMapInfo)
{
    (VOID)pRequest;
    BOOL isOfflineRequired     = FALSE;

    if (NULL != pRequestMapInfo)
    {
        // for all advance feature, it need to be handled in offline thread
        if (NULL != pRequestMapInfo->pFeature)
        {
            if (TRUE == pRequestMapInfo->triggerOfflineReprocess)
            {
                isOfflineRequired = TRUE;
            }
            else
            {
                isOfflineRequired = FALSE;
            }
        }
        else
        {
            if (TRUE == pRequestMapInfo->isSnapshotReturnNeeded)
            {
                isOfflineRequired = TRUE;
            }
            else
            {
                isOfflineRequired = FALSE;
            }
        }
    }

    return isOfflineRequired;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::SaveDebugData
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::SaveDebugData(
    const CHICAPTURERESULT* pResult)
{
    const INT    numberOfSlots      = BufferQueueDepth;
    DebugData*   pDebug             = NULL;
    ChiMetadata* pChiOutputMetadata = NULL;

    pChiOutputMetadata = m_pMetadataManager->GetMetadataFromHandle(pResult->pOutputMetadata);

    if (TRUE == ChxUtils::IsVendorTagPresent(pChiOutputMetadata, DebugDataTag))
    {
        CHAR* pData = NULL;
        ChxUtils::GetVendorTagValue(pChiOutputMetadata, DebugDataTag, (VOID**)&pData);
        if (NULL != pData)
        {
            DebugData* pDebug = reinterpret_cast<DebugData*>(pData);
            if ((NULL == pDebug->pData) || (0 == pDebug->size))
            {
                // No data, nothing to save
                return;
            }
            if (NULL == m_debugDataRealTime.pBaseAllocation)
            {
                SIZE_T  baseAllocationSize          = numberOfSlots * pDebug->size;
                SIZE_T  baseOffset                  = 0;
                INT     index                       = 0;
                m_debugDataRealTime.pBaseAllocation = CHX_CALLOC(baseAllocationSize);
                m_debugDataRealTime.pMeta           = static_cast<debugDataQueue*>(
                                                                 CHX_CALLOC(sizeof(debugDataQueue) * numberOfSlots));
                if ((NULL == m_debugDataRealTime.pBaseAllocation) || (NULL == m_debugDataRealTime.pMeta))
                {
                    // Fail to allocate memory
                    if (NULL != m_debugDataRealTime.pBaseAllocation)
                    {
                        CHX_FREE(m_debugDataRealTime.pBaseAllocation);
                        m_debugDataRealTime.pBaseAllocation = NULL;
                    }
                    if (NULL != m_debugDataRealTime.pMeta)
                    {
                        CHX_FREE(m_debugDataRealTime.pMeta);
                        m_debugDataRealTime.pMeta = NULL;
                    }
                    return;
                }

                m_debugDataRealTime.fullSize = baseAllocationSize;
                m_debugDataRealTime.maxQueue = numberOfSlots;
                m_debugDataRealTime.queueIndex = 0;

                baseOffset = 0;
                for (index = 0; index < numberOfSlots; index++)
                {
                    m_debugDataRealTime.pMeta[index].pData = static_cast<BYTE*>(
                                                                             m_debugDataRealTime.pBaseAllocation) + baseOffset;
                    m_debugDataRealTime.pMeta[index].size  = pDebug->size;

                    baseOffset += pDebug->size;
                }
            }

            if (pDebug->size != m_debugDataRealTime.pMeta[m_debugDataRealTime.queueIndex].size)
            {
                // Size mismatch
                return;
            }

            // All resources ready, now copy and set key for retrieval
            ChxUtils::Memcpy(m_debugDataRealTime.pMeta[m_debugDataRealTime.queueIndex].pData,
                             pDebug->pData,
                             pDebug->size);

            // Set key, overriting older data
            m_debugDataRealTime.pMeta[m_debugDataRealTime.queueIndex].pKey = pDebug->pData;


            CHX_LOG("DebugDataAll: inserting: %p key: %p, index: %d (next %d)",
                    m_debugDataRealTime.pMeta[m_debugDataRealTime.queueIndex].pData,
                    m_debugDataRealTime.pMeta[m_debugDataRealTime.queueIndex].pKey,
                    m_debugDataRealTime.queueIndex,
                   (m_debugDataRealTime.queueIndex + 1) % numberOfSlots);

            // Set next queue index for insertion
            ++m_debugDataRealTime.queueIndex;
            m_debugDataRealTime.queueIndex %= numberOfSlots;
        }
        else
        {
            CHX_LOG_ERROR("DebugDataAll: Unable to get vendor tag data!");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UsecaseMultiCamera::MergeDebugData
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID UsecaseMultiCamera::MergeDebugData(
    const CHICAPTUREREQUEST* pRequest,
    const BOOL isMaster)
{
    CDKResult result                = CDKResultSuccess;
    if (NULL != pRequest->pInputMetadata)
    {
        ChiMetadata* pChiInputMetadata  = m_pMetadataManager->GetMetadataFromHandle(pRequest->pInputMetadata);

        // Debug-data deep-copy
        if (FALSE == isMaster)
        {
            DebugData emptyData = { 0 };
            result = ChxUtils::SetVendorTagValue(pChiInputMetadata,
                                                 DebugDataTag,
                                                 sizeof(DebugData),
                                                 &emptyData);
            if (CDKResultSuccess != result)
            {
                CHX_LOG_ERROR("DebugDataAll: Fail to set debugdata tag in offline input metadata");
            }
        }
        else if (TRUE == ChxUtils::IsVendorTagPresent(pChiInputMetadata, DebugDataTag))
        {
            CHAR* pData = NULL;
            ChxUtils::GetVendorTagValue(pChiInputMetadata, DebugDataTag, (VOID**)&pData);
            DebugData* pDebug = reinterpret_cast<DebugData*>(pData);

            if (NULL != pData)
            {
                DebugData* pDebug = reinterpret_cast<DebugData*>(pData);

                if ((NULL != pDebug->pData) && (0 < pDebug->size))
                {
                    // Allocate debug-data for offline processing
                    if (NULL == m_debugDataOffline.pData)
                    {
                        m_debugDataOffline.pData = CHX_CALLOC(pDebug->size);
                        if (NULL != m_debugDataOffline.pData)
                        {
                            m_debugDataOffline.size = pDebug->size;
                        }
                        else
                        {
                            m_debugDataOffline.size = 0;
                        }
                    }
                    else if (pDebug->size != m_debugDataOffline.size)
                    {
                        CHX_LOG_ERROR("DebugDataAll: mismatch size %zu != offline %zu",
                                      pDebug->size,
                                      m_debugDataOffline.size);
                    }

                    if ((NULL != m_debugDataOffline.pData) &&
                        (m_debugDataOffline.pData != pDebug->pData)) // For snapshot same metadata buffer is use, so avoid copy.
                    {
                        // Get real-time debug-data copy, since current data was already reset by pipeline for re-use
                        BOOL    isDataAvailable = FALSE;
                        INT     savedIndex = 0;
                        for (savedIndex = 0; savedIndex < m_debugDataRealTime.maxQueue; savedIndex++)
                        {
                            if (pDebug->pData == m_debugDataRealTime.pMeta[savedIndex].pKey)
                            {
                                isDataAvailable = TRUE;
                                break;
                            }
                        }

                        // Copy saved data
                        if ((TRUE == isDataAvailable) &&
                            (m_debugDataOffline.size == m_debugDataRealTime.pMeta[savedIndex].size))
                        {
                            ChxUtils::Memcpy(m_debugDataOffline.pData,
                                             m_debugDataRealTime.pMeta[savedIndex].pData,
                                             m_debugDataRealTime.pMeta[savedIndex].size);

                            CHX_LOG("DebugDataAll: GenerateSnapshotRequest NEW MASTER: %p, orig/key: %p, saved: %p",
                                    m_debugDataOffline.pData, pDebug->pData, m_debugDataRealTime.pMeta[savedIndex].pData);

                            result = ChxUtils::SetVendorTagValue(pChiInputMetadata,
                                                                 DebugDataTag,
                                                                 sizeof(DebugData),
                                                                 &m_debugDataOffline);
                            if (CDKResultSuccess != result)
                            {
                                CHX_LOG_ERROR("DebugDataAll: Fail to set debugdata tag in offline input metadata");
                            }
                        }
                    }
                }
            }
            else
            {
                CHX_LOG_ERROR("DebugDataAll: Unable to get vendor tag data!");
                result = CDKResultEFailed;
            }
        }
    }
    else
    {
        CHX_LOG_ERROR("Metadata is NULL");
        result = CDKResultEFailed;
    }
}

CDKResult UsecaseMultiCamera::updateSessionSettings(
    Pipeline* m_pPipeline,
    camera3_stream_configuration_t* pStreamConfig)
{
    CDKResult result           = CDKResultSuccess;
    CameraConfigs cameraConfig = {0};

    if ((NULL == m_pLogicalCameraInfo) || (NULL == m_pPipeline) ||
        (NULL == pStreamConfig))
    {
        result = CDKResultEInvalidArg;
    }

    if (CDKResultSuccess == result)
    {
        result = m_pPipeline->SetAndroidMetadata(pStreamConfig);
    }

    if (CDKResultSuccess == result)
    {
        cameraConfig.numPhysicalCameras   = m_numOfPhysicalDevices;
        cameraConfig.primaryCameraId      = m_pLogicalCameraInfo->primaryCameraId;

        for (UINT32 camindex = 0; camindex < m_numOfPhysicalDevices; camindex++)
        {
            CameraConfiguration *pDst            = &cameraConfig.cameraConfigs[camindex];
            DeviceInfo          *pSrc            = m_pLogicalCameraInfo->ppDeviceInfo[camindex];
            CHISENSORMODEINFO*   pSensorModeInfo = UsecaseSelector::GetSensorModeInfo(pSrc->cameraId, pStreamConfig, 1);

            pDst->cameraId                = pSrc->cameraId;
            pDst->cameraRole              = pSrc->pDeviceConfig->cameraRole;
            pDst->transitionZoomRatioLow  = pSrc->pDeviceConfig->transitionZoomRatioMin;
            pDst->transitionZoomRatioHigh = pSrc->pDeviceConfig->transitionZoomRatioMax;
            pDst->enableSmoothTransition  = pSrc->pDeviceConfig->enableSmoothTransition;
            pDst->sensorCaps              = pSrc->m_pDeviceCaps->sensorCaps;
            pDst->lensCaps                = pSrc->m_pDeviceCaps->lensCaps;
            pDst->sensorModeInfo          = *(pSensorModeInfo);
        }

        ChiMetadata* pMetadata = m_pPipeline->GetDescriptorMetadata();
        result = pMetadata->SetTag("com.qti.chi.cameraconfiguration",
            "PhysicalCameraConfigs", &cameraConfig, sizeof(cameraConfig));

    }

    return result;
}

VOID UsecaseMultiCamera::ProcessFeatureDataNotify(
    UINT32   appFrameNum,
    Feature* pCurrentFeature,
    VOID*    pData)
{
    CAMX_UNREFERENCED_PARAM(appFrameNum);
    CAMX_UNREFERENCED_PARAM(pCurrentFeature);
    CAMX_UNREFERENCED_PARAM(pData);

    m_pFeatureDataMutex->Lock();
    if(NULL != pCurrentFeature)
    {
        switch(pCurrentFeature->GetFeatureType())
        {
            case FeatureType::MFNR:
                // pData is anchor index value from MFNR feature. Needed for aux camera to select the right RDI frame.
                m_featureAnchorIndex = *(static_cast<UINT32*>(pData));
                m_waitForFeatureData = FALSE;
                break;
            case FeatureType::SWMF:
                m_featureAnchorIndex = *(static_cast<UINT32*>(pData));
                m_waitForFeatureData = FALSE;
                break;
            default:
                CHX_LOG_WARN("Unknown feature type!");
        }
        CHX_LOG_INFO("featuretype:%d,anchorIndex;%d", pCurrentFeature->GetFeatureType(), m_featureAnchorIndex);
        m_pFeatureDataAvailable->Signal();
    }
    m_pFeatureDataMutex->Unlock();

    return;
}

VOID UsecaseMultiCamera::LogFeatureRequestMappings(
    UINT32          inFrameNum,
    UINT32          reqFrameNum,
    const CHAR*     identifyingData)
{
    UINT32 internalFrameNumIndex = inFrameNum % MaxOutstandingRequests;

    CHX_LOG_REQMAP("frame: %u  <==>  chiOverrideFrameNum: %u  <==> internalFrameNum: %u <==> chiFrameNum: %u -- %s",
        GetAppFrameNum(m_requestMapInfo[internalFrameNumIndex].frameNumber),
        m_requestMapInfo[internalFrameNumIndex].frameNumber,
        inFrameNum,
        reqFrameNum,
        identifyingData);
}

VOID UsecaseMultiCamera::ProcessFeatureDone(
    UINT32            internalFrameNum,
    Feature*          pCurrentFeature,
    CHICAPTURERESULT* pResult)
{
    UINT32 internalFrameNumIndex = internalFrameNum % MaxOutstandingRequests;
    UINT32 rtPipelineIndex       = 0;
    if (NULL != pResult->pPrivData)
    {
        rtPipelineIndex = pResult->pPrivData->streamIndex;
    }
    pResult->frameworkFrameNum   = internalFrameNum;

    CHX_LOG("appFrameNum:%d, resultMetadata %p pStream %p rtPipelineIndex %d",
        internalFrameNum, pResult->pOutputMetadata, pResult->pOutputBuffers[0].pStream, rtPipelineIndex);

    if(NULL != pCurrentFeature)
    {
        CHX_LOG_INFO("Feature done %d", pCurrentFeature->GetFeatureType());
        OfflineSnapshotResult(pResult, (rtPipelineIndex + OFFLINE_RDI_SESSION0));
    }
    return;
}

VOID UsecaseMultiCamera::UpdateAdvanceFeatureStatus(
    Feature* pFeature)
{
    m_isLLSSnapshot = (TRUE == IsLLSNeeded()) ? TRUE : FALSE;

    if (TRUE == ExtensionModule::GetInstance()->ForceEnableIHDRSnapshot())
    {
        CHX_LOG("Force enable IHDR snapshot, original is m_isIHDRSnapshotEnable = %u, m_isIHDRAECTrigger = %u",
                m_isIHDRSnapshotEnable, m_isIHDRAECTrigger);

        m_isIHDRSnapshotEnable = TRUE;
        m_isIHDRAECTrigger     = TRUE;
    }

    if ((TRUE == m_isIHDRSnapshotEnable) && (TRUE == m_isIHDRAECTrigger))
    {
        m_isIHDRSnapshot = TRUE;
    }

    //  Feature HDR doesn't support LLS and IHDR snapshot
    if (FeatureType::HDR == pFeature->GetFeatureType())
    {
        m_isLLSSnapshot  = FALSE;
        m_isIHDRSnapshot = FALSE;
    }

    //  Currently, we only support MFNR + IHDR snapshot and we assume the priority of IHDR is higher than LLS.
    if ((FeatureType::MFNR == pFeature->GetFeatureType()) && (TRUE == m_isIHDRSnapshot))
    {
        //  If so, then we keep m_isIHDRSnapshot and disable m_isLLSSnapshot due to they are mutually exclusive
        m_isLLSSnapshot = FALSE;
    }
    else
    {
        //  Else we disable m_isIHDRSnapshot
        m_isIHDRSnapshot = FALSE;
    }
}
