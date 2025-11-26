////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxfeaturemfsr.cpp
/// @brief CHX mfsr feature class implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_MFSR
#include "chi.h"
#include "chistatsproperty.h"
#include "chxadvancedcamerausecase.h"
#include "chxfeaturemfsr.h"
#include "chxincs.h"

#include "chxusecase.h"

// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files

extern CHICONTEXTOPS g_chiContextOps;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::Create
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FeatureMFSR* FeatureMFSR::Create(
    AdvancedCameraUsecase* pUsecase)
{
    FeatureMFSR* pFeature = CHX_NEW FeatureMFSR;

    if (NULL != pFeature)
    {
        if (CDKResultSuccess != pFeature->Initialize(pUsecase))
        {
            pFeature->Destroy(FALSE);
            pFeature = NULL;
        }
    }

    if (NULL == pFeature)
    {
        CHX_LOG_ERROR("Not able to creaet FeatureMFSR");
    }

    return pFeature;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::Initialize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFSR::Initialize(
    AdvancedCameraUsecase* pUsecase)
{
    CDKResult result = CDKResultSuccess;

    Feature::InitializePrivateResources();

    m_pUsecase          = pUsecase;
    m_pMfsrResultMutex  = Mutex::Create();
    m_pMfsrResultAvailable = Condition::Create();

    m_pApplicationInputMeta =
        allocate_camera_metadata(ReplacedMetadataEntryCapacity, ReplacedMetadataDataCapacity);
    m_pInterStageMetadata   =
        allocate_camera_metadata(ReplacedMetadataEntryCapacity, ReplacedMetadataDataCapacity);

    m_pRDIResultMutex     = Mutex::Create();
    m_pRDIResultAvailable = Condition::Create();

    m_mfsrTotalNumFrames = MfsrDefaultInputFrames;

    /// As now preview keeps going during snapshot,
    /// if the MFSR snpashot reprocess is long, the input RDI buffer could be overwritten by up-coming preivew request.
    /// there's no such issue in normal case, but we can easily meet this problem when frame dump is eanbled.
    /// add a flag here, if need to dump frame, probably need to block upcoming preview.
    m_blockPreviewForSnapshot = FALSE;

    if (TRUE == m_blockPreviewForSnapshot)
    {
        m_pSnapshotResultMutex     = Mutex::Create();
        m_pSnapshotResultAvailable = Condition::Create();
    }

    InitializeInternalStreams(CameraType::Wide);
    if (TRUE == m_pUsecase->IsMultiCameraUsecase())
    {
        InitializeInternalStreams(CameraType::Tele);
    }

    ChxUtils::Memset(&m_preFilterStageResult, 0, sizeof(m_preFilterStageResult));
    ChxUtils::Memset(&m_blendStageResult, 0, sizeof(m_blendStageResult));
    ChxUtils::Memset(&m_postFilterStageResult, 0, sizeof(m_postFilterStageResult));
    ChxUtils::Memset(&m_bpsRegResultBuffer, 0, sizeof(m_bpsRegResultBuffer));

    m_pOfflineRequestMutex           = Mutex::Create();
    CHX_ASSERT(NULL != m_pOfflineRequestMutex);
    m_pOfflineRequestAvailable       = Condition::Create();
    CHX_ASSERT(NULL != m_pOfflineRequestAvailable);

    m_offlineRequestProcessTerminate = FALSE;
    m_aPauseInProgress               = FALSE;

    m_offlineRequestProcessThread.pPrivateData = this;

    m_captureRequest = { 0 };

    result = ChxUtils::ThreadCreate(FeatureMFSR::RequestThread,
                                    &m_offlineRequestProcessThread,
                                    &m_offlineRequestProcessThread.hThreadHandle);
    CHX_ASSERT(CDKResultSuccess != result);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::InitializeInternalStreams
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFSR::InitializeInternalStreams(CameraType type)
{
    for (UINT i = 0; i < MfsrReferenceMax; i++)
    {
        m_pReferenceOutStream[type][i] = (CHISTREAM*)(CHX_CALLOC(sizeof(CHISTREAM)));
        CHX_ASSERT(NULL != m_pReferenceOutStream[type][i]);
        m_pReferenceInStream[type][i]      = (CHISTREAM*)(CHX_CALLOC(sizeof(CHISTREAM)));
        CHX_ASSERT(NULL != m_pReferenceInStream[type][i]);
    }

    m_pPrefilterInStream[type]   = (CHISTREAM*)(CHX_CALLOC(sizeof(CHISTREAM)));
    CHX_ASSERT(NULL != m_pPrefilterInStream[type]);
    m_pPostFilterOutStream[type] = (CHISTREAM*)(CHX_CALLOC(sizeof(CHISTREAM)));
    CHX_ASSERT(NULL != m_pPostFilterOutStream[type]);
    m_pBpsRegOutStream[type]     = (CHISTREAM*)(CHX_CALLOC(sizeof(CHISTREAM)));
    CHX_ASSERT(NULL != m_pBpsRegOutStream[type]);
    m_pBpsRegInStream[type]      = (CHISTREAM*)(CHX_CALLOC(sizeof(CHISTREAM)));
    CHX_ASSERT(NULL != m_pBpsRegInStream[type]);

    CHX_LOG_INFO("FeatureMFSR::InitializeMultiCamera role %d m_pRdiStream: %p",
                 type, m_pRdiStream[type]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::SetupInternalStreams
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFSR::SetupInternalStreams(CameraType type, UINT32 prefilterWidth, UINT32 prefilterHeight)
{
    UINT scale;

    for (UINT i = 0; i < MfsrReferenceMax; i++)
    {
        scale = static_cast<UINT>(pow(4, i));
        UINT32 width  = ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(prefilterWidth, scale) / scale);
        UINT32 height = ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(prefilterHeight, scale) / scale);

        CHISTREAMFORMAT format = (i == MfsrReferenceFull) ? ChiStreamFormatUBWCTP10 : ChiStreamFormatPD10;
        GrallocUsage    grallocUsage = 0;
        if (ChiStreamFormatUBWCTP10 == format)
        {
            grallocUsage = ChiGralloc1ProducerUsageCamera | ChiGralloc1ConsumerUsageCamera |
                ChiGralloc1ProducerUsagePrivate_0 | (1ULL << 27);
        }
        m_pReferenceOutStream[type][i]->format        = format;
        m_pReferenceOutStream[type][i]->width         = width;
        m_pReferenceOutStream[type][i]->height        = height;
        m_pReferenceOutStream[type][i]->maxNumBuffers = 0;
        m_pReferenceOutStream[type][i]->rotation      = StreamRotationCCW0;
        m_pReferenceOutStream[type][i]->streamType    = ChiStreamTypeOutput;
        m_pReferenceOutStream[type][i]->grallocUsage  = grallocUsage;

        *m_pReferenceInStream[type][i]                = *m_pReferenceOutStream[type][i];
        m_pReferenceInStream[type][i]->streamType     = ChiStreamTypeInput;

        if (i == MfsrReferenceFull)
        {
            *m_pPostFilterOutStream[type]        = *m_pReferenceOutStream[type][i];
            m_pPostFilterOutStream[type]->width  = m_pSnapshotStream->width;
            m_pPostFilterOutStream[type]->height = m_pSnapshotStream->height;
            m_pPostFilterOutStream[type]->format = (CHISTREAMFORMAT)HAL_PIXEL_FORMAT_YCrCb_420_SP;
        }
        else if (i == MfsrReferenceDS4)
        {
            *m_pPrefilterInStream[type]            = *m_pReferenceOutStream[type][i];
            m_pPrefilterInStream[type]->streamType = ChiStreamTypeInput;
        }
    }

    UINT32 width = ChxUtils::EvenCeilingUINT32(prefilterWidth / 3);
    UINT32 height = ChxUtils::EvenCeilingUINT32(prefilterHeight / 3);

    m_pBpsRegOutStream[type]->format = ChiStreamFormatYCbCr420_888;
    m_pBpsRegOutStream[type]->width = width;
    m_pBpsRegOutStream[type]->height = height;
    m_pBpsRegOutStream[type]->maxNumBuffers = 0;
    m_pBpsRegOutStream[type]->rotation = StreamRotationCCW0;
    m_pBpsRegOutStream[type]->streamType = ChiStreamTypeOutput;
    m_pBpsRegOutStream[type]->grallocUsage = 0;
    *m_pBpsRegInStream[type] = *m_pBpsRegOutStream[type];
    m_pBpsRegInStream[type]->streamType = ChiStreamTypeInput;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::CreateInternalBuffers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFSR::CreateInternalBuffers(CameraType type)
{
    CDKResult result = CDKResultSuccess;

    for (UINT i = 0; i < MfsrReferenceMax; i++)
    {
        UINT width  = m_pReferenceOutStream[type][i]->width;
        UINT height = m_pReferenceOutStream[type][i]->height;

        CHIBufferManagerCreateData createPreFilterBuffers = { 0 };
        createPreFilterBuffers.width                =
            (m_pReferenceOutStream[type][i]->format == ChiStreamFormatPD10) ? (width * 4) : width;
        createPreFilterBuffers.height               = height;
        createPreFilterBuffers.format               = m_pReferenceOutStream[type][i]->format;
        createPreFilterBuffers.producerFlags        =
            ChiGralloc1ProducerUsageCamera | ChiGralloc1ProducerUsageCpuRead | ChiGralloc1ProducerUsageCpuWrite;
        createPreFilterBuffers.consumerFlags        =
            ChiGralloc1ConsumerUsageCamera | ChiGralloc1ConsumerUsageCpuRead;
        createPreFilterBuffers.immediateBufferCount = 0;
        createPreFilterBuffers.maxBufferCount       = MfsrMaxPreFilterStageBuffers;
        createPreFilterBuffers.bEnableLateBinding   = ExtensionModule::GetInstance()->EnableCHILateBinding();
        createPreFilterBuffers.bufferHeap           = BufferHeapDefault;
        createPreFilterBuffers.pChiStream           = m_pReferenceOutStream[type][i];

        if (ChiStreamFormatUBWCTP10 == m_pReferenceOutStream[type][i]->format)
        {
            createPreFilterBuffers.producerFlags |= ChiGralloc1ProducerUsagePrivate_0 | (1ULL << 27);
            createPreFilterBuffers.consumerFlags |= ChiGralloc1ConsumerUsagePrivate_0 | (1ULL << 27);
        }

        m_pMfsrBufferManager[type][i] = CHIBufferManager::Create(mfsrBufferManagerNames[i], &createPreFilterBuffers);
        if (m_pMfsrBufferManager[type][i] == NULL)
        {
            CHX_LOG_ERROR("[ERROR] m_pMfsrBufferManager MfsrStagePrefilter %d allocate fail!!", i);
            result = CDKResultENoMemory;
        }
        else
        {
            CHX_LOG_INFO("[SUCCESS] m_pMfsrBufferManager MfsrStagePrefilter success");
        }
    }

    CHIBufferManagerCreateData createBpsRegOutBuffers = { 0 };
    createBpsRegOutBuffers.width                = m_pBpsRegOutStream[type]->width;
    createBpsRegOutBuffers.height               = m_pBpsRegOutStream[type]->height;
    createBpsRegOutBuffers.format               = m_pBpsRegOutStream[type]->format;
    createBpsRegOutBuffers.producerFlags        =
        ChiGralloc1ProducerUsageCamera | ChiGralloc1ProducerUsageCpuRead | ChiGralloc1ProducerUsageCpuWrite;
    createBpsRegOutBuffers.consumerFlags        =
        ChiGralloc1ConsumerUsageCamera | ChiGralloc1ConsumerUsageCpuRead;
    createBpsRegOutBuffers.immediateBufferCount = 0;
    createBpsRegOutBuffers.maxBufferCount       = MfsrMaxBpsRegOutBuffers;
    createBpsRegOutBuffers.bEnableLateBinding   = ExtensionModule::GetInstance()->EnableCHILateBinding();
    createBpsRegOutBuffers.bufferHeap           = BufferHeapDefault;
    createBpsRegOutBuffers.pChiStream           = m_pBpsRegOutStream[type];

    m_pMfsrBpsRegOutBufferManager[type] = CHIBufferManager::Create("MfsrBpsRegOutBufferManager", &createBpsRegOutBuffers);

    if (m_pMfsrBpsRegOutBufferManager[type] == NULL) {
        CHX_LOG_ERROR("[ERROR] m_pMfsrBpsRegOutBufferManager allocate fail!!");
        result = CDKResultENoMemory;
    }
    else
    {
        CHX_LOG_INFO("[SUCCESS] m_pMfsrBpsRegOutBufferManager success");
    }

    return result;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::Pause
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFSR::Pause(
    BOOL isForced)
{
    CHX_LOG_INFO("FeatureMfnr::Pause(), isForced %d E.", isForced);
    ChxUtils::AtomicStoreU32(&m_aPauseInProgress, TRUE);
    m_pMfsrResultMutex->Lock();
    m_resultsAvailable = TRUE;
    m_pMfsrResultAvailable->Signal();
    m_pMfsrResultMutex->Unlock();

    m_pOfflineRequestMutex->Lock();
    m_offlineRequestProcessTerminate = TRUE;
    m_pOfflineRequestAvailable->Signal();
    m_pOfflineRequestMutex->Unlock();

    ChxUtils::ThreadTerminate(m_offlineRequestProcessThread.hThreadHandle);
    CHX_LOG_INFO("FeatureMfsr::Pause(), isForced %d X.", isForced);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::Destroy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFSR::Destroy(BOOL isForced)
{
    CHX_LOG("FeatureMFSR::Destroy(), isForced %d E.", isForced);

    m_pSnapshotStream = NULL;
    m_pPreviewStream  = NULL;
    m_pFdStream       = NULL;

    for (int cameraType = CameraType::Wide; cameraType < CameraType::Count; cameraType++)
    {
        m_pRdiStream[cameraType] = NULL;

        for (UINT i = 0; i < MfsrReferenceMax; i++)
        {
            if (m_pReferenceOutStream[cameraType][i] != NULL)
            {
                CHX_FREE(m_pReferenceOutStream[cameraType][i]);
                m_pReferenceOutStream[cameraType][i] = NULL;
            }
            if (m_pReferenceInStream[cameraType][i] != NULL)
            {
                CHX_FREE(m_pReferenceInStream[cameraType][i]);
                m_pReferenceInStream[cameraType][i] = NULL;
            }
        }

        if (m_pPrefilterInStream[cameraType] != NULL)
        {
            CHX_FREE(m_pPrefilterInStream[cameraType]);
            m_pPrefilterInStream[cameraType] = NULL;
        }

        if (m_pPostFilterOutStream[cameraType] != NULL)
        {
            CHX_FREE(m_pPostFilterOutStream[cameraType]);
            m_pPostFilterOutStream[cameraType] = NULL;
        }

        if (m_pBpsRegOutStream[cameraType] != NULL)
        {
            CHX_FREE(m_pBpsRegOutStream[cameraType]);
            m_pBpsRegOutStream[cameraType] = NULL;
        }

        if (m_pBpsRegInStream[cameraType] != NULL)
        {
            CHX_FREE(m_pBpsRegInStream[cameraType]);
            m_pBpsRegInStream[cameraType] = NULL;
        }

        for (UINT j = 0; j < MfsrReferenceMax; j++)
        {
            if (NULL != m_pMfsrBufferManager[cameraType][j])
            {
                m_pMfsrBufferManager[cameraType][j]->Destroy();
                m_pMfsrBufferManager[cameraType][j] = NULL;
            }
        }

        if (m_pMfsrBpsRegOutBufferManager[cameraType] != NULL)
        {
            m_pMfsrBpsRegOutBufferManager[cameraType]->Destroy();
            m_pMfsrBpsRegOutBufferManager[cameraType] = NULL;
        }
    }

    if (m_pInterStageMetadata != NULL)
    {
        ChxUtils::FreeMetaData(m_pInterStageMetadata);
        m_pInterStageMetadata = NULL;
    }

    // Mutex destroy
    if (NULL != m_pMfsrResultMutex)
    {
        m_pMfsrResultMutex->Destroy();
        m_pMfsrResultMutex = NULL;
    }

    // Result destroy
    if (NULL != m_pMfsrResultAvailable)
    {
        m_pMfsrResultAvailable->Destroy();
        m_pMfsrResultAvailable = NULL;
    }

    // Mutex destroy
    if (NULL != m_pRDIResultMutex)
    {
        m_pRDIResultMutex->Destroy();
        m_pRDIResultMutex = NULL;
    }

    // Condition destroy
    if (NULL != m_pRDIResultAvailable)
    {
        m_pRDIResultAvailable->Destroy();
        m_pRDIResultAvailable = NULL;
    }

    // Mutex destroy
    if (NULL != m_pSnapshotResultMutex)
    {
        m_pSnapshotResultMutex->Destroy();
        m_pSnapshotResultMutex = NULL;
    }

    // Condition destroy
    if (NULL != m_pSnapshotResultAvailable)
    {
        m_pSnapshotResultAvailable->Destroy();
        m_pSnapshotResultAvailable = NULL;
    }

    if (NULL != m_pApplicationInputMeta)
    {
        free_camera_metadata(m_pApplicationInputMeta);
        m_pApplicationInputMeta = NULL;
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

    m_pChiUsecase = NULL;
    ChxUtils::AtomicStoreU32(&m_aPauseInProgress, FALSE);

    // free m_captureRequest output_buffers memory
    if (NULL != m_captureRequest.output_buffers)
    {
        CHX_FREE(const_cast<VOID*>(static_cast<const VOID*>(m_captureRequest.output_buffers)));
        m_captureRequest.output_buffers = NULL;
    }

    Feature::DestroyPrivateResources();

    CHX_DELETE(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::GetTargetIndex
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
INT FeatureMFSR::GetTargetIndex(ChiTargetPortDescriptorInfo* pTargets, const char* pTargetName)
{
    INT index = -1;

    for (UINT i = 0; i < pTargets->numTargets; i++)
    {
        if (0 == strcmp(pTargetName, pTargets->pTargetPortDesc[i].pTargetName))
        {
            index = i;
            break;
        }
    }

    return index;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::GetOutputBuffer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFSR::GetOutputBuffer(
    CHIBufferManager*   pBufferManager,
    ChiStream*          pChiStream,
    CHISTREAMBUFFER*    pOutputBuffer)
{
    CDKResult result = CDKResultSuccess;

    pOutputBuffer->size                 = sizeof(CHISTREAMBUFFER);
    pOutputBuffer->bufferInfo           = pBufferManager->GetImageBufferInfo();
    pOutputBuffer->acquireFence.valid   = FALSE;
    pOutputBuffer->pStream              = pChiStream;

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::OverrideUsecase
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ChiUsecase* FeatureMFSR::OverrideUsecase(
    LogicalCameraInfo*              pCameraInfo,
    camera3_stream_configuration_t* pStreamConfig)
{
    (VOID)pCameraInfo;

    CameraType                         type         = CameraType::Wide;
    ChiPipelineTargetCreateDescriptor* pPreviewDesc = NULL;
    INT32                              previewPipelineIndex;

    CHX_LOG("OverrideUsecase for MFSR");

    CHX_ASSERT(2 == pStreamConfig->num_streams);

    m_captureRequest.output_buffers = static_cast<camera3_stream_buffer_t*>(
        CHX_CALLOC(sizeof(camera3_stream_buffer_t) * pStreamConfig->num_streams));

    m_pPreviewStream   = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::PreviewStream);
    m_pRdiStream[type] = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::RdiStream);
    m_pFdStream        = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::FdStream);
    m_pSnapshotStream  = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::SnapshotStream);

    m_pChiUsecase      = m_pUsecase->GetChiUseCase();

    // FEATURE_MULTIFRAME_PORT
    /// @todo - TargetName to ease the matching of streamId vs Target per pipeline. Need to map the rest of internal streams
    for (UINT target = 0; target < m_pChiUsecase->numTargets; target++)
    {
        if (TRUE == StreamIsInternal(m_pChiUsecase->ppChiTargets[target]->pChiStream))
        {
            if (ChiFormatRawMIPI == m_pChiUsecase->ppChiTargets[target]->pBufferFormats[0])
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

    previewPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                AdvancedPipelineType::ZSLPreviewRawType, m_realtimePipelineIndex);
    pPreviewDesc         = &m_pChiUsecase->pPipelineTargetCreateDesc[previewPipelineIndex];

    CHX_LOG("preview idx:%d, previewDesc:%p", previewPipelineIndex, pPreviewDesc);

    SetupInternalPipelines(CameraType::Wide);
    if (TRUE == m_pUsecase->IsMultiCameraUsecase())
    {
        SetupInternalPipelines(CameraType::Tele);
    }

    return m_pChiUsecase;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SetupInternalPipelines
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFSR::SetupInternalPipelines(CameraType type)
{
    m_pRdiStream[type] = m_pUsecase->GetSharedStream(
        (type == CameraType::Wide) ? AdvancedCameraUsecase::RdiStream : AdvancedCameraUsecase::RdiStreamAux);

    SetupInternalMFSRPreFilterPipeline(type);

    SetupInternalMFSRBlendPipeline(type);

    SetupInternalMFSRPostFilterPipeline(type);

    SetupInternalMFSRSnapshotPipeline(type);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SetupInternalMFSRPreFilterPipeline - pre-filter pipeline (#1)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFSR::SetupInternalMFSRPreFilterPipeline(CameraType type)
{
    const CHAR*                        pTargetName = NULL;
    AdvancedPipelineType               pipelineType;
    INT32                              prefilterPipelineIndex;
    ChiPipelineTargetCreateDescriptor* pPrefilterPipelineDesc;
    const CHAR*                        pSourceName = NULL;
    ChiTargetPortDescriptorInfo*       pSinkTarget = NULL;
    ChiTargetPortDescriptorInfo*       pSrcTarget  = NULL;
    UINT                               mfsrBpsRegOutUIndex;

    pipelineType = (type == CameraType::Wide) ?
        AdvancedPipelineType::MFSRPrefilterType : AdvancedPipelineType::MFSRPrefilterAuxType;
    prefilterPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                pipelineType, m_realtimePipelineIndex);
    pPrefilterPipelineDesc = &m_pChiUsecase->pPipelineTargetCreateDesc[prefilterPipelineIndex];

    // Multiframe pipeline-1 (Prefilter)
    UINT mfsrFullOutRefIndex;
    UINT mfsrDs4OutRefIndex;
    UINT mfsrDs4InRefIndex;
    UINT mfsrDs16OutRefIndex;
    UINT mfsrDs64OutRefIndex;

    // Source Port
    pSrcTarget  = &pPrefilterPipelineDesc->sourceTarget;
    pSrcTarget->pTargetPortDesc[0].pTarget->pChiStream = m_pRdiStream[type];

    // Sink Ports
    pSinkTarget = &pPrefilterPipelineDesc->sinkTarget;

    pTargetName = (type == CameraType::Wide) ? "TARGET_BUFFER_MFSR_FULL_OUT_REF" : "TARGET_BUFFER_MFSR_FULL_OUT_REF_AUX";
    mfsrFullOutRefIndex = GetTargetIndex(pSinkTarget, pTargetName);
    pSinkTarget->pTargetPortDesc[mfsrFullOutRefIndex].pTarget->pChiStream = m_pReferenceOutStream[type][MfsrReferenceFull];

    pTargetName = (type == CameraType::Wide) ? "TARGET_BUFFER_MFSR_DS4_OUT_REF" : "TARGET_BUFFER_MFSR_DS4_OUT_REF_AUX";
    mfsrDs4OutRefIndex = GetTargetIndex(pSinkTarget, pTargetName);
    pSinkTarget->pTargetPortDesc[mfsrDs4OutRefIndex].pTarget->pChiStream = m_pReferenceOutStream[type][MfsrReferenceDS4];

    pTargetName = (type == CameraType::Wide) ? "TARGET_BUFFER_MFSR_DS16_OUT_REF" : "TARGET_BUFFER_MFSR_DS16_OUT_REF_AUX";
    mfsrDs16OutRefIndex = GetTargetIndex(pSinkTarget, pTargetName);
    pSinkTarget->pTargetPortDesc[mfsrDs16OutRefIndex].pTarget->pChiStream = m_pReferenceOutStream[type][MfsrReferenceDS16];

    pTargetName = (type == CameraType::Wide) ? "TARGET_BUFFER_MFSR_DS64_OUT_REF" : "TARGET_BUFFER_MFSR_DS64_OUT_REF_AUX";
    mfsrDs64OutRefIndex = GetTargetIndex(pSinkTarget, pTargetName);
    pSinkTarget->pTargetPortDesc[mfsrDs64OutRefIndex].pTarget->pChiStream = m_pReferenceOutStream[type][MfsrReferenceDS64];

    pTargetName = (type == CameraType::Wide) ? "TARGET_BUFFER_MFSR_REG_OUT" : "TARGET_BUFFER_MFSR_REG_OUT_AUX";
    mfsrBpsRegOutUIndex = GetTargetIndex(pSinkTarget, pTargetName);
    pSinkTarget->pTargetPortDesc[mfsrBpsRegOutUIndex].pTarget->pChiStream = m_pBpsRegOutStream[type];
}

VOID FeatureMFSR::SetupInternalMFSRBlendPipeline(CameraType type)
{
    const CHAR*                        pTargetName = NULL;
    AdvancedPipelineType               pipelineType;
    INT32                              blendPipelineIndex;
    ChiPipelineTargetCreateDescriptor* pBlendPipelineDesc;
    ChiTargetPortDescriptorInfo*       pSinkTarget;
    ChiTargetPortDescriptorInfo*       pSrcTarget;
    UINT                               mfsrInFullRefIndex;
    UINT                               mfsrInDs4RefIndex;
    UINT                               mfsrInDs16RefIndex;
    UINT                               mfsrInDs64RefIndex;
    UINT                               mfsrInRawIndex;
    UINT                               mfsrInRegOutIndex;
    UINT                               mfsrFullOutRefIndex;

    pipelineType = (type == CameraType::Wide) ?
        AdvancedPipelineType::MFSRBlendType : AdvancedPipelineType::MFSRBlendAuxType;
    blendPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                            pipelineType, m_realtimePipelineIndex);
    pBlendPipelineDesc = &m_pChiUsecase->pPipelineTargetCreateDesc[blendPipelineIndex];

    // Multiframe pipeline-2 (Blend)
    // Source Port
    pSrcTarget  = &pBlendPipelineDesc->sourceTarget;

    pTargetName = (type == CameraType::Wide) ? "TARGET_BUFFER_RAW" : "TARGET_BUFFER_RAW_AUX";
    mfsrInRawIndex = GetTargetIndex(pSrcTarget, pTargetName);
    pSrcTarget->pTargetPortDesc[mfsrInRawIndex].pTarget->pChiStream = m_pRdiStream[type];

    pTargetName = (type == CameraType::Wide) ? "TARGET_BUFFER_MFSR_FULL_IN_REF" : "TARGET_BUFFER_MFSR_FULL_IN_REF_AUX";
    mfsrInFullRefIndex = GetTargetIndex(pSrcTarget, pTargetName);
    pSrcTarget->pTargetPortDesc[mfsrInFullRefIndex].pTarget->pChiStream = m_pReferenceInStream[type][MfsrReferenceFull];

    pTargetName = (type == CameraType::Wide) ? "TARGET_BUFFER_MFSR_DS4_IN_REF" : "TARGET_BUFFER_MFSR_DS4_IN_REF_AUX";
    mfsrInDs4RefIndex = GetTargetIndex(pSrcTarget, pTargetName);
    pSrcTarget->pTargetPortDesc[mfsrInDs4RefIndex].pTarget->pChiStream = m_pReferenceInStream[type][MfsrReferenceDS4];

    pTargetName = (type == CameraType::Wide) ? "TARGET_BUFFER_MFSR_DS16_IN_REF" : "TARGET_BUFFER_MFSR_DS16_IN_REF_AUX";
    mfsrInDs16RefIndex = GetTargetIndex(pSrcTarget, pTargetName);
    pSrcTarget->pTargetPortDesc[mfsrInDs16RefIndex].pTarget->pChiStream =  m_pReferenceInStream[type][MfsrReferenceDS16];

    pTargetName = (type == CameraType::Wide) ? "TARGET_BUFFER_MFSR_DS64_IN_REF" : "TARGET_BUFFER_MFSR_DS64_IN_REF_AUX";
    mfsrInDs64RefIndex = GetTargetIndex(pSrcTarget, pTargetName);
    pSrcTarget->pTargetPortDesc[mfsrInDs64RefIndex].pTarget->pChiStream = m_pReferenceInStream[type][MfsrReferenceDS64];

    pTargetName = (type == CameraType::Wide) ? "TARGET_BUFFER_MFSR_REG_IN" : "TARGET_BUFFER_MFSR_REG_IN_AUX";
    mfsrInRegOutIndex = GetTargetIndex(pSrcTarget, pTargetName);
    pSrcTarget->pTargetPortDesc[mfsrInRegOutIndex].pTarget->pChiStream = m_pBpsRegInStream[type];

    // Sink Port
    pSinkTarget = &pBlendPipelineDesc->sinkTarget;

    pTargetName = (type == CameraType::Wide) ? "TARGET_BUFFER_MFSR_FULL_OUT_REF" : "TARGET_BUFFER_MFSR_FULL_OUT_REF_AUX";
    mfsrFullOutRefIndex = GetTargetIndex(pSinkTarget, pTargetName);
    pSinkTarget->pTargetPortDesc[mfsrFullOutRefIndex].pTarget->pChiStream = m_pReferenceOutStream[type][MfsrReferenceFull];
}

VOID FeatureMFSR::SetupInternalMFSRPostFilterPipeline(CameraType type)
{
    const CHAR*                        pTargetName = NULL;
    AdvancedPipelineType               pipelineType;
    INT32                              postfilterPipelineIndex;
    ChiPipelineTargetCreateDescriptor* pPostfilterPipelineDesc;
    ChiTargetPortDescriptorInfo*       pSinkTarget = NULL;
    ChiTargetPortDescriptorInfo*       pSrcTarget  = NULL;
    UINT                               mfsrInFullRefIndex;
    UINT                               mfsrInRawIndex;
    UINT                               mfsrInRegOutIndex;

    pipelineType = (type == CameraType::Wide) ?
        AdvancedPipelineType::MFSRPostFilterType : AdvancedPipelineType::MFSRPostFilterAuxType;
    postfilterPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                pipelineType, m_realtimePipelineIndex);

    pPostfilterPipelineDesc = &m_pChiUsecase->pPipelineTargetCreateDesc[postfilterPipelineIndex];

    // Multiframe pipeline-3 (post-filter)
    // Source Ports
    pSrcTarget  = &pPostfilterPipelineDesc->sourceTarget;

    pTargetName = (type == CameraType::Wide) ? "TARGET_BUFFER_RAW" : "TARGET_BUFFER_RAW_AUX";
    mfsrInRawIndex = GetTargetIndex(pSrcTarget, pTargetName);
    pSrcTarget->pTargetPortDesc[mfsrInRawIndex].pTarget->pChiStream = m_pRdiStream[type];

    pTargetName = (type == CameraType::Wide) ? "TARGET_BUFFER_MFSR_FULL_IN_REF" : "TARGET_BUFFER_MFSR_FULL_IN_REF_AUX";
    mfsrInFullRefIndex = GetTargetIndex(pSrcTarget, pTargetName);
    pSrcTarget->pTargetPortDesc[mfsrInFullRefIndex].pTarget->pChiStream = m_pReferenceInStream[type][MfsrReferenceFull];

    pTargetName = (type == CameraType::Wide) ? "TARGET_BUFFER_MFSR_REG_IN" : "TARGET_BUFFER_MFSR_REG_IN_AUX";
    mfsrInRegOutIndex = GetTargetIndex(pSrcTarget, pTargetName);
    pSrcTarget->pTargetPortDesc[mfsrInRegOutIndex].pTarget->pChiStream = m_pBpsRegInStream[type];

    // Sink Port
    pSinkTarget = &pPostfilterPipelineDesc->sinkTarget;
    pSinkTarget->pTargetPortDesc[0].pTarget->pChiStream = m_pPostFilterOutStream[type];
}

VOID FeatureMFSR::SetupInternalMFSRSnapshotPipeline(CameraType type)
{
    const CHAR*                        pTargetName = NULL;
    AdvancedPipelineType               pipelineType;
    INT32                              snapshotPipelineIndex;
    ChiPipelineTargetCreateDescriptor* pSnapshotDesc;
    ChiTargetPortDescriptorInfo*       pSinkTarget = NULL;
    ChiTargetPortDescriptorInfo*       pSrcTarget  = NULL;

    pipelineType = (type == CameraType::Wide) ?
        AdvancedPipelineType::InternalZSLYuv2JpegMFSRType : AdvancedPipelineType::InternalZSLYuv2JpegMFSRAuxType;
    snapshotPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                pipelineType, m_realtimePipelineIndex);

    pSnapshotDesc = &m_pChiUsecase->pPipelineTargetCreateDesc[snapshotPipelineIndex];

    // Multiframe final jpeg pipeline-3 (Jpeg)
    pSnapshotDesc->sourceTarget.pTargetPortDesc[0].pTarget->pChiStream = m_pPostFilterOutStream[type];
    pSnapshotDesc->sinkTarget.pTargetPortDesc[0].pTarget->pChiStream = m_pSnapshotStream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::PipelineCreated
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFSR::PipelineCreated(
    UINT sessionId,          ///< Id of session created
    UINT32 pipelineIndex)    ///< Index of the pipeline created (within the context of the session)
{
    CDKResult                       result        = CDKResultSuccess;
    ChiSensorModeInfo*              pSensorInfo   = NULL;
    const ChiPipelineInputOptions*  pInputOptions = NULL;
    UINT                            pipelineId =
        m_pUsecase->GetSessionData(sessionId)->pipelines[pipelineIndex].id;
    UINT                            scale;

    // FEATURE_MULTIFRAME_PORT
    /// @todo need to find the mapping between session's pipelineId vs XMLs pipelineId
    UINT sessionPipelineId = m_pUsecase->GetSessionData(sessionId)->numPipelines - 1;
    pSensorInfo     = m_pUsecase->GetSessionData(sessionId)->pipelines[sessionPipelineId].pPipeline->GetSensorModeInfo();
    pInputOptions   = m_pUsecase->GetSessionData(sessionId)->pipelines[sessionPipelineId].pPipeline->GetInputOptions();

    CHX_LOG_INFO("MFSR sessionId: %d, pipelineId: %d, sessionPipelineId: %d, "
                 "Sensor widthxheight: %d x %d, InputOptions widthxheight: %d x %d",
                 sessionId,
                 pipelineId,
                 sessionPipelineId,
                 pSensorInfo->frameDimension.width, pSensorInfo->frameDimension.height,
                 pInputOptions->bufferOptions.optimalDimension.width,
                 pInputOptions->bufferOptions.optimalDimension.height);

    // FEATURE_MULTIFRAME_PORT
    /// @todo - Need to find means of associating grallocUsage
    INT32  advPipelineId = m_pUsecase->GetAdvancedPipelineTypeByPipelineId(pipelineId);
    switch (advPipelineId)
    {
    case AdvancedPipelineType::InternalZSLYuv2JpegMFSRType:
    case AdvancedPipelineType::InternalZSLYuv2JpegMFSRAuxType:
        {
            UINT prefilterWidth  = pSensorInfo->frameDimension.width;
            UINT prefilterHeight = pSensorInfo->frameDimension.height;
            CameraType type = CameraType::Wide;
            if (m_pUsecase->GetAdvancedPipelineTypeByPipelineId(pipelineId) ==
                AdvancedPipelineType::InternalZSLYuv2JpegMFSRAuxType)
            {
                type = CameraType::Tele;
            }

            SetupInternalStreams(type, prefilterWidth, prefilterHeight);
            // must call the function after SetupInternalStreams
            result = CreateInternalBuffers(type);
            break;
        }

    case AdvancedPipelineType::MFSRPrefilterType:
    case AdvancedPipelineType::MFSRPrefilterAuxType:
    {
        CameraType type = CameraType::Wide;
        if (m_pUsecase->GetAdvancedPipelineTypeByPipelineId(pipelineId) ==
            AdvancedPipelineType::MFSRPrefilterAuxType)
        {
            type = CameraType::Tele;
        }
        m_pRdiStream[type]->format        = ChiStreamFormatRaw10;
        // Create the RDI stream output based on the input buffer requirements to generate the Bayer stream buffer
        m_pRdiStream[type]->width         = pSensorInfo->frameDimension.width;
        m_pRdiStream[type]->height        = pSensorInfo->frameDimension.height;
        m_pRdiStream[type]->maxNumBuffers = 0;
        m_pRdiStream[type]->rotation      = StreamRotationCCW90;
        m_pRdiStream[type]->streamType    = ChiStreamTypeOutput;
        m_pRdiStream[type]->grallocUsage  = 0;

        m_pUsecase->ConfigFdStream();

        break;
    }
    case AdvancedPipelineType::MFSRBlendType:
    case AdvancedPipelineType::MFSRBlendAuxType:
    case AdvancedPipelineType::MFSRPostFilterType:
    case AdvancedPipelineType::MFSRPostFilterAuxType:
        break;

    default:
        {
            CHX_LOG_INFO("[INFO] Unknown advanced pipeline type %d", advPipelineId);
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::ExecuteProcessRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult  FeatureMFSR::ExecuteProcessRequest(
    camera3_capture_request_t* pRequest)              ///< Request parameters
{
    CDKResult                 result             = CDKResultSuccess;
    CHISTREAMBUFFER           previewBuffers[3]  = {{0}};
    UINT                      previewCount       = 0;
    UINT                      snapshotCount      = 0;
    UINT32                    frameNumber        = pRequest->frame_number;
    UINT32                    frameIndex         = (pRequest->frame_number % MaxOutstandingRequests);
    UINT                      snapshotReqIdIndex = (m_maxSnapshotReqId % MaxOutstandingRequests);
    CameraType                type               = CameraType::Wide;

    if (TRUE == m_pUsecase->IsMultiCameraUsecase())
    {
        RequestMapInfo requestMapInfo = { 0 };

        m_internalRequestId  = pRequest->frame_number;
        requestMapInfo       = m_pUsecase->GetRequestMapInfo(pRequest->frame_number);
        m_requestFrameNumber = requestMapInfo.frameNumber;

        CHX_LOG("FeatureMFSR internalReqId:%d, appFrameNum:%d", m_internalRequestId, m_requestFrameNumber);

        if (0 != m_activeCameraId)
        {
            type = CameraType::Tele;
        }

        m_sensorModeIndex = m_pUsecase->GetRealtimeSession()->GetSensorModeInfo(type)->modeIndex;
    }

    if (TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress)))
    {
        m_resultsAvailable = FALSE;
        m_pMfsrResultMutex->Unlock();
        CHX_LOG_INFO("MFSR pre filter scale process return because of cleanup");
        return CDKResultSuccess;
    }

    if (FALSE == m_pUsecase->IsMultiCameraUsecase())
    {
        camera3_capture_result_t* pUsecaseResult     = m_pUsecase->GetCaptureResult(frameIndex);

        pUsecaseResult->result             = NULL;
        pUsecaseResult->frame_number       = pRequest->frame_number;
        pUsecaseResult->num_output_buffers = 0;

        m_isSnapshotFrame[frameIndex]     = FALSE;
        m_sensorModeIndex = m_pUsecase->GetSessionData(m_realtime)->pSession->GetSensorModeInfo()->modeIndex;

        ChxUtils::FillTuningModeData(const_cast<camera_metadata_t*>(pRequest->settings),
            pRequest,
            m_sensorModeIndex,
            m_pUsecase->GetEffectMode(),
            m_pUsecase->GetSceneMode(),
            m_pUsecase->GetFeature1Mode(),
            m_pUsecase->GetFeature2Mode());
        ChxUtils::FillCameraId(const_cast<camera_metadata_t*>(pRequest->settings), m_pUsecase->GetCameraId());

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
                ChxUtils::FreeMetaData(m_pSnapshotInputMeta[snapshotReqIdIndex]);
                m_pSnapshotInputMeta[snapshotReqIdIndex]    =
                    ChxUtils::AllocateCopyMetaData(reinterpret_cast<const VOID*>(pRequest->settings));

                CHX_LOG("Snapshot Frame %d", pRequest->frame_number);
                m_requestFrameNumber = pRequest->frame_number;

                ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i],
                                                       &m_snapshotBuffers[snapshotReqIdIndex][snapshotCount]);

                UINT32 reqId = m_pRdiTargetBuffer->lastReadySequenceID;
                m_mfsrTotalNumFrames = GetMFSRTotalFramesByGain(m_pRdiTargetBuffer->bufferQueue[reqId % BufferQueueDepth].pMetadata);
                CHX_LOG("lastReadySequenceID:%d, m_mfsrTotalNumFrames:%d", reqId, m_mfsrTotalNumFrames);

                snapshotCount++;
                m_allRDIResultsAvaliable = FALSE;
                m_allFDResultsAvaliable  = FALSE;
                m_isLLSSnapshot          = FALSE;
            }
        }

        PipelineData* pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_realtime, 0));
        UINT          requestIdIndex = (pPipelineData->seqId % MaxOutstandingRequests);

        pPipelineData->seqIdToFrameNum[requestIdIndex] = frameNumber;

        CHX_LOG("Realtime AppFrameNum to ReqId: %d <--> %d", frameNumber, pPipelineData->seqId);

        result = m_pUsecase->GetOutputBufferFromRDIQueue(pPipelineData->seqId, m_rdiStreamIndex, &previewBuffers[previewCount]);
        if (CDKResultSuccess == result)
        {
            previewCount++;
        }
        else
        {
            CHX_LOG_ERROR("FeatureMFSR: GetOutputBufferFromRDIQueue failed for frameNumber %d", frameNumber);
        }

        result = m_pUsecase->GetOutputBufferFromFDQueue(pPipelineData->seqId, m_fdStreamIndex, &previewBuffers[previewCount]);

        if (CDKResultSuccess == result)
        {
            previewCount++;
        }
        else
        {
            CHX_LOG_ERROR("FeatureMFNR: GetImageBuffer failed, No FD buffer for frameNumber %d", frameNumber);
        }

        const Session* pSession = m_pUsecase->GetSessionData(m_realtime)->pSession;

        CHICAPTUREREQUEST request       = { 0 };
        request.frameNumber             = pPipelineData->seqId++;
        request.hPipelineHandle         = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
        request.numOutputs              = previewCount;
        request.pOutputBuffers          = previewBuffers;
        request.pMetadata               = pRequest->settings;
        request.pPrivData               = &m_privData[request.frameNumber % MaxOutstandingRequests];
        request.pPrivData->featureType  = FeatureType::MFSR;

        CHIPIPELINEREQUEST submitRequest = { 0 };
        submitRequest.pSessionHandle     = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
        submitRequest.numRequests        = 1;
        submitRequest.pCaptureRequests   = &request;

        m_pUsecase->SetRequestToFeatureMapping(m_realtime, request.frameNumber, this);
        if (FALSE == pSession->IsPipelineActive())
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

        result = m_pUsecase->SubmitRequest(&submitRequest);

        if ((0 < snapshotCount) && (TRUE == m_pUsecase->IsLLSNeeded()))
        {
            // set m_isLLSSnapshot to TRUE here, check this flag all through the code,
            // because m_pUsecase->IsLLSNeeded() might change during the snapshot.
            m_isLLSSnapshot = TRUE;

            CHX_LOG_INFO("FeatureMFSR Trigger LLS snapshot");
            result = TriggerInternalLLSRequests(pRequest);
        }

        if ((0 < snapshotCount) && (FALSE == m_pUsecase->IsLLSNeeded()))
        {
            m_allRDIResultsAvaliable = TRUE;
            m_allFDResultsAvaliable  = TRUE;
        }
    }
    else
    {
        for (UINT32 i = 0; i < pRequest->num_output_buffers; i++)
        {
            if (m_pSnapshotStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream))
            {
                ChxUtils::FreeMetaData(m_pSnapshotInputMeta[snapshotReqIdIndex]);
                m_pSnapshotInputMeta[snapshotReqIdIndex] =
                    ChxUtils::AllocateCopyMetaData(reinterpret_cast<const VOID*>(pRequest->settings));

                CHX_LOG("Snapshot Frame %d", pRequest->frame_number);
                ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i],
                                                       &m_snapshotBuffers[snapshotReqIdIndex][snapshotCount]);
                snapshotCount++;
            }
        }

        if ((0 == snapshotCount) && (NULL != pRequest->output_buffers))
        {
            // this means the final snapshot buffer is not included in the request from usecasemc
            // fallback to use snapshot buffer saved during GetRequestInfo()
            pRequest->num_output_buffers = 1;
            ChxUtils::PopulateChiToHALStreamBuffer(&m_snapshotBuffers[snapshotReqIdIndex][0],
                                                   const_cast<camera3_stream_buffer_t*>(&pRequest->output_buffers[0]));

            CHX_LOG("use previous saved snapshot buffer");

            snapshotCount++;
        }
    }

    if ((CDKResultSuccess == result) && (0 < snapshotCount))
    {
        CHX_LOG_INFO("Snapshot in MFSR HW Multiframe");

        m_pOfflineRequestMutex->Lock();
        m_isSnapshotFrame[frameIndex]                 = TRUE;
        m_snapshotBufferNum[snapshotReqIdIndex]       = snapshotCount;
        m_snapshotReqIdToFrameNum[snapshotReqIdIndex] = frameNumber;
        // Do Deep Copy if framework uses same request for future requests
        ChxUtils::DeepCopyCamera3CaptureRequest(pRequest, &m_captureRequest);

        if (FALSE == m_isLLSSnapshot)
        {
            CHX_LOG("FeatureMFSR Non-LLS signal request thead");
            m_pOfflineRequestAvailable->Signal();
        }

        m_pOfflineRequestMutex->Unlock();

        if ((FALSE == m_pUsecase->IsMultiCameraUsecase()) && (TRUE == m_isLLSSnapshot))
        {
            if (TRUE == m_blockPreviewForSnapshot)
            {
                CHX_LOG("FeatureMFSR LLS wait final snapshot ready");
                m_pSnapshotResultMutex->Lock();
                while (FALSE == m_snapshotResultAvailable)
                {
                    m_pSnapshotResultAvailable->Wait(m_pSnapshotResultMutex->GetNativeHandle());
                }
                m_snapshotResultAvailable = FALSE;
                m_pSnapshotResultMutex->Unlock();
                CHX_LOG("FeatureMFSR LLS snapshot ready, accept new requests");
            }
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::TriggerInternalLLSRequests
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFSR::TriggerInternalLLSRequests(
    camera3_capture_request_t*  pRequest)
{
    CDKResult result = CDKResultSuccess;
    UINT   internalrequest = 0;

    // already sumbmit one request with preview + RDI, count that one also.
    UINT32                   totalFramesRequired    = GetRequiredInputFrames();
    UINT32                   internalRequestsNum    = totalFramesRequired - 1;
    const UINT32             maxInternalRequestsNum = MfsrMaxInputRDIFrames - 1;
    const camera_metadata_t* pLLSSettings[maxInternalRequestsNum] = { 0 };

    CHX_LOG("FeatureMFSR TriggerInternalLLSRequests(), total required RDI frames:%d", totalFramesRequired);

    if (maxInternalRequestsNum < internalRequestsNum)
    {
        CHX_LOG_ERROR("invalide internalRequestsNum:%d", internalRequestsNum);
        result = CDKResultEInvalidArg;
    }

    if (CDKResultSuccess == result)
    {
        /// Generate internal request settings
        result = GenerateLLSRequestSettings(pRequest->settings,
                                            internalRequestsNum,
                                            &pLLSSettings[0]);

        const Session* pSession       = m_pUsecase->GetSessionData(m_realtime)->pSession;
        PipelineData*  pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_realtime, 0));
        m_allRDIResultsAvaliable      = FALSE;
        m_allFDResultsAvaliable       = FALSE;

        /// Submit request(s) to realtime pipeline
        for (UINT32 i = 0; i < internalRequestsNum; i++)
        {
            /// update seqId <-> appFrameNum mapping
            UINT reqIdIdx = (pPipelineData->seqId % MaxOutstandingRequests);
            pPipelineData->seqIdToFrameNum[reqIdIdx] = pRequest->frame_number;

            /// get RDI buffer
            CHISTREAMBUFFER outputBuffers[2] = { { 0 } };
            internalrequest                  = 0;

            result = m_pUsecase->GetOutputBufferFromRDIQueue(pPipelineData->seqId, m_rdiStreamIndex, &outputBuffers[internalrequest]);
            if (CDKResultSuccess == result)
            {
                internalrequest++;
            }
            else
            {
                CHX_LOG_ERROR("FeatureMFSR: GetOutputBufferFromRDIQueue failed for frameNumber %d", pRequest->frame_number);
            }

            result = m_pUsecase->GetOutputBufferFromFDQueue(pPipelineData->seqId, m_fdStreamIndex, &outputBuffers[internalrequest]);

            if (CDKResultSuccess == result)
            {
                internalrequest++;
            }
            else
            {
                CHX_LOG_ERROR("FeatureMFSR: GetOutputBufferFromFDQueue failed for frameNumber %d", pRequest->frame_number);
            }

            if (CDKResultSuccess == result)
            {
                CHX_LOG("FeatureMFSR submit internal RDI requet AppFrameNum:%d <--> rtReqId:%d",
                    pRequest->frame_number, pPipelineData->seqId);

                CHICAPTUREREQUEST request       = { 0 };
                request.frameNumber             = pPipelineData->seqId++;
                request.hPipelineHandle         = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
                request.numOutputs              = internalrequest; // RDI and FD buffer
                request.pOutputBuffers          = outputBuffers;
                request.pMetadata               = pLLSSettings[i];
                request.pPrivData               = &m_privData[request.frameNumber % MaxOutstandingRequests];
                request.pPrivData->featureType  = FeatureType::MFSR;

                CHIPIPELINEREQUEST submitRequest = { 0 };
                submitRequest.pSessionHandle     = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
                submitRequest.numRequests        = 1;
                submitRequest.pCaptureRequests   = &request;

                m_pUsecase->SetRequestToFeatureMapping(m_realtime, request.frameNumber, this);
                result = m_pUsecase->SubmitRequest(&submitRequest);

                if (CDKResultSuccess != result)
                {
                    CHX_LOG_ERROR("TriggerInternalLLSRequests SubmitRequest failed!");
                    break;
                }
            }
        }

        if (CDKResultSuccess == result)
        {
            UINT32 lastReqId = pPipelineData->seqId - 1;

            m_triggerMFSRReprocess[lastReqId % MaxOutstandingRequests] = TRUE;

            CHX_LOG("lastReqId:%d, set m_triggerMFSRReprocess[%d] to TRUE", lastReqId, lastReqId % MaxOutstandingRequests);
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::GenerateLLSRequestSettings
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFSR::GenerateLLSRequestSettings(
    const camera_metadata_t*    pInputSetting,
    UINT32                      numFrames,
    const camera_metadata_t**   pOutputSettingArray)
{
    CDKResult result = CDKResultSuccess;

    for (UINT32 i = 0; i < numFrames; i++)
    {
        // settings are same for all the RDI request, so simple pointing to the original meta,
        // no copy here, so no need to care about free metadata
        pOutputSettingArray[i] = pInputSetting;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::WaitRDIResultsReady
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFSR::WaitForRDIResultsReady()
{
    // TODO: Utilize WaitForBufferMetaReady API and get rid of m_pRDIResultMutex and m_pRDIResultAvailable
    CDKResult result = CDKResultSuccess;

    CHX_LOG("FeatureMFSR LLS wait for RDI buffers/metadatas ready");

    m_pRDIResultMutex->Lock();
    while (FALSE == m_allRDIResultsAvaliable)
    {
        m_pRDIResultAvailable->Wait(m_pRDIResultMutex->GetNativeHandle());
    }
    m_allRDIResultsAvaliable = FALSE;
    m_pRDIResultMutex->Unlock();

    CHX_LOG("FeatureMFSR LLS RDI buffers/metadatas are ready");

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::GetRequestInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFSR::GetRequestInfo(
    camera3_capture_request_t*  pRequest,
    FeatureRequestInfo*         pOutputRequests,
    FeatureRequestType          requestType)
{
    CDKResult result = CDKResultSuccess;

    ChxUtils::ResetMetadata(m_pApplicationInputMeta);
    ChxUtils::MergeMetadata(const_cast<camera_metadata_t*>(pRequest->settings), m_pApplicationInputMeta);

    UINT32 snapshotReqIdIndex = (m_maxSnapshotReqId % MaxOutstandingRequests);

    for (UINT32 i = 0; i < pRequest->num_output_buffers; i++)
    {
        if (m_pSnapshotStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream))
        {
            CHX_LOG("Copy Snapshot buffer, appFrameNum %d", pRequest->frame_number);

            ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i], &m_snapshotBuffers[snapshotReqIdIndex][0]);
         }
    }

    if (FeatureRequestType::LLS == requestType)
    {
        UINT32                   totalFramesRequired                    = GetRequiredInputFrames();
        const camera_metadata_t* pLLSSettings[MfsrMaxInputRDIFrames]    = { 0 };

        CHX_LOG("FeatureMFSR GetRequestInfo(), total required RDI frames:%d", totalFramesRequired);

        if (MfsrMaxInputRDIFrames < totalFramesRequired)
        {
            CHX_LOG_ERROR("invalide totalFramesRequired:%d", totalFramesRequired);
            result = CDKResultEInvalidArg;
        }

        if (CDKResultSuccess == result)
        {
            result = GenerateLLSRequestSettings(m_pApplicationInputMeta,
                                                totalFramesRequired,
                                                &pLLSSettings[0]);
        }

        if ((CDKResultSuccess == result) && (NULL != pOutputRequests))
        {
            ChxUtils::Memset(pOutputRequests, 0, sizeof(FeatureRequestInfo));

            for (UINT32 i = 0; i < totalFramesRequired; i++)
            {
                ChxUtils::Memcpy(&pOutputRequests->request[i], pRequest, sizeof(camera3_capture_request));
                pOutputRequests->request[i].settings = pLLSSettings[i];
            }

            pOutputRequests->numOfRequest      = totalFramesRequired;
            pOutputRequests->isReturnResult[0] = TRUE;

            CHX_LOG("total input RDI frame required:%d, isReturnResult[0]:%d",
                totalFramesRequired,
                pOutputRequests->isReturnResult[0]);
            m_isLLSSnapshot = TRUE;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::CreateMFSRInputInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFSR::CreateMFSRInputInfo(
    MFSRInputInfo*             pInputInfo,
    camera3_capture_request_t* pRequest)
{
    CDKResult result                = CDKResultSuccess;
    UINT32 internalFrameNumber      = pRequest->frame_number;
    UINT32 offset                   = 0;
    UINT32 zslQueueIndex            = 0;
    FeatureRequestType  requestType = FeatureRequestType::COMMON;

    if (TRUE == m_isLLSSnapshot)
    {
        requestType = FeatureRequestType::LLS;
    }

    pInputInfo->numOfBuffers     = m_mfsrTotalNumFrames;

    if (FALSE == m_pUsecase->IsMultiCameraUsecase())
    {
        internalFrameNumber = m_pRdiTargetBuffer->lastReadySequenceID;

        GetRequestInfo(pRequest, NULL, requestType);

        for (UINT32 i = 0; i< pInputInfo->numOfBuffers; i++)
        {
            offset    = i;
            if (offset > internalFrameNumber)
            {
                offset = internalFrameNumber;
            }

            result = m_pUsecase->WaitForBufferMetaReady(internalFrameNumber - offset, m_rdiStreamIndex);

            if (CDKResultSuccess == result)
            {
                m_pUsecase->GetTargetBuffer(internalFrameNumber - offset,
                                            m_pRdiTargetBuffer,
                                            NULL,
                                            &(pInputInfo->bufferArray[i]));

                m_pUsecase->GetTargetBuffer(internalFrameNumber - offset,
                    m_pFdTargetBuffer,
                    NULL,
                    &(pInputInfo->fdbufferArray[i]));

                // Dynamically decrementing ValidBufferLength by one during MFSR so less RDI/FD buffers will be reserved in the queue.
                // It will be increased once the input RDIs are released after each stage.
                m_pUsecase->UpdateValidRDIBufferLength(m_rdiStreamIndex, -1);
                m_pUsecase->UpdateValidFDBufferLength(m_fdStreamIndex, -1);

                zslQueueIndex            = ((internalFrameNumber - offset) % BufferQueueDepth);
                pInputInfo->pMetadata[i] = ChxUtils::AllocateCopyMetaData(m_pRdiTargetBuffer->bufferQueue[zslQueueIndex].pMetadata);
            }
            else
            {
                CHX_LOG_ERROR("FeatureMFNR: wait rdi and meta timeout! frameNumber=%d, InputFrameNumber=%d",
                              pRequest->frame_number, internalFrameNumber - offset);
                break;
            }
        }

        if (CDKResultSuccess == result)
        {
            // Flush the RDI/FD queue as all the buffers of frameNumber older than 'internalFrameNumber - offset' won't be needed anymore
            m_pUsecase->FlushRDIQueue(internalFrameNumber - offset, m_rdiStreamIndex);
            m_pUsecase->FlushFDQueue(internalFrameNumber - offset, m_fdStreamIndex);
        }
    }
    else
    {
        RequestMapInfo requestInfo = m_pUsecase->GetRequestMapInfo(internalFrameNumber);
        m_activeCameraId           = requestInfo.masterCameraID;
        m_activePipelineIndex      = requestInfo.activePipelineID;
        for (UINT32 i = 0; i< pInputInfo->numOfBuffers; i++)
        {
            pInputInfo->pMetadata[i] = NULL;
            m_pUsecase->GetInputBufferFromRDIQueue(internalFrameNumber,
                                                   m_activePipelineIndex,
                                                   i,
                                                   &(pInputInfo->bufferArray[i]),
                                                   reinterpret_cast<camera_metadata_t**>(&(pInputInfo->pMetadata[i])),
                                                   FALSE);

            m_pUsecase->GetInputBufferFromFDQueue(internalFrameNumber,
                m_activePipelineIndex,
                i,
                &(pInputInfo->fdbufferArray[i]),
                FALSE);
        }
    }

    // dump MFSR input buffer/meta info
    for (UINT32 i = 0; i < pInputInfo->numOfBuffers; ++i)
    {
        CHX_LOG("[%d/%d] stream:%p, buffer handle:%p, meta:%p, size:%d",
            i, pInputInfo->numOfBuffers,
            pInputInfo->bufferArray[i].pStream,
            &pInputInfo->bufferArray[i].bufferInfo.phBuffer,
            pInputInfo->pMetadata[i],
            pInputInfo->bufferArray[i].size);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::SubmitOfflineMfsrPrefilterRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFSR::SubmitOfflineMfsrPrefilterRequest(
    UINT32                     appFrameNumber,
    camera3_capture_request_t* pRequest,
    MFSRInputInfo*             pMfsrInputInfo,
    CameraType                 type)
{
    CDKResult               result                             = CDKResultSuccess;
    CHISTREAMBUFFER         inputBuffers[MaxChiStreamBuffers]  = { { 0 } };
    CHISTREAMBUFFER         outputBuffers[MaxChiStreamBuffers] = { { 0 } };
    camera_metadata_t*      pMergedMetadata                    = NULL;
    UINT32                  numstageInputs                     = 0;
    UINT32                  numStageOutputs                    = 0;
    UINT                    outputIndex                        = 0;
    UINT                    inputIndex                         = 0;
    VOID*                   pRequestMetadata                   = NULL;
    UINT32                  feature2Mode;

    CHX_LOG_INFO("--------------------------- Mfsr pre-filter stage -------------------------- ");
    numstageInputs                   = 1;
    numStageOutputs                  = 5;  // BPS Reg Out, Full/DS4/DS16/DS64
    m_remainingPrefilterStageResults = 1;
    m_numExpectedStageBuffers        = numStageOutputs;
    outputIndex                      = 0;
    inputIndex                       = 0;
    ChxUtils::Memset(&m_preFilterStageResult, 0, sizeof(m_preFilterStageResult));

    ChxUtils::Memcpy(&inputBuffers[0], &pMfsrInputInfo->bufferArray[0], sizeof(CHISTREAMBUFFER));

    // Release FD Buffers
    if (FALSE == m_pUsecase->IsMultiCameraUsecase())
    {
        for (UINT32 i = 0; i < pMfsrInputInfo->numOfBuffers; ++i)
        {
            m_pFdTargetBuffer->pBufferManager->ReleaseReference(
                (&pMfsrInputInfo->fdbufferArray[i].bufferInfo));
            m_pUsecase->UpdateValidFDBufferLength(m_fdStreamIndex, 1);
        }
    }
    else
    {
        for (UINT32 i = 0; i < pMfsrInputInfo->numOfBuffers; ++i)
        {
            m_pUsecase->ReleaseSingleOffineFDInputResource(pRequest->frame_number, m_activePipelineIndex, i);
        }
    }

    if ((NULL != m_pApplicationInputMeta) && (NULL != pMfsrInputInfo->pMetadata[0]))
    {
        CHITAGSOPS tagOps = { 0 };
        g_chiContextOps.pTagOps(&tagOps);
        UINT8 preFilterFlashState = 0;

        // FlashState needs to be stored at prefilter stage
        // and use during snapshot request
        tagOps.pGetMetaData((CHIHANDLE)pMfsrInputInfo->pMetadata[0],
            ANDROID_FLASH_STATE,
            &preFilterFlashState,
            sizeof(preFilterFlashState));
        tagOps.pSetMetaData((CHIHANDLE)m_pApplicationInputMeta,
            ANDROID_FLASH_STATE,
            &preFilterFlashState,
            sizeof(preFilterFlashState));
    }

    ChxUtils::ResetMetadata(m_pInterStageMetadata);
    if (NULL != m_pApplicationInputMeta)
    {
        ChxUtils::MergeMetadata(m_pApplicationInputMeta, m_pInterStageMetadata);
    }
    ChxUtils::MergeMetadata(pMfsrInputInfo->pMetadata[0], m_pInterStageMetadata);

    GetOutputBuffer(m_pMfsrBufferManager[type][MfsrReferenceFull],
        m_pReferenceOutStream[type][MfsrReferenceFull],
        &outputBuffers[outputIndex++]);

    GetOutputBuffer(m_pMfsrBufferManager[type][MfsrReferenceDS4],
        m_pReferenceOutStream[type][MfsrReferenceDS4],
        &outputBuffers[outputIndex++]);

    GetOutputBuffer(m_pMfsrBufferManager[type][MfsrReferenceDS16],
        m_pReferenceOutStream[type][MfsrReferenceDS16],
        &outputBuffers[outputIndex++]);

    GetOutputBuffer(m_pMfsrBufferManager[type][MfsrReferenceDS64],
        m_pReferenceOutStream[type][MfsrReferenceDS64],
        &outputBuffers[outputIndex++]);

    GetOutputBuffer(m_pMfsrBpsRegOutBufferManager[type], m_pBpsRegOutStream[type], &outputBuffers[outputIndex++]);

    // Use MFNRBlend. Depends on prescale / postscale ratio to determine
    // whether MFNR or MFSR tuning data is used.
    feature2Mode = static_cast<UINT32>(ChiModeFeature2SubModeType::MFNRBlend);
    ChxUtils::FillTuningModeData(m_pInterStageMetadata,
        pRequest,
        m_sensorModeIndex,
        m_pUsecase->GetEffectMode(),
        m_pUsecase->GetSceneMode(),
        m_pUsecase->GetFeature1Mode(),
        &feature2Mode);

    PublicMFSRTotalFrames(m_pInterStageMetadata);
    m_resultsAvailable = FALSE;
    ExecuteMfsrRequest(MfsrStagePrefilter,
        appFrameNumber,
        numStageOutputs,
        &outputBuffers[0],
        numstageInputs,
        &inputBuffers[0],
        m_pInterStageMetadata);

    m_pMfsrResultMutex->Lock();

    // Wait for all prefilter results to come back
    // 5 output buffers: Full, DS4, DS16, DS64, BpsRegOut and Metadata
    while (FALSE == m_resultsAvailable)
    {
        m_pMfsrResultAvailable->Wait(m_pMfsrResultMutex->GetNativeHandle());
    }

    if (TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress)))
    {
        m_resultsAvailable = FALSE;
        m_pMfsrResultMutex->Unlock();
        CHX_LOG_INFO("MFSR prefilter process return because of cleanup");
        return CDKResultENoMore;
    }

    if (FALSE == m_pUsecase->IsMultiCameraUsecase())
    {
        CHX_LOG("Releasing Reference RDI Buffer %p for frame 0", &pMfsrInputInfo->bufferArray[0].bufferInfo);
        m_pRdiTargetBuffer->pBufferManager->ReleaseReference(
            &pMfsrInputInfo->bufferArray[0].bufferInfo);
        m_pUsecase->UpdateValidRDIBufferLength(m_rdiStreamIndex, 1);
    }
    else
    {
        m_pUsecase->ReleaseSingleOffineInputResource(pRequest->frame_number, m_activePipelineIndex, 0);
    }

    m_resultsAvailable = FALSE;
    m_isLLSSnapshot    = FALSE;

    m_pMfsrResultMutex->Unlock();

    // Deactivate pre-filter pipeline
    CHX_LOG_INFO("Deactivate MfsrStagePrefilter - DeactivateModeReleaseBuffer");
    Session* pOfflineSession = m_pUsecase->GetSessionData(m_offline)->pSession;
    result = ExtensionModule::GetInstance()->DeactivatePipeline(pOfflineSession->GetSessionHandle(),
        pOfflineSession->GetPipelineHandle(GetPipelineIndex(MfsrStagePrefilter)), CHIDeactivateModeReleaseBuffer);

    if (CDKResultSuccess != result)
    {
        CHX_LOG_ERROR("Deactivate MfsrStagePrefilter failed!");
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::SubmitOfflineMfsrBlendRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFSR::SubmitOfflineMfsrBlendRequest(
    UINT32                     appFrameNumber,
    camera3_capture_request_t* pRequest,
    MFSRInputInfo*             pMfsrInputInfo,
    CameraType                 type)

{
    CDKResult               result                             = CDKResultSuccess;
    CHISTREAMBUFFER         inputBuffers[MaxChiStreamBuffers]  = { { 0 } };
    CHISTREAMBUFFER         outputBuffers[MaxChiStreamBuffers] = { { 0 } };
    camera_metadata_t*      pMergedMetadata                    = NULL;
    UINT32                  numstageInputs                     = 0;
    UINT32                  numStageOutputs                    = 0;
    UINT                    outputIndex                        = 0;
    UINT                    inputIndex                         = 0;
    VOID*                   pRequestMetadata                   = NULL;
    UINT32                  feature2Mode;

    for (UINT i = 0; i < (m_mfsrTotalNumFrames - 2); i++)
    {
        CHX_LOG_INFO("------------------------------- Mfsr blend stage ----------------------------");
        numstageInputs               = 6;               // Rdi, BpsRegOut, PreFilter Full/DS4/DS16/DS64
        numStageOutputs              = 1;               // Full
        m_numExpectedStageBuffers    = numStageOutputs; // Full
        outputIndex                  = 0;
        inputIndex                   = 0;
        m_remainingBlendStageResults = 1;

        ChxUtils::Memset(&m_blendStageResult, 0, sizeof(m_blendStageResult));

        if (NULL != m_pApplicationInputMeta)
        {
            ChxUtils::MergeMetadata(m_pApplicationInputMeta, m_pInterStageMetadata);
        }
        ChxUtils::MergeMetadata(pMfsrInputInfo->pMetadata[i + 1], m_pInterStageMetadata);

        ChxUtils::Memcpy(&inputBuffers[inputIndex++], &pMfsrInputInfo->bufferArray[i + 1],
            sizeof(CHISTREAMBUFFER));

        ChxUtils::Memcpy(&inputBuffers[inputIndex], &m_bpsRegResultBuffer, sizeof(CHISTREAMBUFFER));
        inputBuffers[inputIndex].acquireFence.valid = FALSE;
        inputBuffers[inputIndex].size               = sizeof(CHISTREAMBUFFER);
        inputBuffers[inputIndex++].pStream          = m_pBpsRegInStream[type];

        ChxUtils::Memcpy(&inputBuffers[inputIndex],
            &m_preFilterStageResult.refOutputBuffer,
            sizeof(CHISTREAMBUFFER));
        inputBuffers[inputIndex].acquireFence.valid = FALSE;
        inputBuffers[inputIndex].size               = sizeof(CHISTREAMBUFFER);
        inputBuffers[inputIndex++].pStream          = m_pReferenceInStream[type][MfsrReferenceFull];

        ChxUtils::Memcpy(&inputBuffers[inputIndex],
            &m_preFilterStageResult.refOutputBuffer[MfsrReferenceDS4],
            sizeof(CHISTREAMBUFFER));
        inputBuffers[inputIndex].acquireFence.valid = FALSE;
        inputBuffers[inputIndex].size               = sizeof(CHISTREAMBUFFER);
        inputBuffers[inputIndex++].pStream          = m_pReferenceInStream[type][MfsrReferenceDS4];

        ChxUtils::Memcpy(&inputBuffers[inputIndex],
            &m_preFilterStageResult.refOutputBuffer[MfsrReferenceDS16],
            sizeof(CHISTREAMBUFFER));
        inputBuffers[inputIndex].acquireFence.valid = FALSE;
        inputBuffers[inputIndex].size               = sizeof(CHISTREAMBUFFER);
        inputBuffers[inputIndex++].pStream          = m_pReferenceInStream[type][MfsrReferenceDS16];

        ChxUtils::Memcpy(&inputBuffers[inputIndex],
            &m_preFilterStageResult.refOutputBuffer[MfsrReferenceDS64],
            sizeof(CHISTREAMBUFFER));
        inputBuffers[inputIndex].acquireFence.valid = FALSE;
        inputBuffers[inputIndex].size               = sizeof(CHISTREAMBUFFER);
        inputBuffers[inputIndex++].pStream          = m_pReferenceInStream[type][MfsrReferenceDS64];

        GetOutputBuffer(m_pMfsrBufferManager[type][MfsrReferenceFull],
                        m_pReferenceOutStream[type][MfsrReferenceFull],
                        &outputBuffers[outputIndex++]);

        // Use MFNRBlend. Depends on prescale / postscale ratio to determine
        // whether MFNR or MFSR tuning data is used.
        feature2Mode = static_cast<UINT32>(ChiModeFeature2SubModeType::MFNRBlend);
        ChxUtils::FillTuningModeData(m_pInterStageMetadata,
            pRequest,
            m_sensorModeIndex,
            m_pUsecase->GetEffectMode(),
            m_pUsecase->GetSceneMode(),
            m_pUsecase->GetFeature1Mode(),
            &feature2Mode);

        PublicMFSRTotalFrames(m_pInterStageMetadata);
        m_resultsAvailable = FALSE;
        ExecuteMfsrRequest(MfsrStageBlend,
            appFrameNumber,
            numStageOutputs,
            &outputBuffers[0],
            numstageInputs,
            &inputBuffers[0],
            m_pInterStageMetadata);

        m_pMfsrResultMutex->Lock();

        // Wait for all blend results to come back - 1 output buffers: Full, and Metadata
        while (FALSE == m_resultsAvailable)
        {
            m_pMfsrResultAvailable->Wait(m_pMfsrResultMutex->GetNativeHandle());
        }

        if (TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress)))
        {
            m_resultsAvailable = FALSE;
            m_pMfsrResultMutex->Unlock();
            CHX_LOG_INFO("MFSR blend process return because of cleanup");
            return CDKResultENoMore;
        }

        // Release reference to prefilter input buffers
        m_pMfsrBufferManager[type][MfsrReferenceFull]->ReleaseReference(
            &m_preFilterStageResult.refOutputBuffer[MfsrReferenceFull].bufferInfo);

        // Copy blend Full Result and keep DS4/DS16/DS64 from m_preFilterStageResult.refOutputBuffer
        ChxUtils::Memcpy(&m_preFilterStageResult.refOutputBuffer[MfsrReferenceFull],
                         &m_blendStageResult.refOutputBuffer,
                         sizeof(CHISTREAMBUFFER));

        if (FALSE == m_pUsecase->IsMultiCameraUsecase())
        {
            CHX_LOG("Releasing Reference RDI Buffer %p for frame %d", &pMfsrInputInfo->bufferArray[i + 1].bufferInfo, i + 1);
            m_pRdiTargetBuffer->pBufferManager->ReleaseReference(&pMfsrInputInfo->bufferArray[i + 1].bufferInfo);
            m_pUsecase->UpdateValidRDIBufferLength(m_rdiStreamIndex, 1);
        }
        else
        {
            m_pUsecase->ReleaseSingleOffineInputResource(pRequest->frame_number, m_activePipelineIndex, i + 1);
        }

        m_resultsAvailable = FALSE;

        m_pMfsrResultMutex->Unlock();

        if (TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress)))
        {
            CHX_LOG_INFO("MFSR blend process return because of cleanup");
            return CDKResultENoMore;
        }
    }

    // Deactivate blend pipeline
    CHX_LOG_INFO("Deactivate MfsrStageBlend - DeactivateModeReleaseBuffer");
    Session* pOfflineSession = m_pUsecase->GetSessionData(m_offline)->pSession;
    result = ExtensionModule::GetInstance()->DeactivatePipeline(pOfflineSession->GetSessionHandle(),
        pOfflineSession->GetPipelineHandle(GetPipelineIndex(MfsrStageBlend)), CHIDeactivateModeReleaseBuffer);

    if (CDKResultSuccess != result)
    {
        CHX_LOG_ERROR("Deactivate MfsrStageBlend failed!");
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::SubmitOfflineMfsrPostfilterRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFSR::SubmitOfflineMfsrPostfilterRequest(
    UINT32                     appFrameNumber,
    camera3_capture_request_t* pRequest,
    MFSRInputInfo*             pMfsrInputInfo,
    CameraType                 type)
{
    CDKResult               result                             = CDKResultSuccess;
    CHISTREAMBUFFER         inputBuffers[MaxChiStreamBuffers]  = { { 0 } };
    CHISTREAMBUFFER         outputBuffers[MaxChiStreamBuffers] = { { 0 } };
    camera_metadata_t*      pMergedMetadata                    = NULL;
    UINT32                  numstageInputs                     = 0;
    UINT32                  numStageOutputs                    = 0;
    UINT                    outputIndex                        = 0;
    UINT                    inputIndex                         = 0;
    VOID*                   pRequestMetadata                   = NULL;
    UINT32                  feature2Mode;

    CHX_LOG_INFO("------------------------------- Mfsr post-filter stage ----------------------------");
    numstageInputs                    = 5;               // Rdi, BpsRegOut, PreFilter Full/DS4/DS4
    numStageOutputs                   = 1;               // Full
    m_numExpectedStageBuffers         = numStageOutputs; // Full
    outputIndex                       = 0;
    inputIndex                        = 0;
    m_remainingPostFilterStageResults = 1;

    ChxUtils::Memset(&m_postFilterStageResult, 0, sizeof(m_postFilterStageResult));

    ChxUtils::Memcpy(&inputBuffers[inputIndex++],
                     &pMfsrInputInfo->bufferArray[m_mfsrTotalNumFrames - 1],
                     sizeof(CHISTREAMBUFFER));

    ChxUtils::MergeMetadata(m_pApplicationInputMeta, m_pInterStageMetadata);
    ChxUtils::MergeMetadata(pMfsrInputInfo->pMetadata[m_mfsrTotalNumFrames - 1], m_pInterStageMetadata);

    ChxUtils::Memcpy(&inputBuffers[inputIndex], &m_bpsRegResultBuffer, sizeof(CHISTREAMBUFFER));
    inputBuffers[inputIndex].acquireFence.valid = FALSE;
    inputBuffers[inputIndex].size               = sizeof(CHISTREAMBUFFER);
    inputBuffers[inputIndex++].pStream          = m_pBpsRegInStream[type];

    ChxUtils::Memcpy(&inputBuffers[inputIndex],
        &m_preFilterStageResult.refOutputBuffer[MfsrReferenceFull],
        sizeof(CHISTREAMBUFFER));
    inputBuffers[inputIndex].acquireFence.valid = FALSE;
    inputBuffers[inputIndex].size               = sizeof(CHISTREAMBUFFER);
    inputBuffers[inputIndex++].pStream          = m_pReferenceInStream[type][MfsrReferenceFull];

    ChxUtils::Memcpy(&inputBuffers[inputIndex],
        &m_preFilterStageResult.refOutputBuffer[MfsrReferenceDS4],
        sizeof(CHISTREAMBUFFER));
    inputBuffers[inputIndex].acquireFence.valid = FALSE;
    inputBuffers[inputIndex].size               = sizeof(CHISTREAMBUFFER);
    inputBuffers[inputIndex++].pStream          = m_pReferenceInStream[type][MfsrReferenceDS4];

    ChxUtils::Memcpy(&inputBuffers[inputIndex],
        &m_preFilterStageResult.refOutputBuffer[MfsrReferenceDS16],
        sizeof(CHISTREAMBUFFER));
    inputBuffers[inputIndex].acquireFence.valid = FALSE;
    inputBuffers[inputIndex].size               = sizeof(CHISTREAMBUFFER);
    inputBuffers[inputIndex++].pStream          = m_pReferenceInStream[type][MfsrReferenceDS16];

    ChxUtils::Memcpy(&inputBuffers[inputIndex],
        &m_preFilterStageResult.refOutputBuffer[MfsrReferenceDS64],
        sizeof(CHISTREAMBUFFER));
    inputBuffers[inputIndex].acquireFence.valid = FALSE;
    inputBuffers[inputIndex].size               = sizeof(CHISTREAMBUFFER);
    inputBuffers[inputIndex++].pStream          = m_pReferenceInStream[type][MfsrReferenceDS64];

    GetOutputBuffer(m_pMfsrBufferManager[type][MfsrReferenceFull],
        m_pPostFilterOutStream[type],
        &outputBuffers[outputIndex++]);

    // Use MFNRPostFilter. Depends on prescale / postscale ratio to determine
    // whether MFNR or MFSR tuning data is used.
    feature2Mode = static_cast<UINT32>(ChiModeFeature2SubModeType::MFNRPostFilter);
    ChxUtils::FillTuningModeData(m_pInterStageMetadata,
        pRequest,
        m_sensorModeIndex,
        m_pUsecase->GetEffectMode(),
        m_pUsecase->GetSceneMode(),
        m_pUsecase->GetFeature1Mode(),
        &feature2Mode);

    PublicMFSRTotalFrames(m_pInterStageMetadata);
    m_resultsAvailable = FALSE;
    ExecuteMfsrRequest(MfsrStagePostfilter,
        appFrameNumber,
        numStageOutputs,
        &outputBuffers[0],
        numstageInputs,
        &inputBuffers[0],
        m_pInterStageMetadata);

    m_pMfsrResultMutex->Lock();

    // Wait for all post-prefilter scale results to come back - 2 output buffers: DS16, DS64, and Metadata
    while (FALSE == m_resultsAvailable)
    {
        m_pMfsrResultAvailable->Wait(m_pMfsrResultMutex->GetNativeHandle());
    }

    if (TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress)))
    {
        m_resultsAvailable = FALSE;
        m_pMfsrResultMutex->Unlock();
        CHX_LOG_INFO("MFSR postfilter process return because of cleanup");
        return CDKResultENoMore;
    }

    // Release reference to prefilter input buffers
    m_pMfsrBufferManager[type][MfsrReferenceFull]->ReleaseReference(
        &m_preFilterStageResult.refOutputBuffer[MfsrReferenceFull].bufferInfo);
    m_pMfsrBufferManager[type][MfsrReferenceDS4]->ReleaseReference(
        &m_preFilterStageResult.refOutputBuffer[MfsrReferenceDS4].bufferInfo);
    m_pMfsrBufferManager[type][MfsrReferenceDS16]->ReleaseReference(
        &m_preFilterStageResult.refOutputBuffer[MfsrReferenceDS16].bufferInfo);
    m_pMfsrBufferManager[type][MfsrReferenceDS64]->ReleaseReference(
        &m_preFilterStageResult.refOutputBuffer[MfsrReferenceDS64].bufferInfo);
    m_pMfsrBpsRegOutBufferManager[type]->ReleaseReference(
        &m_bpsRegResultBuffer.bufferInfo);

    if (FALSE == m_pUsecase->IsMultiCameraUsecase())
    {
        CHX_LOG("Releasing Reference RDI Buffer %p for frame %d",
                &pMfsrInputInfo->bufferArray[m_mfsrTotalNumFrames - 1].bufferInfo,
                m_mfsrTotalNumFrames - 1);
        m_pRdiTargetBuffer->pBufferManager->ReleaseReference(
            &pMfsrInputInfo->bufferArray[m_mfsrTotalNumFrames - 1].bufferInfo);
        m_pUsecase->UpdateValidRDIBufferLength(m_rdiStreamIndex, 1);
    }
    else
    {
        m_pUsecase->ReleaseSingleOffineInputResource(pRequest->frame_number, m_activePipelineIndex, m_mfsrTotalNumFrames - 1);
    }

    m_resultsAvailable = FALSE;

    m_pMfsrResultMutex->Unlock();

    // Deactivate pre-filter pipeline
    CHX_LOG_INFO("Deactivate MfsrStagePostfilter - DeactivateModeReleaseBuffer");
    Session* pOfflineSession = m_pUsecase->GetSessionData(m_offline)->pSession;
    result = ExtensionModule::GetInstance()->DeactivatePipeline(pOfflineSession->GetSessionHandle(),
        pOfflineSession->GetPipelineHandle(GetPipelineIndex(MfsrStagePostfilter)), CHIDeactivateModeReleaseBuffer);

    if (CDKResultSuccess != result)
    {
        CHX_LOG_ERROR("Deactivate MfsrStagePostfilter failed!");
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::SubmitOfflineMfsrSnapshotRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFSR::SubmitOfflineMfsrSnapshotRequest(
    UINT32                     appFrameNumber,
    camera3_capture_request_t* pRequest,
    CameraType                 type)
{
    (VOID)pRequest;
    CDKResult               result              = CDKResultSuccess;
    camera_metadata_t*      pMergedMetadata     = NULL;
    UINT32                  numstageInputs      = 0;
    UINT32                  numStageOutputs     = 0;
    UINT                    outputIndex         = 0;
    UINT                    inputIndex          = 0;
    VOID*                   pRequestMetadata    = NULL;
    UINT32                  feature2Mode;

    CHX_LOG_INFO("------------------------------- Mfsr snapshot stage ----------------------------");

    if ((NULL != m_pApplicationInputMeta) && (NULL != m_pInterStageMetadata))
    {
        UINT8      appFlashState = 0;
        CHITAGSOPS tagOps        = { 0 };

        g_chiContextOps.pTagOps(&tagOps);

        // Restore FlashState from prefliter stage
        tagOps.pGetMetaData((CHIHANDLE)m_pApplicationInputMeta,
            ANDROID_FLASH_STATE,
            &appFlashState,
            sizeof(appFlashState));
        tagOps.pSetMetaData((CHIHANDLE)m_pInterStageMetadata,
            ANDROID_FLASH_STATE,
            &appFlashState,
            sizeof(appFlashState));
    }

    numstageInputs                  = 1;
    numStageOutputs                 = 1;
    m_remainingSnapshotStageResults = 1;
    m_numExpectedStageBuffers       = numStageOutputs;
    m_postFilterStageResult.refOutputBuffer.pStream = m_pPostFilterOutStream[type];

    ExecuteMfsrRequest(MfsrStageSnapshot,
        appFrameNumber,
        numStageOutputs,
        &m_snapshotBuffers[m_maxSnapshotReqId % MaxOutstandingRequests][0],
        numstageInputs,
        &m_postFilterStageResult.refOutputBuffer,
        m_pInterStageMetadata);

    m_pMfsrResultMutex->Lock();
    // Wait for all snapshot stage results
    while (FALSE == m_resultsAvailable)
    {
        m_pMfsrResultAvailable->Wait(m_pMfsrResultMutex->GetNativeHandle());
    }

    m_pMfsrBufferManager[type][MfsrReferenceFull]->ReleaseReference(&m_postFilterStageResult.refOutputBuffer.bufferInfo);

    // Free all MFSR buffers to reduce memory
    m_pMfsrBufferManager[type][MfsrReferenceFull]->Deactivate(TRUE);
    m_pMfsrBufferManager[type][MfsrReferenceDS4]->Deactivate(TRUE);
    m_pMfsrBufferManager[type][MfsrReferenceDS16]->Deactivate(TRUE);
    m_pMfsrBufferManager[type][MfsrReferenceDS64]->Deactivate(TRUE);
    m_pMfsrBpsRegOutBufferManager[type]->Deactivate(TRUE);

    m_resultsAvailable = FALSE;
    m_pMfsrResultMutex->Unlock();

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::SubmitOfflineMfsrRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFSR::SubmitOfflineMfsrRequest(
    UINT32 appFrameNumber,
    camera3_capture_request_t* pRequest,
    UINT32 rtPipelineReqId)
{
    (VOID)rtPipelineReqId;
    CDKResult               result              = CDKResultSuccess;
    camera_metadata_t*      pMergedMetadata     = NULL;
    UINT32                  numstageInputs      = 0;
    UINT32                  numStageOutputs     = 0;
    UINT                    outputIndex         = 0;
    UINT                    inputIndex          = 0;
    VOID*                   pRequestMetadata    = NULL;
    UINT32                  feature2Mode;
    CameraType              type                = CameraType::Wide;
    MFSRInputInfo           mfsrInputInfo;

    CreateMFSRInputInfo(&mfsrInputInfo, pRequest);
    if (TRUE == m_pUsecase->IsMultiCameraUsecase())
    {
        if (m_activeCameraId != 0)
            type = CameraType::Tele;
    }

    result = SubmitOfflineMfsrPrefilterRequest(appFrameNumber, pRequest, &mfsrInputInfo, type);

    if (CDKResultENoMore != result)
    {
        result = SubmitOfflineMfsrBlendRequest(appFrameNumber, pRequest, &mfsrInputInfo, type);
    }

    if (CDKResultENoMore != result)
    {
        result = SubmitOfflineMfsrPostfilterRequest(appFrameNumber, pRequest, &mfsrInputInfo, type);
    }

    if (CDKResultENoMore != result)
    {
        SubmitOfflineMfsrSnapshotRequest(appFrameNumber, pRequest, type);
    }

    if (FALSE == m_pUsecase->IsMultiCameraUsecase())
    {
        for (UINT32 i = 0; i < mfsrInputInfo.numOfBuffers; i++)
        {
            CHX_LOG_INFO("Free metadata %p", mfsrInputInfo.pMetadata[i]);
            ChxUtils::FreeMetaData(mfsrInputInfo.pMetadata[i]);
            mfsrInputInfo.pMetadata[i] = NULL;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::ExecuteMfsrRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFSR::ExecuteMfsrRequest(
    MFSRStage           pipelineStage,
    UINT32              frameNumber,
    UINT32              numOutputs,
    CHISTREAMBUFFER*    pOutputBuffers,
    UINT32              numInputs,
    CHISTREAMBUFFER*    pInputBuffers,
    const VOID*         pSettings)
{
    CDKResult          result         = CDKResultSuccess;
    CHIPIPELINEREQUEST submitRequest  = { 0 };
    CHICAPTUREREQUEST  captureRequest = { 0 };
    const Session*     pSession       = m_pUsecase->GetSessionData(m_offline)->pSession;
    PipelineData*      pPipelineData  =
        const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_offline, GetPipelineIndex(pipelineStage)));
    UINT               requestIdIndex = (pPipelineData->seqId % MaxOutstandingRequests);

    pPipelineData->seqIdToFrameNum[requestIdIndex] = frameNumber;

    captureRequest.frameNumber            = pPipelineData->seqId++;
    captureRequest.hPipelineHandle        = reinterpret_cast<CHIPIPELINEHANDLE>(
                                                pSession->GetPipelineHandle(GetPipelineIndex(pipelineStage)));
    captureRequest.numInputs              = numInputs;
    captureRequest.numOutputs             = numOutputs;
    captureRequest.pInputBuffers          = pInputBuffers;
    captureRequest.pOutputBuffers         = pOutputBuffers;
    captureRequest.pMetadata              = pSettings;
    captureRequest.pPrivData              = &m_privData[captureRequest.frameNumber % MaxOutstandingRequests];
    captureRequest.pPrivData->featureType = FeatureType::MFSR;

    submitRequest.pSessionHandle          = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
    submitRequest.numRequests             = 1;
    submitRequest.pCaptureRequests        = &captureRequest;

    CHX_LOG_INFO("Sending MFSR request application-frameNumber: %d, sessionId: %d, session-requestId: %d, "
                 "pPrivData: %p",
                 frameNumber, m_offline, (UINT)captureRequest.frameNumber, captureRequest.pPrivData);

    if (FALSE == pSession->IsPipelineActive())
    {
        CDKResult result = CDKResultSuccess;

        result = ExtensionModule::GetInstance()->ActivatePipeline(pSession->GetSessionHandle(),
            pSession->GetPipelineHandle(GetPipelineIndex(pipelineStage)));

        if (CDKResultSuccess == result)
        {
            pSession->SetPipelineActivateFlag();
        }
    }
    m_pUsecase->SetRequestToFeatureMapping(m_offline, captureRequest.frameNumber, this);
    result = m_pUsecase->SubmitRequest(&submitRequest);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::ProcessResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFSR::ProcessResult(
    CHICAPTURERESULT*   pResult,
    VOID*               pPrivateCallbackData)
{
    if ((TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress))) ||
        (FlushStatus::NotFlushing != m_pUsecase->GetFlushStatus()))
    {
        CHX_LOG_INFO("MFSR process result return because of cleanup");
        return;
    }

    SessionPrivateData* pCbData               = static_cast<SessionPrivateData*>(pPrivateCallbackData);
    BOOL                isAppResultsAvailable = FALSE;
    UINT32              resultFrameNum        = pResult->frameworkFrameNum;
    UINT32              numstageInputs        = 0;
    UINT32              numStageOutputs       = 0;
    UINT32              rtPipelineReqId       = 0;
    CameraType          type                  = CameraType::Wide;

    if (TRUE == m_pUsecase->IsMultiCameraUsecase())
    {
        RequestMapInfo requestMapInfo = m_pUsecase->GetRequestMapInfo(m_internalRequestId);
        if (requestMapInfo.masterCameraID != 0)
        {
            type = CameraType::Tele;
        }
        resultFrameNum = requestMapInfo.frameNumber;
    }

    CHX_LOG_INFO("MFSR HW session id: %d, offline id: %d", pCbData->sessionId, m_offline);
    if (m_offline == pCbData->sessionId)
    {
        const PipelineData* pPipelineData;

        if (m_remainingPrefilterStageResults > 0)
        {
            pPipelineData = m_pUsecase->GetPipelineData(pCbData->sessionId, GetPipelineIndex(MfsrStagePrefilter));
        }
        if (m_remainingBlendStageResults > 0)
        {
            pPipelineData = m_pUsecase->GetPipelineData(pCbData->sessionId, GetPipelineIndex(MfsrStageBlend));
        }
        if (m_remainingPostFilterStageResults > 0)
        {
            pPipelineData = m_pUsecase->GetPipelineData(pCbData->sessionId, GetPipelineIndex(MfsrStagePostfilter));
        }
        if (m_remainingSnapshotStageResults > 0)
        {
            pPipelineData = m_pUsecase->GetPipelineData(pCbData->sessionId, GetPipelineIndex(MfsrStageSnapshot));
        }
        resultFrameNum  = pPipelineData->seqIdToFrameNum[pResult->frameworkFrameNum % MaxOutstandingRequests];
    }
    else if (m_realtime == pCbData->sessionId)
    {
        PipelineData*     pPipelineData =
            const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_realtime, 0));

        resultFrameNum = pPipelineData->seqIdToFrameNum[pResult->frameworkFrameNum % MaxOutstandingRequests];
        rtPipelineReqId = pResult->frameworkFrameNum;

        CHX_LOG("Realtime ReqId to AppFrameNum: %d <--> %d", pResult->frameworkFrameNum, resultFrameNum);
    }

    CHX_LOG("ProcessResult resultFrameNum: %d, sessionId: %d, pResultMetadata: %p, numOutputBuffers: %d pResult->fmwkFrNo %d",
            resultFrameNum, pCbData->sessionId, pResult->pResultMetadata, pResult->numOutputBuffers, pResult->frameworkFrameNum);

    UINT32                    resultFrameIndex = resultFrameNum % MaxOutstandingRequests;

    camera3_capture_result_t* pUsecaseResult   = m_pUsecase->GetCaptureResult(resultFrameIndex);

    pUsecaseResult->frame_number = resultFrameNum;

    // If result contain metadata and metadata has not been sent to framework
    if ((NULL != pResult->pResultMetadata))
    {
        BOOL               isResultMetadataAvailable = FALSE;
        UINT64             timestamp                 = m_pUsecase->GetRequestShutterTimestamp(resultFrameNum);
        camera_metadata_t* pMetadata                 =
            const_cast<camera_metadata_t*>(static_cast<const camera_metadata_t*>(pResult->pResultMetadata));

        if (m_realtime == pCbData->sessionId)
        {
            m_pUsecase->FillMetadataForRDIQueue(rtPipelineReqId, m_rdiStreamIndex, pMetadata);
        }

        if (FALSE == m_pUsecase->IsMetadataSent(resultFrameIndex))
        {
            // Do Not wait for Snapshot frame metadata, Return Preview metadata back to fwk.
            // If we wait for snapshot, and if it takes more time to process, we will block the preview.
            pUsecaseResult->result = static_cast<const camera_metadata_t*>(pResult->pResultMetadata);
            pUsecaseResult->partial_result = pResult->numPartialMetadata;
            isResultMetadataAvailable = TRUE;

            // Override snapshot frame sensor timestamp metadata with shutter event for same frame number
            if ((NULL != pMetadata) && (0 != timestamp))
            {
                UINT32                  SensorTimestampTag = 0x000E0010;
                camera_metadata_entry_t entry              = { 0 };

                INT32 status = find_camera_metadata_entry(pMetadata, SensorTimestampTag, &entry);

                if (-ENOENT == status)
                {
                    status = add_camera_metadata_entry(pMetadata, SensorTimestampTag, &timestamp, 1);
                }
                else if (0 == status)
                {
                    status = update_camera_metadata_entry(pMetadata, entry.index, &timestamp, 1, NULL);
                }

                CHX_LOG("Update sensor timestamp for frame %d, timestamp = %" PRIu64 "  Status %d",
                        resultFrameNum, timestamp, status);

                if (TRUE == isResultMetadataAvailable)
                {
                    m_pUsecase->SetMetadataAvailable(resultFrameIndex);
                    isAppResultsAvailable                   = TRUE;
                }
            }
        }

        if ((m_remainingPrefilterStageResults > 0) && (m_offline == pCbData->sessionId))
        {
            camera_metadata_t* pMetadata =
                const_cast<camera_metadata_t*>(static_cast<const camera_metadata_t*>(pResult->pResultMetadata));

            CHX_LOG_INFO("pMetadata %p from Prefilter stage output", pMetadata);
            ChxUtils::MergeMetadata(pMetadata, m_pInterStageMetadata);
        }
    }

    if (pResult->numOutputBuffers > 0)
    {
        for (UINT i = 0; i < pResult->numOutputBuffers; i++)
        {
            // If our internal stream, copy the result into the target buffer to be consumed by the offline pipeline
            if (m_pRdiStream[type] == pResult->pOutputBuffers[i].pStream)
            {
                m_pUsecase->UpdateBufferReadyForRDIQueue(rtPipelineReqId, m_rdiStreamIndex, TRUE);

                if (TRUE == m_triggerMFSRReprocess[rtPipelineReqId % MaxOutstandingRequests])
                {
                    m_allRDIResultsAvaliable = TRUE;
                }

                if ((TRUE == m_isSnapshotFrame[resultFrameIndex]) &&
                    (TRUE == m_isLLSSnapshot)                     &&
                    (TRUE == m_allRDIResultsAvaliable)            &&
                    (TRUE == m_allFDResultsAvaliable))
                {
                    m_triggerMFSRReprocess[rtPipelineReqId % MaxOutstandingRequests] = FALSE;
                    CHX_LOG("FeatureMFSR LLS signal request thead");

                    m_captureRequest.frame_number = rtPipelineReqId;
                    m_pOfflineRequestAvailable->Signal();
                }
            }
            else if (m_pFdStream == pResult->pOutputBuffers[i].pStream)
            {
                m_pUsecase->UpdateBufferReadyForFDQueue(rtPipelineReqId, m_fdStreamIndex, TRUE);

                if (TRUE == m_triggerMFSRReprocess[rtPipelineReqId % MaxOutstandingRequests])
                {
                    m_allFDResultsAvaliable = TRUE;
                }

                if (TRUE == m_triggerMFSRReprocess[rtPipelineReqId % MaxOutstandingRequests])
                {
                    m_allFDResultsAvaliable = TRUE;
                }

                if ((TRUE == m_isSnapshotFrame[resultFrameIndex]) &&
                    (TRUE == m_isLLSSnapshot) &&
                    (TRUE == m_allRDIResultsAvaliable) &&
                    (TRUE == m_allFDResultsAvaliable))
                {
                    m_triggerMFSRReprocess[rtPipelineReqId % MaxOutstandingRequests] = FALSE;
                    CHX_LOG("FeatureMFNR LLS signal request thead");

                    m_captureRequest.frame_number = rtPipelineReqId;
                    m_pOfflineRequestAvailable->Signal();
                }
            }
            else if ((m_remainingPrefilterStageResults > 0) && (m_offline == pCbData->sessionId))
            {
                // There will be 5 buffers per prefilter stage results - Full, DS4, DS16, DS64, BpRegOut
                if (pResult->pOutputBuffers[i].pStream == m_pReferenceOutStream[type][MfsrReferenceFull])
                {
                    CHX_LOG_INFO("MFSR-SNAPSHOT: Received Prefilter FULL output buffer");

                    ChxUtils::Memcpy(&m_preFilterStageResult.refOutputBuffer[MfsrReferenceFull],
                        &pResult->pOutputBuffers[i],
                        sizeof(CHISTREAMBUFFER));
                    m_numExpectedStageBuffers--;

                }
                else if (pResult->pOutputBuffers[i].pStream == m_pReferenceOutStream[type][MfsrReferenceDS4])
                {
                    CHX_LOG_INFO("MFSR-SNAPSHOT: Received Prefilter DS4 output buffer");

                    ChxUtils::Memcpy(&m_preFilterStageResult.refOutputBuffer[MfsrReferenceDS4],
                        &pResult->pOutputBuffers[i],
                        sizeof(CHISTREAMBUFFER));
                    m_numExpectedStageBuffers--;
                }
                else if (pResult->pOutputBuffers[i].pStream == m_pReferenceOutStream[type][MfsrReferenceDS16])
                {
                    CHX_LOG_INFO("MFSR-SNAPSHOT: Received Prefilter DS16 output buffer");

                    ChxUtils::Memcpy(&m_preFilterStageResult.refOutputBuffer[MfsrReferenceDS16],
                        &pResult->pOutputBuffers[i],
                        sizeof(CHISTREAMBUFFER));
                    m_numExpectedStageBuffers--;
                }
                else if (pResult->pOutputBuffers[i].pStream == m_pReferenceOutStream[type][MfsrReferenceDS64])
                {
                    CHX_LOG_INFO("MFSR-SNAPSHOT: Received Prefilter DS64 output buffer");

                    ChxUtils::Memcpy(&m_preFilterStageResult.refOutputBuffer[MfsrReferenceDS64],
                        &pResult->pOutputBuffers[i],
                        sizeof(CHISTREAMBUFFER));
                    m_numExpectedStageBuffers--;
                }
                else if (pResult->pOutputBuffers[i].pStream == m_pBpsRegOutStream[type])
                {
                    CHX_LOG_INFO("MFSR-SNAPSHOT: Received Prefilter BPS Reg output buffer");

                    ChxUtils::Memcpy(&m_bpsRegResultBuffer, &pResult->pOutputBuffers[i], sizeof(CHISTREAMBUFFER));
                    m_numExpectedStageBuffers--;
                }
                if (m_numExpectedStageBuffers == 0)
                {
                    CHX_LOG_INFO("MFSR-SNAPSHOT: All Prefilter stage output buffers received");

                    m_remainingPrefilterStageResults--;
                    m_pMfsrResultMutex->Lock();
                    m_resultsAvailable = TRUE;
                    m_pMfsrResultAvailable->Signal();
                    m_pMfsrResultMutex->Unlock();
                }
            }
            // Blending stage
            else if ((m_remainingBlendStageResults > 0) && (m_offline == pCbData->sessionId))
            {
                if (pResult->pOutputBuffers[i].pStream == m_pReferenceOutStream[type][MfsrReferenceFull])
                {
                    CHX_LOG_INFO("MFSR-SNAPSHOT: Received Blend Full output buffer");

                    ChxUtils::Memcpy(&m_blendStageResult.refOutputBuffer,
                        &pResult->pOutputBuffers[i],
                        sizeof(CHISTREAMBUFFER));
                    m_numExpectedStageBuffers--;
                }
                else
                {
                    CHX_LOG_INFO("MFSR-SNAPSHOT: Received (blend) one %d stream pointer %p type %d  %d x %d",
                        i, pResult->pOutputBuffers[i].pStream, pResult->pOutputBuffers[i].pStream->streamType,
                        pResult->pOutputBuffers[i].pStream->width, pResult->pOutputBuffers[i].pStream->height);
                }
                if (m_numExpectedStageBuffers == 0)
                {
                    CHX_LOG_INFO("MFSR-SNAPSHOT: All Blend stage output buffers received");
                    m_remainingBlendStageResults--;
                    m_pMfsrResultMutex->Lock();
                    m_resultsAvailable = TRUE;
                    m_pMfsrResultAvailable->Signal();
                    m_pMfsrResultMutex->Unlock();
                }

            }
            // Postfilter stage
            else if ((m_remainingPostFilterStageResults > 0) && (m_offline == pCbData->sessionId))
            {
                if (pResult->pOutputBuffers[i].pStream == m_pPostFilterOutStream[type])
                {
                    CHX_LOG_INFO("MFSR-SNAPSHOT: Received Postfilter Full output buffer");

                    ChxUtils::Memcpy(&m_postFilterStageResult.refOutputBuffer,
                        &pResult->pOutputBuffers[0],
                        sizeof(CHISTREAMBUFFER));
                    m_numExpectedStageBuffers--;
                }
                else
                {
                    CHX_LOG_INFO("MFSR-SNAPSHOT: Received (postfilter) one %d stream pointer %p type %d  %d x %d",
                        i, pResult->pOutputBuffers[i].pStream, pResult->pOutputBuffers[i].pStream->streamType,
                        pResult->pOutputBuffers[i].pStream->width, pResult->pOutputBuffers[i].pStream->height);
                }
                if (m_numExpectedStageBuffers == 0)
                {
                    CHX_LOG_INFO("MFSR-SNAPSHOT: All Postfilter stage output buffers received");
                    m_remainingPostFilterStageResults--;
                    m_pMfsrResultMutex->Lock();
                    m_resultsAvailable = TRUE;
                    m_pMfsrResultAvailable->Signal();
                    m_pMfsrResultMutex->Unlock();
                }
            }
            // Snapshot stage
            else if ((m_remainingSnapshotStageResults > 0) && (m_offline == pCbData->sessionId))
            {
                CHX_LOG_INFO("MFSR-SNAPSHOT: Received Snapshot output buffer");
                CHX_LOG_INFO("MFSR-SNAPSHOT: copy snapshot result in output buffer atframe %d",
                    pUsecaseResult->frame_number);

                if (TRUE == m_blockPreviewForSnapshot)
                {
                    m_pSnapshotResultMutex->Lock();
                    m_snapshotResultAvailable = TRUE;
                    m_pSnapshotResultAvailable->Signal();
                    m_pSnapshotResultMutex->Unlock();
                }

                // queue a buffer as part of the normal result
                m_pUsecase->GetAppResultMutex()->Lock();
                camera3_stream_buffer_t* pResultBuffer =
                    const_cast<camera3_stream_buffer_t*>(
                        &pUsecaseResult->output_buffers[pUsecaseResult->num_output_buffers++]);

                ChxUtils::PopulateChiToHALStreamBuffer(&pResult->pOutputBuffers[i], pResultBuffer);
                m_pUsecase->GetAppResultMutex()->Unlock();

                isAppResultsAvailable = TRUE;
                m_numExpectedStageBuffers--;
                if (m_numExpectedStageBuffers == 0)
                {
                    CHX_LOG_INFO("MFSR-SNAPSHOT: All Snapshot stage output buffers received");
                    m_remainingSnapshotStageResults--;
                    m_pMfsrResultMutex->Lock();
                    m_resultsAvailable = TRUE;
                    m_pMfsrResultAvailable->Signal();
                    m_pMfsrResultMutex->Unlock();
                }
            }
            else
            {
                // queue a buffer as part of the normal result
                m_pUsecase->GetAppResultMutex()->Lock();
                camera3_stream_buffer_t* pResultBuffer =
                    const_cast<camera3_stream_buffer_t*>(
                        &pUsecaseResult->output_buffers[pUsecaseResult->num_output_buffers++]);

                ChxUtils::PopulateChiToHALStreamBuffer(&pResult->pOutputBuffers[i], pResultBuffer);
                m_pUsecase->GetAppResultMutex()->Unlock();

                isAppResultsAvailable = TRUE;

                CHX_LOG_INFO("MFSR-SNAPSHOT: Received normal result %d stream pointer %p type %d  %d x %d",
                    i, pResult->pOutputBuffers[i].pStream, pResult->pOutputBuffers[i].pStream->streamType,
                    pResult->pOutputBuffers[i].pStream->width, pResult->pOutputBuffers[i].pStream->height);
            }
        }
    }

    if (TRUE == isAppResultsAvailable)
    {
        CHX_LOG("MFSR ProcessAndReturnFinishedResults for frame: %d", pUsecaseResult->frame_number);
        m_pUsecase->ProcessAndReturnFinishedResults();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::ProcessCHIPartialData
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFSR::ProcessCHIPartialData(
    UINT32    frameNum,
    UINT32    sessionId)
{
    CAMX_UNREFERENCED_PARAM(frameNum);
    CAMX_UNREFERENCED_PARAM(sessionId);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::ProcessDriverPartialCaptureResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFSR::ProcessDriverPartialCaptureResult(
    CHIPARTIALCAPTURERESULT*    pResult,
    VOID*                       pPrivateCallbackData)
{
    CAMX_UNREFERENCED_PARAM(pResult);
    CAMX_UNREFERENCED_PARAM(pPrivateCallbackData);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFSR::ProcessMessage
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFSR::ProcessMessage(
    const CHIMESSAGEDESCRIPTOR* pMessageDescriptor,
    VOID*                       pPrivateCallbackData)
{
    (VOID)pMessageDescriptor;
    (VOID)pPrivateCallbackData;
}

INT32 FeatureMFSR::GetRequiredPipelines(
    AdvancedPipelineType* pPipelines,
    INT32 size)
{
    INT32 count = 0;
    const INT32 pipelineCount = 11;

    if (NULL != pPipelines && size >= pipelineCount)
    {
        INT32                index = 0;
        AdvancedPipelineType pipelineGroup[pipelineCount];
        UINT                 cameraId[pipelineCount];
        UINT                 physicalCameraID   = m_pUsecase->GetPhysicalCameraId(m_physicalCameraIndex);
        INT32                pipelineGroupIndex = 0;

        if (InvalidPhysicalCameraId != physicalCameraID)
        {
            pPipelines[index]                   = AdvancedPipelineType::InternalZSLYuv2JpegMFSRType;
            cameraId[pipelineGroupIndex]        = physicalCameraID;
            pipelineGroup[pipelineGroupIndex++] = pPipelines[index];
            index++;

            pPipelines[index] = AdvancedPipelineType::MFSRPrefilterType;
            cameraId[pipelineGroupIndex]        = physicalCameraID;
            pipelineGroup[pipelineGroupIndex++] = pPipelines[index];
            index++;

            pPipelines[index]                   = AdvancedPipelineType::MFSRBlendType;
            cameraId[pipelineGroupIndex]        = physicalCameraID;
            pipelineGroup[pipelineGroupIndex++] = pPipelines[index];
            index++;

            pPipelines[index]                   = AdvancedPipelineType::MFSRPostFilterType;
            cameraId[pipelineGroupIndex]        = physicalCameraID;
            pipelineGroup[pipelineGroupIndex++] = pPipelines[index];
            index++;

            // group the offline session together and get the session id
            m_offline           = m_pUsecase->GetUniqueSessionId(&pipelineGroup[0], pipelineGroupIndex, m_realtimePipelineIndex);
            m_pUsecase->SetPipelineCameraId(&pipelineGroup[0], &cameraId[0], pipelineGroupIndex);
            pipelineGroupIndex  = 0;

            if (FALSE == m_pUsecase->IsMultiCameraUsecase())
            {
                pPipelines[index]                   = AdvancedPipelineType::ZSLPreviewRawType;
                cameraId[pipelineGroupIndex]        = physicalCameraID;
                pipelineGroup[pipelineGroupIndex++] = pPipelines[index];
                // get the preview session id
                m_realtime          = m_pUsecase->GetUniqueSessionId(&pipelineGroup[0], pipelineGroupIndex, m_realtimePipelineIndex);
                m_pUsecase->SetPipelineCameraId(&pipelineGroup[0], &cameraId[0], pipelineGroupIndex);
                pipelineGroupIndex  = 0;
                index++;
            }
            else
            {
                m_realtime = 0xFFFFFFFF;
                CHX_LOG("Don't use Realtime pipeline in advance usecase for multicamera usecase");
            }

            count = index;
        }
    }

    CHX_LOG("FeatureMFSR::GetRequiredPipelines, required pipeline count:%d", count);

    CHX_ASSERT(pipelineCount == count);
    return count;
}

UINT FeatureMFSR::GetPipelineIndex(MFSRStage pipelineStage)
{
    UINT index = pipelineStage;
    if (TRUE == m_pUsecase->IsMultiCameraUsecase())
    {
        if (m_activeCameraId != 0)
        {
            index = MfsrStageMax + pipelineStage;
        }

    }

    CHX_LOG("pipeline index %d for stage %d", index, pipelineStage);
    return index;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureMFSR::RequestThread
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID* FeatureMFSR::RequestThread(
    VOID* pThreadData)
{
    PerThreadData* pPerThreadData = reinterpret_cast<PerThreadData*>(pThreadData);

    FeatureMFSR* pCameraUsecase = reinterpret_cast<FeatureMFSR*>(pPerThreadData->pPrivateData);

    pCameraUsecase->RequestThreadProcessing();

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureMFSR::RequestThreadProcessing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFSR::RequestThreadProcessing()
{
    UINT32 index = 0;
    CHX_LOG_INFO("RequestThreadProcessing Entered");
    while (TRUE)
    {
        m_pOfflineRequestMutex->Lock();

        if (FALSE == m_offlineRequestProcessTerminate)
        {
            m_pOfflineRequestAvailable->Wait(m_pOfflineRequestMutex->GetNativeHandle());
        }

        if (TRUE == m_offlineRequestProcessTerminate)
        {
            m_pOfflineRequestMutex->Unlock();
            break;
        }

        UINT32 frameNumber      = m_requestFrameNumber;
        UINT32 rtPipelineReqId  = 0;

        SubmitOfflineMfsrRequest(frameNumber, &m_captureRequest, rtPipelineReqId);
        m_maxSnapshotReqId++;
        m_pOfflineRequestMutex->Unlock();
    }
    CHX_LOG_INFO("MFSRThreading Exited");
}

UINT FeatureMFSR::GetMFSRTotalFramesByGain(
    camera_metadata_t* pMeta)
{
    UINT    totalMFSRNumFrames = MfsrDefaultInputFrames;

    CHITAGSOPS   tagOps = { 0 };
    UINT32       metadataAECFrameControl;
    g_chiContextOps.pTagOps(&tagOps);

    CDKResult result = tagOps.pQueryVendorTagLocation("org.quic.camera2.statsconfigs", "AECFrameControl",
                                                       &metadataAECFrameControl);
    if (CDKResultSuccess == result)
    {
        AECFrameControl frameCtrl;
        memset(&frameCtrl, 0, sizeof(AECFrameControl));

        result = tagOps.pGetMetaData(pMeta,
                                     metadataAECFrameControl,
                                     &frameCtrl,
                                     sizeof(AECFrameControl));

        if (CDKResultSuccess == result)
        {
            FLOAT realGain = frameCtrl.exposureInfo[ExposureIndexSafe].linearGain;
            CHX_LOG_INFO("#2 AEC Gain received = %f", realGain);

            // need update from latest tuning xml file
            if (realGain <= 2.0f)
            {
                totalMFSRNumFrames = 3;
            }
            else if (realGain <= 4.0f)
            {
                totalMFSRNumFrames = 4;
            }
            else if (realGain <= 8.0f)
            {
                totalMFSRNumFrames = 5;
            }
            else if (realGain <= 16.0f)
            {
                totalMFSRNumFrames = 6;
            }
            else if (realGain <= 32.0f)
            {
                totalMFSRNumFrames = 7;
            }
            else
            {
                totalMFSRNumFrames = 8;
            }
        }
        else
        {
            CHX_LOG_WARN("Cannot get AEC frame info from metadata");
        }
        CHX_LOG_INFO("Total number of MFSR Frames = %d", totalMFSRNumFrames);
    }
    else
    {
        CHX_LOG_WARN("Not able to obtain AEC gain for calculation of Total MFSR frames");
    }

    return totalMFSRNumFrames;
}

CDKResult FeatureMFSR::PublicMFSRTotalFrames(
    camera_metadata_t* pMeta)
{
    CHITAGSOPS   tagOps = { 0 };
    UINT32       metadataMFSRTotalNumFrames;
    g_chiContextOps.pTagOps(&tagOps);

    CDKResult result = tagOps.pQueryVendorTagLocation("org.quic.camera2.mfsrconfigs", "MFSRTotalNumFrames",
        &metadataMFSRTotalNumFrames);

    if (CDKResultSuccess == result)
    {
        result = tagOps.pSetMetaData(pMeta,
                                     metadataMFSRTotalNumFrames,
                                     &m_mfsrTotalNumFrames,
                                     sizeof(UINT));
        if (CDKResultSuccess != result)
        {
            CHX_LOG_WARN("Cannot set MFSR Total Number of Frames info into metadata");
        }
    }
    else
    {
        CHX_LOG_WARN("Cannot query MFSR vendor tag info from metadata");
    }

    return result;
}
#endif
