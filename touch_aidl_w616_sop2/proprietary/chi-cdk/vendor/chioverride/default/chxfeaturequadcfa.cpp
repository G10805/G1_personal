////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxfeaturequadcfa.cpp
/// @brief CHX QuadCFA feature class implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "chxadvancedcamerausecase.h"
#include "chxincs.h"
#include "chxfeaturequadcfa.h"
#include "chxusecase.h"

// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureQuadCFA::Create
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FeatureQuadCFA* FeatureQuadCFA::Create(
    AdvancedCameraUsecase* pUsecase)
{
    FeatureQuadCFA* pFeature = CHX_NEW FeatureQuadCFA;

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
/// FeatureQuadCFA::Pause
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureQuadCFA::Pause(
        BOOL isForced)
{
    CHX_LOG_INFO("FeatureQuadCFA::Pause(), isForced %d E.", isForced);
    ChxUtils::AtomicStoreU32(&m_aPauseInProgress, TRUE);

    if ((NULL != m_pFullSizeRawTargetBuffer) &&
        (NULL != m_pFullSizeRawTargetBuffer->pMutex) &&
        (NULL != m_pFullSizeRawTargetBuffer->pCondition))
    {
        m_pFullSizeRawTargetBuffer->pMutex->Lock();
        m_pFullSizeRawTargetBuffer->pCondition->Signal();
        m_pFullSizeRawTargetBuffer->pMutex->Unlock();
    }

    if ((NULL != m_pRemosaicRawTargetBuffer) &&
        (NULL != m_pRemosaicRawTargetBuffer->pMutex) &&
        (NULL != m_pRemosaicRawTargetBuffer->pCondition))
    {
        m_pRemosaicRawTargetBuffer->pMutex->Lock();
        m_pRemosaicRawTargetBuffer->pCondition->Signal();
        m_pRemosaicRawTargetBuffer->pMutex->Unlock();
    }

    if ((NULL != m_pBayer2YuvTargetBuffer) &&
        (NULL != m_pBayer2YuvTargetBuffer->pMutex) &&
        (NULL != m_pBayer2YuvTargetBuffer->pCondition))
    {
        m_pBayer2YuvTargetBuffer->pMutex->Lock();
        m_pBayer2YuvTargetBuffer->pCondition->Signal();
        m_pBayer2YuvTargetBuffer->pMutex->Unlock();
    }

    m_pSnapshotRDIResultMutex->Lock();
    m_pSnapshotRDIResultAvailable->Signal();
    m_pSnapshotRDIResultMutex->Unlock();

    m_pOfflineRequestMutex->Lock();
    m_offlineRequestProcessTerminate = TRUE;
    m_pOfflineRequestAvailable->Signal();
    m_pOfflineRequestMutex->Unlock();

    if (NULL != m_offlineRequestProcessThread.pPrivateData)
    {
        CHX_LOG("Terminating offline thread in FeatureQuadCFA");
        ChxUtils::ThreadTerminate(m_offlineRequestProcessThread.hThreadHandle);
        CHX_LOG("Terminated offline thread");
        m_offlineRequestProcessThread = { 0 };
    }
    CHX_LOG_INFO("FeatureQuadCFA::Pause(), isForced %d X.", isForced);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureQuadCFA::Initialize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureQuadCFA::Initialize(
    AdvancedCameraUsecase* pUsecase)
{
    CDKResult    result = CDKResultSuccess;

    Feature::InitializePrivateResources();

    m_pUsecase     = pUsecase;
    m_pResultMutex = Mutex::Create();

    m_lastRdiFrameAvailable          = InvalidFrameNum;
    m_lastRealtimeMetadataAvailable  = InvalidFrameNum;
    m_pSnapshotRDIResultMutex        = Mutex::Create();
    m_pSnapshotRDIResultAvailable    = Condition::Create();
    m_offlineRequestWaitingForFrame  = InvalidFrameNum;

    m_pOfflineRequestMutex           = Mutex::Create();
    m_pOfflineRequestAvailable       = Condition::Create();
    m_offlineRequestProcessTerminate = FALSE;
    m_aPauseInProgress               = FALSE;

    m_rdiStreamIndex                 = InvalidStreamIndex;
    m_fullSizeRawStreamIdx           = InvalidStreamIndex;
    m_remosaicRawStreamIdx           = InvalidStreamIndex;
    m_fdStreamIndex                  = InvalidStreamIndex;
    m_bayer2YuvStreamIdx             = InvalidStreamIndex;

    m_EmptyMetaData                  = ChxUtils::AndroidMetadata::AllocateMetaData(0, 0);
    m_pMetadataManager               = pUsecase->GetMetadataManager();

    CHX_LOG("m_pMetadataManager:%p", m_pMetadataManager);

    m_remosaicPipelineIndex          = InvalidPipelineIndex;
    m_bayer2YuvPipelineIndex         = InvalidPipelineIndex;
    m_snapshotPipelineIndex          = InvalidPipelineIndex;

    m_remosaicSessionId              = InvalidSessionId;
    m_bayer2YuvSessionId             = InvalidSessionId;
    m_snapshotSessionId              = InvalidSessionId;

    m_isYuvSnapshotFromApp           = FALSE;

    // currently jpeg is handled inside each feature, so set this to FALSE
    // when jpeg is movoed to a separate Feature, we can set this to TRUE
    m_isSeparateFeatureForJpeg       = FALSE;

    m_offlineRequestProcessThread.pPrivateData = this;

    if (NULL != m_pUsecase)
    {
        const LogicalCameraInfo* pCamInfo = m_pUsecase->GetLogicalCameraInfo();
        m_RemosaicType                    = GetRemosaicType(pCamInfo);

        if (CHIREMOSAICTYPE::UnKnown == m_RemosaicType)
        {
            // Legacy QCFA sensor drivers assume SW Remosaic Type,
            // so set default type as SW Remosaic Type.
            CHX_LOG_WARN("UnKnown remosaic type from driver, fallback to use SWRemosaic type");
            m_RemosaicType = CHIREMOSAICTYPE::SWRemosaic;
        }

        CHX_LOG("Remosaic type: %d", m_RemosaicType);
    }

    result = ChxUtils::ThreadCreate(FeatureQuadCFA::RequestThread,
                                    &m_offlineRequestProcessThread,
                                    &m_offlineRequestProcessThread.hThreadHandle);

    if (CDKResultSuccess != result)
    {
        CHX_LOG("Offline request thread create failed with result %d", result);
        m_offlineRequestProcessThread = { 0 };
    }

    CHX_LOG("m_physicalCameraIndex: %d", m_physicalCameraIndex);

    SetFeatureStatus(FeatureStatus::READY);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureQuadCFA::Destroy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureQuadCFA::Destroy(
    BOOL isForced)
{
    CHX_LOG("isForced %d", isForced);

    m_pSnapshotStream   = NULL;
    m_pPreviewStream    = NULL;
    m_pRdiStream        = NULL;
    m_pFdStream         = NULL;
    m_pVideoStream      = NULL;
    m_pJPEGInputStream  = NULL;

    if (NULL != m_pQcfaBayer2YuvStream)
    {
        CHX_FREE(m_pQcfaBayer2YuvStream);
        m_pQcfaBayer2YuvStream = NULL;
    }

    if (NULL != m_pFullSizeRawStream)
    {
        CHX_FREE(m_pFullSizeRawStream);
        m_pFullSizeRawStream = NULL;
    }

    if (NULL != m_pRemosaicRawStream)
    {
        CHX_FREE(m_pRemosaicRawStream);
        m_pRemosaicRawStream = NULL;
    }

    if (NULL != m_pDummyPreviewStream)
    {
        CHX_FREE(m_pDummyPreviewStream);
        m_pDummyPreviewStream = NULL;
    }

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

    if (NULL != m_pSnapshotRDIResultMutex)
    {
        m_pSnapshotRDIResultMutex->Destroy();
        m_pSnapshotRDIResultMutex = NULL;
    }

    if (NULL != m_pSnapshotRDIResultAvailable)
    {
        m_pSnapshotRDIResultAvailable->Destroy();
        m_pSnapshotRDIResultAvailable = NULL;
    }

    if (NULL != m_EmptyMetaData)
    {
        ChxUtils::AndroidMetadata::FreeMetaData(m_EmptyMetaData);
        m_EmptyMetaData = NULL;
    }

    DestroyLocalDebugData();
    Feature::DestroyPrivateResources();

    CHX_DELETE(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureQuadCFA::OverrideUsecase
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ChiUsecase* FeatureQuadCFA::OverrideUsecase(
    LogicalCameraInfo*              pCameraInfo,
    camera3_stream_configuration_t* pStreamConfig)
{
    (VOID)pCameraInfo;
    CHX_LOG("OverrideUsecase for FeatureQuadCFA, num_streams = %d", pStreamConfig->num_streams);

    m_pChiUsecase = m_pUsecase->GetChiUseCase();

    m_pPreviewStream        = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::PreviewStream);
    m_pRdiStream            = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::RdiStream, m_physicalCameraIndex);
    m_pSnapshotStream       = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::SnapshotStream);
    m_pFdStream             = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::FdStream, m_physicalCameraIndex);
    m_pVideoStream          = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::VideoStream);

    // allocate memory for streams only used in feature quad cfa
    m_pFullSizeRawStream    = static_cast<CHISTREAM*>(CHX_CALLOC(sizeof(CHISTREAM)));
    m_pRemosaicRawStream    = static_cast<CHISTREAM*>(CHX_CALLOC(sizeof(CHISTREAM)));
    m_pDummyPreviewStream   = static_cast<CHISTREAM*>(CHX_CALLOC(sizeof(CHISTREAM)));

    if ((NULL == m_pFullSizeRawStream) || (NULL == m_pRemosaicRawStream) || (NULL == m_pDummyPreviewStream))
    {
        CHX_LOG_ERROR("Fail to allocate memory for ChiStream");
        return NULL;
    }

    CHX_LOG("m_pFdStream:%p, m_pFullSizeRawStream:%p, m_pRemosaicRawStream: %p, m_pDummyPreviewStream:%p",
        m_pFdStream, m_pFullSizeRawStream, m_pRemosaicRawStream, m_pDummyPreviewStream);

    /// m_pPreviewStream is used for realtime preview pipeline.
    /// m_pDummyPreviewStream is used for full size raw pipeline.
    /// Don't use the same m_pPreviewStream for both preivew and fullsize raw pipeline, otherwise it will cause black preview.
    *m_pDummyPreviewStream  = *m_pPreviewStream;

    // setup bayer2yuv stream and JPEGInputStream for internal yuv usecase
    if (FALSE == IsYuvSnapshotFromAPP())
    {
        // For qcfa, bayer2yuv operates on full size raw, different from other features.
        // so here use separate pipeline and stream/target
        m_pQcfaBayer2YuvStream = static_cast<CHISTREAM*>(CHX_CALLOC(sizeof(CHISTREAM)));

        if (NULL == m_pQcfaBayer2YuvStream)
        {
            CHX_LOG_ERROR("Fail to allocate memory for ChiStream");
            return NULL;
        }

        if (TRUE == IsJPEGOutputRequired())
        {
            m_pJPEGInputStream  = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::JPEGInputStream, m_physicalCameraIndex);
        }
    }

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

    CHX_LOG("m_previewPipelineIndex:%d, m_continuousRdiCapture = %d", m_previewPipelineIndex, m_continuousRdiCapture);

    if (CHIREMOSAICTYPE::SWRemosaic == m_RemosaicType)
    {
        /// Setup input(s)/output(s) for pipeline: QuadCFARemosaic
        m_remosaicPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
            AdvancedPipelineType::QuadCFARemosaicType,
            m_physicalCameraIndex);
        CHX_LOG_INFO("m_remosaicPipelineIndex:%d", m_remosaicPipelineIndex);

        ChiPipelineTargetCreateDescriptor* pRemosaicDesc = &m_pChiUsecase->pPipelineTargetCreateDesc[m_remosaicPipelineIndex];
        pRemosaicDesc->sourceTarget.pTargetPortDesc[0].pTarget->pChiStream = m_pFullSizeRawStream;
        pRemosaicDesc->sinkTarget.pTargetPortDesc[0].pTarget->pChiStream   = m_pRemosaicRawStream;
    }

    ChiStream*                         pYuvOutStream = NULL;
    ChiPipelineTargetCreateDescriptor* pYuvDesc      = NULL;

    if (TRUE == IsYuvSnapshotFromAPP())
    {
        pYuvOutStream            = m_pSnapshotStream;
        m_snapshotPipelineIndex  = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                        AdvancedPipelineType::QuadCFASnapshotYuvType,
                                        m_physicalCameraIndex);
        pYuvDesc                 = &m_pChiUsecase->pPipelineTargetCreateDesc[m_snapshotPipelineIndex];
    }
    else
    {
        pYuvOutStream            = m_pQcfaBayer2YuvStream;
        m_bayer2YuvPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                        AdvancedPipelineType::QuadCFASnapshotYuvType,
                                        m_physicalCameraIndex);
        pYuvDesc                 = &m_pChiUsecase->pPipelineTargetCreateDesc[m_bayer2YuvPipelineIndex];
    }

    /// Setup input(s)/output(s) for pipeline: QuadCFASnapshotYuvType
    if (CHIREMOSAICTYPE::SWRemosaic == m_RemosaicType)
    {
        pYuvDesc->sourceTarget.pTargetPortDesc[0].pTarget->pChiStream = m_pRemosaicRawStream;
        pYuvDesc->sinkTarget.pTargetPortDesc[0].pTarget->pChiStream   = pYuvOutStream;
    }
    else if (CHIREMOSAICTYPE::HWRemosaic == m_RemosaicType)
    {
        pYuvDesc->sourceTarget.pTargetPortDesc[0].pTarget->pChiStream = m_pFullSizeRawStream;
        pYuvDesc->sinkTarget.pTargetPortDesc[0].pTarget->pChiStream   = pYuvOutStream;
    }

    /// Setup input(s)/output(s) for pipeline: InternalZSLYuv2JpegType
    if (TRUE == IsJPEGOutputRequired())
    {
        m_snapshotPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
            AdvancedPipelineType::InternalZSLYuv2JpegType,
            m_physicalCameraIndex);

        ChiPipelineTargetCreateDescriptor* pSnapshotDesc = &m_pChiUsecase->pPipelineTargetCreateDesc[m_snapshotPipelineIndex];

        pSnapshotDesc->sourceTarget.pTargetPortDesc[0].pTarget->pChiStream = m_pJPEGInputStream;
        pSnapshotDesc->sinkTarget.pTargetPortDesc[0].pTarget->pChiStream   = m_pSnapshotStream;
    }

    /// Setup input(s)/output(s) for pipeline: QuadCFAFullSizeRaw
    m_fullSizeRawPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
        AdvancedPipelineType::QuadCFAFullSizeRawType,
        m_physicalCameraIndex);
    CHX_LOG_INFO("m_fullSizeRawPipelineIndex:%d", m_fullSizeRawPipelineIndex);

    ChiPipelineTargetCreateDescriptor* pQuadCFAFullSizeRawDesc = &m_pChiUsecase->pPipelineTargetCreateDesc[m_fullSizeRawPipelineIndex];
    for (UINT i = 0; i < pQuadCFAFullSizeRawDesc->sinkTarget.numTargets; ++i)
    {
        ChiTarget* pTarget = pQuadCFAFullSizeRawDesc->sinkTarget.pTargetPortDesc[i].pTarget;

        if (ChiFormatRawMIPI == pTarget->pBufferFormats[0])
        {
            pTarget->pChiStream = m_pFullSizeRawStream;
        }
        else
        {
            pTarget->pChiStream = m_pDummyPreviewStream;
        }
    }

    // If app configured jpeg output, but jpeg pipeline is not present in this feature
    // setup bayer2yuv stream first before buffer negotiation
    if ((FALSE == IsYuvSnapshotFromAPP()) && (FALSE == IsJPEGOutputRequired()) && (NULL != m_pQcfaBayer2YuvStream))
    {
        // setup m_pQcfaBayer2YuvStream here as the final out stream of the feature
        m_pQcfaBayer2YuvStream->format           = ChiStreamFormatYCbCr420_888;
        m_pQcfaBayer2YuvStream->width            = m_pSnapshotStream->width;
        m_pQcfaBayer2YuvStream->height           = m_pSnapshotStream->height;
        m_pQcfaBayer2YuvStream->maxNumBuffers    = 0;
        m_pQcfaBayer2YuvStream->rotation         = StreamRotationCCW90;
        m_pQcfaBayer2YuvStream->streamType       = ChiStreamTypeOutput;
        m_pQcfaBayer2YuvStream->grallocUsage     = 0;

        CHX_LOG("m_pQcfaBayer2YuvStream:%p, wxh: %dx%d",
            m_pQcfaBayer2YuvStream, m_pQcfaBayer2YuvStream->width, m_pQcfaBayer2YuvStream->height);
    }

    return m_pChiUsecase;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureQuadCFA::PipelineCreated
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureQuadCFA::PipelineCreated(
    UINT sessionId,       ///< Id of session created
    UINT pipelineIndex)   ///< Pipeline of session created
{
    ChiSensorModeInfo*              pSensorInfo     = NULL;
    const ChiPipelineInputOptions*  pInputOptions   = NULL;
    UINT                            pipelineId      = 0;

    pipelineId      = m_pUsecase->GetSessionData(sessionId)->pipelines[pipelineIndex].id;
    pSensorInfo     = m_pUsecase->GetSessionData(sessionId)->pipelines[0].pPipeline->GetSensorModeInfo();
    pInputOptions   = m_pUsecase->GetSessionData(sessionId)->pipelines[0].pPipeline->GetInputOptions();

    CHX_LOG("FeatureQuadCFA sessionId:%d, pipeline index of session:%d, pipelineId:%d, pipelinetype:%d,"
            "Sensor widthxheight: %d x %d, InputOptions widthxheight: %d x %d",
            sessionId,
            pipelineIndex,
            pipelineId,
            m_pUsecase->GetAdvancedPipelineTypeByPipelineId(pipelineId),
            pSensorInfo->frameDimension.width,
            pSensorInfo->frameDimension.height,
            pInputOptions->bufferOptions.optimalDimension.width,
            pInputOptions->bufferOptions.optimalDimension.height);

    switch (m_pUsecase->GetAdvancedPipelineTypeByPipelineId(pipelineId))
    {
        case AdvancedPipelineType::InternalZSLYuv2JpegType:
        {
            m_pQcfaBayer2YuvStream->format           = ChiStreamFormatYCbCr420_888;
            m_pQcfaBayer2YuvStream->width            = pInputOptions->bufferOptions.optimalDimension.width;
            m_pQcfaBayer2YuvStream->height           = pInputOptions->bufferOptions.optimalDimension.height;
            m_pQcfaBayer2YuvStream->maxNumBuffers    = 0;
            m_pQcfaBayer2YuvStream->rotation         = StreamRotationCCW90;
            m_pQcfaBayer2YuvStream->streamType       = ChiStreamTypeOutput;
            m_pQcfaBayer2YuvStream->grallocUsage     = 0;

            // we must have valid m_pJPEGInputStream if jpeg pipeline is added
            m_pJPEGInputStream->format               = ChiStreamFormatYCbCr420_888;
            m_pJPEGInputStream->width                = pInputOptions->bufferOptions.optimalDimension.width;
            m_pJPEGInputStream->height               = pInputOptions->bufferOptions.optimalDimension.height;
            m_pJPEGInputStream->maxNumBuffers        = 0;
            m_pJPEGInputStream->rotation             = StreamRotationCCW90;
            m_pJPEGInputStream->streamType           = ChiStreamTypeInput;
            m_pJPEGInputStream->grallocUsage         = 0;

            CHX_LOG("m_pQcfaBayer2YuvStream:%p, wxh: %dx%d",
                m_pQcfaBayer2YuvStream, m_pQcfaBayer2YuvStream->width, m_pQcfaBayer2YuvStream->height);

            break;
        }

        case AdvancedPipelineType::QuadCFARemosaicType:
        {
            m_pFullSizeRawStream->format        = ChiStreamFormatRaw10;
            m_pFullSizeRawStream->width         = pSensorInfo->frameDimension.width;
            m_pFullSizeRawStream->height        = pSensorInfo->frameDimension.height;
            m_pFullSizeRawStream->maxNumBuffers = 0;
            m_pFullSizeRawStream->rotation      = StreamRotationCCW90;
            m_pFullSizeRawStream->streamType    = ChiStreamTypeOutput; // ChiStreamTypeInput;
            m_pFullSizeRawStream->grallocUsage  = 0;
            CHX_LOG_INFO("m_pFullSizeRawStream size:%dx%d", m_pFullSizeRawStream->width, m_pFullSizeRawStream->height);

            break;
        }

        case AdvancedPipelineType::QuadCFASnapshotYuvType:
        {
            if (CHIREMOSAICTYPE::SWRemosaic == m_RemosaicType)
            {
                m_pRemosaicRawStream->format        = ChiStreamFormatRaw16;
                m_pRemosaicRawStream->width         = pSensorInfo->frameDimension.width;
                m_pRemosaicRawStream->height        = pSensorInfo->frameDimension.height;
                m_pRemosaicRawStream->maxNumBuffers = 0;
                m_pRemosaicRawStream->rotation      = StreamRotationCCW90;
                m_pRemosaicRawStream->streamType    = ChiStreamTypeOutput; // ChiStreamTypeInput;
                m_pRemosaicRawStream->grallocUsage  = 0;
                CHX_LOG_INFO("m_pRemosaicRawStream size:%dx%d", m_pRemosaicRawStream->width, m_pRemosaicRawStream->height);
            }
            else if (CHIREMOSAICTYPE::HWRemosaic == m_RemosaicType)
            {
                m_pFullSizeRawStream->format        = ChiStreamFormatRaw10;
                m_pFullSizeRawStream->width         = pSensorInfo->frameDimension.width;
                m_pFullSizeRawStream->height        = pSensorInfo->frameDimension.height;
                m_pFullSizeRawStream->maxNumBuffers = 0;
                m_pFullSizeRawStream->rotation      = StreamRotationCCW90;
                m_pFullSizeRawStream->streamType    = ChiStreamTypeOutput; // ChiStreamTypeInput;
                m_pFullSizeRawStream->grallocUsage  = 0;

                CHX_LOG_INFO("m_pFullSizeRawStream size:%dx%d", m_pFullSizeRawStream->width, m_pFullSizeRawStream->height);
            }
            else
            {
                CHX_LOG_ERROR("should not configure chistream for ohter remosaic type in FeatureQuadCFA");
            }

            break;
        }


        case AdvancedPipelineType::QuadCFAFullSizeRawType:
        {
            CHX_LOG("pipeline QuadCFAFullSizeRaw created");
            break;
        }

        case AdvancedPipelineType::ZSLPreviewRawType:
        case AdvancedPipelineType::ZSLPreviewRawYUVType:
        {
            CHX_LOG("pipeline PreviewRaw created");
            break;
        }

        default:
        {
            break;
        }
    }

    if (InvalidStreamIndex == m_rdiStreamIndex)
    {
        m_rdiStreamIndex   = m_pUsecase->GetInternalTargetBufferIndex(m_pRdiStream);
        m_pRdiTargetBuffer = m_pUsecase->GetTargetBufferPointer(m_rdiStreamIndex);

        CHX_LOG_INFO("m_rdiStreamIndex:%d, m_pRdiTargetBuffer:%p", m_rdiStreamIndex, m_pRdiTargetBuffer);
    }

    if (InvalidStreamIndex == m_fullSizeRawStreamIdx)
    {
        m_fullSizeRawStreamIdx     = m_pUsecase->GetInternalTargetBufferIndex(m_pFullSizeRawStream);
        m_pFullSizeRawTargetBuffer = m_pUsecase->GetTargetBufferPointer(m_fullSizeRawStreamIdx);
        CHX_LOG_INFO("m_fullSizeRawStreamIdx:%d, m_pFullSizeRawTargetBuffer:%p",
            m_fullSizeRawStreamIdx, m_pFullSizeRawTargetBuffer);
    }

    if (InvalidStreamIndex == m_remosaicRawStreamIdx)
    {
        m_remosaicRawStreamIdx     = m_pUsecase->GetInternalTargetBufferIndex(m_pRemosaicRawStream);
        m_pRemosaicRawTargetBuffer = m_pUsecase->GetTargetBufferPointer(m_remosaicRawStreamIdx);
        CHX_LOG_INFO("m_remosaicRawStreamIdx:%d, m_pRemosaicRawTargetBuffer:%p",
            m_remosaicRawStreamIdx, m_pRemosaicRawTargetBuffer);
    }

    if (InvalidStreamIndex == m_fdStreamIndex)
    {
        m_fdStreamIndex   = m_pUsecase->GetInternalTargetBufferIndex(m_pFdStream);
        m_pFdTargetBuffer = m_pUsecase->GetTargetBufferPointer(m_fdStreamIndex);
        CHX_LOG("m_fdStreamIndex:%d, m_pFdTargetBuffer:%p", m_fdStreamIndex, m_pFdTargetBuffer);
    }

    // bayer2yuv buffer manager is not required if app requires YUV as snapshot output
    if ((FALSE == IsYuvSnapshotFromAPP()) && (InvalidStreamIndex == m_bayer2YuvStreamIdx))
    {
        m_bayer2YuvStreamIdx     = m_pUsecase->GetInternalTargetBufferIndex(m_pQcfaBayer2YuvStream);
        m_pBayer2YuvTargetBuffer = m_pUsecase->GetTargetBufferPointer(m_bayer2YuvStreamIdx);
        CHX_LOG_INFO("m_bayer2YuvStreamIdx:%d, m_pBayer2YuvTargetBuffer:%p",
            m_bayer2YuvStreamIdx, m_pBayer2YuvTargetBuffer);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureQuadCFA::HandlePreviewRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult  FeatureQuadCFA::HandlePreviewRequest(
    camera3_capture_request_t* pRequest)              ///< Request parameters
{
    CDKResult       result            = CDKResultSuccess;
    CHISTREAMBUFFER previewBuffers[3] = {{0}};
    UINT            previewCount      = 0;
    UINT            frameNumber       = pRequest->frame_number;
    UINT32          frameIndex        = (pRequest->frame_number % MaxOutstandingRequests);
    UINT32          sensorModeIndex   = 0;

    CHX_LOG("num_output_buffers:%d", pRequest->num_output_buffers);

    for (UINT32 i = 0; i < pRequest->num_output_buffers; i++)
    {
        if ((m_pPreviewStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream)) ||
            (m_pVideoStream   == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream)))
        {
            // Capture preview stream
            ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i], &previewBuffers[previewCount]);
            previewCount++;
        }
    }

    PipelineData* pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_previewSessionId, 0));
    UINT          requestIdIndex = (pPipelineData->seqId % MaxOutstandingRequests);

    pPipelineData->seqIdToFrameNum[requestIdIndex] = frameNumber;

    CHX_LOG("FeatureQuadCFA Realtime AppFrameNum to ReqId: %d <--> %d", frameNumber, pPipelineData->seqId);

    if (TRUE == m_continuousRdiCapture)
    {
        result = m_pUsecase->GetOutputBufferFromRDIQueue(pPipelineData->seqId, m_rdiStreamIndex, &previewBuffers[previewCount]);

        if (CDKResultSuccess == result)
        {
            previewCount++;
        }
        else
        {
            CHX_LOG_ERROR("FeatureQCFA: GetOutputBufferFromRDIQueue failed for frameNumber %d", frameNumber);
        }
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

    const Session* pSession = m_pUsecase->GetSessionData(m_previewSessionId)->pSession;

    if (FALSE == pSession->IsPipelineActive())
    {
        result = ExtensionModule::GetInstance()->ActivatePipeline(pSession->GetSessionHandle(),
            pSession->GetPipelineHandle());
        if (CDKResultSuccess == result)
        {
            pSession->SetPipelineActivateFlag();
        }
    }

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
    request.pInputMetadata    = pChiRTInputMetadata->GetHandle();

    ChiMetadata* pChiOutputMetadata = m_pMetadataManager->Get(pPipelineData->pPipeline->GetMetadataClientId(),
                                                              request.frameNumber);
    if (NULL == pChiOutputMetadata)
    {
        CHX_LOG_ERROR("Output metadata is NULL");
        return CDKResultEFailed;
    }
    request.pOutputMetadata   = pChiOutputMetadata->GetHandle();

    request.pPrivData              = &m_privData[request.frameNumber % MaxOutstandingRequests];
    request.pPrivData->featureType = this->GetFeatureType(); // FeatureType::QuadCFA;

    CHIPIPELINEREQUEST submitRequest = { 0 };
    submitRequest.pSessionHandle     = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
    submitRequest.numRequests        = 1;
    submitRequest.pCaptureRequests   = &request;

    result = m_pUsecase->SubmitRequest(&submitRequest);

    m_pUsecase->SetRequestToFeatureMapping(m_previewSessionId, request.frameNumber, this);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureQuadCFA::HandleSnapshotRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult  FeatureQuadCFA::HandleSnapshotRequest(
    camera3_capture_request_t* pRequest)              ///< Request parameters
{
    CDKResult result              = CDKResultSuccess;
    UINT      snapshotCount       = 0;
    UINT      frameNumber         = pRequest->frame_number;
    UINT32    frameIndex          = (pRequest->frame_number % MaxOutstandingRequests);
    UINT      snapshotReqIdIndex  = (m_maxSnapshotReqId % MaxOutstandingRequests);
    UINT32    sensorModeIndex     = 0;

    CHX_LOG_INFO("frame_number:%d, num_output_buffers:%d", pRequest->frame_number, pRequest->num_output_buffers);

    for (UINT32 i = 0; i < pRequest->num_output_buffers; i++)
    {
        if (m_pSnapshotStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream))
        {
            ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i],
                                                   &m_snapshotBuffers[snapshotReqIdIndex][snapshotCount]);
            snapshotCount++;

            CHX_LOG("Snapshot Frame %d", pRequest->frame_number);
        }
        else
        {
            // If preview is present in snapshot request, return preview buffer as error,
            // because for snapshot request, we have to use full size RDI,
            // and go through offline pipeline (remosaic) to generate preview buffer.
            // Since preview is reastarted as eraly as full size RDI is ready,
            // The preivew buffer in the snapshot request will come later than upcoming preview request, hence skip it.

            camera3_capture_result_t* pUsecaseResult = m_pUsecase->GetCaptureResult(frameIndex);
            camera3_stream_buffer_t*  pResultBuffer  =
                const_cast<camera3_stream_buffer_t*>(&pUsecaseResult->output_buffers[pUsecaseResult->num_output_buffers++]);

            ChxUtils::Memcpy(pResultBuffer, &pRequest->output_buffers[i], sizeof(camera3_stream_buffer_t));

            ChxUtils::SkipPreviewFrame(pResultBuffer);

            CHX_LOG_WARN("skip buffer:%p, stream:%p", pRequest->output_buffers[i].buffer, pRequest->output_buffers[i].stream);

            // close fence fd, otherwise causing fd leak
            if (pResultBuffer->acquire_fence > 0)
            {
                INT32 error = 0;

                CHX_LOG("Wait on acquireFence %d ", pResultBuffer->acquire_fence);

                result = ChxUtils::NativeFenceWait(pResultBuffer->acquire_fence, 5000);

                ChxUtils::Close(pResultBuffer->acquire_fence);

                pResultBuffer->acquire_fence = -1;
            }
        }
    }

    if ((CDKResultSuccess == result) && (0 < snapshotCount))
    {
        CHX_LOG("Snapshot in FeatureQuadCFA, m_isMultiFramesSnapshot:%d", m_isMultiFramesSnapshot);

        SetFeatureStatus(FeatureStatus::BUSY);

        if (TRUE == m_isMultiFramesSnapshot)
        {
            result = HandleMultiFramesSnapshot(pRequest);
        }
        else
        {
            result = HandleSingleFrameSnapshot(pRequest);
        }
    }

    CHX_LOG("X");
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureQuadCFA::HandleSingleFrameSnapshot
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureQuadCFA::HandleSingleFrameSnapshot(
    camera3_capture_request_t*  pRequest)
{
    CDKResult    result              = CDKResultSuccess;
    UINT         frameNumber         = pRequest->frame_number;
    UINT32       frameIndex          = (pRequest->frame_number % MaxOutstandingRequests);
    UINT         snapshotReqIdIndex  = (m_maxSnapshotReqId % MaxOutstandingRequests);
    UINT32       sensorModeIndex     = 0;
    ChiMetadata* pChiRTInputMetadata = NULL;

    CHX_LOG("E.");

    // notify usecase to reconfigure realtime pipeline to get full size snapshot raw
    result = StartFullsizeRDISession(FALSE);

    pChiRTInputMetadata = m_pMetadataManager->GetInput(pRequest->settings, pRequest->frame_number);
    if (NULL == pChiRTInputMetadata)
    {
        CHX_LOG_ERROR("Input metadata is NULL");
        result = CDKResultEFailed;
    }

    if (CDKResultSuccess == result)
    {
        result = GenerateFullsizeRDIRequest(pRequest, pChiRTInputMetadata);
    }

    if (CDKResultSuccess == result)
    {
        PipelineData* pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_fullSizeRawSessionId, 0));
        UINT snapshotRDISeqId        = (pPipelineData->seqId >= 1) ? (pPipelineData->seqId - 1) : (pPipelineData->seqId);

        result = WaitFullsizeRDIReady(snapshotRDISeqId);

        // send snapshot raw buffer to offline reprocess
        m_pOfflineRequestMutex->Lock();

        m_isSnapshotFrame[frameIndex]                 = TRUE;

        m_snapshotBufferNum[snapshotReqIdIndex]       = 1;
        m_snapshotReqIdToFrameNum[snapshotReqIdIndex] = frameNumber;
        m_snapshotInputNum[snapshotReqIdIndex]        = snapshotRDISeqId;
        m_pSnapshotInputMeta[snapshotReqIdIndex]      = pChiRTInputMetadata;

        CHX_LOG_INFO("Added input metadata for snapshot request index: %d", snapshotReqIdIndex);
        if (NULL != pChiRTInputMetadata)
        {
            pChiRTInputMetadata->AddReference();
        }

        CHX_LOG("notify offline thread, frame_number:%d, required rdi reqId:%d, snapshot request id: %d",
            frameNumber, m_snapshotInputNum[snapshotReqIdIndex], m_maxSnapshotReqId);

        m_maxSnapshotReqId++;

        m_pOfflineRequestAvailable->Signal();
        m_pOfflineRequestMutex->Unlock();

        // notify usecase to reconfigure realtime pipeline to restart preview
        result = ReStartPreviewSession(FALSE);
    }
    else
    {
        CHX_LOG_ERROR("Fail to sumbmit full size RDI request, frame number:%d, result:%d", pRequest->frame_number, result);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureQuadCFA::HandleMultiFramesSnapshot
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureQuadCFA::HandleMultiFramesSnapshot(
    camera3_capture_request_t*  pRequest)
{
    CDKResult            result              = CDKResultSuccess;
    UINT32               numRDIFrames        = 1;
    UINT32               sensorModeIndex     = 0;
    Feature*             pNextFeature        = NULL;
    ChiMetadata*         pChiRTInputMetadata = NULL;
    FeatureRequestInfo   requestInfo;

    CHX_LOG("E.");

    pNextFeature = m_pUsecase->GetNextFeatureForSnapshot(pRequest->frame_number, this);
    CHX_ASSERT(pNextFeature != NULL);

    ChxUtils::Memset(&requestInfo, 0, sizeof(FeatureRequestInfo));

    if (NULL != pNextFeature)
    {
        pNextFeature->GetRequestInfo(pRequest, &requestInfo, FeatureRequestType::COMMON);
    }

    CHX_LOG("Num RDI frames required:%d", requestInfo.numOfRequest);
    numRDIFrames = requestInfo.numOfRequest;

    // notify usecase to reconfigure realtime pipeline to get full size snapshot raw
    result = StartFullsizeRDISession(TRUE);

    for (UINT32 i = 0; i < numRDIFrames; i++)
    {
        ChiMetadata* pInputMetadata = NULL;

        if (NULL != requestInfo.metadataInfo[i].pInputMetadata)
        {
            pInputMetadata = requestInfo.metadataInfo[i].pInputMetadata;
            CHX_LOG("[%d] change request settings to:%p", i, pInputMetadata);
        }
        else
        {
            if (NULL == pChiRTInputMetadata)
            {
                pChiRTInputMetadata = m_pMetadataManager->GetInput(pRequest->settings, pRequest->frame_number);

                if (NULL == pChiRTInputMetadata)
                {
                    CHX_LOG_ERROR("Input metadata is NULL");
                    result = CDKResultEFailed;
                    break;
                }
            }
            else
            {
                pChiRTInputMetadata->AddReference();
            }

            pInputMetadata = pChiRTInputMetadata;
            CHX_LOG("[%d] input requet setting:%p", i, pInputMetadata);
        }

        result = GenerateFullsizeRDIRequest(pRequest, pInputMetadata);

        if (CDKResultSuccess != result)
        {
            break;
        }
    }

    if (CDKResultSuccess == result)
    {
        PipelineData* pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_fullSizeRawSessionId, 0));
        UINT snapshotRDISeqId        = (pPipelineData->seqId >= 1) ? (pPipelineData->seqId - 1) : (pPipelineData->seqId);

        result = WaitFullsizeRDIReady(snapshotRDISeqId);

        // set up input/output for next feature
        SnapshotFeatureInfo* pFeatureInfo = m_pUsecase->GetSnapshotFeatureInfo(pRequest->frame_number, pNextFeature);

        sensorModeIndex = m_pUsecase->GetSessionData(m_fullSizeRawSessionId)->pSession->GetSensorModeInfo()->modeIndex;

        if (NULL != pFeatureInfo)
        {
            pFeatureInfo->featureInput.inputBufferQIdx   = m_fullSizeRawStreamIdx;
            pFeatureInfo->featureInput.numOfInputBuffers = numRDIFrames;
            pFeatureInfo->featureInput.lastSeqId         = snapshotRDISeqId;
            pFeatureInfo->featureInput.sensorModeIndex   = sensorModeIndex;

            CHX_LOG("Prepare input/output for next feature. bufferQIndex:%d, num inputs:%d, lastSeqId:%d, sensorModeIndex:%d",
               pFeatureInfo->featureInput.inputBufferQIdx,
               pFeatureInfo->featureInput.numOfInputBuffers,
               pFeatureInfo->featureInput.lastSeqId,
               pFeatureInfo->featureInput.sensorModeIndex);
        }

        m_pUsecase->NotifyFeatureSnapshotDone(pRequest->frame_number, this, pRequest);

        // When received Feature snapshot done notification in advancedcamera usecase, it will call EPR to next feature.
        // For now, this function will be blocked until jpeg is done in next feature, and preview will stop during this time.


        // notify usecase to reconfigure realtime pipeline to restart preview
        CHX_LOG("Restarting preview now");
        result = ReStartPreviewSession(TRUE);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureQuadCFA::GenerateFullsizeRDIRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureQuadCFA::GenerateFullsizeRDIRequest(
    camera3_capture_request_t*  pRequest,
    ChiMetadata*                pRTInputMetadata)
{
    CDKResult result          = CDKResultSuccess;
    UINT      frameNumber     = pRequest->frame_number;
    UINT32    sensorModeIndex = 0;

    sensorModeIndex = m_pUsecase->GetSessionData(m_fullSizeRawSessionId)->pSession->GetSensorModeInfo()->modeIndex;
    ChxUtils::FillTuningModeData(pRTInputMetadata,
                                 pRequest,
                                 sensorModeIndex,
                                 m_pUsecase->GetEffectMode(),
                                 m_pUsecase->GetSceneMode(),
                                 m_pUsecase->GetFeature1Mode(),
                                 m_pUsecase->GetFeature2Mode());

    ChxUtils::FillCameraId(pRTInputMetadata, m_pUsecase->GetCameraId());

    CHISTREAMBUFFER outputBuffer = {0};
    if ((NULL != m_pFullSizeRawTargetBuffer) && (NULL != m_pFullSizeRawTargetBuffer->pBufferManager))
    {
        outputBuffer.bufferInfo     = m_pFullSizeRawTargetBuffer->pBufferManager->GetImageBufferInfo();
        outputBuffer.pStream        = m_pFullSizeRawStream;
        outputBuffer.releaseFence.valid = FALSE;
        outputBuffer.acquireFence.valid = FALSE;
        outputBuffer.size           = sizeof(CHISTREAMBUFFER);
    }
    else
    {
        CHX_LOG_ERROR("FeatureQuadCFA: GetImageBuffer failed, No RDI buffer for frameNumber %d", frameNumber);
    }

    PipelineData* pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_fullSizeRawSessionId, 0));
    UINT          requestIdIndex = (pPipelineData->seqId % MaxOutstandingRequests);

    pPipelineData->seqIdToFrameNum[requestIdIndex] = frameNumber;

    CHX_LOG("FeatureQuadCFA Realtime AppFrameNum to ReqId: %d <--> %d, output buffer handle:%p",
        frameNumber, pPipelineData->seqId, outputBuffer.bufferInfo.phBuffer);

    const Session* pSession = m_pUsecase->GetSessionData(m_fullSizeRawSessionId)->pSession;

    if (FALSE == pSession->IsPipelineActive())
    {
        result = ExtensionModule::GetInstance()->ActivatePipeline(pSession->GetSessionHandle(),
            pSession->GetPipelineHandle());
        if (CDKResultSuccess == result)
        {
            pSession->SetPipelineActivateFlag();
        }
    }

    CHICAPTUREREQUEST request = { 0 };
    request.frameNumber       = pPipelineData->seqId++;
    request.hPipelineHandle   = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
    request.numOutputs        = 1;
    request.pOutputBuffers    = &outputBuffer;

    if (NULL != pRTInputMetadata)
    {
        request.pInputMetadata = pRTInputMetadata->GetHandle();
    }
    else
    {
        CHX_LOG_ERROR("pRTInputMetadata is NULL, fallback to get from input setting.");
        ChiMetadata* pChiRTInputMetadata = m_pMetadataManager->GetInput(pRequest->settings, pRequest->frame_number);
        if (NULL == pChiRTInputMetadata)
        {
            CHX_LOG_ERROR("Input metadata is NULL");
            return CDKResultEFailed;
        }
        request.pInputMetadata = pChiRTInputMetadata->GetHandle();
    }

    ChiMetadata* pChiOutputMetadata = m_pMetadataManager->Get(pPipelineData->pPipeline->GetMetadataClientId(),
                                                              request.frameNumber);
    if (NULL == pChiOutputMetadata)
    {
        CHX_LOG_ERROR("Output metadata is NULL");
        return CDKResultEFailed;
    }
    request.pOutputMetadata        = pChiOutputMetadata->GetHandle();

    request.pPrivData              = &m_privData[request.frameNumber % MaxOutstandingRequests];
    request.pPrivData->featureType = this->GetFeatureType();

    CHIPIPELINEREQUEST submitRequest = { 0 };
    submitRequest.pSessionHandle     = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
    submitRequest.numRequests        = 1;
    submitRequest.pCaptureRequests   = &request;

    if ((FlushStatus::NotFlushing == m_pUsecase->GetFlushStatus()) &&
        (FALSE == ChxUtils::HasInputBufferError(&submitRequest)))
    {
        result = m_pUsecase->SubmitRequest(&submitRequest);

        m_pUsecase->SetRequestToFeatureMapping(m_fullSizeRawSessionId, request.frameNumber, this);
    }
    else
    {
        CHX_LOG_INFO("skip submit request, AppFrameNumber:%d, internal requestid:%" PRIu64 "",
            frameNumber, request.frameNumber);
        InvalidateRequest(&submitRequest);
        result = CDKResultEBusy;
    }

    return result;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureQuadCFA::GenerateRemosaicRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureQuadCFA::GenerateRemosaicRequest(
    UINT               frameNumber,
    UINT               requestIdIndex,
    UINT32             snapshotReqId,
    ChiMetadata*       pInputMetadata,
    ChiMetadata*       pOutputMetadata,
    CHISTREAMBUFFER*   pInputBuffer)
{
    CDKResult result       = CDKResultSuccess;
    CHISTREAMBUFFER outputBuffer = {0};

    outputBuffer.bufferInfo         = m_pRemosaicRawTargetBuffer->pBufferManager->GetImageBufferInfo();
    outputBuffer.pStream            = m_pRemosaicRawStream;
    outputBuffer.releaseFence.valid = FALSE;
    outputBuffer.acquireFence.valid = FALSE;
    outputBuffer.size               = sizeof(CHISTREAMBUFFER);

    const Session* pSession         = m_pUsecase->GetSessionData(m_remosaicSessionId)->pSession;
    PipelineData*  pPipelineData    = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_remosaicSessionId, 0));

    pPipelineData->seqIdToFrameNum[requestIdIndex] = frameNumber;

    CHX_LOG("FeatureQuadCFA SW Remosaic AppFrameNum to ReqId: %d <--> %d", frameNumber, pPipelineData->seqId);

    UpdateDebugDataPtr(pInputMetadata, FALSE);

    CHICAPTUREREQUEST remosaicRequest = { 0 };
    remosaicRequest.frameNumber       = snapshotReqId;
    remosaicRequest.hPipelineHandle   = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
    remosaicRequest.numInputs         = 1;
    remosaicRequest.numOutputs        = 1;
    remosaicRequest.pInputBuffers     = pInputBuffer;
    remosaicRequest.pOutputBuffers    = &outputBuffer;
    remosaicRequest.pInputMetadata    = pInputMetadata->GetHandle();
    remosaicRequest.pOutputMetadata   = pOutputMetadata->GetHandle();
    remosaicRequest.pPrivData         = &m_privData[remosaicRequest.frameNumber % MaxOutstandingRequests];
    remosaicRequest.pPrivData->featureType = this->GetFeatureType();

    CHX_LOG("remosaic reuqest inputbuffer:%p, stream:%p, outputbuffer:%p, stream:%p, meta:%p",
        pInputBuffer->bufferInfo.phBuffer,
        pInputBuffer->pStream,
        outputBuffer.bufferInfo.phBuffer,
        remosaicRequest.pOutputBuffers->pStream,
        remosaicRequest.pMetadata);

    // Save input buffers info for later releasing reference
    remosaicRequest.pPrivData->inputBuffers[0]      = pInputBuffer->bufferInfo;
    remosaicRequest.pPrivData->bufferManagers[0]    = m_pFullSizeRawTargetBuffer->pBufferManager;
    remosaicRequest.pPrivData->numInputBuffers      = 1;

    CHIPIPELINEREQUEST submitRequest = { 0 };
    submitRequest.pSessionHandle     = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
    submitRequest.numRequests        = 1;
    submitRequest.pCaptureRequests   = &remosaicRequest;

    if (FALSE == pSession->IsPipelineActive())
    {
        result = ExtensionModule::GetInstance()->ActivatePipeline(pSession->GetSessionHandle(),
            pSession->GetPipelineHandle());
        if (CDKResultSuccess == result)
        {
            pSession->SetPipelineActivateFlag();
        }
    }

    if ((FlushStatus::NotFlushing == m_pUsecase->GetFlushStatus()) &&
        (FALSE == ChxUtils::HasInputBufferError(&submitRequest)))
    {
        CHX_LOG_INFO("Sending QuadCFA SW remosaic frameNumber:%d, request:%" PRIu64 " metadata:%p",
                    frameNumber, remosaicRequest.frameNumber, pInputMetadata);
        m_pUsecase->SetRequestToFeatureMapping(m_remosaicSessionId, remosaicRequest.frameNumber, this);
        result = m_pUsecase->SubmitRequest(&submitRequest);
    }
    else
    {
        CHX_LOG_INFO("skip submit request, AppFrameNumber:%d, internal requestid:%" PRIu64 "",
            frameNumber, remosaicRequest.frameNumber);
        InvalidateRequest(&submitRequest);
        result = CDKResultEBusy;
    }

    return result;

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureQuadCFA::GenerateSnapshotJpegRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureQuadCFA::GenerateSnapshotJpegRequest(
    UINT               frameNumber,
    UINT               requestIdIndex,
    UINT32             snapshotReqId,
    ChiMetadata*       pInputMetadata,
    ChiMetadata*       pOutputMetadata,
    CHISTREAMBUFFER*   pInputBuffer,
    CHIBufferManager*  pInputBufferManager)
{
    CDKResult      result         = CDKResultSuccess;
    const Session* pSession       = m_pUsecase->GetSessionData(m_snapshotSessionId)->pSession;
    PipelineData*  pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_snapshotSessionId, 0));

    pPipelineData->seqIdToFrameNum[requestIdIndex] = frameNumber;

    CHX_LOG("FeatureQuadCFA SnapshotJpeg AppFrameNum to ReqId: %d <--> %d", frameNumber, pPipelineData->seqId);

    UpdateDebugDataPtr(pInputMetadata, FALSE);

    CHICAPTUREREQUEST snapshotRequest = { 0 };
    snapshotRequest.frameNumber       = snapshotReqId;
    snapshotRequest.hPipelineHandle   = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
    snapshotRequest.numInputs         = 1;
    snapshotRequest.numOutputs        = m_snapshotBufferNum[requestIdIndex];
    snapshotRequest.pInputBuffers     = pInputBuffer;
    snapshotRequest.pOutputBuffers    = m_snapshotBuffers[requestIdIndex];
    snapshotRequest.pInputMetadata    = pInputMetadata->GetHandle();
    snapshotRequest.pOutputMetadata   = pOutputMetadata->GetHandle();
    snapshotRequest.pPrivData         = &m_privData[snapshotRequest.frameNumber % MaxOutstandingRequests];
    snapshotRequest.pPrivData->featureType = this->GetFeatureType();

    CHX_LOG("snapshot reuqest inputbuffer:%p, stream:%p, outputbuffer:%p, stream:%p, meta:%p",
        pInputBuffer->bufferInfo.phBuffer,
        pInputBuffer->pStream,
        m_snapshotBuffers[requestIdIndex][0].bufferInfo.phBuffer,
        snapshotRequest.pOutputBuffers->pStream,
        snapshotRequest.pMetadata);

    // Save input buffers info for later releasing reference
    snapshotRequest.pPrivData->inputBuffers[0]      = pInputBuffer->bufferInfo;
    snapshotRequest.pPrivData->bufferManagers[0]    = pInputBufferManager;//
    snapshotRequest.pPrivData->numInputBuffers      = 1;

    CHIPIPELINEREQUEST submitRequest = { 0 };
    submitRequest.pSessionHandle     = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
    submitRequest.numRequests        = 1;
    submitRequest.pCaptureRequests   = &snapshotRequest;

    if (FALSE == pSession->IsPipelineActive())
    {
        result = ExtensionModule::GetInstance()->ActivatePipeline(pSession->GetSessionHandle(),
            pSession->GetPipelineHandle());
        if (CDKResultSuccess == result)
        {
            pSession->SetPipelineActivateFlag();
        }
    }

    if ((FlushStatus::NotFlushing == m_pUsecase->GetFlushStatus()) &&
        (FALSE == ChxUtils::HasInputBufferError(&submitRequest)))
    {
        CHX_LOG_INFO("Sending QuadCFA snapshot frameNumber:%d, request:%" PRIu64 " metadata:%p",
                    frameNumber, snapshotRequest.frameNumber, pInputMetadata);
        m_pUsecase->SetRequestToFeatureMapping(m_snapshotSessionId, snapshotRequest.frameNumber, this);
        result = m_pUsecase->SubmitRequest(&submitRequest);
    }
    else
    {
        CHX_LOG_INFO("skip submit request, AppFrameNumber:%d, internal requestid:%" PRIu64 "",
            frameNumber, snapshotRequest.frameNumber);
        InvalidateRequest(&submitRequest);
        result = CDKResultEBusy;
    }

    return result;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureQuadCFA::ExecuteProcessRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult  FeatureQuadCFA::ExecuteProcessRequest(
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

    m_isSnapshotFrame[frameIndex]     = FALSE;
    m_isSkipPreview[frameIndex]       = FALSE;

    m_snapshotInputNum[snapshotReqIdIndex] = 0;

    sensorModeIndex = m_pUsecase->GetSessionData(m_previewPipelineIndex)->pSession->GetSensorModeInfo()->modeIndex;
    ChxUtils::AndroidMetadata::FillTuningModeData(const_cast<camera_metadata_t*>(pRequest->settings),
                                 pRequest,
                                 sensorModeIndex,
                                 m_pUsecase->GetEffectMode(),
                                 m_pUsecase->GetSceneMode(),
                                 m_pUsecase->GetFeature1Mode(),
                                 m_pUsecase->GetFeature2Mode());

    ChxUtils::AndroidMetadata::FillCameraId(const_cast<camera_metadata_t*>(pRequest->settings), m_pUsecase->GetCameraId());

    if (TRUE == IsSnapshotRequest(pRequest))
    {
        m_snapshotAppFrameNumber = pRequest->frame_number;
        CHX_LOG("Snapshot Request, frameNumber:%d", m_snapshotAppFrameNumber);

        // For quadcfa full size snapshot,
        // 3a debug data is not available from full size rdi pipeline since we only submit one request,
        // so save a local debugdata copy from last preview result, and write this one to jpeg exif.
        UINT32       bufferIndex = m_pRdiTargetBuffer->lastReadySequenceID % BufferQueueDepth;
        ChiMetadata* pChiMeta    = m_pRdiTargetBuffer->bufferQueue[bufferIndex].pMetadata;
        SaveDebugDataToLocal(pChiMeta);

        Feature* pNextFeature = m_pUsecase->GetNextFeatureForSnapshot(pRequest->frame_number, this);
        CHX_LOG("cur feature:%p, next feature:%p", this, pNextFeature);

        if (NULL != pNextFeature)
        {
            m_isMultiFramesSnapshot = TRUE;
            CHX_LOG("Next Feature type:%d", pNextFeature->GetFeatureType());
        }
        else
        {
            m_isMultiFramesSnapshot = FALSE;
        }

        result = HandleSnapshotRequest(pRequest);
    }
    else
    {
        result = HandlePreviewRequest(pRequest);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureQuadCFA::ProcessResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureQuadCFA::ProcessResult(
    CHICAPTURERESULT*   pResult,
    VOID*               pPrivateCallbackData)
{
    SessionPrivateData* pCbData               = static_cast<SessionPrivateData*>(pPrivateCallbackData);
    BOOL                isAppResultsAvailable = FALSE;
    UINT32              resultFrameNum        = pResult->frameworkFrameNum;
    UINT32              resultFrameIndex      = resultFrameNum % MaxOutstandingRequests;
    UINT32              rtPipelineReqId       = 0;
    ChiMetadata*        pChiInputMetadata     = NULL;
    ChiMetadata*        pChiOutputMetadata    = NULL;

    m_pResultMutex->Lock();

    if ((m_snapshotSessionId == pCbData->sessionId) || (m_bayer2YuvSessionId == pCbData->sessionId))
    {
        resultFrameNum   = m_snapshotReqIdToFrameNum[pResult->frameworkFrameNum % MaxOutstandingRequests];
        resultFrameIndex = resultFrameNum % MaxOutstandingRequests;
        rtPipelineReqId  = pResult->frameworkFrameNum;

        CHX_LOG("Processing result for snapshot frame %d. Metadata: %p NumBuffers: %d Timestamp: %" PRIu64 " Sent: %d",
                resultFrameNum, pResult->pResultMetadata, pResult->numOutputBuffers,
                m_pUsecase->GetRequestShutterTimestamp(resultFrameNum),
                m_pUsecase->IsMetadataSent(resultFrameIndex));
    }
    else if (m_previewSessionId == pCbData->sessionId)
    {
        PipelineData*     pPipelineData =
            const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_previewSessionId, 0));

        resultFrameNum = pPipelineData->seqIdToFrameNum[pResult->frameworkFrameNum % MaxOutstandingRequests];
        resultFrameIndex = resultFrameNum % MaxOutstandingRequests;

        rtPipelineReqId = pResult->frameworkFrameNum;
        CHX_LOG("FeatureQuadCFA Realtime preview ReqId to AppFrameNum: %d <--> %d", pResult->frameworkFrameNum, resultFrameNum);
    }
    else if (m_fullSizeRawSessionId == pCbData->sessionId)
    {
        PipelineData*     pPipelineData =
            const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_fullSizeRawSessionId, 0));

        resultFrameNum = pPipelineData->seqIdToFrameNum[pResult->frameworkFrameNum % MaxOutstandingRequests];
        resultFrameIndex = resultFrameNum % MaxOutstandingRequests;

        rtPipelineReqId = pResult->frameworkFrameNum;
        CHX_LOG("FeatureQuadCFA fullsizeRaw RDI ReqId to AppFrameNum: %d <--> %d", pResult->frameworkFrameNum, resultFrameNum);
    }
    else if (m_remosaicSessionId == pCbData->sessionId)
    {
        PipelineData*     pPipelineData =
            const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_remosaicSessionId, 0));

        resultFrameNum = pPipelineData->seqIdToFrameNum[pResult->frameworkFrameNum % MaxOutstandingRequests];
        resultFrameIndex = resultFrameNum % MaxOutstandingRequests;

        rtPipelineReqId = pResult->frameworkFrameNum;
        CHX_LOG("FeatureQuadCFA Remosaic RDI ReqId to AppFrameNum: %d <--> %d", pResult->frameworkFrameNum, resultFrameNum);
    }

    camera3_capture_result_t* pUsecaseResult   = m_pUsecase->GetCaptureResult(resultFrameIndex);

    pUsecaseResult->frame_number = resultFrameNum;

    // If result contain metadata and metadata has not been sent to framework
    if ((NULL != pResult->pOutputMetadata) && (NULL != pResult->pInputMetadata))
    {
        BOOL               isResultMetadataAvailable = FALSE;
        UINT64             timestamp                 = m_pUsecase->GetRequestShutterTimestamp(resultFrameNum);

        // Validate handles
        pChiOutputMetadata = m_pMetadataManager->GetMetadataFromHandle(pResult->pOutputMetadata);
        pChiInputMetadata  = m_pMetadataManager->GetMetadataFromHandle(pResult->pInputMetadata);

        if (ChiMetadataManager::InvalidClientId == pChiInputMetadata->GetClientId())
        {
            // internal metadata
            pChiInputMetadata->ReleaseReference();
        }
        else
        {
            m_pMetadataManager->Release(pChiInputMetadata);
        }

        if (FALSE == m_pUsecase->IsMetadataSent(resultFrameIndex))
        {
            // Do Not wait for Snapshot frame metadata, Return Preview metadata back to fwk
            // If we wait for snapshot, and if it takes more time to process, we will block the preview.
            ChxUtils::UpdateTimeStamp(pChiOutputMetadata, timestamp, resultFrameNum);

            // check if flash is required for snapshot
            m_isFlashRequired = m_pUsecase->IsFlashRequired(*pChiOutputMetadata);

            m_pUsecase->UpdateAppResultMetadata(pChiOutputMetadata,
                                                resultFrameIndex,
                                                m_pUsecase->GetMetadataClientIdFromPipeline(pCbData->sessionId, 0));

            m_pUsecase->SetMetadataAvailable(resultFrameIndex);

            isAppResultsAvailable = TRUE;
        }

        pUsecaseResult->partial_result = pResult->numPartialMetadata;

        if (m_previewSessionId == pCbData->sessionId)
        {
            m_pUsecase->FillMetadataForRDIQueue(rtPipelineReqId, m_rdiStreamIndex, pChiOutputMetadata);
        }
        else if (m_fullSizeRawSessionId == pCbData->sessionId)
        {
            CHX_LOG("meta for Fullsize RAW RDI session");

            UINT32 snapshotRDIQueueIdx = (rtPipelineReqId % BufferQueueDepth);
            m_pFullSizeRawTargetBuffer->pMutex->Lock();

            m_pFullSizeRawTargetBuffer->bufferQueue[snapshotRDIQueueIdx].pMetadata       = pChiOutputMetadata;
            m_pFullSizeRawTargetBuffer->bufferQueue[snapshotRDIQueueIdx].frameNumber     = resultFrameNum;
            m_pFullSizeRawTargetBuffer->bufferQueue[snapshotRDIQueueIdx].isMetadataReady = TRUE;

            if ((TRUE == m_pFullSizeRawTargetBuffer->bufferQueue[snapshotRDIQueueIdx].isBufferReady) &&
                (TRUE == m_pFullSizeRawTargetBuffer->bufferQueue[snapshotRDIQueueIdx].isMetadataReady))
            {
                m_pFullSizeRawTargetBuffer->lastReadySequenceID = rtPipelineReqId;

                CHX_LOG("Notify Fullsize RAW RDI buffer is ready, rtPipelineReqId:%d", rtPipelineReqId);
                m_pFullSizeRawTargetBuffer->pCondition->Signal();
            }

            m_pFullSizeRawTargetBuffer->pMutex->Unlock();
        }
        else if (m_remosaicSessionId == pCbData->sessionId)
        {
            CHX_LOG("meta for Remosaic RDI session");

            UINT32 remosaicRDIQueueIdx = (rtPipelineReqId % BufferQueueDepth);
            m_pRemosaicRawTargetBuffer->pMutex->Lock();

            m_pRemosaicRawTargetBuffer->bufferQueue[remosaicRDIQueueIdx].pMetadata       = pChiOutputMetadata;
            m_pRemosaicRawTargetBuffer->bufferQueue[remosaicRDIQueueIdx].frameNumber     = resultFrameNum;
            m_pRemosaicRawTargetBuffer->bufferQueue[remosaicRDIQueueIdx].isMetadataReady = TRUE;

            if ((TRUE == m_pRemosaicRawTargetBuffer->bufferQueue[remosaicRDIQueueIdx].isBufferReady) &&
                (TRUE == m_pRemosaicRawTargetBuffer->bufferQueue[remosaicRDIQueueIdx].isMetadataReady))
            {
                m_pRemosaicRawTargetBuffer->lastReadySequenceID = rtPipelineReqId;

                CHX_LOG("Notify Remosaic RDI buffer is ready, rtPipelineReqId:%d", rtPipelineReqId);
                m_pRemosaicRawTargetBuffer->pCondition->Signal();
            }

            m_pRemosaicRawTargetBuffer->pMutex->Unlock();
        }
        else if (m_bayer2YuvSessionId == pCbData->sessionId)
        {
            CHX_LOG("Fill meta for bayer2yuv session：%d, reqId:%d", pCbData->sessionId, rtPipelineReqId);

            UINT32 bayer2YuvQueueIdx = (rtPipelineReqId % BufferQueueDepth);

            m_pBayer2YuvTargetBuffer->pMutex->Lock();
            m_pBayer2YuvTargetBuffer->bufferQueue[bayer2YuvQueueIdx].frameNumber         = resultFrameNum;
            m_pBayer2YuvTargetBuffer->bufferQueue[bayer2YuvQueueIdx].pMetadata           = pChiOutputMetadata;
            m_pBayer2YuvTargetBuffer->bufferQueue[bayer2YuvQueueIdx].isMetadataReady     = TRUE;

            if ((TRUE == m_pBayer2YuvTargetBuffer->bufferQueue[bayer2YuvQueueIdx].isBufferReady) &&
                (TRUE == m_pBayer2YuvTargetBuffer->bufferQueue[bayer2YuvQueueIdx].isMetadataReady))
            {
                m_pBayer2YuvTargetBuffer->lastReadySequenceID = rtPipelineReqId;

                CHX_LOG("Notify bayer2yuv result is ready, rtPipelineReqId:%d", rtPipelineReqId);
                m_pBayer2YuvTargetBuffer->pCondition->Signal();
            }
            m_pBayer2YuvTargetBuffer->pMutex->Unlock();
        }
        else if (m_snapshotSessionId == pCbData->sessionId)
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
        }
        else if (m_pFdStream == pResult->pOutputBuffers[j].pStream)
        {
            m_pUsecase->UpdateBufferReadyForFDQueue(rtPipelineReqId, m_fdStreamIndex, TRUE);
        }
        else if (m_pFullSizeRawStream == pResult->pOutputBuffers[j].pStream)
        {
            UINT32 snapshotRDIQueueIdx = (rtPipelineReqId % BufferQueueDepth);

            CHX_LOG("receved Fullsize RDI Raw Buffer. rtPipelineReqId:%d, snapshotRDIQueueIdx:%d, frame_number:%d",
                rtPipelineReqId, snapshotRDIQueueIdx, resultFrameNum);

            m_pFullSizeRawTargetBuffer->pMutex->Lock();

            // Release reference to the buffer that's going to be replaced out of the queue
            if (rtPipelineReqId >= BufferQueueDepth)
            {
                m_pFullSizeRawTargetBuffer->pBufferManager->ReleaseReference(
                    &m_pFullSizeRawTargetBuffer->bufferQueue[snapshotRDIQueueIdx].pRdiOutputBuffer->bufferInfo);
            }
            ChxUtils::Memcpy(m_pFullSizeRawTargetBuffer->bufferQueue[snapshotRDIQueueIdx].pRdiOutputBuffer,
                             &pResult->pOutputBuffers[j],
                             sizeof(CHISTREAMBUFFER));

            m_pFullSizeRawTargetBuffer->bufferQueue[snapshotRDIQueueIdx].pRdiOutputBuffer->acquireFence.valid = FALSE;
            m_pFullSizeRawTargetBuffer->bufferQueue[snapshotRDIQueueIdx].pRdiOutputBuffer->size         = sizeof(CHISTREAMBUFFER);
            m_pFullSizeRawTargetBuffer->bufferQueue[snapshotRDIQueueIdx].isBufferReady                  = TRUE;

            // during flush, metadata may not goes to chi usecase
            // here force set metaready to ensure wake up offline thread during flush
            if (FlushStatus::NotFlushing != m_pUsecase->GetFlushStatus())
            {
                m_pFullSizeRawTargetBuffer->bufferQueue[snapshotRDIQueueIdx].isMetadataReady = TRUE;
            }

            if ((TRUE == m_pFullSizeRawTargetBuffer->bufferQueue[snapshotRDIQueueIdx].isBufferReady) &&
                (TRUE == m_pFullSizeRawTargetBuffer->bufferQueue[snapshotRDIQueueIdx].isMetadataReady))
            {
                m_pFullSizeRawTargetBuffer->lastReadySequenceID = rtPipelineReqId;

                CHX_LOG("Notify Fullsize RDI buffer is ready, rtPipelineReqId:%d", rtPipelineReqId);
                m_pFullSizeRawTargetBuffer->pCondition->Signal();
            }

            m_pFullSizeRawTargetBuffer->pMutex->Unlock();
        }
        else if (m_pRemosaicRawStream == pResult->pOutputBuffers[j].pStream)
        {
            UINT32 remosaicRDIQueueIdx = (rtPipelineReqId % BufferQueueDepth);

            CHX_LOG("receved Remosaic RDI Raw Buffer. rtPipelineReqId:%d, snapshotRDIQueueIdx:%d, frame_number:%d",
                rtPipelineReqId, remosaicRDIQueueIdx, resultFrameNum);

            // Release input buffer first here, before signal offline request thread.
            // Becuase when the offline request thread submits a request to next stage,
            // it might use the same pPrivData (pointing to same m_privData[]), and overwrite the pPrivData with new buffer,
            // then we will release incorrect input buffer here.
            for(UINT32 i = 0; i < pResult->pPrivData->numInputBuffers; i++)
            {
                if ((NULL != pResult->pPrivData->bufferManagers[i]) &&
                    (NULL != pResult->pPrivData->inputBuffers[i].phBuffer))
                {
                    reinterpret_cast<CHIBufferManager*>(pResult->pPrivData->bufferManagers[i])->
                        ReleaseReference(&pResult->pPrivData->inputBuffers[i]);
                    pResult->pPrivData->bufferManagers[i]        = NULL;
                    pResult->pPrivData->inputBuffers[i].phBuffer = NULL;
                }
            }

            m_pRemosaicRawTargetBuffer->pMutex->Lock();

            // Release reference to the buffer that's going to be replaced out of the queue
            if (rtPipelineReqId >= BufferQueueDepth)
            {
                m_pRemosaicRawTargetBuffer->pBufferManager->ReleaseReference(
                    &m_pRemosaicRawTargetBuffer->bufferQueue[remosaicRDIQueueIdx].pRdiOutputBuffer->bufferInfo);
            }
            ChxUtils::Memcpy(m_pRemosaicRawTargetBuffer->bufferQueue[remosaicRDIQueueIdx].pRdiOutputBuffer,
                             &pResult->pOutputBuffers[j],
                             sizeof(CHISTREAMBUFFER));

            m_pRemosaicRawTargetBuffer->bufferQueue[remosaicRDIQueueIdx].pRdiOutputBuffer->acquireFence.valid = FALSE;
            m_pRemosaicRawTargetBuffer->bufferQueue[remosaicRDIQueueIdx].pRdiOutputBuffer->size               = sizeof(CHISTREAMBUFFER);
            m_pRemosaicRawTargetBuffer->bufferQueue[remosaicRDIQueueIdx].isBufferReady                        = TRUE;

            // during flush, metadata may not goes to chi usecase
            // here force set metaready to ensure wake up offline thread during flush
            if (FlushStatus::NotFlushing != m_pUsecase->GetFlushStatus())
            {
                m_pRemosaicRawTargetBuffer->bufferQueue[remosaicRDIQueueIdx].isMetadataReady = TRUE;
            }

            if ((TRUE == m_pRemosaicRawTargetBuffer->bufferQueue[remosaicRDIQueueIdx].isBufferReady) &&
                (TRUE == m_pRemosaicRawTargetBuffer->bufferQueue[remosaicRDIQueueIdx].isMetadataReady))
            {
                m_pRemosaicRawTargetBuffer->lastReadySequenceID = rtPipelineReqId;

                CHX_LOG("Notify Remosaic RDI buffer is ready, rtPipelineReqId:%d", rtPipelineReqId);
                m_pRemosaicRawTargetBuffer->pCondition->Signal();
            }

            m_pRemosaicRawTargetBuffer->pMutex->Unlock();
        }
        else if (m_pQcfaBayer2YuvStream == pResult->pOutputBuffers[j].pStream)
        {
            UINT32 yuvBufferQIdx = (rtPipelineReqId % BufferQueueDepth);

            CHX_LOG("Notify bayer2yuv result buffer for session：%d, reqId:%d", pCbData->sessionId, rtPipelineReqId);

            m_pUsecase->ReleaseReferenceToInputBuffers(pResult->pPrivData);

            m_pBayer2YuvTargetBuffer->pMutex->Lock();

            // Release reference to the buffer that's going to be replaced out of the queue
            if (rtPipelineReqId >= BufferQueueDepth)
            {
                m_pBayer2YuvTargetBuffer->pBufferManager->ReleaseReference(
                    &m_pBayer2YuvTargetBuffer->bufferQueue[yuvBufferQIdx].pRdiOutputBuffer->bufferInfo);
            }
            ChxUtils::Memcpy(m_pBayer2YuvTargetBuffer->bufferQueue[yuvBufferQIdx].pRdiOutputBuffer,
                             &pResult->pOutputBuffers[j],
                             sizeof(CHISTREAMBUFFER));

            m_pBayer2YuvTargetBuffer->bufferQueue[yuvBufferQIdx].pRdiOutputBuffer->acquireFence.valid = FALSE;
            m_pBayer2YuvTargetBuffer->bufferQueue[yuvBufferQIdx].pRdiOutputBuffer->size               = sizeof(CHISTREAMBUFFER);
            m_pBayer2YuvTargetBuffer->bufferQueue[yuvBufferQIdx].isBufferReady                        = TRUE;

            // during flush, metadata may not goes to chi usecase
            // here force set metaready to ensure wake up offline thread during flush
            if (FlushStatus::NotFlushing != m_pUsecase->GetFlushStatus())
            {
                m_pBayer2YuvTargetBuffer->bufferQueue[yuvBufferQIdx].isMetadataReady = TRUE;
            }

            if ((TRUE == m_pBayer2YuvTargetBuffer->bufferQueue[yuvBufferQIdx].isBufferReady) &&
                (TRUE == m_pBayer2YuvTargetBuffer->bufferQueue[yuvBufferQIdx].isMetadataReady))
            {
                m_pBayer2YuvTargetBuffer->lastReadySequenceID = rtPipelineReqId;

                CHX_LOG("Notify bayer2yuv buffer is ready, rtPipelineReqId:%d", rtPipelineReqId);
                m_pBayer2YuvTargetBuffer->pCondition->Signal();
            }
            m_pBayer2YuvTargetBuffer->pMutex->Unlock();

            if (FALSE == IsJPEGOutputRequired())
            {
               CHX_LOG("Final YUV buffer is ready, appFrameNumber:%d", resultFrameNum);
               m_pUsecase->ProcessFeatureDone(resultFrameNum, this, pResult);
            }
        }
        // Otherwise queue a buffer as part of the normal result
        else
        {
            camera3_stream_buffer_t* pResultBuffer =
                const_cast<camera3_stream_buffer_t*>(&pUsecaseResult->output_buffers[pUsecaseResult->num_output_buffers++]);

            ChxUtils::PopulateChiToHALStreamBuffer(&pResult->pOutputBuffers[j], pResultBuffer);

            if (m_snapshotSessionId == pCbData->sessionId)
            {
                // Release reference to the input buffers of the request to the SnapshotPipeline
                for(UINT32 i = 0; i < pResult->pPrivData->numInputBuffers; i++)
                {
                    if ((NULL != pResult->pPrivData->bufferManagers[i]) &&
                        (NULL != pResult->pPrivData->inputBuffers[i].phBuffer))
                    {
                        reinterpret_cast<CHIBufferManager*>(pResult->pPrivData->bufferManagers[i])->
                            ReleaseReference(&pResult->pPrivData->inputBuffers[i]);
                        pResult->pPrivData->bufferManagers[i]        = NULL;
                        pResult->pPrivData->inputBuffers[i].phBuffer = NULL;
                    }
                }

                SetFeatureStatus(FeatureStatus::READY);
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
/// FeatureQuadCFA::ProcessMessage
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureQuadCFA::ProcessMessage(
    const CHIMESSAGEDESCRIPTOR* pMessageDescriptor,
    VOID*                       pPrivateCallbackData)
{
    (VOID)pMessageDescriptor;
    (VOID)pPrivateCallbackData;
    //@todo this function to be REMOVED
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureZSL::ProcessCHIPartialData
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureQuadCFA::ProcessCHIPartialData(
    UINT32    frameNum,
    UINT32    sessionId)
{
    CAMX_UNREFERENCED_PARAM(frameNum);
    CAMX_UNREFERENCED_PARAM(sessionId);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureZSL::ProcessDriverPartialCaptureResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureQuadCFA::ProcessDriverPartialCaptureResult(
    CHIPARTIALCAPTURERESULT*    pResult,
    VOID*                       pPrivateCallbackData)
{
    camera3_capture_result_t*   pDriverPartialResult    = NULL;
    PipelineData*               pPipelineData           = NULL;
    UINT32                      resultFrameNum          = pResult->frameworkFrameNum;
    UINT32                      resultFrameIndex        = ChxUtils::GetResultFrameIndexChi(resultFrameNum);
    SessionPrivateData*         pCbData                 = static_cast<SessionPrivateData*>(pPrivateCallbackData);
    PartialResultSender         sender                  = PartialResultSender::DriverPartialData;
    ChiMetadata*                pChiOutputMetadata      = NULL;

    if ((TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress))) ||
        (FlushStatus::NotFlushing != m_pUsecase->GetFlushStatus()))
    {
        CHX_LOG_INFO("quadcfa process result return because of cleanup");
        return;
    }

    // This check is to ensure that we have not sent earlier and this is the final pipeline's partial
    // result
    if ((m_pUsecase->GetFinalPipelineForPartialMetaData() == pCbData->sessionId)&&
        (TRUE == m_pUsecase->CheckIfPartialDataCanBeSent(sender, resultFrameIndex)))
    {
        PipelineData*     pPipelineData =
            const_cast<PipelineData*>(m_pUsecase->GetPipelineData(pCbData->sessionId, 0));

        resultFrameNum = pPipelineData->seqIdToFrameNum[pResult->frameworkFrameNum % MaxOutstandingRequests];
        resultFrameIndex = resultFrameNum % MaxOutstandingRequests;

        CHX_LOG("sessionid:%d, seqId to AppFrameNum: %d <--> %d",
            pCbData->sessionId, pResult->frameworkFrameNum, resultFrameNum);

        if (PartialMetaSupport::CombinedPartialMeta == ExtensionModule::GetInstance()->EnableCHIPartialData())
        {
            ProcessCHIPartialData(pResult->frameworkFrameNum, pCbData->sessionId);
        }

        pChiOutputMetadata = m_pMetadataManager->GetMetadataFromHandle(pResult->pPartialResultMetadata);

        m_pUsecase->UpdateAppPartialResultMetadataFromDriver(pChiOutputMetadata,
                                                             resultFrameIndex,
                                                             resultFrameNum,
                                                             m_pUsecase->GetMetadataClientIdFromPipeline(
                                                                 pCbData->sessionId, 0));
        m_pUsecase->ProcessAndReturnPartialMetadataFinishedResults(sender);
    }
}

UINT32 FeatureQuadCFA::RegisterPipelinesToUsecase(
    UINT32                numPipelinesInGroup,
    AdvancedPipelineType* pipelineGroup,
    UINT32*               cameraIds,
    AdvancedPipelineType* pRegisteredPipelines)
{
    UINT32 sessionId = InvalidSessionId;

    if ((NULL == pipelineGroup) || (NULL == pRegisteredPipelines))
    {
        CHX_LOG_ERROR("Invalid input parameters.");
        return sessionId;
    }

    for(UINT32 i = 0; i < numPipelinesInGroup; i++)
    {
        pRegisteredPipelines[m_registeredPipelineCount] = pipelineGroup[i];
        m_registeredPipelineCount++;
    }

    sessionId = m_pUsecase->GetUniqueSessionId(pipelineGroup, numPipelinesInGroup, GetPhysicalCameraIndex());
    m_pUsecase->SetPipelineCameraId(pipelineGroup, cameraIds, numPipelinesInGroup);

    CHX_LOG("numPipelinesInGroup:%d, sessionId:%d", numPipelinesInGroup, sessionId);

    return sessionId;
}

INT32 FeatureQuadCFA::GetRequiredPipelines(
    AdvancedPipelineType* pPipelines,
    INT32 size)
{
    const INT32 maxRequiredPipelines    = 5;
    CDKResult   result                  = CDKResultSuccess;

    CHX_LOG_INFO("chi usecaseId:%d", m_pUsecase->GetUsecaseId());

    if ((FALSE == m_pUsecase->IsQuadCFAUsecase()) && (FALSE == m_pUsecase->IsMultiCameraUsecase()))
    {
        CHX_LOG_ERROR("Invalid chi usecaseId:%d!", m_pUsecase->GetUsecaseId());
        m_previewSessionId     = InvalidSessionId;
        m_fullSizeRawSessionId = InvalidSessionId;
        m_previewSessionId     = InvalidSessionId;

        result = CDKResultEFailed;
    }

    if ((CDKResultSuccess == result) && (NULL != pPipelines) && (size >= maxRequiredPipelines))
    {
        AdvancedPipelineType pipelineGroup[1] = { AdvancedPipelineType::PipelineCount };
        UINT                 physicalCameraID = m_pUsecase->GetPhysicalCameraId(m_physicalCameraIndex);
        ChiStream*           pSnapshotStrem   = NULL;

        if (InvalidPhysicalCameraId != physicalCameraID)
        {
            pSnapshotStrem = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::SnapshotStream);
            if (NULL == pSnapshotStrem)
            {
                CHX_LOG_ERROR("pSnapshotStream is NULL!");
                result = CDKResultEFailed;
                return result;
            }

            if ((UsecaseSelector::IsYUVSnapshotStream(reinterpret_cast<camera3_stream_t*>(pSnapshotStrem))) ||
                (UsecaseSelector::IsHEIFStream(reinterpret_cast<camera3_stream_t*>(pSnapshotStrem))))
            {
                m_isYuvSnapshotFromApp = TRUE;
                CHX_LOG_INFO("App configed with YUV snapshot stream");

                pipelineGroup[0]    = AdvancedPipelineType::QuadCFASnapshotYuvType;
                m_snapshotSessionId = RegisterPipelinesToUsecase(1, pipelineGroup, &physicalCameraID, pPipelines);

                CHX_LOG_INFO("pipeline: QuadCFASnapshotYuvType, m_snapshotSessionId: %d", m_snapshotSessionId);
            }
            else
            {
                if (TRUE == IsJPEGOutputRequired())
                {
                    pipelineGroup[0]    = AdvancedPipelineType::InternalZSLYuv2JpegType;
                    m_snapshotSessionId = RegisterPipelinesToUsecase(1, pipelineGroup, &physicalCameraID, pPipelines);

                    CHX_LOG_INFO("pipeline: InternalZSLYuv2JpegType, m_snapshotSessionId: %d", m_snapshotSessionId);
                }

                pipelineGroup[0]     = AdvancedPipelineType::QuadCFASnapshotYuvType;
                m_bayer2YuvSessionId = RegisterPipelinesToUsecase(1, pipelineGroup, &physicalCameraID, pPipelines);

                CHX_LOG_INFO("pipeline: QuadCFASnapshotYuvType, m_bayer2YuvSessionId: %d", m_bayer2YuvSessionId);
            }

            if (CHIREMOSAICTYPE::SWRemosaic == m_RemosaicType)
            {
                pipelineGroup[0]     = AdvancedPipelineType::QuadCFARemosaicType;
                m_remosaicSessionId  = RegisterPipelinesToUsecase(1, pipelineGroup, &physicalCameraID, pPipelines);

                CHX_LOG_INFO("pipeline: QuadCFARemosaicType, m_remosaicSessionId: %d", m_remosaicSessionId);
            }

            if (TRUE == m_pUsecase->IsMultiCameraUsecase())
            {
                m_fullSizeRawSessionId = InvalidSessionId;
                m_previewSessionId     = InvalidSessionId;
                CHX_LOG("Realtime pipelines are not needed in AdvancedCameraUsecase for multicamera usecase");
            }
            else if (TRUE == m_pUsecase->IsQuadCFAUsecase())
            {
                pipelineGroup[0]       = AdvancedPipelineType::QuadCFAFullSizeRawType;
                m_fullSizeRawSessionId = RegisterPipelinesToUsecase(1, pipelineGroup, &physicalCameraID, pPipelines);

                CHX_LOG_INFO("pipeline: QuadCFAFullSizeRawType, m_fullSizeRawSessionId: %d", m_fullSizeRawSessionId);

                if (NULL == m_pUsecase->GetSharedStream(AdvancedCameraUsecase::VideoStream))
                {
                    pipelineGroup[0] = AdvancedPipelineType::ZSLPreviewRawType;
                }
                else if (TRUE == m_pUsecase->IsQuadCFAUsecase())
                {
                    pipelineGroup[0] = AdvancedPipelineType::ZSLPreviewRawYUVType;
                }

                m_previewSessionId   = RegisterPipelinesToUsecase(1, pipelineGroup, &physicalCameraID, pPipelines);

                CHX_LOG_INFO("pipeline: ZSLPreviewRawType, m_previewSessionId: %d", m_previewSessionId);
            }
        }
    }

    CHX_LOG("registeredPipelineCount:%d, video stream=%p", m_registeredPipelineCount,
        m_pUsecase->GetSharedStream(AdvancedCameraUsecase::VideoStream));

    return m_registeredPipelineCount;
}

CDKResult FeatureQuadCFA::SubmitRequestToSession(
    UINT32                   sessionId,
    const Session*           pSession,
    UINT32                   numRequests,
    const CHICAPTUREREQUEST* pCaptureRequests)
{
    CDKResult          result        = CDKResultSuccess;
    CHIPIPELINEREQUEST submitRequest = { 0 };

    submitRequest.pSessionHandle     = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
    submitRequest.numRequests        = numRequests;
    submitRequest.pCaptureRequests   = pCaptureRequests;

    if (FALSE == pSession->IsPipelineActive())
    {
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

    if (CDKResultSuccess == result)
    {
        if ((FlushStatus::NotFlushing == m_pUsecase->GetFlushStatus()) &&
            (FALSE == ChxUtils::HasInputBufferError(&submitRequest)))
        {
            CHX_LOG_CONFIG("Sending request to session:%d, internal seq id:%" PRIu64 "",
                sessionId, pCaptureRequests->frameNumber);

            m_pUsecase->SetRequestToFeatureMapping(sessionId, pCaptureRequests->frameNumber, this);

            result = m_pUsecase->SubmitRequest(&submitRequest);
        }
        else
        {
            CHX_LOG_INFO("skip submit request, internal requestid:%" PRIu64 "", pCaptureRequests->frameNumber);
            InvalidateRequest(&submitRequest);
            result = CDKResultEBusy;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureQuadCFA::RequestThread
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID* FeatureQuadCFA::RequestThread(
    VOID* pThreadData)
{
    PerThreadData* pPerThreadData = reinterpret_cast<PerThreadData*>(pThreadData);

    FeatureQuadCFA* pFeature = reinterpret_cast<FeatureQuadCFA*>(pPerThreadData->pPrivateData);

    pFeature->RequestThreadProcessing();

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureQuadCFA::RequestThreadProcessing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureQuadCFA::RequestThreadProcessing()
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

        CHX_LOG("offline request available, m_maxSnapshotReqId:%d, m_snapshotReqId:%d", m_maxSnapshotReqId, m_snapshotReqId);

        while (m_maxSnapshotReqId > m_snapshotReqId && m_offlineRequestProcessTerminate == FALSE)
        {
            UINT   requestIdIndex    = (m_snapshotReqId % MaxOutstandingRequests);
            UINT   frameNumber       = m_snapshotReqIdToFrameNum[requestIdIndex];
            UINT32 requestFrameIndex = frameNumber % MaxOutstandingRequests;
            UINT32 currentsnapReqId  = m_snapshotReqId;
            // Check again, If we have the requirements for the snapshot frame for the corresponding frameNumber
            if (TRUE == m_isSnapshotFrame[requestFrameIndex])
            {
                UINT               snapshotRDIReqId    = m_snapshotInputNum[requestIdIndex];
                CHISTREAMBUFFER    inputBuffer         = { 0 };
                ChiMetadata*       pChiInputMeta       = NULL;
                ChiMetadata*       pChiOutputMeta      = NULL;
                CHIBufferManager*  pInputBufferManager = NULL;

                // Extract the current request settings to be merged with metadata of zsl queue
                ChiMetadata*  pChiRTInputMetadata = m_pSnapshotInputMeta[requestIdIndex];
                CHX_LOG("Getting input metadata for snapshot request index: %d count %d",
                        requestIdIndex, pChiRTInputMetadata->Count());

                CHX_LOG("offline snapshot reprocess, intput req id:%d", m_snapshotInputNum[requestIdIndex]);

                // FullRaw RDI buffer and meta are always ready at this point.
                // because we signale offline thread after RDI buffer and meta are ready.
                m_pUsecase->GetTargetBuffer(snapshotRDIReqId,
                                            m_pFullSizeRawTargetBuffer,
                                            pChiRTInputMetadata,
                                            &inputBuffer,
                                            &pChiInputMeta);

                pInputBufferManager = m_pFullSizeRawTargetBuffer->pBufferManager;

                if (NULL == pChiInputMeta)
                {
                    CHX_LOG_ERROR("ERROR Failed to get input metadata from m_pFullSizeRawTargetBuffer");
                    result = CDKResultEFailed;
                }


                if ((CDKResultSuccess == result) && (CHIREMOSAICTYPE::SWRemosaic == m_RemosaicType))
                {
                    // submit to sw remosaic pipeline
                    PipelineData* pPipelineData = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_remosaicSessionId, 0));
                    pChiOutputMeta              = m_pMetadataManager->Get(pPipelineData->pPipeline->GetMetadataClientId(),
                                                                          currentsnapReqId);
                    if (NULL == pChiOutputMeta)
                    {
                        CHX_LOG_ERROR("ERROR Failed to get output metadata for pipeline:m_remosaicPipelineIndex ");
                        result = CDKResultEFailed;
                    }

                    if (CDKResultSuccess == result)
                    {
                        CHX_LOG("offline SW remosaic reprocess, intput req id:%d", m_snapshotInputNum[requestIdIndex]);

                        result = GenerateRemosaicRequest(frameNumber,
                                                requestIdIndex,
                                                currentsnapReqId,
                                                pChiInputMeta,
                                                pChiOutputMeta,
                                                &inputBuffer);

                        if (CDKResultSuccess == result)
                        {
                            //Wait for Remosaic pipeline ready.
                            result = WaitRemosaicRDIReady(snapshotRDIReqId);
                        }

                        if (CDKResultSuccess == result)
                        {
                            // get result from remosaic pipeline, as input to next stage/pipeline
                            m_pUsecase->GetTargetBuffer(currentsnapReqId,
                                                        m_pRemosaicRawTargetBuffer,
                                                        pChiRTInputMetadata,
                                                        &inputBuffer,
                                                        &pChiInputMeta);
                            pInputBufferManager = m_pRemosaicRawTargetBuffer->pBufferManager;

                            if (NULL == pChiInputMeta)
                            {
                                CHX_LOG_ERROR("Failed to get input metadata from m_pRemosaicRawTargetBuffer, requestId:%d",
                                    currentsnapReqId);
                                result = CDKResultEFailed;
                            }
                        }
                    }
                }

                if ((CDKResultSuccess == result) && (TRUE == IsYuvSnapshotFromAPP()))
                {
                    // submit to snapshotyuv pipeline
                    PipelineData* pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_snapshotSessionId, 0));
                    pChiOutputMeta               = m_pMetadataManager->Get(pPipelineData->pPipeline->GetMetadataClientId(),
                                                                           currentsnapReqId);
                    if (NULL == pChiOutputMeta)
                    {
                        CHX_LOG_ERROR("ERROR Failed to get output metadata for pipeline: snapshotYuv(bayer2yuv)");
                        result = CDKResultEFailed;
                    }

                    if (CDKResultSuccess == result)
                    {
                        const Session* pSession = m_pUsecase->GetSessionData(m_snapshotSessionId)->pSession;

                        CHICAPTUREREQUEST snapshotRequest = { 0 };
                        snapshotRequest.frameNumber       = currentsnapReqId;
                        snapshotRequest.hPipelineHandle   = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
                        snapshotRequest.numInputs         = 1;
                        snapshotRequest.numOutputs        = m_snapshotBufferNum[requestIdIndex];
                        snapshotRequest.pInputBuffers     = &inputBuffer;
                        snapshotRequest.pOutputBuffers    = m_snapshotBuffers[requestIdIndex];

                        snapshotRequest.pInputMetadata    = pChiInputMeta->GetHandle();
                        snapshotRequest.pOutputMetadata   = pChiOutputMeta->GetHandle();
                        snapshotRequest.pPrivData         = &m_privData[snapshotRequest.frameNumber % MaxOutstandingRequests];
                        snapshotRequest.pPrivData->featureType = GetFeatureType();;

                        // Save input buffers info for later releasing reference
                        snapshotRequest.pPrivData->inputBuffers[0]      = inputBuffer.bufferInfo;
                        snapshotRequest.pPrivData->bufferManagers[0]    = pInputBufferManager;
                        snapshotRequest.pPrivData->numInputBuffers      = 1;

                        result = SubmitRequestToSession(m_snapshotSessionId, pSession, 1, &snapshotRequest);
                    }
                }
                else if (CDKResultSuccess == result)
                {
                    // submit to bayer2yuv pipeline
                    PipelineData* pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_bayer2YuvSessionId, 0));
                    pChiOutputMeta               = m_pMetadataManager->Get(pPipelineData->pPipeline->GetMetadataClientId(),
                                                                           currentsnapReqId);
                    if (NULL == pChiOutputMeta)
                    {
                        CHX_LOG_ERROR("ERROR Failed to get output metadata for pipeline: m_bayer2YuvSessionId");
                        result = CDKResultEFailed;
                    }

                    if (CDKResultSuccess == result)
                    {
                        CHISTREAMBUFFER outputBuffer    = {0};
                        outputBuffer.bufferInfo         = m_pBayer2YuvTargetBuffer->pBufferManager->GetImageBufferInfo();
                        outputBuffer.pStream            = m_pQcfaBayer2YuvStream;
                        outputBuffer.releaseFence.valid = FALSE;
                        outputBuffer.acquireFence.valid = FALSE;
                        outputBuffer.size               = sizeof(CHISTREAMBUFFER);

                        const Session* pSession = m_pUsecase->GetSessionData(m_bayer2YuvSessionId)->pSession;

                        CHICAPTUREREQUEST snapshotRequest = { 0 };
                        snapshotRequest.frameNumber       = currentsnapReqId;
                        snapshotRequest.hPipelineHandle   = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
                        snapshotRequest.numInputs         = 1;
                        snapshotRequest.numOutputs        = 1;
                        snapshotRequest.pInputBuffers     = &inputBuffer;
                        snapshotRequest.pOutputBuffers    = &outputBuffer;

                        snapshotRequest.pInputMetadata    = pChiInputMeta->GetHandle();
                        snapshotRequest.pOutputMetadata   = pChiOutputMeta->GetHandle();
                        snapshotRequest.pPrivData         = &m_privData[snapshotRequest.frameNumber % MaxOutstandingRequests];
                        snapshotRequest.pPrivData->featureType = GetFeatureType();

                        // Save input buffers info for later releasing reference
                        snapshotRequest.pPrivData->inputBuffers[0]      = inputBuffer.bufferInfo;
                        snapshotRequest.pPrivData->bufferManagers[0]    = pInputBufferManager;
                        snapshotRequest.pPrivData->numInputBuffers      = 1;

                        CHX_LOG("sending bayer2yuv reuqest, reqid:%d, inputbuffer:%p, outputbuffer:%p",
                            currentsnapReqId,
                            inputBuffer.bufferInfo.phBuffer,
                            outputBuffer.bufferInfo.phBuffer);

                        result = SubmitRequestToSession(m_bayer2YuvSessionId, pSession, 1, &snapshotRequest);
                    }

                    if ((CDKResultSuccess == result) && (TRUE == IsJPEGOutputRequired()))
                    {
                        // wait bayer2yuv buffer ready and then submit to jpeg pipeline

                        CHX_LOG("wait for bayer2yuv result ready, reqid:%d, target index:%d",
                            currentsnapReqId, m_bayer2YuvStreamIdx);

                        result = m_pUsecase->WaitForBufferMetaReady(currentsnapReqId, m_bayer2YuvStreamIdx);

                        if (CDKResultSuccess == result)
                        {
                            CHX_LOG("bayer2yuv result ready");

                            m_pUsecase->GetTargetBuffer(currentsnapReqId,
                                                        m_pBayer2YuvTargetBuffer,
                                                        pChiRTInputMetadata,
                                                        &inputBuffer,
                                                        &pChiInputMeta);

                            PipelineData* pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_snapshotSessionId, 0));
                            pChiOutputMeta               = m_pMetadataManager->Get(pPipelineData->pPipeline->GetMetadataClientId(),
                                                                                   currentsnapReqId);
                            if ((NULL == pChiOutputMeta) || (NULL == pChiInputMeta))
                            {
                                CHX_LOG_ERROR("Failed to get input/output metadata, requestId:%d, inputMeta:%p, outputMeta:%p",
                                    currentsnapReqId, pChiInputMeta, pChiOutputMeta);
                                result = CDKResultEFailed;
                            }
                        }
                        else
                        {
                            CHX_LOG_ERROR("error wait bayer2yuv result ready, reqid:%d, target index:%d, result:%d",
                                currentsnapReqId, m_bayer2YuvStreamIdx, result);
                        }

                        if (CDKResultSuccess == result)
                        {
                            inputBuffer.pStream = m_pJPEGInputStream;

                            result = GenerateSnapshotJpegRequest(frameNumber,
                                                        requestIdIndex,
                                                        currentsnapReqId,
                                                        pChiInputMeta,
                                                        pChiOutputMeta,
                                                        &inputBuffer,
                                                        m_pBayer2YuvTargetBuffer->pBufferManager);
                        }
                    }
                    else
                    {
                        CHX_LOG_INFO("Jpeg is not required in FeatureQuadcfa");
                    }
                }

                if (CDKResultSuccess != result)
                {
                    CHX_LOG_ERROR("Failure in offline process, appFrameNumber:%d, snapshotReqId:%d, result:%d",
                        frameNumber, currentsnapReqId, result);

                    SetFeatureStatus(FeatureStatus::READY);
                }

                if (NULL != pChiRTInputMetadata)
                {
                    // release the snapshot input meta at last
                    pChiRTInputMetadata->ReleaseReference();
                }

                m_snapshotReqId++;
            }
        }
    }
    CHX_LOG_INFO("RequestThreadProcessing Exited");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureQuadCFA::AllocateLocalDebugDataBuffer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureQuadCFA::AllocateLocalDebugDataBuffer(SIZE_T size)
{
    CDKResult  result = CDKResultSuccess;

    if (NULL == m_localDebugData.pData)
    {
        m_localDebugData.pData = CHX_CALLOC(size);

        if (NULL == m_localDebugData.pData)
        {
            CHX_LOG_ERROR("Fail to allocate buffer for local debug data!");
            result = CDKResultENoMemory;
        }
        else
        {
            m_localDebugData.size = size;
        }
    }
    else
    {
        CHX_LOG("Debug data buffer is already allocated!");
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureQuadCFA::DestroyLocalDebugData
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureQuadCFA::DestroyLocalDebugData()
{
    if (NULL != m_localDebugData.pData)
    {
        CHX_FREE(m_localDebugData.pData);
        m_localDebugData.pData = NULL;
        m_localDebugData.size  = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureQuadCFA::SaveDebugDataToLocal
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureQuadCFA::SaveDebugDataToLocal(ChiMetadata * pChiMeta)
{
    CDKResult  result     = CDKResultSuccess;
    DebugData* pDebugData = NULL;

    if ((NULL != pChiMeta) &&
        (TRUE == ChxUtils::IsVendorTagPresent(pChiMeta, DebugDataTag)))
    {
        ChxUtils::GetVendorTagValue(pChiMeta, DebugDataTag, (VOID**)&pDebugData);

        if ((NULL != pDebugData) && (NULL != pDebugData->pData) && (0 < pDebugData->size))
        {
            // Allocate debug-data for offline processing
            if (NULL == m_localDebugData.pData)
            {
                result = AllocateLocalDebugDataBuffer(pDebugData->size);
            }

            if ((CDKResultSuccess == result) &&
                (pDebugData->size != m_localDebugData.size))
            {
                CHX_LOG_ERROR("debug data size mismatch, size in meta:%zu, size in local copy:%zu",
                    pDebugData->size, m_localDebugData.size);
                result = CDKResultEFailed;
            }

            if ((CDKResultSuccess == result) &&
                (m_localDebugData.pData != pDebugData->pData)) // no need to copy, if the ptrs are already same
            {
                ChxUtils::Memcpy(m_localDebugData.pData, pDebugData->pData, pDebugData->size);
            }
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureQuadCFA::UpdateDebugDataPtr
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureQuadCFA::UpdateDebugDataPtr(
    ChiMetadata* pChiMeta,
    BOOL         saveOrigin)
{
    CDKResult  result     = CDKResultSuccess;
    DebugData* pDebugData = NULL;

    if ((NULL != pChiMeta) &&
        (TRUE == ChxUtils::IsVendorTagPresent(pChiMeta, DebugDataTag)))
    {
        ChxUtils::GetVendorTagValue(pChiMeta, DebugDataTag, (VOID**)&pDebugData);

        if ((NULL != pDebugData) && (NULL != pDebugData->pData) && (0 < pDebugData->size))
        {
            if (TRUE == saveOrigin)
            {
                result = SaveDebugDataToLocal(pChiMeta);
            }

            if ((CDKResultSuccess == result)                  &&
                (NULL != m_localDebugData.pData)              &&
                (m_localDebugData.pData != pDebugData->pData) &&
                (m_localDebugData.size  == pDebugData->size))    // no need to update, if the ptrs are already same
            {
                result = ChxUtils::SetVendorTagValue(pChiMeta,
                                                     DebugDataTag,
                                                     sizeof(DebugData),
                                                     &m_localDebugData);

                if (CDKResultSuccess != result)
                {
                    CHX_LOG_ERROR("Fail to set debugdata tag for offline input metadata");
                }
                else
                {
                    CHX_LOG("Update debug data ptr to:%p", m_localDebugData.pData);
                }
            }
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureQuadCFA::WaitFullsizeRDIReady
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureQuadCFA::WaitFullsizeRDIReady(
    UINT32 snapshotRDISeqId)
{
    CDKResult result     = CDKResultSuccess;
    UINT      queueIndex = snapshotRDISeqId % BufferQueueDepth;

    CHX_LOG("Last snapshot RDI input req id:%d", snapshotRDISeqId);

    m_pFullSizeRawTargetBuffer->pMutex->Lock();

    while ((FALSE == m_pFullSizeRawTargetBuffer->bufferQueue[queueIndex].isBufferReady) ||
           (FALSE == m_pFullSizeRawTargetBuffer->bufferQueue[queueIndex].isMetadataReady) ||
           (m_pFullSizeRawTargetBuffer->lastReadySequenceID < snapshotRDISeqId))
    {
        m_pFullSizeRawTargetBuffer->pCondition->Wait(m_pFullSizeRawTargetBuffer->pMutex->GetNativeHandle());

       if (FlushStatus::NotFlushing != m_pUsecase->GetFlushStatus())
       {
           CHX_LOG_INFO("In flush status, return EBusy! snapshotRDISeqId:%d", snapshotRDISeqId);
           result = CDKResultEBusy;
           break;
       }
   }

    m_pFullSizeRawTargetBuffer->pMutex->Unlock();

    CHX_LOG("Snapshot RDI result is ready");

    UpdateDebugDataPtr(m_pFullSizeRawTargetBuffer->bufferQueue[queueIndex].pMetadata, FALSE);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureQuadCFA::WaitRemosaicRDIReady
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureQuadCFA::WaitRemosaicRDIReady(
    UINT32 snapshotRDISeqId)
{
    CDKResult result     = CDKResultSuccess;
    UINT      queueIndex = snapshotRDISeqId % BufferQueueDepth;

    CHX_LOG("Last Remosaic RDI input req id:%d", snapshotRDISeqId);

    m_pRemosaicRawTargetBuffer->pMutex->Lock();

    while ((FALSE == m_pRemosaicRawTargetBuffer->bufferQueue[queueIndex].isBufferReady) ||
           (FALSE == m_pRemosaicRawTargetBuffer->bufferQueue[queueIndex].isMetadataReady))
    {
        m_pRemosaicRawTargetBuffer->pCondition->Wait(m_pRemosaicRawTargetBuffer->pMutex->GetNativeHandle());

        if (FlushStatus::NotFlushing != m_pUsecase->GetFlushStatus())
        {
            CHX_LOG_INFO("In flush status, return EBusy! snapshotRDISeqId:%d", snapshotRDISeqId);
            result = CDKResultEBusy;
            break;
        }
    }

    m_pRemosaicRawTargetBuffer->pMutex->Unlock();

    CHX_LOG("Remosaic RDI result is ready");

    return result;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureQuadCFA::StartFullsizeRDISession
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureQuadCFA::StartFullsizeRDISession(
    BOOL restartOfflineSessions)
{
    CDKResult result = CDKResultSuccess;

    CHX_LOG("E, restartOfflineSessions:%d, m_snapshotAppFrameNumber:%d", restartOfflineSessions, m_snapshotAppFrameNumber);

    result = m_pUsecase->ReconfigureRealtimeSession(m_previewSessionId, m_fullSizeRawSessionId, restartOfflineSessions);

    // Flush binning mode RDI queue and release the buffers
    m_pUsecase->FlushRDIQueue(InvalidFrameNum, m_rdiStreamIndex);
    m_pRdiTargetBuffer->pBufferManager->Deactivate(FALSE);

    if (TRUE == m_pUsecase->IsFDBuffersNeeded())
    {
        if ((InvalidStreamIndex != m_fdStreamIndex) && (NULL != m_pFdTargetBuffer))
        {
            m_pUsecase->FlushRDIQueue(InvalidFrameNum, m_fdStreamIndex);
            m_pFdTargetBuffer->pBufferManager->Deactivate(FALSE);
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureQuadCFA::ReStartPreviewSession
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureQuadCFA::ReStartPreviewSession(
    BOOL restartOfflineSessions)
{
    CDKResult result = CDKResultSuccess;

    CHX_LOG("E, restartOfflineSessions:%d", restartOfflineSessions);

    result = m_pUsecase->ReconfigureRealtimeSession(m_fullSizeRawSessionId, m_previewSessionId, restartOfflineSessions);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureQuadCFA::GetRemosaicType
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CHIREMOSAICTYPE FeatureQuadCFA::GetRemosaicType(
    const LogicalCameraInfo* pCamInfo)
{
    CHIREMOSAICTYPE type = CHIREMOSAICTYPE::UnKnown;

    if (NULL != pCamInfo)
    {
        for (UINT i = 0; i < pCamInfo->m_cameraCaps.numSensorModes; i++)
        {
            CHX_LOG("i:%d, sensor mode:%d", i, pCamInfo->pSensorModeInfo[i].sensorModeCaps.value);
            if (CHIREMOSAICTYPE::UnKnown != pCamInfo->pSensorModeInfo[i].remosaictype)
            {
                type = pCamInfo->pSensorModeInfo[i].remosaictype;
                break;
            }
        }
    }
    else
    {
        CHX_LOG_ERROR("GetRemosaicType: pCamInfo is NULL");
    }

    return type;
}
