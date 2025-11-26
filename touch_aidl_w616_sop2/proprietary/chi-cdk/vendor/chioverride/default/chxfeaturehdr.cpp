////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxfeaturehdr.cpp
/// @brief CHX HDR feature class implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "chxadvancedcamerausecase.h"
#include "chxincs.h"
#include "chxfeaturehdr.h"
#include "chxusecase.h"

// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureHDR::Create
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FeatureHDR* FeatureHDR::Create(
    AdvancedCameraUsecase* pUsecase,
    UINT32                 physicalCameraIndex)
{
    FeatureHDR* pFeature = CHX_NEW FeatureHDR;

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
/// FeatureHDR::Pause
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureHDR::Pause(
    BOOL isForced)
{
    CHX_LOG_CONFIG("Feature Pause isForced =%d", isForced);
    //Implement if required
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureHDR::Initialize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureHDR::Initialize(
    AdvancedCameraUsecase* pUsecase)
{
    CDKResult result = CDKResultSuccess;

    Feature::InitializePrivateResources();

    m_pUsecase          = pUsecase;

    m_pMergeYuvStream   = static_cast<CHISTREAM*>(CHX_CALLOC(sizeof(CHISTREAM)));

    for (UINT32 i = 0; i < MaxInputPorts; i++)
    {
        m_pMergePipelineInputStreams[i] = static_cast<CHISTREAM*>(CHX_CALLOC(sizeof(CHISTREAM)));
    }

    m_previewRawReqId       = 0;
    m_lastShutterFrameNum   = 0xFFFFFFFF;

    m_snapshotSessionId     = InvalidSessionId;
    m_mergeSessionId        = InvalidSessionId;
    m_bayer2YuvSessionId    = InvalidSessionId;
    m_previewSessionId      = InvalidSessionId;

    m_pBayer2YuvOutputBufferManager = NULL;
    m_pMergeOutputBufferManager     = NULL;

    m_pMetadataManager      = m_pUsecase->GetMetadataManager();

    ExtensionModule::GetInstance()->GetVendorTagOps(&m_vendorTagOps);

    m_numAeBracketFrames = 3;
    m_expValues[0] = -6;
    m_expValues[1] = 0;
    m_expValues[2] = 6;

    CHX_LOG_CONFIG("FeatureHDR m_pRdiStream: %p, m_pBayer2YuvStream: %p, m_pMergeYuvStream: %p, pGetMetaData:%p, pSetMetaData:%p. "
            "Using default m_numAeBracketFrames:%d, ev bracket (%d, %d, %d)",
            m_pRdiStream, m_pBayer2YuvStream,
            m_pMergeYuvStream, m_vendorTagOps.pGetMetaData,
            m_vendorTagOps.pSetMetaData, m_numAeBracketFrames,
            m_expValues[0], m_expValues[1], m_expValues[2]);

    for (UINT32 i = 0; i < MaxMultiFrames; i++)
    {
        // update with framework input keys
        m_pOverrideAppSetting[i] = ChiMetadata::Create(NULL, 0, true /*default keys*/, NULL);

        if (NULL == m_pOverrideAppSetting[i])
        {
            result = CDKResultENoMemory;
            break;
        }
    }

    return CDKResultSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureHDR::Destroy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureHDR::Destroy(BOOL isForced)
{
    CHX_LOG_INFO("FeatureHDR::Destroy(), isForced %d E.", isForced);

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

    for (UINT32 i = 0; i < MaxMultiFrames; i++)
    {
        if (NULL != m_pOverrideAppSetting[i])
        {
            m_pOverrideAppSetting[i]->Destroy();
            m_pOverrideAppSetting[i] = NULL;
        }
    }

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

    Feature::DestroyPrivateResources();

    CHX_DELETE(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureHDR::ConfigureTargetStream
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureHDR::ConfigureTargetStream()
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
/// FeatureHDR::OverrideUsecase
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ChiUsecase* FeatureHDR::OverrideUsecase(
    LogicalCameraInfo*              pCameraInfo,
    camera3_stream_configuration_t* pStreamConfig)
{
    (VOID)pCameraInfo;
    CHX_ASSERT(2 == pStreamConfig->num_streams);

    m_pChiUsecase           = m_pUsecase->GetChiUseCase();

    m_pPreviewStream        = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::PreviewStream);
    m_pRdiStream            = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::RdiStream, m_physicalCameraIndex);
    m_pFdStream             = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::FdStream, m_physicalCameraIndex);
    m_pSnapshotStream       = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::SnapshotStream);
    m_pBayer2YuvStream      = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::Bayer2YuvStream, m_physicalCameraIndex);
    m_pJPEGInputStream      = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::JPEGInputStream, m_physicalCameraIndex);

    m_snapshotPipelineIndex  = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                        AdvancedPipelineType::InternalZSLYuv2JpegType, m_physicalCameraIndex);
    m_mergePipelineIndex     = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                        AdvancedPipelineType::Merge3YuvCustomTo1YuvType, m_physicalCameraIndex);
    m_bayer2YuvPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                        AdvancedPipelineType::ZSLSnapshotYUVType, m_physicalCameraIndex);
    m_previewPipelineIndex   = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                        AdvancedPipelineType::ZSLPreviewRawType, m_physicalCameraIndex);

    CHX_LOG("OverrideUsecase for HDR: m_previewPipelineIndex:%d, m_bayer2YuvPipelineIndex:%d, m_mergePipelineIndex:%d, "
            "m_snapshotPipelineIndex:%d,",
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

    // HDR Stage-1 (BayerToYuv)
    pBayer2YuvDesc->sourceTarget.pTargetPortDesc[0].pTarget->pChiStream = m_pRdiStream;
    pBayer2YuvDesc->sinkTarget.pTargetPortDesc[0].pTarget->pChiStream   = m_pBayer2YuvStream;
    CHX_LOG_CONFIG("BayerToYuv sourceTarget stream type: %d, sinkTarget stream type: %d",
                  m_pRdiStream->streamType, m_pBayer2YuvStream->streamType);

    // HDR Stage-2 (Merge)
    ChxUtils::Memset(m_mergePipelineInputTargets, 0, sizeof(m_mergePipelineInputTargets));
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

    pMergeDesc->sinkTarget.pTargetPortDesc[0].pTarget->pChiStream   = m_pMergeYuvStream;

    CHX_LOG_CONFIG("Merge Pipeline source target: %d", pMergeDesc->sourceTarget.numTargets);

    // HDR Stage-3 (Jpeg)
    if (NULL != pSnapshotDesc)
    {
        pSnapshotDesc->sourceTarget.pTargetPortDesc[0].pTarget->pChiStream  = m_pJPEGInputStream;
        pSnapshotDesc->sinkTarget.pTargetPortDesc[0].pTarget->pChiStream    = m_pSnapshotStream;
    }

    CHX_LOG_CONFIG("OverrideUsecase for HDR RdiStream: %p, PreviewStream: %p, SnapshotStream: %p, Bayer2YuvStream: %p, "
                    "JPEGInputStream: %p",
                    m_pRdiStream,
                    m_pPreviewStream,
                    m_pSnapshotStream,
                    m_pBayer2YuvStream,
                    m_pJPEGInputStream);

    ConfigureTargetStream();

    return m_pChiUsecase;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureHDR::PipelineCreated
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureHDR::PipelineCreated(
    UINT32 sessionId,        ///< Id of session created
    UINT32 pipelineIndex)    ///< Index of the pipeline created (within the context of the session)
{
    ChiSensorModeInfo*              pSensorInfo     = NULL;
    const ChiPipelineInputOptions*  pInputOptions   = NULL;
    UINT                            pipelineId      =
        m_pUsecase->GetSessionData(sessionId)->pipelines[pipelineIndex].id;

    /// @todo need to find the mapping between session's pipelineId vs XMLs pipelineId
    pSensorInfo     = m_pUsecase->GetSessionData(sessionId)->pipelines[0].pPipeline->GetSensorModeInfo();
    pInputOptions   = m_pUsecase->GetSessionData(sessionId)->pipelines[0].pPipeline->GetInputOptions();

    /* Partial Meta data can come from multiple pipeline. The below check is
    to mark the final pipeline that will submit the Partial Meta data*/
    if (m_previewPipelineIndex == sessionId)
    {
        CHX_LOG_INFO("FeatureHDR: Partial Data will be sent from Session:%d Pipeline:%d", sessionId, pipelineIndex);
        m_pUsecase->SetFinalPipelineForPartialMetaData(sessionId);
    }

    CHX_LOG_CONFIG("HDR sessionId:%d, pipelineId: %d, Sensor width x height: %d x %d, "
                   "InputOptions width x height: %d x %d",
                    sessionId,
                    pipelineId,
                    pSensorInfo->frameDimension.width, pSensorInfo->frameDimension.height,
                    pInputOptions->bufferOptions.optimalDimension.width,
                    pInputOptions->bufferOptions.optimalDimension.height);
    CHX_LOG_CONFIG("snapshotstream resolution:%dx%d", m_pSnapshotStream->width, m_pSnapshotStream->height);
    // FEATURE_AEBRACKET_PORT
    /// @todo - Need to find means of associating grallocUsage
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
                m_pMergeOutputBufferManager = CHIBufferManager::Create("HDRMergeBufferManager", &createData);
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
            break;
        }
        case AdvancedPipelineType::Merge3YuvCustomTo1YuvType:
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

            createBayertoYuvData.width                  = m_pBayer2YuvStream->width;
            createBayertoYuvData.height                 = m_pBayer2YuvStream->height;
            createBayertoYuvData.format                 = ChiStreamFormatYCbCr420_888;
            createBayertoYuvData.producerFlags          = ChiGralloc1ProducerUsageCamera;
            createBayertoYuvData.consumerFlags          = ChiGralloc1ConsumerUsageCamera | ChiGralloc1ConsumerUsageCpuRead;
            createBayertoYuvData.maxBufferCount         = MinOutputBuffers;
            createBayertoYuvData.immediateBufferCount   = CHIImmediateBufferCountZSL;
            createBayertoYuvData.bEnableLateBinding     = ExtensionModule::GetInstance()->EnableCHILateBinding();
            createBayertoYuvData.bufferHeap             = BufferHeapDefault;
            createBayertoYuvData.pChiStream             = m_pBayer2YuvStream;

            if (NULL == m_pBayer2YuvOutputBufferManager)
            {
                m_pBayer2YuvOutputBufferManager = CHIBufferManager::Create("HDRBayertoYuvBufferManager", &createBayertoYuvData);
                if (NULL == m_pBayer2YuvOutputBufferManager)
                {
                    CHX_LOG_ERROR("m_pBayer2YuvOutputBufferManager is NULL");
                }
                else
                {
                    m_bayer2YuvTargetBuffers.pBufferManager = m_pBayer2YuvOutputBufferManager;
                    m_bayer2YuvTargetBuffers.pMutex         = Mutex::Create();

                    for (UINT queueIndex = 0; queueIndex < BufferQueueDepth; queueIndex++)
                    {
                        CHISTREAMBUFFER* pRdiOutputBuffer = static_cast<CHISTREAMBUFFER*>(CHX_CALLOC(sizeof(CHISTREAMBUFFER)));

                        m_bayer2YuvTargetBuffers.bufferQueue[queueIndex].pRdiOutputBuffer = pRdiOutputBuffer;

                        if (NULL != pRdiOutputBuffer)
                        {
                            pRdiOutputBuffer->size                  = sizeof(CHISTREAMBUFFER);
                            pRdiOutputBuffer->acquireFence.valid    = FALSE;
                            pRdiOutputBuffer->bufferInfo.phBuffer   = NULL;
                            pRdiOutputBuffer->pStream               = m_pBayer2YuvStream;
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
            CHX_LOG("FeatureHDR pipeline not used in HDR.");
            break;
        }
    }

    m_rdiStreamIndex   = m_pUsecase->GetInternalTargetBufferIndex(m_pRdiStream);
    m_fdStreamIndex    = m_pUsecase->GetInternalTargetBufferIndex(m_pFdStream);
    m_pRdiTargetBuffer = m_pUsecase->GetTargetBufferPointer(m_rdiStreamIndex);
    m_pFdTargetBuffer  = m_pUsecase->GetTargetBufferPointer(m_fdStreamIndex);

    CHX_LOG_INFO("rdiStreamIndex:%d, fdStreamIndex:%d",
        m_rdiStreamIndex, m_fdStreamIndex);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureHDR::ExecuteProcessRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult  FeatureHDR::ExecuteProcessRequest(
    camera3_capture_request_t* pRequest)              ///< Request parameters
{
    CDKResult                 result             = CDKResultSuccess;
    CHISTREAMBUFFER           previewBuffers[3]  = {{0}};
    UINT                      previewCount       = 0;
    UINT                      snapshotCount      = 0;
    UINT                      frameNumber        = pRequest->frame_number;
    UINT32                    frameIndex         = (pRequest->frame_number % MaxOutstandingRequests);
    UINT                      snapshotReqIdIndex = (m_maxSnapshotReqId % MaxOutstandingRequests);
    camera3_capture_result_t* pUsecaseResult     = m_pUsecase->GetCaptureResult(frameIndex);
    ChiMetadata*              pInputChiMetadata  = NULL;
    ChiMetadata*              pOutputChiMetadata = NULL;

    if (TRUE == m_pUsecase->IsMultiCameraUsecase())
    {
        RequestMapInfo requestMapInfo = m_pUsecase->GetRequestMapInfo(pRequest->frame_number);
        m_masterCameraId              = requestMapInfo.masterCameraID;
        m_activePipelineID            = requestMapInfo.activePipelineID;

        if (requestMapInfo.frameNumber != m_snapshotAppFrameNum)
        {
            CHX_LOG_ERROR("ERROR: AppFrameNumber does not match! requestMapInfo.frameNumber:%d, m_snapshotAppFrameNum:%d",
                          requestMapInfo.frameNumber, m_snapshotAppFrameNum);
            result = CDKResultEFailed;
        }

        // if INVALIDSEQUENCEID == m_internalFrameNum, it should be first request for this HDR capture
        if (INVALIDSEQUENCEID == m_internalFrameNum)
        {
            m_internalFrameNum = pRequest->frame_number + m_firstNormalExpIdx;
        }

        if (NULL != pRequest->output_buffers->buffer)
        {
            UINT32         appFrameNum = requestMapInfo.frameNumber;
            UINT32         appFrameIdx = (appFrameNum % MaxOutstandingRequests);

            ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[0], &m_snapshotBuffers[appFrameIdx][0]);
            m_snapshotBuffers[appFrameIdx][0].bufferInfo.bufferType = ChiNative;
        }
        CHX_LOG_INFO("FeatureHDR Snapshot Request for dual camera!, rtReqId:%d, AppFrameNum:%d, "
                "masterCameraId:%d, activePipelineID:%d, instance:%d",
                pRequest->frame_number, m_snapshotAppFrameNum, m_masterCameraId, m_activePipelineID, m_physicalCameraIndex);

        UINT32 sessionId = m_bayer2YuvSessionId;

        const PipelineData* pPipelineData  = m_pUsecase->GetPipelineData(sessionId, 0);
        UINT32              bayer2YuvReqId = pPipelineData->seqId;

        CHX_LOG("AppFrameNum:%d <--> bayer2YuvReqId:%d, input RDI request id:%d, sessionId:%d",
                m_snapshotAppFrameNum, bayer2YuvReqId, pRequest->frame_number, sessionId);

        CHISTREAMBUFFER outputBuffer = {0};
        UINT32          queueIndex   = bayer2YuvReqId % BufferQueueDepth;

        m_pBayer2YuvTargetBuffer->bufferQueue[queueIndex].pRdiOutputBuffer->bufferInfo =
            m_pBayer2YuvTargetBuffer->pBufferManager->GetImageBufferInfo();

        ChxUtils::Memcpy(&outputBuffer,
                         m_pBayer2YuvTargetBuffer->bufferQueue[queueIndex].pRdiOutputBuffer,
                         sizeof(CHISTREAMBUFFER));

        outputBuffer.acquireFence.valid = FALSE;
        outputBuffer.size               = sizeof(CHISTREAMBUFFER);

        SubmitRequest(sessionId, m_snapshotAppFrameNum, m_pRdiTargetBuffer,
                      &outputBuffer, pRequest->frame_number, FALSE, "HDR Bayer2YUV MultiCam request");

        return result;
    }

    CHX_LOG("FeatureHDR::ExecuteProcessRequest, frameNum:%d, setting:%p", pRequest->frame_number, pRequest->settings);

    pUsecaseResult->result             = NULL;
    pUsecaseResult->frame_number       = pRequest->frame_number;
    pUsecaseResult->num_output_buffers = 0;


    m_isSkipPreview[frameIndex]       = FALSE;
    m_isSnapshotFrame[frameIndex]     = FALSE;
    m_shutterTimestamp[frameIndex]    = 0;

    // get metadata
    pInputChiMetadata = m_pMetadataManager->GetInput(pRequest->settings, pRequest->frame_number);

    if (NULL == pInputChiMetadata)
    {
        CHX_LOG_ERROR("ERROR Cannot get input metadata buffer for frame %u metadata %p",
                      pRequest->frame_number,
                      pInputChiMetadata);

        return CDKResultEFailed;
    }

    UINT32 sensorModeIndex = m_pUsecase->GetSessionData(m_previewSessionId)->pSession->GetSensorModeInfo()->modeIndex;

    ChxUtils::FillTuningModeData(pInputChiMetadata,
                                 pRequest,
                                 sensorModeIndex,
                                 m_pUsecase->GetEffectMode(),
                                 m_pUsecase->GetSceneMode(),
                                 m_pUsecase->GetFeature1Mode(),
                                 m_pUsecase->GetFeature2Mode());

    ChxUtils::FillCameraId(pInputChiMetadata, m_pUsecase->GetCameraId());

    for (UINT32 i = 0; i < pRequest->num_output_buffers; i++)
    {
        ///< @todo (CAMX-123456789) Decouple CHISTREAM and camera3_stream
        if (m_pPreviewStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream))
        {
            // Capture preview stream
            ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i], &previewBuffers[previewCount]);
            previewCount++;
        }

        if (m_pSnapshotStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream))
        {
            if (FALSE == m_pUsecase->m_rejectedSnapshotRequestList[pRequest->frame_number % MaxOutstandingRequests])
            {
                m_pApplicationInputMeta = pInputChiMetadata;

                CHX_LOG_INFO("Snapshot Frame %d", pRequest->frame_number);
                ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i],
                                                       &m_snapshotBuffers[frameIndex][snapshotCount]);
                snapshotCount++;
            }
            else
            {
                CHX_LOG_INFO("Rejecting  only snapshot request for frame %d", pRequest->frame_number);
            }
        }
    }

    if (snapshotCount == 0)
    {
        PipelineData* pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_previewSessionId, 0));
        UINT          requestIdIndex = (pPipelineData->seqId % MaxOutstandingRequests);

        pPipelineData->seqIdToFrameNum[requestIdIndex] = frameNumber;

        CHX_LOG("FeatureHDR Realtime chiOverrideFrameNum to ReqId: %d <--> %d", frameNumber, pPipelineData->seqId);

        m_previewRawReqId = pPipelineData->seqId;

        result = m_pUsecase->GetOutputBufferFromRDIQueue(pPipelineData->seqId, m_rdiStreamIndex, &previewBuffers[previewCount]);

        if (CDKResultSuccess == result)
        {
            previewCount++;
        }
        else
        {
            CHX_LOG_ERROR("FeatureHDR: GetOutputBufferFromRDIQueue failed for frameNumber %d", frameNumber);
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
                CHX_LOG_ERROR("FeatureHDR: GetOutputBufferFromFDQueue failed, No FD buffer for frameNumber %d", frameNumber);
            }
        }

        pOutputChiMetadata  = m_pMetadataManager->Get(
            pPipelineData->pPipeline->GetMetadataClientId(),
            pPipelineData->seqId+1);

        if (NULL == pOutputChiMetadata)
        {
            CHX_LOG_ERROR("ERROR Cannot get output metadata buffer for frame %u metadata %p",
                          pRequest->frame_number,
                          pInputChiMetadata);
            return CDKResultEFailed;
        }

        const Session* pSession = m_pUsecase->GetSessionData(m_previewSessionId)->pSession;

        CHICAPTUREREQUEST request = { 0 };
        request.frameNumber       = pPipelineData->seqId++;
        request.hPipelineHandle   = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
        request.numOutputs        = previewCount;
        request.pOutputBuffers    = previewBuffers;
        request.pInputMetadata    = pInputChiMetadata->GetHandle();
        request.pOutputMetadata   = pOutputChiMetadata->GetHandle();

        request.pPrivData              = &m_realtimePrivData[request.frameNumber % MaxOutstandingRequests];
        request.pPrivData->featureType = FeatureType::HDR;

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
                    CHX_LOG_CONFIG("Setting pipeline activate flag for frame: %d", pRequest->frame_number);
                }
            }
        }

        m_pUsecase->LogFeatureRequestMappings(frameNumber, m_previewRawReqId, "HDR Preview request");

        result = m_pUsecase->SubmitRequest(&submitRequest);

        CHX_LOG("Preview/RDI request, appFrameNum:%d <--> real time pipeline ReqId:%d",
            pRequest->frame_number, m_previewRawReqId);
    }
    else
    {
        m_isSnapshotFrame[frameIndex]         = TRUE;
        m_snapshotBufferNum[frameIndex]       = snapshotCount;
        m_snapshotReqIdToFrameNum[frameIndex] = frameNumber;
        m_isSkipPreview[frameIndex]           = TRUE;
        m_numBayer2YuvFrames                  = 0;
        m_maxSnapshotReqId++;

        CHX_LOG("FeatureHDR Snapshot Request received, appFrameNum:%d frame index %d",
                pRequest->frame_number, frameIndex);
        SetFeatureStatus(FeatureStatus::BUSY);
        result           = GenerateAEBracketRequest(pRequest);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureHDR::isJPEGOutputRequired
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL FeatureHDR::isJPEGOutputRequired() const
{
    return m_pUsecase->IsJPEGSnapshotStream();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureHDR::ProcessResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureHDR::ProcessResult(
    CHICAPTURERESULT*   pResult,
    VOID*               pPrivateCallbackData)
{
    SessionPrivateData* pCbData               = static_cast<SessionPrivateData*>(pPrivateCallbackData);
    BOOL                isAppResultsAvailable = FALSE;
    UINT32              resultFrameNum        = pResult->frameworkFrameNum;
    UINT32              realtimeReqId         = 0;
    UINT32              offlineReqId          = 0;
    ChiMetadata*        pInputChiMetadata     = NULL;
    ChiMetadata*        pOutputChiMetadata    = NULL;

    if (m_previewSessionId != pCbData->sessionId)
    {
        offlineReqId   = pResult->frameworkFrameNum;
        resultFrameNum = m_pUsecase->GetChiFrameNumFromReqId(pCbData->sessionId, 0, offlineReqId);
    }
    else if (m_previewSessionId == pCbData->sessionId)
    {
        realtimeReqId  = pResult->frameworkFrameNum;
        resultFrameNum = m_pUsecase->GetChiFrameNumFromReqId(m_previewSessionId, 0, realtimeReqId);

        CHX_LOG("Realtime ReqId to AppFrameNum: %d <--> %d", pResult->frameworkFrameNum, resultFrameNum);
    }

    CHX_LOG("FeatureHDR ProcessResult resultFrameNum: %d, sessionId: %d, pResultMetadata: %p, numOutputBuffers: %d",
            resultFrameNum, pCbData->sessionId, pResult->pOutputMetadata, pResult->numOutputBuffers);

    UINT32                    resultFrameIndex = resultFrameNum % MaxOutstandingRequests;
    camera3_capture_result_t* pUsecaseResult   = m_pUsecase->GetCaptureResult(resultFrameIndex);

    pUsecaseResult->frame_number = resultFrameNum;

    // If result contain metadata and metadata has not been sent to framework
    if ((NULL != pResult->pOutputMetadata) && (NULL != pResult->pInputMetadata))
    {
        BOOL isResultMetadataAvailable = FALSE;

        // release input metadata
        pInputChiMetadata = m_pMetadataManager->GetMetadataFromHandle(
           pResult->pInputMetadata);

        CHX_ASSERT(NULL != pInputChiMetadata);

        if (ChiMetadataManager::InvalidClientId == pInputChiMetadata->GetClientId())
        {
            // internal metadata
            pInputChiMetadata->ReleaseReference();
        }
        else
        {
            m_pMetadataManager->Release(pInputChiMetadata);
        }

        // get output metadata
        pOutputChiMetadata = m_pMetadataManager->GetMetadataFromHandle(
           pResult->pOutputMetadata);

        CHX_ASSERT(NULL != pOutputChiMetadata);

        // update timestamp
        ChxUtils::UpdateTimeStamp(pOutputChiMetadata,
                                  (FALSE == m_pUsecase->IsMultiCameraUsecase())  ? m_shutterTimestamp[resultFrameIndex] :
                                  m_pUsecase->GetRequestShutterTimestamp(resultFrameNum) ,
                                  resultFrameNum);


        if (m_previewSessionId == pCbData->sessionId)
        {
            CHX_LOG("meta for Preview Pipeline, frame number:%d, meta:%p", resultFrameNum, pOutputChiMetadata);

            m_pUsecase->FillMetadataForRDIQueue(realtimeReqId, m_rdiStreamIndex, pOutputChiMetadata);
        }
        else if (m_bayer2YuvSessionId == pCbData->sessionId)
        {
            UINT32 bufferQueueIdx = (offlineReqId % BufferQueueDepth);

            CHX_LOG("meta for Bayer2Yuv Pipeline, frame number:%d, meta:%p", resultFrameNum, pOutputChiMetadata);

            m_pBayer2YuvTargetBuffer->bufferQueue[bufferQueueIdx].pMetadata   = pOutputChiMetadata;
            m_pBayer2YuvTargetBuffer->bufferQueue[bufferQueueIdx].frameNumber = pResult->frameworkFrameNum;
        }
        else if (m_mergeSessionId == pCbData->sessionId)
        {
            CHX_LOG("meta for Merge Pipeline, frame number:%d, meta:%p", resultFrameNum, pOutputChiMetadata);

            UINT32 queueIndex = (resultFrameNum % BufferQueueDepth);

            m_pMergeTargetBuffer->bufferQueue[queueIndex].pMetadata   = pOutputChiMetadata;
            m_pMergeTargetBuffer->bufferQueue[queueIndex].frameNumber = pResult->frameworkFrameNum;
            m_featureOutputMetaHandle = pResult->pOutputMetadata;
        }
        else if (m_snapshotSessionId == pCbData->sessionId)
        {
            m_pMetadataManager->Release(pOutputChiMetadata);
        }

        if (FALSE == m_pUsecase->IsMetadataSent(resultFrameIndex))
        {
            // Do Not wait for Snapshot frame metadata, Return Preview metadata back to fwk
            // If we wait for snapshot, and if it takes more time to process, we will block the preview.
            pUsecaseResult->partial_result = pResult->numPartialMetadata;
            isResultMetadataAvailable = TRUE;

            m_pUsecase->UpdateAppResultMetadata(pOutputChiMetadata,
                                                resultFrameIndex,
                                                m_pUsecase->GetMetadataClientIdFromPipeline(pCbData->sessionId, 0));

            m_pUsecase->SetMetadataAvailable(resultFrameIndex);
            isAppResultsAvailable = TRUE;

            CHX_LOG("returnmetadata HDR JPEG metadata %d", resultFrameNum);
        }
    }

    for (UINT32 j = 0; j < pResult->numOutputBuffers; j++)
    {
        // If our internal stream, copy the result into the target buffer to be consumed by the offline pipeline
        if (m_pRdiStream == pResult->pOutputBuffers[j].pStream)
        {
            CHX_LOG("FeatureHDR Received Rdi stream, reqid:%d, frame num:%d", realtimeReqId, resultFrameNum);

            m_pUsecase->UpdateBufferReadyForRDIQueue(realtimeReqId, m_rdiStreamIndex, TRUE);

            // If we have the requirements for the snapshot frame, kick it off immediately
            if (TRUE == m_isSnapshotFrame[resultFrameIndex])
            {
                UINT32              zslQueueIndex   = (realtimeReqId % BufferQueueDepth);
                TargetBufferInfo*   pBufferQueue    = &m_pRdiTargetBuffer->bufferQueue[zslQueueIndex];

                CHX_LOG("FeatureHDR send buffer to Bayer2YuvPipelineIndex, frame number:%d, buffer:%p, meta:%p", resultFrameNum,
                        pBufferQueue->pRdiOutputBuffer,
                        pBufferQueue->pMetadata);

                const PipelineData* pPipelineData   = m_pUsecase->GetPipelineData(m_bayer2YuvSessionId, 0);
                UINT32              bayer2YuvReqId  = pPipelineData->seqId;
                UINT32              queueIndex      = bayer2YuvReqId % BufferQueueDepth;

                CHX_LOG("AppFrameNum:%d <--> bayer2YuvReqId:%d, input RDI request id:%d",
                        resultFrameNum, bayer2YuvReqId, realtimeReqId);

                CHX_LOG("m_pBayer2YuvTargetBuffer:%p, pRdiOutputBuffer:%p",
                        m_pBayer2YuvTargetBuffer,
                        m_pBayer2YuvTargetBuffer->bufferQueue[queueIndex].pRdiOutputBuffer);

                CHISTREAMBUFFER outputBuffer = {0};

                m_pBayer2YuvTargetBuffer->bufferQueue[queueIndex].pRdiOutputBuffer->bufferInfo =
                    m_pBayer2YuvTargetBuffer->pBufferManager->GetImageBufferInfo();

                ChxUtils::Memcpy(&outputBuffer,
                                 m_pBayer2YuvTargetBuffer->bufferQueue[queueIndex].pRdiOutputBuffer,
                                 sizeof(CHISTREAMBUFFER));

                outputBuffer.acquireFence.valid = FALSE;
                outputBuffer.size               = sizeof(CHISTREAMBUFFER);

                SubmitRequest(m_bayer2YuvSessionId, resultFrameNum, m_pRdiTargetBuffer,
                              &outputBuffer, realtimeReqId, FALSE, "HDR Bayer2YUV request");
            }
        }
        else if (m_pFdStream == pResult->pOutputBuffers[j].pStream)
        {

            CHX_LOG("FeatureHDR Received FD stream, reqid:%d, frame num:%d", realtimeReqId, resultFrameNum);

            m_pUsecase->UpdateBufferReadyForFDQueue(realtimeReqId, m_fdStreamIndex, TRUE);

        }
        else if (m_pBayer2YuvStream == pResult->pOutputBuffers[j].pStream)
        {
            CHX_LOG("FeatureHDR Received Bayer2Yuv stream, reqid:%d, frame num:%d", offlineReqId, resultFrameNum);

            UINT32 bufferQueueIdx = (offlineReqId % BufferQueueDepth);

            ChxUtils::Memcpy(m_pBayer2YuvTargetBuffer->bufferQueue[bufferQueueIdx].pRdiOutputBuffer,
                             &pResult->pOutputBuffers[j],
                             sizeof(CHISTREAMBUFFER));

            m_numBayer2YuvFrames++;
            CHX_LOG("received bayer2Yuv frames:%d", m_numBayer2YuvFrames);

            // Release reference to the input buffers of the request to the Bayer2YuvPipeline
            m_pUsecase->ReleaseReferenceToInputBuffers(pResult->pPrivData);

            if (TRUE == m_pUsecase->IsMultiCameraUsecase())
            {
                m_pUsecase->UpdateValidRDIBufferLength(m_activePipelineID, 1);
            }

            // submit request to merge pipeline until we got all buffers and frames.
            // because meta alway comes before or along with frames, so here it's sure meta is received.
            if ((TRUE == m_isSnapshotFrame[resultFrameIndex]) && (m_numBayer2YuvFrames == m_numAeBracketFrames))
            {
                CHX_LOG("FeatureHDR received all bayer2Yuv Frames, send to merge pipeline..");

                CHISTREAMBUFFER outputBuffer    = {0};
                UINT32          queueIndex      = resultFrameNum % BufferQueueDepth;

                if (TRUE == isJPEGOutputRequired())
                {
                    m_pMergeTargetBuffer->bufferQueue[queueIndex].pRdiOutputBuffer->bufferInfo =
                        m_pMergeTargetBuffer->pBufferManager->GetImageBufferInfo();
                }
                else
                {
                    m_pMergeTargetBuffer->bufferQueue[queueIndex].pRdiOutputBuffer->bufferInfo =
                        m_snapshotBuffers[resultFrameIndex][0].bufferInfo;
                }
                ChxUtils::Memcpy(&outputBuffer,
                                 m_pMergeTargetBuffer->bufferQueue[queueIndex].pRdiOutputBuffer,
                                 sizeof(CHISTREAMBUFFER));

                outputBuffer.acquireFence.valid = FALSE;
                outputBuffer.size               = sizeof(CHISTREAMBUFFER);

                // SubmitRequest(MergePipelineIndex, resultFrameNum, m_pBayer2YuvTargetBuffer,
                //              &outputBuffer, offlineReqId);
                {
                    PipelineData*       pPipelineData               =
                        const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_mergeSessionId, 0));
                    UINT                requestIdIndex              = (pPipelineData->seqId % MaxOutstandingRequests);
                    UINT32              frameIndex                  = (resultFrameNum % MaxOutstandingRequests);
                    CHISTREAMBUFFER     inputBuffer[MaxMultiFrames] = { { 0 } };
                    ChiMetadata*        pBayer2YUVInputMeta[MaxMultiFrames];
                    ChiMetadata*        pBayer2YUVOutputMeta;

                    pPipelineData->seqIdToFrameNum[requestIdIndex] = resultFrameNum;

                    // find the first ReqId for this snapshot
                    UINT32 firstReqId = offlineReqId - m_numBayer2YuvFrames + 1;
                    CHX_LOG("firstReqId:%d", firstReqId);

                    CHICAPTUREREQUEST request = { 0 };

                    request.pPrivData               = &m_offlinePrivData[request.frameNumber % MaxOutstandingRequests];
                    request.pPrivData->featureType  = FeatureType::HDR;
                    request.pPrivData->streamIndex  = m_physicalCameraIndex;
                    for (UINT32 i = 0; i < m_numBayer2YuvFrames; i++)
                    {
                        CHX_LOG("%d, get input buffer from reqid:%d", i, firstReqId + i);

                        // check if application metadata needs to be copied again?

                        m_pUsecase->GetTargetBuffer(firstReqId + i,
                                                    m_pBayer2YuvTargetBuffer,
                                                    NULL,
                                                    &inputBuffer[i],
                                                    &pBayer2YUVInputMeta[i]);

                        if (m_firstNormalExpIdx != i)
                        {
                            m_pMetadataManager->Release(pBayer2YUVInputMeta[i]);
                        }

                        inputBuffer[i].pStream                  = m_pMergePipelineInputStreams[i];
                        request.pPrivData->bufferManagers[i]    = m_pBayer2YuvTargetBuffer->pBufferManager;
                        request.pPrivData->inputBuffers[i]      = inputBuffer[i].bufferInfo;
                    }
                    request.pPrivData->numInputBuffers = m_numBayer2YuvFrames;

                    // Get output metadata
                    pBayer2YUVOutputMeta = m_pMetadataManager->Get(pPipelineData->pPipeline->GetMetadataClientId(),
                                                                   pPipelineData->seqId);

                    if (NULL == pBayer2YUVOutputMeta)
                    {
                        CHX_LOG_ERROR("[CMB_ERROR] Cannot get output metadata for bayer2yuv");
                        return;
                    }

                    const Session*  pSession = m_pUsecase->GetSessionData(m_mergeSessionId)->pSession;

                    request.frameNumber       = pPipelineData->seqId++;
                    request.hPipelineHandle   = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
                    request.numInputs         = m_numBayer2YuvFrames;
                    request.numOutputs        = m_snapshotBufferNum[frameIndex];
                    request.pInputBuffers     = &inputBuffer[0];
                    request.pOutputBuffers    = &outputBuffer;
                    request.pInputMetadata    = pBayer2YUVInputMeta[m_firstNormalExpIdx]->GetHandle();
                    request.pOutputMetadata   = pBayer2YUVOutputMeta->GetHandle();

                    CHIPIPELINEREQUEST submitRequest    = { 0 };
                    submitRequest.pSessionHandle        = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
                    submitRequest.numRequests           = 1;
                    submitRequest.pCaptureRequests      = &request;

                    CHX_LOG_INFO(
                        "FeatureHDR Sending HDR request pipelineIdx: %d frameNumber:%d, request:%" PRIu64 " metadata:%p",
                        m_mergePipelineIndex, resultFrameNum, request.frameNumber, request.pMetadata);

                    m_pUsecase->SetRequestToFeatureMapping(m_mergeSessionId, request.frameNumber, this);

                    if ((TRUE == ChxUtils::HasInputBufferError(&submitRequest)) ||
                        (FlushStatus::NotFlushing != m_pUsecase->GetFlushStatus()))
                    {
                        InvalidateRequest(&submitRequest);
                    }
                    else
                    {
                        m_pUsecase->LogFeatureRequestMappings(resultFrameNum, request.frameNumber, "HDR Merge Request");
                        m_pUsecase->SubmitRequest(&submitRequest);
                    }
                }
            }
        }
        else if (m_pMergeYuvStream == pResult->pOutputBuffers[j].pStream)
        {
            CHX_LOG("FeatureHDR Received MergeYuvStream stream, reqid:%d, frame num:%d", offlineReqId, resultFrameNum);
            UINT32 queueIndex = (resultFrameNum % BufferQueueDepth);

            ChxUtils::Memcpy(m_pMergeTargetBuffer->bufferQueue[queueIndex].pRdiOutputBuffer,
                             &pResult->pOutputBuffers[j],
                             sizeof(CHISTREAMBUFFER));

            // Release reference to the input buffers of the request to the MergeYuvPipeline
            m_pUsecase->ReleaseReferenceToInputBuffers(pResult->pPrivData);

            // If we have the requirements for the snapshot frame, kick it off immediately
            if (TRUE == m_isSnapshotFrame[resultFrameIndex])
            {
                if (TRUE == isJPEGOutputRequired())
                {
                    SubmitRequest(m_snapshotSessionId, resultFrameNum, m_pMergeTargetBuffer,
                                  &m_snapshotBuffers[resultFrameNum % MaxOutstandingRequests][0], 0 /* resultFrameNum */,
                                  TRUE, "HDR JPEG snapshot");
                }
                else
                {
                    //todo process feature done
                    CHX_LOG_INFO("HDR yuv buffer is coming!:%p", m_featureOutputMetaHandle);
                    if (NULL == pResult->pOutputMetadata)
                    {
                        pResult->pOutputMetadata = m_featureOutputMetaHandle;
                    }
                    SetFeatureStatus(FeatureStatus::READY);
                    m_pUsecase->ProcessFeatureDone(m_internalFrameNum, this, pResult);
                    m_pUsecase->GetAppResultMutex()->Lock();
                    camera3_stream_buffer_t* pResultBuffer =
                        const_cast<camera3_stream_buffer_t*>(&pUsecaseResult->output_buffers[pUsecaseResult->num_output_buffers++]);

                    ChxUtils::PopulateChiToHALStreamBuffer(&pResult->pOutputBuffers[j], pResultBuffer);
                    m_pUsecase->GetAppResultMutex()->Unlock();
                    isAppResultsAvailable = TRUE;
                }
            }
        }
        // Otherwise queue a buffer as part of the normal result
        else
        {
            UINT32 queueIndex = (resultFrameNum % MaxOutstandingRequests);
            if (m_pSnapshotStream == pResult->pOutputBuffers[j].pStream)
            {
                CHX_LOG_INFO("FeatureHDR Received Final Snapshot(JPEG) stream, reqId:%d, frameNum:%d",
                             offlineReqId, resultFrameNum);

                // Release reference to the input buffers of the request to the SnapshotPipeline
                m_pUsecase->ReleaseReferenceToInputBuffers(pResult->pPrivData);
                SetFeatureStatus(FeatureStatus::READY);
            }

            m_pUsecase->GetAppResultMutex()->Lock();
            camera3_stream_buffer_t* pResultBuffer =
                const_cast<camera3_stream_buffer_t*>(&pUsecaseResult->output_buffers[pUsecaseResult->num_output_buffers++]);

            ChxUtils::PopulateChiToHALStreamBuffer(&pResult->pOutputBuffers[j], pResultBuffer);
            m_pUsecase->GetAppResultMutex()->Unlock();

            if ((m_pPreviewStream == pResult->pOutputBuffers[j].pStream) &&
                (TRUE             == m_isSkipPreview[queueIndex]))
            {
                CHX_LOG("Feature HDR, skip the preview for index: %d", queueIndex);
                m_isSkipPreview[queueIndex] = FALSE;
                ChxUtils::SkipPreviewFrame(pResultBuffer);
            }

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
        ((m_bayer2YuvSessionId  == pCbData->sessionId)                  ||
         (m_snapshotSessionId   == pCbData->sessionId)))
    {
        // Process debug-data only bayer2YUV & Snapshot sessions
        m_pUsecase->ProcessDebugData(pResult, pPrivateCallbackData, resultFrameNum);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureHDR::ProcessCHIPartialData
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureHDR::ProcessCHIPartialData(
    UINT32    frameNum,
    UINT32    sessionId)
{
    CAMX_UNREFERENCED_PARAM(frameNum);
    CAMX_UNREFERENCED_PARAM(sessionId);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureHDR::ProcessDriverPartialCaptureResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureHDR::ProcessDriverPartialCaptureResult(
    CHIPARTIALCAPTURERESULT*    pResult,
    VOID*                       pPrivateCallbackData)
{
    UINT32              resultFrameNum      = pResult->frameworkFrameNum;
    UINT32              realtimeReqId       = 0;
    UINT32              offlineReqId        = 0;
    UINT32              resultFrameIndex    = resultFrameNum % MaxOutstandingRequests;
    ChiMetadata*        pChiOutputMetadata  = NULL;

    SessionPrivateData*         pCbData     =
        static_cast<SessionPrivateData*>(pPrivateCallbackData);
    PartialResultSender         sender      =
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

    resultFrameIndex    = resultFrameNum % MaxOutstandingRequests;
    pChiOutputMetadata  = m_pMetadataManager->GetMetadataFromHandle(pResult->pPartialResultMetadata);

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
/// FeatureHDR::ProcessMessage
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureHDR::ProcessMessage(
    const CHIMESSAGEDESCRIPTOR* pMessageDescriptor,
    VOID*                       pPrivateCallbackData)
{

    if (TRUE == m_pUsecase->IsMultiCameraUsecase())
    {
        CHX_LOG("FeatureHDR Ignore shutter message for multi camera!");

        return;
    }

    if (ChiMessageTypeSof == pMessageDescriptor->messageType)
    {
        // SOF notifications are not sent to the HAL3 application
        CHX_LOG("FeatureHDR Chi Notify SOF frameNum %u framework frameNum %u, timestamp %" PRIu64,
            pMessageDescriptor->message.sofMessage.sofId,
            pMessageDescriptor->message.sofMessage.frameworkFrameNum,
            pMessageDescriptor->message.sofMessage.timestamp);
    }
    else if (ChiMessageTypeMetaBufferDone == pMessageDescriptor->messageType)
    {
        CHX_LOG("FeatureHDR MetaBuffer Done frameNum %u i/p metadata %p o/p metadata %p",
            pMessageDescriptor->message.metaBufferDoneMessage.frameworkFrameNum,
            pMessageDescriptor->message.metaBufferDoneMessage.inputMetabuffer,
            pMessageDescriptor->message.metaBufferDoneMessage.outputMetabuffer);
    }
    else
    {
        SessionPrivateData* pCbData = static_cast<SessionPrivateData*>(pPrivateCallbackData);
        BOOL                dropCb  = TRUE;

        if (m_previewSessionId != pCbData->sessionId)
        {
            return;
        }

        UINT32 rtResultId     = pMessageDescriptor->message.shutterMessage.frameworkFrameNum;
        UINT32 resultFrameNum = m_pUsecase->GetChiFrameNumFromReqId(pCbData->sessionId, 0, rtResultId);

        CHX_LOG("FeatureHDR Shutter Notify. ReqId:%d <-> AppFrameNum:%d, timestamp:%" PRIu64,
            pMessageDescriptor->message.shutterMessage.frameworkFrameNum,
            resultFrameNum,
            pMessageDescriptor->message.shutterMessage.timestamp);

        if (m_lastShutterFrameNum != 0xFFFFFFFF && resultFrameNum <= m_lastShutterFrameNum)
        {
            CHX_LOG("Current:%d, last:%d", resultFrameNum, m_lastShutterFrameNum);
            return;
        }

        m_lastShutterFrameNum = resultFrameNum;

        ChiMessageDescriptor* pOverrideMessageDescriptor = const_cast<ChiMessageDescriptor*>(pMessageDescriptor);
        pOverrideMessageDescriptor->message.shutterMessage.frameworkFrameNum = resultFrameNum;

        if (ChiMessageTypeShutter == pMessageDescriptor->messageType)
        {
            if (m_previewSessionId == pCbData->sessionId)
            {
                UINT32 frameNumIndex = (resultFrameNum % MaxOutstandingRequests);
                m_shutterTimestamp[frameNumIndex] = pMessageDescriptor->message.shutterMessage.timestamp;
                dropCb = FALSE;
            }
        }

        if (FALSE == dropCb)
        {
            m_pUsecase->ReturnFrameworkMessage((camera3_notify_msg_t*)pMessageDescriptor, m_pUsecase->GetCameraId());
        }
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureHDR::GetRequestInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult   FeatureHDR::GetRequestInfo(
    camera3_capture_request_t*  pRequest,
    FeatureRequestInfo*         pOutputRequests,
    FeatureRequestType          requestType)
{
    (VOID)requestType;

    CDKResult result = CDKResultSuccess;

    if (NULL == pOutputRequests)
    {
        CHX_LOG("pOutputRequests is NULL.");
        result = CDKResultEInvalidArg;
    }

    CHX_LOG("GetRequestInfo for AppFrameNum:%d", pRequest->frame_number);
    UINT32 frameIndex         = (pRequest->frame_number % MaxOutstandingRequests);

    CHX_LOG("FeatureHDR Request on Snapshot stream:%p", m_pSnapshotStream);

    // if come to here, it should be snapshot request, no need to check if snapshot stream is exited.
    // initialize some variables for this process.
    m_isSnapshotFrame[frameIndex]         = TRUE;
    m_snapshotBufferNum[frameIndex]       = 1;
    m_snapshotReqIdToFrameNum[frameIndex] = pRequest->frame_number;
    m_maxSnapshotReqId++;

    m_snapshotAppFrameNum = pRequest->frame_number;
    m_internalFrameNum   = INVALIDSEQUENCEID;
    m_numBayer2YuvFrames = 0;

    if (CDKResultSuccess == result)
    {
        result = GenerateAEBracketSettings(pRequest);
    }

    if (CDKResultSuccess == result)
    {
        pOutputRequests->numOfRequest = m_numAeBracketFrames;
        for (UINT32 i = 0; i < m_numAeBracketFrames; ++i)
        {
            if (m_firstNormalExpIdx == i)
            {
                pOutputRequests->isReturnResult[i] = TRUE;
            }
            else
            {
                pOutputRequests->isReturnResult[i] = FALSE;
            }
            ChxUtils::Memcpy(&pOutputRequests->request[i],
                    pRequest,
                    sizeof(camera3_capture_request));
            pOutputRequests->metadataInfo[i].pInputMetadata     = m_pOverrideAppSetting[i];
            pOutputRequests->metadataInfo[i].pOutputMetadata    = NULL;
        }

        CHX_LOG("m_numAeBracketFrames:%d, isReturnResult[%d]:%d",
            m_numAeBracketFrames, m_firstNormalExpIdx, pOutputRequests->isReturnResult[m_firstNormalExpIdx]);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureHDR::GetRequiredPipelines
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
INT32 FeatureHDR::GetRequiredPipelines(
    AdvancedPipelineType * pPipelines,
    INT32 size)
{
    INT32 count = 0;
    const INT32 MaxPipelineCount = 8;

    if (NULL != pPipelines && size >= MaxPipelineCount)
    {
        INT32 index = 0;
        AdvancedPipelineType pipelineGroup[1];
        UINT                 cameraId[MaxPipelineCount];
        UINT                 physicalCameraId = m_pUsecase->GetPhysicalCameraId(m_physicalCameraIndex);

        if (InvalidPhysicalCameraId != physicalCameraId)
        {
            if (TRUE == isJPEGOutputRequired())
            {
                pPipelines[index]    = AdvancedPipelineType::InternalZSLYuv2JpegType;
                pipelineGroup[0]     = pPipelines[index];
                cameraId[index]      = physicalCameraId;
                m_snapshotSessionId  = m_pUsecase->GetUniqueSessionId(&pipelineGroup[0], 1, m_physicalCameraIndex);
                index++;
            }

            pPipelines[index]    = AdvancedPipelineType::Merge3YuvCustomTo1YuvType;
            pipelineGroup[0]     = pPipelines[index];
            cameraId[index]      = physicalCameraId;
            m_mergeSessionId     = m_pUsecase->GetUniqueSessionId(&pipelineGroup[0], 1, m_physicalCameraIndex);
            index++;

            pPipelines[index]    = AdvancedPipelineType::ZSLSnapshotYUVType;
            pipelineGroup[0]     = pPipelines[index];
            cameraId[index]      = physicalCameraId;
            m_bayer2YuvSessionId = m_pUsecase->GetUniqueSessionId(&pipelineGroup[0], 1, m_physicalCameraIndex);
            index++;

            if (FALSE == m_pUsecase->IsMultiCameraUsecase())
            {
                pPipelines[index]    = AdvancedPipelineType::ZSLPreviewRawType;
                pipelineGroup[0]     = pPipelines[index];
                cameraId[index]      = physicalCameraId;
                m_previewSessionId   = m_pUsecase->GetUniqueSessionId(&pipelineGroup[0], 1, m_physicalCameraIndex);
                index++;
            }
            else
            {
                m_previewSessionId = InvalidSessionId;
                CHX_LOG_WARN("Don't use Realtime pipeline in advance usecase for multicamera usecase");
            }

            m_pUsecase->SetPipelineCameraId(&pPipelines[0], &cameraId[0], index);

            CHX_LOG("m_previewSessionId:%d, m_bayer2YuvSessionId:%d, m_mergeSessionId:%d, m_snapshotSessionId:%d",
                m_previewSessionId,
                m_bayer2YuvSessionId,
                m_mergeSessionId,
                m_snapshotSessionId);

            count = index;
        }
        else
        {
            CHX_LOG_ERROR("Invalid physical camera ID!");
        }
    }

    CHX_LOG("FeatureHDR::GetRequiredPipelines, required pipeline count:%d", count);
    return count;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureHDR::GetOutputBufferManager
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CHIBufferManager* FeatureHDR::GetOutputBufferManager(
    CHISTREAMBUFFER* pOutputBuffer)
{
    CHIBufferManager* result  = NULL;
    CHISTREAM* const  pStream = pOutputBuffer->pStream;
    if (m_pMergeYuvStream == pStream)
    {
        result = m_pMergeTargetBuffer->pBufferManager;
    }
    else if (m_pBayer2YuvStream == pStream)
    {
        result = m_pBayer2YuvTargetBuffer->pBufferManager;
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureHDR::SubmitRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureHDR::SubmitRequest(
    UINT32              sessionIdx,
    UINT                frameNumber,
    TargetBuffer*       pInputTargetBuffer,
    CHISTREAMBUFFER*    pOutputBuffer,
    UINT32              inputPipelineReqId,
    BOOL                canInvalidate,
    const CHAR*         identifierString)
{
    CDKResult       result         = CDKResultSuccess;
    const Session*  pSession       = m_pUsecase->GetSessionData(sessionIdx)->pSession;
    PipelineData*   pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(sessionIdx, 0));
    UINT            requestIdIndex = (pPipelineData->seqId % MaxOutstandingRequests);
    UINT32          frameIndex     = (frameNumber % MaxOutstandingRequests);
    CHISTREAMBUFFER inputBuffer = {};
    ChiMetadata*    pSrcMetadata   = NULL;
    ChiMetadata*    pDstMetadata   = NULL;

    pPipelineData->seqIdToFrameNum[requestIdIndex] = frameNumber;

    CHIBufferManager*   pInputBufferManager = NULL;
    if ((TRUE == m_pUsecase->IsMultiCameraUsecase()) &&
        (sessionIdx == m_bayer2YuvSessionId))
    {
        // here to get RDI buffer from external Queue
        m_pUsecase->GetInputBufferFromRDIQueue(inputPipelineReqId,
                                               m_activePipelineID,
                                               0,
                                               &inputBuffer,
                                               &pSrcMetadata,
                                               TRUE);

        CHX_LOG("Get RDI Buffer from RDIQueue, reqId:%d, masterCameraId:%d, activePipelineID:%d, buffer handle:%p, meta:%p",
                inputPipelineReqId, m_masterCameraId, m_activePipelineID, inputBuffer.bufferInfo.phBuffer, pSrcMetadata);
        pInputBufferManager = m_pUsecase->GetBufferManager(m_activePipelineID);
    }
    else
    {
        if (inputPipelineReqId != 0)
        {
            CHX_LOG("get input buffer from reqid:%d", inputPipelineReqId);
            m_pUsecase->GetTargetBuffer(inputPipelineReqId,
                                        pInputTargetBuffer,
                                        NULL,
                                        &inputBuffer,
                                        &pSrcMetadata);
        }
        else
        {
            m_pUsecase->GetTargetBuffer(frameNumber,
                                        pInputTargetBuffer,
                                        NULL,
                                        &inputBuffer,
                                        &pSrcMetadata);
        }
        pInputBufferManager = pInputTargetBuffer->pBufferManager;
    }

    if (m_snapshotSessionId == sessionIdx)
    {
        CHX_LOG("Override input stream to JPEGInputStream:%p", m_pJPEGInputStream);
        inputBuffer.pStream = m_pJPEGInputStream;
    }

    // get output metadata
    pDstMetadata = m_pMetadataManager->Get(pPipelineData->pPipeline->GetMetadataClientId(),
                                           pPipelineData->seqId);

    if ((NULL == pDstMetadata) || (NULL == pSrcMetadata))
    {
        CHX_LOG_ERROR("[CMB_ERROR] cannot get %s for session %d frame %u", (NULL == pDstMetadata)? "DstMetadata":"SrcMetadata", sessionIdx, pPipelineData->seqId);
        return;
    }

    CHICAPTUREREQUEST request = { 0 };
    request.frameNumber       = pPipelineData->seqId++;
    request.hPipelineHandle   = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
    request.numInputs         = 1;
    request.numOutputs        = m_snapshotBufferNum[frameIndex];
    request.pInputBuffers     = &inputBuffer;
    request.pOutputBuffers    = pOutputBuffer;
    request.pInputMetadata    = pSrcMetadata->GetHandle();
    request.pOutputMetadata   = pDstMetadata->GetHandle();

    request.pPrivData                       = &m_offlinePrivData[request.frameNumber % MaxOutstandingRequests];
    request.pPrivData->featureType          = FeatureType::HDR;

    request.pPrivData->bufferManagers[0]    = pInputBufferManager;
    request.pPrivData->inputBuffers[0]      = inputBuffer.bufferInfo;

    request.pPrivData->numInputBuffers      = 1;
    request.pPrivData->streamIndex          = m_physicalCameraIndex;

    CHIPIPELINEREQUEST submitRequest    = { 0 };
    submitRequest.pSessionHandle        = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
    submitRequest.numRequests           = 1;
    submitRequest.pCaptureRequests      = &request;

    // Debug-data deep-copy
    if (TRUE == ChxUtils::IsVendorTagPresent(pSrcMetadata, DebugDataTag))
    {
        CHAR* pData = NULL;
        ChxUtils::GetVendorTagValue(pSrcMetadata, DebugDataTag, (VOID**)&pData);
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

                    result = ChxUtils::SetVendorTagValue(pSrcMetadata,
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

    CHX_LOG_INFO("Sending HDR request sessionIdx: %d frameNumber:%d, request:%" PRIu64 " metadata:%p",
        sessionIdx, frameNumber, request.frameNumber, pSrcMetadata);

    m_pUsecase->SetRequestToFeatureMapping(sessionIdx, request.frameNumber, this);

    if (((TRUE == ChxUtils::HasInputBufferError(&submitRequest)) ||
         (FlushStatus::NotFlushing != m_pUsecase->GetFlushStatus())) &&
         (TRUE == canInvalidate))
    {
        InvalidateRequest(&submitRequest);
    }
    else
    {
        m_pUsecase->LogFeatureRequestMappings(frameNumber, request.frameNumber, identifierString);
        m_pUsecase->SubmitRequest(&submitRequest);
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureHDR::GenerateAEBracketRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureHDR::DumpMeta(ChiMetadata* pChiMeta, UINT index)
{
    CDK_UNUSED_PARAM(pChiMeta);
    CDK_UNUSED_PARAM(index);
#ifdef __HDR_DUMP_META__
    CHAR metaFileName[MaxFileLen];
    CdkUtils::SNPrintF(metaFileName, sizeof(metaFileName), "hdr_meta_%d.txt", index);
    m_pOverrideAppSetting[request]->DumpDetailsToFile(metaFileName);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureHDR::GenerateAEBracketRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureHDR::GenerateAEBracketRequest(
    camera3_capture_request_t* pRequest)
{
    CDKResult       result            = CDKResultSuccess;
    UINT8           sceneMode         = ANDROID_CONTROL_SCENE_MODE_HDR;
    INT32           expCompensation   = 0;
    UINT8           aeLock            = 1;
    UINT32          firstNormalExpIdx = 0;
    UINT32          outputCount       = 0;
    CHISTREAMBUFFER outputBuffers[3];
    CHAR            metaclientName[MaxFileLen];

    CHX_LOG("E. num frames for ae bracket:%d", m_numAeBracketFrames);

    // find the first normal exposure index, will use it as framework request,
    // also return it's shutter/meta to framework
    for (UINT32 request = 0; request < m_numAeBracketFrames; request++)
    {
        if (m_expValues[request] == 0)
        {
            firstNormalExpIdx = request;
            break;
        }
    }

    m_firstNormalExpIdx = (firstNormalExpIdx < m_numAeBracketFrames) ? firstNormalExpIdx : 0;

    PipelineData* pPipelineData = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_previewSessionId, 0));

    for (UINT32 request = 0; request < m_numAeBracketFrames; request++)
    {
        ChxUtils::Memset(outputBuffers, 0, sizeof(outputBuffers));
        outputCount = 0;

        UINT requestIdIndex = (pPipelineData->seqId % MaxOutstandingRequests);

        pPipelineData->seqIdToFrameNum[requestIdIndex] = pRequest->frame_number;

        CHX_LOG("Realtime AppFrameNum to ReqId: %d <--> %d", pRequest->frame_number, pPipelineData->seqId);

        m_previewRawReqId = pPipelineData->seqId;

        result = m_pUsecase->GetOutputBufferFromRDIQueue(pPipelineData->seqId, m_rdiStreamIndex, &outputBuffers[outputCount]);

        if (CDKResultSuccess == result)
        {
            outputCount++;
        }
        else
        {
            CHX_LOG_ERROR("FeatureHDR: GetOutputBufferFromRDIQueue failed for frameNumber %d", pRequest->frame_number);
        }

        if (TRUE == m_pUsecase->IsFDBuffersNeeded())
        {
            result = m_pUsecase->GetOutputBufferFromFDQueue(pPipelineData->seqId, m_fdStreamIndex, &outputBuffers[outputCount]);

            if (CDKResultSuccess == result)
            {
                outputCount++;
            }
            else
            {
                CHX_LOG_ERROR("FeatureHDR: GetOutputBufferFromFDQueue failed for frameNumber %d", pRequest->frame_number);
            }
        }

        // also add preview for ev +0 capture, if it is present in the request
        if ((firstNormalExpIdx == request) && (pRequest->num_output_buffers > 1)) {
            for (UINT32 i = 0; i < pRequest->num_output_buffers; i++)
            {
                if (m_pPreviewStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream))
                {
                    // Capture preview stream
                    ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i], &outputBuffers[outputCount]);

                    outputCount++;
                    break;
                }
            }
        }

        /// update setting for ae bracket: ae_lock / ev +0 / hdr_scene_mode
        expCompensation = m_expValues[request];

        ChiMetadata* pInputChiMetadata = m_pOverrideAppSetting[request];

        CdkUtils::SNPrintF(metaclientName, sizeof(metaclientName), "hdrIn_%d", request);

        pInputChiMetadata->Invalidate();

        pInputChiMetadata->Merge(*m_pApplicationInputMeta);

        pInputChiMetadata->SetTag(ANDROID_CONTROL_AE_EXPOSURE_COMPENSATION,
                                  &expCompensation,
                                  1);

        UINT8 AEBracketMode = 1;
        pInputChiMetadata->SetTag("org.codeaurora.qcamera3.ae_bracket",
                                  "mode",
                                  &AEBracketMode,
                                  1);

        DumpMeta(m_pOverrideAppSetting[request], request);

        ChiMetadata* pOutputChiMetadata = m_pMetadataManager->Get(pPipelineData->pPipeline->GetMetadataClientId(),
                                                                  pPipelineData->seqId);

        if (NULL == pOutputChiMetadata)
        {
            CHX_LOG_ERROR("ERROR Cannot get output metadata buffer for frame %u metadata %p",
                          pRequest->frame_number,
                          pOutputChiMetadata);
            return CDKResultEFailed;
        }

        CHX_LOG_CONFIG("AE Bracketing request [%d/%d], realtimeReqId:%d -> appFrameNum:%d, expCompensation:%d, "
                       "output buffers:%d",
                        request,
                        m_numAeBracketFrames,
                        m_previewRawReqId,
                        pRequest->frame_number,
                        expCompensation,
                        outputCount);

        const Session* pSession = m_pUsecase->GetSessionData(m_previewSessionId)->pSession;

        CHICAPTUREREQUEST realTimeRequest       = { 0 };
        realTimeRequest.frameNumber             = pPipelineData->seqId++;
        realTimeRequest.hPipelineHandle         = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
        realTimeRequest.numOutputs              = outputCount;
        realTimeRequest.pOutputBuffers          = outputBuffers;
        realTimeRequest.pInputMetadata          = pInputChiMetadata->GetHandle();
        realTimeRequest.pOutputMetadata         = pOutputChiMetadata->GetHandle();

        realTimeRequest.pPrivData               = &m_realtimePrivData[realTimeRequest.frameNumber % MaxOutstandingRequests];
        realTimeRequest.pPrivData->featureType  = FeatureType::HDR;

        CHIPIPELINEREQUEST submitRequest;
        submitRequest.pSessionHandle    = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
        submitRequest.numRequests       = 1;
        submitRequest.pCaptureRequests  = &realTimeRequest;

        m_pUsecase->SetRequestToFeatureMapping(m_previewSessionId, realTimeRequest.frameNumber, this);

        m_pUsecase->LogFeatureRequestMappings(pRequest->frame_number, realTimeRequest.frameNumber, "HDR AE Bracket Request");

        result = m_pUsecase->SubmitRequest(&submitRequest);
    }

    CHX_LOG("X. result:%d", result);
    return result;

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureHDR::GenerateAEBracketSettings
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureHDR::GenerateAEBracketSettings(camera3_capture_request_t * pRequest)
{
    CDKResult result            = CDKResultSuccess;
    UINT8     sceneMode         = ANDROID_CONTROL_SCENE_MODE_HDR;
    INT32     expCompensation   = 0;
    UINT8     aeLock            = 1;
    UINT32    firstNormalExpIdx = 0;
    UINT32    outputCount       = 0;
    (VOID)pRequest;

    CHX_LOG("GenerateAEBracketSettings E. num frames for ae bracket:%d", m_numAeBracketFrames);

    // find the first normal exposure index, will use it as framework request,
    // also return it's shutter/meta to framework
    for (UINT32 request = 0; request < m_numAeBracketFrames; request++)
    {
        if (m_expValues[request] == 0)
        {
            m_firstNormalExpIdx = request;
            CHX_LOG("m_firstNormalExpIdx: %d", m_firstNormalExpIdx);
            break;
        }
    }

    for (UINT32 request = 0; request < m_numAeBracketFrames; request++)
    {
        m_pOverrideAppSetting[request]->Invalidate();

        /// update setting for ae bracket: ae_lock / ev +0 / hdr_scene_mode
        expCompensation = m_expValues[request];

        m_pOverrideAppSetting[request]->SetTag(ANDROID_CONTROL_AE_EXPOSURE_COMPENSATION,
                                                &expCompensation,
                                                1);

        m_pOverrideAppSetting[request]->SetTag(ANDROID_CONTROL_AE_LOCK,
                                                &aeLock,
                                                sizeof(aeLock));

        UINT8 AEBracketMode = 1;
        m_pOverrideAppSetting[request]->SetTag("org.codeaurora.qcamera3.ae_bracket",
                                               "mode",
                                               &AEBracketMode,
                                               sizeof(AEBracketMode));

        DumpMeta(m_pOverrideAppSetting[request], request);

        CHX_LOG("AE Bracketing request [%d/%d], appFrameNum:%d, expCompensation:%d",
            request,
            m_numAeBracketFrames,
            pRequest->frame_number,
            expCompensation);
    }
    CHX_LOG("X. result:%d", result);
    return result;
}
