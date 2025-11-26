////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
// All Rights Reserved
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxfeaturemultiframe.cpp
/// @brief CHX Multiframe feature class implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "chxadvancedcamerausecase.h"
#include "chxincs.h"
#include "chxfeaturemultiframe.h"
#include "chxusecase.h"

// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMultiframe::Create
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FeatureMultiframe* FeatureMultiframe::Create(
    AdvancedCameraUsecase* pUsecase,
    UINT32                 physicalCameraIndex)
{
    FeatureMultiframe* pFeature = CHX_NEW FeatureMultiframe;

    if (NULL != pFeature)
    {
        if (CDKResultSuccess != pFeature->Initialize(pUsecase))
        {
            pFeature->Destroy(FALSE);
            pFeature = NULL;
        }
        else
        {
            pFeature->SetPhysicalCameraIndex(physicalCameraIndex);
        }
    }

    return pFeature;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMultiframe::Pause
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMultiframe::Pause(
    BOOL isForced)
{
    CHX_LOG_INFO("FeatureMultiframe::Pause(), isForced %d E.", isForced);
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
    CHX_LOG_INFO("FeatureMultiframe::Pause(), isForced %d X.", isForced);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMultiframe::Initialize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMultiframe::Initialize(
    AdvancedCameraUsecase* pUsecase)
{
    CDKResult    result = CDKResultSuccess;

    Feature::InitializePrivateResources();

    m_pUsecase          = pUsecase;

    m_pMergeYuvStream   = static_cast<CHISTREAM*>(CHX_CALLOC(sizeof(CHISTREAM)));
    m_pMetadataManager  = pUsecase->GetMetadataManager();

    for (UINT32 i = 0; i < MaxInputPorts; i++)
    {
        m_pMergePipelineInputStreams[i] = static_cast<CHISTREAM*>(CHX_CALLOC(sizeof(CHISTREAM)));
    }

    m_lastShutterFrameNum    = 0;

    if (StreamConfigModeFastShutter == ExtensionModule::GetInstance()->GetOpMode(m_pUsecase->GetCameraId()))
    {
        m_numMultiFramesRequired = m_SWMFNumFramesforFS;
    }
    else
    {
        m_numMultiFramesRequired = DefaultSWMFNumFrames;
    }

    m_snapshotSessionId      = InvalidSessionId;
    m_mergeSessionId         = InvalidSessionId;
    m_bayer2YuvSessionId     = InvalidSessionId;
    m_previewSessionId       = InvalidSessionId;

    m_pBayer2YuvOutputBufferManager = NULL;
    m_pMergeOutputBufferManager     = NULL;

    CHX_LOG("FeatureSWMF m_pRdiStream: %p, m_pBayer2YuvStream: %p, m_pMergeYuvStream: %p",
            m_pRdiStream, m_pBayer2YuvStream, m_pMergeYuvStream);

    ExtensionModule::GetInstance()->GetVendorTagOps(&m_vendorTagOps);

    m_pOfflineRequestMutex                     = Mutex::Create();
    m_pOfflineRequestAvailable                 = Condition::Create();
    m_pLastPreviewRequestMutex                 = Mutex::Create();
    m_pLastPreviewResultAvailable              = Condition::Create();

    m_lastPreviewFrameNum                      = 0;
    m_offlineRequestProcessTerminate           = FALSE;
    m_aPauseInProgress                         = FALSE;
    m_offlineThreadCaptureRequestId            = 0;
    m_offlineRequestProcessThread.pPrivateData = this;

    result = ChxUtils::ThreadCreate(FeatureMultiframe::RequestThread,
        &m_offlineRequestProcessThread,
        &m_offlineRequestProcessThread.hThreadHandle);

        return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMultiframe::Destroy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMultiframe::Destroy(BOOL isForced)
{

    CHX_LOG("FeatureSWMF::Destroy(), isForced %d E.", isForced);

    m_pOfflineRequestMutex->Lock();
    m_offlineRequestProcessTerminate = TRUE;
    m_pOfflineRequestAvailable->Signal();
    m_pOfflineRequestMutex->Unlock();
    ChxUtils::ThreadTerminate(m_offlineRequestProcessThread.hThreadHandle);

    /// only reset shared stream to NULL
    /// no need to free, as advance will handle it.
    m_pSnapshotStream  = NULL;
    m_pPreviewStream   = NULL;
    m_pRdiStream       = NULL;
    m_pBayer2YuvStream = NULL;
    m_pJPEGInputStream = NULL;
    m_pFdStream        = NULL;

    if (NULL != m_pMergeYuvStream)
    {
        CHX_FREE(m_pMergeYuvStream);
        m_pMergeYuvStream = NULL;
    }

    for (UINT32 i = 0; i < MaxInputPorts; i++)
    {
        CHX_FREE(m_pMergePipelineInputStreams[i]);
        m_pMergePipelineInputStreams[i] = NULL;
    }

    m_pChiUsecase = NULL;

    if (NULL != m_pMergeOutputBufferManager)
    {
        m_pMergeOutputBufferManager->Destroy();
        m_pMergeOutputBufferManager = NULL;
    }

    if (NULL != m_pBayer2YuvOutputBufferManager)
    {
        m_pBayer2YuvOutputBufferManager->Destroy();
        m_pBayer2YuvOutputBufferManager = NULL;
    }

    for (UINT queueIndex = 0; queueIndex < BufferQueueDepth; queueIndex++)
    {
        if (NULL != m_mergeTargetBuffers.bufferQueue[queueIndex].pRdiOutputBuffer)
        {
            CHX_FREE(m_mergeTargetBuffers.bufferQueue[queueIndex].pRdiOutputBuffer);
            m_mergeTargetBuffers.bufferQueue[queueIndex].pRdiOutputBuffer = NULL;
        }
        if (NULL != m_mergeTargetBuffers.bufferQueue[queueIndex].pMetadata)
        {
            m_mergeTargetBuffers.bufferQueue[queueIndex].pMetadata = NULL;
        }

        if (NULL != m_bayer2YuvTargetBuffers.bufferQueue[queueIndex].pRdiOutputBuffer)
        {
            CHX_FREE(m_bayer2YuvTargetBuffers.bufferQueue[queueIndex].pRdiOutputBuffer);
            m_bayer2YuvTargetBuffers.bufferQueue[queueIndex].pRdiOutputBuffer = NULL;
        }
        if (NULL != m_bayer2YuvTargetBuffers.bufferQueue[queueIndex].pMetadata)
        {
            m_bayer2YuvTargetBuffers.bufferQueue[queueIndex].pMetadata = NULL;
        }
    }

    if (NULL != m_debugDataOffline.pData)
    {
        CHX_FREE(m_debugDataOffline.pData);
        m_debugDataOffline.pData    = NULL;
        m_debugDataOffline.size     = 0;
    }

    if (NULL != m_mergeTargetBuffers.pMutex)
    {
        m_mergeTargetBuffers.pMutex->Destroy();
        m_mergeTargetBuffers.pMutex = NULL;
    }

    if (NULL != m_bayer2YuvTargetBuffers.pMutex)
    {
        m_bayer2YuvTargetBuffers.pMutex->Destroy();
        m_bayer2YuvTargetBuffers.pMutex = NULL;
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

    if (NULL != m_pLastPreviewRequestMutex)
    {
        m_pLastPreviewRequestMutex->Destroy();
        m_pLastPreviewRequestMutex = NULL;
    }

    if (NULL != m_pLastPreviewResultAvailable)
    {
        m_pLastPreviewResultAvailable->Destroy();
        m_pLastPreviewResultAvailable = NULL;
    }

    for (UINT32 i = 0; i < MaxMultiFrameRequests; i++)
    {
        if (NULL != m_offlineRequestData[i].pRequestMetadata)
        {
            m_offlineRequestData[i].pRequestMetadata = NULL;
        }
    }

    Feature::DestroyPrivateResources();

    CHX_DELETE(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMultiframe::isJPEGOutputRequired
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL FeatureMultiframe::isJPEGOutputRequired() const
{
    return m_pUsecase->IsJPEGSnapshotStream();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMultiframe::ConfigureTargetStream
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMultiframe::ConfigureTargetStream()
{
    CDKResult result = CDKResultSuccess;

    // if JPEG is not needed for this feature, mergeYUVstream is the sink target stream, we need configure it with
    // priority for buffer negotiation
    if (FALSE == isJPEGOutputRequired())
    {
        m_pMergeYuvStream->format           = ChiStreamFormatYCbCr420_888;
        // Create the Merge stream output based on the input buffer requirements to generate the snapshot stream buffer
        m_pMergeYuvStream->width            = m_pSnapshotStream->width;
        m_pMergeYuvStream->height           = m_pSnapshotStream->height;
        m_pMergeYuvStream->maxNumBuffers    = 0;
        m_pMergeYuvStream->rotation         = StreamRotationCCW90;
        m_pMergeYuvStream->streamType       = ChiStreamTypeOutput;
        m_pMergeYuvStream->grallocUsage     = 0;

        /// Create buffer managers
        CHIBufferManagerCreateData createData = { 0 };

        createData.width                = m_pMergeYuvStream->width;
        createData.height               = m_pMergeYuvStream->height;
        createData.format               = m_pMergeYuvStream->format;
        createData.producerFlags        = ChiGralloc1ProducerUsageCamera;
        createData.consumerFlags        = ChiGralloc1ConsumerUsageCamera | ChiGralloc1ConsumerUsageCpuRead;
        createData.maxBufferCount       = MinOutputBuffers;
        createData.immediateBufferCount = 0;
        createData.bEnableLateBinding   = ExtensionModule::GetInstance()->EnableCHILateBinding();
        createData.bufferHeap           = BufferHeapDefault;
        createData.pChiStream           = m_pMergeYuvStream;

        if (NULL == m_pMergeOutputBufferManager)
        {
            m_pMergeOutputBufferManager = CHIBufferManager::Create("HDRMergeBufferManager", &createData);
            if (NULL == m_pMergeOutputBufferManager)
            {
                CHX_LOG_ERROR("m_pMergeOutputBufferManager create failed!");
                result = CDKResultEFailed;
            }
            else
            {
                m_mergeTargetBuffers.pBufferManager = m_pMergeOutputBufferManager;
                m_mergeTargetBuffers.pMutex         = Mutex::Create();

                for (UINT queueIndex = 0; queueIndex < BufferQueueDepth; queueIndex++)
                {
                    CHISTREAMBUFFER* pRdiOutputBuffer = static_cast<CHISTREAMBUFFER*>(CHX_CALLOC(sizeof(CHISTREAMBUFFER)));

                    m_mergeTargetBuffers.bufferQueue[queueIndex].pRdiOutputBuffer = pRdiOutputBuffer;

                    if (NULL != pRdiOutputBuffer)
                    {
                        pRdiOutputBuffer->size                = sizeof(CHISTREAMBUFFER);
                        pRdiOutputBuffer->acquireFence.valid  = FALSE;
                        pRdiOutputBuffer->bufferInfo.phBuffer = NULL;
                        pRdiOutputBuffer->pStream             = m_pMergeYuvStream;
                    }
                }
                m_pMergeTargetBuffer = &m_mergeTargetBuffers;
                CHX_LOG_INFO("m_pMergeOutputBufferManager=%p m_pMergeTargetBuffer=%p",
                             m_pMergeOutputBufferManager, m_pMergeTargetBuffer);
            }
        }
        else
        {
            CHX_LOG_INFO("m_pMergeOutputBufferManager=%p m_pMergeTargetBuffer=%p already created!",
                         m_pMergeOutputBufferManager, m_pMergeTargetBuffer);
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMultiframe::OverrideUsecase
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ChiUsecase* FeatureMultiframe::OverrideUsecase(
    LogicalCameraInfo*              pCameraInfo,
    camera3_stream_configuration_t* pStreamConfig)
{
    (VOID)pCameraInfo;
    CHX_LOG("FeatureSWMF OverrideUsecase for Multiframe, instanceID:%d", m_physicalCameraIndex);
    CHX_ASSERT(2 == pStreamConfig->num_streams);

    m_pChiUsecase = m_pUsecase->GetChiUseCase();

    m_pPreviewStream        = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::PreviewStream);
    m_pRdiStream            = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::RdiStream, m_physicalCameraIndex);
    m_pFdStream             = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::FdStream, m_physicalCameraIndex);
    m_pSnapshotStream       = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::SnapshotStream);
    m_pBayer2YuvStream      = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::Bayer2YuvStream,
                                m_physicalCameraIndex);
    m_pJPEGInputStream      = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::JPEGInputStream,
                                m_physicalCameraIndex);

    m_snapshotPipelineIndex  = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                    AdvancedPipelineType::InternalZSLYuv2JpegType, m_physicalCameraIndex);
    m_mergePipelineIndex     = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                    AdvancedPipelineType::SWMFMergeYuvType, m_physicalCameraIndex);
    m_bayer2YuvPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                    AdvancedPipelineType::ZSLSnapshotYUVType, m_physicalCameraIndex);

    if (StreamConfigModeFastShutter == ExtensionModule::GetInstance()->GetOpMode(m_pUsecase->GetCameraId()))
    {
        m_previewPipelineIndex   = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                    AdvancedPipelineType::ZSLPreviewRawFSType, m_physicalCameraIndex);
    }
    else
    {
        m_previewPipelineIndex   = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                    AdvancedPipelineType::ZSLPreviewRawType, m_physicalCameraIndex);
    }

    CHX_LOG("m_previewPipelineIndex:%d, m_bayer2YuvPipelineIndex:%d, m_mergePipelineIndex:%d, m_snapshotPipelineIndex:%d,",
        m_previewPipelineIndex,
        m_bayer2YuvPipelineIndex,
        m_mergePipelineIndex,
        m_snapshotPipelineIndex);

    ChiPipelineTargetCreateDescriptor* pBayer2YuvDesc   = &m_pChiUsecase->pPipelineTargetCreateDesc[m_bayer2YuvPipelineIndex];
    ChiPipelineTargetCreateDescriptor* pMergeDesc       = &m_pChiUsecase->pPipelineTargetCreateDesc[m_mergePipelineIndex];
    ChiPipelineTargetCreateDescriptor* pSnapshotDesc    = NULL;

    if (TRUE == isJPEGOutputRequired())
    {
        pSnapshotDesc = &m_pChiUsecase->pPipelineTargetCreateDesc[m_snapshotPipelineIndex];
    }

    // Multiframe Stage-1 (BayerToYuv)
    pBayer2YuvDesc->sourceTarget.pTargetPortDesc[0].pTarget->pChiStream = m_pRdiStream;
    pBayer2YuvDesc->sinkTarget.pTargetPortDesc[0].pTarget->pChiStream   = m_pBayer2YuvStream;

    // HDR Stage-2 (Merge)
    CHX_LOG("FeatureSWMF Merge Pipeline source target:%d", pMergeDesc->sourceTarget.numTargets);
    ChxUtils::Memset(m_mergePipelineInputTargets,  0, sizeof(m_mergePipelineInputTargets));
    ChxUtils::Memset(&m_mergePipelineOutputTarget, 0, sizeof(m_mergePipelineOutputTarget));

    for (UINT32 i = 0; i < pMergeDesc->sourceTarget.numTargets; i++)
    {
        pMergeDesc->sourceTarget.pTargetPortDesc[i].pTarget->pChiStream = m_pMergePipelineInputStreams[i];

        CHX_LOG("[%d], Node:%d, port:%d to input stream:%p",
            i,
            pMergeDesc->sourceTarget.pTargetPortDesc[i].pNodePort[0].nodeId,
            pMergeDesc->sourceTarget.pTargetPortDesc[i].pNodePort[0].nodePortId,
            m_pMergePipelineInputStreams[i]);
    }

    pMergeDesc->sinkTarget.pTargetPortDesc[0].pTarget->pChiStream       = m_pMergeYuvStream;

    // HDR Stage-3 (Jpeg)
    if (NULL != pSnapshotDesc)
    {
        pSnapshotDesc->sourceTarget.pTargetPortDesc[0].pTarget->pChiStream  = m_pJPEGInputStream;
        pSnapshotDesc->sinkTarget.pTargetPortDesc[0].pTarget->pChiStream    = m_pSnapshotStream;
    }
    CHX_LOG("m_rdiStreamIndex:%d, m_Bayer2YuvStreamIndex:%d, m_mergeStreamIndex:%d",
        m_rdiStreamIndex, m_Bayer2YuvStreamIndex, m_mergeStreamIndex);

    CHX_LOG("FeatureSWMF RdiStream: %p, PreviewStream: %p, SnapshotStream: %p, Bayer2YuvStream: %p, JPEGInputStream:%p",
            m_pRdiStream, m_pPreviewStream, m_pSnapshotStream, m_pBayer2YuvStream, m_pJPEGInputStream);

    ConfigureTargetStream();

    return m_pChiUsecase;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMultiframe::PipelineCreated
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMultiframe::PipelineCreated(
    UINT32 sessionId,       ///< Id of session created
    UINT32 pipelineIndex)   ///< Index of the pipeline created (within the context of the session)
{
    ChiSensorModeInfo*              pSensorInfo     = NULL;
    const ChiPipelineInputOptions*  pInputOptions   = NULL;
    UINT                            pipelineId =
        m_pUsecase->GetSessionData(sessionId)->pipelines[pipelineIndex].id;

    pSensorInfo   = m_pUsecase->GetSessionData(sessionId)->pipelines[0].pPipeline->GetSensorModeInfo();
    pInputOptions = m_pUsecase->GetSessionData(sessionId)->pipelines[0].pPipeline->GetInputOptions();

    CHX_LOG("FeatureSWMF sessionId:%d, pipelineId: %d, Sensor widthxheight: %d x %d, InputOptions widthxheight: %d x %d",
                  sessionId,
                  pipelineId,
                  pSensorInfo->frameDimension.width, pSensorInfo->frameDimension.height,
                  pInputOptions->bufferOptions.optimalDimension.width,
                  pInputOptions->bufferOptions.optimalDimension.height);

    switch (m_pUsecase->GetAdvancedPipelineTypeByPipelineId(pipelineId))
    {
        case AdvancedPipelineType::InternalZSLYuv2JpegType:
        {
            m_pMergeYuvStream->format           = ChiStreamFormatYCbCr420_888;
            // Create the Merge stream output based on the input buffer requirements to generate the snapshot stream buffer
            m_pMergeYuvStream->width            = pInputOptions->bufferOptions.optimalDimension.width;
            m_pMergeYuvStream->height           = pInputOptions->bufferOptions.optimalDimension.height;
            m_pMergeYuvStream->maxNumBuffers    = 0;
            m_pMergeYuvStream->rotation         = StreamRotationCCW90;
            m_pMergeYuvStream->streamType       = ChiStreamTypeOutput;
            m_pMergeYuvStream->grallocUsage     = 0;

            m_pJPEGInputStream->format           = ChiStreamFormatYCbCr420_888;
            // Create the Merge stream output based on the input buffer requirements to generate the snapshot stream buffer
            m_pJPEGInputStream->width            = pInputOptions->bufferOptions.optimalDimension.width;
            m_pJPEGInputStream->height           = pInputOptions->bufferOptions.optimalDimension.height;
            m_pJPEGInputStream->maxNumBuffers    = 0;
            m_pJPEGInputStream->rotation         = StreamRotationCCW90;
            m_pJPEGInputStream->streamType       = ChiStreamTypeInput;
            m_pJPEGInputStream->grallocUsage     = 0;

            CHX_LOG("MergeYuvStream/JPEGInputStream: %dx%d", m_pMergeYuvStream->width, m_pMergeYuvStream->height);

            /// Create buffer managers
            CHIBufferManagerCreateData createData = { 0 };

            createData.width                = m_pMergeYuvStream->width;
            createData.height               = m_pMergeYuvStream->height;
            createData.format               = ChiStreamFormatYCbCr420_888;
            createData.producerFlags        = ChiGralloc1ProducerUsageCamera;
            createData.consumerFlags        = ChiGralloc1ConsumerUsageCamera | ChiGralloc1ConsumerUsageCpuRead;
            createData.maxBufferCount       = MinOutputBuffers;
            createData.immediateBufferCount = 1;
            createData.bEnableLateBinding   = ExtensionModule::GetInstance()->EnableCHILateBinding();
            createData.bufferHeap           = BufferHeapDefault;
            createData.pChiStream           = m_pMergeYuvStream;

            if (NULL == m_pMergeOutputBufferManager)
            {
                m_pMergeOutputBufferManager = CHIBufferManager::Create("SWMFBufferManagaer", &createData);
                if (NULL == m_pMergeOutputBufferManager)
                {
                    CHX_LOG_ERROR("m_pMergeOutputBufferManager is NULL");
                }
                else
                {
                    m_mergeTargetBuffers.pBufferManager = m_pMergeOutputBufferManager;
                    m_mergeTargetBuffers.pMutex         = Mutex::Create();

                    for (UINT queueIndex = 0; queueIndex < BufferQueueDepth; queueIndex++)
                    {
                        CHISTREAMBUFFER* pRdiOutputBuffer = static_cast<CHISTREAMBUFFER*>(CHX_CALLOC(sizeof(CHISTREAMBUFFER)));

                        m_mergeTargetBuffers.bufferQueue[queueIndex].pRdiOutputBuffer   = pRdiOutputBuffer;
                        m_mergeTargetBuffers.bufferQueue[queueIndex].pMetadata          = NULL;

                        if (NULL != pRdiOutputBuffer)
                        {
                            pRdiOutputBuffer->size                = sizeof(CHISTREAMBUFFER);
                            pRdiOutputBuffer->acquireFence.valid  = FALSE;
                            pRdiOutputBuffer->bufferInfo.phBuffer = NULL;
                            pRdiOutputBuffer->pStream             = m_pMergeYuvStream;
                        }
                    }
                    m_pMergeTargetBuffer = &m_mergeTargetBuffers;
                    CHX_LOG_INFO("m_pMergeOutputBufferManager=%p m_pMergeTargetBuffer=%p",
                                 m_pMergeOutputBufferManager, m_pMergeTargetBuffer);
                }
            }
            else
            {
                CHX_LOG_INFO("m_pMergeOutputBufferManager=%p m_pMergeTargetBuffer=%p already created!",
                             m_pMergeOutputBufferManager, m_pMergeTargetBuffer);
            }
            break;
        }
        case AdvancedPipelineType::SWMFMergeYuvType:
        {
            m_pBayer2YuvStream->format          = ChiStreamFormatYCbCr420_888;
            // Create the Bayer2Yuv stream output based on the input buffer requirements to generate the Merge stream buffer
            m_pBayer2YuvStream->width           = pInputOptions->bufferOptions.optimalDimension.width;
            m_pBayer2YuvStream->height          = pInputOptions->bufferOptions.optimalDimension.height;
            m_pBayer2YuvStream->maxNumBuffers   = 0;
            m_pBayer2YuvStream->rotation        = StreamRotationCCW90;
            m_pBayer2YuvStream->streamType      = ChiStreamTypeOutput;
            m_pBayer2YuvStream->grallocUsage    = 0;

            CHIBufferManagerCreateData createBayertoYuvData = { 0 };

            createBayertoYuvData.width                = m_pBayer2YuvStream->width;
            createBayertoYuvData.height               = m_pBayer2YuvStream->height;
            createBayertoYuvData.format               = ChiStreamFormatYCbCr420_888;
            createBayertoYuvData.producerFlags        = ChiGralloc1ProducerUsageCamera;
            createBayertoYuvData.consumerFlags        = ChiGralloc1ConsumerUsageCamera | ChiGralloc1ConsumerUsageCpuRead;
            createBayertoYuvData.maxBufferCount       = MinOutputBuffers;
            createBayertoYuvData.immediateBufferCount = CHIImmediateBufferCountZSL;
            createBayertoYuvData.bEnableLateBinding   = ExtensionModule::GetInstance()->EnableCHILateBinding();
            createBayertoYuvData.bufferHeap           = BufferHeapDefault;
            createBayertoYuvData.pChiStream           = m_pBayer2YuvStream;

            if (NULL == m_pBayer2YuvOutputBufferManager)
            {
                m_pBayer2YuvOutputBufferManager = CHIBufferManager::Create("SWMFBayertoYuvBufferManager", &createBayertoYuvData);
                if (NULL == m_pBayer2YuvOutputBufferManager)
                {
                    CHX_LOG_ERROR("m_pBayer2YuvOutputBufferManager is NULL");
                }
                else
                {
                    m_bayer2YuvTargetBuffers.pBufferManager = m_pBayer2YuvOutputBufferManager;
                    m_bayer2YuvTargetBuffers.pMutex = Mutex::Create();

                    for (UINT queueIndex = 0; queueIndex < BufferQueueDepth; queueIndex++)
                    {
                        CHISTREAMBUFFER* pRdiOutputBuffer = static_cast<CHISTREAMBUFFER*>(CHX_CALLOC(sizeof(CHISTREAMBUFFER)));

                        m_bayer2YuvTargetBuffers.bufferQueue[queueIndex].pRdiOutputBuffer = pRdiOutputBuffer;
                        m_bayer2YuvTargetBuffers.bufferQueue[queueIndex].pMetadata        = NULL;

                        if (NULL != pRdiOutputBuffer)
                        {
                            pRdiOutputBuffer->size                = sizeof(CHISTREAMBUFFER);
                            pRdiOutputBuffer->acquireFence.valid  = FALSE;
                            pRdiOutputBuffer->bufferInfo.phBuffer = NULL;
                            pRdiOutputBuffer->pStream             = m_pBayer2YuvStream;
                        }
                    }
                    m_pBayer2YuvTargetBuffer = &m_bayer2YuvTargetBuffers;
                    CHX_LOG_INFO("m_pBayer2YuvOutputBufferManager=%p m_pBayer2YuvTargetBuffer=%p",
                                 m_pBayer2YuvOutputBufferManager, m_pBayer2YuvTargetBuffer);
                }
            }
            else
            {
                CHX_LOG_INFO("m_pBayer2YuvOutputBufferManager=%p m_pBayer2YuvTargetBuffer=%p already created!",
                             m_pBayer2YuvOutputBufferManager, m_pBayer2YuvTargetBuffer);
            }

            for (UINT32 i = 0; i < MaxInputPorts; i++)
            {
                ChxUtils::Memcpy(m_pMergePipelineInputStreams[i], m_pBayer2YuvStream, sizeof(ChiStream));
                m_pMergePipelineInputStreams[i]->streamType = ChiStreamTypeInput;
            }

            CHX_LOG("m_pBayer2YuvStream: %dx%d", m_pMergeYuvStream->width, m_pMergeYuvStream->height);

            SetFeatureStatus(FeatureStatus::READY);
            break;
        }
        default:
        {
            CHX_LOG("FeatureSWMF pipeline not used in SWMF.");
            break;
        }
    }

    m_rdiStreamIndex = m_pUsecase->GetInternalTargetBufferIndex(m_pRdiStream);
    m_fdStreamIndex  = m_pUsecase->GetInternalTargetBufferIndex(m_pFdStream);
    m_pRdiTargetBuffer = m_pUsecase->GetTargetBufferPointer(m_rdiStreamIndex);
    m_pFdTargetBuffer = m_pUsecase->GetTargetBufferPointer(m_fdStreamIndex);

    CHX_LOG_CONFIG("rdiStreamIndex:%d,pRDItargetBuffer:%p,fdStreawmIndex;%d",
        m_rdiStreamIndex, m_pRdiTargetBuffer, m_fdStreamIndex);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMultiframe::ExecuteProcessRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult  FeatureMultiframe::ExecuteProcessRequest(
    camera3_capture_request_t* pRequest)              ///< Request parameters
{
    CDKResult                 result                          = CDKResultSuccess;
    CHISTREAMBUFFER           previewBuffers[3]               = { {0} };
    UINT                      previewCount                    = 0;
    UINT                      snapshotCount                   = 0;
    const UINT                frameNumber                     = pRequest->frame_number;
    UINT32                    frameIndex                      = (pRequest->frame_number % MaxOutstandingRequests);
    UINT                      snapshotReqIdIndex              = (m_maxSnapshotReqId % MaxOutstandingRequests);
    camera3_capture_result_t* pUsecaseResult                  = m_pUsecase->GetCaptureResult(frameIndex);
    BOOL                      hasSnapshotStream               = FALSE;
    ChiMetadata*              pInputMeta                      = NULL;
    BOOL                      hasPreviewStream                = FALSE;

    if (TRUE == m_pUsecase->IsMultiCameraUsecase())
    {
        /// Assume it always take snapshot in dualcamera usecase.
        UINT32         rtReqId     = pRequest->frame_number;
        RequestMapInfo info        = m_pUsecase->GetRequestMapInfo(rtReqId);
        UINT32         appFrameNum = info.frameNumber;
        UINT32         appFrameIdx = (appFrameNum % MaxOutstandingRequests);
        m_internalFrameNum         = pRequest->frame_number;
        m_masterCameraId           = info.masterCameraID;
        m_activePipelineID         = info.activePipelineID;

        m_isSnapshotFrame[appFrameIdx]         = TRUE;
        m_snapshotBufferNum[appFrameIdx]       = 1;
        m_snapshotReqIdToFrameNum[appFrameIdx] = appFrameNum;
        m_maxSnapshotReqId++;
        m_numBayer2YuvFrames = 0;

        for (UINT32 i = 0; i < pRequest->num_output_buffers; i++)
        {
            if (m_pSnapshotStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream))
            {
                /// in dual camera usecase, setting here is NULL, no need to copy the meta.
                /// only copy the snapshot buffer
                ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i], &m_snapshotBuffers[appFrameIdx][0]);
                hasSnapshotStream  = TRUE;
            }
        }

        // here need customer to do customization by their anchor pick algorithm
        // if it needs to take long time, it is better to do in offline thread to avoid occupy long time
        // in multi camera offline thread.
        UINT32 anchorFrame = m_numMultiFramesRequired -1;
        m_pUsecase->ProcessFeatureDataNotify(pRequest->frame_number, this, static_cast<VOID*>(&anchorFrame));

        for (UINT32 i = 0; i < m_numMultiFramesRequired; i++)
        {
            UINT32 reqId = rtReqId - i;
            info         = m_pUsecase->GetRequestMapInfo(reqId);

            CHX_LOG("FeatureSWMF ExecuteProcessRequest for dual camera!, rtReqId:%d, AppFrameNum:%d" \
                    "masterCameraId:%d, activePipelineID:%d, instance:%d",
                    reqId, appFrameNum, m_masterCameraId, m_activePipelineID, m_physicalCameraIndex);

            UINT32 sessionId = m_bayer2YuvSessionId;

            const PipelineData* pPipelineData = m_pUsecase->GetPipelineData(sessionId, 0);
            UINT32              queueIndex    = (pPipelineData->seqId) % BufferQueueDepth;

            CHX_LOG("AppFrameNum:%d <--> bayer2YuvReqId:%d, input RDI request id:%d, sessionId:%d, queueIndex:%d",
                appFrameNum, pPipelineData->seqId, reqId, sessionId, queueIndex);

            CHISTREAMBUFFER outputBuffer = {0};

            outputBuffer.bufferInfo         = m_pBayer2YuvTargetBuffer->pBufferManager->GetImageBufferInfo();
            outputBuffer.pStream            = m_pBayer2YuvStream;
            outputBuffer.acquireFence.valid = FALSE;
            outputBuffer.size               = sizeof(CHISTREAMBUFFER);

            if (CDKResultSuccess == result)
            {
                SubmitRequest(sessionId, appFrameNum, m_pRdiTargetBuffer, &outputBuffer, reqId, i, TRUE,
                              "SWMF MutliCam Bayer2YUV Request");
            }
            else
            {
                CHX_LOG_ERROR("FeatureSWMF request is not submitted to Bayer2YuvPipeline! appFrameNum = %d, sessionId = %d",
                              appFrameNum, sessionId);
                break;
            }
        }
        return result;
    }

    CHX_LOG("FeatureSWMF::ExecuteProcessRequest, frameNum:%d, setting:%p", pRequest->frame_number, pRequest->settings);

    pInputMeta = m_pMetadataManager->GetInput(pRequest->settings, frameNumber);

    if (NULL == pInputMeta)
    {
        CHX_LOG_ERROR("InputMeta not available %p", pInputMeta);
        return CDKResultEFailed;
    }

    pUsecaseResult->result             = NULL;
    pUsecaseResult->frame_number       = pRequest->frame_number;
    pUsecaseResult->num_output_buffers = 0;

    m_isSnapshotFrame[frameIndex]      = FALSE;
    m_shutterTimestamp[frameIndex]     = 0;
    for (UINT32 i = 0; i < pRequest->num_output_buffers; i++)
    {
        if (m_pPreviewStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream))
        {
            hasPreviewStream = TRUE;
            // Capture preview stream
            ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i], &previewBuffers[previewCount]);
            previewCount++;
        }

        if (m_pSnapshotStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream))
        {
            if (FALSE == m_pUsecase->m_rejectedSnapshotRequestList[pRequest->frame_number % MaxOutstandingRequests])
            {
                pInputMeta->AddReference();
                m_pSnapshotInputMeta[frameIndex] = pInputMeta;
                m_pBayer2YuvInputMeta[frameIndex] = pInputMeta;
                m_pMergeInputMeta[frameIndex] = pInputMeta;
                CHX_LOG("Snapshot Frame %d", pRequest->frame_number);
                ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i], &m_snapshotBuffers[frameIndex][snapshotCount]);

                snapshotCount++;
                SetFeatureStatus(FeatureStatus::BUSY);

                const Session* pSession = m_pUsecase->GetSessionData(m_bayer2YuvSessionId)->pSession;
                UINT32 sensorModeIndexSnapshot = pSession->GetSensorModeInfo(0)->modeIndex;
                ChxUtils::FillTuningModeData(pInputMeta,
                    pRequest,
                    sensorModeIndexSnapshot,
                    m_pUsecase->GetEffectMode(),
                    m_pUsecase->GetSceneMode(),
                    m_pUsecase->GetFeature1Mode(),
                    m_pUsecase->GetFeature2Mode());
                hasSnapshotStream = TRUE;
            }
            else
            {
                CHX_LOG_INFO("Rejecting  only snapshot request for frame %d", pRequest->frame_number);
            }
        }
    }

    if (FALSE == hasSnapshotStream)
    {
        const Session* pSession       = m_pUsecase->GetSessionData(m_previewSessionId)->pSession;
        UINT32 sensorModeIndexPreview = pSession->GetSensorModeInfo(0)->modeIndex;
        ChxUtils::FillTuningModeData(pInputMeta,
            pRequest,
            sensorModeIndexPreview,
            m_pUsecase->GetEffectMode(),
            m_pUsecase->GetSceneMode(),
            m_pUsecase->GetFeature1Mode(),
            m_pUsecase->GetFeature2Mode());
    }

    if (TRUE == hasPreviewStream)
    {
        PipelineData* pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_previewSessionId, 0));
        UINT          requestIdIndex = (pPipelineData->seqId % MaxOutstandingRequests);

        pPipelineData->seqIdToFrameNum[requestIdIndex] = frameNumber;

        CHX_LOG("FeatureSWMF Realtime AppFrameNum to ReqId: %d <--> %d", frameNumber, pPipelineData->seqId);

        // Include the RDI and FD streams during preview only if it if not FS2 mode
        if (StreamConfigModeFastShutter != ExtensionModule::GetInstance()->GetOpMode(m_pUsecase->GetCameraId()))
        {
            result = m_pUsecase->GetOutputBufferFromRDIQueue(pPipelineData->seqId, m_rdiStreamIndex, &previewBuffers[previewCount]);
            if (CDKResultSuccess == result)
            {
                previewCount++;
            }
            else
            {
                CHX_LOG_ERROR("FeatureSWMF: GetOutputBufferFromRDIQueue failed for frameNumber %d", frameNumber);
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
                    CHX_LOG_ERROR("FeatureSWMF: GetOutputBufferFromFDQueue failed, No FD buffer for frameNumber %d", frameNumber);
                }
            }
        }
        else
        {
            m_lastPreviewFrameNum = frameNumber;
            CHX_LOG("FeatureSWMF: FSMode: Not including RDI and FD steams for preview");
        }

        const Session* pSession   = m_pUsecase->GetSessionData(m_previewSessionId)->pSession;

        CHICAPTUREREQUEST request = { 0 };
        request.frameNumber       = pPipelineData->seqId++;
        request.hPipelineHandle   = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
        request.numOutputs        = previewCount;
        request.pOutputBuffers    = previewBuffers;
        request.pInputMetadata    = pInputMeta->GetHandle();
        ChiMetadata* pOutputMeta  = m_pMetadataManager->Get(pSession->GetMetadataClientId(), request.frameNumber);

        if (NULL == pOutputMeta)
        {
            CHX_LOG_ERROR("OutputMeta not available %p", pOutputMeta);
            return CDKResultEFailed;
        }
        request.pOutputMetadata   = pOutputMeta->GetHandle();

        request.pPrivData                = &m_realtimePrivData[request.frameNumber % MaxOutstandingRequests];
        request.pPrivData->featureType   = FeatureType::SWMF;

        CHIPIPELINEREQUEST submitRequest = { 0 };
        submitRequest.pSessionHandle     = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
        submitRequest.numRequests        = 1;
        submitRequest.pCaptureRequests   = &request;

        m_pUsecase->SetRequestToFeatureMapping(m_previewSessionId, request.frameNumber, this);

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

        m_pUsecase->LogFeatureRequestMappings(pRequest->frame_number, request.frameNumber, "SWMF realtime request");
        result = m_pUsecase->SubmitRequest(&submitRequest);
    }

    if (0 < snapshotCount)
    {
        if (StreamConfigModeFastShutter == ExtensionModule::GetInstance()->GetOpMode(m_pUsecase->GetCameraId()))
        {
            // Wait for all the preview results before submitting FS2 snapshot
            // Wait time is set as 500ms taking care of low light condition as well.
            // For FS2 snapshot we need to submit the RDI request in burst.
            CHX_LOG("FeatureSWMF: FSMode: Waiting for all preview results");

            m_pLastPreviewRequestMutex->Lock();
            result = m_pLastPreviewResultAvailable->TimedWait(m_pLastPreviewRequestMutex->GetNativeHandle(), WaitAllPreviewResultTime);
            m_pLastPreviewRequestMutex->Unlock();

            if (CamxResultSuccess != result)
            {
                CHX_LOG_ERROR("TimedWait for results timed out at %u ms ", WaitAllPreviewResultTime);
            }
            else
            {
                CHX_LOG("TimedWait returned with success ");
            }

            if (CamxResultSuccess == result)
            {
                result = ExecuteFastShutterSnapshotRequest(pRequest, pInputMeta, snapshotCount);
            }
        }
        else
        {
            m_isSnapshotFrame[frameIndex]         = TRUE;
            m_snapshotBufferNum[frameIndex]       = snapshotCount;
            m_snapshotReqIdToFrameNum[frameIndex] = frameNumber;
            m_maxSnapshotReqId++;

            m_numBayer2YuvFrames = 0;

            CHX_LOG("FeatureSWMF Snapshot Request received, appFrameNum:%d.", pRequest->frame_number);
            // If we have the requirements for the snapshot frame, kick it off immediately
            if (TRUE == m_isSnapshotFrame[frameIndex])
            {
                UINT32 realtimeReqId    = m_pRdiTargetBuffer->lastReadySequenceID;
                UINT32 reqId            = 0;
                UINT32 queueIndex       = 0;

                for (UINT32 i = 0; i < m_numMultiFramesRequired; i++)
                {
                    reqId = (i > realtimeReqId) ? 0 : realtimeReqId - i;
                    queueIndex = reqId % BufferQueueDepth;

                    if (NULL == m_pRdiTargetBuffer->bufferQueue[queueIndex].pRdiOutputBuffer->
                        bufferInfo.phBuffer)
                    {
                        CHX_LOG_ERROR(" FATAL RDI Buffer is NULL! Dont start Offline Processing!");
                        result           = CDKResultEFailed;
                        SetFeatureStatus(FeatureStatus::READY);

                    }
                }

                if (CDKResultSuccess == result)
                {

                    for (UINT32 i = 0; i < m_numMultiFramesRequired; i++)
                    {
                        reqId = (i > realtimeReqId) ? 0 : realtimeReqId - i;
                        queueIndex = reqId % BufferQueueDepth;

                        CHX_LOG("FeatureSWMF send buffer to Bayer2YuvPipeline, frame number:%d, reqId:%d, phBuffer:%p, meta:%p",
                            frameNumber, reqId,
                            m_pRdiTargetBuffer->bufferQueue[queueIndex].pRdiOutputBuffer->bufferInfo.phBuffer,
                            m_pRdiTargetBuffer->bufferQueue[queueIndex].pMetadata);

                        const PipelineData* pPipelineData = m_pUsecase->GetPipelineData(m_bayer2YuvSessionId, 0);
                        CHISTREAMBUFFER     outputBuffer  = { 0 };

                        outputBuffer.bufferInfo         = m_pBayer2YuvTargetBuffer->pBufferManager->GetImageBufferInfo();
                        outputBuffer.pStream            = m_pBayer2YuvStream;
                        outputBuffer.acquireFence.valid = FALSE;
                        outputBuffer.size               = sizeof(CHISTREAMBUFFER);

                        if (CDKResultSuccess == result)
                        {
                            if (FALSE == hasSnapshotStream)
                            {
                                const Session* pSession = m_pUsecase->GetSessionData(m_previewSessionId)->pSession;
                                UINT32 sensorModeIndexPreview = pSession->GetSensorModeInfo(0)->modeIndex;
                                ChxUtils::FillTuningModeData(pInputMeta,
                                    pRequest,
                                    sensorModeIndexPreview,
                                    m_pUsecase->GetEffectMode(),
                                    m_pUsecase->GetSceneMode(),
                                    m_pUsecase->GetFeature1Mode(),
                                    m_pUsecase->GetFeature2Mode());
                            }
                            if (TRUE == hasSnapshotStream)
                            {
                                const Session* pSession = m_pUsecase->GetSessionData(m_bayer2YuvSessionId)->pSession;
                                UINT32 sensorModeIndexSnapshot = pSession->GetSensorModeInfo(0)->modeIndex;
                                ChxUtils::FillTuningModeData(pInputMeta,
                                    pRequest,
                                    sensorModeIndexSnapshot,
                                    m_pUsecase->GetEffectMode(),
                                    m_pUsecase->GetSceneMode(),
                                    m_pUsecase->GetFeature1Mode(),
                                    m_pUsecase->GetFeature2Mode());
                            }

                            // TODO: have another thread for offline request processing to unblock realtime thread
                            SubmitRequest(m_bayer2YuvSessionId, frameNumber, m_pRdiTargetBuffer,
                                &outputBuffer, reqId, i, TRUE, "SWMF Bayer2Yuv Snapshot Request");
                        }
                        else
                        {
                            CHX_LOG_ERROR("FeatureSWMF request is not submitted to Bayer2YuvPipeline! frameNumber = %d", frameNumber);
                            break;
                        }
                    }

                    if (CDKResultSuccess == result)
                    {
                        // Flush the RDI queue as all the buffers of frameNumber older than 'reqId' won't be needed anymore
                        m_pUsecase->FlushRDIQueue(reqId, m_rdiStreamIndex);
                    }
                }
            }
        }
    }

    return result;
}

