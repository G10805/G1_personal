////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxfeatureyuvcb.cpp
/// @brief CHX YUV callback stream feature class implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "chxadvancedcamerausecase.h"
#include "chxincs.h"
#include "chxfeatureyuvcb.h"
#include "chxusecase.h"

// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureYuvCB::Create
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FeatureYuvCB* FeatureYuvCB::Create(
    AdvancedCameraUsecase* pUsecase)
{
    FeatureYuvCB* pFeature = CHX_NEW FeatureYuvCB;

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
/// FeatureYuvCB::Pause
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureYuvCB::Pause(
    BOOL isForced)
{
    CHX_LOG_INFO("FeatureZSL::Pause(), isForced %d E.", isForced);
    ChxUtils::AtomicStoreU32(&m_aPauseInProgress, TRUE);

    m_pOfflineRequestMutex->Lock();
    m_offlineRequestProcessTerminate = TRUE;
    m_pOfflineRequestAvailable->Signal();
    m_pOfflineRequestMutex->Unlock();

    if (NULL != m_offlineRequestProcessThread.pPrivateData)
    {
        CHX_LOG("Terminating offline thread in featureZSL");
        ChxUtils::ThreadTerminate(m_offlineRequestProcessThread.hThreadHandle);
        CHX_LOG("Terminated offline thread");
        m_offlineRequestProcessThread = { 0 };
    }
    CHX_LOG_INFO("FeatureZSL::Pause(), isForced %d X.", isForced);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureYuvCB::Initialize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureYuvCB::Initialize(
    AdvancedCameraUsecase* pUsecase)
{
    CDKResult    result = CDKResultEFailed;

    Feature::InitializePrivateResources();

    m_pUsecase     = pUsecase;
    m_pResultMutex = Mutex::Create();

    m_maxSnapshotReqId               = 0;
    m_snapshotReqId                  = 0;

    m_pOfflineRequestMutex           = Mutex::Create();
    m_pOfflineRequestAvailable       = Condition::Create();
    m_offlineRequestProcessTerminate = FALSE;
    m_aPauseInProgress               = FALSE;
    m_pMetadataManager               = pUsecase->GetMetadataManager();

    if ((NULL != m_pOfflineRequestMutex) && (NULL != m_pOfflineRequestAvailable))
    {
        m_offlineRequestProcessThread.pPrivateData = this;

        result = ChxUtils::ThreadCreate(FeatureYuvCB::RequestThread,
                                        &m_offlineRequestProcessThread,
                                        &m_offlineRequestProcessThread.hThreadHandle);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureYuvCB::Destroy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureYuvCB::Destroy(BOOL isForced)
{
    CHX_LOG("isForced %d ", isForced);

    if ((NULL != m_pOfflineRequestMutex) && (NULL != m_pOfflineRequestAvailable))
    {
        m_pOfflineRequestMutex->Lock();
        m_offlineRequestProcessTerminate = TRUE;
        m_pOfflineRequestAvailable->Signal();
        m_pOfflineRequestMutex->Unlock();

        ChxUtils::ThreadTerminate(m_offlineRequestProcessThread.hThreadHandle);
    }

    m_pYuvCBStream   = NULL;
    m_pPreviewStream = NULL;
    m_pRdiStream     = NULL;
    m_pFdStream      = NULL;
    m_pBlobStream    = NULL;

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

    Feature::DestroyPrivateResources();

    CHX_DELETE(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureYuvCB::OverrideUsecase
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ChiUsecase* FeatureYuvCB::OverrideUsecase(
    LogicalCameraInfo*              pCameraInfo,
    camera3_stream_configuration_t* pStreamConfig)
{
    (VOID)pCameraInfo;
    CHX_LOG("OverrideUsecase for YuvCB and Blob 4 stream");
    CHX_ASSERT(4 == pStreamConfig->num_streams);

    m_pChiUsecase = m_pUsecase->GetChiUseCase();

    m_pPreviewStream = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::PreviewStream);
    m_pRdiStream     = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::RdiStream);
    m_pFdStream      = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::FdStream);
    m_pYuvCBStream   = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::YuvCBStream);
    m_pBlobStream    = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::SnapshotStream);
    m_pYuvInStream   = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::YuvInputStream);

    m_previewPipelineIndex  = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
        AdvancedPipelineType::ZSLPreviewRawType, m_physicalCameraIndex);

    if (static_cast<CHISTREAMFORMAT>(HAL_PIXEL_FORMAT_BLOB) != m_pYuvCBStream->format)
    {
        m_YuvCBPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                    AdvancedPipelineType::ZSLSnapshotYUVType, m_physicalCameraIndex);
    }
    else
    {
        CHX_LOG_ERROR("expected only YUV request in YuvCB stream");
    }

    ChiPipelineTargetCreateDescriptor* pYuvCBDesc = &m_pChiUsecase->pPipelineTargetCreateDesc[m_YuvCBPipelineIndex];
    pYuvCBDesc->sourceTarget.pTargetPortDesc[0].pTarget->pChiStream = m_pRdiStream;
    pYuvCBDesc->sinkTarget.pTargetPortDesc[0].pTarget->pChiStream   = m_pYuvCBStream;

    m_offlineBlobPipelineIndex =
        m_pUsecase->GetPipelineIdByAdvancedPipelineType(
            AdvancedPipelineType::InternalZSLYuv2JpegType, m_physicalCameraIndex);
    ChiPipelineTargetCreateDescriptor* pBlobDesc = &m_pChiUsecase->pPipelineTargetCreateDesc[m_offlineBlobPipelineIndex];
    pBlobDesc->sourceTarget.pTargetPortDesc[0].pTarget->pChiStream = m_pYuvInStream;
    pBlobDesc->sinkTarget.pTargetPortDesc[0].pTarget->pChiStream   = m_pBlobStream;

    m_offlineYuvPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                    AdvancedPipelineType::ZSLYuv2YuvType, m_physicalCameraIndex);
    ChiPipelineTargetCreateDescriptor* pOfflineYuvDesc = &m_pChiUsecase->pPipelineTargetCreateDesc[m_offlineYuvPipelineIndex];
    pOfflineYuvDesc->sourceTarget.pTargetPortDesc[0].pTarget->pChiStream = m_pYuvInStream;
    pOfflineYuvDesc->sinkTarget.pTargetPortDesc[0].pTarget->pChiStream   = m_pYuvCBStream;

    m_realTimeBlobPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                    AdvancedPipelineType::ZSLSnapshotJpegType, m_physicalCameraIndex);
    if (TRUE == ExtensionModule::GetInstance()->UseGPURotationUsecase())
    {
        m_realTimeBlobPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType
                                        (AdvancedPipelineType::ZSLSnapshotJpegGPUType, m_physicalCameraIndex);
    }

    ChiPipelineTargetCreateDescriptor* pSnapshotDesc = &m_pChiUsecase->pPipelineTargetCreateDesc[m_realTimeBlobPipelineIndex];
    pSnapshotDesc->sourceTarget.pTargetPortDesc[0].pTarget->pChiStream = m_pRdiStream;
    pSnapshotDesc->sinkTarget.pTargetPortDesc[0].pTarget->pChiStream   = m_pBlobStream;


    return m_pChiUsecase;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureYuvCB::PipelineCreated
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureYuvCB::PipelineCreated(
    UINT sessionId,         ///< Id of session created
    UINT pipelineIndex)    ///< Index of the pipeline created (within the context of the session)
{
    (VOID)pipelineIndex;
    ChiSensorModeInfo*              pSensorInfo = NULL;

    pSensorInfo = m_pUsecase->GetSessionData(sessionId)->pipelines[0].pPipeline->GetSensorModeInfo();
    if (NULL == pSensorInfo)
    {
        CHX_LOG_ERROR("pSensorInfo is NULL");
        return;
    }

    m_rdiStreamIndex = m_pUsecase->GetInternalTargetBufferIndex(m_pRdiStream);
    m_fdStreamIndex  = m_pUsecase->GetInternalTargetBufferIndex(m_pFdStream);
    m_pRdiTargetBuffer = m_pUsecase->GetTargetBufferPointer(m_rdiStreamIndex);
    m_pFdTargetBuffer = m_pUsecase->GetTargetBufferPointer(m_fdStreamIndex);

    m_pUsecase->ConfigRdiStream(pSensorInfo, 0);
    m_pUsecase->ConfigFdStream();
    SetFeatureStatus(FeatureStatus::READY);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureYuvCB::ExecuteProcessRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult  FeatureYuvCB::ExecuteProcessRequest(
    camera3_capture_request_t* pRequest)              ///< Request parameters
{
    CDKResult       result             = CDKResultSuccess;
    CHISTREAMBUFFER previewBuffers[3]  = {{0}};
    UINT            previewCount       = 0;
    UINT            snapshotCount      = 0;
    UINT            frameNumber        = pRequest->frame_number;
    UINT32          sensorModeIndex;
    UINT32          frameIndex         = (pRequest->frame_number % MaxOutstandingRequests);
    UINT            snapshotReqIdIndex = (m_maxSnapshotReqId % MaxOutstandingRequests);
    BOOL            bIsRealTimeRequest = TRUE;
    CHISTREAMBUFFER offlineBuffers[2]  = { { 0 } };
    UINT            offlineRequestCnt  = 0;
    UINT            pipelineIndex      = 0;

    m_isSnapshotFrame[frameIndex]       = FALSE;
    m_isRealTimeBlobFrame[frameIndex]   = FALSE;

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
        if (NULL == pRequest->input_buffer)
        {
            bIsRealTimeRequest = TRUE;
            if (m_pPreviewStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream))
            {
                // Capture preview stream
                ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i], &previewBuffers[previewCount]);

                previewCount++;
            }
            else if ((m_pYuvCBStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream)) ||
                (m_pBlobStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream)))
            {

                ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i],
                                                       &m_snapshotBuffers[snapshotReqIdIndex][snapshotCount]);
                snapshotCount++;
                if (m_pBlobStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream))
                {
                    m_isRealTimeBlobFrame[frameIndex]   = TRUE;
                }
            }
            else
            {
                CHX_LOG_ERROR("YuvCB: Not expected stream");
            }
        }
        else if (m_pBlobStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream))
        {
            ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i], &offlineBuffers[offlineRequestCnt]);

            bIsRealTimeRequest = FALSE;
            offlineRequestCnt++;
            pipelineIndex       = m_offlineBlobPipelineIndex;
            CHX_LOG("YuvCB submitting to offlineBlob %d", pRequest->frame_number);
        }
        else
        {
            ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i], &offlineBuffers[offlineRequestCnt]);

            bIsRealTimeRequest = FALSE;
            offlineRequestCnt++;
            pipelineIndex       = m_offlineYuvPipelineIndex;
            CHX_LOG("YuvCB submitting to offlineYUV %d", pRequest->frame_number);
        }
    }

    if (TRUE == bIsRealTimeRequest)
    {
        PipelineData* pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_previewSessionId, 0));
        UINT          requestIdIndex = (pPipelineData->seqId % MaxOutstandingRequests);

        pPipelineData->seqIdToFrameNum[requestIdIndex] = frameNumber;

        CHX_LOG("FeatureYuvCB Realtime AppFrameNum to ReqId: %d <--> %d", frameNumber, pPipelineData->seqId);

        result = m_pUsecase->GetOutputBufferFromRDIQueue(frameNumber, m_rdiStreamIndex, &previewBuffers[previewCount]);

        if (CDKResultSuccess == result)
        {
            previewCount++;
        }
        else
        {
            CHX_LOG_ERROR("FeatureYUVCb: GetOutputBufferFromRDIQueue failed for frameNumber %d", frameNumber);
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
                CHX_LOG_ERROR("FeatureYUVCB: GetOutputBufferFromFDQueue failed, No FD buffer for frameNumber %d", frameNumber);
            }
        }

        const Session* pSession = m_pUsecase->GetSessionData(m_previewPipelineIndex)->pSession;

        ChiMetadataBundle rtMetaBundle;
        result = m_pUsecase->GetMetadataBundle(pRequest->settings, pRequest->frame_number, rtMetaBundle,
            pPipelineData->pPipeline->GetMetadataClientId());

        if (result != CDKResultSuccess)
        {
            CHX_LOG_ERROR("FeatureYUVCB: Cannot get metadata frameNumber %d", frameNumber);
            return result;
        }

        CHICAPTUREREQUEST request = { 0 };
        request.frameNumber       = pPipelineData->seqId++;
        request.hPipelineHandle   = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
        request.numOutputs        = previewCount;
        request.pOutputBuffers    = previewBuffers;
        request.pInputMetadata    = rtMetaBundle.pInputMetadata->GetHandle();
        request.pOutputMetadata   = rtMetaBundle.pOutputMetadata->GetHandle();

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
                }
            }
        }

        if (0 < snapshotCount)
        {
            m_pUsecase->WaitForDeferThread();
            m_pOfflineRequestMutex->Lock();

            m_isSnapshotFrame[frameIndex]                 = TRUE;
            m_snapshotBufferNum[snapshotReqIdIndex]       = snapshotCount;
            m_snapshotReqIdToFrameNum[snapshotReqIdIndex] = frameNumber;
            m_maxSnapshotReqId++;
            m_pSnapshotInputMeta[snapshotReqIdIndex]      = rtMetaBundle.pInputMetadata;
            rtMetaBundle.pInputMetadata->AddReference();

            m_pOfflineRequestAvailable->Signal();
            m_pOfflineRequestMutex->Unlock();

            CHX_LOG_ERROR("YUV callback request in ZSL");
        }

        m_pUsecase->LogFeatureRequestMappings(frameNumber, pRequest->frame_number, "YUV Realtime Request");
        result = m_pUsecase->SubmitRequest(&submitRequest);
    }
    else
    {
        CHICAPTUREREQUEST request       = { 0 };
        CHISTREAMBUFFER   inputBuffer   = { 0 };

        if (NULL != pRequest->input_buffer)
        {
            ChxUtils::PopulateHALToChiStreamBuffer(pRequest->input_buffer, &inputBuffer);

            request.numInputs     = 1;
            request.pInputBuffers = &inputBuffer;
        }

        PipelineData* pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(pipelineIndex, 0));

        ChiMetadata* pReprocessInputMeta  = m_pMetadataManager->GetInput(pRequest->settings, pRequest->frame_number, FALSE);
        ChiMetadata* pReprocessOutputMeta = m_pMetadataManager->Get(
            pPipelineData->pPipeline->GetMetadataClientId(), frameNumber);

        if (NULL == pReprocessOutputMeta || NULL == pReprocessInputMeta)
        {
            CHX_LOG_INFO("Metadata is null pInputMeta %p pOutputMeta %p", pReprocessInputMeta, pReprocessOutputMeta);
            return CDKResultEFailed;
        }

        const Session* pSession   = m_pUsecase->GetSessionData(pipelineIndex)->pSession;
        request.frameNumber       = pRequest->frame_number;
        request.hPipelineHandle   = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
        request.numOutputs        = offlineRequestCnt;
        request.pOutputBuffers    = offlineBuffers;
        request.pInputMetadata    = pReprocessInputMeta->GetHandle();
        request.pOutputMetadata   = pReprocessOutputMeta->GetHandle();
        request.pPrivData         = &m_privData[frameIndex];

        CHIPIPELINEREQUEST submitRequest = { 0 };
        submitRequest.pSessionHandle     = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
        submitRequest.numRequests        = 1;
        submitRequest.pCaptureRequests   = &request;

        m_pUsecase->SetRequestToFeatureMapping(pipelineIndex, request.frameNumber, this);

        if (FALSE == pSession->IsPipelineActive())
        {
            result =
                ExtensionModule::GetInstance()->ActivatePipeline(pSession->GetSessionHandle(),
                                                                    pSession->GetPipelineHandle());
            if (CDKResultSuccess == result)
            {
                pSession->SetPipelineActivateFlag();
            }
        }

        m_pUsecase->LogFeatureRequestMappings(frameNumber, pRequest->frame_number, "YUV Offline Request");
        result = m_pUsecase->SubmitRequest(&submitRequest);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureYuvCB::ProcessResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureYuvCB::ProcessResult(
    CHICAPTURERESULT*   pResult,
    VOID*               pPrivateCallbackData)
{
    if ((TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress))))
    {
        CHX_LOG_INFO("YUV process result return because of cleanup");
        return;
    }

    m_pResultMutex->Lock();

    SessionPrivateData* pCbData               = static_cast<SessionPrivateData*>(pPrivateCallbackData);
    BOOL                isAppResultsAvailable = FALSE;
    UINT32              resultFrameNum        = pResult->frameworkFrameNum;
    UINT32              resultFrameIndex      = resultFrameNum % MaxOutstandingRequests;
    UINT32              rtPipelineReqId       = 0;
    BOOL                bReprocessPipeLine    = FALSE;

    if ((m_YuvCBPipelineIndex == pCbData->sessionId) || (m_realTimeBlobPipelineIndex == pCbData->sessionId))
    {
        resultFrameNum   = m_snapshotReqIdToFrameNum[pResult->frameworkFrameNum % MaxOutstandingRequests];
        resultFrameIndex = resultFrameNum % MaxOutstandingRequests;
    }
    else if (m_previewSessionId == pCbData->sessionId)
    {
        UINT32 rtResultId     = pResult->frameworkFrameNum;
        resultFrameNum        = m_pUsecase->GetChiFrameNumFromReqId(m_previewSessionId, 0, rtResultId);
        resultFrameIndex      = resultFrameNum % MaxOutstandingRequests;
        rtPipelineReqId       = resultFrameNum;

        CHX_LOG("Realtime ReqId to Chi FrameNum: %d <--> %d", pResult->frameworkFrameNum, resultFrameNum);
    }
    else
    {
        bReprocessPipeLine = TRUE;
    }

    camera3_capture_result_t* pUsecaseResult   = m_pUsecase->GetCaptureResult(resultFrameIndex);

    pUsecaseResult->frame_number = resultFrameNum;

    // If result contain metadata
    if ((NULL != pResult->pOutputMetadata) && (NULL != pResult->pInputMetadata))
    {
        BOOL               isResultMetadataAvailable = FALSE;
        UINT64             timestamp                 = m_pUsecase->GetRequestShutterTimestamp(resultFrameNum);
        ChiMetadata*       pOutputMetadata           = m_pMetadataManager->GetMetadataFromHandle(pResult->pOutputMetadata);
        ChiMetadata*       pInputMetadata            = m_pMetadataManager->GetMetadataFromHandle(pResult->pInputMetadata);

        // release input
        m_pMetadataManager->Release(pInputMetadata);

        if ((FALSE == m_pUsecase->IsMetadataSent(resultFrameIndex)) && (NULL == pUsecaseResult->result))
        {
            // Do Not wait for Snapshot frame metadata, Return Preview metadata back to fwk.
            // If we wait for snapshot, and if it takes more time to process, we will block the preview.
            pUsecaseResult->result = static_cast<const camera_metadata_t*>(pResult->pResultMetadata);
            pUsecaseResult->partial_result = pResult->numPartialMetadata;
            isResultMetadataAvailable = TRUE;

            // Override snapshot frame sensor timestamp metadata with shutter event for same frame number
            if (0 != timestamp)
            {
                ChxUtils::UpdateTimeStamp(pOutputMetadata, timestamp, resultFrameNum);
                m_pUsecase->SetMetadataAvailable(resultFrameIndex);
                isAppResultsAvailable = TRUE;

                m_pUsecase->UpdateAppResultMetadata(pOutputMetadata,
                                                    resultFrameIndex,
                                                    m_pUsecase->GetMetadataClientIdFromPipeline(pCbData->sessionId, 0));
                CHX_LOG("returnmetadata SWMF JPEG metadata：%d", resultFrameNum);
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

        if (m_previewPipelineIndex == pCbData->sessionId)
        {
            m_pUsecase->FillMetadataForRDIQueue(rtPipelineReqId, m_rdiStreamIndex, pOutputMetadata);
        }
        else
        {
            m_pMetadataManager->Release(pOutputMetadata);
        }

        // check if flash is required for snapshot
        m_isFlashRequired = m_pUsecase->IsFlashRequired(*pOutputMetadata);
    }

    if (TRUE == bReprocessPipeLine)
    {
        if (NULL != &pResult->pInputBuffer[0])
        {
            camera3_stream_buffer_t* pResultInBuffer =
                const_cast<camera3_stream_buffer_t*>(pUsecaseResult->input_buffer);

            ChxUtils::PopulateChiToHALStreamBuffer(pResult->pInputBuffer, pResultInBuffer);

        }
        isAppResultsAvailable = TRUE;
    }

    for (UINT32 j = 0; j < pResult->numOutputBuffers; j++)
    {
        // If our internal stream, copy the result into the target buffer to be consumed by the offline pipeline
        if (m_pRdiStream == pResult->pOutputBuffers[j].pStream)
        {
            m_pUsecase->UpdateBufferReadyForRDIQueue(rtPipelineReqId, m_rdiStreamIndex, TRUE);
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

            if (m_YuvCBPipelineIndex == pCbData->sessionId)
            {
                // Release reference to the input buffers of the request to the SnapshotPipeline
                m_pUsecase->ReleaseReferenceToInputBuffers(pResult->pPrivData);
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
/// FeatureYuvCB::ProcessCHIPartialData
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureYuvCB::ProcessCHIPartialData(
    UINT32    frameNum,
    UINT32    sessionId)
{
    CAMX_UNREFERENCED_PARAM(frameNum);
    CAMX_UNREFERENCED_PARAM(sessionId);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureYuvCB::ProcessDriverPartialCaptureResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureYuvCB::ProcessDriverPartialCaptureResult(
    CHIPARTIALCAPTURERESULT*    pResult,
    VOID*                       pPrivateCallbackData)
{
    ChiMetadata*                pChiOutputMetadata = NULL;
    UINT32                      resultFrameNum = pResult->frameworkFrameNum;
    UINT32                      resultFrameIndex = ChxUtils::GetResultFrameIndexChi(resultFrameNum);
    SessionPrivateData*         pCbData =
        static_cast<SessionPrivateData*>(pPrivateCallbackData);
    PartialResultSender         sender =
        PartialResultSender::DriverPartialData;

    if ((TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress))) ||
        (FlushStatus::NotFlushing != m_pUsecase->GetFlushStatus()))
    {
        CHX_LOG_INFO("ZSL process result return because of cleanup");
        return;
    }

    CHX_LOG("FeatureYUVCB Driver Capture result from sessionid:%d", pCbData->sessionId);
    if ((m_YuvCBPipelineIndex == pCbData->sessionId) || (m_realTimeBlobPipelineIndex == pCbData->sessionId))
    {
        resultFrameNum   = m_snapshotReqIdToFrameNum[pResult->frameworkFrameNum % MaxOutstandingRequests];
        resultFrameIndex = resultFrameNum % MaxOutstandingRequests;
        CHX_LOG("FeatureYUVCB Snapshot ReqId to AppFrameNum: %d <--> %d",
            pResult->frameworkFrameNum,
            resultFrameNum);
    }
    else if (m_previewSessionId == pCbData->sessionId)
    {
        resultFrameNum   = m_pUsecase->GetChiFrameNumFromReqId(m_previewSessionId, 0, pResult->frameworkFrameNum);
        resultFrameIndex = resultFrameNum % MaxOutstandingRequests;
        CHX_LOG("FeatureYuvCB ProcessResult Realtime ReqId to AppFrameNum: %d <--> %d",
            pResult->frameworkFrameNum,
            resultFrameNum);
    }

    pChiOutputMetadata = m_pMetadataManager->GetMetadataFromHandle(pResult->pPartialResultMetadata);

    // This check is to ensure that we have not sent earlier and this is the final pipeline's partial
    // result
    if ((m_pUsecase->GetFinalPipelineForPartialMetaData() == pCbData->sessionId) &&
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
/// FeatureYuvCB::ProcessMessage
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureYuvCB::ProcessMessage(
    const CHIMESSAGEDESCRIPTOR* pMessageDescriptor,
    VOID*                       pPrivateCallbackData)
{
    (VOID)pPrivateCallbackData;
    if (ChiMessageTypeMetaBufferDone == pMessageDescriptor->messageType)
    {
        CHX_LOG("FeatureYUVCB MetaBuffer Done frameNum %u i/p metadata %p o/p metadata %p",
                pMessageDescriptor->message.metaBufferDoneMessage.frameworkFrameNum,
                pMessageDescriptor->message.metaBufferDoneMessage.inputMetabuffer,
                pMessageDescriptor->message.metaBufferDoneMessage.outputMetabuffer);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureYuvCB::IsPseudoZSL
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL FeatureYuvCB::IsPseudoZSL(ChiMetadata* pMetadata)
{
    // It is Pseudo ZSL
    // if application hasn't set the ZSL mode OR
    // If the snapshot is captured with Flash or Manual 3A gains

    UINT zslMode = 0;
    m_pUsecase->GetZSLMode(&zslMode, pMetadata);

    CHX_LOG("zslMode: %d m_isFlashRequired:%d.", zslMode, m_isFlashRequired);

    return ((0 == zslMode) || (TRUE == m_isFlashRequired));
}

INT32 FeatureYuvCB::GetRequiredPipelines(
    AdvancedPipelineType* pPipelines,
    INT32 size)
{
    INT32 count = 0;
    const INT32 pipelineCount = 4;

    if (NULL != pPipelines && size >= pipelineCount)
    {
        INT32 index = 0;
        AdvancedPipelineType pipelineGroup[1];
        UINT                 cameraId[1];
        UINT                 physicalCameraID = m_pUsecase->GetPhysicalCameraId(m_physicalCameraIndex);

        if (InvalidPhysicalCameraId != physicalCameraID)
        {
            ChiStream* pStream = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::YuvCBStream);
            if (static_cast<CHISTREAMFORMAT>(HAL_PIXEL_FORMAT_BLOB) != pStream->format)
            {
                pPipelines[index] = AdvancedPipelineType::ZSLSnapshotYUVType;
            }
            else
            {
                CHX_LOG_ERROR("NOT expected in YUVInBlobOut usecase");
            }

            pipelineGroup[0]    = pPipelines[index];
            cameraId[0]         = physicalCameraID;
            m_YuvCBSessionId    = m_pUsecase->GetUniqueSessionId(&pipelineGroup[0], 1, m_physicalCameraIndex);

            m_pUsecase->SetPipelineCameraId(&pipelineGroup[0], &cameraId[0], 1);
            index++;

            if (FALSE == m_pUsecase->IsMultiCameraUsecase())
            {
                pPipelines[index]   = AdvancedPipelineType::ZSLPreviewRawType;
                pipelineGroup[0]    = pPipelines[index];
                cameraId[0]         = physicalCameraID;
                m_previewSessionId  = m_pUsecase->GetUniqueSessionId(&pipelineGroup[0], 1, m_physicalCameraIndex);
                m_pUsecase->SetPipelineCameraId(&pipelineGroup[0], &cameraId[0], 1);
                index++;

                pPipelines[index]       = AdvancedPipelineType::InternalZSLYuv2JpegType;
                pipelineGroup[0]        = pPipelines[index];
                cameraId[0]             = physicalCameraID;
                m_offlineBlobSessionId  = m_pUsecase->GetUniqueSessionId(&pipelineGroup[0], 1, m_physicalCameraIndex);
                m_pUsecase->SetPipelineCameraId(&pipelineGroup[0], &cameraId[0], 1);
                index++;

                pPipelines[index]       = AdvancedPipelineType::ZSLYuv2YuvType;
                pipelineGroup[0]        = pPipelines[index];
                cameraId[0]             = physicalCameraID;
                m_offlineYuvSessionId   = m_pUsecase->GetUniqueSessionId(&pipelineGroup[0], 1, m_physicalCameraIndex);
                m_pUsecase->SetPipelineCameraId(&pipelineGroup[0], &cameraId[0], 1);
                index++;

                pPipelines[index]       = AdvancedPipelineType::ZSLSnapshotJpegType;
                if (TRUE == ExtensionModule::GetInstance()->UseGPURotationUsecase())
                {
                    pPipelines[index] = AdvancedPipelineType::ZSLSnapshotJpegGPUType;
                }
                pipelineGroup[0]        = pPipelines[index];
                cameraId[0]             = physicalCameraID;
                m_offlineYuvSessionId   = m_pUsecase->GetUniqueSessionId(&pipelineGroup[0], 1, m_physicalCameraIndex);
                m_pUsecase->SetPipelineCameraId(&pipelineGroup[0], &cameraId[0], 1);
                index++;
            }
            else
            {
                CHX_LOG("Don't use Realtime pipeline in advance usecase for multicamera usecase");
            }
            count = index;
        }
    }

    CHX_LOG("FeatureYuvCB::required pipeline count:%d", count);
    return count;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureYuvCB::RequestThread
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID* FeatureYuvCB::RequestThread(
    VOID* pThreadData)
{
    PerThreadData* pPerThreadData = reinterpret_cast<PerThreadData*>(pThreadData);

    FeatureYuvCB* pFeatureYuvCB = reinterpret_cast<FeatureYuvCB*>(pPerThreadData->pPrivateData);

    pFeatureYuvCB->RequestThreadProcessing();

    return NULL;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureYuvCB::RequestThreadProcessing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureYuvCB::RequestThreadProcessing()
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

        while (m_maxSnapshotReqId > m_snapshotReqId)
        {
            UINT   requestIdIndex    = (m_snapshotReqId % MaxOutstandingRequests);
            UINT   frameNumber       = m_snapshotReqIdToFrameNum[requestIdIndex];
            UINT32 requestFrameIndex = frameNumber % MaxOutstandingRequests;

            // Check again, If we have the requirements for the snapshot frame for the corresponding frameNumber
            if (TRUE == m_isSnapshotFrame[requestFrameIndex])
            {
                UINT            zslInputFrameNumber = frameNumber;
                CHISTREAMBUFFER snapshotInputBuffer = { 0 };

                UINT32 activePipelineIndex;

                if (TRUE == m_isRealTimeBlobFrame[requestFrameIndex])
                {
                    activePipelineIndex = m_realTimeBlobPipelineIndex;
                }
                else
                {
                    activePipelineIndex = m_YuvCBPipelineIndex;
                }

                // Extract the current request settings to be merged with metadata of zsl queue
                ChiMetadata*  pChiRTInputMetadata = m_pSnapshotInputMeta[requestIdIndex];

                // If ZSLmode is enabled, give the recently available SOF frame as input
                if (FALSE == IsPseudoZSL(pChiRTInputMetadata))
                {
                    UINT32 lastReadyRDIFrameNumber = m_pUsecase->GetLastReadyFrameNum(m_rdiStreamIndex);
                    zslInputFrameNumber = (INVALIDSEQUENCEID == lastReadyRDIFrameNumber) ? 0 : lastReadyRDIFrameNumber;
                    CHX_LOG_INFO("zslInputFrameNumber set in Non PsuedoZsl case = %u", zslInputFrameNumber);
                }

                if (TRUE == m_pUsecase->HasRDIBuffer(zslInputFrameNumber, m_rdiStreamIndex))
                {
                    // Check whether realtime metadata and Rdi buffer for zslInputFrameNumber are available.
                    CHX_LOG_INFO("waiting for rdi and meta of frame %d for triggering snapshot request", zslInputFrameNumber);

                    result = m_pUsecase->WaitForBufferMetaReady(zslInputFrameNumber, m_rdiStreamIndex);

                    if (CDKResultSuccess != result)
                    {
                        CHX_LOG_ERROR("FeatureZSL: wait rdi and meta timeout! frameNumber=%d, zslInputFrameNumber=%d", frameNumber, zslInputFrameNumber);
                    }
                }
                else
                {
                    CHX_LOG_ERROR("frameNumber:%d zslInputFrameNumber:%d doesn't have RDI!", frameNumber, zslInputFrameNumber);
                    result = CDKResultEFailed;
                }

                if (TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress)))
                {
                    CHX_LOG_INFO("Destroy is in progress come out of loop");
                    break;
                }
                if (CDKResultSuccess == result)
                {
                    PipelineData* pPipelineData  = const_cast<PipelineData*>(
                        m_pUsecase->GetPipelineData(activePipelineIndex, 0));

                    ChiMetadata* pChiSnapshotInputMeta = NULL;
                    m_pUsecase->GetTargetBuffer(zslInputFrameNumber,
                                                m_pRdiTargetBuffer,
                                                pChiRTInputMetadata,
                                                &snapshotInputBuffer,
                                                &pChiSnapshotInputMeta);

                    ChiMetadata* pChiOutputMetadata = m_pMetadataManager->Get(
                        pPipelineData->pPipeline->GetMetadataClientId(), m_snapshotReqId);

                    if ((pChiOutputMetadata == NULL) || (pChiSnapshotInputMeta == NULL))
                    {
                        CHX_LOG_ERROR("Failed to get snaphot metadata buffer, cannot submit request");
                        return;
                    }

                    pPipelineData->seqIdToFrameNum[requestIdIndex] = frameNumber;

                    pChiSnapshotInputMeta->AddReference();
                    pChiRTInputMetadata->ReleaseReference();

                    const Session* pSession = m_pUsecase->GetSessionData(activePipelineIndex)->pSession;

                    CHICAPTUREREQUEST snapshotRequest = { 0 };
                    snapshotRequest.frameNumber       = m_snapshotReqId++;
                    snapshotRequest.hPipelineHandle   = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
                    snapshotRequest.numInputs         = 1;
                    snapshotRequest.numOutputs        = m_snapshotBufferNum[requestIdIndex];
                    snapshotRequest.pInputBuffers     = &snapshotInputBuffer;
                    snapshotRequest.pOutputBuffers    = m_snapshotBuffers[requestIdIndex];
                    snapshotRequest.pInputMetadata    = pChiSnapshotInputMeta->GetHandle();
                    snapshotRequest.pOutputMetadata   = pChiOutputMetadata->GetHandle();
                    snapshotRequest.pPrivData         = &m_privData[snapshotRequest.frameNumber % MaxOutstandingRequests];
                    snapshotRequest.pPrivData->featureType = FeatureType::ZSL;

                    // Save input buffers info for later releasing reference
                    snapshotRequest.pPrivData->inputBuffers[0]       = snapshotInputBuffer.bufferInfo;
                    snapshotRequest.pPrivData->bufferManagers[0]     = m_pRdiTargetBuffer->pBufferManager;
                    snapshotRequest.pPrivData->numInputBuffers       = 1;

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

                    CHX_LOG_INFO("Sending ZSL snapshot frameNumber:%d, request:%" PRIu64 " metadata:%p",
                                 frameNumber, snapshotRequest.frameNumber, pChiSnapshotInputMeta);
                    m_pUsecase->SetRequestToFeatureMapping(activePipelineIndex, snapshotRequest.frameNumber, this);

                    m_pUsecase->LogFeatureRequestMappings(frameNumber, snapshotRequest.frameNumber, "YuvCB ZSL request");
                    if ((TRUE == ChxUtils::HasInputBufferError(&submitRequest)) ||
                       (FlushStatus::NotFlushing != m_pUsecase->GetFlushStatus()))
                    {
                        InvalidateRequest(&submitRequest);
                    }
                    else
                    {
                        m_pUsecase->SubmitRequest(&submitRequest);
                    }
                }
            }
        }
    }
    CHX_LOG("RequestThreadProcessing Exited");
}
