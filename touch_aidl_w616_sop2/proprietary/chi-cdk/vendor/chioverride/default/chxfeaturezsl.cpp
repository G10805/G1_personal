////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxfeaturezsl.cpp
/// @brief CHX ZSL feature class implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "chxadvancedcamerausecase.h"
#include "chxincs.h"
#include "chxfeaturezsl.h"
#include "chxusecase.h"

// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureZSL::Create
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FeatureZSL* FeatureZSL::Create(
    AdvancedCameraUsecase* pUsecase)
{
    FeatureZSL* pFeature = CHX_NEW FeatureZSL;

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
/// FeatureZSL::Pause
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureZSL::Pause(
        BOOL isForced)
{
    CHX_LOG_CONFIG("FeatureZSL::Pause(), isForced %d E.", isForced);
    ChxUtils::AtomicStoreU32(&m_aPauseInProgress, TRUE);

    m_pOfflineRequestMutex->Lock();
    m_offlineRequestProcessTerminate = TRUE;
    m_pOfflineRequestAvailable->Signal();
    m_pOfflineRequestMutex->Unlock();

    if (NULL != m_offlineRequestProcessThread.pPrivateData)
    {
        CHX_LOG_INFO("Terminating offline thread in featureZSL");
        ChxUtils::ThreadTerminate(m_offlineRequestProcessThread.hThreadHandle);
        CHX_LOG_INFO("Terminated offline thread");
        m_offlineRequestProcessThread = { 0 };
    }
    CHX_LOG_CONFIG("FeatureZSL::Pause(), isForced %d X.", isForced);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureZSL::Initialize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureZSL::Initialize(
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
    m_EmptyMetaData                  = ChxUtils::AndroidMetadata::AllocateMetaData(0, 0);
    m_pMetadataManager               = pUsecase->GetMetadataManager();

    m_offlineRequestProcessThread.pPrivateData = this;

    result = ChxUtils::ThreadCreate(FeatureZSL::RequestThread,
                                    &m_offlineRequestProcessThread,
                                    &m_offlineRequestProcessThread.hThreadHandle);
    if (CDKResultSuccess != result)
    {
        CHX_LOG("Offline request thread create failed with result %d", result);
        m_offlineRequestProcessThread = { 0 };
    }
    m_lastSnapshotInput = INVALIDSEQUENCEID;
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureZSL::Destroy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureZSL::Destroy(
    BOOL isForced)
{
    CHX_LOG_INFO("FeatureZSL::Destroy() isForced %d", isForced);

    m_pSnapshotStream   = NULL;
    m_pPreviewStream    = NULL;
    m_pRdiStream        = NULL;
    m_pFdStream         = NULL;
    m_pVideoStream      = NULL;

    if (NULL != m_pResultMutex)
    {
        m_pResultMutex->Destroy();
        m_pResultMutex = NULL;
    }

    if (NULL != m_pOfflineRequestAvailable)
    {
        m_pOfflineRequestAvailable->Destroy();
        m_pOfflineRequestAvailable = NULL;
    }

    if (NULL != m_pOfflineRequestMutex)
    {
        m_pOfflineRequestMutex->Destroy();
        m_pOfflineRequestMutex = NULL;
    }

    if (NULL != m_EmptyMetaData)
    {
        ChxUtils::AndroidMetadata::FreeMetaData(m_EmptyMetaData);
        m_EmptyMetaData = NULL;
    }

    Feature::DestroyPrivateResources();

    CHX_DELETE(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureZSL::OverrideUsecase
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ChiUsecase* FeatureZSL::OverrideUsecase(
    LogicalCameraInfo*              pCameraInfo,
    camera3_stream_configuration_t* pStreamConfig)
{
    (VOID)pCameraInfo;

    m_pChiUsecase           = m_pUsecase->GetChiUseCase();

    m_pPreviewStream        = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::PreviewStream);
    m_pRdiStream            = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::RdiStream);
    m_pFdStream             = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::FdStream);
    m_pSnapshotStream       = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::SnapshotStream);
    m_pVideoStream          = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::VideoStream);

    if (NULL != m_pVideoStream)
    {
        m_previewPipelineIndex  = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
            AdvancedPipelineType::ZSLPreviewRawYUVType, m_physicalCameraIndex);
        m_continuousRdiCapture  = FALSE;
    }
    else
    {
        m_previewPipelineIndex  = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
            AdvancedPipelineType::ZSLPreviewRawType, m_physicalCameraIndex);
        m_continuousRdiCapture  = TRUE;
    }

    CHX_LOG_INFO("OverrideUsecase for ZSL, num_streams = %d, m_continuousRdiCapture = %d", pStreamConfig->num_streams, m_continuousRdiCapture);

    m_snapshotPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
            AdvancedPipelineType::ZSLSnapshotJpegType, m_physicalCameraIndex);
    if (static_cast<CHISTREAMFORMAT>(HAL_PIXEL_FORMAT_BLOB) != m_pSnapshotStream->format)
    {
        m_snapshotPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
            AdvancedPipelineType::ZSLSnapshotYUVHALType, m_physicalCameraIndex);
    }
    else if (TRUE == ExtensionModule::GetInstance()->UseGPURotationUsecase())
    {
        m_snapshotPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
            AdvancedPipelineType::ZSLSnapshotJpegGPUType, m_physicalCameraIndex);
    }

    ChiPipelineTargetCreateDescriptor* pSnapshotDesc = &m_pChiUsecase->pPipelineTargetCreateDesc[m_snapshotPipelineIndex];
    pSnapshotDesc->sourceTarget.pTargetPortDesc[0].pTarget->pChiStream = m_pRdiStream;
    pSnapshotDesc->sinkTarget.pTargetPortDesc[0].pTarget->pChiStream   = m_pSnapshotStream;

    return m_pChiUsecase;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureZSL::PipelineCreated
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureZSL::PipelineCreated(
    UINT sessionId,          ///< Id of session created
    UINT pipelineIndex)      ///< Index of the pipeline created (within the context of the session)
{
    ChiSensorModeInfo*              pSensorInfo   = NULL;
    const ChiPipelineInputOptions*  pInputOptions = NULL;
    UINT                            pipelineId    = 0;

    pipelineId      = m_pUsecase->GetSessionData(sessionId)->pipelines[pipelineIndex].id;
    pSensorInfo     = m_pUsecase->GetSessionData(sessionId)->pipelines[0].pPipeline->GetSensorModeInfo();
    pInputOptions   = m_pUsecase->GetSessionData(sessionId)->pipelines[0].pPipeline->GetInputOptions();

    CHX_LOG("FeatureZSL sessionId:%d, pipeline index of session:%d, pipelineId:%d, pipelinetype:%d,"
            "Sensor widthxheight: %d x %d, InputOptions widthxheight: %d x %d",
            sessionId,
            pipelineIndex,
            pipelineId,
            m_pUsecase->GetAdvancedPipelineTypeByPipelineId(pipelineId),
            pSensorInfo->frameDimension.width,
            pSensorInfo->frameDimension.height,
            pInputOptions->bufferOptions.optimalDimension.width,
            pInputOptions->bufferOptions.optimalDimension.height);

    CHX_LOG_CONFIG("ZSL pipelineId: %d, Sensor Width x Height: %d x %d, ",
        pipelineId,
        pSensorInfo->frameDimension.width,
        pSensorInfo->frameDimension.height);

    /* Partial Meta data can come from multiple pipeline. The below check is
       to mark the final pipeline that will submit the Partial Meta data*/
    if (m_previewPipelineIndex == sessionId)
    {
        CHX_LOG_INFO("FeatureZSL: Partial Data will be sent from Session:%d Pipeline:%d", sessionId, pipelineIndex);
        m_pUsecase->SetFinalPipelineForPartialMetaData(sessionId);
    }

    /* For realtime pipeline we need to reconfigure RDI based on sensor mode*/
    if (m_previewPipelineIndex == sessionId)
    {
        m_pUsecase->ConfigRdiStream(pSensorInfo, 0);
    }
    SetFeatureStatus(FeatureStatus::READY);

    m_rdiStreamIndex   = m_pUsecase->GetInternalTargetBufferIndex(m_pRdiStream);
    m_fdStreamIndex    = m_pUsecase->GetInternalTargetBufferIndex(m_pFdStream);
    m_pRdiTargetBuffer = m_pUsecase->GetTargetBufferPointer(m_rdiStreamIndex);
    m_pFdTargetBuffer  = m_pUsecase->GetTargetBufferPointer(m_fdStreamIndex);

    CHX_LOG_INFO("rdiStreamIndex:%d, fdStreamIndex:%d", m_rdiStreamIndex, m_fdStreamIndex);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureZSL::ExecuteProcessRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult  FeatureZSL::ExecuteProcessRequest(
    camera3_capture_request_t* pRequest)              ///< Request parameters
{
    CDKResult                  result              = CDKResultSuccess;
    CHISTREAMBUFFER            previewBuffers[3]   = {{0}};
    UINT                       previewCount        = 0;
    UINT                       snapshotCount       = 0;
    UINT                       frameNumber         = pRequest->frame_number;
    UINT32                     sensorModeIndex;
    UINT32                     frameIndex          = (pRequest->frame_number % MaxOutstandingRequests);
    UINT                       snapshotReqIdIndex  = (m_maxSnapshotReqId % MaxOutstandingRequests);

    m_isSnapshotFrame[frameIndex]               = FALSE;
    m_isSkipPreview[frameIndex]                 = FALSE;
    m_isFutureFrameSnapshot[frameIndex]         = FALSE;

    sensorModeIndex = m_pUsecase->GetSessionData(m_previewPipelineIndex)->pSession->GetSensorModeInfo()->modeIndex;
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
        if ((m_pPreviewStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream)) ||
            (m_pVideoStream   == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream)))
        {
            // Capture preview stream
            ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i], &previewBuffers[previewCount]);
            previewCount++;
        }

        if (m_pSnapshotStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream))
        {
            CHX_LOG_INFO("Snapshot Frame %d", pRequest->frame_number);

            if (FALSE == m_pUsecase->m_rejectedSnapshotRequestList[pRequest->frame_number % MaxOutstandingRequests])
            {

                ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i],
                    &m_snapshotBuffers[snapshotReqIdIndex][snapshotCount]);
                snapshotCount++;
            }
            else
            {
                CHX_LOG("Rejecting  only snapshot request for frame %d", pRequest->frame_number);
            }
        }
    }

    PipelineData* pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_previewSessionId, 0));
    UINT          requestIdIndex = (pPipelineData->seqId % MaxOutstandingRequests);

    pPipelineData->seqIdToFrameNum[requestIdIndex] = frameNumber;

    CHX_LOG("FeatureZSL Realtime AppFrameNum to ReqId: %d <--> %d", frameNumber, pPipelineData->seqId);

    if ((TRUE == m_continuousRdiCapture) || (0 < snapshotCount))
    {
        result = m_pUsecase->GetOutputBufferFromRDIQueue(pPipelineData->seqId, m_rdiStreamIndex, &previewBuffers[previewCount]);

        if (CDKResultSuccess == result)
        {
            previewCount++;
        }
        else
        {
            CHX_LOG_ERROR("FeatureZSL: GetOutputBufferFromRDIQueue failed for frameNumber %d", frameNumber);
        }

        if (TRUE == m_pUsecase->IsFDBuffersNeeded())
        {
            result = m_pUsecase->GetOutputBufferFromFDQueue(pPipelineData->seqId, m_fdStreamIndex, &previewBuffers[previewCount]);

            if (CDKResultSuccess == result)
            {
                previewCount++;
            }
            else
            {
                CHX_LOG_ERROR("FeatureZSL: GetOutputBufferFromFDQueue failed, No FD buffer for frameNumber %d", frameNumber);
            }
        }
    }

    const Session* pSession = m_pUsecase->GetSessionData(m_previewPipelineIndex)->pSession;

    CHICAPTUREREQUEST request = { 0 };
    request.frameNumber       = pPipelineData->seqId++;
    request.hPipelineHandle   = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
    request.numOutputs        = previewCount;
    request.pOutputBuffers    = previewBuffers;

    ChiMetadata* pChiRTInputMetadata = m_pMetadataManager->GetInput(pRequest->settings, pRequest->frame_number);
    if (NULL == pChiRTInputMetadata)
    {
        CHX_LOG_ERROR("Input metadata is NULL");
        return CDKResultEFailed;
    }
    request.pInputMetadata      = pChiRTInputMetadata->GetHandle();

    ChiMetadata* pChiOutputMetadata = m_pMetadataManager->Get(pPipelineData->pPipeline->GetMetadataClientId(),
                                                              request.frameNumber);
    if (NULL == pChiOutputMetadata)
    {
        CHX_LOG_ERROR("Output metadata is NULL");
        return CDKResultEFailed;
    }
    request.pOutputMetadata      = pChiOutputMetadata->GetHandle();

    request.pPrivData              = &m_privData[request.frameNumber % MaxOutstandingRequests];
    request.pPrivData->featureType = FeatureType::ZSL;


    CHIPIPELINEREQUEST submitRequest = { 0 };
    submitRequest.pSessionHandle     = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
    submitRequest.numRequests        = 1;
    submitRequest.pCaptureRequests   = &request;

    m_pUsecase->SetRequestToFeatureMapping(m_previewPipelineIndex, request.frameNumber, this);

    if (FALSE == pSession->IsPipelineActive())
    {
        if (ExtensionModule::GetInstance()->GetNumPCRsBeforeStreamOn(const_cast<camera_metadata_t*>(pRequest->settings))
            == pRequest->frame_number)
        {
            result = ExtensionModule::GetInstance()->ActivatePipeline(pSession->GetSessionHandle(),
                pSession->GetPipelineHandle());
            if (CDKResultSuccess == result)
            {
                pSession->SetPipelineActivateFlag();
                CHX_LOG_CONFIG("Setting pipeline activate flag for frame: %d", pRequest->frame_number);
            }
        }
    }

    if (0 < snapshotCount)
    {
        m_pOfflineRequestMutex->Lock();

        m_isSnapshotFrame[frameIndex]                 = TRUE;
        m_snapshotBufferNum[snapshotReqIdIndex]       = snapshotCount;
        m_snapshotReqIdToFrameNum[snapshotReqIdIndex] = frameNumber;
        m_ZSLInputRDIReqId[snapshotReqIdIndex]        = request.frameNumber;

        m_pSnapshotInputMeta[snapshotReqIdIndex]      = pChiRTInputMetadata;

        CHX_LOG_INFO("Added input metadata for snapshot request index: %d", snapshotReqIdIndex);

        pChiRTInputMetadata->AddReference();

        m_maxSnapshotReqId++;

        if ((TRUE == m_pUsecase->IsLLSNeeded()) ||
            (TRUE == IsPseudoZSL(pChiRTInputMetadata))||
            (TRUE == ChxUtils::GetFlashMode(pChiRTInputMetadata)) ||
            (TRUE == m_pUsecase->IsFlashNeeded()))
        {
            // Do Not Skip Preview in case of LLS.
            // if OEM's wants skip preview in LLS, they can make this m_isSkipPreview flag to TRUE.
            CHX_LOG_INFO("ZSL Snapshot Requested using future RDI frame");
            m_isSkipPreview[frameIndex]         = FALSE;
            m_isFutureFrameSnapshot[frameIndex] = TRUE;
        }
        else
        {
            CHX_LOG_INFO("FeatureZSL Trigger Non-LLS snapshot, use Old RDI frame");
            m_pOfflineRequestAvailable->Signal();
        }

        m_pOfflineRequestMutex->Unlock();

        CHX_LOG_INFO("Snapshot in ZSL frame %u", pRequest->frame_number);
    }

    m_pUsecase->LogFeatureRequestMappings(frameNumber, pRequest->frame_number, "ZSL request");
    result = m_pUsecase->SubmitRequest(&submitRequest);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureZSL::ProcessResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureZSL::ProcessResult(
    CHICAPTURERESULT*   pResult,
    VOID*               pPrivateCallbackData)
{
    if (TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress)) ||
        (FlushStatus::NotFlushing != m_pUsecase->GetFlushStatus()))
    {
        CHX_LOG_INFO("Feature paused - Dropping result for frame: %d", pResult->frameworkFrameNum);
        return;
    }
    m_pResultMutex->Lock();

    SessionPrivateData* pCbData               = static_cast<SessionPrivateData*>(pPrivateCallbackData);
    BOOL                isAppResultsAvailable = FALSE;
    UINT32              resultFrameNum        = pResult->frameworkFrameNum;
    UINT32              resultFrameIndex      = resultFrameNum % MaxOutstandingRequests;
    UINT32              rtPipelineReqId       = pResult->frameworkFrameNum;
    ChiMetadata*        pChiInputMetadata     = NULL;
    ChiMetadata*        pChiOutputMetadata    = NULL;

    GetResultFrameInfo(
        pCbData->sessionId,
        pResult->frameworkFrameNum,
        &resultFrameNum,
        &resultFrameIndex);

    if (m_snapshotSessionId == pCbData->sessionId)
    {
        CHX_LOG_INFO("Processing result for snapshot frame %d. Metadata: %p NumBuffers: %d Timestamp: %" PRIu64 " Sent: %d",
            resultFrameNum, pResult->pOutputMetadata, pResult->numOutputBuffers,
            m_pUsecase->GetRequestShutterTimestamp(resultFrameNum),
            m_pUsecase->IsMetadataSent(resultFrameIndex));
    }
    else if (m_previewSessionId == pCbData->sessionId)
    {
        CHX_LOG_INFO("Realtime frame %d. Metadata: %p NumBuffers: %d Timestamp: %" PRIu64 " Sent: %d",
            resultFrameNum, pResult->pOutputMetadata, pResult->numOutputBuffers,
            m_pUsecase->GetRequestShutterTimestamp(resultFrameNum),
            m_pUsecase->IsMetadataSent(resultFrameIndex));
    }

    camera3_capture_result_t* pUsecaseResult = m_pUsecase->GetCaptureResult(resultFrameIndex);

    pUsecaseResult->frame_number = resultFrameNum;

    // If result contain metadata and metadata has not been sent to framework
    if ((NULL != pResult->pOutputMetadata) && (NULL != pResult->pInputMetadata))
    {
        UINT64             timestamp                 = m_pUsecase->GetRequestShutterTimestamp(resultFrameNum);

        // Validate handles
        pChiOutputMetadata = m_pMetadataManager->GetMetadataFromHandle(pResult->pOutputMetadata);
        pChiInputMetadata  = m_pMetadataManager->GetMetadataFromHandle(pResult->pInputMetadata);

        // release input
        m_pMetadataManager->Release(pChiInputMetadata);

        m_isFlashFired = ChxUtils::GetFlashFiredState(pChiOutputMetadata);

        if ((FALSE == m_pUsecase->IsMetadataSent(resultFrameIndex)) && NULL == pUsecaseResult->result)
        {
            // Do Not wait for Snapshot frame metadata, Return Preview metadata back to fwk.
            // If we wait for snapshot, and if it takes more time to process, we will block the preview.
            pUsecaseResult->partial_result = pResult->numPartialMetadata;

            // Override snapshot frame sensor timestamp metadata with shutter event for same frame number
            if (0 != timestamp)
            {
                ChxUtils::UpdateTimeStamp(pChiOutputMetadata, timestamp, resultFrameNum);
                m_pUsecase->SetMetadataAvailable(resultFrameIndex);
                isAppResultsAvailable = TRUE;

                m_pUsecase->UpdateAppResultMetadata(pChiOutputMetadata,
                                                    resultFrameIndex,
                                                    m_pUsecase->GetMetadataClientIdFromPipeline(pCbData->sessionId, 0));
            }
            // If shutter from realtime for snapshot frame has not come yet, metadata will be dispatched from notify
            else
            {
                CHX_LOG("add metadata to capture result for frame %d", resultFrameNum);
                m_pUsecase->UpdateAppResultMetadata(pChiOutputMetadata,
                                                    resultFrameIndex,
                                                    m_pUsecase->GetMetadataClientIdFromPipeline(pCbData->sessionId, 0));
            }
        }


        if (m_previewPipelineIndex == pCbData->sessionId)
        {
            if ((TRUE == m_continuousRdiCapture) || (TRUE == m_isSnapshotFrame[resultFrameIndex]))
            {
                CHX_LOG_INFO("Reserve metadata for frame %d. sessionId %u", resultFrameNum, pCbData->sessionId);
                m_pUsecase->FillMetadataForRDIQueue(rtPipelineReqId, m_rdiStreamIndex, pChiOutputMetadata);
            }
            else
            {
                m_pMetadataManager->Release(pChiOutputMetadata);
            }
        }
        else if (m_snapshotPipelineIndex == pCbData->sessionId)
        {
            m_pMetadataManager->Release(pChiOutputMetadata);
        }
    }

    for (UINT32 j = 0; j < pResult->numOutputBuffers; j++)
    {
        // If our internal stream, copy the result into the target buffer to be consumed by the offline pipeline
        if (m_pRdiStream == pResult->pOutputBuffers[j].pStream)
        {
            m_pUsecase->UpdateBufferReadyForRDIQueue(rtPipelineReqId, m_rdiStreamIndex, TRUE);
            if (TRUE == m_isFutureFrameSnapshot[resultFrameNum % MaxOutstandingRequests])
            {
                CHX_LOG("FeatureZSL Trigger LLS or Flash snapshot, using future RDI frame");
                m_pOfflineRequestAvailable->Signal();
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
            m_pUsecase->GetAppResultMutex()->Unlock();

            if (m_snapshotPipelineIndex == pCbData->sessionId)
            {
                // Release reference to the input buffers of the request to the SnapshotPipeline
                m_pUsecase->ReleaseReferenceToInputBuffers(pResult->pPrivData);
            }

            if ((m_pPreviewStream == pResult->pOutputBuffers[j].pStream) &&
                (TRUE == m_isSkipPreview[resultFrameNum % MaxOutstandingRequests]))
            {
                CHX_LOG("skip the frame for display! resultFrameNum:%d",
                    resultFrameNum);
                ChxUtils::SkipPreviewFrame(pResultBuffer);
            }

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
/// FeatureZSL::ProcessCHIPartialData
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureZSL::ProcessCHIPartialData(
    UINT32    frameNum,
    UINT32    sessionId)
{
    camera3_capture_result_t*   pCHIPartialResult    = NULL;
    UINT32                      resultFrameNum       = frameNum;
    UINT32                      resultFrameIndex     = ChxUtils::GetResultFrameIndexChi(resultFrameNum);
    PartialResultSender         sender               = PartialResultSender::CHIPartialData;

    GetResultFrameInfo(
        sessionId,
        frameNum,
        &resultFrameNum,
        &resultFrameIndex);

    // This check is to ensure that we have not sent earlier and we support sending CHI PartialData
    if (TRUE == m_pUsecase->CheckIfPartialDataCanBeSent(sender, resultFrameIndex))
    {
        pCHIPartialResult       = m_pUsecase->GetCHIPartialCaptureResult(resultFrameIndex);

        pCHIPartialResult->frame_number          = resultFrameNum;
        pCHIPartialResult->input_buffer          = NULL;
        pCHIPartialResult->output_buffers        = NULL;
        pCHIPartialResult->partial_result        = static_cast<int>(ChxUtils::GetPartialResultCount(
            PartialResultSender::CHIPartialData));
        pCHIPartialResult->num_output_buffers    = 0;

        // This needs to be populated as per requirements by the feature. This is kept intentionally
        // as empty for reference.
        pCHIPartialResult->result                = static_cast<camera_metadata*>(m_EmptyMetaData);

        m_pUsecase->ProcessAndReturnPartialMetadataFinishedResults(sender);

    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureZSL::ProcessDriverPartialCaptureResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureZSL::ProcessDriverPartialCaptureResult(
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

    GetResultFrameInfo(
        pCbData->sessionId,
        pResult->frameworkFrameNum,
        &resultFrameNum,
        &resultFrameIndex);

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
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureZSL::ProcessMessage
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureZSL::ProcessMessage(
    const CHIMESSAGEDESCRIPTOR* pMessageDescriptor,
    VOID*                       pPrivateCallbackData)
{
    (VOID)pPrivateCallbackData;

    if (ChiMessageTypeSof == pMessageDescriptor->messageType)
    {
        // SOF notifications are not sent to the HAL3 application
        CHX_LOG("FeatureZSL Chi Notify SOF frameNum %u framework frameNum %u, timestamp %" PRIu64,
            pMessageDescriptor->message.sofMessage.sofId,
            pMessageDescriptor->message.sofMessage.frameworkFrameNum,
            pMessageDescriptor->message.sofMessage.timestamp);

        // If SOF notification needs to be sent as shutter notification for App Processing,
        // then we can utilize CHI Partial Data to send the info.
        // Below is an example
        if ((PartialMetaSupport::SeperatePartialMeta == ExtensionModule::GetInstance()->EnableCHIPartialData()) &&
            (pMessageDescriptor->message.sofMessage.frameworkFrameNum > 0))
        {
            ProcessCHIPartialData(pMessageDescriptor->message.sofMessage.frameworkFrameNum,
                m_previewSessionId);
        }
    }
    else if (ChiMessageTypeMetaBufferDone == pMessageDescriptor->messageType)
    {
        CHX_LOG("FeatureZSL MetaBuffer Done frameNum %u i/p metadata %p o/p metadata %p",
            pMessageDescriptor->message.metaBufferDoneMessage.frameworkFrameNum,
            pMessageDescriptor->message.metaBufferDoneMessage.inputMetabuffer,
            pMessageDescriptor->message.metaBufferDoneMessage.outputMetabuffer);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureZSL::IsPseudoZSL
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL FeatureZSL::IsPseudoZSL(
    ChiMetadata* pMetadata)
{
    // It is Pseudo ZSL
    // if application hasn't set the ZSL mode OR

    UINT zslMode = 0;
    m_pUsecase->GetZSLMode(&zslMode, pMetadata);

    CHX_LOG("zslMode: %d ", zslMode);

    return (0 == zslMode);
}

INT32 FeatureZSL::GetRequiredPipelines(
    AdvancedPipelineType* pPipelines,
    INT32 size)
{
    INT32 count = 0;
    const INT32 pipelineCount = 2;

    if (NULL != pPipelines && size >= pipelineCount)
    {
        INT32 index = 0;
        AdvancedPipelineType pipelineGroup[1];
        UINT                 cameraId[1];
        UINT                 physicalCameraID = m_pUsecase->GetPhysicalCameraId(m_physicalCameraIndex);

        if (InvalidPhysicalCameraId != physicalCameraID)
        {
            pPipelines[index] = AdvancedPipelineType::ZSLSnapshotJpegType;
            // If the format isnt blob->JPEG fallback to YUV
            ChiStream* pStream = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::SnapshotStream);
            if (static_cast<CHISTREAMFORMAT>(HAL_PIXEL_FORMAT_BLOB) != pStream->format)
            {
                pPipelines[index] = AdvancedPipelineType::ZSLSnapshotYUVHALType;
            }
            else if (TRUE == ExtensionModule::GetInstance()->UseGPURotationUsecase())
            {
                pPipelines[index] = AdvancedPipelineType::ZSLSnapshotJpegGPUType;
            }
            pipelineGroup[0]    = pPipelines[index];
            cameraId[0]         = physicalCameraID;
            m_snapshotSessionId = m_pUsecase->GetUniqueSessionId(&pipelineGroup[0], 1, m_physicalCameraIndex);
            m_pUsecase->SetPipelineCameraId(&pipelineGroup[0], &cameraId[0], 1);
            index++;

            if (FALSE == m_pUsecase->IsMultiCameraUsecase())
            // if (TRUE)
            {
                if (NULL == m_pUsecase->GetSharedStream(AdvancedCameraUsecase::VideoStream))
                {
                    pPipelines[index] = AdvancedPipelineType::ZSLPreviewRawType;
                }
                else
                {
                    pPipelines[index] = AdvancedPipelineType::ZSLPreviewRawYUVType;
                }
                pipelineGroup[0]    = pPipelines[index];
                cameraId[0]         = physicalCameraID;
                m_previewSessionId = m_pUsecase->GetUniqueSessionId(&pipelineGroup[0], 1, m_physicalCameraIndex);
                m_pUsecase->SetPipelineCameraId(&pipelineGroup[0], &cameraId[0], 1);
                index++;
            }
            else
            {
                // m_previewSessionId = 0xFFFFFFFF;
                CHX_LOG("Don't use Realtime pipeline in advance usecase for multicamera usecase");
            }

            count = index;
        }
    }

    CHX_LOG("FeatureZSL::GetRequiredPipelines, required pipeline count:%d video stream=%p", count,
        m_pUsecase->GetSharedStream(AdvancedCameraUsecase::VideoStream));
    return count;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureZSL::RequestThread
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID* FeatureZSL::RequestThread(
    VOID* pThreadData)
{
    PerThreadData* pPerThreadData = reinterpret_cast<PerThreadData*>(pThreadData);

    FeatureZSL* pFeatureZSL = reinterpret_cast<FeatureZSL*>(pPerThreadData->pPrivateData);

    pFeatureZSL->RequestThreadProcessing();

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureZSL::RequestThreadProcessing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureZSL::RequestThreadProcessing()
{
    CDKResult result = CDKResultSuccess;

    CHX_LOG_INFO("RequestThreadProcessing Entered");

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

        while (m_maxSnapshotReqId > m_snapshotReqId && m_offlineRequestProcessTerminate == FALSE)
        {
            UINT   requestIdIndex    = (m_snapshotReqId % MaxOutstandingRequests);
            UINT   frameNumber       = m_snapshotReqIdToFrameNum[requestIdIndex];
            UINT32 requestFrameIndex = frameNumber % MaxOutstandingRequests;

            // Check again, If we have the requirements for the snapshot frame for the corresponding frameNumber
            if (TRUE == m_isSnapshotFrame[requestFrameIndex])
            {
                UINT            zslInputFrameNumber = m_ZSLInputRDIReqId[requestIdIndex];
                CHISTREAMBUFFER snapshotInputBuffer = { 0 };

                // Extract the current request settings to be merged with metadata of zsl queue
                ChiMetadata*  pChiRTInputMetadata = m_pSnapshotInputMeta[requestIdIndex];
                CHX_LOG("Getting input metadata for snapshot request index: %d count %d",
                        requestIdIndex, pChiRTInputMetadata->Count());

                UINT32 lastReadyRDIFrameNumber;
                if (TRUE == m_isFutureFrameSnapshot[requestFrameIndex])
                {
                    lastReadyRDIFrameNumber = m_ZSLInputRDIReqId[requestIdIndex];
                }
                else
                {
                    lastReadyRDIFrameNumber = m_pUsecase->GetLastReadyFrameNum(m_rdiStreamIndex);
                }

                zslInputFrameNumber                 = (INVALIDSEQUENCEID == lastReadyRDIFrameNumber)
                                                    ? 0 : lastReadyRDIFrameNumber;

                // If last snapshot input is equal or greater than zslInputFrameNumber,
                // zslInputFrameNumber should be wrong
                if ((m_lastSnapshotInput != INVALIDSEQUENCEID) && (m_lastSnapshotInput >= zslInputFrameNumber))
                {
                    zslInputFrameNumber = m_lastSnapshotInput + 1;
                }

                m_lastSnapshotInput = zslInputFrameNumber;
                CHX_LOG_INFO("zslInputFrameNumber   = %u", zslInputFrameNumber);

                if (TRUE == m_pUsecase->HasRDIBuffer(zslInputFrameNumber, m_rdiStreamIndex))
                {

                    // Check whether realtime metadata and Rdi buffer for zslInputFrameNumber are available.
                    CHX_LOG_CONFIG("Waiting for RDI/Meta of zslInputFrameNumber %d, requestIdIndex: %d, requestFrameIndex: %d "
                                    "for triggering snapshot request",
                                    zslInputFrameNumber,
                                    requestIdIndex,
                                    requestFrameIndex);

                    result = m_pUsecase->WaitForBufferMetaReady(zslInputFrameNumber, m_rdiStreamIndex);

                    if (CDKResultSuccess != result)
                    {
                        CHX_LOG_ERROR("FeatureZSL: wait rdi and meta timeout! frameNumber=%d, zslInputFrameNumber=%d",
                                      frameNumber,
                                      zslInputFrameNumber);
                        return;
                    }
                }
                else
                {
                    UINT32 zslFrameworkFrameNumber      = 0;
                    UINT32 zslFrameworkFrameIndex       = 0;
                    GetResultFrameInfo(m_previewSessionId, zslInputFrameNumber,
                        &zslFrameworkFrameNumber, &zslFrameworkFrameIndex);

                    CHX_LOG_ERROR("Requested frameNumber:%d doesn't have RDI! Last ready zslInputFrameNumber:%d",
                        frameNumber, zslFrameworkFrameNumber);
                    break;
                }

                if (TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress)))
                {
                    CHX_LOG_INFO("Destroy is in progress come out of loop");
                    break;
                }

                ChiMetadata* pChiSnapshotInputMeta = NULL;

                m_pUsecase->GetTargetBuffer(zslInputFrameNumber,
                                            m_pRdiTargetBuffer,
                                            pChiRTInputMetadata,
                                            &snapshotInputBuffer,
                                            &pChiSnapshotInputMeta);

                CHX_LOG("buffer handle:%p", snapshotInputBuffer.bufferInfo.phBuffer);

                PipelineData* pSnapshotPipelineData = const_cast<PipelineData*>(
                    m_pUsecase->GetPipelineData(m_snapshotSessionId, 0));

                ChiMetadata* pChiOutputMetadata = m_pMetadataManager->Get(
                    pSnapshotPipelineData->pPipeline->GetMetadataClientId(),
                    m_snapshotReqId);

                if ((NULL == pChiOutputMetadata) || (NULL == pChiSnapshotInputMeta))
                {
                    CHX_LOG_ERROR("ERROR Failed to get snapshot metadata buffer, in %p out %p cannot submit request",
                                  pChiSnapshotInputMeta, pChiOutputMetadata);
                    result = CDKResultEResource;
                    return;
                }
                else
                {
                    pSnapshotPipelineData->seqIdToFrameNum[requestIdIndex] = frameNumber;

                    pChiSnapshotInputMeta->AddReference();
                    pChiRTInputMetadata->ReleaseReference();
                    const Session* pSession = m_pUsecase->GetSessionData(m_snapshotPipelineIndex)->pSession;
                    CHICAPTUREREQUEST snapshotRequest = { 0 };
                    snapshotRequest.frameNumber       = m_snapshotReqId++;
                    snapshotRequest.hPipelineHandle   = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
                    snapshotRequest.numInputs         = 1;
                    snapshotRequest.numOutputs        = m_snapshotBufferNum[requestIdIndex];
                    snapshotRequest.pInputBuffers     = &snapshotInputBuffer;
                    snapshotRequest.pOutputBuffers    = m_snapshotBuffers[requestIdIndex];
                    snapshotRequest.pInputMetadata    = pChiSnapshotInputMeta ? pChiSnapshotInputMeta->GetHandle() : NULL;
                    snapshotRequest.pOutputMetadata   = pChiOutputMetadata ? pChiOutputMetadata->GetHandle() : NULL;
                    snapshotRequest.pPrivData         = &m_privData[snapshotRequest.frameNumber % MaxOutstandingRequests];
                    snapshotRequest.pPrivData->featureType = FeatureType::ZSL;
                    // Save input buffers info for later releasing reference
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
                        else
                        {
                            CHX_LOG_ERROR("ERROR Failed to activate the pipeline");
                        }
                    }
                    if (FlushStatus::NotFlushing == m_pUsecase->GetFlushStatus())
                    {
                        CHX_LOG_CONFIG("Sending ZSL snapshot requestIdIndex:%d, requestFrameIndex:%d, frameNumber:%d "
                                    "for RDI frame:%d, request:%" PRIu64 " metadata:%p count %d",
                                    requestIdIndex,
                                    requestFrameIndex,
                                    frameNumber,
                                    zslInputFrameNumber,
                                    snapshotRequest.frameNumber,
                                    pChiSnapshotInputMeta,
                                    pChiSnapshotInputMeta->Count());
                        m_pUsecase->SetRequestToFeatureMapping(m_snapshotPipelineIndex, snapshotRequest.frameNumber, this);

                        m_pUsecase->LogFeatureRequestMappings(frameNumber, snapshotRequest.frameNumber, "ZSL snapshot request");
                        m_pUsecase->SubmitRequest(&submitRequest);
                    }
                }
            }
        }
    }

    CHX_LOG_INFO("RequestThreadProcessing Exited");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureZSL::GetResultFrameInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureZSL::GetResultFrameInfo(
    UINT32 sessionId,
    UINT32 captureResultframeworkFrameNum,
    UINT32 *pResultFrameNumber,
    UINT32 *pResultFrameIndex)
{
    *pResultFrameNumber = captureResultframeworkFrameNum;
    *pResultFrameIndex  = captureResultframeworkFrameNum % MaxOutstandingRequests;

    if (m_snapshotSessionId == sessionId)
    {
        *pResultFrameNumber = m_snapshotReqIdToFrameNum[captureResultframeworkFrameNum % MaxOutstandingRequests];
        *pResultFrameIndex  = *pResultFrameNumber % MaxOutstandingRequests;
    }
    else if (m_previewSessionId == sessionId)
    {
        *pResultFrameNumber = m_pUsecase->GetChiFrameNumFromReqId(m_previewSessionId, 0, captureResultframeworkFrameNum);
        *pResultFrameIndex  = *pResultFrameNumber % MaxOutstandingRequests;
    }
    CHX_LOG("FeatureZSL AppFrameNum: %d <--> %d",
        captureResultframeworkFrameNum,
        *pResultFrameNumber);
}