//  @todo (CAMX-4432): Move submit future RDI request functionality to base feature class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMultiframe::ExecuteFastShutterSnapshotRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult  FeatureMultiframe::ExecuteFastShutterSnapshotRequest(
    camera3_capture_request_t* pRequest,
    ChiMetadata*               pInputMeta,
    UINT                       snapshotCount)
{
    CDKResult       result                                   = CDKResultSuccess;
    UINT            frameNumber                              = pRequest->frame_number;
    UINT32          frameIndex                               = (pRequest->frame_number % MaxOutstandingRequests);
    CHISTREAMBUFFER snapshotRDIbuffers[m_SWMFNumFramesforFS] = {{0}};
    UINT            snapshotRdicount                         = 0;

    m_isSnapshotFrame[frameIndex]             = TRUE;
    m_snapshotBufferNum[frameIndex]           = snapshotCount;
    m_snapshotReqIdToFrameNum[frameIndex]     = frameNumber;
    m_maxSnapshotReqId++;
    m_numSnapshotRdiFramesreceived            = 0;
    m_numBayer2YuvFrames                      = 0;

    // FS2: Using the same realtime pipeline/session for snapshot RDI. In FS2, during snapshot there is no preview request
    PipelineData*  pPipelineData          = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_previewSessionId, 0));
    const Session* pSession               = m_pUsecase->GetSessionData(m_previewSessionId)->pSession;
    UINT           requestIdIndex;
    UINT           skipSnapshotStatsParse = 1;

    CHX_LOG("FS2Mode Snapshot: Skip Stats processing for RDI request framenum: %d", frameNumber);
    pInputMeta->SetTag("com.qti.chi.statsSkip", "skipFrame", &skipSnapshotStatsParse, 1);

    // We have to submit m_SWMFNumFramesforFS RDI requests for snapshot in FastShutter mode
    for (UINT32 i = 0; i < m_SWMFNumFramesforFS; i++)
    {
        CHX_LOG("FS2Mode Snapshot: FeatureSWMF Realtime AppFrameNum to ReqId: %d <--> %d",
                                                       frameNumber, pPipelineData->seqId);
        requestIdIndex = (pPipelineData->seqId % MaxOutstandingRequests);
        pPipelineData->seqIdToFrameNum[requestIdIndex] = frameNumber;

        pInputMeta->AddReference();

        m_pUsecase->GetOutputBufferFromRDIQueue(pPipelineData->seqId, m_rdiStreamIndex, &snapshotRDIbuffers[snapshotRdicount]);

        CHICAPTUREREQUEST request        = { 0 };
        request.frameNumber              = pPipelineData->seqId++;
        request.hPipelineHandle          = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
        request.numOutputs               = 1;
        request.pOutputBuffers           = &snapshotRDIbuffers[snapshotRdicount];
        request.pInputMetadata           = pInputMeta->GetHandle();
        ChiMetadata* pOutputMeta         = m_pMetadataManager->Get(pSession->GetMetadataClientId(), request.frameNumber);

        if (NULL == pOutputMeta)
        {
            CHX_LOG_ERROR("OutputMeta not available %p", pOutputMeta);
            return CDKResultEFailed;
        }
        request.pOutputMetadata          = pOutputMeta->GetHandle();

        request.pPrivData                = &m_realtimePrivData[request.frameNumber % MaxOutstandingRequests];
        request.pPrivData->featureType   = FeatureType::SWMF;

        CHIPIPELINEREQUEST submitRequest = { 0 };
        submitRequest.pSessionHandle     = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
        submitRequest.numRequests        = 1;
        submitRequest.pCaptureRequests   = &request;

        m_pUsecase->SetRequestToFeatureMapping(m_previewSessionId, request.frameNumber, this);
        m_pUsecase->LogFeatureRequestMappings(pRequest->frame_number, request.frameNumber, "SWMF FastShutter Preview Request");
        result = m_pUsecase->SubmitRequest(&submitRequest);

        snapshotRdicount = (snapshotRdicount + 1) % m_SWMFNumFramesforFS;
    }
    return result;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMultiframe::ProcessResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMultiframe::ProcessResult(
    CHICAPTURERESULT*   pResult,
    VOID*               pPrivateCallbackData)
{
    if ((TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress))))
    {
        CHX_LOG_INFO("Multiframe process result return because of cleanup");
        return;
    }

    CDKResult           result                = CDKResultSuccess;
    SessionPrivateData* pCbData               = static_cast<SessionPrivateData*>(pPrivateCallbackData);
    BOOL                isAppResultsAvailable = FALSE;
    UINT32              resultFrameNum        = pResult->frameworkFrameNum;
    UINT32              realtimeReqId         = 0;
    UINT32              offlineReqId          = 0;

    if (m_previewSessionId != pCbData->sessionId)
    {
        offlineReqId   = pResult->frameworkFrameNum;
        resultFrameNum = m_pUsecase->GetChiFrameNumFromReqId(pCbData->sessionId, 0, offlineReqId);
        CHX_LOG("offline ReqId to AppFrameNum: %d <--> %d", pResult->frameworkFrameNum, resultFrameNum);
    }
    else if (m_previewSessionId == pCbData->sessionId)
    {
        realtimeReqId  = pResult->frameworkFrameNum;
        resultFrameNum = m_pUsecase->GetChiFrameNumFromReqId(m_previewSessionId, 0, realtimeReqId);

        CHX_LOG("Realtime ReqId to AppFrameNum: %d <--> %d", pResult->frameworkFrameNum, resultFrameNum);
    }

    CHX_LOG("FeatureSWMF ProcessResult resultFrameNum: %d, sessionId: %d, pResultMetadata: %p, numOutputBuffers: %d",
            resultFrameNum, pCbData->sessionId, pResult->pResultMetadata, pResult->numOutputBuffers);
    CHX_LOG("pResult->pOutputMetadata = %p, pResult->pInputMetadata = %p", pResult->pOutputMetadata, pResult->pInputMetadata);

    UINT32                    resultFrameIndex = resultFrameNum % MaxOutstandingRequests;
    camera3_capture_result_t* pUsecaseResult   = m_pUsecase->GetCaptureResult(resultFrameIndex);

    pUsecaseResult->frame_number = resultFrameNum;

    // If result contain metadata and metadata has not been sent to framework
    if ((NULL != pResult->pOutputMetadata) && (NULL != pResult->pInputMetadata))
    {
        ChiMetadata*   pInputMeta   = m_pMetadataManager->GetMetadataFromHandle(pResult->pInputMetadata);
        ChiMetadata*   pOutputMeta  = m_pMetadataManager->GetMetadataFromHandle(pResult->pOutputMetadata);
        const Session* pSession     = m_pUsecase->GetSessionData(pCbData->sessionId)->pSession;

        m_pMetadataManager->Release(pInputMeta);

        BOOL    isResultMetadataAvailable = FALSE;
        UINT64  timestamp                 = m_shutterTimestamp[resultFrameNum % MaxOutstandingRequests];

        if (0 == timestamp)
        {
            timestamp = m_pUsecase->GetRequestShutterTimestamp(resultFrameNum);
        }

        if ((FALSE == m_pUsecase->IsMetadataSent(resultFrameIndex)) && NULL == pUsecaseResult->result)
        {
            // Do Not wait for Snapshot frame metadata, Return Preview metadata back to fwk.
            // If we wait for snapshot, and if it takes more time to process, we will block the preview.
            pUsecaseResult->partial_result = pResult->numPartialMetadata;
            isResultMetadataAvailable = TRUE;

            // Override snapshot frame sensor timestamp metadata with shutter event for same frame number
            if (0 != timestamp)
            {
                ChxUtils::UpdateTimeStamp(pOutputMeta, timestamp, resultFrameNum);
                if (TRUE == isResultMetadataAvailable)
                {
                    m_pUsecase->SetMetadataAvailable(resultFrameIndex);
                    isAppResultsAvailable = TRUE;

                    m_pUsecase->UpdateAppResultMetadata(pOutputMeta,
                                                        resultFrameIndex,
                                                        m_pUsecase->GetMetadataClientIdFromPipeline(pCbData->sessionId, 0));

                    CHX_LOG("returnmetadata SWMF JPEG metadata：%d", resultFrameNum);
                }
            }
            // If shutter from realtime for snapshot frame has not come yet, metadata will be dispatched from notify
            else
            {
                CHX_LOG("add metadata to capture result for frame %d", resultFrameNum);
                m_pUsecase->UpdateAppResultMetadata(pOutputMeta,
                                                    resultFrameIndex,
                                                    m_pUsecase->GetMetadataClientIdFromPipeline(pCbData->sessionId, 0));
            }

        }

        if (m_previewSessionId == pCbData->sessionId)
        {
            // in FS2 mode there is NO RDI request along with preview
            if (StreamConfigModeFastShutter != ExtensionModule::GetInstance()->GetOpMode(m_pUsecase->GetCameraId()) ||
                TRUE == m_isSnapshotFrame[resultFrameIndex])
            {
                m_pUsecase->FillMetadataForRDIQueue(realtimeReqId, m_rdiStreamIndex, pOutputMeta);
            }
        }
        else if (m_bayer2YuvSessionId == pCbData->sessionId)
        {
            UINT32 bufferQueueIdx = (offlineReqId % BufferQueueDepth);

            CHX_LOG("meta for Bayer2Yuv Pipeline, frame number:%d, meta:%p", resultFrameNum, pOutputMeta->GetHandle());

            m_pBayer2YuvTargetBuffer->bufferQueue[bufferQueueIdx].pMetadata = pOutputMeta;
            m_pBayer2YuvTargetBuffer->bufferQueue[bufferQueueIdx].frameNumber = resultFrameNum;
        }
        else if (m_mergeSessionId == pCbData->sessionId)
        {
            CHX_LOG("meta for Merge Pipeline, frame number:%d, meta:%p", resultFrameNum, pOutputMeta->GetHandle());

            UINT32 queueIndex = (resultFrameNum % BufferQueueDepth);

            m_pMergeTargetBuffer->bufferQueue[queueIndex].pMetadata   = pOutputMeta;
            m_pMergeTargetBuffer->bufferQueue[queueIndex].frameNumber = resultFrameNum;
            m_featureOutputMetaHandle                                 = pResult->pOutputMetadata;
        }
        else if (m_snapshotSessionId == pCbData->sessionId)
        {
            CHX_LOG("meta for jpeg Pipeline, frame number:%d, meta:%p", resultFrameNum, pOutputMeta->GetHandle());
            m_pMetadataManager->Release(pOutputMeta);
        }
    }

    for (UINT32 j = 0; j < pResult->numOutputBuffers; j++)
    {
        // If our internal stream, copy the result into the target buffer to be consumed by the offline pipeline
        if (m_pRdiStream == pResult->pOutputBuffers[j].pStream)
        {
            if (StreamConfigModeFastShutter == ExtensionModule::GetInstance()->GetOpMode(m_pUsecase->GetCameraId()))
            {
                m_numSnapshotRdiFramesreceived++;
                CHX_LOG("FS2: received Snapshot RDI frames:%d", m_numSnapshotRdiFramesreceived);

                if (CDKResultSuccess == result)
                {
                    UINT32              bufferQueueIdx      = (realtimeReqId % BufferQueueDepth);
                    CHX_LOG("realtimeReqId = %d, bufferQueueIdx = %d", realtimeReqId, bufferQueueIdx);
                    CHISTREAMBUFFER*    pRdiOutputBuffer    = m_pRdiTargetBuffer->bufferQueue[bufferQueueIdx].pRdiOutputBuffer;

                    ChxUtils::Memcpy(pRdiOutputBuffer, &pResult->pOutputBuffers[j], sizeof(CHISTREAMBUFFER));
                    pRdiOutputBuffer->acquireFence.valid  = FALSE;
                    pRdiOutputBuffer->size                = sizeof(CHISTREAMBUFFER);

                    PipelineData* pPipelineData = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_bayer2YuvSessionId, 0));
                    const Session* pSession   = m_pUsecase->GetSessionData(m_bayer2YuvSessionId)->pSession;

                    UINT          requestIdIndex = (pPipelineData->seqId % MaxOutstandingRequests);
                    pPipelineData->seqIdToFrameNum[requestIdIndex] = resultFrameNum;

                    CHISTREAMBUFFER     outputBuffer  = { 0 };

                    outputBuffer.bufferInfo         = m_pBayer2YuvTargetBuffer->pBufferManager->GetImageBufferInfo();
                    outputBuffer.pStream            = m_pBayer2YuvStream;
                    outputBuffer.acquireFence.valid = FALSE;
                    outputBuffer.size               = sizeof(CHISTREAMBUFFER);

                    CHICAPTUREREQUEST request = { 0 };
                    request.frameNumber       = pPipelineData->seqId++;
                    request.hPipelineHandle   = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
                    request.numInputs         = 1;
                    request.numOutputs        = 1;
                    request.pInputBuffers     = pRdiOutputBuffer;
                    request.pOutputBuffers    = &outputBuffer;;
                    request.pInputMetadata    = m_pRdiTargetBuffer->bufferQueue[bufferQueueIdx].pMetadata->GetHandle();
                    ChiMetadata* pOutputMeta  = m_pMetadataManager->Get(pSession->GetMetadataClientId(), request.frameNumber);

                    if (NULL == pOutputMeta)
                    {
                        CHX_LOG_ERROR("OutputMeta not available %p", pOutputMeta);
                        return;
                    }
                    request.pOutputMetadata   = pOutputMeta->GetHandle();

                    request.pPrivData                = &m_offlinePrivData[request.frameNumber % MaxOutstandingRequests];
                    request.pPrivData->featureType   = FeatureType::SWMF;

                    // Save input buffers info for later releasing reference
                    request.pPrivData->numInputBuffers      = 1;
                    request.pPrivData->inputBuffers[0]      = m_pRdiTargetBuffer->bufferQueue[bufferQueueIdx].pRdiOutputBuffer->bufferInfo;
                    request.pPrivData->bufferManagers[0]    = m_pRdiTargetBuffer->pBufferManager;

                    CHIPIPELINEREQUEST submitRequest = { 0 };
                    submitRequest.pSessionHandle     = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
                    submitRequest.numRequests        = 1;
                    submitRequest.pCaptureRequests   = &request;

                    m_pUsecase->SetRequestToFeatureMapping(m_bayer2YuvSessionId, request.frameNumber, this);

                    result = m_pUsecase->SubmitRequest(&submitRequest);
                }
            }
            else
            {
                m_pUsecase->UpdateBufferReadyForRDIQueue(realtimeReqId, m_rdiStreamIndex, TRUE);
            }
        }
        else if (m_pFdStream == pResult->pOutputBuffers[j].pStream)
        {
            m_pUsecase->UpdateBufferReadyForFDQueue(realtimeReqId, m_fdStreamIndex, TRUE);
        }
        else if (m_pBayer2YuvStream == pResult->pOutputBuffers[j].pStream)
        {
            CHX_LOG("FeatureSWMF Received Bayer2Yuv stream, reqid:%d, frame num:%d", offlineReqId, resultFrameNum);

            // Release reference to the input buffers of the request to Bayer2YuvPipeline
            m_pUsecase->ReleaseReferenceToInputBuffers(pResult->pPrivData);

            // Dynamically increasing ValidBufferLength as soon as RDI buffer is release so that
            // more will be reserved in the queue.
            if (TRUE == m_pUsecase->IsMultiCameraUsecase())
            {
                m_pUsecase->UpdateValidRDIBufferLength(m_activePipelineID, 1);
            }
            else
            {
                //m_pUsecase->UpdateValidRDIBufferLength(m_rdiStreamIndex, 1);
            }

            UINT32              bufferQueueIdx      = (offlineReqId % BufferQueueDepth);
            CHISTREAMBUFFER*    pRdiOutputBuffer    = m_pBayer2YuvTargetBuffer->bufferQueue[bufferQueueIdx].pRdiOutputBuffer;

            ChxUtils::Memcpy(pRdiOutputBuffer, &pResult->pOutputBuffers[j], sizeof(CHISTREAMBUFFER));

            pRdiOutputBuffer->acquireFence.valid  = FALSE;
            pRdiOutputBuffer->size                = sizeof(CHISTREAMBUFFER);

            m_numBayer2YuvFrames++;
            CHX_LOG("received bayer2Yuv frames:%d", m_numBayer2YuvFrames);

            // submit request to merge pipeline until we got all buffers and frames.
            // because meta alway comes before or along with frames, so here it's sure meta is received.
            if ((TRUE == m_isSnapshotFrame[resultFrameIndex]) && (m_numBayer2YuvFrames == m_numMultiFramesRequired))
            {
                CHX_LOG("FeatureSWMF received all bayer2Yuv Frames, send to merge pipeline..");

                UINT32 offlineThreadReqNum = m_offlineThreadCaptureRequestId % MaxMultiFrameRequests;

                ChxUtils::Memset(&m_offlineRequestData[offlineThreadReqNum], 0, sizeof(CHXOFFLINEREQUEST));
                CHISTREAMBUFFER* pOutputBuffer = &m_offlineRequestData[offlineThreadReqNum].outputBuffer;
                if (TRUE == isJPEGOutputRequired())
                {
                    pOutputBuffer->bufferInfo     = m_pMergeTargetBuffer->pBufferManager->GetImageBufferInfo();
                }
                else
                {
                    pOutputBuffer->bufferInfo     = m_snapshotBuffers[resultFrameIndex][0].bufferInfo;
                    pOutputBuffer->bufferInfo.bufferType = ChiNative;
                }
                pOutputBuffer->pStream            = m_pMergeYuvStream;
                pOutputBuffer->acquireFence.valid = FALSE;
                pOutputBuffer->size               = sizeof(CHISTREAMBUFFER);

                if (CDKResultSuccess == result)
                {
                    PipelineData* pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_mergeSessionId, 0));
                    UINT          requestIdIndex = (pPipelineData->seqId % MaxOutstandingRequests);
                    UINT32        frameIndex     = (resultFrameNum % MaxOutstandingRequests);

                    CHISTREAMBUFFER* pInputBuffer = &m_offlineRequestData[offlineThreadReqNum].inputBuffer[0];

                    pPipelineData->seqIdToFrameNum[requestIdIndex]      = resultFrameNum;

                    // find the first ReqId for this snapshot
                    UINT32 firstReqId = offlineReqId - m_numBayer2YuvFrames + 1;
                    CHX_LOG("firstReqId:%d", firstReqId);


                    CHICAPTUREREQUEST* pRequest = &m_offlineRequestData[offlineThreadReqNum].request;
                    pRequest->frameNumber       = pPipelineData->seqId++;
                    pRequest->pPrivData               = &m_privData[pRequest->frameNumber % MaxOutstandingRequests];
                    pRequest->pPrivData->featureType  = GetFeatureType();
                    pRequest->pPrivData->streamIndex  = m_physicalCameraIndex;

                    for (UINT32 i = 0; i < m_numBayer2YuvFrames; i++)
                    {
                        CHX_LOG("%d, get input buffer from reqid:%d", i, firstReqId + i);
                        m_pUsecase->GetTargetBuffer(firstReqId + i,
                                                    m_pBayer2YuvTargetBuffer,
                                                    m_pSnapshotInputMeta[frameIndex],
                                                    &pInputBuffer[i],
                                                    &m_offlineRequestData[offlineThreadReqNum].pRequestMetadata);

                        pInputBuffer[i].pStream = reinterpret_cast<CHISTREAM*>(m_pMergePipelineInputStreams[i]);

                        pRequest->pPrivData->inputBuffers[i]      = pInputBuffer[i].bufferInfo;
                        pRequest->pPrivData->bufferManagers[i]    = m_pBayer2YuvTargetBuffer->pBufferManager;

                        // Only need one output metadata, we can release the other inputs
                        // todo: Need to couple this with release reference to input buffer
                        if (i < m_numBayer2YuvFrames - 1)
                        {
                            ChiMetadata* pReleaseMeta = m_offlineRequestData[offlineThreadReqNum].pRequestMetadata;
                            if (NULL != pReleaseMeta)
                            {
                                m_pMetadataManager->Release(pReleaseMeta);
                            }
                        }
                    }
                    pRequest->pPrivData->numInputBuffers = m_numBayer2YuvFrames;

                    const Session*  pSession  = m_pUsecase->GetSessionData(m_mergeSessionId)->pSession;

                    ChiMetadata* pInputMeta  = m_offlineRequestData[offlineThreadReqNum].pRequestMetadata;
                    ChiMetadata* pOutputMeta = m_pMetadataManager->Get(pSession->GetMetadataClientId(), pRequest->frameNumber);

                    if ((NULL == pInputMeta) || (NULL == pOutputMeta))
                    {
                        CHX_LOG_ERROR("Metadata not available %p %p", pInputMeta, pOutputMeta);
                        return;
                    }

                    pRequest->hPipelineHandle   = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
                    pRequest->numInputs         = m_numBayer2YuvFrames;
                    pRequest->numOutputs        = m_snapshotBufferNum[frameIndex];
                    pRequest->pInputBuffers     = reinterpret_cast<CHISTREAMBUFFER*>(pInputBuffer);
                    pRequest->pOutputBuffers    = reinterpret_cast<CHISTREAMBUFFER*>(pOutputBuffer);
                    pRequest->pInputMetadata    = pInputMeta->GetHandle();
                    pRequest->pOutputMetadata   = pOutputMeta->GetHandle();

                    if (TRUE == m_pUsecase->IsMultiCameraUsecase())
                    {
                        pRequest->pMetadata     = m_pBayer2YuvTargetBuffer->bufferQueue[firstReqId % BufferQueueDepth].pMetadata;
                    }

                    CHIPIPELINEREQUEST* pSubmitRequest    = &m_offlineRequestData[offlineThreadReqNum].submitRequest;
                    pSubmitRequest->pSessionHandle        = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
                    pSubmitRequest->numRequests           = 1;
                    pSubmitRequest->pCaptureRequests      = pRequest;

                    CHX_LOG_INFO("FeatureSWMF Sending SWMF request pipelineIdx: %d frameNumber:%d, request:%" PRIu64 " metadata:%p",
                        m_mergePipelineIndex, resultFrameNum, pRequest->frameNumber, pRequest->pMetadata);

                    m_pUsecase->SetRequestToFeatureMapping(m_mergeSessionId, pRequest->frameNumber, this);

                    //  There is only one request in submit request
                    if ((TRUE == ChxUtils::HasInputBufferError(pSubmitRequest)) ||
                        (FlushStatus::NotFlushing != m_pUsecase->GetFlushStatus()))
                    {
                        InvalidateRequest(pSubmitRequest);
                    }
                    else
                    {
                        m_pUsecase->LogFeatureRequestMappings(resultFrameNum, pRequest->frameNumber, "SWMF Offline Merge Request");
                        m_pOfflineRequestMutex->Lock();
                        m_pOfflineRequestAvailable->Signal();
                        m_offlineThreadCaptureRequestId++;
                        m_pOfflineRequestMutex->Unlock();
                    }
                }
                else
                {
                    CHX_LOG("FeatureSWMF request is not submitted to merge pipeline! frameNumber = %d", resultFrameNum);
                }
            }
        }
        else if (m_pMergeYuvStream == pResult->pOutputBuffers[j].pStream)
        {
            CHX_LOG("FeatureSWMF Received MergeYuvStream stream, reqid:%d, frame num:%d", offlineReqId, resultFrameNum);

            UINT32              queueIndex          = (resultFrameNum % BufferQueueDepth);
            CHISTREAMBUFFER*    pRdiOutputBuffer    = m_pMergeTargetBuffer->bufferQueue[queueIndex].pRdiOutputBuffer;

            ChxUtils::Memcpy(pRdiOutputBuffer, &pResult->pOutputBuffers[j], sizeof(CHISTREAMBUFFER));

            pRdiOutputBuffer->acquireFence.valid  = FALSE;
            pRdiOutputBuffer->size                = sizeof(CHISTREAMBUFFER);

            // Release reference to the input buffers of the request to the MergeYUVPipeline
            m_pUsecase->ReleaseReferenceToInputBuffers(pResult->pPrivData);

            // If we have the requirements for the snapshot frame, kick it off immediately
            if (TRUE == m_isSnapshotFrame[resultFrameIndex])
            {
                if (TRUE == isJPEGOutputRequired())
                {
                    SubmitRequest(m_snapshotSessionId, resultFrameNum, m_pMergeTargetBuffer,
                                  &m_snapshotBuffers[resultFrameNum % MaxOutstandingRequests][0], 0, 0, FALSE,
                                  "SWMF Snapshot JPEG Request");
                }
                else
                {
                    if (NULL == pResult->pOutputMetadata)
                    {
                        pResult->pOutputMetadata = m_featureOutputMetaHandle;
                    }
                    SetFeatureStatus(FeatureStatus::READY);
                    m_pUsecase->ProcessFeatureDone(m_internalFrameNum, this, pResult);
                    CHX_LOG_ERROR("SWMF YUV buffer is coming!");
                }
            }
        }
        // Otherwise queue a buffer as part of the normal result
        else
        {
            CHX_LOG_ERROR("Result stream:%p",pResult->pOutputBuffers[j].pStream);
            if (m_pSnapshotStream == pResult->pOutputBuffers[j].pStream)
            {
                CHX_LOG("FeatureSWMF Received Final Snapshot(JPEG) stream, reqId:%d, frameNum:%d", offlineReqId, resultFrameNum);

                // Release reference to the input buffers of the request to the SnapshotPipeline
                m_pUsecase->ReleaseReferenceToInputBuffers(pResult->pPrivData);
                SetFeatureStatus(FeatureStatus::READY);
            }

            if (StreamConfigModeFastShutter == ExtensionModule::GetInstance()->GetOpMode(m_pUsecase->GetCameraId()) &&
                 (m_pPreviewStream == pResult->pOutputBuffers[j].pStream) &&
                 (resultFrameNum == m_lastPreviewFrameNum))
            {
                m_pLastPreviewRequestMutex->Lock();
                m_pLastPreviewResultAvailable->Signal();
                m_pLastPreviewRequestMutex->Unlock();
            }

            m_pUsecase->GetAppResultMutex()->Lock();
            camera3_stream_buffer_t* pResultBuffer =
                const_cast<camera3_stream_buffer_t*>(&pUsecaseResult->output_buffers[pUsecaseResult->num_output_buffers++]);

            ChxUtils::PopulateChiToHALStreamBuffer(&pResult->pOutputBuffers[j], pResultBuffer);
            m_pUsecase->GetAppResultMutex()->Unlock();

            isAppResultsAvailable = TRUE;
        }
    }

    if (TRUE == isAppResultsAvailable)
    {
        CHX_LOG("isAppResultsAvailable:%d", isAppResultsAvailable);
        m_pUsecase->ProcessAndReturnFinishedResults();
    }

    if ((1 <= ExtensionModule::GetInstance()->EnableDumpDebugData())    &&
        (NULL != pResult->pOutputMetadata)                              &&
        (m_previewSessionId != pCbData->sessionId)                      &&
        (m_mergeSessionId   != pCbData->sessionId))
    {
        // Process debug-data only for offline processing
        m_pUsecase->ProcessDebugData(pResult, pPrivateCallbackData, resultFrameNum);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMultiframe::ProcessCHIPartialData
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMultiframe::ProcessCHIPartialData(
    UINT32    frameNum,
    UINT32    sessionId)
{
    CAMX_UNREFERENCED_PARAM(frameNum);
    CAMX_UNREFERENCED_PARAM(sessionId);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMultiframe::ProcessDriverPartialCaptureResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMultiframe::ProcessDriverPartialCaptureResult(
    CHIPARTIALCAPTURERESULT*    pResult,
    VOID*                       pPrivateCallbackData)
{
    UINT32              resultFrameNum = pResult->frameworkFrameNum;
    UINT32              realtimeReqId = 0;
    UINT32              offlineReqId = 0;
    UINT32              resultFrameIndex = resultFrameNum % MaxOutstandingRequests;
    ChiMetadata*        pChiOutputMetadata = NULL;

    SessionPrivateData*         pCbData =
        static_cast<SessionPrivateData*>(pPrivateCallbackData);
    PartialResultSender         sender =
        PartialResultSender::DriverPartialData;

    if (m_previewSessionId != pCbData->sessionId)
    {
        offlineReqId = pResult->frameworkFrameNum;
        resultFrameNum = m_pUsecase->GetChiFrameNumFromReqId(pCbData->sessionId, 0, offlineReqId);
        CHX_LOG("Offline ReqId to AppFrameNum: %d <--> %d", pResult->frameworkFrameNum, resultFrameNum);
    }
    else if (m_previewSessionId == pCbData->sessionId)
    {
        realtimeReqId = pResult->frameworkFrameNum;
        resultFrameNum = m_pUsecase->GetChiFrameNumFromReqId(m_previewSessionId, 0, realtimeReqId);
        CHX_LOG("Realtime ReqId to AppFrameNum: %d <--> %d", pResult->frameworkFrameNum, resultFrameNum);
    }

    resultFrameIndex = resultFrameNum % MaxOutstandingRequests;
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
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMultiframe::ProcessMessage
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMultiframe::ProcessMessage(
    const CHIMESSAGEDESCRIPTOR* pMessageDescriptor,
    VOID*                       pPrivateCallbackData)
{
    (VOID)pMessageDescriptor;
    (VOID)pPrivateCallbackData;
    //@todo this function to be REMOVED
}

INT32 FeatureMultiframe::GetRequiredPipelines(
    AdvancedPipelineType* pPipelines,
    INT32 size)
{
    INT32 count = 0;
    const INT32 maxPipelineCount = 8;

    if (NULL != pPipelines && size >= maxPipelineCount)
    {
        INT32 index = 0;
        AdvancedPipelineType pipelineGroup[1];
        UINT                 cameraId[1];
        UINT                 physicalCameraID = m_pUsecase->GetPhysicalCameraId(m_physicalCameraIndex);

        if (InvalidPhysicalCameraId != physicalCameraID)
        {
            if (TRUE == isJPEGOutputRequired())
            {
                pPipelines[index]    = AdvancedPipelineType::InternalZSLYuv2JpegType;
                pipelineGroup[0]     = pPipelines[index];
                cameraId[0]          = physicalCameraID;
                m_snapshotSessionId  = m_pUsecase->GetUniqueSessionId(&pipelineGroup[0], 1, m_physicalCameraIndex);
                m_pUsecase->SetPipelineCameraId(&pipelineGroup[0], &cameraId[0], 1);
                index++;
            }

            pPipelines[index]    = AdvancedPipelineType::SWMFMergeYuvType;
            pipelineGroup[0]     = pPipelines[index];
            cameraId[0]          = physicalCameraID;
            m_mergeSessionId     = m_pUsecase->GetUniqueSessionId(&pipelineGroup[0], 1, m_physicalCameraIndex);
            m_pUsecase->SetPipelineCameraId(&pipelineGroup[0], &cameraId[0], 1);
            index++;

            pPipelines[index]    = AdvancedPipelineType::ZSLSnapshotYUVType;
            pipelineGroup[0]     = pPipelines[index];
            cameraId[0]          = physicalCameraID;
            m_bayer2YuvSessionId = m_pUsecase->GetUniqueSessionId(&pipelineGroup[0], 1, m_physicalCameraIndex);
            m_pUsecase->SetPipelineCameraId(&pipelineGroup[0], &cameraId[0], 1);
            index++;

            if (FALSE == m_pUsecase->IsMultiCameraUsecase())
            {
                if (StreamConfigModeFastShutter == ExtensionModule::GetInstance()->GetOpMode(m_pUsecase->GetCameraId()))
                {
                    pPipelines[index]    = AdvancedPipelineType::ZSLPreviewRawFSType;
                }
                else
                {
                    pPipelines[index]    = AdvancedPipelineType::ZSLPreviewRawType;
                }
                pipelineGroup[0]     = pPipelines[index];
                cameraId[0]          = physicalCameraID;
                m_previewSessionId   = m_pUsecase->GetUniqueSessionId(&pipelineGroup[0], 1, m_physicalCameraIndex);
                m_pUsecase->SetPipelineCameraId(&pipelineGroup[0], &cameraId[0], 1);
                index++;
            }
            else
            {
                m_previewSessionId = InvalidSessionId;
                CHX_LOG("Don't use Realtime pipeline in advance usecase for multicamera usecase");
            }

            CHX_LOG("m_previewSessionId:%d, m_bayer2YuvSessionId:%d, m_mergeSessionId:%d, m_snapshotSessionId:%d",
                    m_previewSessionId,
                    m_bayer2YuvSessionId,
                    m_mergeSessionId,
                    m_snapshotSessionId);

            count = index;
        }
    }

    CHX_LOG("FeatureSWMF::GetRequiredPipelines, required pipeline count:%d", count);
    return count;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMultiframe::GetOutputBufferManager
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CHIBufferManager* FeatureMultiframe::GetOutputBufferManager(
    CHISTREAMBUFFER* pOutputBuffer)
{
    CHIBufferManager* result  = NULL;
    CHISTREAM* const  pStream = pOutputBuffer->pStream;

    if (pStream == m_pMergeYuvStream)
    {
        result = m_pMergeTargetBuffer->pBufferManager;
    }
    else if (pStream == m_pBayer2YuvStream)
    {
        result = m_pBayer2YuvTargetBuffer->pBufferManager;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMultiframe::SubmitRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMultiframe::SubmitRequest(UINT32 sessionIdx, UINT frameNumber,
                                      TargetBuffer* pInputTargetBuffer,
                                      CHISTREAMBUFFER *pOutputBuffer,
                                      UINT32 inputPipelineReqId,
                                      UINT32 bufferIndex,
                                      BOOL   isBatched,
                                      const CHAR* identifierData)
{
    CDKResult      result         = CDKResultSuccess;
    const Session* pSession       = m_pUsecase->GetSessionData(sessionIdx)->pSession;
    PipelineData*  pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(sessionIdx, 0));
    UINT           requestIdIndex = (pPipelineData->seqId % MaxOutstandingRequests);
    UINT32         frameIndex     = (frameNumber % MaxOutstandingRequests);

    CHISTREAMBUFFER         inputBuffer         = { 0 };
    // Create copy so that future updates to input metadata don't free it
    ChiMetadata* pRequestMetadata      = m_pSnapshotInputMeta[frameIndex];
    ChiMetadata* pRDIMeta              = NULL;
    ChiMetadata* pChiSnapshotInputMeta = NULL;
    ChiMetadata* pOutputMeta           = NULL;

    CHIBufferManager* pInputBufferManager = NULL;
    pPipelineData->seqIdToFrameNum[requestIdIndex] = frameNumber;

    if ((TRUE == m_pUsecase->IsMultiCameraUsecase()) &&
        (sessionIdx == m_bayer2YuvSessionId))
    {
        // here to get RDI buffer from external Queue
        m_pUsecase->GetInputBufferFromRDIQueue((inputPipelineReqId + bufferIndex),
                                               m_activePipelineID,
                                               bufferIndex,
                                               &inputBuffer,
                                               &pRDIMeta,
                                               TRUE);

        CHX_LOG("Get RDI Buffer reqId:%d, masterCameraId:%d, activePipelineID:%d, buffer type:%d handle:%p, meta:%p,stream:%p",
                inputPipelineReqId,
                m_masterCameraId,
                m_activePipelineID,
                inputBuffer.bufferInfo.bufferType,
                inputBuffer.bufferInfo.phBuffer,
                pRDIMeta,
                inputBuffer.pStream);

        pInputBufferManager = m_pUsecase->GetBufferManager(m_activePipelineID);
        // For multicamera usecase use RDI meta directly
        pChiSnapshotInputMeta = pRDIMeta;
    }
    else
    {
        if (m_bayer2YuvSessionId == sessionIdx)
        {
            CHX_LOG_INFO("waiting for rdi and meta of frame %d for triggering snapshot request", inputPipelineReqId);

            result = m_pUsecase->WaitForBufferMetaReady(inputPipelineReqId, m_rdiStreamIndex);

            if (CDKResultSuccess == result)
            {
                m_pUsecase->GetTargetBuffer(inputPipelineReqId,
                                            pInputTargetBuffer,
                                            pRequestMetadata,
                                            &inputBuffer,
                                            &pChiSnapshotInputMeta);

                if (NULL != pChiSnapshotInputMeta)
                {
                    pChiSnapshotInputMeta->AddReference();
                }

                if (NULL != pRequestMetadata)
                {
                    pRequestMetadata->ReleaseReference();
                }
                // Dynamically decrementing ValidBufferLength by one during SWMF so less RDI buffers will be reserved in the queue.
                // It will be increased once the input RDIs are released after receiving Bayer2YUV result.
                //m_pUsecase->UpdateValidRDIBufferLength(m_rdiStreamIndex, -1);
            }
            else
            {
                CHX_LOG_ERROR("FeatureSWMF: wait rdi and meta timeout! frameNumber=%d, inputPipelineReqId=%d", frameNumber, inputPipelineReqId);
            }
        }
        else
        {
            m_pUsecase->GetTargetBuffer(frameNumber,
                                        pInputTargetBuffer,
                                        pRequestMetadata,
                                        &inputBuffer,
                                        &pChiSnapshotInputMeta);

            if (NULL != pRequestMetadata)
            {
                pRequestMetadata->ReleaseReference();
            }
        }
        pInputBufferManager = pInputTargetBuffer->pBufferManager;
    }

    if (CDKResultSuccess == result)
    {
        if (m_snapshotSessionId == sessionIdx)
        {
            CHX_LOG("Override input stream to JPEGInputStream:%p", m_pJPEGInputStream);
            inputBuffer.pStream = m_pJPEGInputStream;
        }

        pOutputMeta = m_pMetadataManager->Get(pSession->GetMetadataClientId(), pPipelineData->seqId);
        if ((NULL == pChiSnapshotInputMeta) || (NULL == pOutputMeta))
        {
            CHX_LOG_ERROR("Metadata not available %p %p", pChiSnapshotInputMeta, pOutputMeta);
            return;
        }

        CHICAPTUREREQUEST request = { 0 };
        request.frameNumber       = pPipelineData->seqId++;
        request.hPipelineHandle   = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
        request.numInputs         = 1;
        request.numOutputs        = m_snapshotBufferNum[frameIndex];
        request.pInputBuffers     = &inputBuffer;
        request.pOutputBuffers    = pOutputBuffer;
        request.pInputMetadata    = pChiSnapshotInputMeta->GetHandle();
        request.pOutputMetadata   = pOutputMeta->GetHandle();

        // Debug-data deep-copy
        if (TRUE == ChxUtils::IsVendorTagPresent(pChiSnapshotInputMeta, DebugDataTag))
        {
            CHAR* pData = NULL;
            ChxUtils::GetVendorTagValue(pChiSnapshotInputMeta, DebugDataTag, (VOID**)&pData);
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
                        // This condition is only for protection in case debug-data size is change in run time while camera is
                        // already processing. This is not allow for this property at this time.
                        CHX_FREE(m_debugDataOffline.pData);
                        m_debugDataOffline.pData = NULL;
                        m_debugDataOffline.size  = 0;

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

                    if ((NULL != m_debugDataOffline.pData) &&
                        (m_debugDataOffline.pData != pDebug->pData)) // For snapshot same metadata buffer is use, so avoid copy.
                    {
                        ChxUtils::Memcpy(m_debugDataOffline.pData, pDebug->pData, pDebug->size);

                        result = ChxUtils::SetVendorTagValue(pChiSnapshotInputMeta,
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
            else
            {
                CHX_LOG_ERROR("DebugDataAll: Unable to get vendor tag data!");
                result = CDKResultEFailed;
            }
        }

        request.pPrivData                       = &m_offlinePrivData[request.frameNumber % MaxOutstandingRequests];
        request.pPrivData->featureType          = FeatureType::SWMF;

        // Save input buffers info for later releasing reference
        request.pPrivData->numInputBuffers      = 1;
        request.pPrivData->inputBuffers[0]      = inputBuffer.bufferInfo;
        request.pPrivData->bufferManagers[0]    = pInputBufferManager;
        request.pPrivData->streamIndex          = m_physicalCameraIndex;
        CHIPIPELINEREQUEST submitRequest    = { 0 };
        submitRequest.pSessionHandle        = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
        submitRequest.numRequests           = 1;
        submitRequest.pCaptureRequests      = &request;

        CHX_LOG_INFO("Sending SWMF request sessionIdx: %d frameNumber:%d, request:%" PRIu64 " metadata:%p",
                     sessionIdx, frameNumber, request.frameNumber, request.pMetadata);

        m_pUsecase->SetRequestToFeatureMapping(sessionIdx, request.frameNumber, this);

        if (((TRUE == ChxUtils::HasInputBufferError(&submitRequest)) ||
            (FlushStatus::NotFlushing != m_pUsecase->GetFlushStatus())) &&
            (FALSE == isBatched))
        {
            InvalidateRequest(&submitRequest);
        }
        else
        {
            m_pUsecase->LogFeatureRequestMappings(frameNumber, request.frameNumber, identifierData);
            m_pUsecase->SubmitRequest(&submitRequest);
        }
    }

    pRequestMetadata = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureMultiframe::RequestThread
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID* FeatureMultiframe::RequestThread(
    VOID* pThreadData)
{
    PerThreadData* pPerThreadData = reinterpret_cast<PerThreadData*>(pThreadData);
    FeatureMultiframe* pFeatureMultiFrame = reinterpret_cast<FeatureMultiframe*>(pPerThreadData->pPrivateData);

    pFeatureMultiFrame->RequestThreadProcessing();

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureMultiframe::RequestThreadProcessing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMultiframe::RequestThreadProcessing()
{
    CDKResult result = CDKResultSuccess;
    m_offlineThreadCaptureRequestSubmitId = 0;
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

        if (TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress)))
        {
            CHX_LOG_INFO("Destroy is in progress come out of loop");
            break;
        }
        while (m_offlineThreadCaptureRequestSubmitId < m_offlineThreadCaptureRequestId)
        {
            CHIPIPELINEREQUEST* pSubmitRequest = &m_offlineRequestData[m_offlineThreadCaptureRequestSubmitId % MaxMultiFrameRequests].submitRequest;
            result = m_pUsecase->SubmitRequest(pSubmitRequest);

            m_offlineRequestData[m_offlineThreadCaptureRequestSubmitId % MaxMultiFrameRequests].pRequestMetadata = NULL;
            CHX_LOG_INFO("RequestThreadProcessing for frame no: %d, result = %d",
                m_offlineThreadCaptureRequestSubmitId, result);
            m_offlineThreadCaptureRequestSubmitId++;
        }
    }
}
