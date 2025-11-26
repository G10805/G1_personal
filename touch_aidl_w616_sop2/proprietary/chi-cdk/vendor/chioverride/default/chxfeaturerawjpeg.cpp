////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxfeaturerawjpeg.cpp
/// @brief CHX raw + jpeg(simultaneous raw and jpeg capture) feature class implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "chxadvancedcamerausecase.h"
#include "chxincs.h"
#include "chxfeaturerawjpeg.h"
#include "chxusecase.h"

// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureRawJPEG::Create
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FeatureRawJPEG* FeatureRawJPEG::Create(
    AdvancedCameraUsecase* pUsecase)
{
    FeatureRawJPEG* pFeature = CHX_NEW FeatureRawJPEG;

    if (NULL != pFeature)
    {
        if (CDKResultSuccess != pFeature->Initialize(pUsecase))
        {
            pFeature->Destroy(FALSE);
            pFeature = NULL;
        }
    }

    return pFeature;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureRawJPEG::Pause
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureRawJPEG::Pause(
        BOOL isForced)
{
    CHX_LOG("Feature Pause isForced =%d", isForced);
    ChxUtils::AtomicStoreU32(&m_aPauseInProgress, TRUE);

    m_pOfflineRequestMutex->Lock();
    m_offlineRequestProcessTerminate = TRUE;
    m_pOfflineRequestAvailable->Signal();
    m_pOfflineRequestMutex->Unlock();
    ChxUtils::ThreadTerminate(m_offlineRequestProcessThread.hThreadHandle);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureRawJPEG::IsPseudoZSL
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL FeatureRawJPEG::IsPseudoZSL(ChiMetadata* pMetadata)
{
    // It is Pseudo ZSL
    // if application hasn't set the ZSL mode OR
    // If the snapshot is captured with Flash or Manual 3A gains

    UINT zslMode = 0;
    m_pUsecase->GetZSLMode(&zslMode, pMetadata);

    CHX_LOG("FeatureRawJPEG: zslMode: %d", zslMode);

    return (0 == zslMode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureRawJPEG::Initialize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureRawJPEG::Initialize(
    AdvancedCameraUsecase* pUsecase)
{
    CDKResult    result = CDKResultSuccess;

    Feature::InitializePrivateResources();

    m_pUsecase     = pUsecase;
    m_pResultMutex = Mutex::Create();

    m_pOfflineRequestMutex           = Mutex::Create();
    m_pOfflineRequestAvailable       = Condition::Create();
    m_offlineRequestProcessTerminate = FALSE;
    m_aPauseInProgress               = FALSE;
    m_pMetadataManager               = pUsecase->GetMetadataManager();

    m_offlineRequestProcessThread.pPrivateData = this;

    result = ChxUtils::ThreadCreate(FeatureRawJPEG::RequestThread,
                                    &m_offlineRequestProcessThread,
                                    &m_offlineRequestProcessThread.hThreadHandle);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureRawJPEG::Destroy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureRawJPEG::Destroy(BOOL isForced)
{

    CHX_LOG("FeatureRawJPEG::Destroy(), isForced %d E.", isForced);

    m_pSnapshotStream     = NULL;
    m_pRdiFrameworkStream = NULL;
    m_pFdStream           = NULL;
    m_pRdiStream          = NULL;

    if ((TRUE == m_useDummyPreview) && (NULL != m_pPreviewStream))
    {
        CHX_FREE(m_pPreviewStream);
        m_pPreviewStream = NULL;
    }
    else
    {
        m_pPreviewStream = NULL;
    }

    if (NULL != m_pResultMutex)
    {
        m_pResultMutex->Destroy();
        m_pResultMutex = NULL;
    }

    if (NULL != m_pOfflineRequestMutex)
    {
        m_pOfflineRequestMutex->Destroy();
        m_pOfflineRequestMutex = NULL;
    }

    if (NULL != m_pChiUsecase)
    {
        CHX_FREE(m_pChiUsecase->pPipelineTargetCreateDesc);
        m_pChiUsecase->pPipelineTargetCreateDesc = NULL;
        CHX_FREE(m_pChiUsecase);
        m_pChiUsecase = NULL;
    }

    ChxUtils::AtomicStoreU32(&m_aPauseInProgress, FALSE);

    Feature::DestroyPrivateResources();

    CHX_DELETE(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureRawJPEG::OverrideUsecase
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ChiUsecase* FeatureRawJPEG::OverrideUsecase(
    LogicalCameraInfo*              pCameraInfo,
    camera3_stream_configuration_t* pStreamConfig)
{
    CHX_LOG("Initializing usecase for RawJPEG feature, num_streams:%d", pStreamConfig->num_streams);

    /// @todo - Split up the ZSL pipelines in XML so we can use the default matching
    ChiUsecase* pZslUsecase = g_pUsecaseZSL;

    if (NULL == pZslUsecase)
    {
        CHX_LOG_ERROR("Fail to get UsecaseZSL from xml!");
        return NULL;
    }

    UsecaseZSLPipelineIds snapshotPipelineId = UsecaseZSLPipelineIds::ZSLSnapshotJpeg;

    m_pRdiStream = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::RdiStream);
    m_pFdStream  = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::FdStream);

    m_isRdiFormatRaw16 = FALSE;
    for (UINT32 i = 0; i < pStreamConfig->num_streams; i++)
    {
        INT format = pStreamConfig->streams[i]->format;
        if (HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED == format ||
            HAL_PIXEL_FORMAT_YCbCr_420_888          == format)
        {
            m_pPreviewStream  = reinterpret_cast<ChiStream*>(pStreamConfig->streams[i]);
        }
        else if (HAL_PIXEL_FORMAT_BLOB == format)
        {
            m_pSnapshotStream  = reinterpret_cast<ChiStream*>(pStreamConfig->streams[i]);
        }
        else if (HAL_PIXEL_FORMAT_RAW10 == format || HAL_PIXEL_FORMAT_RAW16 == format)
        {
            // Save this RDI stream from frameworks.
            // Used later to send the result back to frameworks.
            m_pRdiFrameworkStream = reinterpret_cast<ChiStream*>(pStreamConfig->streams[i]);
            if (HAL_PIXEL_FORMAT_RAW16 == format)
            {
                m_isRdiFormatRaw16 = TRUE;
            }
            m_pRdiFrameworkStream->maxNumBuffers = 8;

            // Config the RDI size, this decide the sensor output size.
            // For the JPEG+RAW, the sensor output size decided by the RAW stream size.
            m_pRdiStream->width  = pStreamConfig->streams[i]->width;
            m_pRdiStream->height = pStreamConfig->streams[i]->height;
            CHX_LOG("FeatureRawJPEG config RDI stream w*h: %dx%d", m_pRdiStream->width,m_pRdiStream->height);
        }
    }

    if (NULL == m_pPreviewStream)
    {
        m_useDummyPreview = TRUE;
        m_pPreviewStream  = static_cast<CHISTREAM*>(CHX_CALLOC(sizeof(CHISTREAM)));
        if (NULL != m_pPreviewStream)
        {
            m_pPreviewStream->format        = ChiStreamFormatImplDefined;
            m_pPreviewStream->width         = DefaultPreviewWidth;
            m_pPreviewStream->height        = DefaultPreviewHeight;
            m_pPreviewStream->streamType    = ChiStreamTypeOutput;
            m_pPreviewStream->grallocUsage  = 0x00000100;
            m_pPreviewStream->maxNumBuffers = MaxStreamBuffers;
        }
        else
        {
            CHX_LOG_ERROR("Calloc failed");
            return NULL;
        }
    }

    m_pChiUsecase = static_cast<ChiUsecase*>(CHX_CALLOC(sizeof(ChiUsecase)));
    if (NULL != m_pChiUsecase)
    {
        m_pChiUsecase->pPipelineTargetCreateDesc = static_cast<ChiPipelineTargetCreateDescriptor*>
            (CHX_CALLOC(2 * sizeof(ChiPipelineTargetCreateDescriptor)));
        if (NULL != m_pChiUsecase->pPipelineTargetCreateDesc)
        {
            m_pChiUsecase->pUsecaseName     = pZslUsecase->pUsecaseName;
            m_pChiUsecase->streamConfigMode = pZslUsecase->streamConfigMode;
            m_pChiUsecase->numTargets       = pZslUsecase->numTargets;
            m_pChiUsecase->ppChiTargets     = pZslUsecase->ppChiTargets;
            m_pChiUsecase->numPipelines     = 2; // Override pipeline count

            m_pChiUsecase->pPipelineTargetCreateDesc[PreviewPipelineIndex]  =
                pZslUsecase->pPipelineTargetCreateDesc[UsecaseZSLPipelineIds::ZSLPreviewRaw];
            m_pChiUsecase->pPipelineTargetCreateDesc[SnapshotPipelineIndex] =
                pZslUsecase->pPipelineTargetCreateDesc[snapshotPipelineId];

            UINT cameraIds[2] = { pCameraInfo->ppDeviceInfo[0]->cameraId, pCameraInfo->ppDeviceInfo[0]->cameraId };
            m_pUsecase->SetPipelineToCameraMapping(2, cameraIds);

            m_pUsecase->m_previewAspectRatio = static_cast<FLOAT>(m_pPreviewStream->width) / m_pPreviewStream->height;

            ChiPipelineTargetCreateDescriptor* pSnapshotDesc = NULL;
            ChiPipelineTargetCreateDescriptor* pPreviewDesc  = NULL;
            pSnapshotDesc = &m_pChiUsecase->pPipelineTargetCreateDesc[SnapshotPipelineIndex];
            pPreviewDesc  = &m_pChiUsecase->pPipelineTargetCreateDesc[PreviewPipelineIndex];

            if ((NULL != pSnapshotDesc) && (NULL != pPreviewDesc))
            {
                UINT                               previewIndex;
                UINT                               rdiIndex;
                UINT                               fdIndex;

                pSnapshotDesc->sourceTarget.pTargetPortDesc[0].pTarget->pChiStream = m_pRdiStream;
                pSnapshotDesc->sinkTarget.pTargetPortDesc[0].pTarget->pChiStream = m_pSnapshotStream;

                // Assuming the Raw target has only one format.
                CHX_ASSERT((1 == pPreviewDesc->sinkTarget.pTargetPortDesc[0].pTarget->numFormats) ||
                           (1 == pPreviewDesc->sinkTarget.pTargetPortDesc[1].pTarget->numFormats));

                // Setup the raw target with the RDI stream. Raw has only one buffer format, it will either match here, or
                // not matter in the output format because none of its will match (another assumption)
                previewIndex = m_pUsecase->GetTargetIndex(&pPreviewDesc->sinkTarget, "TARGET_BUFFER_DISPLAY");
                rdiIndex     = m_pUsecase->GetTargetIndex(&pPreviewDesc->sinkTarget, "TARGET_BUFFER_RAW");
                fdIndex      = m_pUsecase->GetTargetIndex(&pPreviewDesc->sinkTarget, "TARGET_BUFFER_FD");

                if (CDKInvalidId != previewIndex)
                {
                    pPreviewDesc->sinkTarget.pTargetPortDesc[previewIndex].pTarget->pChiStream = m_pPreviewStream;
                }
                if (CDKInvalidId != rdiIndex)
                {
                    pPreviewDesc->sinkTarget.pTargetPortDesc[rdiIndex].pTarget->pChiStream = m_pRdiStream;
                }
                if (CDKInvalidId != fdIndex)
                {
                    pPreviewDesc->sinkTarget.pTargetPortDesc[fdIndex].pTarget->pChiStream = m_pFdStream;
                }
            }

            for (UINT target = 0; target < m_pChiUsecase->numTargets; target++)
            {
                if (TRUE == StreamIsInternal(m_pChiUsecase->ppChiTargets[target]->pChiStream))
                {
                    if (ChiFormatRawMIPI == m_pChiUsecase->ppChiTargets[target]->pBufferFormats[0] ||
                        ChiFormatRawPlain16 == m_pChiUsecase->ppChiTargets[target]->pBufferFormats[0])
                    {
                        m_rdiStreamIndex = target;
                        m_pRdiTargetBuffer = m_pUsecase->GetTargetBufferPointer(m_rdiStreamIndex);
                    }
                    else if (ChiFormatYUV420NV12 == m_pChiUsecase->ppChiTargets[target]->pBufferFormats[0])
                    {
                        m_fdStreamIndex = target;
                        m_pFdTargetBuffer = m_pUsecase->GetTargetBufferPointer(m_fdStreamIndex);
                    }
                }
            }
        }
        else
        {
            CHX_LOG_ERROR("Calloc failed");
        }
    }
    else
    {
        CHX_LOG_ERROR("Calloc failed");
    }

    return m_pChiUsecase;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureRawJPEG::PipelineCreated
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureRawJPEG::PipelineCreated(
    UINT sessionId,       ///< Id of session created
    UINT pipelineIndex)   ///< Index of the pipeline created (within the context of the session)
{
    (VOID) sessionId;
    (VOID)pipelineIndex;

    if (TRUE == m_isRdiFormatRaw16)
    {
        m_pRdiStream->format = ChiStreamFormatRaw16;
    }
    else
    {
        m_pRdiStream->format = ChiStreamFormatRaw10;
    }

    // For the JPEG+RAW, RDI stream output based on raw stream size configure from APP
    // so, will not update the RDI stream output here,it have been initialized in the OverrideUsecase
    m_pRdiStream->maxNumBuffers = 0;
    m_pRdiStream->rotation      = StreamRotationCCW90;
    m_pRdiStream->streamType    = ChiStreamTypeOutput;
    m_pRdiStream->grallocUsage  = 0;

    m_rdiStreamIndex = m_pUsecase->GetInternalTargetBufferIndex(m_pRdiStream);
    m_fdStreamIndex  = m_pUsecase->GetInternalTargetBufferIndex(m_pFdStream);
    m_pRdiTargetBuffer = m_pUsecase->GetTargetBufferPointer(m_rdiStreamIndex);
    m_pFdTargetBuffer = m_pUsecase->GetTargetBufferPointer(m_fdStreamIndex);

    m_pUsecase->ConfigFdStream();
    SetFeatureStatus(FeatureStatus::READY);

    /* Partial Meta data can come from multiple pipeline. The below check is
    to mark the final pipeline that will submit the Partial Meta data*/
    if (PreviewPipelineIndex == sessionId)
    {
        CHX_LOG_INFO("FeatureRAWJPEG: Partial Data will be sent from Session:%d Pipeline:%d",
            sessionId,
            pipelineIndex);
        m_pUsecase->SetFinalPipelineForPartialMetaData(sessionId);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureRawJPEG::ExecuteProcessRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult  FeatureRawJPEG::ExecuteProcessRequest(
    camera3_capture_request_t* pRequest)              ///< Request parameters
{
    CDKResult                 result             = CDKResultSuccess;
    CHISTREAMBUFFER           previewBuffers[5]  = {{0}};
    UINT                      previewCount       = 0;
    UINT                      snapshotCount      = 0;
    UINT                      frameNumber        = pRequest->frame_number;
    UINT32                    frameIndex         = (pRequest->frame_number % MaxOutstandingRequests);
    UINT                      snapshotReqIdIndex = (m_maxSnapshotReqId % MaxOutstandingRequests);
    camera3_capture_result_t* pUsecaseResult     = m_pUsecase->GetCaptureResult(frameIndex);
    UINT32                    sensorModeIndex;

    pUsecaseResult->result             = NULL;
    pUsecaseResult->frame_number       = pRequest->frame_number;
    pUsecaseResult->num_output_buffers = 0;

    m_isSnapshotFrame[frameIndex]          = FALSE;
    m_isRdiFrameRequested[frameIndex]      = FALSE;
    m_shutterTimestamp[frameIndex]         = 0;
    ChxUtils::Memset(&m_pRdiFrameworkStreamBuffer[frameIndex], 0, sizeof(camera3_stream_buffer_t));



    sensorModeIndex = m_pUsecase->GetSessionData(PreviewPipelineIndex)->pSession->GetSensorModeInfo()->modeIndex;
    ChxUtils::AndroidMetadata::FillTuningModeData(const_cast<camera_metadata_t*>(pRequest->settings),
                                 pRequest,
                                 sensorModeIndex,
                                 m_pUsecase->GetEffectMode(),
                                 m_pUsecase->GetSceneMode(),
                                 m_pUsecase->GetFeature1Mode(),
                                 m_pUsecase->GetFeature2Mode());

    ChxUtils::AndroidMetadata::FillCameraId(const_cast<camera_metadata_t*>(pRequest->settings), m_pUsecase->GetCameraId());

    for (UINT32 i = 0; i < pRequest->num_output_buffers; i++)
    {
        if (m_pPreviewStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream))
        {
            // Capture preview stream
            ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i], &previewBuffers[previewCount]);
            previewCount++;
        }

        if (m_pSnapshotStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream))
        {

            CHX_LOG("Snapshot Frame %d", pRequest->frame_number);
            ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i],
                                                   &m_snapshotBuffers[snapshotReqIdIndex][snapshotCount]);
            snapshotCount++;
        }

        if (m_pRdiFrameworkStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream))
        {
            ChxUtils::Memcpy(&m_pRdiFrameworkStreamBuffer[frameIndex],
                             &pRequest->output_buffers[i],
                             sizeof(camera3_stream_buffer_t));

            m_isRdiFrameRequested[frameIndex] = TRUE;
        }

    }

    // update capture request for raw frame, if there is a request for either Rdi frame or snapshot frame.
    if (TRUE == m_isRdiFrameRequested[frameIndex] || snapshotCount > 0)
    {
        if (TRUE == m_isRdiFrameRequested[frameIndex])
        {
            // If there is buffer provided by framework for Rdi, then
            // override with framework's stream buffer, Raw buffer will be directly updated on this.
            m_pUsecase->ReserveBufferQueueSlot(frameNumber, m_rdiStreamIndex, m_pRdiFrameworkStreamBuffer[frameIndex].buffer);
            previewBuffers[previewCount].bufferInfo.bufferType  = HALGralloc;
            previewBuffers[previewCount].bufferInfo.phBuffer    = m_pRdiFrameworkStreamBuffer[frameIndex].buffer;
            previewBuffers[previewCount].pStream                = m_pRdiStream;
            previewBuffers[previewCount].acquireFence.valid     = FALSE;
            previewBuffers[previewCount].size                   = sizeof(CHISTREAMBUFFER);
            previewCount++;
        }
        else
        {
            result = m_pUsecase->GetOutputBufferFromRDIQueue(frameNumber, m_rdiStreamIndex, &previewBuffers[previewCount]);

            if (CDKResultSuccess == result)
            {
                previewCount++;
            }
            else
            {
                CHX_LOG_ERROR("FeatureRawJpeg: GetOutputBufferFromRDIQueue failed for frameNumber %d", frameNumber);
            }
        }
    }
    else if (FALSE == m_isRdiFrameRequested[frameIndex])
    {
        result = m_pUsecase->GetOutputBufferFromRDIQueue(frameNumber, m_rdiStreamIndex, &previewBuffers[previewCount]);

        if (CDKResultSuccess == result)
        {
            previewCount++;
        }
        else
        {
            CHX_LOG_ERROR("FeatureRawJpeg: GetOutputBufferFromRDIQueue failed for frameNumber %d", frameNumber);
        }

        if (TRUE == m_pUsecase->IsFDBuffersNeeded())
        {
            result = m_pUsecase->GetOutputBufferFromFDQueue(frameNumber, m_fdStreamIndex, &previewBuffers[previewCount]);

            if (CDKResultSuccess == result)
            {
                previewCount++;
            }
            else
            {
                CHX_LOG_ERROR("FeatureRawJpeg: GetOutputBufferFromFDQueue failed, No FD buffer for frameNumber %d",
                    frameNumber);
            }
        }
    }

    PipelineData* pPipelineData = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(PreviewPipelineIndex, 0));
    UINT          requestIdIndex = (pPipelineData->seqId % MaxOutstandingRequests);

    pPipelineData->seqIdToFrameNum[requestIdIndex] = frameNumber;

    const Session *pSession = m_pUsecase->GetSessionData(PreviewPipelineIndex)->pSession;

    ChiMetadata* pInputMetadata  = m_pMetadataManager->GetInput(pRequest->settings, pRequest->frame_number);
    ChiMetadata* pOutputMetadata = m_pMetadataManager->Get(
        pPipelineData->pPipeline->GetMetadataClientId(), pPipelineData->seqId);

    if ((NULL == pInputMetadata) || (NULL == pOutputMetadata))
    {
        CHX_LOG_ERROR("FeatureRawJpeg: Cannot get input or output metadata %p %p", pInputMetadata, pOutputMetadata);
        result = CDKResultEFailed;
    }

    CHICAPTUREREQUEST  request       = { 0 };
    CHIPIPELINEREQUEST submitRequest = { 0 };

    if (CDKResultSuccess == result)
    {
        request.frameNumber       = pPipelineData->seqId++;
        request.hPipelineHandle   = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
        request.numOutputs        = previewCount;
        request.pOutputBuffers    = previewBuffers;
        request.pInputMetadata    = pInputMetadata->GetHandle();
        request.pOutputMetadata   = pOutputMetadata->GetHandle();
        request.pPrivData = &m_privData[request.frameNumber % MaxOutstandingRequests];

        submitRequest.pSessionHandle     = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
        submitRequest.numRequests        = 1;
        submitRequest.pCaptureRequests   = &request;

        // adds reference for the input metadata to be used by snapshot pipeline
        if (0 < snapshotCount)
        {
            pInputMetadata->AddReference();
        }
    }

    if ((CDKResultSuccess == result) && (FALSE == pSession->IsPipelineActive()))
    {
        if (ExtensionModule::GetInstance()->GetNumPCRsBeforeStreamOn(const_cast<camera_metadata_t*>(
            pRequest->settings)) == pRequest->frame_number)
        {
            result = ExtensionModule::GetInstance()->ActivatePipeline(pSession->GetSessionHandle(),
                pSession->GetPipelineHandle());
            if (CDKResultSuccess == result)
            {
                pSession->SetPipelineActivateFlag();
            }
        }
    }

    if ((CDKResultSuccess == result) && NULL != pInputMetadata)
    {
        m_isZSLSnapshotRequested[frameIndex] = IsPseudoZSL(pInputMetadata) ? FALSE : TRUE;
    }

    if (CDKResultSuccess == result)
    {
        m_pUsecase->LogFeatureRequestMappings(frameNumber, request.frameNumber, "RAW Jpeg Request");
        result = m_pUsecase->SubmitRequest(&submitRequest);
    }
    else if (NULL != pInputMetadata)
    {
        // release the reference in case of failure
        pInputMetadata->ReleaseReference();
    }

    if ((CDKResultSuccess == result) && (0 < snapshotCount))
    {
        m_isSnapshotFrame[frameIndex]                 = TRUE;
        m_snapshotBufferNum[snapshotReqIdIndex]       = snapshotCount;
        m_snapshotReqIdToFrameNum[snapshotReqIdIndex] = frameNumber;
        m_maxSnapshotReqId++;

        m_pSnapshotInputMeta[snapshotReqIdIndex] = pInputMetadata;

        m_pOfflineRequestMutex->Lock();
        m_pOfflineRequestAvailable->Signal();
        m_pOfflineRequestMutex->Unlock();
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureRawJPEG::ProcessResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureRawJPEG::ProcessResult(
    CHICAPTURERESULT*   pResult,
    VOID*               pPrivateCallbackData)
{
    if ((TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress))) ||
        (FlushStatus::NotFlushing != m_pUsecase->GetFlushStatus()))
    {
         CHX_LOG_INFO("ZSL process result return because of cleanup");
         return;
    }
    m_pResultMutex->Lock();

    SessionPrivateData* pCbData               = static_cast<SessionPrivateData*>(pPrivateCallbackData);
    BOOL                isAppResultsAvailable = FALSE;
    UINT32              resultFrameNum        = pResult->frameworkFrameNum;
    UINT32              resultFrameIndex      = resultFrameNum % MaxOutstandingRequests;
    UINT32              rtPipelineReqId       = 0;

    if (SnapshotPipelineIndex == pCbData->sessionId)
    {
        resultFrameNum = m_snapshotReqIdToFrameNum[pResult->frameworkFrameNum % MaxOutstandingRequests];
        resultFrameIndex = resultFrameNum % MaxOutstandingRequests;

        CHX_LOG("Processing result for snapshot frame %d. Metadata: %p NumBuffers: %d Timestamp: %" PRIu64 " Sent: %d",
            resultFrameNum, pResult->pOutputMetadata, pResult->numOutputBuffers,
            m_shutterTimestamp[resultFrameNum],
            m_pUsecase->IsMetadataSent(resultFrameIndex));
    }
    else if (PreviewPipelineIndex == pCbData->sessionId)
    {
        rtPipelineReqId = pResult->frameworkFrameNum;
        resultFrameNum = m_pUsecase->GetChiFrameNumFromReqId(PreviewPipelineIndex, 0, rtPipelineReqId);
        resultFrameIndex = resultFrameNum % MaxOutstandingRequests;
        CHX_LOG("Realtime ReqId to ChiFrameNum: %d <--> %d", pResult->frameworkFrameNum, resultFrameNum);
    }

    camera3_capture_result_t* pUsecaseResult   = m_pUsecase->GetCaptureResult(resultFrameIndex);

    pUsecaseResult->frame_number = resultFrameNum;

    // If result contain metadata and metadata has not been sent to framework
    if ((NULL != pResult->pOutputMetadata) && (NULL != pResult->pInputMetadata))
    {
        UINT64       timestamp                 = m_shutterTimestamp[resultFrameIndex];
        ChiMetadata* pInputMetadata            = m_pMetadataManager->GetMetadataFromHandle(pResult->pInputMetadata);
        ChiMetadata* pOutputMetadata           = m_pMetadataManager->GetMetadataFromHandle(pResult->pOutputMetadata);

        m_pMetadataManager->Release(pInputMetadata);

        if ((NULL != pOutputMetadata) && (PreviewPipelineIndex == pCbData->sessionId))
        {
            m_pUsecase->ParseResultMetadata(m_pMetadataManager->GetMetadataFromHandle(pResult->pOutputMetadata));
        }

        // Do Not wait for Snapshot frame metadata, Return Preview metadata back to fwk
        // If we wait for snapshot, and if it takes more time to process, we will block the preview.
        pUsecaseResult->partial_result = pResult->numPartialMetadata;

        if (PreviewPipelineIndex == pCbData->sessionId)
        {
            m_pUsecase->FillMetadataForRDIQueue(rtPipelineReqId, m_rdiStreamIndex, pOutputMetadata);
        }
        else
        {
            //snapshot pipeline release output meta
            const Session *pSession = m_pUsecase->GetSessionData(SnapshotPipelineIndex)->pSession;
            m_pMetadataManager->Release(pOutputMetadata);
        }

        // Override snapshot frame sensor timestamp metadata with shutter event for same frame number
        if ((FALSE == m_pUsecase->IsMetadataSent(resultFrameIndex)) && (NULL == pUsecaseResult->result))
        {
            // Do Not wait for Snapshot frame metadata, Return Preview metadata back to fwk.
            // If we wait for snapshot, and if it takes more time to process, we will block the preview.
            pUsecaseResult->partial_result = pResult->numPartialMetadata;

            // Override snapshot frame sensor timestamp metadata with shutter event for same frame number
            if (0 != timestamp)
            {
                ChxUtils::UpdateTimeStamp(pOutputMetadata, timestamp, resultFrameNum);
                m_pUsecase->SetMetadataAvailable(resultFrameIndex);
                isAppResultsAvailable = TRUE;

                m_pUsecase->UpdateAppResultMetadata(pOutputMetadata,
                                                    resultFrameIndex,
                                                    m_pUsecase->GetMetadataClientIdFromPipeline(pCbData->sessionId, 0));
            }
            // If shutter from realtime for snapshot frame has not come yet, metadata will be dispatched from notify
            else
            {
                CHX_LOG("add metadata to capture result for frame %d", resultFrameNum);
                m_pUsecase->UpdateAppResultMetadata(pOutputMetadata,
                                                    resultFrameIndex,
                                                    m_pUsecase->GetMetadataClientIdFromPipeline(pCbData->sessionId, 0));
            }
        }
    }

    for (UINT32 j = 0; j < pResult->numOutputBuffers; j++)
    {
        // If our internal stream, copy the result into the target buffer to be consumed by the offline pipeline
        if (m_pRdiStream == pResult->pOutputBuffers[j].pStream)
        {
            m_pUsecase->UpdateBufferReadyForRDIQueue(rtPipelineReqId, m_rdiStreamIndex, TRUE);
            if ((TRUE == m_isSnapshotFrame[resultFrameIndex]))
            {
                m_pUsecase->GetAppResultMutex()->Lock();
                camera3_stream_buffer_t* pResultBuffer =
                    const_cast<camera3_stream_buffer_t*>(&pUsecaseResult->output_buffers[pUsecaseResult->num_output_buffers++]);
                // Send the Rdi frame from here to framework.
                ChxUtils::Memcpy(pResultBuffer, &m_pRdiFrameworkStreamBuffer[resultFrameIndex],
                    sizeof(camera3_stream_buffer_t));
                m_pUsecase->GetAppResultMutex()->Unlock();
                isAppResultsAvailable = TRUE;

                if (FALSE == m_isZSLSnapshotRequested[resultFrameIndex])
                {
                    m_pOfflineRequestMutex->Lock();
                    m_pOfflineRequestAvailable->Signal();
                    m_pOfflineRequestMutex->Unlock();
                }
                //Clear out the buffer queue of framework RDI as it is not used for snapshot
                else
                {
                    m_pUsecase->ClearBufferQueueSlot(rtPipelineReqId, m_rdiStreamIndex);
                }

            }
        }
        else if (m_pFdStream == pResult->pOutputBuffers[j].pStream)
        {
            m_pUsecase->UpdateBufferReadyForFDQueue(rtPipelineReqId, m_fdStreamIndex, TRUE);
        }
        // Otherwise queue a buffer as part of the normal result
        else
        {
            m_pUsecase->GetAppResultMutex()->Lock();
            camera3_stream_buffer_t* pResultBuffer =
                const_cast<camera3_stream_buffer_t*>(&pUsecaseResult->output_buffers[pUsecaseResult->num_output_buffers++]);

            ChxUtils::PopulateChiToHALStreamBuffer(&pResult->pOutputBuffers[j], pResultBuffer);

            if ((SnapshotPipelineIndex == pCbData->sessionId) &&
                (ChiNative == pResult->pPrivData->inputBuffers[0].bufferType))
            {
                // Release reference to the input buffers of the request to SnapshotPipeline
                m_pUsecase->ReleaseReferenceToInputBuffers(pResult->pPrivData);
            }
            m_pUsecase->GetAppResultMutex()->Unlock();

            isAppResultsAvailable = TRUE;
        }
    }

    if (TRUE == isAppResultsAvailable)
    {
        m_pUsecase->ProcessAndReturnFinishedResults();
    }

    m_pResultMutex->Unlock();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureRawJPEG::ProcessCHIPartialData
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureRawJPEG::ProcessCHIPartialData(
    UINT32    frameNum,
    UINT32    sessionId)
{
    CAMX_UNREFERENCED_PARAM(frameNum);
    CAMX_UNREFERENCED_PARAM(sessionId);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureRawJPEG::ProcessDriverPartialCaptureResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureRawJPEG::ProcessDriverPartialCaptureResult(
    CHIPARTIALCAPTURERESULT*    pResult,
    VOID*                       pPrivateCallbackData)
{
    ChiMetadata*                pChiOutputMetadata      = NULL;
    UINT32                      resultFrameNum          = pResult->frameworkFrameNum;
    UINT32                      resultFrameIndex        = ChxUtils::GetResultFrameIndexChi(resultFrameNum);
    SessionPrivateData*         pCbData                 =
        static_cast<SessionPrivateData*>(pPrivateCallbackData);
    PartialResultSender         sender                  =
        PartialResultSender::DriverPartialData;

    if ((TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress))) ||
        (FlushStatus::NotFlushing != m_pUsecase->GetFlushStatus()))
    {
        CHX_LOG_INFO("ZSL process result return because of cleanup");
        return;
    }

    CHX_LOG("FeatureRAWJPEG Driver Capture result from sessionid:%d", pCbData->sessionId);
    if (SnapshotPipelineIndex == pCbData->sessionId)
    {
        resultFrameNum   = m_snapshotReqIdToFrameNum[pResult->frameworkFrameNum % MaxOutstandingRequests];
        resultFrameIndex = resultFrameNum % MaxOutstandingRequests;
        CHX_LOG("FeatureRAWJPEG Snapshot ReqId to AppFrameNum: %d <--> %d",
            pResult->frameworkFrameNum,
            resultFrameNum);
    }
    else if (PreviewPipelineIndex == pCbData->sessionId)
    {
        resultFrameNum   = m_pUsecase->GetChiFrameNumFromReqId(PreviewPipelineIndex, 0, pResult->frameworkFrameNum);
        resultFrameIndex = resultFrameNum % MaxOutstandingRequests;
        CHX_LOG("FeatureRAWJPEG Realtime ReqId to AppFrameNum: %d <--> %d",
            pResult->frameworkFrameNum,
            resultFrameNum);
    }

    pChiOutputMetadata = m_pMetadataManager->GetMetadataFromHandle(pResult->pPartialResultMetadata);

    // This check is to ensure that we have not sent earlier and this is the final pipeline's partial
    // result
    if ((m_pUsecase->GetFinalPipelineForPartialMetaData() == pCbData->sessionId)&&
        (TRUE == m_pUsecase->CheckIfPartialDataCanBeSent(sender, resultFrameIndex)))
    {
        if (PartialMetaSupport::CombinedPartialMeta == ExtensionModule::GetInstance()->EnableCHIPartialData())
        {
            ProcessCHIPartialData(pResult->frameworkFrameNum, pCbData->sessionId);
        }

        m_pUsecase->UpdateAppPartialResultMetadataFromDriver(pChiOutputMetadata,
                                                             resultFrameIndex,
                                                             resultFrameNum,
                                                             m_pUsecase->GetMetadataClientIdFromPipeline(
                                                                 pCbData->sessionId, 0));
        m_pUsecase->ProcessAndReturnPartialMetadataFinishedResults(sender);
    }
    else
    {
        CHX_LOG("FeatureRAWJPEG Driver Capture result from sessionid:%d cannot be sent", pCbData->sessionId);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureRawJPEG::ProcessMessage
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureRawJPEG::ProcessMessage(
    const CHIMESSAGEDESCRIPTOR* pMessageDescriptor,
    VOID*                       pPrivateCallbackData)
{
    m_pResultMutex->Lock();

    if (ChiMessageTypeSof == pMessageDescriptor->messageType)
    {
        // SOF notifications are not sent to the HAL3 application
        CHX_LOG("RawJPEG Chi Notify SOF frameNum %u framework frameNum %u, timestamp %" PRIu64,
            pMessageDescriptor->message.sofMessage.sofId,
            pMessageDescriptor->message.sofMessage.frameworkFrameNum,
            pMessageDescriptor->message.sofMessage.timestamp);
    }
    else
    {
        SessionPrivateData* pCbData = static_cast<SessionPrivateData*>(pPrivateCallbackData);
        BOOL                dropCb  = FALSE;

        if (ChiMessageTypeShutter == pMessageDescriptor->messageType)
        {
            // For features using past RDI frames, timestamp may not be available when snapshot meta is available,
            // in that case meta is dispatched from here

            UINT32                    frameNum       = pMessageDescriptor->message.shutterMessage.frameworkFrameNum;
            UINT32                    frameNumIndex  = (frameNum % MaxOutstandingRequests);
            camera3_capture_result_t* pAppResult     = m_pUsecase->GetCaptureResult(frameNumIndex);
            UINT64                    timeStamp      = pMessageDescriptor->message.shutterMessage.timestamp;

            CHX_LOG("Shutter notify Feature JPEG ReqId to AppFrameNum: %d ", frameNum);

            if ((FALSE == m_pUsecase->IsMetadataSent(frameNumIndex)) && (NULL != pAppResult->result))
            {
                ChxUtils::AndroidMetadata::UpdateTimeStamp(
                    const_cast<camera_metadata_t*>(pAppResult->result), timeStamp, frameNum);
                m_pUsecase->SetMetadataAvailable(frameNumIndex);
                CHX_LOG_INFO("Send metadata for AppFrameNum: %d ", frameNum);
                m_pUsecase->ProcessAndReturnFinishedResults();
            }
            else
            {
                if (SnapshotPipelineIndex == pCbData->sessionId)
                {
                    dropCb = TRUE;
                }
                else
                {
                    UINT32 frameNumIndex = (frameNum % MaxOutstandingRequests);
                    m_shutterTimestamp[frameNumIndex] = timeStamp;
                }
            }
        }
        if (ChiMessageTypeMetaBufferDone == pMessageDescriptor->messageType)
        {
            dropCb = TRUE;
            CHX_LOG("FeatureJpeg MetaBuffer Done frameNum %u i/p metadata %p o/p metadata %p",
                pMessageDescriptor->message.metaBufferDoneMessage.frameworkFrameNum,
                pMessageDescriptor->message.metaBufferDoneMessage.inputMetabuffer,
                pMessageDescriptor->message.metaBufferDoneMessage.outputMetabuffer);
        }
        if (FALSE == dropCb)
        {
            m_pUsecase->ReturnFrameworkMessage((camera3_notify_msg_t*)pMessageDescriptor, m_pUsecase->GetCameraId());
        }
    }

    m_pResultMutex->Unlock();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureRawJPEG::RequestThread
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID* FeatureRawJPEG::RequestThread(
    VOID* pThreadData)
{
    PerThreadData* pPerThreadData = reinterpret_cast<PerThreadData*>(pThreadData);

    FeatureRawJPEG* pFeatureRawJPEG = reinterpret_cast<FeatureRawJPEG*>(pPerThreadData->pPrivateData);

    pFeatureRawJPEG->RequestThreadProcessing();

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureRawJPEG::RequestThreadProcessing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureRawJPEG::RequestThreadProcessing()
{
    CDKResult result = CDKResultSuccess;
    CHX_LOG_INFO("FeatureRawJPEG offline RequestThreadProcessing Entered");
    while (1)
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

        while (m_maxSnapshotReqId > m_snapshotReqId)
        {
            UINT    requestIdIndex     = (m_snapshotReqId % MaxOutstandingRequests);
            UINT    frameNumber        = m_snapshotReqIdToFrameNum[requestIdIndex];
            UINT32  requestFrameIndex  = frameNumber % MaxOutstandingRequests;
            BOOL    useFrameworkBuffer = TRUE;

            // Check again, If we have the requirements for the snapshot frame for the corresponding frameNumber
            if (TRUE == m_isSnapshotFrame[requestFrameIndex])
            {
                UINT            rawJPEGInputFrame   = frameNumber;
                CHISTREAMBUFFER snapshotInputBuffer = { 0 };
                ChiMetadata*    pChiRTInputMetadata = m_pSnapshotInputMeta[requestIdIndex];

                // If ZSLmode is enabled, give the recently available SOF frame as input
                if ((FALSE == IsPseudoZSL(m_pSnapshotInputMeta[requestIdIndex])) &&
                    (FALSE == m_pUsecase->IsLLSNeeded()) &&
                    (FALSE == ChxUtils::GetFlashMode(m_pSnapshotInputMeta[requestIdIndex])) &&
                    (FALSE == m_pUsecase->IsFlashNeeded()))
                {
                    UINT32 lastReadyRDIFrameNumber = m_pUsecase->GetLastReadyFrameNum(m_rdiStreamIndex);
                    rawJPEGInputFrame = (INVALIDSEQUENCEID == lastReadyRDIFrameNumber) ? 0 : lastReadyRDIFrameNumber;
                    useFrameworkBuffer = FALSE;
                    CHX_LOG_INFO("rawJPEGInputFrame set in Zsl case = %u", rawJPEGInputFrame);
                }
                else
                {
                    CHX_LOG_INFO("Selected rawJPEGInputFrame in pseudo ZSL case: %u", rawJPEGInputFrame);
                }

                if (TRUE == m_pUsecase->HasRDIBuffer(rawJPEGInputFrame, m_rdiStreamIndex))
                {
                    // Check whether realtime metadata and Rdi buffer for rawJPEGInputFrame are available.
                    CHX_LOG_INFO("waiting for rdi and meta of frame %d for triggering snapshot request", rawJPEGInputFrame);

                    result = m_pUsecase->WaitForBufferMetaReady(rawJPEGInputFrame, m_rdiStreamIndex);

                    if (CDKResultSuccess != result)
                    {
                        CHX_LOG_ERROR("FeatureRawJpeg: wait rdi and meta timeout! frameNumber=%d, rawJPEGInputFrame=%d",
                            frameNumber, rawJPEGInputFrame);
                    }
                }
                else
                {
                    CHX_LOG_ERROR("frameNumber:%d rawJPEGInputFrame:%d doesn't have RDI!", frameNumber, rawJPEGInputFrame);
                    result = CDKResultEFailed;
                }

                if (TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress)))
                {
                    break;
                }

                if (CDKResultSuccess == result)
                {
                    ChiMetadata* pChiSnapshotInputMeta = NULL;

                    m_pUsecase->GetTargetBuffer(rawJPEGInputFrame,
                                                m_pRdiTargetBuffer,
                                                pChiRTInputMetadata,
                                                &snapshotInputBuffer,
                                                &pChiSnapshotInputMeta);

                    // If there is buffer provided by framework for Rdi and non-zsl, then
                    // Override Input buffer with framework's stream buffer.
                    // It was already filled by Raw stream in realtime pipeline.
                    if (TRUE == useFrameworkBuffer)
                    {
                        snapshotInputBuffer.bufferInfo.bufferType = HALGralloc;
                        snapshotInputBuffer.bufferInfo.phBuffer   = m_pRdiFrameworkStreamBuffer[requestFrameIndex].buffer;
                    }

                    if (NULL != pChiSnapshotInputMeta)
                    {
                        pChiSnapshotInputMeta->AddReference();
                    }

                    if (NULL != pChiRTInputMetadata)
                    {
                        pChiRTInputMetadata->ReleaseReference();
                    }

                    const Session* pSession = m_pUsecase->GetSessionData(SnapshotPipelineIndex)->pSession;

                    ChiMetadata* pChiOutputMetadata = m_pMetadataManager->Get(pSession->GetMetadataClientId(), m_snapshotReqId);

                    if (NULL == pChiOutputMetadata || NULL == pChiSnapshotInputMeta)
                    {
                        CHX_LOG_ERROR("RawJpeg: NULL snapshot metadata I/P %p O/P %p frameNumber:%d",
                                      pChiOutputMetadata,
                                      pChiSnapshotInputMeta,
                                      m_snapshotReqId);
                        // fall through.. submit will fail and proceed with next request
                    }

                    CHICAPTUREREQUEST snapshotRequest = { 0 };
                    snapshotRequest.frameNumber       = m_snapshotReqId++;
                    snapshotRequest.hPipelineHandle   = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
                    snapshotRequest.numInputs         = 1;
                    snapshotRequest.numOutputs        = m_snapshotBufferNum[requestIdIndex];
                    snapshotRequest.pInputBuffers     = &snapshotInputBuffer;
                    snapshotRequest.pOutputBuffers    = m_snapshotBuffers[requestIdIndex];
                    snapshotRequest.pInputMetadata    = pChiSnapshotInputMeta ? pChiSnapshotInputMeta->GetHandle() : NULL;
                    snapshotRequest.pOutputMetadata   = pChiOutputMetadata ? pChiOutputMetadata->GetHandle() : NULL;

                    // Save input buffers info for later releasing reference
                    snapshotRequest.pPrivData = &m_offlinePrivData[snapshotRequest.frameNumber % MaxOutstandingRequests];
                    snapshotRequest.pPrivData->inputBuffers[0]      = snapshotInputBuffer.bufferInfo;
                    snapshotRequest.pPrivData->bufferManagers[0]    = m_pRdiTargetBuffer->pBufferManager;
                    snapshotRequest.pPrivData->numInputBuffers      = 1;

                    CHIPIPELINEREQUEST submitRequest = { 0 };
                    submitRequest.pSessionHandle     = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
                    submitRequest.numRequests        = 1;
                    submitRequest.pCaptureRequests   = &snapshotRequest;

                    if (FALSE == pSession->IsPipelineActive())
                    {
                        CDKResult result = CDKResultSuccess;

                        result = ExtensionModule::GetInstance()->ActivatePipeline(pSession->GetSessionHandle(),
                                                                                  pSession->GetPipelineHandle());
                        if (CDKResultSuccess == result)
                        {
                            pSession->SetPipelineActivateFlag();
                        }
                    }

                    CHX_LOG_INFO("Sending RawJPEG snapshot frameNumber:%d, request:%" PRIu64 " metadata:%p",
                                 frameNumber, snapshotRequest.frameNumber, snapshotRequest.pOutputMetadata);
                    if (NULL != pChiSnapshotInputMeta)
                    {
                        pChiSnapshotInputMeta->DumpDetailsToFile("snapshotmeta.txt");
                    }

                    m_pUsecase->LogFeatureRequestMappings(frameNumber, snapshotRequest.frameNumber,
                                                         "RAW Jpeg Snapshot Request");
                    m_pUsecase->SubmitRequest(&submitRequest);
                }
            }
       }
    }

    CHX_LOG_INFO("RequestThreadProcessing Exited");
}
