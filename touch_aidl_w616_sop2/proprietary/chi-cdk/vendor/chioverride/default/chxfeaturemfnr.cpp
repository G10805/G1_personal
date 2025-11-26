////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxfeaturemfnr.cpp
/// @brief CHX mfnr feature class implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <cmath>

#include "chi.h"
#include "chiispstatsdefs.h"
#include "chistatsproperty.h"
#include "chxadvancedcamerausecase.h"
#include "chxfeaturemfnr.h"
#include "chxincs.h"
#include "chxusecase.h"
#include "chxutils.h"

// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files

typedef INT32 CSLFence;

enum ChiFenceState
{
    ChiFenceInit    = 0,            ///< Fence initialized
    ChiFenceSuccess,                ///< Fence signaled with success
    ChiFenceFailed,                 ///< Fence signaled with failure
    ChiFenceInvalid                 ///< Fence invalid state
};

struct ChiFence
{
    CHIFENCEHANDLE  hChiFence;      ///< Handle to this Chi fence instance
    CHIFENCETYPE    type;           ///< Chi fence type
    INT             aRefCount;      ///< Reference count
    CSLFence        hFence;         ///< CSL fence representing this Chi fence
    ChiFenceState   resultState;    ///< Fence signal result state
    union
    {
        UINT64      eglSync;        ///< EGL sync object (need to cast to EGLSyncKHR)
        INT         nativeFenceFD;  ///< Native fence file descriptor
    };
};

/// @brief Define CSL fence signal status codes
typedef enum
{
    CSLFenceResultSuccess = 0,      ///< Fence signaled with success
    CSLFenceResultFailed            ///< Fence signaled with failure
} CSLFenceResult;

extern CHICONTEXTOPS g_chiContextOps;

using namespace std;

static const SIZE_T ChiStreamBufferSize     = sizeof(CHISTREAMBUFFER);
static const UINT32 MaxActiveBlendRequests  = 2;
static const UINT32 MaxChiFenceDependencies = 8;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::Create
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FeatureMFNR* FeatureMFNR::Create(
    AdvancedCameraUsecase* pUsecase,
    UINT32 physicalCameraIndex)
{
    FeatureMFNR* pFeature = CHX_NEW FeatureMFNR;

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
/// DumpMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static VOID DumpMetadata(
   ChiMetadata* pMetadata,
   const CHAR*  pStagename)
{
#ifdef __MFNR_DUMP_METADATA__
    char fileName[MaxFileLen];
    snprintf (fileName, sizeof(fileName), "%s.txt", pStagename);
    pMetadata->DumpDetailsToFile(fileName);
#else
    (VOID)pMetadata;
    (VOID)pStagename;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::Initialize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::Initialize(
    AdvancedCameraUsecase* pUsecase)
{
    CDKResult result = CDKResultSuccess;

    Feature::InitializePrivateResources();

    m_pUsecase              = pUsecase;
    m_pMfnrResultMutex      = Mutex::Create();
    m_pMfnrResultAvailable  = Condition::Create();

    m_pRDIResultMutex       = Mutex::Create();
    m_pRDIResultAvailable   = Condition::Create();

    m_mfnrTotalNumFrames = MfnrDefaultInputFrames;
    m_pMetadataManager   = pUsecase->GetMetadataManager();

    // Debug-Data for offline processing
    m_debugDataOffline.pData = NULL;
    m_debugDataOffline.size  = 0;


    /// As now preview keeps going during snapshot,
    /// if the MFNR snpashot reprocess is long, the input RDI buffer could be overwritten by up-coming preivew request.
    /// there's no such issue in normal case, but we can easily meet this problem when frame dump is eanbled.
    /// add a flag here, if need to dump frame, probably need to block upcoming preview.
    m_blockPreviewForSnapshot = FALSE;

    m_pSnapshotResultMutex     = Mutex::Create();
    m_pSnapshotResultAvailable = Condition::Create();

    m_pMfnrBpsRegOutBufferManager           = NULL;
    m_pOfflineNoiseReprocessBufferManager   = NULL;
    ChxUtils::Memset(m_pMfnrBufferManager, 0, sizeof(m_pMfnrBufferManager));

    m_noiseReprocessEnable = m_pUsecase->IsOfflineNoiseReprocessEnabled();
    CHX_LOG("MFNR: Offline noise reprocessing pipeline enabled: %d", m_noiseReprocessEnable);

    InitializeInternalStreams();

    ChxUtils::Memset(&m_preFilterStageResult,                0, sizeof(m_preFilterStageResult));
    ChxUtils::Memset(&m_scaleStageResult,                    0, sizeof(m_scaleStageResult));
    ChxUtils::Memset(&m_blendStageResult,                    0, sizeof(m_blendStageResult));
    ChxUtils::Memset(&m_postFilterStageResult,               0, sizeof(m_postFilterStageResult));
    ChxUtils::Memset(&m_preFilterAnchorFrameRegResultBuffer, 0, sizeof(m_preFilterAnchorFrameRegResultBuffer));
    ChxUtils::Memset(&m_pInterStageInputMetadata,            0, sizeof(m_pInterStageInputMetadata));
    ChxUtils::Memset(&m_noiseReprocessStageResult,           0, sizeof(m_noiseReprocessStageResult));

    m_pOfflineRequestMutex           = Mutex::Create();
    CHX_ASSERT(NULL != m_pOfflineRequestMutex);
    m_pOfflineRequestAvailable       = Condition::Create();
    CHX_ASSERT(NULL != m_pOfflineRequestAvailable);

    m_offlineRequestProcessTerminate = FALSE;
    m_aPauseInProgress               = FALSE;

    m_offlineRequestProcessThread.pPrivateData = this;

    m_captureRequest = { 0 };

    m_pApplicationInputMeta = NULL;

    m_isIHDRSnapshotEnable  = ExtensionModule::GetInstance()->EnableIHDRSnapshot();

    m_stateIHDRSnapshot     = IHDR_NONE;

    for (UINT32 index = 0; index < MfnrMaxMetadata; ++index)
    {
        // create empty buffer
        m_pInterStageInputMetadata[index] = ChiMetadata::Create();

        if (NULL == m_pInterStageInputMetadata[index])
        {
            result = CDKResultENoMemory;
            CHX_LOG_ERROR("Failed to create interstage input metadata %d", index);
            break;
        }
    }

    if (CDKResultSuccess == result)
    {
        result = ChxUtils::ThreadCreate(FeatureMFNR::RequestThread,
                                        &m_offlineRequestProcessThread,
                                        &m_offlineRequestProcessThread.hThreadHandle);
    }

    CHX_ASSERT(CDKResultSuccess != result);

    return result;
}

VOID FeatureMFNR::InitializeInternalStreams()
{
    for (UINT i = 0; i < MfnrReferenceMax; i++)
    {
        m_pPrefilterOutStream[i] = (CHISTREAM*)(CHX_CALLOC(sizeof(CHISTREAM)));
        CHX_ASSERT(NULL != m_pPrefilterOutStream[i]);
        m_pBlendOutStream[i]     = (CHISTREAM*)(CHX_CALLOC(sizeof(CHISTREAM)));
        CHX_ASSERT(NULL != m_pBlendOutStream[i]);
        m_pBlendInStream[i]      = (CHISTREAM*)(CHX_CALLOC(sizeof(CHISTREAM)));
        CHX_ASSERT(NULL != m_pBlendInStream[i]);
        m_pScaleInStream[i]      = (CHISTREAM*)(CHX_CALLOC(sizeof(CHISTREAM)));
        CHX_ASSERT(NULL != m_pScaleInStream[i]);
    }

    m_pMfnrPostFilterOutStream = (CHISTREAM*)(CHX_CALLOC(sizeof(CHISTREAM)));
    CHX_ASSERT(NULL != m_pMfnrPostFilterOutStream);

    m_pMfnrBpsRegOutStream = (CHISTREAM*)(CHX_CALLOC(sizeof(CHISTREAM)));
    CHX_ASSERT(NULL != m_pMfnrBpsRegOutStream);

    m_pMfnrBpsRegInStream  = (CHISTREAM*)(CHX_CALLOC(sizeof(CHISTREAM)));
    CHX_ASSERT(NULL != m_pMfnrBpsRegInStream);

    m_pJPEGInputStream  = (CHISTREAM*)(CHX_CALLOC(sizeof(CHISTREAM)));
    CHX_ASSERT(NULL != m_pJPEGInputStream);

    if (TRUE == m_noiseReprocessEnable)
    {
        m_pNoiseReprocessInStream  = (CHISTREAM*)(CHX_CALLOC(sizeof(CHISTREAM)));
        CHX_ASSERT(NULL != m_pNoiseReprocessInStream);

        m_pNoiseReprocessOutStream = (CHISTREAM*)(CHX_CALLOC(sizeof(CHISTREAM)));
        CHX_ASSERT(NULL != m_pNoiseReprocessOutStream);
    }
}

VOID FeatureMFNR::ConfigureInternalStreams()
{
    CHX_LOG_INFO("Configuring the internal streams info");
    UINT       prefilterWidth  = m_pRdiStream->width;
    UINT       prefilterHeight = m_pRdiStream->height;
    UINT32     scale           = 1;
    UINT32     width           = 0;
    UINT32     height          = 0;

    for (UINT i = 0; i < MfnrReferenceMax; i++)
    {
        CHISTREAMFORMAT format       = (MfnrReferenceFull == i) ? ChiStreamFormatUBWCTP10 : ChiStreamFormatPD10;
        GrallocUsage    grallocUsage = 0;

        scale  = static_cast<UINT>(pow(4, i));
        width  = ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(prefilterWidth, scale) / scale);
        height = ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(prefilterHeight, scale) / scale);

        if (ChiStreamFormatUBWCTP10 == format)
        {
            grallocUsage = ChiGralloc1ProducerUsageCamera    |
                           ChiGralloc1ConsumerUsageCamera    |
                           ChiGralloc1ProducerUsagePrivate_0 |
                           (1ULL << 27);
        }

        m_pPrefilterOutStream[i]->format        = format;
        m_pPrefilterOutStream[i]->width         = width;
        m_pPrefilterOutStream[i]->height        = height;
        m_pPrefilterOutStream[i]->maxNumBuffers = 0;
        m_pPrefilterOutStream[i]->rotation      = StreamRotationCCW0;
        m_pPrefilterOutStream[i]->streamType    = ChiStreamTypeOutput;
        m_pPrefilterOutStream[i]->grallocUsage  = grallocUsage;

        *m_pBlendOutStream[i]                   = *m_pPrefilterOutStream[i];
        *m_pBlendInStream[i]                    = *m_pBlendOutStream[i];
        *m_pScaleInStream[i]                    = *m_pBlendOutStream[i];
        m_pBlendInStream[i]->streamType         = ChiStreamTypeInput;
        m_pScaleInStream[i]->streamType         = ChiStreamTypeInput;
        if (MfnrReferenceFull == i)
        {
            *m_pMfnrPostFilterOutStream         = *m_pPrefilterOutStream[i];
            if (TRUE == m_noiseReprocessEnable)
            {
                m_pMfnrPostFilterOutStream->format    = (CHISTREAMFORMAT)ChiStreamFormatYCbCr420_888;

                *m_pNoiseReprocessInStream            = *m_pMfnrPostFilterOutStream;
                m_pNoiseReprocessInStream->streamType = ChiStreamTypeInput;

                *m_pNoiseReprocessOutStream           = *m_pMfnrPostFilterOutStream;
                m_pNoiseReprocessOutStream->width     = m_pSnapshotStream->width;
                m_pNoiseReprocessOutStream->height    = m_pSnapshotStream->height;

                *m_pJPEGInputStream = *m_pNoiseReprocessOutStream;
            }
            else
            {
                m_pMfnrPostFilterOutStream->width   = m_pSnapshotStream->width;
                m_pMfnrPostFilterOutStream->height  = m_pSnapshotStream->height;
                m_pMfnrPostFilterOutStream->format  =(CHISTREAMFORMAT)HAL_PIXEL_FORMAT_YCrCb_420_SP;
                *m_pJPEGInputStream = *m_pMfnrPostFilterOutStream;
            }
            m_pJPEGInputStream->streamType = ChiStreamTypeInput;
        }

        CHX_LOG("[%d] wxh:%dx%d, m_pPrefilterOutStream:%dx%d, m_pMfnrPostFilterOutStream:%dx%d",
            i, width, height,
            m_pPrefilterOutStream[i]->width,
            m_pPrefilterOutStream[i]->height,
            m_pMfnrPostFilterOutStream->width,
            m_pMfnrPostFilterOutStream->height);
    }

    width  = ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(prefilterWidth / 3, 64));
    height = ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(prefilterHeight / 3, 64));

    m_pMfnrBpsRegOutStream->format        = ChiStreamFormatYCbCr420_888;
    m_pMfnrBpsRegOutStream->width         = width;
    m_pMfnrBpsRegOutStream->height        = height;
    m_pMfnrBpsRegOutStream->maxNumBuffers = 0;
    m_pMfnrBpsRegOutStream->rotation      = StreamRotationCCW0;
    m_pMfnrBpsRegOutStream->streamType    = ChiStreamTypeOutput;
    m_pMfnrBpsRegOutStream->grallocUsage  = 0;
    *m_pMfnrBpsRegInStream                = *m_pMfnrBpsRegOutStream;
    m_pMfnrBpsRegInStream->streamType     = ChiStreamTypeInput;

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMfnr::Pause
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::Pause(
       BOOL isForced)
{
    CHX_LOG_INFO("FeatureMfnr::Pause(), isForced %d E.", isForced);
    ChxUtils::AtomicStoreU32(&m_aPauseInProgress, TRUE);

    m_pMfnrResultMutex->Lock();

    m_resultsAvailable = TRUE;
    m_pMfnrResultAvailable->Signal();

    m_pMfnrResultMutex->Unlock();

    m_pOfflineRequestMutex->Lock();

    m_offlineRequestProcessTerminate = TRUE;
    m_pOfflineRequestAvailable->Signal();

    m_pOfflineRequestMutex->Unlock();

    ChxUtils::ThreadTerminate(m_offlineRequestProcessThread.hThreadHandle);
    CHX_LOG_INFO("FeatureMfnr::Pause(), isForced %d X.", isForced);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::Destroy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::Destroy(BOOL isForced)
{
    CHX_LOG("FeatureMFNR::Destroy(), isForced %d E.", isForced);

    m_pSnapshotStream = NULL;
    m_pPreviewStream  = NULL;
    m_pFdStream       = NULL;
    m_pRdiStream = NULL;

    for (UINT i = 0; i < MfnrReferenceMax; i++)
    {
        if (m_pPrefilterOutStream[i] != NULL)
        {
            CHX_FREE(m_pPrefilterOutStream[i]);
            m_pPrefilterOutStream[i] = NULL;
        }

        if (m_pBlendOutStream[i] != NULL)
        {
            CHX_FREE(m_pBlendOutStream[i]);
            m_pBlendOutStream[i] = NULL;
        }

        if (m_pBlendInStream[i] != NULL)
        {
            CHX_FREE(m_pBlendInStream[i]);
            m_pBlendInStream[i] = NULL;
        }
        if (m_pScaleInStream[i] != NULL)
        {
            CHX_FREE(m_pScaleInStream[i]);
            m_pScaleInStream[i] = NULL;
        }
    }

    if (m_pMfnrPostFilterOutStream != NULL)
    {
        CHX_FREE(m_pMfnrPostFilterOutStream);
        m_pMfnrPostFilterOutStream = NULL;
    }
    if (m_pMfnrBpsRegOutStream != NULL)
    {
        CHX_FREE(m_pMfnrBpsRegOutStream);
        m_pMfnrBpsRegOutStream = NULL;
    }

    if (m_pMfnrBpsRegInStream != NULL)
    {
        CHX_FREE(m_pMfnrBpsRegInStream);
        m_pMfnrBpsRegInStream = NULL;
    }

    if (m_pNoiseReprocessOutStream != NULL)
    {
        CHX_FREE(m_pNoiseReprocessOutStream);
        m_pNoiseReprocessOutStream = NULL;
    }

    if (m_pNoiseReprocessInStream != NULL)
    {
        CHX_FREE(m_pNoiseReprocessInStream);
        m_pNoiseReprocessInStream = NULL;
    }

    for (UINT j = 0; j < MfnrReferenceMax; j++)
    {
        if (NULL != m_pMfnrBufferManager[MfnrStagePrefilter][j])
        {
            m_pMfnrBufferManager[MfnrStagePrefilter][j]->Destroy();
            m_pMfnrBufferManager[MfnrStagePrefilter][j] = NULL;
        }
        if (NULL != m_pMfnrBufferManager[MfnrStageBlend][j])
        {
            m_pMfnrBufferManager[MfnrStageBlend][j]->Destroy();
            m_pMfnrBufferManager[MfnrStageBlend][j] = NULL;
        }
    }

    if (m_pMfnrBpsRegOutBufferManager != NULL)
    {
        m_pMfnrBpsRegOutBufferManager->Destroy();
        m_pMfnrBpsRegOutBufferManager = NULL;
    }

    if (NULL != m_pOfflineNoiseReprocessBufferManager)
    {
        m_pOfflineNoiseReprocessBufferManager->Destroy();
        m_pOfflineNoiseReprocessBufferManager = NULL;
    }

    if (NULL != m_pJPEGInputStream)
    {
        CHX_FREE(m_pJPEGInputStream);
        m_pJPEGInputStream = NULL;
    }

    for (UINT32 index = 0; index < MfnrMaxMetadata; ++index)
    {
        if (NULL != m_pInterStageInputMetadata[index])
        {
            m_pInterStageInputMetadata[index]->Destroy();
            m_pInterStageInputMetadata[index] = NULL;
        }
    }

    if (NULL != m_debugDataOffline.pData)
    {
        CHX_FREE(m_debugDataOffline.pData);
        m_debugDataOffline.pData    = NULL;
        m_debugDataOffline.size     = 0;
    }
    if (NULL != m_debugDataOfflineSnapshot.pData)
    {
        CHX_FREE(m_debugDataOfflineSnapshot.pData);
        m_debugDataOfflineSnapshot.pData    = NULL;
        m_debugDataOfflineSnapshot.size     = 0;
    }

    // Result destroy
    if (NULL != m_pMfnrResultAvailable)
    {
        m_pMfnrResultAvailable->Destroy();
        m_pMfnrResultAvailable = NULL;
    }

    // Mutex destroy
    if (NULL != m_pMfnrResultMutex)
    {
        m_pMfnrResultMutex->Destroy();
        m_pMfnrResultMutex = NULL;
    }

    // Condition destroy
    if (NULL != m_pRDIResultAvailable)
    {
        m_pRDIResultAvailable->Destroy();
        m_pRDIResultAvailable = NULL;
    }

    // Mutex destroy
    if (NULL != m_pRDIResultMutex)
    {
        m_pRDIResultMutex->Destroy();
        m_pRDIResultMutex = NULL;
    }

    // Condition destroy
    if (NULL != m_pSnapshotResultAvailable)
    {
        m_pSnapshotResultAvailable->Destroy();
        m_pSnapshotResultAvailable = NULL;
    }

    // Mutex destroy
    if (NULL != m_pSnapshotResultMutex)
    {
        m_pSnapshotResultMutex->Destroy();
        m_pSnapshotResultMutex = NULL;
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
/// FeatureMFNR::GetTargetIndex
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
INT FeatureMFNR::GetTargetIndex(ChiTargetPortDescriptorInfo* pTargets, const char* pTargetName)
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
/// FeatureMFNR::GetOutputBuffer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::GetOutputBuffer(
    CHIBufferManager*   pBufferManager,
    ChiStream*          pChiStream,
    CHISTREAMBUFFER*    pOutputBuffer)
{
    CDKResult result   = CDKResultSuccess;
    UINT      refCount = 0;

    pOutputBuffer->size               = sizeof(CHISTREAMBUFFER);
    pOutputBuffer->bufferInfo         = pBufferManager->GetImageBufferInfo();
    pOutputBuffer->acquireFence.valid = FALSE;
    pOutputBuffer->pStream            = pChiStream;

    refCount                          = pBufferManager->GetReference(&pOutputBuffer->bufferInfo);

    CHX_LOG_VERBOSE("pBufferManager:%p phBuffer:%p refCount:%u",
        pBufferManager,
        pOutputBuffer->bufferInfo.phBuffer,
        refCount);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::OverrideUsecase
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ChiUsecase* FeatureMFNR::OverrideUsecase(
    LogicalCameraInfo*              pCameraInfo,
    camera3_stream_configuration_t* pStreamConfig)
{
    (VOID)pCameraInfo;

    ChiPipelineTargetCreateDescriptor* pPreviewDesc = NULL;
    INT32                              previewPipelineIndex;

    CHX_LOG("OverrideUsecase for MFNR, instanceID:%d", m_physicalCameraIndex);

    CHX_ASSERT(2 == pStreamConfig->num_streams);

    m_captureRequest.output_buffers = static_cast<camera3_stream_buffer_t*>(
        CHX_CALLOC(sizeof(camera3_stream_buffer_t) * pStreamConfig->num_streams));

    m_pPreviewStream     = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::PreviewStream);
    m_pRdiStream         = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::RdiStream, m_physicalCameraIndex);
    m_pFdStream          = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::FdStream, m_physicalCameraIndex);
    m_pSnapshotStream    = m_pUsecase->GetSharedStream(AdvancedCameraUsecase::SnapshotStream);

    m_pChiUsecase        = m_pUsecase->GetChiUseCase();

    m_isJPEGSnapshotConfigured = m_pUsecase->IsJPEGSnapshotStream();
    CHX_LOG("m_isJPEGSnapshotConfigured %d", m_isJPEGSnapshotConfigured);

    if (StreamConfigModeFastShutter == ExtensionModule::GetInstance()->GetOpMode(m_pUsecase->GetCameraId()))
    {
        previewPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                    AdvancedPipelineType::ZSLPreviewRawFSType, m_physicalCameraIndex);
    }
    else
    {
        previewPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
            AdvancedPipelineType::ZSLPreviewRawType, m_physicalCameraIndex);
    }

    pPreviewDesc         = &m_pChiUsecase->pPipelineTargetCreateDesc[previewPipelineIndex];

    CHX_LOG("preview idx:%d, previewDesc:%p", previewPipelineIndex, pPreviewDesc);

    SetupInternalPipelines();

    return m_pChiUsecase;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SetupInternalMFNRPreFilterPipeline - pre-filter pipeline (#1)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::SetupInternalMFNRPreFilterPipeline()
{
    AdvancedPipelineType               pipelineType;
    UINT                               mfnrFullRefIndex;
    UINT                               mfnrDs4Index;
    UINT                               mfnrBpsRegOutUIndex;
    INT32                              prefilterPipelineIndex;
    ChiPipelineTargetCreateDescriptor* pPrefilterPipelineDesc = NULL;
    const CHAR*                        pTargetName            = NULL;
    ChiTargetPortDescriptorInfo*       pSinkTarget            = NULL;
    ChiTargetPortDescriptorInfo*       pSrcTarget             = NULL;

    CHX_LOG("SetupInternalMFNRPreFilterPipeline");

    pipelineType           = AdvancedPipelineType::MFNRPrefilterType;

    prefilterPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                pipelineType, m_physicalCameraIndex);
    pPrefilterPipelineDesc = &m_pChiUsecase->pPipelineTargetCreateDesc[prefilterPipelineIndex];

    // Source Port(s)
    pSrcTarget          = &pPrefilterPipelineDesc->sourceTarget;

    pSrcTarget->pTargetPortDesc[0].pTarget->pChiStream = m_pRdiStream;

    // Sink Port(s)
    pSinkTarget         = &pPrefilterPipelineDesc->sinkTarget;

    pTargetName         = "TARGET_BUFFER_FULL_OUT_REF";
    mfnrFullRefIndex    = GetTargetIndex(pSinkTarget, pTargetName);
    pSinkTarget->pTargetPortDesc[mfnrFullRefIndex].pTarget->pChiStream    = m_pPrefilterOutStream[MfnrReferenceFull];

    pTargetName         = "TARGET_BUFFER_DS4_OUT_REF";
    mfnrDs4Index        = GetTargetIndex(pSinkTarget, pTargetName);
    pSinkTarget->pTargetPortDesc[mfnrDs4Index].pTarget->pChiStream        = m_pPrefilterOutStream[MfnrReferenceDS4];

    pTargetName         = "TARGET_BUFFER_REG_OUT";
    mfnrBpsRegOutUIndex = GetTargetIndex(pSinkTarget, pTargetName);
    pSinkTarget->pTargetPortDesc[mfnrBpsRegOutUIndex].pTarget->pChiStream = m_pMfnrBpsRegOutStream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SetupInternalMFNRBlendPipeline - Blend pipeline (#2)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::SetupInternalMFNRBlendPipeline()
{
    AdvancedPipelineType               pipelineType;
    INT32                              blendPipelineIndex;
    UINT                               mfnrDs4RefIndex;
    UINT                               mfnrInFullRefIndex;
    UINT                               mfnrInDs4RefIndex;
    UINT                               mfnrInRawIndex;
    UINT                               mfnrInRegOutIndex;
    UINT                               mfnrFullRefIndex;
    UINT                               mfnrDs4Index;
    ChiPipelineTargetCreateDescriptor* pBlendPipelineDesc = NULL;
    const CHAR*                        pTargetName        = NULL;
    ChiTargetPortDescriptorInfo*       pSinkTarget        = NULL;
    ChiTargetPortDescriptorInfo*       pSrcTarget         = NULL;

    CHX_LOG("SetupInternalMFNRBlendPipeline");

    pipelineType       = AdvancedPipelineType::MFNRBlendType;


    blendPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                            pipelineType, m_physicalCameraIndex);
    pBlendPipelineDesc = &m_pChiUsecase->pPipelineTargetCreateDesc[blendPipelineIndex];

    // Source Port(s)
    pSrcTarget         = &pBlendPipelineDesc->sourceTarget;

    pTargetName        = "TARGET_BUFFER_DS4_SCALE_IN_REF";
    mfnrDs4RefIndex    = GetTargetIndex(pSrcTarget, pTargetName);
    pSrcTarget->pTargetPortDesc[mfnrDs4RefIndex].pTarget->pChiStream    = m_pScaleInStream[MfnrReferenceDS4];

    pTargetName        = "TARGET_BUFFER_FULL_IN_REF";
    mfnrInFullRefIndex = GetTargetIndex(pSrcTarget, pTargetName);
    pSrcTarget->pTargetPortDesc[mfnrInFullRefIndex].pTarget->pChiStream = m_pBlendInStream[MfnrReferenceFull];

    pTargetName        = "TARGET_BUFFER_DS4_IN_REF";
    mfnrInDs4RefIndex  = GetTargetIndex(pSrcTarget, pTargetName);
    pSrcTarget->pTargetPortDesc[mfnrInDs4RefIndex].pTarget->pChiStream  = m_pBlendInStream[MfnrReferenceDS4];

    pTargetName        = "TARGET_BUFFER_RAW";
    mfnrInRawIndex     = GetTargetIndex(pSrcTarget, pTargetName);
    pSrcTarget->pTargetPortDesc[mfnrInRawIndex].pTarget->pChiStream     = m_pRdiStream;

    pTargetName        = "TARGET_BUFFER_REG_IN";
    mfnrInRegOutIndex  = GetTargetIndex(pSrcTarget, pTargetName);
    pSrcTarget->pTargetPortDesc[mfnrInRegOutIndex].pTarget->pChiStream  = m_pMfnrBpsRegInStream;

    // Sink Ports
    pSinkTarget        = &pBlendPipelineDesc->sinkTarget;

    pTargetName        = "TARGET_BUFFER_FULL_OUT_REF";
    mfnrFullRefIndex   = GetTargetIndex(pSinkTarget, pTargetName);
    pSinkTarget->pTargetPortDesc[mfnrFullRefIndex].pTarget->pChiStream  = m_pBlendOutStream[MfnrReferenceFull];

    pTargetName        = "TARGET_BUFFER_DS4_OUT_REF";
    mfnrDs4Index       = GetTargetIndex(pSinkTarget, pTargetName);
    pSinkTarget->pTargetPortDesc[mfnrDs4Index].pTarget->pChiStream      = m_pBlendOutStream[MfnrReferenceDS4];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SetupInternalMFNRBlendPipeline - post-filter pipeline (#3)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::SetupInternalMFNRPostFilterPipeline()
{
    AdvancedPipelineType               pipelineType;
    UINT                               mfnrDs4RefIndex;
    UINT                               mfnrInFullRefIndex;
    UINT                               mfnrInDs4RefIndex;
    UINT                               mfnrInRawIndex;
    UINT                               mfnrInRegOutIndex;
    UINT                               mfnrPostFilterOutIndex;
    INT32                              postfilterPipelineIndex;
    ChiPipelineTargetCreateDescriptor* pPostfilterPipelineDesc = NULL;
    ChiTargetPortDescriptorInfo*       pSinkTarget             = NULL;
    ChiTargetPortDescriptorInfo*       pSrcTarget              = NULL;
    const CHAR*                        pTargetName             = NULL;

    pipelineType            = AdvancedPipelineType::MFNRPostFilterType;

    CHX_LOG("SetupInternalMFNRPostFilterPipeline");

    postfilterPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                pipelineType, m_physicalCameraIndex);
    pPostfilterPipelineDesc = &m_pChiUsecase->pPipelineTargetCreateDesc[postfilterPipelineIndex];

    // Source Port(s)
    pSrcTarget         = &pPostfilterPipelineDesc->sourceTarget;

    pTargetName        = "TARGET_BUFFER_DS4_SCALE_IN_REF";
    mfnrDs4RefIndex    = GetTargetIndex(pSrcTarget, pTargetName);
    pSrcTarget->pTargetPortDesc[mfnrDs4RefIndex].pTarget->pChiStream    = m_pScaleInStream[MfnrReferenceDS4];

    pTargetName        = "TARGET_BUFFER_FULL_IN_REF";
    mfnrInFullRefIndex = GetTargetIndex(pSrcTarget, pTargetName);
    pSrcTarget->pTargetPortDesc[mfnrInFullRefIndex].pTarget->pChiStream = m_pBlendInStream[MfnrReferenceFull];

    pTargetName        = "TARGET_BUFFER_DS4_IN_REF";
    mfnrInDs4RefIndex  = GetTargetIndex(pSrcTarget, pTargetName);
    pSrcTarget->pTargetPortDesc[mfnrInDs4RefIndex].pTarget->pChiStream  = m_pBlendInStream[MfnrReferenceDS4];

    pTargetName        = "TARGET_BUFFER_RAW";
    mfnrInRawIndex     = GetTargetIndex(pSrcTarget, pTargetName);
    pSrcTarget->pTargetPortDesc[mfnrInRawIndex].pTarget->pChiStream     = m_pRdiStream;

    pTargetName        = "TARGET_BUFFER_REG_IN";
    mfnrInRegOutIndex  = GetTargetIndex(pSrcTarget, pTargetName);
    pSrcTarget->pTargetPortDesc[mfnrInRegOutIndex].pTarget->pChiStream  = m_pMfnrBpsRegInStream;

    // Sink Port(s)
    pSinkTarget        = &pPostfilterPipelineDesc->sinkTarget;

    pTargetName            = "TARGET_BUFFER_YUV_MFNR";
    mfnrPostFilterOutIndex = GetTargetIndex(pSinkTarget, pTargetName);

    if (TRUE == m_noiseReprocessEnable)
    {
        pSinkTarget->pTargetPortDesc[mfnrPostFilterOutIndex].pTarget->pChiStream = m_pNoiseReprocessInStream;
    }
    else if (TRUE == IsJPEGSnapshotConfigured())
    {
        pSinkTarget->pTargetPortDesc[mfnrPostFilterOutIndex].pTarget->pChiStream = m_pMfnrPostFilterOutStream;
    }
    else
    {
        pSinkTarget->pTargetPortDesc[mfnrPostFilterOutIndex].pTarget->pChiStream = m_pSnapshotStream;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SetupInternalMFNRBlendPipeline - jpeg pipeline (#4 | final)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::SetupInternalMFNRSnapshotPipeline()
{
    AdvancedPipelineType               pipelineType;
    INT32                              snapshotPipelineIndex;
    ChiPipelineTargetCreateDescriptor* pSnapshotDesc = NULL;
    ChiTargetPortDescriptorInfo*       pSinkTarget   = NULL;
    ChiTargetPortDescriptorInfo*       pSrcTarget    = NULL;

    CHX_LOG("SetupInternalMFNRSnapshotPipeline");

    pipelineType          = AdvancedPipelineType::InternalZSLYuv2JpegMFNRType;

    snapshotPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(
                                pipelineType, m_physicalCameraIndex);
    pSnapshotDesc         = &m_pChiUsecase->pPipelineTargetCreateDesc[snapshotPipelineIndex];

    // Source Port(s)
    pSrcTarget            = &pSnapshotDesc->sourceTarget;

    pSrcTarget->pTargetPortDesc[0].pTarget->pChiStream  = m_pJPEGInputStream;

    // Sink Port(s)
    pSinkTarget           = &pSnapshotDesc->sinkTarget;

    pSinkTarget->pTargetPortDesc[0].pTarget->pChiStream = m_pSnapshotStream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SetupOfflineNoiseReprocessPipeline - noise reprocess pipeline
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::SetupOfflineNoiseReprocessPipeline()
{
    AdvancedPipelineType               pipelineType;
    INT32                              reprocessPipelineIndex;
    UINT                               reprocessYUVInIndex;
    const CHAR*                        pTargetName    = NULL;
    ChiPipelineTargetCreateDescriptor* pReprocessDesc = NULL;
    ChiTargetPortDescriptorInfo*       pSinkTarget    = NULL;
    ChiTargetPortDescriptorInfo*       pSrcTarget     = NULL;

    CHX_LOG("SetupOfflineNoiseReprocessPipeline");

    pipelineType           = AdvancedPipelineType::OfflineNoiseReprocessType;

    reprocessPipelineIndex = m_pUsecase->GetPipelineIdByAdvancedPipelineType(pipelineType, m_physicalCameraIndex);
    pReprocessDesc         = &m_pChiUsecase->pPipelineTargetCreateDesc[reprocessPipelineIndex];

    // Source Port(s)
    pSrcTarget  = &pReprocessDesc->sourceTarget;

    pTargetName = "TARGET_BUFFER_YUV_IN";
    reprocessYUVInIndex = GetTargetIndex(pSrcTarget, pTargetName);
    pSrcTarget->pTargetPortDesc[reprocessYUVInIndex].pTarget->pChiStream = m_pNoiseReprocessInStream;

    // Sink Port(s)
    pSinkTarget = &pReprocessDesc->sinkTarget;
    if (TRUE == IsJPEGSnapshotConfigured())
    {
        pSinkTarget->pTargetPortDesc[0].pTarget->pChiStream = m_pNoiseReprocessOutStream;
    }
    else
    {
        pSinkTarget->pTargetPortDesc[0].pTarget->pChiStream = m_pSnapshotStream;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SetupInternalPipelines
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::SetupInternalPipelines()
{
    ConfigureInternalStreams();

    SetupInternalMFNRPreFilterPipeline();

    SetupInternalMFNRBlendPipeline();

    SetupInternalMFNRPostFilterPipeline();

    if (TRUE == m_noiseReprocessEnable)
    {
        SetupOfflineNoiseReprocessPipeline();
    }

    if (TRUE == IsJPEGSnapshotConfigured())
    {
        SetupInternalMFNRSnapshotPipeline();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::PipelineCreated
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::PipelineCreated(
    UINT sessionId,         ///< Id of session created
    UINT pipelineIndex)     ///< Index of the pipeline created (within the context of the session)
{
    CDKResult                      result            = CDKResultSuccess;
    ChiSensorModeInfo*             pSensorInfo       = NULL;
    const ChiPipelineInputOptions* pInputOptions     = NULL;
    Pipeline*                      pAdvancedPipeline = NULL;
    UINT                           pipelineId        =
        m_pUsecase->GetSessionData(sessionId)->pipelines[pipelineIndex].id;

    // FEATURE_MULTIFRAME_PORT
    /// @todo need to find the mapping between session's pipelineId vs XMLs pipelineId
    UINT   sessionPipelineId = m_pUsecase->GetSessionData(sessionId)->numPipelines - 1;
    INT32 advancedPipelineId = m_pUsecase->GetAdvancedPipelineTypeByPipelineId(pipelineId);

    pAdvancedPipeline  = m_pUsecase->GetSessionData(sessionId)->pipelines[sessionPipelineId].pPipeline;
    pSensorInfo        = pAdvancedPipeline->GetSensorModeInfo();
    pInputOptions      = pAdvancedPipeline->GetInputOptions();

    CHX_LOG_INFO("MFNR pipelineId: %d, adv pipelineId %d, Sensor widthxheight: %d x %d, nodeId %d, "
        "nodeInstanceId %d nodePortId %d, "
        "InputOptions widthxheight: %d x %d",
        pipelineId,
        advancedPipelineId,
        pSensorInfo->frameDimension.width,
        pSensorInfo->frameDimension.height,
        pInputOptions->nodePort.nodeId,
        pInputOptions->nodePort.nodeInstanceId,
        pInputOptions->nodePort.nodePortId,
        pInputOptions->bufferOptions.optimalDimension.width,
        pInputOptions->bufferOptions.optimalDimension.height);

    // FEATURE_MULTIFRAME_PORT
    /// @todo - Need to find means of associating grallocUsage

    switch (advancedPipelineId)
    {
    case AdvancedPipelineType::MFNRPrefilterType:
        {
            CHX_LOG_INFO("%s", PipelineType[advancedPipelineId]);

            if ((m_pRdiStream->width == 0) || (m_pRdiStream->height == 0))
            {
                CHX_LOG_WARN("Invalid m_pRdiStream dimension!");
            }
            break;
        }

    case AdvancedPipelineType::MFNRPostFilterType:
        {
            CHX_LOG_INFO("%s", PipelineType[advancedPipelineId]);
            UINT32     bufferWidth  = m_pRdiStream->width;
            UINT32     bufferHeight = m_pRdiStream->height;
            UINT32     scale        = 1;
            UINT32     width        = 0;
            UINT32     height       = 0;

            // In upscale usecase, postfiler stream is larger than prefilter
            // so use the larger one for buffer allocation here
            if (m_pMfnrPostFilterOutStream->width > bufferWidth)
            {
                bufferWidth = m_pMfnrPostFilterOutStream->width;
            }
            if (m_pMfnrPostFilterOutStream->height > bufferHeight)
            {
                bufferHeight = m_pMfnrPostFilterOutStream->height;
            }
            CHX_LOG("prefilter stream buffer size: %dx%d", bufferWidth, bufferHeight);

            for (UINT i = 0; i < MfnrReferenceMax; i++)
            {
                CHIBufferManagerCreateData createPreFilterBuffers = { 0 };

                scale  = static_cast<UINT>(pow(4, i));
                width  = ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(bufferWidth, scale) / scale);
                height = ChxUtils::EvenCeilingUINT32(ChxUtils::AlignGeneric32(bufferHeight, scale) / scale);

                createPreFilterBuffers.width                = (m_pPrefilterOutStream[i]->format == ChiStreamFormatPD10) ?
                                                                  (width * 4) : width;
                createPreFilterBuffers.height               = height;
                createPreFilterBuffers.format               = m_pPrefilterOutStream[i]->format;
                createPreFilterBuffers.producerFlags        = ChiGralloc1ProducerUsageCamera   |
                                                              ChiGralloc1ProducerUsageCpuRead |
                                                              ChiGralloc1ProducerUsageCpuWrite;
                createPreFilterBuffers.consumerFlags        = ChiGralloc1ConsumerUsageCamera   |
                                                              ChiGralloc1ConsumerUsageCpuRead;
                createPreFilterBuffers.immediateBufferCount = 0;
                createPreFilterBuffers.maxBufferCount       = MfnrMaxPreFilterStageBuffers;
                createPreFilterBuffers.bEnableLateBinding   = ExtensionModule::GetInstance()->EnableCHILateBinding();
                createPreFilterBuffers.bufferHeap           = BufferHeapDefault;
                createPreFilterBuffers.pChiStream           = m_pPrefilterOutStream[i];
                if (ChiStreamFormatUBWCTP10 == m_pPrefilterOutStream[i]->format)
                {
                    createPreFilterBuffers.producerFlags |= ChiGralloc1ProducerUsagePrivate_0 | (1ULL << 27);
                    createPreFilterBuffers.consumerFlags |= ChiGralloc1ConsumerUsagePrivate_0 | (1ULL << 27);
                }

                if (NULL == m_pMfnrBufferManager[MfnrStagePrefilter][i])
                {
                    m_pMfnrBufferManager[MfnrStagePrefilter][i] = CHIBufferManager::Create(mfnrBufferManagerNames[i],
                                                                                           &createPreFilterBuffers);
                    if (NULL == m_pMfnrBufferManager[MfnrStagePrefilter][i])
                    {
                        CHX_LOG_ERROR("m_pMfnrBufferManager MfnrStagePrefilter %d allocate fail!!", i);
                        result = CDKResultENoMemory;
                    }
                    else
                    {
                        CHX_LOG_INFO("m_pMfnrBufferManager MfnrStagePrefilter %p success",
                                     m_pMfnrBufferManager[MfnrStagePrefilter][i]);
                    }
                }
                else
                {
                    CHX_LOG_INFO("m_pMfnrBufferManager MfnrStagePrefilter %p already created!",
                                 m_pMfnrBufferManager[MfnrStagePrefilter][i]);
                }
            }

            CHIBufferManagerCreateData createBpsRegOutBuffers = { 0 };

            createBpsRegOutBuffers.width                = m_pMfnrBpsRegOutStream->width;
            createBpsRegOutBuffers.height               = m_pMfnrBpsRegOutStream->height;
            createBpsRegOutBuffers.format               = m_pMfnrBpsRegOutStream->format;
            createBpsRegOutBuffers.producerFlags        = ChiGralloc1ProducerUsageCamera   |
                                                          ChiGralloc1ProducerUsageCpuRead |
                                                          ChiGralloc1ProducerUsageCpuWrite;
            createBpsRegOutBuffers.consumerFlags        = ChiGralloc1ConsumerUsageCamera   |
                                                          ChiGralloc1ConsumerUsageCpuRead;
            createBpsRegOutBuffers.immediateBufferCount = 0;
            createBpsRegOutBuffers.maxBufferCount       = MfnrMaxBpsRegOutBuffers;
            createBpsRegOutBuffers.bEnableLateBinding   = ExtensionModule::GetInstance()->EnableCHILateBinding();
            createBpsRegOutBuffers.bufferHeap           = BufferHeapDefault;
            createBpsRegOutBuffers.pChiStream           = m_pMfnrBpsRegOutStream;

            if (NULL == m_pMfnrBpsRegOutBufferManager)
            {
                m_pMfnrBpsRegOutBufferManager = CHIBufferManager::Create("MfnrBpsRegOutBufferManager", &createBpsRegOutBuffers);

                if (NULL == m_pMfnrBpsRegOutBufferManager) {
                    CHX_LOG_ERROR("m_pMfnrBpsRegOutBufferManager allocate fail!!");
                    result = CDKResultENoMemory;
                }
                else
                {
                    CHX_LOG_INFO("m_pMfnrBpsRegOutBufferManager %p success", m_pMfnrBpsRegOutBufferManager);
                }
            }
            else
            {
                CHX_LOG_INFO("m_pMfnrBpsRegOutBufferManager %p already created!", m_pMfnrBpsRegOutBufferManager);
            }

            break;
            SetFeatureStatus(FeatureStatus::READY);
        }
        case AdvancedPipelineType::MFNRBlendType:
        case AdvancedPipelineType::ZSLPreviewRawType:
        case AdvancedPipelineType::InternalZSLYuv2JpegMFNRType:
        {
            CHX_LOG_INFO("%s", PipelineType[advancedPipelineId]);
            break;
        }
        case AdvancedPipelineType::OfflineNoiseReprocessType:
        {
            CHX_LOG_INFO("%s", PipelineType[advancedPipelineId]);

            CHIBufferManagerCreateData createOfflineNoiseReprocessBuffers = { 0 };

            createOfflineNoiseReprocessBuffers.width                = m_pNoiseReprocessOutStream->width;
            createOfflineNoiseReprocessBuffers.height               = m_pNoiseReprocessOutStream->height;
            createOfflineNoiseReprocessBuffers.format               = m_pNoiseReprocessOutStream->format;
            createOfflineNoiseReprocessBuffers.producerFlags        = ChiGralloc1ProducerUsageCamera   |
                                                                      ChiGralloc1ProducerUsageCpuRead |
                                                                      ChiGralloc1ProducerUsageCpuWrite;
            createOfflineNoiseReprocessBuffers.consumerFlags        = ChiGralloc1ConsumerUsageCamera   |
                                                                      ChiGralloc1ConsumerUsageCpuRead;
            createOfflineNoiseReprocessBuffers.immediateBufferCount = 0;
            createOfflineNoiseReprocessBuffers.maxBufferCount       = MaxChiStreamBuffers;
            createOfflineNoiseReprocessBuffers.bEnableLateBinding   = ExtensionModule::GetInstance()->EnableCHILateBinding();
            createOfflineNoiseReprocessBuffers.bufferHeap           = BufferHeapDefault;
            createOfflineNoiseReprocessBuffers.pChiStream           = m_pNoiseReprocessOutStream;

            if (NULL == m_pOfflineNoiseReprocessBufferManager)
            {
                m_pOfflineNoiseReprocessBufferManager =
                    CHIBufferManager::Create("OfflineNoiseReprocessBufferManager", &createOfflineNoiseReprocessBuffers);

                if (NULL == m_pOfflineNoiseReprocessBufferManager)
                {
                    CHX_LOG_ERROR("m_pOfflineNoiseReprocessBufferManager allocate fail!!");
                    result = CDKResultENoMemory;
                }
                else
                {
                    CHX_LOG_INFO("m_pOfflineNoiseReprocessBufferManager %p success",
                                 m_pOfflineNoiseReprocessBufferManager);
                }
            }
            else
            {
                CHX_LOG_INFO("m_pOfflineNoiseReprocessBufferManager %p already created!",
                             m_pOfflineNoiseReprocessBufferManager);
            }

            break;
        }
        case AdvancedPipelineType::ZSLSnapshotJpegType:
        {
            ConfigureInternalStreams();
            break;
        }
        default:
        {
            CHX_LOG_VERBOSE("Unknown advanced pipeline type %d for pipelineId %u!",
                advancedPipelineId,
                pipelineId);
            result = CDKResultEFailed;
            break;
        }
    }

    m_rdiStreamIndex   = m_pUsecase->GetInternalTargetBufferIndex(m_pRdiStream);
    m_fdStreamIndex    = m_pUsecase->GetInternalTargetBufferIndex(m_pFdStream);
    m_pRdiTargetBuffer = m_pUsecase->GetTargetBufferPointer(m_rdiStreamIndex);
    m_pFdTargetBuffer  = m_pUsecase->GetTargetBufferPointer(m_fdStreamIndex);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::PipelineDestroyed
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::PipelineDestroyed(
    UINT32 sessionId,         ///< Id of session created
    UINT32 pipelineIndex)     ///< Index of the pipeline created (within the context of the session)
{
    CDKResult                      result            = CDKResultSuccess;
    ChiSensorModeInfo*             pSensorInfo       = NULL;
    const ChiPipelineInputOptions* pInputOptions     = NULL;
    Pipeline*                      pAdvancedPipeline = NULL;
    UINT                           pipelineId        =
        m_pUsecase->GetSessionData(sessionId)->pipelines[pipelineIndex].id;

    INT32 advancedPipelineId = m_pUsecase->GetAdvancedPipelineTypeByPipelineId(pipelineId);

    CHX_LOG_INFO("MFNR pipelineIndex:%d, pipelineId: %d, AdvancedPipelineType:%d",
        pipelineIndex, pipelineId, advancedPipelineId);

    switch (advancedPipelineId)
    {
    case AdvancedPipelineType::InternalZSLYuv2JpegMFNRType:
    case AdvancedPipelineType::InternalZSLYuv2JpegMFNRAuxType:
        {
            CameraType type = CameraType::Wide;

            if (AdvancedPipelineType::InternalZSLYuv2JpegMFNRAuxType == advancedPipelineId)
            {
                type = CameraType::Tele;
            }

            for (UINT i = 0; i < MfnrReferenceMax; i++)
            {
                ChxUtils::Memset(m_pPrefilterOutStream[i], 0, sizeof(ChiStream));
                ChxUtils::Memset(m_pBlendOutStream[i],     0, sizeof(ChiStream));
                ChxUtils::Memset(m_pBlendInStream[i],      0, sizeof(ChiStream));
                ChxUtils::Memset(m_pScaleInStream[i],      0, sizeof(ChiStream));
            }

            ChxUtils::Memset(m_pMfnrPostFilterOutStream, 0, sizeof(ChiStream));
            ChxUtils::Memset(m_pMfnrBpsRegOutStream,     0, sizeof(ChiStream));
            ChxUtils::Memset(m_pMfnrBpsRegInStream,      0, sizeof(ChiStream));

            break;
        }

    case AdvancedPipelineType::MFNRPostFilterType:
    case AdvancedPipelineType::MFNRPostFilterAuxType:
        {
            CameraType type = CameraType::Wide;

            if (AdvancedPipelineType::MFNRPostFilterAuxType == advancedPipelineId)
            {
                type = CameraType::Tele;
            }

            for (UINT j = 0; j < MfnrReferenceMax; j++)
            {
                if (NULL != m_pMfnrBufferManager[MfnrStagePrefilter][j])
                {
                    m_pMfnrBufferManager[MfnrStagePrefilter][j]->Destroy();
                    m_pMfnrBufferManager[MfnrStagePrefilter][j] = NULL;
                }
                if (NULL != m_pMfnrBufferManager[MfnrStageBlend][j])
                {
                    m_pMfnrBufferManager[MfnrStageBlend][j]->Destroy();
                    m_pMfnrBufferManager[MfnrStageBlend][j] = NULL;
                }
            }

            if (m_pMfnrBpsRegOutBufferManager != NULL)
            {
                m_pMfnrBpsRegOutBufferManager->Destroy();
                m_pMfnrBpsRegOutBufferManager = NULL;
            }

            break;
        }

    case AdvancedPipelineType::MFNRPrefilterType:
    case AdvancedPipelineType::MFNRPrefilterAuxType:
    case AdvancedPipelineType::MFNRBlendType:
    case AdvancedPipelineType::ZSLPreviewRawType:
        {
            CHX_LOG_INFO("%s", PipelineType[advancedPipelineId]);
            break;
        }

    case AdvancedPipelineType::OfflineNoiseReprocessType:
        {
            if (NULL != m_pOfflineNoiseReprocessBufferManager)
            {
                m_pOfflineNoiseReprocessBufferManager->Destroy();
                m_pOfflineNoiseReprocessBufferManager = NULL;
            }
            break;
        }

    default:
        {
            CHX_LOG_ERROR("Unknown advanced pipeline type %d for pipelineId %u!",
                advancedPipelineId,
                pipelineId);
            result = CDKResultEFailed;
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::ExecuteProcessRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::ExecuteProcessRequest(
    camera3_capture_request_t* pRequest)         ///< Request parameters
{
    CDKResult       result             = CDKResultSuccess;
    CHISTREAMBUFFER previewBuffers[3]  = { { 0 } };
    UINT            previewCount       = 0;
    UINT            snapshotCount      = 0;
    UINT32          frameNumber        = pRequest->frame_number;
    UINT32          frameIndex         = (pRequest->frame_number % MaxOutstandingRequests);
    UINT            snapshotReqIdIndex = (m_maxSnapshotReqId % MaxOutstandingRequests);
    ChiMetadata*    pChiInputMetadata  = NULL;
    ChiMetadata*    pChiOutputMetadata = NULL;
    BOOL            nonZSLMFNRShot     = FALSE;
    Feature*        pPreviousFeature   = NULL;

    pPreviousFeature = m_pUsecase->GetPreviousFeatureForSnapshot(pRequest->frame_number, this);
    if (NULL != pPreviousFeature)
    {
        m_blockPreviewForSnapshot = TRUE;
    }
    else
    {
        m_blockPreviewForSnapshot = FALSE;
    }

    if (TRUE == m_pUsecase->IsMultiCameraUsecase())
    {
        RequestMapInfo requestMapInfo = { 0 };

        m_internalRequestId  = pRequest->frame_number;
        requestMapInfo       = m_pUsecase->GetRequestMapInfo(pRequest->frame_number);
        m_requestFrameNumber = requestMapInfo.frameNumber;

        m_sensorModeIndex = m_pUsecase->GetRealtimeSession()->GetSensorModeInfo(m_physicalCameraIndex)->modeIndex;

        CHX_LOG_INFO("FeatureMFNR Snapshot Request for dual camera!, rtReqId:%d, m_requestFrameNumber %d"
                "masterCameraId:%d, activePipelineID:%d, instance:%d",
                pRequest->frame_number, m_requestFrameNumber,
                requestMapInfo.masterCameraID,
                requestMapInfo.activePipelineID,
                m_physicalCameraIndex);
    }

    if (TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress)))
    {
        m_resultsAvailable = FALSE;
        m_pMfnrResultMutex->Unlock();

        CHX_LOG_INFO("MFNR pre filter scale process return because of cleanup");
        return CDKResultSuccess;
    }

    if ((FALSE == m_pUsecase->IsMultiCameraUsecase()) && (NULL == pPreviousFeature))
    {
        camera3_capture_result_t* pUsecaseResult = m_pUsecase->GetCaptureResult(frameIndex);

        pUsecaseResult->result             = NULL;
        pUsecaseResult->frame_number       = pRequest->frame_number;
        pUsecaseResult->num_output_buffers = 0;

        m_isSnapshotFrame[frameIndex]      = FALSE;

        m_sensorModeIndex = m_pUsecase->GetSessionData(m_realtime)->pSession->GetSensorModeInfo()->modeIndex;

        ChxUtils::AndroidMetadata::FillTuningModeData(const_cast<camera_metadata_t*>(pRequest->settings),
            pRequest,
            m_sensorModeIndex,
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
                ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i],
                                                       &previewBuffers[previewCount]);
                previewCount++;
            }

            if (m_pSnapshotStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream))
            {
                BOOL         rejectSnapshot = m_pUsecase->m_rejectedSnapshotRequestList[
                                                  pRequest->frame_number % MaxOutstandingRequests];

                if (m_pRdiTargetBuffer->lastReadySequenceID == INVALIDSEQUENCEID)
                {
                    nonZSLMFNRShot = TRUE;
                }

                CHX_LOG("Snapshot Frame %d last request %u",
                        pRequest->frame_number,
                        m_pRdiTargetBuffer->lastReadySequenceID);

                // For FS2 mode we need 8 RDI.
                if (StreamConfigModeFastShutter != ExtensionModule::GetInstance()->GetOpMode(m_pUsecase->GetCameraId()))
                {
                    CalculateMFNRTotalFramesByGain();
                }
                else
                {

                    m_mfnrTotalNumFrames = MfnrNumFramesforFS;
                }

                if (FALSE == rejectSnapshot)
                {
                    m_requestFrameNumber = pRequest->frame_number;

                    ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i],
                        &m_snapshotBuffers[snapshotReqIdIndex][snapshotCount]);

                    snapshotCount++;
                    m_allRDIResultsAvaliable = FALSE;
                    m_allFDResultsAvaliable  = FALSE;
                    m_isLLSSnapshot          = FALSE;
                    SetFeatureStatus(FeatureStatus::BUSY);
                }
                else
                {
                    CHX_LOG_INFO("Rejecting  only snapshot request for frame %d", pRequest->frame_number);
                }

            }
        }

        // FS2 mode there is no preview stream for snapshot request
        if (StreamConfigModeFastShutter == ExtensionModule::GetInstance()->GetOpMode(m_pUsecase->GetCameraId()))
        {
            if ((snapshotCount > 0) && (previewCount > 0))
            {
                CHX_LOG_ERROR("FeatureMFNR: FS2 mode preview stream is not supported for snapshot request frameNumber %d", frameNumber);
            }
        }

        PipelineData* pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_realtime, 0));
        UINT          requestIdIndex = (pPipelineData->seqId % MaxOutstandingRequests);

        pPipelineData->seqIdToFrameNum[requestIdIndex] = frameNumber;

        // for FS2 no RDI and FD streams during preview
        if (StreamConfigModeFastShutter != ExtensionModule::GetInstance()->GetOpMode(m_pUsecase->GetCameraId()))
        {

            CHX_LOG("Realtime AppFrameNum to ReqId: %d <--> %d", frameNumber, pPipelineData->seqId);

            result = m_pUsecase->GetOutputBufferFromRDIQueue(pPipelineData->seqId, m_rdiStreamIndex, &previewBuffers[previewCount]);

            if (CDKResultSuccess == result)
            {
                previewCount++;
            }
            else
            {
                CHX_LOG_ERROR("FeatureMFNR: GetOutputBufferFromRDIQueue failed for frameNumber %d", frameNumber);
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
        }

        const Session* pSession = m_pUsecase->GetSessionData(m_realtime)->pSession;

        // get metadata buffers
        pChiInputMetadata  = m_pMetadataManager->GetInput(pRequest->settings, pRequest->frame_number);
        pChiOutputMetadata = m_pMetadataManager->Get(pPipelineData->pPipeline->GetMetadataClientId(),
                                                     pPipelineData->seqId + 1);

        if (NULL == pChiInputMetadata)
        {
            CHX_LOG_ERROR("FeatureMFNR: ERROR NULL Input Metadata");
            m_pMetadataManager->PrintAllBuffers();
            result = CDKResultEResource;
        }
        else if (NULL == pChiOutputMetadata)
        {
            CHX_LOG_INFO("FeatureMFNR: ERROR NULL Output Metadata ");
            m_pMetadataManager->PrintAllBuffers(pPipelineData->pPipeline->GetMetadataClientId());
            result = CDKResultEResource;
        }

        if ((CDKResultSuccess == result) && (0 < snapshotCount))
        {
            m_pApplicationInputMeta = pChiInputMetadata;
            m_pApplicationInputMeta->AddReference("mfnr_input");
        }

        //switch sensor setting for 3-exposure-hdr snapshot
        CHX_LOG_INFO("IHDR Snapshot enable %d", m_isIHDRSnapshotEnable);
        if ((TRUE  == m_isIHDRSnapshotEnable) && (TRUE == m_pUsecase->IsIHDRSnapshotNeeded()) && (CDKResultSuccess == result))
        {
            CHX_LOG_INFO("IHDR Snapshot triggered, write vendortag to sensor command");
            m_stateIHDRSnapshot      = IHDR_START;
            result = pChiInputMetadata->SetTag("com.qti.ihdr_control", "state_ihdr_snapshot", &m_stateIHDRSnapshot, 1);
            CHX_ASSERT(CamxResultSuccess == result);
        }

        CHICAPTUREREQUEST captureRequest = { 0 };

        captureRequest.frameNumber            = pPipelineData->seqId++;
        captureRequest.hPipelineHandle        = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
        captureRequest.numOutputs             = previewCount;
        captureRequest.pOutputBuffers         = previewBuffers;
        captureRequest.pInputMetadata         = pChiInputMetadata ? pChiInputMetadata->GetHandle() : NULL;
        captureRequest.pOutputMetadata        = pChiOutputMetadata ? pChiOutputMetadata->GetHandle() : NULL;
        captureRequest.pPrivData              = &m_privData[captureRequest.frameNumber % MaxOutstandingRequests];
        captureRequest.pPrivData->featureType = FeatureType::MFNR;

        CHIPIPELINEREQUEST submitRequest      = { 0 };
        submitRequest.pSessionHandle          = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
        submitRequest.numRequests             = 1;
        submitRequest.pCaptureRequests        = &captureRequest;

        m_pUsecase->SetRequestToFeatureMapping(m_realtime, captureRequest.frameNumber, this);

        if (FALSE == pSession->IsPipelineActive())
        {
            if (ExtensionModule::GetInstance()->GetNumPCRsBeforeStreamOn(const_cast<camera_metadata*>(
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

        m_pUsecase->LogFeatureRequestMappings(frameNumber, captureRequest.frameNumber, "MFNR Request");
        result = m_pUsecase->SubmitRequest(&submitRequest);

        if ((CDKResultSuccess == result) && (0 < snapshotCount) &&
            ((TRUE == m_pUsecase->IsLLSNeeded() ||
            (StreamConfigModeFastShutter == ExtensionModule::GetInstance()->GetOpMode(m_pUsecase->GetCameraId())))
            || (TRUE == nonZSLMFNRShot)))
        {
            // set m_isLLSSnapshot to TRUE here, check this flag all through the code,
            // because m_pUsecase->IsLLSNeeded() might change during the snapshot.
            m_isLLSSnapshot = TRUE;

            CHX_LOG_INFO("FeatureMFNR Trigger LLS snapshot %p", m_pApplicationInputMeta);
            result = TriggerInternalLLSRequests(pRequest);
        }

        if ((CDKResultSuccess == result) && (0 < snapshotCount) &&
            (IHDR_START == m_stateIHDRSnapshot))
        {
            CHX_LOG_ERROR("FeatureMFNR Trigger IHDR snapshot %p", m_pApplicationInputMeta);
            result = TriggerInternalIHDRRequests(pRequest);
        }

        if ((CDKResultSuccess == result) && (0 < snapshotCount) && (FALSE == m_isLLSSnapshot))
        {
            m_allRDIResultsAvaliable = TRUE;
            m_allFDResultsAvaliable  = TRUE;
        }
    }
    else if (NULL != pPreviousFeature)
    {
        CHX_LOG("call from previous feature!");

        for (UINT32 i = 0; i < pRequest->num_output_buffers; i++)
        {
            if (m_pSnapshotStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream))
            {
                CHX_LOG("Snapshot Frame %d", pRequest->frame_number);
                m_requestFrameNumber = pRequest->frame_number;

                ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i],
                                                       &m_snapshotBuffers[snapshotReqIdIndex][snapshotCount]);
                snapshotCount++;
            }
        }

        CHX_LOG("snapshotReqIdIndex:%d, snapshotCount:%d", snapshotReqIdIndex, snapshotCount);
    }
    else
    {
        for (UINT32 i = 0; i < pRequest->num_output_buffers; i++)
        {
            if (m_pSnapshotStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream))
            {
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
        CHX_LOG_INFO("Snapshot in MFNR HW Multiframe %u", pRequest->frame_number);

        m_pOfflineRequestMutex->Lock();

        CHX_LOG_INFO("Snapshot in MFNR HW Multiframe %u after lock", pRequest->frame_number);

        m_isSnapshotFrame[frameIndex]                 = TRUE;
        m_snapshotBufferNum[snapshotReqIdIndex]       = snapshotCount;
        m_snapshotReqIdToFrameNum[snapshotReqIdIndex] = frameNumber;

        // Do Deep Copy if framework uses same request for future requests
        ChxUtils::DeepCopyCamera3CaptureRequest(pRequest, &m_captureRequest);

        //for multi camera usecase, always come in
        if ((FALSE == m_isLLSSnapshot) || (TRUE == m_pUsecase->IsMultiCameraUsecase()))
        {
            CHX_LOG("FeatureMFNR Non-LLS signal request thead");
            m_pOfflineRequestAvailable->Signal();
        }

        m_pOfflineRequestMutex->Unlock();


        if (TRUE == m_blockPreviewForSnapshot)
        {
            CHX_LOG("FeatureMNFR wait for snapshot jpeg done.");

            m_pSnapshotResultMutex->Lock();

            while (FALSE == m_snapshotResultAvailable)
            {
                m_pSnapshotResultAvailable->Wait(m_pSnapshotResultMutex->GetNativeHandle());
            }
            m_snapshotResultAvailable = FALSE;

            m_pSnapshotResultMutex->Unlock();

            CHX_LOG("FeatureMFNR snapshot jpeg done");
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::TriggerInternalLLSRequests
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::TriggerInternalLLSRequests(
    camera3_capture_request_t*  pRequest)
{
    CDKResult result        = CDKResultSuccess;
    UINT   internalrequest  = 0;

    UINT32                   totalFramesRequired;
    UINT32                   internalRequestsNum;
    UINT32                   maxInternalRequestsNum;

    if (StreamConfigModeFastShutter != ExtensionModule::GetInstance()->GetOpMode(m_pUsecase->GetCameraId()))
    {
        // already sumbmit one request with preview + RDI, count that one also.
        totalFramesRequired    = m_mfnrTotalNumFrames;
        internalRequestsNum    = totalFramesRequired - 1;
        maxInternalRequestsNum = MfnrMaxInputRDIFrames - 1;
    }
    else
    {
        // in FS2 mode there is NO RDI request along with preview
        totalFramesRequired    = m_mfnrTotalNumFrames;
        internalRequestsNum    = totalFramesRequired;
        maxInternalRequestsNum = MfnrMaxInputRDIFrames;
    }

    CHX_LOG("FeatureMFNR TriggerInternalLLSRequests(), total required RDI frames:%d", totalFramesRequired);

    if (maxInternalRequestsNum < internalRequestsNum)
    {
        CHX_LOG_ERROR("invalide internalRequestsNum:%d", internalRequestsNum);
        result = CDKResultEInvalidArg;
    }

    if (CDKResultSuccess == result)
    {
        const Session* pSession       = m_pUsecase->GetSessionData(m_realtime)->pSession;
        PipelineData*  pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_realtime, 0));

        m_allRDIResultsAvaliable = FALSE;
        m_allFDResultsAvaliable  = FALSE;

        /// Submit request(s) to realtime pipeline
        for (UINT32 i = 0; i < internalRequestsNum; i++)
        {
            /// update seqId <-> appFrameNum mapping
            UINT reqIdIdx = (pPipelineData->seqId % MaxOutstandingRequests);
            pPipelineData->seqIdToFrameNum[reqIdIdx] = pRequest->frame_number;

            /// get RDI buffer
            CHISTREAMBUFFER outputBuffers[2] = { { 0 } };
            internalrequest                  = 0;

            result = m_pUsecase->GetOutputBufferFromRDIQueue(pPipelineData->seqId,
                                                             m_rdiStreamIndex,
                                                             &outputBuffers[internalrequest]);
            if (CDKResultSuccess == result)
            {
                internalrequest++;
            }
            else
            {
                CHX_LOG_ERROR("FeatureMFNR: GetOutputBufferFromRDIQueue failed for frameNumber %d", pRequest->frame_number);
            }

            // No Fd stream for FS2
            if (StreamConfigModeFastShutter != ExtensionModule::GetInstance()->GetOpMode(m_pUsecase->GetCameraId()))
            {
                result = m_pUsecase->GetOutputBufferFromFDQueue(pPipelineData->seqId,
                                                                m_fdStreamIndex,
                                                                &outputBuffers[internalrequest]);
                if (CDKResultSuccess == result)
                {
                    internalrequest++;
                }
                else
                {
                    CHX_LOG_ERROR("FeatureMFNR: GetOutputBufferFromFDQueue failed for frameNumber %d", pRequest->frame_number);
                }
            }

            // update metadata
            if (NULL == m_pApplicationInputMeta)
            {
                result = CDKResultEFailed;
                CHX_LOG_ERROR("Cannot get Application input metadata for %d", internalRequestsNum);
                break;
            }

            else
            {
                m_pApplicationInputMeta->AddReference("lls_requests");
            }
            ChiMetadata* pChiOutputMetadata = m_pMetadataManager->Get(pPipelineData->pPipeline->GetMetadataClientId(),
                                                                      pPipelineData->seqId + 1);
            if (NULL == pChiOutputMetadata)
            {
                result = CDKResultEFailed;
                CHX_LOG_ERROR("Cannot get output metadata for %d", internalRequestsNum);
                break;
            }

            if (CDKResultSuccess == result)
            {
                CHX_LOG("FeatureMFNR submit internal RDI requet AppFrameNum:%d <--> rtReqId:%d",
                    pRequest->frame_number, pPipelineData->seqId);

                CHICAPTUREREQUEST request      = { 0 };
                request.frameNumber            = pPipelineData->seqId++;
                request.hPipelineHandle        = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
                request.numOutputs             = internalrequest; // RDI and FD buffer
                request.pOutputBuffers         = outputBuffers;
                request.pInputMetadata         = m_pApplicationInputMeta->GetHandle();
                request.pOutputMetadata        = pChiOutputMetadata->GetHandle();
                request.pPrivData              = &m_privData[request.frameNumber % MaxOutstandingRequests];
                request.pPrivData->featureType = FeatureType::MFNR;

                CHIPIPELINEREQUEST submitRequest = { 0 };
                submitRequest.pSessionHandle     = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
                submitRequest.numRequests        = 1;
                submitRequest.pCaptureRequests   = &request;

                m_pUsecase->SetRequestToFeatureMapping(m_realtime, request.frameNumber, this);

                m_pUsecase->LogFeatureRequestMappings(pRequest->frame_number, request.frameNumber, "MFNR LLS Realtime request");
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

            m_triggerMFNRReprocess[lastReqId % MaxOutstandingRequests] = TRUE;

            CHX_LOG("lastReqId:%d, set m_triggerMFNRReprocess[%d] to TRUE", lastReqId, lastReqId % MaxOutstandingRequests);
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::TriggerInternalIHDRRequests
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::TriggerInternalIHDRRequests(
    camera3_capture_request_t*  pRequest)
{
    CDKResult result        = CDKResultSuccess;
    UINT   internalrequest  = 0;

    // already sumbmit one request with preview + RDI, count that one also.
    UINT32                   totalFramesRequired    = m_mfnrTotalNumFrames;
    UINT32                   internalRequestsNum    = totalFramesRequired + 2;
    const UINT32             maxInternalRequestsNum = MfnrMaxInputRDIFrames;

    CHX_LOG("FeatureMFNR TriggerInternalIHDRRequests(), total required RDI frames:%d", totalFramesRequired);
    // Flush binning mode RDI queue and release the buffers
    m_pUsecase->FlushRDIQueue(InvalidFrameNum, m_rdiStreamIndex);
    if (maxInternalRequestsNum < internalRequestsNum)
    {
        CHX_LOG_ERROR("invalide internalRequestsNum:%d", internalRequestsNum);
        result = CDKResultEInvalidArg;
    }

    if (CDKResultSuccess == result)
    {
        const Session* pSession       = m_pUsecase->GetSessionData(m_realtime)->pSession;
        PipelineData*  pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_realtime, 0));

        m_allRDIResultsAvaliable = FALSE;
        m_allFDResultsAvaliable  = FALSE;

        /// Submit request(s) to realtime pipeline
        for (UINT32 i = 0; i < internalRequestsNum; i++)
        {
            /// update seqId <-> appFrameNum mapping
            UINT reqIdIdx = (pPipelineData->seqId % MaxOutstandingRequests);
            pPipelineData->seqIdToFrameNum[reqIdIdx] = pRequest->frame_number;

            /// get RDI buffer
            CHISTREAMBUFFER outputBuffers[2] = { { 0 } };
            internalrequest                  = 0;

            result = m_pUsecase->GetOutputBufferFromRDIQueue(pPipelineData->seqId,
                                                             m_rdiStreamIndex,
                                                             &outputBuffers[internalrequest]);
            if (CDKResultSuccess == result)
            {
                internalrequest++;
            }
            else
            {
                CHX_LOG_ERROR("FeatureMFNR: GetOutputBufferFromRDIQueue failed for frameNumber %d", pRequest->frame_number);
            }

            result = m_pUsecase->GetOutputBufferFromFDQueue(pPipelineData->seqId,
                                                            m_fdStreamIndex,
                                                            &outputBuffers[internalrequest]);
            if (CDKResultSuccess == result)
            {
                internalrequest++;
            }
            else
            {
                CHX_LOG_ERROR("FeatureMFNR: GetOutputBufferFromFDQueue failed for frameNumber %d", pRequest->frame_number);
            }

            // update metadata
            ChiMetadata* pChiInputMetadata  = m_pMetadataManager->GetInput(pRequest->settings, pRequest->frame_number);
            ChiMetadata* pChiOutputMetadata = m_pMetadataManager->Get(pPipelineData->pPipeline->GetMetadataClientId(),
                                                                      pPipelineData->seqId + 1);
            if (NULL == pChiInputMetadata)
            {
                result = CDKResultEFailed;
                CHX_LOG_ERROR("Cannot get input metadata for %d", internalRequestsNum);
                break;
            }
            else if (NULL == pChiOutputMetadata)
            {
                result = CDKResultEFailed;
                CHX_LOG_ERROR("Cannot get output metadata for %d", internalRequestsNum);
                break;
            }

            if (internalRequestsNum - 1 <= i)
            {
                CHX_LOG_INFO("IHDR Snapshot triggered, write stop vendortag to sensor command");
                m_stateIHDRSnapshot     = IHDR_STOP;
                result = pChiInputMetadata->SetTag("com.qti.ihdr_control", "state_ihdr_snapshot", &m_stateIHDRSnapshot, 1);
                CHX_ASSERT(CamxResultSuccess == result);
            }
            else
            {
                CHX_LOG_INFO("IHDR Snapshot enabled");
                m_stateIHDRSnapshot     = IHDR_ENABLED;
                result = pChiInputMetadata->SetTag("com.qti.ihdr_control", "state_ihdr_snapshot", &m_stateIHDRSnapshot, 1);
                CHX_ASSERT(CamxResultSuccess == result);
            }

            if (NULL != m_pApplicationInputMeta)
            {
                m_pApplicationInputMeta->AddReference("lls_requests");
            }
            else
            {
                result = CDKResultEFailed;
                CHX_LOG_ERROR("Application input meta is NULL");
            }

            if (CDKResultSuccess == result)
            {
                CHX_LOG("FeatureMFNR submit internal RDI requet AppFrameNum:%d <--> rtReqId:%d",
                    pRequest->frame_number, pPipelineData->seqId);

                CHICAPTUREREQUEST request      = { 0 };
                request.frameNumber            = pPipelineData->seqId++;
                request.hPipelineHandle        = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
                request.numOutputs             = internalrequest; // RDI and FD buffer
                request.pOutputBuffers         = outputBuffers;
                request.pInputMetadata         = m_pApplicationInputMeta->GetHandle();
                request.pOutputMetadata        = pChiOutputMetadata->GetHandle();
                request.pPrivData              = &m_privData[request.frameNumber % MaxOutstandingRequests];
                request.pPrivData->featureType = FeatureType::MFNR;

                CHIPIPELINEREQUEST submitRequest = { 0 };
                submitRequest.pSessionHandle     = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
                submitRequest.numRequests        = 1;
                submitRequest.pCaptureRequests   = &request;

                m_pUsecase->SetRequestToFeatureMapping(m_realtime, request.frameNumber, this);

                m_pUsecase->LogFeatureRequestMappings(pRequest->frame_number, request.frameNumber, "MFNR HDR request");
                result = m_pUsecase->SubmitRequest(&submitRequest);

                if (CDKResultSuccess != result)
                {
                    CHX_LOG_ERROR("TriggerInternalHDRRequests SubmitRequest failed!");
                    break;
                }
            }
        }

        if (CDKResultSuccess == result)
        {
            UINT32 lastReqId = pPipelineData->seqId - 1;

            m_triggerMFNRReprocess[lastReqId % MaxOutstandingRequests] = TRUE;

            CHX_LOG("lastReqId:%d, set m_triggerMFNRReprocess[%d] to TRUE", lastReqId, lastReqId % MaxOutstandingRequests);
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::WaitForRDIResultsReady
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::WaitForRDIResultsReady()
{
    // TODO: Utilize WaitForBufferMetaReady API and get rid of m_pRDIResultMutex and m_pRDIResultAvailable
    CDKResult result = CDKResultSuccess;

    CHX_LOG("FeatureMFNR LLS wait for RDI buffers/metadatas ready");

    m_pRDIResultMutex->Lock();

    while (FALSE == m_allRDIResultsAvaliable)
    {
        m_pRDIResultAvailable->Wait(m_pRDIResultMutex->GetNativeHandle());
    }
    m_allRDIResultsAvaliable = FALSE;

    m_pRDIResultMutex->Unlock();

    CHX_LOG("FeatureMFNR LLS RDI buffers/metadatas are ready");

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::GetRequestInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::GetRequestInfo(
    camera3_capture_request_t*  pRequest,
    FeatureRequestInfo*         pOutputRequests,
    FeatureRequestType          requestType)
{
    CDKResult result = CDKResultSuccess;

    UINT32 snapshotReqIdIndex = (m_maxSnapshotReqId % MaxOutstandingRequests);

    for (UINT32 i = 0; i < pRequest->num_output_buffers; i++)
    {
        if (m_pSnapshotStream == reinterpret_cast<CHISTREAM*>(pRequest->output_buffers[i].stream))
        {
            CHX_LOG("Copy Snapshot buffer, appFrameNum %d", pRequest->frame_number);

            ChxUtils::PopulateHALToChiStreamBuffer(&pRequest->output_buffers[i],
                                                   &m_snapshotBuffers[snapshotReqIdIndex][0]);
         }
    }

    if ((FeatureRequestType::LLS  == requestType) ||
        (FeatureRequestType::IHDR == requestType) ||
        (TRUE == m_pUsecase->IsQuadCFAUsecase()))
    {
        UINT32 totalFramesRequired = m_mfnrTotalNumFrames;

        CHX_LOG("FeatureMFNR GetRequestInfo(), total required RDI frames:%d", totalFramesRequired);

        if (MfnrMaxInputRDIFrames < totalFramesRequired)
        {
            CHX_LOG_ERROR("invalide totalFramesRequired:%d", totalFramesRequired);
            result = CDKResultEInvalidArg;
        }

        if ((CDKResultSuccess == result) && (NULL != pOutputRequests))
        {
            pOutputRequests->numOfRequest      = totalFramesRequired;

            for (UINT32 i = 0; i < totalFramesRequired; i++)
            {
                // return result at the last internal request
                if ((totalFramesRequired - 1) == i)
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
                pOutputRequests->metadataInfo[i].pInputMetadata     = NULL;
                pOutputRequests->metadataInfo[i].pOutputMetadata    = NULL;
            }

            m_isLLSSnapshot  = (FeatureRequestType::LLS  == requestType)? TRUE: FALSE;
            m_isIHDRSnapshot = (FeatureRequestType::IHDR == requestType)? TRUE: FALSE;

            CHX_LOG("total input RDI frame required:%d, isReturnResult[%d]:%d",
                totalFramesRequired,
                (totalFramesRequired - 1),
                pOutputRequests->isReturnResult[totalFramesRequired - 1]);
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::FetchVendorTagsForAnchorFrameSelection
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::FetchVendorTagsForAnchorFrameSelection(
    UINT32* pVendorTagLocationFocusValue,
    UINT32* pVendorTagLocationBuckets,
    UINT32* pVendorTagLocationStats)
{
    CDKResult  result       = CDKResultSuccess;
    CHITAGSOPS vendorTagOps = { 0 };

    CHX_ASSERT(NULL != pVendorTagLocationFocusValue);
    CHX_ASSERT(NULL != pVendorTagLocationBuckets);
    CHX_ASSERT(NULL != pVendorTagLocationStats);

    *pVendorTagLocationFocusValue = 0;
    *pVendorTagLocationBuckets    = 0;
    *pVendorTagLocationStats      = 0;

    ExtensionModule::GetInstance()->GetVendorTagOps(&vendorTagOps);

    result = vendorTagOps.pQueryVendorTagLocation(
                 "org.quic.camera.focusvalue",
                 "FocusValue",
                 pVendorTagLocationFocusValue);
    if (CDKResultSuccess == result)
    {
        result = vendorTagOps.pQueryVendorTagLocation(
                     "org.codeaurora.qcamera3.histogram",
                     "buckets",
                     pVendorTagLocationBuckets);
        if (CDKResultSuccess == result)
        {
            result = vendorTagOps.pQueryVendorTagLocation(
                         "org.codeaurora.qcamera3.histogram",
                         "stats",
                         pVendorTagLocationStats);
            if (CDKResultSuccess != result)
            {
                CHX_LOG_ERROR("Finding vendor tag location for Stats failed");
            }
        }
        else
        {
            CHX_LOG_ERROR("Finding vendor tag location for buckets failed");
        }
    }
    else
    {
        CHX_LOG_ERROR("Finding vendor tag location for FocusValue failed");
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::PopulateAnchorFrameSelectionDataFromVTags
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::PopulateAnchorFrameSelectionDataFromVTags(
    AnchorFrameSelectionData* pAnchorFrameSelectionData,
    MFNRInputInfo*                 pMfnrInputInfo)
{
    CDKResult result     = CDKResultSuccess;

    m_numOfImagesCaptured = 0;

    CHX_ASSERT(NULL != pAnchorFrameSelectionData);

    UINT32 zslQueueIndex = 0;

    for (UINT32 index = 0; index < pMfnrInputInfo->numOfBuffers; index++)
    {
        ChiMetadata* pChiMetadata = pMfnrInputInfo->pChiMetadata[index];

        FLOAT* pFocusValue  = static_cast<FLOAT*>(pChiMetadata->GetTag("org.quic.camera.focusvalue", "FocusValue"));
        INT32* pBucketValue = static_cast<INT32*>(pChiMetadata->GetTag("org.codeaurora.qcamera3.histogram", "buckets"));

        pAnchorFrameSelectionData->focusValue[index] = pFocusValue ? *pFocusValue : 0.f;
        pAnchorFrameSelectionData->timestamps[index] =
            m_pUsecase->GetRequestShutterTimestamp(pMfnrInputInfo->realtimeFrameNum[index]);

        HDRBHistStatsOutput* pStatsOut = static_cast<HDRBHistStatsOutput*>(
           pChiMetadata->GetTag("org.codeaurora.qcamera3.histogram", "stats"));

        if ((NULL != pBucketValue) && (NULL != pStatsOut) &&
            ((MaxBHistBinNum  == *pBucketValue) || (NumHDRBHistBins == *pBucketValue)))
        {
            /// @todo (CAMX-4025) - Need to change this block for binning 1024 into 256
            pAnchorFrameSelectionData->histogram[index] = pStatsOut->greenHistogram;
        }
        else
        {
            pAnchorFrameSelectionData->histogram[index] = NULL;
            CHX_LOG_ERROR("Cannot set histogram stats/bucket %p/%p", pBucketValue, pStatsOut);
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::CalculateBrightness
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 FeatureMFNR::CalculateBrightness(
    UINT32*  histogram,
    UINT     minHistrogramBin,
    UINT     maxHistrogramBin)
{
    UINT32 counter = 0;
    UINT32 medianCounter = 0;
    UINT32 resultBrightness = maxHistrogramBin + 1;

    // Calculate the sum of the histogram's relevant bins
    for (UINT i = minHistrogramBin; i <= maxHistrogramBin; i++)
    {
        medianCounter += histogram[i];
    }

    // Calculate median brightness: accumulate until half the sum
    medianCounter /= 2;

    for (UINT i = minHistrogramBin; i <= maxHistrogramBin; i++)
    {
        counter += histogram[i];

        if (counter >= medianCounter)
        {
            resultBrightness = i;
            break;
        }
    }
    return resultBrightness;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::CalculateSharpness
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FLOAT FeatureMFNR::CalculateSharpness(
    CHISTREAMBUFFER*    pInputFDBuffer,
    UINT32              sharpnessBlockSize,
    float               sharpnessRankValue)
{
    CDKResult   result              = CDKResultSuccess;
    VOID*       pHostptr            = NULL;
    INT         bufferLength        = (pInputFDBuffer->pStream->width * pInputFDBuffer->pStream->height * 3) / 2;
    FLOAT       sharpnessresult     = 0.0f;
    UINT32      numOfBlocksX        = (m_pFdStream->width - 1) / sharpnessBlockSize;
    UINT32      numOfBlocksY        = (m_pFdStream->height - 1) / sharpnessBlockSize;
    UINT32      numOfBlocks         = numOfBlocksX * numOfBlocksY;
    UINT32      downscaledStride    = pInputFDBuffer->pStream->width;

    pHostptr = CHIBufferManager::GetCPUAddress(&pInputFDBuffer->bufferInfo, bufferLength);

    CHX_LOG("Type=%d, phBuffer=%p, pHostptr=%p",
            pInputFDBuffer->bufferInfo.bufferType, pInputFDBuffer->bufferInfo.phBuffer, pHostptr);

    if ((CDKResultSuccess == result) && (pHostptr != NULL))
    {
        const UINT8* pImageLine = reinterpret_cast<const UINT8*>(pHostptr) +
            (m_pFdStream->height - numOfBlocksY * sharpnessBlockSize) / 2 * downscaledStride +
            (m_pFdStream->width - numOfBlocksX * sharpnessBlockSize) / 8 * 4;
        UINT16* blockSharpnessBuffer = static_cast<UINT16*>(CHX_CALLOC(numOfBlocks * sizeof(UINT16)));

        if (NULL != blockSharpnessBuffer)
        {
            UINT16* pSharpnessLine = blockSharpnessBuffer;

            for (UINT32 blockCounterY = 0; blockCounterY < numOfBlocksY; ++blockCounterY)
            {
                for (UINT32 pixelCounterY = 0; pixelCounterY < sharpnessBlockSize; ++pixelCounterY)
                {
                    const UINT8* pImage = pImageLine;
                    UINT16* pSharpness = pSharpnessLine;
                    for (UINT32 blockCounterX = 0; blockCounterX < numOfBlocksX; ++blockCounterX)
                    {
                        UINT32 sharpness = *pSharpness;
                        for (UINT32 pixelCounterX = 0; pixelCounterX < sharpnessBlockSize; ++pixelCounterX)
                        {
                            UINT8 derivX = abs(pImage[1] - pImage[0]);
                            UINT8 derivY = abs(pImage[downscaledStride] - pImage[0]);
                            sharpness += derivX;
                            sharpness += derivY;
                            ++pImage;
                        }
                        *pSharpness++ = static_cast<UINT16>(std::min<UINT32>(sharpness, 0xffff));
                    }
                    pImageLine += downscaledStride;
                }
                pSharpnessLine += numOfBlocksX;
            }

            UINT32 index = UINT32(numOfBlocks * sharpnessRankValue);
            std::nth_element(blockSharpnessBuffer, blockSharpnessBuffer + index, blockSharpnessBuffer + numOfBlocks);
            sharpnessresult = blockSharpnessBuffer[index];
            sharpnessresult /= sharpnessBlockSize * sharpnessBlockSize * 2;

            CHX_FREE(blockSharpnessBuffer);
        }
        else
        {
            CHX_LOG_ERROR("Calloc for blockSharpnessBuffer failed!");
            result = CDKResultENoMemory;
        }

        CHIBufferManager::PutCPUAddress(&pInputFDBuffer->bufferInfo, bufferLength, pHostptr);
    }
    else
    {
        CHX_LOG_ERROR("Invalid ChiStreamBuffer Type=%d, phBuffer=%p, pHostptr=%p",
                      pInputFDBuffer->bufferInfo.bufferType, pInputFDBuffer->bufferInfo.phBuffer, pHostptr);
    }

    return sharpnessresult;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::CreateMFNRInputInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::CreateMFNRInputInfo(
    MFNRInputInfo*             pInputInfo,
    camera3_capture_request_t* pRequest,
    SnapshotFeatureInfo*       pFeatureInfo)
{
    CDKResult result = CDKResultSuccess;

    if ((NULL != pFeatureInfo) && (0 < pFeatureInfo->featureInput.numOfInputBuffers))
    {
        pInputInfo->numOfBuffers = pFeatureInfo->featureInput.numOfInputBuffers;
    }
    else
    {
        pInputInfo->numOfBuffers = m_mfnrTotalNumFrames;
    }

    if (FALSE == m_pUsecase->IsMultiCameraUsecase())
    {
        UINT32             targetBufferIndex   = 0;
        TargetBuffer*      pTargetBuffer       = NULL;
        UINT32             internalFrameNumber = 0;
        UINT32             zslQueueIndex       = 0;
        UINT32             inputFrameNumber    = 0;
        FeatureRequestType requestType         = FeatureRequestType::COMMON;

        if (TRUE == m_isLLSSnapshot)
        {
            requestType = FeatureRequestType::LLS;
        }
        else if(TRUE == IsIHDRSnapshotEnabled())
        {
            requestType = FeatureRequestType::IHDR;
        }

        if (NULL == pFeatureInfo)
        {
            targetBufferIndex = m_rdiStreamIndex;
            pTargetBuffer     = m_pRdiTargetBuffer;
        }
        else
        {
            targetBufferIndex = pFeatureInfo->featureInput.inputBufferQIdx;
            pTargetBuffer     = m_pUsecase->GetTargetBufferPointer(pFeatureInfo->featureInput.inputBufferQIdx);

            m_sensorModeIndex = pFeatureInfo->featureInput.sensorModeIndex;
            CHX_LOG("m_sensorModeIndex:%d", m_sensorModeIndex);
        }

        pInputInfo->targetBufferQIdx = targetBufferIndex;

        internalFrameNumber = pTargetBuffer->lastReadySequenceID;

        // Get Snapshot buffers from capture request output buffers
        GetRequestInfo(pRequest, NULL, requestType);

        CHX_LOG("lastReadyFramenumber:%d",internalFrameNumber);
        CHX_ASSERT(INVALIDSEQUENCEID != internalFrameNumber);

        /// @todo (CAMX-4025): pTargetBuffer (struct TargetBuffer) does not have a member
        ///                    indicating the number of buffers captured/available. This isn't
        ///                    the same as the BufferQueueDepth or the queueDepth(e.g., is set
        ///                    CameraUsecaseBase::AddNewInternalBufferQueue() for reference).
        ///                    OR is it assumed that the m_pRdiTargetBuffer, will have all the
        ///                    BufferQueueDepth buffers already availale before reaching this
        ///                    point?

        // Get the input buffer from ZSL queue, traversing in reverse order
        // starting with the most recent RDI buffer sequence id available

        // Make sure the most recent RDI buffer is ready and others are implicitly assumed ready as well
        result = m_pUsecase->WaitForBufferMetaReady(internalFrameNumber, targetBufferIndex);

        for (UINT32 i = 0; i < pInputInfo->numOfBuffers; i++)
        {
            inputFrameNumber = (i > internalFrameNumber) ? (0) : (internalFrameNumber - i);
            if (CDKResultSuccess == result)
            {
                m_pUsecase->GetTargetBuffer(inputFrameNumber,
                                            pTargetBuffer,
                                            NULL,
                                            &(pInputInfo->bufferArray[i]),
                                            &(pInputInfo->pChiMetadata[i]));

                if (NULL == pInputInfo->pChiMetadata[i])
                {
                    if (0 != i)
                    {
                        for (UINT j = i; j < m_mfnrTotalNumFrames; ++j)
                        {
                            pInputInfo->bufferArray[j]   = pInputInfo->bufferArray[i-1];
                            pInputInfo->pChiMetadata[j]  = pInputInfo->pChiMetadata[i-1];
                            pInputInfo->fdbufferArray[j] = pInputInfo->fdbufferArray[i-1];

                            pInputInfo->realtimeFrameNum[i] = inputFrameNumber;
                        }
                        CHX_LOG_INFO("Cannot obtain metadata for frame :%d duplicate from %d -> %d",
                                      inputFrameNumber, i, m_mfnrTotalNumFrames);
                    }
                    else
                    {
                        CHX_LOG_ERROR("ERROR: Cannot obtain metadata for frame :%d",
                                      inputFrameNumber);
                    }
                    break;
                }

                DumpMetadata(pInputInfo->pChiMetadata[i], GetUniqueStageName(i, "mfnr_proc_init_"));

                m_pUsecase->GetTargetBuffer(inputFrameNumber,
                                            m_pFdTargetBuffer,
                                            NULL,
                                            &(pInputInfo->fdbufferArray[i]));

                pInputInfo->realtimeFrameNum[i] = inputFrameNumber;

                // Dynamically decrementing ValidBufferLength by one during MFNR so less RDI/FD buffers will be reserved in the queue.
                // It will be increased once the input RDIs are released after each stage.
                m_pUsecase->UpdateValidRDIBufferLength(targetBufferIndex, -1);
                m_pUsecase->UpdateValidFDBufferLength(m_fdStreamIndex, -1);
            }

            if (CDKResultSuccess != result)
            {
                CHX_LOG_ERROR("FeatureMFNR: wait rdi and meta timeout! frameNumber=%d, InputFrameNumber=%d",
                              pRequest->frame_number, inputFrameNumber);

                // clean up in error case
                for (UINT32 frameIndex = 0; frameIndex < m_mfnrTotalNumFrames; frameIndex++)
                {
                    if (NULL != pInputInfo->pChiMetadata[frameIndex])
                    {
                        pInputInfo->pChiMetadata[frameIndex]  = NULL;
                    }
                }
                break;
            }
        }

        if (CDKResultSuccess == result)
        {
            // Flush the RDI/FD queue as all the buffers of frameNumber older than 'inputFrameNumber' won't be needed anymore
            m_pUsecase->FlushRDIQueue(inputFrameNumber, targetBufferIndex);
            m_pUsecase->FlushFDQueue(inputFrameNumber, m_fdStreamIndex);
        }
    }
    else
    {
        UINT32 internalFrameNumber = pRequest->frame_number;
        RequestMapInfo requestInfo = m_pUsecase->GetRequestMapInfo(internalFrameNumber);
        m_activeCameraId           = requestInfo.masterCameraID;
        m_activePipelineIndex      = requestInfo.activePipelineID;

        CHX_LOG("lastReadyFramenumber:%d",internalFrameNumber);

        for (UINT32 frameNumber = 0, i = 0; i < m_mfnrTotalNumFrames; i++)
        {
            pInputInfo->pChiMetadata[i] = NULL;

            m_pUsecase->GetInputBufferFromRDIQueue(internalFrameNumber,
                                                   m_activePipelineIndex,
                                                   i,
                                                   &(pInputInfo->bufferArray[i]),
                                                   &(pInputInfo->pChiMetadata[i]),
                                                   FALSE);

            if (NULL == pInputInfo->pChiMetadata[i])
            {
                result = CDKResultEFailed;
                CHX_LOG_ERROR("ERROR obtaining metadata for frame :%d m_activePipelineIndex = %d i = %d",
                    internalFrameNumber, m_activePipelineIndex, i);
                break;
            }

            m_pUsecase->GetInputBufferFromFDQueue(internalFrameNumber,
                                                  m_activePipelineIndex,
                                                  i,
                                                  &(pInputInfo->fdbufferArray[i]),
                                                  FALSE);

            pInputInfo->realtimeFrameNum[i] = internalFrameNumber;
        }
    }

    if (CDKResultSuccess == result)
    {
        m_metadataIndex = 0; // reset metadata index
        PrepareMFNRInputMetadata(pInputInfo);
    }

    if (CDKResultSuccess == result)
    {
        // dump MFNR input buffer/meta info
        for (UINT32 i = 0; i < pInputInfo->numOfBuffers; ++i)
        {
            CHX_LOG("[%d/%d] stream:%p, buffer type %d handle:%p, meta:%p",
                    i,
                    pInputInfo->numOfBuffers,
                    pInputInfo->bufferArray[i].pStream,
                    pInputInfo->bufferArray[i].bufferInfo.bufferType,
                    pInputInfo->bufferArray[i].bufferInfo.phBuffer,
                    pInputInfo->pChiMetadata[i]);
            if (NULL == pInputInfo->bufferArray[i].bufferInfo.phBuffer)
            {
                CHX_LOG_ERROR(" FATAL RDI Buffer is NULL! Dont start Offline Processing!");
                result = CDKResultEFailed;
            }
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::PrepareMFNRInputMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::PrepareMFNRInputMetadata(
        MFNRInputInfo*             pInputInfo)
{
    PipelineData* pPipelineData;
    MFNRStage     pipelineStage;
    CDKResult     result                        = CDKResultSuccess;
    UINT32        refCount                      = m_mfnrTotalNumFrames;
    UINT32        noiseReprocessIndex           = MfnrMaxMetadata;
    UINT32        snapshotIndex                 = MfnrMaxMetadata;
    UINT32        offlineMetadataIndex          = 0;
    UINT32        noiseRerocessMetadataIndex    = 0;

    if (TRUE == m_noiseReprocessEnable)
    {
        noiseReprocessIndex = refCount;
        refCount++;
    }

    if (TRUE == IsJPEGSnapshotConfigured())
    {
        snapshotIndex = refCount;
        refCount++;
    }

    if (refCount > MfnrMaxMetadata)
    {
        CHX_LOG_ERROR("MFNR refCount is too big, refCount = %d, MaxCount = %d",
                refCount, MfnrMaxMetadata);
        result = CDKResultEFailed;
    }

    if (CDKResultSuccess == result)
    {
        for (UINT32 index = 0; index < refCount; ++index)
        {
            if (index < m_mfnrTotalNumFrames)
            {
                pPipelineData                       =
                    const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_offline, GetPipelineIndex(GetPipelineStage(index))));

                m_pInterStageOutputMetadata[index]  =
                    m_pMetadataManager->Get(pPipelineData->pPipeline->GetMetadataClientId(), offlineMetadataIndex++);

            }
            else if (index == noiseReprocessIndex)
            {
                pPipelineData = const_cast<PipelineData*>(
                        m_pUsecase->GetPipelineData(m_offlineNoiseReprocess, 0));
                m_pInterStageOutputMetadata[index] =
                    m_pMetadataManager->Get(pPipelineData->pPipeline->GetMetadataClientId(), noiseReprocessIndex++);
            }
            else if (index == snapshotIndex)
            {
                pPipelineData = const_cast<PipelineData*>(
                        m_pUsecase->GetPipelineData(m_offline, GetPipelineIndex(MfnrStageSnapshot)));
                m_pInterStageOutputMetadata[index] =
                    m_pMetadataManager->Get(pPipelineData->pPipeline->GetMetadataClientId(), offlineMetadataIndex++);

            }
            else
            {
                CHX_LOG_ERROR("ERROR: can't get pPipelineData!");
                result = CDKResultEFailed;
                break;
            }

            if (NULL != m_pInterStageOutputMetadata[index])
            {
                CHX_LOG_INFO("Start MFNR metadata refcount for index %d is %u %p", index,
                        m_pInterStageInputMetadata[index]->ReferenceCount(),
                        m_pInterStageInputMetadata[index]->GetHandle());

                if (index < m_mfnrTotalNumFrames)
                {
                    pInputInfo->pChiMetadata[index]->AddReference();
                }

                // update refcount for the input and output metadata
                m_pInterStageInputMetadata[index]->AddReference(GetUniqueStageName(index));
                if (NULL != m_pApplicationInputMeta)
                {
                    // merge the metadata with input
                    m_pInterStageInputMetadata[index]->Merge(*(m_pApplicationInputMeta));
                }
            }
            else
            {
                result = CDKResultEFailed;
                CHX_LOG_ERROR("ERROR obtaining metadata for client %x frame :%d",
                        pPipelineData->pPipeline->GetMetadataClientId(), index);
                break;
            }
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SelectFixedFrameOrder
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::SelectFixedFrameOrder(
    UINT* pFrameNumber,
    UINT  totalNumberOfFrames)
{
    CDKResult result = CDKResultSuccess;

    CHX_ASSERT(NULL != pFrameNumber);
    if (totalNumberOfFrames > BufferQueueDepth)
    {
        CHX_LOG_ERROR("totalNumberOfFrames should not bigger than %d", BufferQueueDepth);
        result = CDKResultEFailed;
    }

    if (CDKResultSuccess == result)
    {
        for (UINT i = 0; i < totalNumberOfFrames; ++i)
        {
            pFrameNumber[i] = i;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::PerformAnchorImagePicking
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::PerformAnchorImagePicking(
    MFNRInputInfo*                 pMfnrInputInfo,
    AnchorFrameSelectionData*      pAnchorFrameSelectionData,
    UINT*                          pFrameOrder)
{
    CDKResult result = CDKResultSuccess;
    float sharpness[BufferQueueDepth] = { 0 };

    CHX_ASSERT(NULL != pMfnrInputInfo);
    CHX_ASSERT(NULL != pAnchorFrameSelectionData);

    // Set initial processing order
    for (UINT32 i = 0; i < pMfnrInputInfo->numOfBuffers; i++)
    {
        pFrameOrder[i] = i;
    }

    if ((pAnchorFrameSelectionData->anchorFrameSelectionMode == AnchorFrameSelectionMode::Sharpness) ||
        (pAnchorFrameSelectionData->anchorFrameSelectionMode == AnchorFrameSelectionMode::Lighting))
    {
        for (UINT32 i = 0; i < pAnchorFrameSelectionData->numSharpnessImages; i++)
        {
            sharpness[i] = CalculateSharpness(&(pMfnrInputInfo->fdbufferArray[i]),
                pAnchorFrameSelectionData->sharpnessBlockSize,
                pAnchorFrameSelectionData->sharpnessRankValue);
        }
    }

    switch (pAnchorFrameSelectionData->anchorFrameSelectionMode)
    {
        case AnchorFrameSelectionMode::Sharpness:
        {
            // Choose the sharpest image
            int anchorIndex = 0;
            for (UINT i = 0; i < pMfnrInputInfo->numOfBuffers; ++i)
            {
                if (IsCandidateForAnchor(
                    pAnchorFrameSelectionData->numImagesAllowedAsAnchor, i)
                    && sharpness[i] > sharpness[anchorIndex])
                {
                    anchorIndex = i;
                }
            }
            pFrameOrder[0]           = anchorIndex;
            pFrameOrder[anchorIndex] = 0;

            // Sort the remaining images by sharpness
            auto compare = [&](int i1, int i2) {
                bool cand1 = IsCandidateForAnchor(
                    pAnchorFrameSelectionData->numImagesAllowedAsAnchor, i1);
                bool cand2 = IsCandidateForAnchor(
                    pAnchorFrameSelectionData->numImagesAllowedAsAnchor, i2);
                if (cand1 != cand2)
                    return cand1;
                else
                    return sharpness[i1] > sharpness[i2];
            };
            std::sort(pFrameOrder + 1,
                pFrameOrder + pAnchorFrameSelectionData->numOfImagesToBlend, compare);
        }
        break;

        case AnchorFrameSelectionMode::Lighting:
        {
            int brightness[BufferQueueDepth];
            int blendingPotential[BufferQueueDepth] = { 0 };

            // Calculate brightness
            for (UINT i = 0; i < pMfnrInputInfo->numOfBuffers; ++i)
            {
                brightness[i] = CalculateBrightness(pAnchorFrameSelectionData->histogram[i],
                    pAnchorFrameSelectionData->minHistrogramBin,
                    pAnchorFrameSelectionData->maxHistrogramBin);
            }

            // Calculate blending potential
            for (UINT i1 = 0; i1 < pMfnrInputInfo->numOfBuffers; ++i1)
            {
                for (UINT i2 = 0; i2 < i1; ++i2)
                {
                    if (abs((int)brightness[i1] - (int)brightness[i2]) <
                        (int)pAnchorFrameSelectionData->brightnessTolerance)
                    {
                        ++blendingPotential[i1];
                        ++blendingPotential[i2];
                    }
                }
            }

            // Choose the sharpest image that has maximum blending potential
            int anchorIndex = 0;
            for (UINT i = 0; i < pMfnrInputInfo->numOfBuffers; ++i)
            {
                if (IsCandidateForAnchor(pAnchorFrameSelectionData->numImagesAllowedAsAnchor, i))
                {
                    bool isBetter = blendingPotential[i] > blendingPotential[anchorIndex];

                    if (blendingPotential[i] == blendingPotential[anchorIndex])
                    {
                        isBetter = sharpness[i] > sharpness[anchorIndex];
                    }

                    if (isBetter)
                    {
                        anchorIndex = i;
                    }
                }
            }
            pFrameOrder[0] = anchorIndex;
            pFrameOrder[anchorIndex] = 0;

            // Sort such that potentially blended images are first; otherwise, by sharpness
            auto compare = [&](int i1, int i2) {
                bool blended1 = abs(brightness[anchorIndex] - brightness[i1]) <
                    (int)pAnchorFrameSelectionData->brightnessTolerance;
                bool blended2 = abs(brightness[anchorIndex] - brightness[i2]) <
                    (int)pAnchorFrameSelectionData->brightnessTolerance;
                if (blended1 != blended2)
                {
                    return blended1;
                }
                else
                {
                    bool cand1 = IsCandidateForAnchor(
                        pAnchorFrameSelectionData->numImagesAllowedAsAnchor, i1);
                    bool cand2 = IsCandidateForAnchor(
                        pAnchorFrameSelectionData->numImagesAllowedAsAnchor, i2);
                    if (cand1 != cand2)
                        return cand1;
                    else
                        return sharpness[i1] > sharpness[i2];
                }
            };
            std::sort(pFrameOrder + 1,
                pFrameOrder + pAnchorFrameSelectionData->numOfImagesToBlend, compare);
        }
        break;

        case AnchorFrameSelectionMode::TimeStamp:
        default:
        {
            // Sort the other images by timestamp closeness
            auto compare = [&](int i1, int i2) {
                bool cand1 = IsCandidateForAnchor(
                    pAnchorFrameSelectionData->numImagesAllowedAsAnchor, i1);
                bool cand2 = IsCandidateForAnchor(
                    pAnchorFrameSelectionData->numImagesAllowedAsAnchor, i2);
                if (cand1 != cand2)
                    return cand1;
                else
                    return i1 < i2;
            };
            std::sort(pFrameOrder + 1,
                pFrameOrder + pAnchorFrameSelectionData->numOfImagesToBlend, compare);
        }
        break;
    }

    // Final order after Running Algo
    for (UINT32 i = 0; i < pMfnrInputInfo->numOfBuffers; i++)
    {
        CHX_LOG("RDI processing order:%d", pFrameOrder[i]);
    }

    return result;

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::GetRequiredFramesForSnapshot
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 FeatureMFNR::GetRequiredFramesForSnapshot(
        const camera_metadata* pMetadata)
{
    (VOID)pMetadata;

    // CalculateMFNRTotalFramesByGain should be call here for get latest required snapshot frame
    CalculateMFNRTotalFramesByGain();

    return m_mfnrTotalNumFrames;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::IsCandidateForAnchor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL FeatureMFNR::IsCandidateForAnchor(UINT32 numImagesAllowedAsAnchor, UINT32 imageIndex)
{
    return imageIndex < numImagesAllowedAsAnchor;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SelectNonFixedFrameOrder
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::SelectNonFixedFrameOrder(
    MFNRInputInfo*   pMfnrInputInfo,
    UINT* pFrameNumber)
{
    CDKResult result = CDKResultSuccess;

    CHX_ASSERT(NULL != pFrameNumber);
    CHX_ASSERT(NULL != pMfnrInputInfo);

    // While populating the anchor frame selection data
    // - m_numOfImagesCaptured: will be set, and
    // - m_processingOrder[0] : will have anchor frame index

    m_numOfImagesCaptured = 0;

    AnchorFrameSelectionData anchorFrameSelectionData = { { 0 } };

    result = PopulateAnchorFrameSelectionDataFromVTags(&anchorFrameSelectionData, pMfnrInputInfo);
    if (CDKResultSuccess != result)
    {
        CHX_LOG_WARN("Failed in populating vendor tag data into Anchor Frame selection data");
    }

    /// OEM's Can Choose Anchor selection Mode, by default we will use sharpness
    anchorFrameSelectionData.anchorFrameSelectionMode   = AnchorFrameSelectionMode::Sharpness;
    anchorFrameSelectionData.numImagesAllowedAsAnchor   = 3;
    anchorFrameSelectionData.minHistrogramBin           = 0;
    anchorFrameSelectionData.maxHistrogramBin           = 255;
    anchorFrameSelectionData.numOfImagesToBlend         = pMfnrInputInfo->numOfBuffers - 1;
    anchorFrameSelectionData.desiredAnchorFrameIndex    = pMfnrInputInfo->numOfBuffers - 1;
    anchorFrameSelectionData.anchorFrameTimeRange       = 200000000;
    anchorFrameSelectionData.brightnessTolerance        = 3;
    anchorFrameSelectionData.removeExpectedBadImages    = FALSE;
    anchorFrameSelectionData.numSharpnessImages         = 3;
    anchorFrameSelectionData.sharpnessBlockSize         = 16;
    anchorFrameSelectionData.sharpnessRankValue         = 0.9375;


    result = PerformAnchorImagePicking(pMfnrInputInfo, &anchorFrameSelectionData, pFrameNumber);

    if (CDKResultSuccess != result)
    {
        CHX_LOG_ERROR("Failed in populating anchor frame selection data");
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SelectMFNRAnchorFrameAndFrameOrder
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::SelectMFNRAnchorFrameAndFrameOrder(
    MFNRInputInfo*                 pMfnrInputInfo,              // MFNR Input Info.
    CHISTREAMBUFFER*               selectionInputBuffer,        // Out: MFNR Anchor Selection Buffer
    size_t                         selectionInputBufferSize,    // In:  # of MFNR Anchor Selection Buffer
    ChiMetadata**                  ppMergedMetadata,            // Out: Merged Metadata of the Anchor Frame
    AnchorFrameSelectionAlgorithm* anchorSelectionAlgorithm)    // Out: MFNR Anchor Selection Algorithm Option
{
    // Also uses member field m_processingOrder

    CDKResult result = CDKResultSuccess;

    CHX_ASSERT(NULL != pMfnrInputInfo);
    CHX_ASSERT(NULL != selectionInputBuffer);
    CHX_ASSERT(0    != selectionInputBufferSize);
    CHX_ASSERT(NULL != anchorSelectionAlgorithm);

    *anchorSelectionAlgorithm = static_cast<AnchorFrameSelectionAlgorithm>(
                                    ExtensionModule::GetInstance()->EnableMFNRAnchorSelectionAlgorithm());

    // Set default processing order
    for (UINT32 i = 0; i < pMfnrInputInfo->numOfBuffers; i++)
    {
        m_processingOrder[i] = i;
    }
    UINT32 zslInputFrameIndex = 0;

    switch (*anchorSelectionAlgorithm)
    {
        case AnchorFrameSelectionAlgorithm::None:
        {
            // anchor frame as input
            ChxUtils::Memcpy(&selectionInputBuffer[zslInputFrameIndex],
                             &(pMfnrInputInfo->bufferArray[zslInputFrameIndex]),
                             selectionInputBufferSize);

            break;
        }
        case AnchorFrameSelectionAlgorithm::Fixed:
        {
            ChxUtils::Memset(m_processingOrder, 0, sizeof(m_processingOrder));
            CHX_ASSERT(m_mfnrTotalNumFrames <= (sizeof(m_processingOrder) / sizeof(m_processingOrder[0])));

            result = SelectFixedFrameOrder(&m_processingOrder[0], m_mfnrTotalNumFrames);
            if (CDKResultSuccess != result)
            {
                CHX_LOG_ERROR("Failed in fixed anchor frame selection and frame order");
                result = CDKResultEFailed;
            }

            zslInputFrameIndex = m_processingOrder[0];

            // anchor frame as input
            ChxUtils::Memcpy(&selectionInputBuffer[0],
                             &(pMfnrInputInfo->bufferArray[zslInputFrameIndex]),
                             selectionInputBufferSize);

            break;
        }
        case AnchorFrameSelectionAlgorithm::NonFixed:
        {
            ChxUtils::Memset(m_processingOrder, 0, sizeof(m_processingOrder));
            CHX_ASSERT(m_mfnrTotalNumFrames <= (sizeof(m_processingOrder) / sizeof(m_processingOrder[0])));

            result = SelectNonFixedFrameOrder(pMfnrInputInfo, &m_processingOrder[0]);
            if (CDKResultSuccess != result)
            {
                CHX_LOG_ERROR("Failed in non-fixed anchor frame selection and frame order");
                result = CDKResultEFailed;
            }

            zslInputFrameIndex = m_processingOrder[0];

            // anchor frame as input from the ZSL queue and associated metadata (if requested)
            ChxUtils::Memcpy(&selectionInputBuffer[0],
                             &(pMfnrInputInfo->bufferArray[zslInputFrameIndex]),
                             selectionInputBufferSize);

            if (NULL != ppMergedMetadata)
            {
                *ppMergedMetadata = pMfnrInputInfo->pChiMetadata[zslInputFrameIndex];
            }

            break;
        }
        default: // Unknown/Invalid option
        {
            CHX_LOG_ERROR("Unknown/Invalid anchor frame selection and frame order option");
            result = CDKResultEFailed;

            break;
        }
    }

    if (CDKResultSuccess == result)
    {
        // Send infomration about Anchor frame index to Usecase.
        m_pUsecase->ProcessFeatureDataNotify(m_internalRequestId, this, static_cast<VOID*>(&zslInputFrameIndex));
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::ActivateOfflinePipeline
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::ActivateOfflinePipeline(
    MFNRStage pipelineStage)
{
    CDKResult        result           = CDKResultSuccess;
    ExtensionModule* pExtensionModule = ExtensionModule::GetInstance();
    const Session*   pOfflineSession  = m_pUsecase->GetSessionData(m_offline)->pSession;

    if (FALSE == pOfflineSession->IsPipelineActive(GetPipelineIndex(pipelineStage)))
    {
        CHX_LOG_INFO("Activate %s", MFNRStageNames[pipelineStage]);

        result = pExtensionModule->ActivatePipeline(pOfflineSession->GetSessionHandle(),
                                       pOfflineSession->GetPipelineHandle(GetPipelineIndex(pipelineStage)));
        if (CDKResultSuccess == result)
        {
            pOfflineSession->SetPipelineActivateFlag(GetPipelineIndex(pipelineStage));
        }
        else
        {
            CHX_LOG_ERROR("Activate %s failed!", MFNRStageNames[pipelineStage]);
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::DeactivateOfflinePipeline
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::DeactivateOfflinePipeline(
    MFNRStage pipelineStage)
{
    CDKResult        result           = CDKResultSuccess;
    ExtensionModule* pExtensionModule = ExtensionModule::GetInstance();
    const Session*   pOfflineSession  = m_pUsecase->GetSessionData(m_offline)->pSession;

    if (TRUE == pOfflineSession->IsPipelineActive(GetPipelineIndex(pipelineStage)))
    {
        CHX_LOG_INFO("Deactivate %s - DeactivateModeReleaseBuffer", MFNRStageNames[pipelineStage]);

        result = pExtensionModule->DeactivatePipeline(pOfflineSession->GetSessionHandle(),
                                       pOfflineSession->GetPipelineHandle(GetPipelineIndex(pipelineStage)),
                                       CHIDeactivateModeReleaseBuffer);
        if (CDKResultSuccess != result)
        {
            CHX_LOG_ERROR("Deactivate %s failed!", MFNRStageNames[pipelineStage]);
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::ActivateOfflineNoiseReprocessPipeline
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::ActivateOfflineNoiseReprocessPipeline()
{
    CDKResult        result                        = CDKResultSuccess;
    ExtensionModule* pExtensionModule              = ExtensionModule::GetInstance();
    const Session*   pOfflineNoiseReprocessSession = m_pUsecase->GetSessionData(m_offlineNoiseReprocess)->pSession;

    if (FALSE == pOfflineNoiseReprocessSession->IsPipelineActive())
    {
        CHX_LOG_INFO("Activate OfflineNoiseReprocess Pipeline");

        result = pExtensionModule->ActivatePipeline(pOfflineNoiseReprocessSession->GetSessionHandle(),
            pOfflineNoiseReprocessSession->GetPipelineHandle());
        if (CDKResultSuccess == result)
        {
            pOfflineNoiseReprocessSession->SetPipelineActivateFlag();
        }
        else
        {
            CHX_LOG_ERROR("Activate OfflineNoiseReprocess Pipeline failed");
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::DeactivateOfflineNoiseReprocessPipeline
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::DeactivateOfflineNoiseReprocessPipeline()
{
    CDKResult        result                        = CDKResultSuccess;
    ExtensionModule* pExtensionModule              = ExtensionModule::GetInstance();
    const Session*   pOfflineNoiseReprocessSession = m_pUsecase->GetSessionData(m_offlineNoiseReprocess)->pSession;

    if (TRUE == pOfflineNoiseReprocessSession->IsPipelineActive())
    {
        CHX_LOG_INFO("Deactivate OfflineNoiseReprocess Pipeline");

        result = pExtensionModule->DeactivatePipeline(pOfflineNoiseReprocessSession->GetSessionHandle(),
                                       pOfflineNoiseReprocessSession->GetPipelineHandle(),
                                       CHIDeactivateModeReleaseBuffer);
        if (CDKResultSuccess != result)
        {
            CHX_LOG_ERROR("Deactivate OfflineNoiseReprocess pipelne failed");
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::CleanupOfflineMetadata
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::CleanupOfflineMetadata(
    MFNRInputInfo* pMfnrInputInfo)
{
    // invalidate and check reference count
    UINT32 appMetaRefCount = m_pApplicationInputMeta ? m_pApplicationInputMeta->ReleaseReference("mfnr_input") : 0;
    for (INT32 index = MfnrMaxMetadata - 1; index >= 0 ; --index)
    {
        // release output
        UINT32 outRefCnt = 0;
        if (NULL != m_pInterStageOutputMetadata[index])
        {
            outRefCnt = m_pInterStageOutputMetadata[index]->ReferenceCount();
            m_pMetadataManager->Release(m_pInterStageOutputMetadata[index]);
            m_pInterStageOutputMetadata[index] = NULL;
        }

        UINT32 rdiMetaRefCount = 0;
        if ((NULL != pMfnrInputInfo->pChiMetadata[index]) && (index < static_cast<INT32>(m_mfnrTotalNumFrames)))
        {
            rdiMetaRefCount = pMfnrInputInfo->pChiMetadata[index]->ReleaseReference();
        }

        // invalidate input
        m_pInterStageInputMetadata[index]->Invalidate();

        UINT32 inputRefCount = m_pInterStageInputMetadata[index]->ReleaseReference(GetUniqueStageName(index));
        CHX_LOG_INFO("End MFNR metadata refcount for index %d is app %u input %u rdi %d out %d meta_in %p meta_out %p",
            index, appMetaRefCount, inputRefCount, rdiMetaRefCount, outRefCnt,
            m_pInterStageInputMetadata[index]->GetHandle(),
            m_pInterStageOutputMetadata[index] ? m_pInterStageOutputMetadata[index]->GetHandle() : NULL);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SubmitOfflineMfnrRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::SubmitOfflineMfnrRequest(
    UINT32                     appFrameNumber,
    camera3_capture_request_t* pRequest)
{
    CDKResult     result        = CDKResultSuccess;
    MFNRInputInfo mfnrInputInfo = { };

    CHX_LOG("E. appFrameNumber:%d, request->framenumber:%d", appFrameNumber, pRequest->frame_number);

    Feature*             pPreviousFeature = NULL;
    SnapshotFeatureInfo* pFeatureInfo     = NULL;

    pPreviousFeature = m_pUsecase->GetPreviousFeatureForSnapshot(appFrameNumber, this);
    if (NULL != pPreviousFeature)
    {
        pFeatureInfo = m_pUsecase->GetSnapshotFeatureInfo(appFrameNumber, this);

        if (NULL != pFeatureInfo)
        {
            CHX_LOG("Get request input info from SnapshotFeatureInfo: num inputs:%d, buffer q index:%d, lastSeqId:%d",
                pFeatureInfo->featureInput.numOfInputBuffers,
                pFeatureInfo->featureInput.inputBufferQIdx,
                pFeatureInfo->featureInput.lastSeqId);
        }
    }

    result = CreateMFNRInputInfo(&mfnrInputInfo, pRequest, pFeatureInfo);

    if (CDKResultSuccess == result)
    {
        result = SubmitOfflinePreFilterStageRequest(appFrameNumber, pRequest, &mfnrInputInfo);
        CHX_ASSERT(CDKResultSuccess == result);

        result = SubmitOfflineBlendStageRequest(appFrameNumber, pRequest, &mfnrInputInfo);
        CHX_ASSERT(CDKResultSuccess == result);

        result = SubmitOfflinePostfilterStageRequest(appFrameNumber, pRequest, &mfnrInputInfo);
        CHX_ASSERT(CDKResultSuccess == result);

        if (TRUE == m_noiseReprocessEnable)
        {
            result = SubmitOfflineNoiseReprocessStageRequest(appFrameNumber, pRequest, &mfnrInputInfo);
            CHX_ASSERT(CDKResultSuccess == result);
        }
        if (TRUE == IsJPEGSnapshotConfigured())
        {
            result = SubmitOfflineSnapshotStageRequest(appFrameNumber, pRequest, &mfnrInputInfo);
            CHX_ASSERT(CDKResultSuccess == result);
        }
    }
    else
    {
        CHX_LOG_ERROR(" FATAL Could not process MFNR snapshot Processing!");
    }

    if (FALSE == IsJPEGSnapshotConfigured())
    {
        // Free all MFNR buffers to reduce memory
        m_pMfnrBufferManager[MfnrStagePrefilter][MfnrReferenceFull]->Deactivate(TRUE);
        m_pMfnrBufferManager[MfnrStagePrefilter][MfnrReferenceDS4]->Deactivate(TRUE);
        m_pMfnrBpsRegOutBufferManager->Deactivate(TRUE);

        if (TRUE == m_noiseReprocessEnable)
        {
            m_pOfflineNoiseReprocessBufferManager->Deactivate(TRUE);
        }

        m_isLLSSnapshot = FALSE;
    }

    CleanupOfflineMetadata(&mfnrInputInfo);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SetupStageChiFences
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult  FeatureMFNR::SetupStageChiFences(
    UINT32           numInputs,
    CHISTREAMBUFFER* pInputBuffers,
    UINT32           numOutputs,
    CHISTREAMBUFFER* pOutputBuffers)
{
    CDKResult result = CDKResultEFailed;

    CHX_ASSERT(MaxChiFenceDependencies > numInputs);
    CHX_ASSERT(MaxChiFenceDependencies > numOutputs);

    if (((NULL != pInputBuffers)  && (0 != numInputs)) &&
        ((NULL != pOutputBuffers) && (0 != numOutputs)))
    {
        result  = SetupStageChiInputFences(numInputs, pInputBuffers);
        CHX_ASSERT(CDKResultSuccess == result);
        if (CDKResultSuccess == result)
        {
            result = SetupStageChiOutputFences(numOutputs, pOutputBuffers);
            CHX_ASSERT(CDKResultSuccess == result);
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SetupStageChiInputFences
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult  FeatureMFNR::SetupStageChiInputFences(
    UINT32           numInputs,
    CHISTREAMBUFFER* pInputBuffers,
    BOOL             skipRefFences)
{
    CDKResult            result           = CDKResultSuccess;
    ExtensionModule*     pExtensionModule = NULL;
    UINT                 inputIndex       = 0;
    CHIFENCECREATEPARAMS chiFenceParams   =
    {
        .size = sizeof(CHIFENCECREATEPARAMS),
        .type = ChiFenceTypeInternal
    };

    CHX_LOG_VERBOSE("(E)");

    pExtensionModule = ExtensionModule::GetInstance();
    CHX_ASSERT(NULL != pExtensionModule);

    inputIndex = 2;  // Starting index of inputBuffers[req#][MfnrReferenceFull]

    // Create Input Buffers CHI Fences
    for (UINT32 idx = 0; idx < inputIndex; idx++)
    {
        CHIFENCEINFO& acquireFence = pInputBuffers[idx].acquireFence;

        result = pExtensionModule->CreateChiFence(&chiFenceParams,
                                                  &acquireFence.hChiFence);
        CHX_ASSERT(CDKResultSuccess == result);
        CHX_ASSERT(NULL != acquireFence.hChiFence);

        acquireFence.valid = TRUE;

        CHX_LOG_VERBOSE("MFNR Input Buffer[%u]: %p *** acquireFence %p (%d) valid:%d (Setup) ***",
            idx,
            &(pInputBuffers[idx]),
            acquireFence.hChiFence,
            reinterpret_cast<ChiFence*>(acquireFence.hChiFence)->hFence,
            acquireFence.valid);
    }

    if (FALSE == skipRefFences)
    {
        for (UINT32 idx = inputIndex; idx < numInputs; idx++)
        {
            CHIFENCEINFO& acquireFence = pInputBuffers[idx].acquireFence;

            result = pExtensionModule->CreateChiFence(&chiFenceParams,
                                                      &acquireFence.hChiFence);
            CHX_ASSERT(CDKResultSuccess == result);
            CHX_ASSERT(NULL != acquireFence.hChiFence);

            acquireFence.valid = TRUE;

            CHX_LOG_VERBOSE("MFNR Input Buffer[%u]: %p *** acquireFence %p (%d) valid:%d (Setup) ***",
                idx,
                &(pInputBuffers[idx]),
                acquireFence.hChiFence,
                reinterpret_cast<ChiFence*>(acquireFence.hChiFence)->hFence,
                acquireFence.valid);
        }
    }

    CHX_LOG_VERBOSE("(X) *** result: %i ***", result);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SetupStageChiOutputFences
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult  FeatureMFNR::SetupStageChiOutputFences(
    UINT32           numOutputs,
    CHISTREAMBUFFER* pOutputBuffers)
{
    CDKResult            result           = CDKResultSuccess;
    ExtensionModule*     pExtensionModule = NULL;
    CHIFENCECREATEPARAMS chiFenceParams   =
    {
        .size = sizeof(CHIFENCECREATEPARAMS),
        .type = ChiFenceTypeInternal
    };

    CHX_LOG_VERBOSE("(E)");

    pExtensionModule = ExtensionModule::GetInstance();
    CHX_ASSERT(NULL != pExtensionModule);

    // Create Output Buffers CHI Fences
    for (UINT32 idx = 0; idx < numOutputs; idx++)
    {
        CHIFENCEINFO& releaseFence = pOutputBuffers[idx].releaseFence;

        result = pExtensionModule->CreateChiFence(&chiFenceParams,
                                                  &releaseFence.hChiFence);
        CHX_ASSERT(CDKResultSuccess == result);
        CHX_ASSERT(NULL != releaseFence.hChiFence);

        releaseFence.valid = TRUE;

        CHX_LOG_VERBOSE("MFNR Output Buffer[%u]: %p *** releaseFence %p (%d) valid:%d (Setup) ***",
            idx,
            &(pOutputBuffers[idx]),
            releaseFence.hChiFence,
            reinterpret_cast<ChiFence*>(releaseFence.hChiFence)->hFence,
            releaseFence.valid);
    }

    CHX_LOG_VERBOSE("(X) *** result: %i ***", result);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SignalStageChiFences
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult  FeatureMFNR::SignalStageChiFences(
    UINT32           numInputs,
    CHISTREAMBUFFER* pInputBuffers,
    UINT32           numOutputs,
    CHISTREAMBUFFER* pOutputBuffers)
{
    CDKResult result = CDKResultEFailed;

    CHX_ASSERT(MaxChiFenceDependencies > numInputs);
    CHX_ASSERT(MaxChiFenceDependencies > numOutputs);

    if (((NULL != pInputBuffers)  && (0 != numInputs)) &&
        ((NULL != pOutputBuffers) && (0 != numOutputs)))
    {
        result = SignalStageChiInputFences(numInputs, pInputBuffers);
        CHX_ASSERT(CDKResultSuccess == result);
        if (CDKResultSuccess == result)
        {
            result = SignalStageChiOutputFences(numOutputs, pOutputBuffers);
            CHX_ASSERT(CDKResultSuccess == result);
        }
    }

    return result;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SignalStageChiInputFences
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult  FeatureMFNR::SignalStageChiInputFences(
    UINT32           numInputs,
    CHISTREAMBUFFER* pInputBuffers,
    BOOL             skipBPSFences,
    BOOL             skipRefFences)
{
    CDKResult        result           = CDKResultSuccess;
    ExtensionModule* pExtensionModule = NULL;
    UINT             inputIndex       = 0;

    CHX_LOG_VERBOSE("(E)");

    pExtensionModule = ExtensionModule::GetInstance();
    CHX_ASSERT(NULL != pExtensionModule);

    inputIndex = 2;  // Starting index of inputBuffers[req#][MfnrReferenceFull]

    if (FALSE == skipBPSFences)
    {
        for (UINT32 idx = 0; idx < inputIndex; idx++)
        {
            CHIFENCEINFO acquireFence = pInputBuffers[idx].acquireFence;

            CHX_ASSERT(ChiFenceTypeInternal == acquireFence.type);

            if (TRUE == acquireFence.valid)
            {
                ChiFence* phChiFence = reinterpret_cast<ChiFence*>(acquireFence.hChiFence);
                CHX_ASSERT(NULL != phChiFence);

                CSLFence  hCSLFence  = phChiFence->hFence;

                if ((-1 != hCSLFence) && (0 != hCSLFence))
                {
                    CHX_LOG_VERBOSE("MFNR Input Buffer[%u]: %p *** acquireFence %p (%d) valid:%d (Signal)***",
                        idx,
                        &(pInputBuffers[idx]),
                        acquireFence.hChiFence,
                        hCSLFence,
                        acquireFence.valid);

                    result = pExtensionModule->SignalChiFence(acquireFence.hChiFence,
                                                              CHXFenceResultSuccess);
                    CHX_ASSERT(CDKResultSuccess == result);
                }
                else
                {
                    CHX_LOG_ERROR("MFNR Input Buffer[%u]: %p *** acquireFence %p (%d) invalid (Signal) ***",
                        idx,
                        &(pInputBuffers[idx]),
                        acquireFence.hChiFence,
                        hCSLFence);
                }
            }
            else
            {
                CHX_LOG_ERROR("MFNR Input Buffer[%u]: %p *** acquireFence %p invalid (Signal) ***",
                    idx,
                    &(pInputBuffers[idx]),
                    acquireFence.hChiFence);
            }
        }
    }

    // Signal Input Buffers Ref Fences
    if (FALSE == skipRefFences)
    {
        for (UINT32 idx = inputIndex; idx < numInputs; idx++)
        {
            CHIFENCEINFO acquireFence = pInputBuffers[idx].acquireFence;

            CHX_ASSERT(ChiFenceTypeInternal == acquireFence.type);

            if (TRUE == acquireFence.valid)
            {
                ChiFence* phChiFence = reinterpret_cast<ChiFence*>(acquireFence.hChiFence);
                CHX_ASSERT(NULL != phChiFence);

                CSLFence  hCSLFence  = phChiFence->hFence;

                if ((-1 != hCSLFence) && (0 != hCSLFence))
                {
                    CHX_LOG_VERBOSE("MFNR Input Buffer[%u]: %p *** acquireFence %p (%d) valid:%d (Signal)***",
                        idx,
                        &(pInputBuffers[idx]),
                        acquireFence.hChiFence,
                        hCSLFence,
                        acquireFence.valid);

                    result = pExtensionModule->SignalChiFence(acquireFence.hChiFence,
                                                              CHXFenceResultSuccess);
                    CHX_ASSERT(CDKResultSuccess == result);
                }
                else
                {
                    CHX_LOG_ERROR("MFNR Input Buffer[%u]: %p *** acquireFence %p (%d) invalid (Signal) ***",
                        idx,
                        &(pInputBuffers[idx]),
                        acquireFence.hChiFence,
                        hCSLFence);
                }
            }
            else
            {
                CHX_LOG_ERROR("MFNR Input Buffer[%u]: %p *** acquireFence %p invalid (Signal) ***",
                    idx,
                    &(pInputBuffers[idx]),
                    acquireFence.hChiFence);
            }
        }
    }

    CHX_LOG_VERBOSE("(X) *** result: %i ***", result);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SignalStageChiOutputFences
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult  FeatureMFNR::SignalStageChiOutputFences(
    UINT32           numOutputs,
    CHISTREAMBUFFER* pOutputBuffers)
{
    CDKResult        result           = CDKResultSuccess;
    ExtensionModule* pExtensionModule = NULL;

    CHX_LOG_VERBOSE("(E)");

    pExtensionModule = ExtensionModule::GetInstance();
    CHX_ASSERT(NULL != pExtensionModule);

    // Create Output Buffers CHI Fences
    for (UINT32 idx = 0; idx < numOutputs; idx++)
    {
        CHIFENCEINFO releaseFence = pOutputBuffers[idx].releaseFence;

        CHX_ASSERT(ChiFenceTypeInternal == releaseFence.type);

        if (TRUE == releaseFence.valid)
        {
            ChiFence* phChiFence = reinterpret_cast<ChiFence*>(releaseFence.hChiFence);
            CHX_ASSERT(NULL != phChiFence);

            CSLFence  hCSLFence  = phChiFence->hFence;

            if ((-1 != hCSLFence) && (0 != hCSLFence))
            {
                CHX_LOG_VERBOSE("MFNR Output Buffer[%u]: %p *** releaseFence %p (%d) valid:%d (Signal) ***",
                    idx,
                    &(pOutputBuffers[idx]),
                    releaseFence.hChiFence,
                    hCSLFence,
                    releaseFence.valid);

                result = pExtensionModule->SignalChiFence(releaseFence.hChiFence,
                                                          CHXFenceResultSuccess);
                CHX_ASSERT(CDKResultSuccess == result);
            }
            else
            {
                CHX_LOG_ERROR("MFNR Output Buffer[%u]: %p *** releaseFence %p (%d) invalid (Signal) ***",
                    idx,
                    &(pOutputBuffers[idx]),
                    releaseFence.hChiFence,
                    hCSLFence);
            }
        }
        else
        {
            CHX_LOG_ERROR("MFNR Output Buffer[%u]: %p *** releaseFence %p invalid (Signal) ***",
                idx,
                &(pOutputBuffers[idx]),
                releaseFence.hChiFence);
        }
    }

    CHX_LOG_VERBOSE("(X) *** result: %i ***", result);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::ClearStageChiFences
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult  FeatureMFNR::ClearStageChiFences(
    UINT32           numInputs,
    CHISTREAMBUFFER* pInputBuffers,
    UINT32           numOutputs,
    CHISTREAMBUFFER* pOutputBuffers)
{
    CDKResult result = CDKResultEFailed;

    CHX_ASSERT(MaxChiFenceDependencies > numInputs);
    CHX_ASSERT(MaxChiFenceDependencies > numOutputs);

    if (((NULL != pInputBuffers)  && (0 != numInputs)) &&
        ((NULL != pOutputBuffers) && (0 != numOutputs)))
    {
        result = ClearStageChiInputFences(numInputs, pInputBuffers);
        CHX_ASSERT(CDKResultSuccess == result);
        if (CDKResultSuccess == result)
        {
            result = ClearStageChiOutputFences(numOutputs, pOutputBuffers);
            CHX_ASSERT(CDKResultSuccess == result);
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::ClearStageChiInputFences
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult  FeatureMFNR::ClearStageChiInputFences(
    UINT32           numInputs,
    CHISTREAMBUFFER* pInputBuffers,
    BOOL             skipRefFences)
{
    CDKResult        result           = CDKResultSuccess;
    ExtensionModule* pExtensionModule = NULL;
    UINT             inputIndex       = 0;

    CHX_LOG_VERBOSE("(E)");

    pExtensionModule = ExtensionModule::GetInstance();
    CHX_ASSERT(NULL != pExtensionModule);

    inputIndex = 2;  // Starting index of inputBuffers[req#][MfnrReferenceFull]

    // Release Input Buffers CHI Fences
    for (UINT32 idx = 0; idx < inputIndex; idx++)
    {
        CHIFENCEINFO& acquireFence = pInputBuffers[idx].acquireFence;

        CHX_ASSERT(ChiFenceTypeInternal == acquireFence.type);

        if (TRUE == acquireFence.valid)
        {
            ChiFence* phChiFence = reinterpret_cast<ChiFence*>(acquireFence.hChiFence);
            CHX_ASSERT(NULL != phChiFence);

            CSLFence  hCSLFence  = phChiFence->hFence;

            if ((-1 != hCSLFence) && (0 != hCSLFence))
            {
                CHX_LOG_VERBOSE("MFNR Input Buffer[%u]: %p *** acquireFence %p (%d) valid:%d (Clear) ***",
                    idx,
                    &(pInputBuffers[idx]),
                    acquireFence.hChiFence,
                    hCSLFence,
                    acquireFence.valid);

                result = pExtensionModule->ReleaseChiFence(acquireFence.hChiFence);
                CHX_ASSERT(CDKResultSuccess == result);

                acquireFence.valid     = FALSE;
                acquireFence.hChiFence = NULL;
            }
            else
            {
                CHX_LOG_ERROR("MFNR Input Buffer[%u]: %p *** acquireFence %p (%d) invalid (Clear) ***",
                    idx,
                    &(pInputBuffers[idx]),
                    acquireFence.hChiFence,
                    hCSLFence);
            }
        }
        else
        {
            CHX_LOG_ERROR("MFNR Input Buffer[%u]: %p *** acquireFence %p invalid (Clear) ***",
                idx,
                &(pInputBuffers[idx]),
                acquireFence.hChiFence);
        }
    }

    if (FALSE == skipRefFences)
    {
        for (UINT32 idx = inputIndex; idx < numInputs; idx++)
        {
            CHIFENCEINFO& acquireFence = pInputBuffers[idx].acquireFence;

            CHX_ASSERT(ChiFenceTypeInternal == acquireFence.type);

            if (TRUE == acquireFence.valid)
            {
                ChiFence* phChiFence = reinterpret_cast<ChiFence*>(acquireFence.hChiFence);
                CHX_ASSERT(NULL != phChiFence);

                CSLFence  hCSLFence  = phChiFence->hFence;

                if ((-1 != hCSLFence) && (0 != hCSLFence))
                {
                    CHX_LOG_VERBOSE("MFNR Input Buffer[%u]: %p *** acquireFence %p (%d) valid:%d (Clear) ***",
                        idx,
                        &(pInputBuffers[idx]),
                        acquireFence.hChiFence,
                        hCSLFence,
                        acquireFence.valid);

                    result = pExtensionModule->ReleaseChiFence(acquireFence.hChiFence);
                    CHX_ASSERT(CDKResultSuccess == result);

                    acquireFence.valid     = FALSE;
                    acquireFence.hChiFence = NULL;
                }
                else
                {
                    CHX_LOG_ERROR("MFNR Input Buffer[%u]: %p *** acquireFence %p (%d) invalid (Clear) ***",
                        idx,
                        &(pInputBuffers[idx]),
                        acquireFence.hChiFence,
                        hCSLFence);
                }
            }
            else
            {
                CHX_LOG_ERROR("MFNR Input Buffer[%u]: %p *** acquireFence %p invalid (Clear) ***",
                    idx,
                    &(pInputBuffers[idx]),
                    acquireFence.hChiFence);
            }
        }
    }

    CHX_LOG_VERBOSE("(X) *** result: %i ***", result);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::ClearStageChiOutputFences
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult  FeatureMFNR::ClearStageChiOutputFences(
    UINT32           numOutputs,
    CHISTREAMBUFFER* pOutputBuffers)
{
    CDKResult        result           = CDKResultSuccess;
    ExtensionModule* pExtensionModule = NULL;

    CHX_LOG_VERBOSE("(E)");

    pExtensionModule = ExtensionModule::GetInstance();
    CHX_ASSERT(NULL != pExtensionModule);

    // Release Output Buffers CHI Fences
    for (UINT32 idx = 0; idx < numOutputs; idx++)
    {
        CHIFENCEINFO& releaseFence = pOutputBuffers[idx].releaseFence;

        CHX_ASSERT(ChiFenceTypeInternal == releaseFence.type);

        if (TRUE == releaseFence.valid)
        {
            ChiFence* phChiFence = reinterpret_cast<ChiFence*>(releaseFence.hChiFence);
            CHX_ASSERT(NULL != phChiFence);

            CSLFence  hCSLFence  = phChiFence->hFence;

            if ((-1 != hCSLFence) && (0 != hCSLFence))
            {
                CHX_LOG_VERBOSE("MFNR Output Buffer[%u]: %p *** releaseFence %p (%d) valid:%d (Clear) ***",
                    idx,
                    &(pOutputBuffers[idx]),
                    releaseFence.hChiFence,
                    hCSLFence,
                    releaseFence.valid);

                result = pExtensionModule->ReleaseChiFence(releaseFence.hChiFence);
                CHX_ASSERT(CDKResultSuccess == result);

                releaseFence.valid     = FALSE;
                releaseFence.hChiFence = NULL;
            }
            else
            {
                CHX_LOG_ERROR("MFNR Output Buffer[%u]: %p *** releaseFence %p (%d) invalid (Clear) ***",
                    idx,
                    &(pOutputBuffers[idx]),
                    releaseFence.hChiFence,
                    hCSLFence);
            }
        }
        else
        {
            CHX_LOG_ERROR("MFNR Output Buffer[%u]: %p *** releaseFence %p invalid (Clear) ***",
                idx,
                &(pOutputBuffers[idx]),
                releaseFence.hChiFence);
        }
    }

    CHX_LOG_VERBOSE("(X) *** result: %i ***", result);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SubmitOfflinePreFilterStageRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::SubmitOfflinePreFilterStageRequest(
    UINT32                     appFrameNumber,
    camera3_capture_request_t* pRequest,
    MFNRInputInfo*             pMfnrInputInfo)
{
    CDKResult                     result                   = CDKResultSuccess;
    CHISTREAMBUFFER               inputBuffers[8]          = { };
    CHISTREAMBUFFER               outputBuffers[8]         = { };
    ChiMetadata*                  pMergedMetadata          = NULL;
    UINT32                        numStageInputs           = 0;
    UINT32                        numStageOutputs          = 0;
    UINT                          outputIndex              = 0;
    AnchorFrameSelectionAlgorithm anchorSelectionAlgorithm = AnchorFrameSelectionAlgorithm::None;
    UINT32                        feature2Mode;

    CHX_LOG("------------------------------< Mfnr pre-filter stage >------------------------------");

    numStageInputs                   = 1;
    numStageOutputs                  = 3;
    m_remainingPrefilterStageResults = 1;

    m_numExpectedStageBuffers        = numStageOutputs;
    outputIndex                      = 0;

    ChxUtils::Memset(&m_preFilterStageResult, 0, sizeof(m_preFilterStageResult));

    result = SelectMFNRAnchorFrameAndFrameOrder(pMfnrInputInfo,
                                                inputBuffers,
                                                sizeof(CHISTREAMBUFFER),
                                                &pMergedMetadata,
                                                &anchorSelectionAlgorithm);

    CHX_ASSERT(CDKResultSuccess != result);

    // Release FD Buffers
    if (FALSE == m_pUsecase->IsMultiCameraUsecase())
    {
        for (UINT32 i = 0; i < pMfnrInputInfo->numOfBuffers; ++i)
        {
            m_pFdTargetBuffer->pBufferManager->ReleaseReference(&pMfnrInputInfo->fdbufferArray[i].bufferInfo);
            m_pUsecase->UpdateValidFDBufferLength(m_fdStreamIndex, 1);
        }
    }
    else
    {
        for (UINT32 i = 0; i < pMfnrInputInfo->numOfBuffers; ++i)
        {
            m_pUsecase->ReleaseSingleOffineFDInputResource(pRequest->frame_number, m_activePipelineIndex, i);
        }
    }

    GetOutputBuffer(m_pMfnrBufferManager[MfnrStagePrefilter][MfnrReferenceFull],
                    m_pPrefilterOutStream[MfnrReferenceFull],
                    &outputBuffers[outputIndex++]);

    GetOutputBuffer(m_pMfnrBufferManager[MfnrStagePrefilter][MfnrReferenceDS4],
                    m_pPrefilterOutStream[MfnrReferenceDS4],
                    &outputBuffers[outputIndex++]);

    GetOutputBuffer(m_pMfnrBpsRegOutBufferManager,
                    m_pMfnrBpsRegOutStream,
                    &outputBuffers[outputIndex]);

    DumpMetadata(pMfnrInputInfo->pChiMetadata[m_processingOrder[0]], GetUniqueStageName(m_processingOrder[0], "mfnr_proc_"));

    m_pInterStageInputMetadata[m_metadataIndex]->Merge(*(pMfnrInputInfo->pChiMetadata[m_processingOrder[0]]), TRUE);
    m_pUsecase->UpdateSnapshotMetadataWithRDITags(*(pMfnrInputInfo->pChiMetadata[m_processingOrder[0]]),
        *(m_pInterStageInputMetadata[m_metadataIndex]));

    feature2Mode = static_cast<UINT32>(ChiModeFeature2SubModeType::MFNRBlend);
    ChxUtils::FillTuningModeData(m_pInterStageInputMetadata[m_metadataIndex],
        pRequest,
        m_sensorModeIndex,
        m_pUsecase->GetEffectMode(),
        m_pUsecase->GetSceneMode(),
        m_pUsecase->GetFeature1Mode(),
        &feature2Mode);

    // Debug-data deep-copy
    if (TRUE == ChxUtils::IsVendorTagPresent(m_pInterStageInputMetadata[m_metadataIndex], DebugDataTag))
    {
        CHAR* pData = NULL;
        ChxUtils::GetVendorTagValue(m_pInterStageInputMetadata[m_metadataIndex], DebugDataTag, (VOID**)&pData);
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
                        m_debugDataOfflineSnapshot.pData = CHX_CALLOC(pDebug->size);
                        if (NULL != m_debugDataOfflineSnapshot.pData)
                        {
                            // Success allocating both offline debug-data
                            m_debugDataOffline.size = pDebug->size;
                            m_debugDataOfflineSnapshot.size = pDebug->size;
                            CHX_LOG("DebugDataAll: MFNR alloc: offline %p, snap: %p, size: %zu",
                                    m_debugDataOffline.pData, m_debugDataOfflineSnapshot.pData, pDebug->size);
                        }
                        else
                        {
                            CHX_FREE(m_debugDataOffline.pData);
                            m_debugDataOffline.pData = NULL;
                            m_debugDataOffline.size  = 0;
                        }
                    }

                }
                else if (pDebug->size != m_debugDataOffline.size)
                {
                    // This condition is only for protection in case debug-data size is change in run time while camera is
                    // already processing. This is not allow for this property at this time.
                    CHX_FREE(m_debugDataOffline.pData);
                    m_debugDataOffline.pData = NULL;
                    m_debugDataOffline.size  = 0;
                    CHX_FREE(m_debugDataOfflineSnapshot.pData);
                    m_debugDataOfflineSnapshot.pData = NULL;
                    m_debugDataOfflineSnapshot.size  = 0;

                    m_debugDataOffline.pData = CHX_CALLOC(pDebug->size);
                    if (NULL != m_debugDataOffline.pData)
                    {
                        m_debugDataOfflineSnapshot.pData = CHX_CALLOC(pDebug->size);
                        if (NULL != m_debugDataOfflineSnapshot.pData)
                        {
                            // Success allocating both offline debug-data
                            m_debugDataOffline.size         = pDebug->size;
                            m_debugDataOfflineSnapshot.size = pDebug->size;
                        }
                        else
                        {
                            CHX_FREE(m_debugDataOffline.pData);
                            m_debugDataOffline.pData = NULL;
                            m_debugDataOffline.size  = 0;
                        }
                    }

                }

                if (NULL != m_debugDataOffline.pData)
                {
                    CHX_LOG("Pre-Filter: Replace DebugData: %p (%zu), new: %p (%zu)",
                            pDebug->pData, pDebug->size,
                            m_debugDataOffline.pData, m_debugDataOffline.size);

                    ChxUtils::Memcpy(m_debugDataOffline.pData, pDebug->pData, pDebug->size);

                    result = ChxUtils::SetVendorTagValue(m_pInterStageInputMetadata[m_metadataIndex],
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

    PublishMFNRTotalFrames(m_pInterStageInputMetadata[m_metadataIndex]);

    m_resultsAvailable = FALSE;

    result = ExecuteMfnrRequest(MfnrStagePrefilter,
                               appFrameNumber,
                               numStageOutputs,
                               &outputBuffers[0],
                               numStageInputs,
                               &inputBuffers[0],
                               m_pInterStageInputMetadata[m_metadataIndex],
                               m_pInterStageOutputMetadata[m_metadataIndex]);

    if (CDKResultSuccess != result)
    {
        CHX_LOG_ERROR("!!!!!!! ExecuteMfnrRequest return failed: %d", result);
    }

    ++m_metadataIndex;

    m_pMfnrResultMutex->Lock();

    // Wait for all prefilter results to come back - 3 output buffers: Full, DS4, BpsRegOut and Metadata
    while (FALSE == m_resultsAvailable && result == CDKResultSuccess)
    {
        m_pMfnrResultAvailable->Wait(m_pMfnrResultMutex->GetNativeHandle());
    }

    if (TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress)))
    {
        m_resultsAvailable = FALSE;

        m_pMfnrResultMutex->Unlock();

        CHX_LOG_INFO("MFNR blend process return because of cleanup");
        return CDKResultSuccess;
    }

    m_resultsAvailable = FALSE;

    m_pMfnrResultMutex->Unlock();

    if (CDKResultSuccess != result)
    {
        DeactivateOfflinePipeline(MfnrStagePrefilter);
    }
    else
    {
        result = DeactivateOfflinePipeline(MfnrStagePrefilter);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::PrepareOfflineBlendStageParallelRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::PrepareOfflineBlendStageParallelRequest(
    MFNRInputInfo*   pMfnrInputInfo,
    UINT             processingOrderIndex,
    UINT32           activeBlendParallelRequest,
    UINT32           numInputs,
    CHISTREAMBUFFER* pInputBuffers,
    UINT32           numOutputs,
    CHISTREAMBUFFER  (*pOutputBuffers)[MaxChiStreamBuffers], // [][8]
    BOOL*            pPrefilterStageOutputBuffersAsInput)
{
    CDKResult  result     = CDKResultEFailed;
    UINT       inputIndex = 0;

    CHX_ASSERT(MaxChiFenceDependencies > numInputs);
    CHX_ASSERT(MaxChiFenceDependencies > numOutputs);

    if (((NULL != pInputBuffers)  && (0 != numInputs)) &&
        ((NULL != pOutputBuffers) && (0 != numOutputs)))
    {
        ChxUtils::Memcpy(&pInputBuffers[inputIndex],
                         &(pMfnrInputInfo->bufferArray[m_processingOrder[processingOrderIndex]]),
                         ChiStreamBufferSize);
        pInputBuffers[inputIndex].size                 = ChiStreamBufferSize;
        pInputBuffers[inputIndex++].acquireFence.valid = FALSE;

        ChxUtils::Memcpy(&pInputBuffers[inputIndex],
                         &m_preFilterAnchorFrameRegResultBuffer,
                         ChiStreamBufferSize);
        pInputBuffers[inputIndex].size               = ChiStreamBufferSize;
        pInputBuffers[inputIndex].acquireFence.valid = FALSE;
        pInputBuffers[inputIndex++].pStream          = m_pMfnrBpsRegInStream;

        CHX_LOG_VERBOSE("Active Blend Parallel Request:%u "
                        "PrefilterStageOutputBuffersAsInput:%u",
                        activeBlendParallelRequest,
                        (*pPrefilterStageOutputBuffersAsInput));

        if ((0 == activeBlendParallelRequest) && (TRUE == (*pPrefilterStageOutputBuffersAsInput)))
        {
            ChxUtils::Memcpy(&pInputBuffers[inputIndex],
                             &m_preFilterStageResult.refOutputBuffer[MfnrReferenceFull],
                             ChiStreamBufferSize);
            pInputBuffers[inputIndex].size               = ChiStreamBufferSize;
            pInputBuffers[inputIndex].acquireFence.valid = FALSE;
            pInputBuffers[inputIndex].pStream            = m_pBlendInStream[MfnrReferenceFull];

            CHX_LOG_VERBOSE("MfnrReferenceFull :: InputBuffers[%u][%u].buffer:%p <- PreFilterStageResult buffer:%p Hostptr=%p",
                            activeBlendParallelRequest,
                            inputIndex,
                            pInputBuffers[inputIndex].bufferInfo.phBuffer,
                            m_preFilterStageResult.refOutputBuffer[MfnrReferenceFull].bufferInfo.phBuffer,
                            CHIBufferManager::GetCPUAddress(&m_preFilterStageResult.refOutputBuffer[MfnrReferenceFull].bufferInfo, 0));

            inputIndex++;

            ChxUtils::Memcpy(&pInputBuffers[inputIndex],
                             &m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4],
                             ChiStreamBufferSize);
            pInputBuffers[inputIndex].size               = ChiStreamBufferSize;
            pInputBuffers[inputIndex].acquireFence.valid = FALSE;
            pInputBuffers[inputIndex].pStream            = m_pBlendInStream[MfnrReferenceDS4];

            CHX_LOG_VERBOSE("MfnrReferenceDS4 :: InputBuffers[%u][%u].buffer:%p <- PreFilterStageResult buffer:%p Hostptr=%p",
                          activeBlendParallelRequest,
                          inputIndex,
                          pInputBuffers[inputIndex].bufferInfo.phBuffer,
                          m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4].bufferInfo.phBuffer,
                          CHIBufferManager::GetCPUAddress(&m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4].bufferInfo, 0));

            inputIndex++;

            ChxUtils::Memcpy(&pInputBuffers[inputIndex],
                             &m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4],
                             ChiStreamBufferSize);
            pInputBuffers[inputIndex].size               = ChiStreamBufferSize;
            pInputBuffers[inputIndex].acquireFence.valid = FALSE;
            pInputBuffers[inputIndex].pStream            = m_pScaleInStream[MfnrReferenceDS4];

            CHX_LOG_VERBOSE("MfnrReferenceDS4 :: InputBuffers[%u][%u].buffer:%p <- PreFilterStageResult buffer:%p Hostptr=%p",
                          activeBlendParallelRequest,
                          inputIndex,
                          pInputBuffers[inputIndex].bufferInfo.phBuffer,
                          m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4].bufferInfo.phBuffer,
                          CHIBufferManager::GetCPUAddress(&m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4].bufferInfo, 0));
        }
        else if (((0 == activeBlendParallelRequest) && (FALSE == (*pPrefilterStageOutputBuffersAsInput))) ||
                 (1 == activeBlendParallelRequest))
        {
            /*/
            /// Connect results/output buffers of the parallel request (#0/#1) as the
            /// request/input buffers for next parallel request (#1/#0) respectively.
            /*/

            UINT32 priorBlendParallelRequest = (activeBlendParallelRequest ^ 1);

            ChxUtils::Memcpy(&pInputBuffers[inputIndex],
                             &pOutputBuffers[priorBlendParallelRequest][MfnrReferenceFull],
                             ChiStreamBufferSize);

            pInputBuffers[inputIndex].size               = ChiStreamBufferSize;
            pInputBuffers[inputIndex].acquireFence.valid = FALSE;
            pInputBuffers[inputIndex].releaseFence.valid = FALSE;
            pInputBuffers[inputIndex].pStream            = m_pBlendInStream[MfnrReferenceFull];

            CHX_LOG_VERBOSE("MfnrReferenceFull :: InputBuffers[%u][%u].buffer:%p <- OutputBuffers[%u][%u].buffer:%p Hostptr=%p",
                            activeBlendParallelRequest,
                            inputIndex,
                            pInputBuffers[inputIndex].bufferInfo.phBuffer,
                            priorBlendParallelRequest,
                            MfnrReferenceFull,
                            pOutputBuffers[priorBlendParallelRequest][MfnrReferenceFull].bufferInfo.phBuffer,
                            CHIBufferManager::GetCPUAddress(&pInputBuffers[inputIndex].bufferInfo, 0));

            inputIndex++;

            ChxUtils::Memcpy(&pInputBuffers[inputIndex],
                             &pOutputBuffers[priorBlendParallelRequest][MfnrReferenceDS4],
                             ChiStreamBufferSize);

            pInputBuffers[inputIndex].size = ChiStreamBufferSize;
            pInputBuffers[inputIndex].acquireFence.valid = FALSE;
            pInputBuffers[inputIndex].releaseFence.valid = FALSE;
            pInputBuffers[inputIndex].pStream = m_pBlendInStream[MfnrReferenceDS4];

            CHX_LOG_VERBOSE("MfnrReferenceDS4 :: InputBuffers[%u][%u].buffer:%p <- OutputBuffers[%u][%u].buffer:%p Hostptr=%p",
                            activeBlendParallelRequest,
                            inputIndex,
                            pInputBuffers[inputIndex].bufferInfo.phBuffer,
                            priorBlendParallelRequest,
                            MfnrReferenceDS4,
                            pOutputBuffers[priorBlendParallelRequest][MfnrReferenceDS4].bufferInfo.phBuffer,
                            CHIBufferManager::GetCPUAddress(&pInputBuffers[inputIndex].bufferInfo, 0));

            inputIndex++;

            ChxUtils::Memcpy(&pInputBuffers[inputIndex],
                             &pOutputBuffers[priorBlendParallelRequest][MfnrReferenceDS4],
                             ChiStreamBufferSize);

            pInputBuffers[inputIndex].size               = ChiStreamBufferSize;
            pInputBuffers[inputIndex].acquireFence.valid = FALSE;
            pInputBuffers[inputIndex].releaseFence.valid = FALSE;
            pInputBuffers[inputIndex].pStream            = m_pScaleInStream[MfnrReferenceDS4];

            CHX_LOG_VERBOSE("MfnrReferenceDS4 :: InputBuffers[%u][%u].buffer:%p <- OutputBuffers[%u][%u].buffer:%p Hostptr=%p",
                            activeBlendParallelRequest,
                            inputIndex,
                            pInputBuffers[inputIndex].bufferInfo.phBuffer,
                            priorBlendParallelRequest,
                            MfnrReferenceDS4,
                            pOutputBuffers[priorBlendParallelRequest][MfnrReferenceDS4].bufferInfo.phBuffer,
                            CHIBufferManager::GetCPUAddress(&pInputBuffers[inputIndex].bufferInfo, 0));
        }
        else
        {
            // Shouldn't be in here
            CHX_ASSERT(FALSE);
        }

        if ((0 == activeBlendParallelRequest) || (1 == activeBlendParallelRequest))
        {
            // Reuse buffers instead of creating them over and again
            if (NULL == pOutputBuffers[activeBlendParallelRequest][MfnrReferenceFull].bufferInfo.phBuffer)
            {
                GetOutputBuffer(m_pMfnrBufferManager[MfnrStagePrefilter][MfnrReferenceFull],
                                m_pBlendOutStream[MfnrReferenceFull],
                                &pOutputBuffers[activeBlendParallelRequest][MfnrReferenceFull]);
                pOutputBuffers[activeBlendParallelRequest][MfnrReferenceFull].acquireFence.valid = FALSE;
                pOutputBuffers[activeBlendParallelRequest][MfnrReferenceFull].releaseFence.valid = FALSE;

                CHX_LOG_VERBOSE("MfnrReferenceFull :: OutputBuffers[%u][%u].buffer:%p (New) Hostptr=%p",
                                activeBlendParallelRequest,
                                MfnrReferenceFull,
                                pOutputBuffers[activeBlendParallelRequest][MfnrReferenceFull].bufferInfo.phBuffer,
                                CHIBufferManager::GetCPUAddress(&pOutputBuffers[activeBlendParallelRequest][MfnrReferenceFull].bufferInfo, 0));
            }

            if (NULL == pOutputBuffers[activeBlendParallelRequest][MfnrReferenceDS4].bufferInfo.phBuffer)
            {
                GetOutputBuffer(m_pMfnrBufferManager[MfnrStagePrefilter][MfnrReferenceDS4],
                                m_pBlendOutStream[MfnrReferenceDS4],
                                &pOutputBuffers[activeBlendParallelRequest][MfnrReferenceDS4]);
                pOutputBuffers[activeBlendParallelRequest][MfnrReferenceDS4].acquireFence.valid = FALSE;
                pOutputBuffers[activeBlendParallelRequest][MfnrReferenceDS4].releaseFence.valid = FALSE;

                CHX_LOG_VERBOSE("MfnrReferenceDS4 :: OutputBuffers[%u][%u].buffer:%p (New) Hostptr=%p",
                                activeBlendParallelRequest,
                                MfnrReferenceDS4,
                                pOutputBuffers[activeBlendParallelRequest][MfnrReferenceDS4].bufferInfo.phBuffer,
                                CHIBufferManager::GetCPUAddress(&pOutputBuffers[activeBlendParallelRequest][MfnrReferenceDS4].bufferInfo, 0));
            }

            result = SetupStageChiFences(numInputs,
                                         pInputBuffers,
                                         numOutputs,
                                         pOutputBuffers[activeBlendParallelRequest]);
            CHX_ASSERT(CDKResultSuccess == result);
        }
        else
        {
            // Shouldn't be in here
            CHX_ASSERT(FALSE);
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SubmitOfflineBlendStageParallelRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::SubmitOfflineBlendStageParallelRequest(
    UINT32           appFrameNumber,
    UINT32           activeBlendParallelRequest,
    UINT32           numInputs,
    CHISTREAMBUFFER* pInputBuffers,
    UINT32           numOutputs,
    CHISTREAMBUFFER  (*pOutputBuffers)[MaxChiStreamBuffers],  // [][8]
    BOOL             priorBlendParallelResultExpected)
{
    CDKResult result = CDKResultSuccess;

    CHX_LOG_VERBOSE("Active Blend Parallel Request:%u", activeBlendParallelRequest);

    if (0 == activeBlendParallelRequest)
    {
        if (FALSE == priorBlendParallelResultExpected)
        {
            /*/
            /// @note: All fences should not be signalled in case of the last tandem request
            /// submision i.e., ParReq#1 followed by ParReq#0, as it would lead to incorrect
            /// signalling w.r.t the buffers from ParReq#1 which are not processed/ready yet
            /// (which would take place only after this submit).
            /*/

            // All fences were already setup, just signal them as ready

            result = SignalStageChiInputFences(numInputs,
                                               pInputBuffers);
            CHX_ASSERT(CDKResultSuccess == result);
        }
        else
        {
            BOOL skipBPSFences = FALSE;
            BOOL skipRefFences = TRUE;

            // Signal only input fences for BPS
            result = SignalStageChiInputFences(numInputs,
                                               pInputBuffers,
                                               skipBPSFences,
                                               skipRefFences);
            CHX_ASSERT(CDKResultSuccess == result);
        }
    }
    else if (1 == activeBlendParallelRequest)
    {
        BOOL skipBPSFences = FALSE;
        BOOL skipRefFences = TRUE;

        // Signal only input fences for BPS
        result = SignalStageChiInputFences(numInputs,
                                           pInputBuffers,
                                           skipBPSFences,
                                           skipRefFences);
        CHX_ASSERT(CDKResultSuccess == result);

        /*/
        /// @note: Remaining/reference input fences would be signalled from the method
        /// ProcessOfflineBlendStageParallelRequest() when output fences from ParReq#0
        /// are signalled. This is to indicate that these are now ready and thus would
        /// ungate the already submitted but pending request due to fence dependencies
        /// with the DRQ mechanism in CAMX layer. The DRQ after resolving dependencies
        /// will submit that request to KMD for processing.
        /*/
    }
    else
    {
        // Shouldn't be in here
        CHX_ASSERT(FALSE);
    }

    result = ExecuteMfnrRequest(MfnrStageBlend,
                                appFrameNumber,
                                numOutputs,
                                pOutputBuffers[activeBlendParallelRequest],
                                numInputs,
                                pInputBuffers,
                                m_pInterStageInputMetadata[m_metadataIndex],
                                m_pInterStageOutputMetadata[m_metadataIndex]);

    ++m_metadataIndex;

    CHX_ASSERT(CDKResultSuccess == result);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::ReleaseOfflineBlendStageOutputBuffers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::ReleaseOfflineBlendStageOutputBuffers(
    UINT32          numOutputs,
    CHISTREAMBUFFER (*pOutputBuffers)[MaxChiStreamBuffers]) // [][8]
{
    CAMX_UNREFERENCED_PARAM(numOutputs);

    for (UINT parallelRequest = 0; (parallelRequest < MaxActiveBlendRequests); parallelRequest++)
    {
        if ((NULL != pOutputBuffers[parallelRequest][MfnrReferenceFull].bufferInfo.phBuffer) &&
            (m_preFilterStageResult.refOutputBuffer[MfnrReferenceFull].bufferInfo.phBuffer !=
             pOutputBuffers[parallelRequest][MfnrReferenceFull].bufferInfo.phBuffer))
        {
            CHX_LOG_VERBOSE("ReleaseReference of MfnrReferenceFull output buffer[%u] for ParReq#:%u phBuffer=%p Hostptr=%p",
                            MfnrReferenceFull,
                            parallelRequest,
                            pOutputBuffers[parallelRequest][MfnrReferenceFull].bufferInfo.phBuffer,
                            CHIBufferManager::GetCPUAddress(&(pOutputBuffers[parallelRequest][MfnrReferenceFull].bufferInfo), 0));

            m_pMfnrBufferManager[MfnrStagePrefilter][MfnrReferenceFull]->ReleaseReference(
                &(pOutputBuffers[parallelRequest][MfnrReferenceFull].bufferInfo));
        }

        if ((NULL != pOutputBuffers[parallelRequest][MfnrReferenceDS4].bufferInfo.phBuffer) &&
            (m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4].bufferInfo.phBuffer !=
             pOutputBuffers[parallelRequest][MfnrReferenceDS4].bufferInfo.phBuffer))
        {
            CHX_LOG_VERBOSE("ReleaseReference of MfnrReferenceDS4 output buffer[%u] for ParReq#:%u phBuffer=%p Hostptr=%p",
                            MfnrReferenceDS4,
                            parallelRequest,
                            pOutputBuffers[parallelRequest][MfnrReferenceDS4].bufferInfo.phBuffer,
                            CHIBufferManager::GetCPUAddress(&(pOutputBuffers[parallelRequest][MfnrReferenceDS4].bufferInfo), 0));

            m_pMfnrBufferManager[MfnrStagePrefilter][MfnrReferenceDS4]->ReleaseReference(
                &(pOutputBuffers[parallelRequest][MfnrReferenceDS4].bufferInfo));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::ProcessOfflineBlendStageParallelRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::ProcessOfflineBlendStageParallelRequest(
    UINT32          activeBlendParallelRequest,
    UINT32          numInputs,
    CHISTREAMBUFFER (*pInputBuffers)[MaxChiStreamBuffers],  // [][8]
    UINT32          numOutputs,
    CHISTREAMBUFFER (*pOutputBuffers)[MaxChiStreamBuffers], // [][8]
    BOOL            priorBlendParallelResultExpected,
    BOOL            nextBlendParallelRequestExpected,
    BOOL            releasePrefilterStageOutputBuffers)
{
    CDKResult      result            = CDKResultSuccess;
    UINT           inputIndex        = 0;
    OSMutexHandle* mutexNativeHandle = m_pMfnrResultMutex->GetNativeHandle();

    m_pMfnrResultMutex->Lock();

    // Wait for all post-prefilter scale results to come back - 2 output buffers: DS16, DS64, and Metadata
    while (FALSE == m_resultsAvailable)
    {
        CHX_LOG_VERBOSE("Wait for blend stage results - ParReq:%u Results Available:%s",
                        activeBlendParallelRequest,
                        ((TRUE == m_resultsAvailable) ? "TRUE" : "FALSE"));

        m_pMfnrResultAvailable->Wait(mutexNativeHandle);
    }

    CHX_LOG_VERBOSE("Wait for blend stage results - ParReq:%u ... DONE Results Available:%s",
                    activeBlendParallelRequest,
                    ((TRUE == m_resultsAvailable) ? "TRUE" : "FALSE"));
    m_resultsAvailable = FALSE;

    CHX_LOG_VERBOSE("Remaining Blend Stage Results:%u", m_remainingBlendStageResults);

    CHX_LOG_VERBOSE("Active Blend Parallel Request:%u "
                    "Prior Blend Parallel Result Expected:%u "
                    "Next Blend Parallel Request Expected:%u "
                    "ReleasePrefilterStageOutputBuffers:%u",
                    activeBlendParallelRequest,
                    priorBlendParallelResultExpected,
                    nextBlendParallelRequestExpected,
                    releasePrefilterStageOutputBuffers);

    if ((0 == activeBlendParallelRequest) && (TRUE == releasePrefilterStageOutputBuffers))
    {
        /*/
        /// Actual Prefilter Stage result buffers, only once. Caller has to make
        /// sure, it is the case.
        /*/

        // Release reference to blend stage output image buffers from Pre-Filter/ParReq #1
        CHX_LOG_VERBOSE("ReleaseReference of MfnrReferenceFull preFilterStage buffer[%u] for ParReq#:%u "
                        "phBuffer=%p Hostptr=%p",
                        MfnrReferenceFull,
                        activeBlendParallelRequest,
                        m_preFilterStageResult.refOutputBuffer[MfnrReferenceFull].bufferInfo.phBuffer,
                        CHIBufferManager::GetCPUAddress(&m_preFilterStageResult.refOutputBuffer[MfnrReferenceFull].bufferInfo, 0));

        m_pMfnrBufferManager[MfnrStagePrefilter][MfnrReferenceFull]->ReleaseReference(
            &(m_preFilterStageResult.refOutputBuffer[MfnrReferenceFull].bufferInfo));

        CHX_LOG_VERBOSE("ReleaseReference of MfnrReferenceDS4 preFilterStage buffer[%u] for ParReq#:%u "
                        "phBuffer=%p Hostptr=%p",
                        MfnrReferenceDS4,
                        activeBlendParallelRequest,
                        m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4].bufferInfo.phBuffer,
                        CHIBufferManager::GetCPUAddress(&m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4].bufferInfo, 0));

        m_pMfnrBufferManager[MfnrStagePrefilter][MfnrReferenceDS4]->ReleaseReference(
            &(m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4].bufferInfo));
    }

    if ((0 == activeBlendParallelRequest) || (1 == activeBlendParallelRequest))
    {
        if (TRUE == priorBlendParallelResultExpected)
        {
            // For ParReq #1|#0 submitted before ParReq #0|#1 being processed

            UINT32 activeNextBlendParallelRequest = (activeBlendParallelRequest ^ 1);
            BOOL   skipBPSFences = TRUE;

            /*/
            /// @note: Results/output buffers of parallel request (#0|#1) used as request/input
            /// buffers for the next parallel request (#1|#0), which have already been setup up
            /// during preparation stage. All that is left to do is to signal the input fences.
            /*/

            // Now signal input ref fences of ParReq (#1|#0) (as output fences of ParReq #0|#1
            // signalled) i.e., no need to signal all, but the ones excluded from ParReq #1|#0
            // submitted before.
            result = SignalStageChiInputFences(numInputs,
                                               pInputBuffers[activeNextBlendParallelRequest],
                                               skipBPSFences);
            CHX_ASSERT(CDKResultSuccess == result);
        }
        else
        {
            /*/
            /// Prefilter Stage result buffers now carry Blend Stage output buffers,
            /// which would be passed onto Postfilter Stage as input buffers thus do
            /// not attempt to release them here. Let Post Filter Stage handle their
            /// release correctly.  Even with Blend Stage processing is disabled, it
            /// would nealty follow the same sequence i.e.,  Prefilter Stage buffers
            /// gets released by Postfilter Stage.
            /*/

            //* Copy blend Full and DS4 Results *//

            // For Only/last ParReq #0|#1 (to handover to post-filter stage)
            if (FALSE == nextBlendParallelRequestExpected)
            {
                ChxUtils::Memcpy(&m_preFilterStageResult.refOutputBuffer[MfnrReferenceFull],
                    &m_blendStageResult.refOutputBuffer[MfnrReferenceFull],
                    ChiStreamBufferSize);
                m_preFilterStageResult.refOutputBuffer[MfnrReferenceFull].size = ChiStreamBufferSize;

                CHX_LOG_VERBOSE("MfnrReferenceFull :: PreFilterStageResult buffer:%p <- BlendStageResult buffer:%p Hostptr=%p",
                    m_preFilterStageResult.refOutputBuffer[MfnrReferenceFull].bufferInfo.phBuffer,
                    m_blendStageResult.refOutputBuffer[MfnrReferenceFull].bufferInfo.phBuffer,
                    CHIBufferManager::GetCPUAddress(&m_preFilterStageResult.refOutputBuffer[MfnrReferenceFull].bufferInfo, 0));

                ChxUtils::Memcpy(&m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4],
                    &m_blendStageResult.refOutputBuffer[MfnrReferenceDS4],
                    ChiStreamBufferSize);
                m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4].size = ChiStreamBufferSize;

                CHX_LOG_VERBOSE("MfnrReferenceDS4 :: PreFilterStageResult buffer:%p <- BlendStageResult buffer:%p Hostptr=%p",
                    m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4].bufferInfo.phBuffer,
                    m_blendStageResult.refOutputBuffer[MfnrReferenceDS4].bufferInfo.phBuffer,
                    CHIBufferManager::GetCPUAddress(&m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4].bufferInfo, 0));

                ReleaseOfflineBlendStageOutputBuffers(numOutputs, pOutputBuffers);
            }
            else
            {
                // Shouldn't be in here
                CHX_ASSERT(FALSE);
            }
        }

        result = ClearStageChiOutputFences(numOutputs,
                                           pOutputBuffers[activeBlendParallelRequest]);
        CHX_ASSERT(CDKResultSuccess == result);

        result = ClearStageChiInputFences(numInputs,
                                          pInputBuffers[activeBlendParallelRequest]);
        CHX_ASSERT(CDKResultSuccess == result);
    }
    else
    {
        // Shouldn't be in here
        CHX_ASSERT(FALSE);
    }

    m_pMfnrResultMutex->Unlock();

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::PrepareAndSubmitNextRequestAndOrProcessCurrentRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::PrepareAndSubmitNextRequestAndOrProcessCurrentRequest(
    INT*                               pActiveBlendParallelRequest,
    UINT32                             appFrameNumber,
    camera3_capture_request_t*         pRequest,
    MFNRInputInfo*                     pMfnrInputInfo,
    CHISTREAMBUFFER                    inputBuffers[][MaxChiStreamBuffers],
    CHISTREAMBUFFER                    outputBuffers[][MaxChiStreamBuffers],
    UINT32                             numStageInputs,
    UINT32                             numStageOutputs,
    UINT                               numberBlendRequests,
    UINT*                              pRemainingBlendRequests,
    UINT*                              pCurrentBlendRequest,
    BlendStageParallelRequestFSMState  nextState,
    BlendStageParallelRequestFSMState* pUpdatedState,
    BOOL*                              pPrefilterStageOutputBuffersAsInput,
    BOOL*                              pReleasePrefilterStageOutputBuffers)
{
    CDKResult result               = CDKResultSuccess;
    UINT      processingOrderIndex = 0;
    UINT32    feature2Mode;

    CHX_ASSERT(NULL != pActiveBlendParallelRequest);
    CHX_ASSERT(NULL != pMfnrInputInfo);
    CHX_ASSERT(NULL != pRemainingBlendRequests);
    CHX_ASSERT(NULL != pUpdatedState);

    CHX_LOG_VERBOSE("Remaining Blend Requests:%u Current Blend Request:%u",
                    (*pRemainingBlendRequests),
                    (*pCurrentBlendRequest));

    if (0 < (*pRemainingBlendRequests))
    {
        auto previousActiveBlendParallelRequest = (*pActiveBlendParallelRequest);
        auto priorBlendParallelResultExpected   = TRUE;
        auto nextBlendParallelRequestExpected   = FALSE;

        (*pActiveBlendParallelRequest) ^= 1;

        CHX_LOG_VERBOSE("Active Blend Parallel Request:%u Previous Active Blend Parallel Request:%u",
                        (*pActiveBlendParallelRequest),
                        previousActiveBlendParallelRequest);

        processingOrderIndex = ((*pCurrentBlendRequest) + 1);

        ChxUtils::Memset(&m_blendStageResult, 0, sizeof(m_blendStageResult));

        DumpMetadata(pMfnrInputInfo->pChiMetadata[m_processingOrder[processingOrderIndex]],
                     GetUniqueStageName(m_processingOrder[processingOrderIndex], "mfnr_proc_"));

        m_pInterStageInputMetadata[m_metadataIndex]->Merge(*(pMfnrInputInfo->pChiMetadata[
                                                             m_processingOrder[processingOrderIndex]]), TRUE);

        feature2Mode = static_cast<UINT32>(ChiModeFeature2SubModeType::MFNRBlend);
        ChxUtils::FillTuningModeData(m_pInterStageInputMetadata[m_metadataIndex],
                                     pRequest,
                                     m_sensorModeIndex,
                                     m_pUsecase->GetEffectMode(),
                                     m_pUsecase->GetSceneMode(),
                                     m_pUsecase->GetFeature1Mode(),
                                     &feature2Mode);

        CHAR debugString[20];
        CdkUtils::SNPrintF(debugString, sizeof(debugString),
                           "MFNR Blend #%u",
                           (*pCurrentBlendRequest));
        result = m_pUsecase->MergeDebugData(m_pInterStageInputMetadata[m_metadataIndex], &m_debugDataOffline, debugString);
        if (CDKResultSuccess != result)
        {
            CHX_LOG_INFO("Fail to merge DebugData");
        }

        PublishMFNRTotalFrames(m_pInterStageInputMetadata[m_metadataIndex]);

        CHX_LOG_VERBOSE("-----------------------< Mfnr blend stage (Prepare and Submit ParReq #%u) >-----------------------",
                        (*pActiveBlendParallelRequest));

        *pPrefilterStageOutputBuffersAsInput = (0 == (*pCurrentBlendRequest));

        result = PrepareOfflineBlendStageParallelRequest(pMfnrInputInfo,
                                                         processingOrderIndex,
                                                         (*pActiveBlendParallelRequest),
                                                         numStageInputs,
                                                         inputBuffers[(*pActiveBlendParallelRequest)],
                                                         numStageOutputs,
                                                         outputBuffers,
                                                         pPrefilterStageOutputBuffersAsInput);
        CHX_ASSERT(CDKResultSuccess == result);

        m_numExpectedStageBuffers += numStageOutputs;    // Full, DS4
        m_remainingBlendStageResults++;

        CHX_LOG_VERBOSE("Number of Stage Outputs:%u "
                        "Number of Expected Stage Buffers:%u "
                        "Remaining Blend Stage Results:%u",
                        numStageOutputs,
                        m_numExpectedStageBuffers,
                        m_remainingBlendStageResults);

        nextBlendParallelRequestExpected = (1 < (*pRemainingBlendRequests));

        result = SubmitOfflineBlendStageParallelRequest(appFrameNumber,
                                                        (*pActiveBlendParallelRequest),
                                                        numStageInputs,
                                                        inputBuffers[(*pActiveBlendParallelRequest)],
                                                        numStageOutputs,
                                                        outputBuffers,
                                                        priorBlendParallelResultExpected);
        CHX_ASSERT(CDKResultSuccess == result);

        (*pRemainingBlendRequests)--;
        (*pCurrentBlendRequest)++;

        CHX_LOG_VERBOSE("Remaining Blend Requests:%u Current Blend Request:%u",
                        (*pRemainingBlendRequests),
                        (*pCurrentBlendRequest));

        CHX_ASSERT(0 != (*pRemainingBlendRequests));
        CHX_ASSERT((*pCurrentBlendRequest) < numberBlendRequests);

        CHX_LOG_VERBOSE("-----------------------< Mfnr blend stage (Process ParReq #%u) >-----------------------",
                        previousActiveBlendParallelRequest);

        // Process the previous request
        result = ProcessOfflineBlendStageParallelRequest(previousActiveBlendParallelRequest,
                                                         numStageInputs,
                                                         inputBuffers,
                                                         numStageOutputs,
                                                         outputBuffers,
                                                         priorBlendParallelResultExpected,
                                                         nextBlendParallelRequestExpected,
                                                         (*pReleasePrefilterStageOutputBuffers));
        CHX_ASSERT(CDKResultSuccess == result);

        *pReleasePrefilterStageOutputBuffers = (0 == (*pCurrentBlendRequest));
        *pUpdatedState                       = nextState;
    }
    else
    {
        auto priorBlendParallelResultExpected = FALSE;
        auto nextBlendParallelRequestExpected = FALSE;

        CHX_LOG_VERBOSE("-----------------------< Mfnr blend stage (Process ParReq #%u) >-----------------------",
                        (*pActiveBlendParallelRequest));

        // Process the previous/last request
        result = ProcessOfflineBlendStageParallelRequest((*pActiveBlendParallelRequest),
                                                         numStageInputs,
                                                         inputBuffers,
                                                         numStageOutputs,
                                                         outputBuffers,
                                                         priorBlendParallelResultExpected,
                                                         nextBlendParallelRequestExpected,
                                                         (*pReleasePrefilterStageOutputBuffers));
        CHX_ASSERT(CDKResultSuccess == result);

        *pUpdatedState = BlendStageParallelRequestFSMState::Exit;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SubmitOfflineBlendStageRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::SubmitOfflineBlendStageRequest(
    UINT32                     appFrameNumber,
    camera3_capture_request_t* pRequest,
    MFNRInputInfo*             pMfnrInputInfo)
{
    CDKResult       result                 = CDKResultSuccess;
    CHISTREAMBUFFER inputBuffers[MaxActiveBlendRequests][MaxChiStreamBuffers]  = { };
    CHISTREAMBUFFER outputBuffers[MaxActiveBlendRequests][MaxChiStreamBuffers] = { };
    UINT32          numStageInputs         = 0;
    UINT32          numStageOutputs        = 0;

    UINT            numberBlendRequests    = 0;
    UINT            remainingBlendRequests = 0;
    UINT            currentBlendRequest    = 0;
    UINT32          feature2Mode;

    CHX_ASSERT(2 == MaxActiveBlendRequests);
    CHX_ASSERT(2 <= m_mfnrTotalNumFrames);

    numberBlendRequests    = (m_mfnrTotalNumFrames - 2);
    remainingBlendRequests = numberBlendRequests;
    currentBlendRequest    = 0;

    CHX_LOG_VERBOSE("E");

    CHX_LOG_VERBOSE("Number of Blend Requests:%u Remaining Blend Requests:%u Current Blend Request:%u",
                    numberBlendRequests,
                    remainingBlendRequests,
                    currentBlendRequest);

    auto activeBlendParallelRequest         = 0;  // Active parallel blend request of: [0..(MaxActiveBlendRequests - 1)]
    auto currentState                       = BlendStageParallelRequestFSMState::Fail;
    auto updatedState                       = BlendStageParallelRequestFSMState::Fail;
    BOOL prefilterStageOutputBuffersAsInput = (0 == currentBlendRequest);
    BOOL releasePrefilterStageOutputBuffers = (0 == currentBlendRequest);

    for (updatedState = BlendStageParallelRequestFSMState::Init,
         currentState = updatedState;
         ((BlendStageParallelRequestFSMState::Done != currentState) &&
          (BlendStageParallelRequestFSMState::Fail != currentState));
         currentState = updatedState)
    {
        CHX_LOG_VERBOSE("Current FSM State: (%s (%d))",
                        BlendStageParallelRequestFSMStateAsString[static_cast<UINT>(currentState)],
                        currentState);

        switch (currentState)
        {
            case BlendStageParallelRequestFSMState::Init:
            {
                CHX_LOG_VERBOSE("FSM execution Started");

                CHX_LOG_VERBOSE("------------------------------< Mfnr blend stage >------------------------------");

                if (0 < remainingBlendRequests)
                {
                    numStageInputs            = 5;    // Rdi, BpsRegOut, PreFilter Full/DS4/DS4
                    numStageOutputs           = 2;    // Full, DS4
                    m_numExpectedStageBuffers = 0;

                    updatedState = BlendStageParallelRequestFSMState::PrepareAndSubmitRequest0;
                }
                else
                {
                    CHX_LOG_VERBOSE("Number of Blend Requests:%u ", numberBlendRequests);

                    updatedState = BlendStageParallelRequestFSMState::Exit;
                }

                break;
            }

            case BlendStageParallelRequestFSMState::PrepareAndSubmitRequest0:
            {
                activeBlendParallelRequest = 0;

                auto processingOrderIndex               = 0;
                auto previousActiveBlendParallelRequest = activeBlendParallelRequest;
                auto priorBlendParallelResultExpected   = FALSE;
                auto nextBlendParallelRequestExpected   = FALSE;

                CHX_LOG_VERBOSE("Active Blend Parallel Request:%u Previous Active Blend Parallel Request:%u",
                                activeBlendParallelRequest,
                                previousActiveBlendParallelRequest);

                processingOrderIndex = (currentBlendRequest + 1);

                ChxUtils::Memset(&m_blendStageResult, 0, sizeof(m_blendStageResult));

                DumpMetadata(pMfnrInputInfo->pChiMetadata[m_processingOrder[processingOrderIndex]],
                             GetUniqueStageName(m_processingOrder[processingOrderIndex], "mfnr_proc_"));

                m_pInterStageInputMetadata[m_metadataIndex]->Merge(*(pMfnrInputInfo->pChiMetadata[
                                                                m_processingOrder[processingOrderIndex]]), TRUE);

                feature2Mode = static_cast<UINT32>(ChiModeFeature2SubModeType::MFNRBlend);
                ChxUtils::FillTuningModeData(m_pInterStageInputMetadata[m_metadataIndex],
                                             pRequest,
                                             m_sensorModeIndex,
                                             m_pUsecase->GetEffectMode(),
                                             m_pUsecase->GetSceneMode(),
                                             m_pUsecase->GetFeature1Mode(),
                                             &feature2Mode);
                CHAR debugString[20];
                CdkUtils::SNPrintF(debugString, sizeof(debugString),
                                   "MFNR Blend #%u",
                                   currentBlendRequest);
                result = m_pUsecase->MergeDebugData(m_pInterStageInputMetadata[m_metadataIndex], &m_debugDataOffline,
                                                    debugString);
                if (CDKResultSuccess != result)
                {
                    CHX_LOG_INFO("Fail to merge DebugData");
                }

                PublishMFNRTotalFrames(m_pInterStageInputMetadata[m_metadataIndex]);

                CHX_LOG_VERBOSE("-----------------------< Mfnr blend stage (Prepare and Submit ParReq #%u) >-----------------------",
                                activeBlendParallelRequest);

                result = PrepareOfflineBlendStageParallelRequest(pMfnrInputInfo,
                                                                 processingOrderIndex,
                                                                 activeBlendParallelRequest,
                                                                 numStageInputs,
                                                                 inputBuffers[activeBlendParallelRequest],
                                                                 numStageOutputs,
                                                                 outputBuffers,
                                                                 &prefilterStageOutputBuffersAsInput);
                CHX_ASSERT(CDKResultSuccess == result);

                m_numExpectedStageBuffers += numStageOutputs;    // Full, DS4
                m_remainingBlendStageResults++;

                CHX_LOG_VERBOSE("Number of Stage Outputs:%u "
                                "Number of Expected Stage Buffers:%u "
                                "Remaining Blend Stage Results:%u",
                                numStageOutputs,
                                m_numExpectedStageBuffers,
                                m_remainingBlendStageResults);

                result = SubmitOfflineBlendStageParallelRequest(appFrameNumber,
                                                                activeBlendParallelRequest,
                                                                numStageInputs,
                                                                inputBuffers[activeBlendParallelRequest],
                                                                numStageOutputs,
                                                                outputBuffers,
                                                                priorBlendParallelResultExpected);
                CHX_ASSERT(CDKResultSuccess == result);

                remainingBlendRequests--;
                currentBlendRequest++;

                CHX_LOG_VERBOSE("Remaining Blend Requests:%u Current Blend Request:%u",
                                remainingBlendRequests,
                                currentBlendRequest);

                CHX_ASSERT(0 != remainingBlendRequests);
                CHX_ASSERT(currentBlendRequest < numberBlendRequests);

                updatedState = BlendStageParallelRequestFSMState::PrepareAndSubmitRequest1AndOrProcessRequest0;

                break;
            }

            case BlendStageParallelRequestFSMState::PrepareAndSubmitRequest1AndOrProcessRequest0:
            {
                PrepareAndSubmitNextRequestAndOrProcessCurrentRequest(
                    &activeBlendParallelRequest,
                    appFrameNumber,
                    pRequest,
                    pMfnrInputInfo,
                    inputBuffers,
                    outputBuffers,
                    numStageInputs,
                    numStageOutputs,
                    numberBlendRequests,
                    &remainingBlendRequests,
                    &currentBlendRequest,
                    BlendStageParallelRequestFSMState::PrepareAndSubmitRequest0AndOrProcessRequest1,
                    &updatedState,
                    &prefilterStageOutputBuffersAsInput,
                    &releasePrefilterStageOutputBuffers);

                break;
            }

            case BlendStageParallelRequestFSMState::PrepareAndSubmitRequest0AndOrProcessRequest1:
            {
                PrepareAndSubmitNextRequestAndOrProcessCurrentRequest(
                    &activeBlendParallelRequest,
                    appFrameNumber,
                    pRequest,
                    pMfnrInputInfo,
                    inputBuffers,
                    outputBuffers,
                    numStageInputs,
                    numStageOutputs,
                    numberBlendRequests,
                    &remainingBlendRequests,
                    &currentBlendRequest,
                    BlendStageParallelRequestFSMState::PrepareAndSubmitRequest1AndOrProcessRequest0,
                    &updatedState,
                    &prefilterStageOutputBuffersAsInput,
                    &releasePrefilterStageOutputBuffers);

                break;
            }

            case BlendStageParallelRequestFSMState::Exit:
            {
                CHX_LOG_VERBOSE("FSM execution completed");

                updatedState = BlendStageParallelRequestFSMState::Done;

                break;
            }

            default:
            {
                CHX_LOG_ERROR("FSM in a failed state(%s (%d))",
                              BlendStageParallelRequestFSMStateAsString[static_cast<UINT>(currentState)],
                              currentState);

                result = CDKResultEFailed;
                break;
            }
        }

        CHX_LOG_VERBOSE("Updated FSM State: (%s (%d))",
                        BlendStageParallelRequestFSMStateAsString[static_cast<UINT>(updatedState)],
                        updatedState);
    }

    /*/
    /// @note: Need to finalize this approach of releasing the RDI target buffers, instead of
    /// trying to release with each processed parallel request, which is resulting in a crash.
    /*/
    for (UINT8 rdiBufferIndex = 0; rdiBufferIndex < numberBlendRequests; rdiBufferIndex++)
    {
        if (FALSE == m_pUsecase->IsMultiCameraUsecase())
        {
            TargetBuffer* pTargetBuffer = m_pUsecase->GetTargetBufferPointer(pMfnrInputInfo->targetBufferQIdx);
            CHX_LOG("TargetBuffer:%p", pTargetBuffer);
            if (NULL != pTargetBuffer)
            {
                pTargetBuffer->pBufferManager->ReleaseReference(&pMfnrInputInfo->bufferArray[m_processingOrder[rdiBufferIndex + 1]].bufferInfo);
            }
            m_pUsecase->UpdateValidRDIBufferLength(pMfnrInputInfo->targetBufferQIdx, 1);
        }
        else
        {
            m_pUsecase->ReleaseSingleOffineInputResource(pRequest->frame_number,
                                                         m_activePipelineIndex,
                                                         (m_processingOrder[rdiBufferIndex + 1]));
        }
    }

    result = DeactivateOfflinePipeline(MfnrStageBlend);

    CHX_LOG_VERBOSE("X");

    return result;
}

VOID FeatureMFNR::DumpMFNRStageResult(
    MfnrStageResult& stageResult)
{
    const CHISTREAMBUFFER* pRefChiStreamBuffer = stageResult.refOutputBuffer;

    CHX_LOG_VERBOSE("Metadata: %p ", stageResult.pChiMetadata);

    for (int bufferIndex = 0; bufferIndex < FeatureMFNR::MfnrReferenceMax; bufferIndex++)
    {
        const CHISTREAMBUFFER* pChiStreamBuffer = &(pRefChiStreamBuffer[bufferIndex]);
        const ChiFence*        pChiAcquireFence = reinterpret_cast<ChiFence*>(pChiStreamBuffer->acquireFence.hChiFence);
        const ChiFence*        pChiReleaseFence = reinterpret_cast<ChiFence*>(pChiStreamBuffer->releaseFence.hChiFence);
        const CSLFence         pCslAcquireFence = ((NULL != pChiAcquireFence) ? pChiAcquireFence->hFence : -1);
        const CSLFence         pCslReleaseFence = ((NULL != pChiReleaseFence) ? pChiReleaseFence->hFence : -1);

        CHX_LOG_VERBOSE("reference Output Buffer[%d]: "
                        "Stream: %p Buffer: %p Buffer Type:%u Buffer Status: %d "
                        "Acquire Fence: %p(%d) Release Fence: %p(%d)",
                        bufferIndex,
                        pChiStreamBuffer->pStream,
                        pChiStreamBuffer->bufferInfo.phBuffer,
                        pChiStreamBuffer->bufferInfo.bufferType,
                        pChiStreamBuffer->bufferStatus,
                        pChiAcquireFence,
                        pCslAcquireFence,
                        pChiReleaseFence,
                        pCslReleaseFence);
    }

    return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SubmitOfflinePostfilterStageRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::SubmitOfflinePostfilterStageRequest(
    UINT32                     appFrameNumber,
    camera3_capture_request_t* pRequest,
    MFNRInputInfo*             pMfnrInputInfo)
{
    CDKResult       result           = CDKResultSuccess;
    CHISTREAMBUFFER inputBuffers[8]  = { };
    CHISTREAMBUFFER outputBuffers[8] = { };
    UINT32          numStageInputs   = 0;
    UINT32          numStageOutputs  = 0;
    UINT            outputIndex      = 0;
    UINT            inputIndex       = 0;
    UINT32          feature2Mode;

    CHX_LOG("-----------------------------< Mfnr post-filter stage >-----------------------------");

    numStageInputs                    = 5;                 // Rdi, BpsRegOut, PreFilter Full/DS4/DS4
    numStageOutputs                   = 1;                 // Full
    m_numExpectedStageBuffers         = numStageOutputs;   // Full
    outputIndex                       = 0;
    inputIndex                        = 0;

    m_remainingPostFilterStageResults = 1;

    ChxUtils::Memset(&m_postFilterStageResult, 0, sizeof(m_postFilterStageResult));

    ChxUtils::Memcpy(&inputBuffers[inputIndex++],
                     &(pMfnrInputInfo->bufferArray[m_processingOrder[m_mfnrTotalNumFrames - 1]]),
                     sizeof(CHISTREAMBUFFER));

    ChxUtils::Memcpy(&inputBuffers[inputIndex],
                     &m_preFilterAnchorFrameRegResultBuffer,
                     sizeof(CHISTREAMBUFFER));
    inputBuffers[inputIndex].acquireFence.valid = FALSE;
    inputBuffers[inputIndex].size               = sizeof(CHISTREAMBUFFER);
    inputBuffers[inputIndex++].pStream          = m_pMfnrBpsRegInStream;

    ChxUtils::Memcpy(&inputBuffers[inputIndex],
                     &m_preFilterStageResult.refOutputBuffer[MfnrReferenceFull],
                     sizeof(CHISTREAMBUFFER));
    inputBuffers[inputIndex].acquireFence.valid = FALSE;
    inputBuffers[inputIndex].size               = sizeof(CHISTREAMBUFFER);
    inputBuffers[inputIndex++].pStream          = m_pBlendInStream[MfnrReferenceFull];

    ChxUtils::Memcpy(&inputBuffers[inputIndex],
                     &m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4],
                     sizeof(CHISTREAMBUFFER));
    inputBuffers[inputIndex].acquireFence.valid = FALSE;
    inputBuffers[inputIndex].size               = sizeof(CHISTREAMBUFFER);
    inputBuffers[inputIndex++].pStream          = m_pBlendInStream[MfnrReferenceDS4];

    ChxUtils::Memcpy(&inputBuffers[inputIndex],
                     &m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4],
                     sizeof(CHISTREAMBUFFER));
    inputBuffers[inputIndex].acquireFence.valid = FALSE;
    inputBuffers[inputIndex].size               = sizeof(CHISTREAMBUFFER);
    inputBuffers[inputIndex++].pStream          = m_pScaleInStream[MfnrReferenceDS4];

    if (TRUE == m_noiseReprocessEnable)
    {
        GetOutputBuffer(m_pMfnrBufferManager[MfnrStagePrefilter][MfnrReferenceFull],
            m_pNoiseReprocessInStream,
            &outputBuffers[outputIndex++]);
    }
    else
    {
        if (TRUE == IsJPEGSnapshotConfigured())
        {
            GetOutputBuffer(m_pMfnrBufferManager[MfnrStagePrefilter][MfnrReferenceFull],
                m_pMfnrPostFilterOutStream,
                &outputBuffers[outputIndex++]);
        }
        else
        {
            // @todo Change this, once Feature ECR accepts ChiRequest structure instead of framework request structure.
            m_snapshotBuffers[m_maxSnapshotReqId % MaxOutstandingRequests][0].bufferInfo.bufferType = ChiNative;
            // Update postfilter output to YUV snapshot output buffer.
            outputBuffers[outputIndex++] = m_snapshotBuffers[m_maxSnapshotReqId % MaxOutstandingRequests][0];
        }
    }

    DumpMetadata(pMfnrInputInfo->pChiMetadata[m_processingOrder[m_mfnrTotalNumFrames - 1]],
                 GetUniqueStageName(m_processingOrder[m_mfnrTotalNumFrames - 1], "mfnr_proc_"));

    m_pInterStageInputMetadata[m_metadataIndex]->Merge(*(pMfnrInputInfo->pChiMetadata[
                                                         m_processingOrder[m_mfnrTotalNumFrames - 1]]), TRUE);

    feature2Mode = static_cast<UINT32>(ChiModeFeature2SubModeType::MFNRPostFilter);
    ChxUtils::FillTuningModeData(m_pInterStageInputMetadata[m_metadataIndex],
                                 pRequest,
                                 m_sensorModeIndex,
                                 m_pUsecase->GetEffectMode(),
                                 m_pUsecase->GetSceneMode(),
                                 m_pUsecase->GetFeature1Mode(),
                                 &feature2Mode);

    result = m_pUsecase->MergeDebugData(m_pInterStageInputMetadata[m_metadataIndex], &m_debugDataOffline, "MFNR Post-filter");
    if (CDKResultSuccess != result)
    {
        CHX_LOG_INFO("Fail to merge DebugData");
    }
    // Copy post-proc data to DebugData used in JPEG
    ChxUtils::Memcpy(m_debugDataOfflineSnapshot.pData, m_debugDataOffline.pData, m_debugDataOffline.size);

    PublishMFNRTotalFrames(m_pInterStageInputMetadata[m_metadataIndex]);

    m_resultsAvailable = FALSE;

    result = ExecuteMfnrRequest(MfnrStagePostfilter,
                               appFrameNumber,
                               numStageOutputs,
                               &outputBuffers[0],
                               numStageInputs,
                               &inputBuffers[0],
                               m_pInterStageInputMetadata[m_metadataIndex],
                               m_pInterStageOutputMetadata[m_metadataIndex]);

    if (CDKResultSuccess != result)
    {
        CHX_LOG_ERROR("!!!!!!! ExecuteMfnrRequest return failed: %d", result);
    }

    ++m_metadataIndex;

    m_pMfnrResultMutex->Lock();

    // Wait for all post-prefilter scale results to come back - 2 output buffers: DS16, DS64, and Metadata
    while (FALSE == m_resultsAvailable && result == CDKResultSuccess)
    {
        m_pMfnrResultAvailable->Wait(m_pMfnrResultMutex->GetNativeHandle());
    }

    if (TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress)))
    {
        m_resultsAvailable = FALSE;
        m_pMfnrResultMutex->Unlock();

        CHX_LOG_VERBOSE("MFNR snapshot return because of cleanup");
        return CDKResultSuccess;
    }

    // Release reference to prefilter output buffers
    CHX_LOG_VERBOSE("ReleaseReference of MfnrReferenceFull preFilterStage buffer[%u]:%p Hostptr=%p",
                    MfnrReferenceFull,
                    m_preFilterStageResult.refOutputBuffer[MfnrReferenceFull].bufferInfo.phBuffer,
                    CHIBufferManager::GetCPUAddress(&m_preFilterStageResult.refOutputBuffer[MfnrReferenceFull].bufferInfo, 0));

    m_pMfnrBufferManager[MfnrStagePrefilter][MfnrReferenceFull]->ReleaseReference(
        &(m_preFilterStageResult.refOutputBuffer[MfnrReferenceFull].bufferInfo));

    CHX_LOG_VERBOSE("ReleaseReference of MfnrReferenceDS4 preFilterStage buffer[%u]:%p Hostptr=%p",
                    MfnrReferenceDS4,
                    m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4].bufferInfo.phBuffer,
                    CHIBufferManager::GetCPUAddress(&m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4].bufferInfo, 0));

    m_pMfnrBufferManager[MfnrStagePrefilter][MfnrReferenceDS4]->ReleaseReference(
        &(m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4].bufferInfo));

    m_pMfnrBpsRegOutBufferManager->ReleaseReference(
        &m_preFilterAnchorFrameRegResultBuffer.bufferInfo);

    if (FALSE == m_pUsecase->IsMultiCameraUsecase())
    {
        TargetBuffer* pTargetBuffer = m_pUsecase->GetTargetBufferPointer(pMfnrInputInfo->targetBufferQIdx);
        CHX_LOG("TargetBuffer:%p", pTargetBuffer);
        if (NULL != pTargetBuffer)
        {
            pTargetBuffer->pBufferManager->ReleaseReference(
                &pMfnrInputInfo->bufferArray[m_processingOrder[m_mfnrTotalNumFrames - 1]].bufferInfo);
        }
        m_pUsecase->UpdateValidRDIBufferLength(pMfnrInputInfo->targetBufferQIdx, 1);
    }
    else
    {
        m_pUsecase->ReleaseSingleOffineInputResource(
                            pRequest->frame_number,
                            m_activePipelineIndex,
                            (m_processingOrder[m_mfnrTotalNumFrames - 1]));
    }
    if (FALSE == IsJPEGSnapshotConfigured())
    {
        if (FALSE == m_pUsecase->IsMultiCameraUsecase())
        {
            TargetBuffer* pTargetBuffer = m_pUsecase->GetTargetBufferPointer(pMfnrInputInfo->targetBufferQIdx);
            CHX_LOG("TargetBuffer:%p", pTargetBuffer);
            if (NULL != pTargetBuffer)
            {
                pTargetBuffer->pBufferManager->ReleaseReference(
                    &pMfnrInputInfo->bufferArray[m_processingOrder[0]].bufferInfo);
            }
            m_pUsecase->UpdateValidRDIBufferLength(pMfnrInputInfo->targetBufferQIdx, 1);
        }
        else
        {
            m_pUsecase->ReleaseSingleOffineInputResource(
                                pRequest->frame_number,
                                m_activePipelineIndex,
                                (m_processingOrder[0]));
        }
    }

    m_resultsAvailable = FALSE;

    m_pMfnrResultMutex->Unlock();

    if (CDKResultSuccess != result)
    {
        DeactivateOfflinePipeline(MfnrStagePostfilter);
    }
    else
    {
        result = DeactivateOfflinePipeline(MfnrStagePostfilter);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SubmitOfflineSnapshotStageRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::SubmitOfflineSnapshotStageRequest(
    UINT32                     appFrameNumber,
    camera3_capture_request_t* pRequest,
    MFNRInputInfo*             pMfnrInputInfo)
{
    (VOID)pRequest;
    (VOID)pMfnrInputInfo;

    CDKResult  result          = CDKResultSuccess;
    UINT32     numStageInputs  = 0;
    UINT32     numStageOutputs = 0;

    CHISTREAMBUFFER* pInputBuffers = NULL;

    CHX_LOG("-----------------------------< Mfnr snapshot stage >-----------------------------");

    m_pInterStageInputMetadata[m_metadataIndex]->Merge(*(m_pInterStageOutputMetadata[m_metadataIndex-1]), TRUE);
    m_pUsecase->UpdateSnapshotMetadataWithRDITags(*(pMfnrInputInfo->pChiMetadata[m_processingOrder[0]]),
        *(m_pInterStageInputMetadata[m_metadataIndex]));

    result = m_pUsecase->MergeDebugData(m_pInterStageInputMetadata[m_metadataIndex], &m_debugDataOfflineSnapshot,
                                        "MFNR Snapshot");
    if (CDKResultSuccess != result)
    {
        CHX_LOG_INFO("Fail to merge DebugData");
    }

    numStageInputs                  = 1;
    numStageOutputs                 = 1;
    m_remainingSnapshotStageResults = 1;
    m_numExpectedStageBuffers       = numStageOutputs;

    if (TRUE == m_noiseReprocessEnable)
    {
        pInputBuffers          =
            static_cast<CHISTREAMBUFFER*>(&m_noiseReprocessStageResult.refOutputBuffer[MfnrReferenceFull]);
        pInputBuffers->pStream = m_pJPEGInputStream;
    }
    else
    {
        pInputBuffers          =
            static_cast<CHISTREAMBUFFER*>(&m_postFilterStageResult.refOutputBuffer[MfnrReferenceFull]);
        pInputBuffers->pStream = m_pJPEGInputStream;
    }

    m_resultsAvailable = FALSE;

    result = ExecuteMfnrRequest(MfnrStageSnapshot,
                               appFrameNumber,
                               numStageOutputs,
                               &m_snapshotBuffers[m_maxSnapshotReqId % MaxOutstandingRequests][0],
                               numStageInputs,
                               pInputBuffers,
                               m_pInterStageInputMetadata[m_metadataIndex],
                               m_pInterStageOutputMetadata[m_metadataIndex]);

    if (CDKResultSuccess != result)
    {
        CHX_LOG_ERROR("!!!!!!! ExecuteMfnrRequest return failed: %d", result);
    }

    m_pMfnrResultMutex->Lock();

    // Wait for all snapshot stage results
    while (FALSE == m_resultsAvailable && result == CDKResultSuccess)
    {
        m_pMfnrResultAvailable->Wait(m_pMfnrResultMutex->GetNativeHandle());
    }

    if (TRUE == m_noiseReprocessEnable)
    {
        m_pOfflineNoiseReprocessBufferManager->ReleaseReference(&(pInputBuffers->bufferInfo));
        m_pOfflineNoiseReprocessBufferManager->Deactivate(TRUE);
    }
    else
    {
        m_pMfnrBufferManager[MfnrStagePrefilter][MfnrReferenceFull]->ReleaseReference(
            &(pInputBuffers->bufferInfo));
    }
    if (TRUE == IsJPEGSnapshotConfigured())
    {
        if (FALSE == m_pUsecase->IsMultiCameraUsecase())
        {
            TargetBuffer* pTargetBuffer = m_pUsecase->GetTargetBufferPointer(pMfnrInputInfo->targetBufferQIdx);
            CHX_LOG("TargetBuffer:%p", pTargetBuffer);
            if (NULL != pTargetBuffer)
            {
                pTargetBuffer->pBufferManager->ReleaseReference(
                    &pMfnrInputInfo->bufferArray[m_processingOrder[0]].bufferInfo);
            }
            m_pUsecase->UpdateValidRDIBufferLength(pMfnrInputInfo->targetBufferQIdx, 1);
        }
        else
        {
            m_pUsecase->ReleaseSingleOffineInputResource(
                                pRequest->frame_number,
                                m_activePipelineIndex,
                                (m_processingOrder[0]));
        }
    }

    // Free all MFNR buffers to reduce memory
    m_pMfnrBufferManager[MfnrStagePrefilter][MfnrReferenceFull]->Deactivate(TRUE);
    m_pMfnrBufferManager[MfnrStagePrefilter][MfnrReferenceDS4]->Deactivate(TRUE);
    m_pMfnrBpsRegOutBufferManager->Deactivate(TRUE);

    m_resultsAvailable = FALSE;
    m_isLLSSnapshot    = FALSE;

    m_pMfnrResultMutex->Unlock();

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SubmitOfflineNoiseReprocessStageRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::SubmitOfflineNoiseReprocessStageRequest(
    UINT32                     appFrameNumber,
    camera3_capture_request_t* pRequest,
    MFNRInputInfo*             pMfnrInputInfo)
{
    (VOID)pRequest;
    (VOID)pMfnrInputInfo;

    CDKResult            result             = CDKResultSuccess;
    UINT                 inputIndex         = 0;
    CHISTREAMBUFFER      inputBuffer        = { 0 };
    CHISTREAMBUFFER      outputBuffer       = { 0 };
    UINT32               numStageInputs     = 1;
    UINT32               numStageOutputs    = 1;
    UINT32               feature2Mode;

    CHX_LOG("------------------------------< Offline Noise Reprocess Stage >------------------------------");

    m_remainingNoiseReprocessStageResults = 1;
    m_numExpectedStageBuffers             = numStageOutputs;

    ChxUtils::Memcpy(&inputBuffer, &m_postFilterStageResult.refOutputBuffer[MfnrReferenceFull], sizeof(CHISTREAMBUFFER));

    inputBuffer.acquireFence.valid = FALSE;
    inputBuffer.size               = sizeof(CHISTREAMBUFFER);
    inputBuffer.pStream            = m_pNoiseReprocessInStream;

    if (TRUE == IsJPEGSnapshotConfigured())
    {
        GetOutputBuffer(m_pOfflineNoiseReprocessBufferManager,
                m_pNoiseReprocessOutStream, &outputBuffer);
    }
    else
    {
        m_snapshotBuffers[m_maxSnapshotReqId % MaxOutstandingRequests][0].bufferInfo.bufferType = ChiNative;

        outputBuffer = m_snapshotBuffers[m_maxSnapshotReqId % MaxOutstandingRequests][0];
    }

    m_resultsAvailable = FALSE;

    m_pInterStageInputMetadata[m_metadataIndex]->Merge(*m_pInterStageOutputMetadata[m_metadataIndex - 1], TRUE);

    feature2Mode = static_cast<UINT32>(ChiModeFeature2SubModeType::OfflineNoiseReprocess);
    ChxUtils::FillTuningModeData(m_pInterStageInputMetadata[m_metadataIndex],
        pRequest,
        m_sensorModeIndex,
        m_pUsecase->GetEffectMode(),
        m_pUsecase->GetSceneMode(),
        m_pUsecase->GetFeature1Mode(),
        &feature2Mode);

    result = m_pUsecase->MergeDebugData(m_pInterStageInputMetadata[m_metadataIndex], &m_debugDataOffline,
                                        "MFNR Noise Repocess");
    if (CDKResultSuccess != result)
    {
        CHX_LOG_INFO("Fail to merge DebugData");
    }

    // Disable zoom crop for offline NoiseReprocess pipeline to avoid double crop/upscale for the same zoom window
    PublishDisableZoomCrop(m_pInterStageInputMetadata[m_metadataIndex]);

    result = SubmitOfflineNoiseReprocessRequest(appFrameNumber,
                                                numStageInputs,
                                                &inputBuffer,
                                                numStageOutputs,
                                                &outputBuffer,
                                                m_pInterStageInputMetadata[m_metadataIndex],
                                                m_pInterStageOutputMetadata[m_metadataIndex]);

    m_metadataIndex++;
    m_pMfnrResultMutex->Lock();

    // Wait for all snapshot stage results
    while (FALSE == m_resultsAvailable)
    {
        m_pMfnrResultAvailable->Wait(m_pMfnrResultMutex->GetNativeHandle());
    }

    m_pMfnrBufferManager[MfnrStagePrefilter][MfnrReferenceFull]->ReleaseReference(
        &m_postFilterStageResult.refOutputBuffer[MfnrReferenceFull].bufferInfo);

    m_resultsAvailable = FALSE;

    m_pMfnrResultMutex->Unlock();

    result = DeactivateOfflineNoiseReprocessPipeline();

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::SubmitOfflineNoiseReprocessStageRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::SubmitOfflineNoiseReprocessRequest(
    UINT32              frameNumber,
    UINT32&             numInputs,
    CHISTREAMBUFFER*    pInputBuffer,
    UINT32&             numOutputs,
    CHISTREAMBUFFER*    pOutputBuffer,
    ChiMetadata*        pChiInputMetadata,
    ChiMetadata*        pChiOutputMetadata)
{
    CDKResult result                 = CDKResultSuccess;

    CHICAPTUREREQUEST  captureRequest = { 0 };
    CHIPIPELINEREQUEST submitRequest  = { 0 };

    const Session* pSession      = m_pUsecase->GetSessionData(m_offlineNoiseReprocess)->pSession;

    PipelineData*  pPipelineData = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_offlineNoiseReprocess, 0));

    UINT requestIdIndex = (pPipelineData->seqId % MaxOutstandingRequests);
    pPipelineData->seqIdToFrameNum[requestIdIndex] = frameNumber;

    captureRequest.frameNumber              = pPipelineData->seqId++;
    captureRequest.hPipelineHandle          = reinterpret_cast<CHIPIPELINEHANDLE>(pSession->GetPipelineHandle());
    captureRequest.numInputs                = numInputs;
    captureRequest.numOutputs               = numOutputs;
    captureRequest.pInputBuffers            = pInputBuffer;
    captureRequest.pOutputBuffers           = pOutputBuffer;
    captureRequest.pInputMetadata           = pChiInputMetadata->GetHandle();
    captureRequest.pOutputMetadata          = pChiOutputMetadata->GetHandle();

    captureRequest.pPrivData                = &m_offlineNoiseReprocessPrivData[captureRequest.frameNumber % MaxOutstandingRequests];
    captureRequest.pPrivData->featureType   = FeatureType::MFNR;
    captureRequest.pPrivData->streamIndex   = m_physicalCameraIndex;

    submitRequest.pSessionHandle            = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
    submitRequest.numRequests               = 1;
    submitRequest.pCaptureRequests          = &captureRequest;

    CHX_LOG_INFO("Sending Offline reprocessing request application-frameNumber: %d, sessionId: %d, session-requestId: %d,",
        frameNumber,
        m_offlineNoiseReprocess,
        (UINT)captureRequest.frameNumber);

    result = ActivateOfflineNoiseReprocessPipeline();

    m_pUsecase->SetRequestToFeatureMapping(m_offlineNoiseReprocess, captureRequest.frameNumber, this);

    m_pUsecase->LogFeatureRequestMappings(frameNumber, captureRequest.frameNumber, "MFNR Noise Reprocess Request");
    result = ExtensionModule::GetInstance()->SubmitRequest(&submitRequest);
    CHX_ASSERT(CDKResultSuccess == result);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::ExecuteMfnrRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::ExecuteMfnrRequest(
    MFNRStage           pipelineStage,
    UINT32              frameNumber,
    UINT32              numOutputs,
    CHISTREAMBUFFER*    pOutputBuffers,
    UINT32              numInputs,
    CHISTREAMBUFFER*    pInputBuffers,
    ChiMetadata*        pChiInputMetadata,
    ChiMetadata*        pChiOutputMetadata)
{
    CDKResult          result         = CDKResultSuccess;
    CHIPIPELINEREQUEST submitRequest  = { 0 };
    CHICAPTUREREQUEST  captureRequest = { 0 };
    const Session*     pSession       = m_pUsecase->GetSessionData(m_offline)->pSession;
    PipelineData*      pPipelineData  = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_offline,
                                                                                  GetPipelineIndex(pipelineStage)));
    UINT               requestIdIndex = (pPipelineData->seqId % MaxOutstandingRequests);

    pPipelineData->seqIdToFrameNum[requestIdIndex] = frameNumber;

    captureRequest.frameNumber            = pPipelineData->seqId++;
    captureRequest.hPipelineHandle        = reinterpret_cast<CHIPIPELINEHANDLE>(
                                                pSession->GetPipelineHandle(GetPipelineIndex(pipelineStage)));
    captureRequest.numInputs              = numInputs;
    captureRequest.numOutputs             = numOutputs;
    captureRequest.pInputBuffers          = pInputBuffers;
    captureRequest.pOutputBuffers         = pOutputBuffers;
    captureRequest.pInputMetadata         = pChiInputMetadata->GetHandle();
    captureRequest.pOutputMetadata        = pChiOutputMetadata->GetHandle();
    captureRequest.pPrivData              = &m_offlinePrivData[captureRequest.frameNumber % MaxOutstandingRequests];
    captureRequest.pPrivData->featureType = FeatureType::MFNR;
    captureRequest.pPrivData->streamIndex = m_physicalCameraIndex;
    submitRequest.pSessionHandle          = reinterpret_cast<CHIHANDLE>(pSession->GetSessionHandle());
    submitRequest.numRequests             = 1;
    submitRequest.pCaptureRequests        = &captureRequest;

    CHX_LOG_INFO("Sending MFNR request application-frameNumber: %d, sessionId: %d, session-requestId: %d, pipeline:%s,"
        "numInputs:%d,inputstream:%p, m_physicalCameraIndex %d",
        frameNumber,
        m_offline,
        (UINT)captureRequest.frameNumber,
        pPipelineData->pPipeline->GetPipelineName(),
        numInputs,
        pInputBuffers->pStream,
        m_physicalCameraIndex);

    result = ActivateOfflinePipeline(pipelineStage);

    m_pUsecase->SetRequestToFeatureMapping(m_offline, captureRequest.frameNumber, this);

    auto stageToIdentifierString = [](MFNRStage stage) {
        switch (stage) {
            case MfnrStagePrefilter:      return "MFNR Offline Prefilter Request";
            case MfnrStageBlend:          return "MFNR Offline Blend Request";
            case MfnrStagePostfilter:     return "MFNR Offline Postfilter Request";
            case MfnrStageSnapshot:       return "MFNR Offline Snapshot Request";
            case MfnrStageNoiseReprocess: return "MFNR Offline Noise Reprocess Request";
            default:                      return "MFNR Request";
        }
    };

    m_pUsecase->LogFeatureRequestMappings(frameNumber, captureRequest.frameNumber, stageToIdentifierString(pipelineStage));

    result = m_pUsecase->SubmitRequest(&submitRequest);
    CHX_ASSERT(CDKResultSuccess == result);

    return result;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::ProcessPreFilterStageResults
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::ProcessPreFilterStageResults(
    const CHISTREAMBUFFER* pChiStreamBuffer)
{
    CHX_LOG_INFO("m_numExpectedStageBuffers:%u", m_numExpectedStageBuffers);

    // There will be 3 buffers per prefilter stage results - Full, DS4, BpRegOut
    if (m_pPrefilterOutStream[MfnrReferenceFull] == pChiStreamBuffer->pStream)
    {
        CHX_LOG_INFO("MFNR-SNAPSHOT: Received FULL output buffer");

        ChxUtils::Memcpy(&m_preFilterStageResult.refOutputBuffer[MfnrReferenceFull],
                         pChiStreamBuffer,
                         ChiStreamBufferSize);
        m_preFilterStageResult.refOutputBuffer[MfnrReferenceFull].size = ChiStreamBufferSize;
        m_numExpectedStageBuffers--;

        CHX_LOG_VERBOSE("MfnrReferenceFull :: preFilterStage result buffer[%u]:%p (Copy) Hostptr=%p",
                        MfnrReferenceFull,
                        m_preFilterStageResult.refOutputBuffer[MfnrReferenceFull].bufferInfo.phBuffer,
                        CHIBufferManager::GetCPUAddress(&m_preFilterStageResult.refOutputBuffer[MfnrReferenceFull].bufferInfo, 0));
    }
    else if (m_pPrefilterOutStream[MfnrReferenceDS4] == pChiStreamBuffer->pStream)
    {
        CHX_LOG_INFO("MFNR-SNAPSHOT: Received DS4 output buffer");

        ChxUtils::Memcpy(&m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4],
                         pChiStreamBuffer,
                         ChiStreamBufferSize);
        m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4].size = ChiStreamBufferSize;
        m_numExpectedStageBuffers--;

        CHX_LOG_VERBOSE("MfnrReferenceDS4 :: preFilterStage result buffer[%u]:%p (Copy) Hostptr=%p",
                        MfnrReferenceDS4,
                        m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4].bufferInfo.phBuffer,
                        CHIBufferManager::GetCPUAddress(&m_preFilterStageResult.refOutputBuffer[MfnrReferenceDS4].bufferInfo, 0));
    }
    else if (m_pMfnrBpsRegOutStream == pChiStreamBuffer->pStream)
    {
        CHX_LOG_INFO("MFNR-SNAPSHOT: Received BPS Reg output buffer");

        ChxUtils::Memcpy(&m_preFilterAnchorFrameRegResultBuffer,
                         pChiStreamBuffer,
                         ChiStreamBufferSize);
        m_preFilterAnchorFrameRegResultBuffer.size = ChiStreamBufferSize;
        m_numExpectedStageBuffers--;
    }

    if (0 == m_numExpectedStageBuffers)
    {
        CHX_LOG_INFO("MFNR-SNAPSHOT: All Prefilter stage output buffers received");

        m_remainingPrefilterStageResults--;

        m_pMfnrResultMutex->Lock();

        m_resultsAvailable = TRUE;
        m_pMfnrResultAvailable->Signal();

        m_pMfnrResultMutex->Unlock();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::ProcessBlendStageResults
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::ProcessBlendStageResults(
    const CHISTREAMBUFFER* pChiStreamBuffer)
{
    const UINT32 numStageOutputs = 2; // Full, DS4

    CHX_LOG_VERBOSE("m_numExpectedStageBuffers:%u", m_numExpectedStageBuffers);

    if (m_pBlendOutStream[MfnrReferenceFull] == pChiStreamBuffer->pStream)
    {
        CHX_LOG_INFO("MFNR-SNAPSHOT: Received Blend Full output buffer");

        ChxUtils::Memcpy(&m_blendStageResult.refOutputBuffer[MfnrReferenceFull],
                         pChiStreamBuffer,
                         ChiStreamBufferSize);
        m_blendStageResult.refOutputBuffer[MfnrReferenceFull].size = ChiStreamBufferSize;
        m_numExpectedStageBuffers--;

        CHX_LOG_VERBOSE("MfnrReferenceFull :: blendStageResult result buffer[%u]:%p (Copy) Hostptr=%p",
                        MfnrReferenceFull,
                        m_blendStageResult.refOutputBuffer[MfnrReferenceFull].bufferInfo.phBuffer,
                        CHIBufferManager::GetCPUAddress(&m_blendStageResult.refOutputBuffer[MfnrReferenceFull].bufferInfo, 0));
    }
    else if (m_pBlendOutStream[MfnrReferenceDS4] == pChiStreamBuffer->pStream)
    {
        CHX_LOG_INFO("MFNR-SNAPSHOT: Received Blend DS4 output buffer");

        ChxUtils::Memcpy(&m_blendStageResult.refOutputBuffer[MfnrReferenceDS4],
                         pChiStreamBuffer,
                         ChiStreamBufferSize);
        m_blendStageResult.refOutputBuffer[MfnrReferenceDS4].size = ChiStreamBufferSize;
        m_numExpectedStageBuffers--;

        CHX_LOG_VERBOSE("MfnrReferenceDS4 :: blendStageResult result buffer[%u]:%p (Copy) Hostptr=%p",
                        MfnrReferenceDS4,
                        m_blendStageResult.refOutputBuffer[MfnrReferenceDS4].bufferInfo.phBuffer,
                        CHIBufferManager::GetCPUAddress(&m_blendStageResult.refOutputBuffer[MfnrReferenceDS4].bufferInfo, 0));
    }

    CHX_LOG_VERBOSE("m_numExpectedStageBuffers:%u", m_numExpectedStageBuffers);
    if (0 == (m_numExpectedStageBuffers % numStageOutputs))
    {
        m_pMfnrResultMutex->Lock();

        m_resultsAvailable = TRUE;

        CHX_LOG_VERBOSE("m_remainingBlendStageResults:%u", m_remainingBlendStageResults);

        CHX_LOG_VERBOSE("m_pMfnrResultAvailable->Signal() for blend stage results");
        m_pMfnrResultAvailable->Signal();

        m_remainingBlendStageResults--;
        CHX_LOG_VERBOSE("m_remainingBlendStageResults:%u", m_remainingBlendStageResults);

        if (0 == m_numExpectedStageBuffers)
        {
            CHX_LOG_INFO("MFNR-SNAPSHOT: All Blend stage output buffers received");
        }

        m_pMfnrResultMutex->Unlock();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::ProcessPostFilterStageResults
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::ProcessPostFilterStageResults(
    const CHISTREAMBUFFER* pChiStreamBuffer)
{
    CHX_LOG_VERBOSE("m_numExpectedStageBuffers:%u", m_numExpectedStageBuffers);

    CHX_LOG_INFO("MFNR-SNAPSHOT: Received Postfilter Full output buffer");

    ChxUtils::Memcpy(&m_postFilterStageResult.refOutputBuffer[MfnrReferenceFull],
                     pChiStreamBuffer,
                     ChiStreamBufferSize);
    m_postFilterStageResult.refOutputBuffer[MfnrReferenceFull].size = ChiStreamBufferSize;
    m_numExpectedStageBuffers--;

    if (0 == m_numExpectedStageBuffers)
    {
        CHX_LOG_INFO("MFNR-SNAPSHOT: All Postfilter stage output buffers received, IHDR enable: %u", m_isIHDRSnapshot);

        m_remainingPostFilterStageResults--;

        m_pMfnrResultMutex->Lock();

        m_resultsAvailable = TRUE;
        m_pMfnrResultAvailable->Signal();

        m_pMfnrResultMutex->Unlock();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::ProcessSnapshotStageResults
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::ProcessSnapshotStageResults(
    const CHISTREAMBUFFER*    pChiStreamBuffer,
    camera3_capture_result_t* pUsecaseResult)
{
    camera3_stream_buffer_t* pResultBuffer = NULL;

    CHX_LOG_VERBOSE("m_numExpectedStageBuffers:%u", m_numExpectedStageBuffers);

    CHX_LOG_INFO("MFNR-SNAPSHOT: Received Snapshot output buffer");
    CHX_LOG_INFO("MFNR-SNAPSHOT: Received Final Snapshot(JPEG) stream, %d",
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

    pResultBuffer = const_cast<camera3_stream_buffer_t*>(
                        &(pUsecaseResult->output_buffers[pUsecaseResult->num_output_buffers++]));

    ChxUtils::PopulateChiToHALStreamBuffer(pChiStreamBuffer, pResultBuffer);

    m_pUsecase->GetAppResultMutex()->Unlock();

    m_numExpectedStageBuffers--;

    if (0 == m_numExpectedStageBuffers)
    {
        CHX_LOG_INFO("MFNR-SNAPSHOT: snapshot stage output buffers received");
            m_remainingSnapshotStageResults--;
            m_pMfnrResultMutex->Lock();

            m_resultsAvailable = TRUE;
            m_pMfnrResultAvailable->Signal();

            m_pMfnrResultMutex->Unlock();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::ProcessOfflineNoiseReprocessResults
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::ProcessOfflineNoiseReprocessResults(
    const CHISTREAMBUFFER* pChiStreamBuffer)
{
    CHX_LOG_VERBOSE("m_numExpectedStageBuffers:%u", m_numExpectedStageBuffers);

    CHX_LOG_INFO("MFNR-SNAPSHOT OfflineNoiseReprocess: Received Offline Noise Reprocess output buffer");

    ChxUtils::Memcpy(&m_noiseReprocessStageResult.refOutputBuffer[MfnrReferenceFull],
        pChiStreamBuffer,
        ChiStreamBufferSize);

    m_noiseReprocessStageResult.refOutputBuffer[MfnrReferenceFull].size = ChiStreamBufferSize;
    m_numExpectedStageBuffers--;

    if (0 == m_numExpectedStageBuffers)
    {
        CHX_LOG_INFO("MFNR-SNAPSHOT OfflineNoiseReprocess: All Reprocess stage output buffers received");

        m_remainingNoiseReprocessStageResults--;

        m_pMfnrResultMutex->Lock();

        m_resultsAvailable = TRUE;
        m_pMfnrResultAvailable->Signal();

        m_pMfnrResultMutex->Unlock();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::ProcessNormalResults
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::ProcessNormalResults(
    const CHISTREAMBUFFER*    pChiStreamBuffer,
    camera3_capture_result_t* pUsecaseResult)
{
    camera3_stream_buffer_t* pResultBuffer = NULL;

    // queue a buffer as part of the normal result
    m_pUsecase->GetAppResultMutex()->Lock();

    pResultBuffer = const_cast<camera3_stream_buffer_t*>(
                        &(pUsecaseResult->output_buffers[pUsecaseResult->num_output_buffers++]));

    ChxUtils::PopulateChiToHALStreamBuffer(pChiStreamBuffer, pResultBuffer);

    m_pUsecase->GetAppResultMutex()->Unlock();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::ProcessResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::ProcessResult(
    CHICAPTURERESULT*   pResult,
    VOID*               pPrivateCallbackData)
{
    if ((TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress))) ||
        (FlushStatus::NotFlushing != m_pUsecase->GetFlushStatus()))
    {
        CHX_LOG_INFO("MFNR process result return because of cleanup");
        return;
    }

    SessionPrivateData* pCbData               = static_cast<SessionPrivateData*>(pPrivateCallbackData);
    BOOL                isAppResultsAvailable = FALSE;
    UINT32              resultFrameNum        = pResult->frameworkFrameNum;
    UINT32              rtPipelineReqId       = 0;

    if (TRUE == m_pUsecase->IsMultiCameraUsecase())
    {
        RequestMapInfo requestMapInfo = m_pUsecase->GetRequestMapInfo(m_internalRequestId);

        resultFrameNum = requestMapInfo.frameNumber;
    }

    if (m_offline == pCbData->sessionId)
    {
        const PipelineData* pPipelineData = NULL;

        if (m_remainingPrefilterStageResults > 0)
        {
            pPipelineData = m_pUsecase->GetPipelineData(pCbData->sessionId, GetPipelineIndex(MfnrStagePrefilter));
        }
        if (m_remainingBlendStageResults > 0)
        {
            pPipelineData = m_pUsecase->GetPipelineData(pCbData->sessionId, GetPipelineIndex(MfnrStageBlend));
        }
        if (m_remainingPostFilterStageResults > 0)
        {
            pPipelineData = m_pUsecase->GetPipelineData(pCbData->sessionId, GetPipelineIndex(MfnrStagePostfilter));
        }
        if (m_remainingSnapshotStageResults > 0)
        {
            pPipelineData = m_pUsecase->GetPipelineData(pCbData->sessionId, GetPipelineIndex(MfnrStageSnapshot));
        }

        if (NULL != pPipelineData)
        {
            resultFrameNum  = pPipelineData->seqIdToFrameNum[pResult->frameworkFrameNum % MaxOutstandingRequests];
        }
    }
    else if (m_offlineNoiseReprocess == pCbData->sessionId)
    {
        if (m_remainingNoiseReprocessStageResults > 0)
        {
            PipelineData* pPipelineData = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_offlineNoiseReprocess, 0));
            if (NULL != pPipelineData)
            {
                resultFrameNum = pPipelineData->seqIdToFrameNum[pResult->frameworkFrameNum % MaxOutstandingRequests];
            }
        }
    }
    else if (m_realtime == pCbData->sessionId)
    {
        PipelineData* pPipelineData = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_realtime, 0));

        resultFrameNum  = pPipelineData->seqIdToFrameNum[pResult->frameworkFrameNum % MaxOutstandingRequests];
        rtPipelineReqId = pResult->frameworkFrameNum;

        CHX_LOG("Realtime ReqId to AppFrameNum: %d <--> %d", pResult->frameworkFrameNum, resultFrameNum);
    }

    CHX_LOG("ProcessResult Frame Number: %d, "
            "Session ID: %d, Result Metadata: %p, Number of Output Buffers: %d Framework Frame Number: %d",
            resultFrameNum,
            pCbData->sessionId,
            pResult->pResultMetadata,
            pResult->numOutputBuffers,
            pResult->frameworkFrameNum);

    UINT32                    resultFrameIndex = resultFrameNum % MaxOutstandingRequests;
    camera3_capture_result_t* pUsecaseResult   = m_pUsecase->GetCaptureResult(resultFrameIndex);

    pUsecaseResult->frame_number = resultFrameNum;

    // If result contain metadata and metadata has not been sent to framework
    if ((NULL != pResult->pOutputMetadata) && (NULL != pResult->pInputMetadata))
    {
        UINT64             shutterTimestamp          = m_pUsecase->GetRequestShutterTimestamp(resultFrameNum);
        ChiMetadata*       pChiOutputMetadata        = m_pMetadataManager->GetMetadataFromHandle(pResult->pOutputMetadata);
        ChiMetadata*       pChiInputMetadata         = m_pMetadataManager->GetMetadataFromHandle(pResult->pInputMetadata);

        // Store the Metadata here, will be used later with buffer result.
        m_featureOutputMetaHandle = pResult->pOutputMetadata;

        if (m_realtime == pCbData->sessionId)
        {
            // release input
            m_pMetadataManager->Release(pChiInputMetadata);

            // in FS2 mode there is NO RDI request along with preview
            if (StreamConfigModeFastShutter != ExtensionModule::GetInstance()->GetOpMode(m_pUsecase->GetCameraId()) ||
                TRUE == m_isSnapshotFrame[resultFrameIndex])
            {
                m_pUsecase->FillMetadataForRDIQueue(rtPipelineReqId, m_rdiStreamIndex, pChiOutputMetadata);
            }
        }

        if (FALSE == m_pUsecase->IsMetadataSent(resultFrameIndex) && NULL == pUsecaseResult->result)
        {
            // Do Not wait for Snapshot frame metadata, Return Preview metadata back to fwk.
            // If we wait for snapshot, and if it takes more time to process, we will block the preview.
            pUsecaseResult->partial_result = pResult->numPartialMetadata;

            if (0 != shutterTimestamp)
            {
                ChxUtils::UpdateTimeStamp(pChiOutputMetadata, shutterTimestamp, resultFrameNum);
                m_pUsecase->SetMetadataAvailable(resultFrameIndex);
                isAppResultsAvailable = TRUE;

                m_pUsecase->UpdateAppResultMetadata(pChiOutputMetadata,
                                                    resultFrameIndex,
                                                    m_pUsecase->GetMetadataClientIdFromPipeline(pCbData->sessionId, 0));
            }
            else
            {
                CHX_LOG("add metadata to capture result for frame %d", resultFrameNum);
                m_pUsecase->UpdateAppResultMetadata(pChiOutputMetadata,
                                                    resultFrameIndex,
                                                    m_pUsecase->GetMetadataClientIdFromPipeline(pCbData->sessionId, 0));
            }
        }
    }

    if (pResult->numOutputBuffers > 0)
    {
        for (UINT i = 0; i < pResult->numOutputBuffers; i++)
        {
            // If our internal stream, copy the result into the target buffer to be consumed by the offline pipeline
            if (m_pRdiStream == pResult->pOutputBuffers[i].pStream)
            {
                m_pUsecase->UpdateBufferReadyForRDIQueue(rtPipelineReqId, m_rdiStreamIndex, TRUE);

                if (TRUE == m_triggerMFNRReprocess[rtPipelineReqId % MaxOutstandingRequests])
                {
                    m_allRDIResultsAvaliable = TRUE;
                }

                // No Fd stream for FS2
                if ((TRUE == m_isSnapshotFrame[resultFrameIndex]) &&
                    (TRUE == m_isLLSSnapshot)                     &&
                    (TRUE == m_allRDIResultsAvaliable)            &&
                    ((TRUE == m_allFDResultsAvaliable) ||
                    (StreamConfigModeFastShutter == ExtensionModule::GetInstance()->GetOpMode(m_pUsecase->GetCameraId()))))
                {
                    m_triggerMFNRReprocess[rtPipelineReqId % MaxOutstandingRequests] = FALSE;
                    CHX_LOG("FeatureMFNR LLS signal request thead");

                    m_captureRequest.frame_number = rtPipelineReqId;
                    m_pOfflineRequestAvailable->Signal();
                }
            }
            else if (m_pFdStream == pResult->pOutputBuffers[i].pStream)
            {
                m_pUsecase->UpdateBufferReadyForFDQueue(rtPipelineReqId, m_fdStreamIndex, TRUE);

                if (TRUE == m_triggerMFNRReprocess[rtPipelineReqId % MaxOutstandingRequests])
                {
                    m_allFDResultsAvaliable = TRUE;
                }

                if ((TRUE == m_isSnapshotFrame[resultFrameIndex]) &&
                    (TRUE == m_isLLSSnapshot)                     &&
                    (TRUE == m_allRDIResultsAvaliable)            &&
                    (TRUE == m_allFDResultsAvaliable))
                {
                    m_triggerMFNRReprocess[rtPipelineReqId % MaxOutstandingRequests] = FALSE;
                    CHX_LOG("FeatureMFNR LLS signal request thead");

                    m_captureRequest.frame_number = rtPipelineReqId;
                    m_pOfflineRequestAvailable->Signal();
                }
            }
            else if ((0 < m_remainingPrefilterStageResults) && (m_offline == pCbData->sessionId))
            {
                ProcessPreFilterStageResults(&(pResult->pOutputBuffers[i]));
            }
            else if ((0 < m_remainingBlendStageResults) && (m_offline == pCbData->sessionId))
            {
                ProcessBlendStageResults(&(pResult->pOutputBuffers[i]));
            }
            else if ((0 < m_remainingPostFilterStageResults) && (m_offline == pCbData->sessionId))
            {

                // m_remainingPostFilterStageResults will be set to 0 when ProcessPostFilterStageResults done.
                // ProcessFeatureDone should be only called when ProcessPostFilterStageResults done
                if ((FALSE  == m_noiseReprocessEnable)              &&
                    (FALSE  == IsJPEGSnapshotConfigured())          &&
                    (1      == m_remainingPostFilterStageResults))

                {
                    // add additional reference since the output metadata will be used by the usecase.
                    CDKResult result                = CDKResultSuccess;
                    ChiMetadata* pChiOutputMetadata = m_pMetadataManager->GetMetadataFromHandle(m_featureOutputMetaHandle);
                    pChiOutputMetadata->AddReference();

                    ProcessPostFilterStageResults(&(pResult->pOutputBuffers[0]));
                    result = m_pUsecase->MergeDebugData(pChiOutputMetadata, &m_debugDataOfflineSnapshot,
                            "MFNR to YUV Snapshot");
                    if (CDKResultSuccess != result)
                    {
                        CHX_LOG_INFO("Fail to merge DebugData");
                    }

                    // Send feature done to usecase, to let usecase know that feature is done with YUV snapshot generation.
                    pResult->pOutputMetadata = m_featureOutputMetaHandle;

                    m_pUsecase->ProcessFeatureDone(m_internalRequestId, this, pResult);

                    pChiOutputMetadata->ReleaseReference();
                }
                else
                {
                    ProcessPostFilterStageResults(&(pResult->pOutputBuffers[0]));
                }
            }
            else if ((0 < m_remainingNoiseReprocessStageResults) && (m_offlineNoiseReprocess == pCbData->sessionId))
            {

                // m_remainingNoiseReprocessStageResults will be set to 0 when ProcessOfflineNoiseReprocessResults done.
                // and ProcessFeatureDone should be only called when ProcessOfflineNoiseReprocessResults done.
                if (FALSE == IsJPEGSnapshotConfigured() && (1 == m_remainingNoiseReprocessStageResults))
                {
                    // add additional reference since the output metadata will be used by the usecase.
                    CDKResult result                = CDKResultSuccess;
                    ChiMetadata* pChiOutputMetadata = m_pMetadataManager->GetMetadataFromHandle(m_featureOutputMetaHandle);
                    pChiOutputMetadata->AddReference();

                    ProcessOfflineNoiseReprocessResults(&(pResult->pOutputBuffers[0]));
                    result = m_pUsecase->MergeDebugData(pChiOutputMetadata, &m_debugDataOfflineSnapshot,
                            "MFNR to YUV Snapshot");
                    if (CDKResultSuccess != result)
                    {
                        CHX_LOG_INFO("Fail to merge DebugData");
                    }

                    // Send feature done to usecase, to let usecase know that feature is done with YUV snapshot generation.
                    pResult->pOutputMetadata = m_featureOutputMetaHandle;
                    m_pUsecase->ProcessFeatureDone(m_internalRequestId, this, pResult);
                    pChiOutputMetadata->ReleaseReference();
                }
                else
                {

                    ProcessOfflineNoiseReprocessResults(&(pResult->pOutputBuffers[0]));
                }
            }
            else if ((0 < m_remainingSnapshotStageResults) && (m_offline == pCbData->sessionId))
            {
                ProcessSnapshotStageResults(&(pResult->pOutputBuffers[i]), pUsecaseResult);
                isAppResultsAvailable = TRUE;
            }
            else
            {
                ProcessNormalResults(&(pResult->pOutputBuffers[i]), pUsecaseResult);
                isAppResultsAvailable = TRUE;
            }
        }
    }

    if (TRUE == isAppResultsAvailable)
    {
        CHX_LOG_INFO("ProcessAndReturnFinishedResults for frame: %d", pUsecaseResult->frame_number);
        m_pUsecase->ProcessAndReturnFinishedResults();
    }

    if ((1 <= ExtensionModule::GetInstance()->EnableDumpDebugData()) &&
        (NULL != pResult->pOutputMetadata)                           &&
        (m_offline == pCbData->sessionId))
    {
        // Process debug-data only for offline processing, this will get only MFNR buffers
        m_pUsecase->ProcessDebugData(pResult, pPrivateCallbackData, resultFrameNum);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::ProcessCHIPartialData
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::ProcessCHIPartialData(
    UINT32    frameNum,
    UINT32    sessionId)
{
    CAMX_UNREFERENCED_PARAM(frameNum);
    CAMX_UNREFERENCED_PARAM(sessionId);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::ProcessDriverPartialCaptureResult
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::ProcessDriverPartialCaptureResult(
    CHIPARTIALCAPTURERESULT*    pResult,
    VOID*                       pPrivateCallbackData)
{
    if ((TRUE == static_cast<BOOL>(ChxUtils::AtomicLoadU32(&m_aPauseInProgress))) ||
        (FlushStatus::NotFlushing != m_pUsecase->GetFlushStatus()))
    {
        CHX_LOG_INFO("MFNR process partial capture result return because of cleanup");
        return;
    }

    SessionPrivateData* pCbData        = static_cast<SessionPrivateData*>(pPrivateCallbackData);
    UINT32              resultFrameNum = pResult->frameworkFrameNum;
    PartialResultSender sender         = PartialResultSender::DriverPartialData;

    if (m_realtime == pCbData->sessionId)
    {
        PipelineData* pPipelineData = const_cast<PipelineData*>(m_pUsecase->GetPipelineData(m_realtime, 0));

        resultFrameNum = pPipelineData->seqIdToFrameNum[pResult->frameworkFrameNum % MaxOutstandingRequests];

        UINT32 resultFrameIndex = resultFrameNum % MaxOutstandingRequests;
        CHX_LOG("MFNR Realtime ReqId to AppFrameNum: %d <--> %d for Real Time sessionid:%d",
            pResult->frameworkFrameNum,
            resultFrameNum,
            pCbData->sessionId);

        // If result contain metadata and metadata has not been sent to framework
        if (NULL != pResult->pPartialResultMetadata)
        {
            ChiMetadata* pChiOutputMetadata = m_pMetadataManager->GetMetadataFromHandle(pResult->pPartialResultMetadata);
            // This check is to ensure that we have not sent earlier
            if (TRUE == m_pUsecase->CheckIfPartialDataCanBeSent(sender, resultFrameIndex))
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
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMFNR::ProcessMessage
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::ProcessMessage(
    const CHIMESSAGEDESCRIPTOR* pMessageDescriptor,
    VOID*                       pPrivateCallbackData)
{
    CAMX_UNREFERENCED_PARAM(pPrivateCallbackData);

    if (ChiMessageTypeMetaBufferDone == pMessageDescriptor->messageType)
    {
        CHX_LOG("FeatureMFNR MetaBuffer Done frameNum %u i/p metadata %p o/p metadata %p",
            pMessageDescriptor->message.metaBufferDoneMessage.frameworkFrameNum,
            pMessageDescriptor->message.metaBufferDoneMessage.inputMetabuffer,
            pMessageDescriptor->message.metaBufferDoneMessage.outputMetabuffer);
    }
}

INT32 FeatureMFNR::GetRequiredPipelines(
    AdvancedPipelineType* pPipelines,
    INT32                 size)
{
    INT32 count               = 0;
    const INT32 pipelineCount = 11;

    if (NULL != pPipelines && size >= pipelineCount)
    {
        AdvancedPipelineType pipelineGroup[pipelineCount];
        UINT                 cameraId[pipelineCount];
        UINT                 physicalCameraID = m_pUsecase->GetPhysicalCameraId(m_physicalCameraIndex);

        if (InvalidPhysicalCameraId != physicalCameraID)
        {
            INT32 pipelineGroupIndex = 0;
            INT32 index              = 0;

            pPipelines[index]                   = AdvancedPipelineType::MFNRPrefilterType;
            cameraId[pipelineGroupIndex]        = physicalCameraID;
            pipelineGroup[pipelineGroupIndex++] = pPipelines[index];
            index++;

            pPipelines[index]                   = AdvancedPipelineType::MFNRBlendType;
            cameraId[pipelineGroupIndex]        = physicalCameraID;
            pipelineGroup[pipelineGroupIndex++] = pPipelines[index];
            index++;

            pPipelines[index]                   = AdvancedPipelineType::MFNRPostFilterType;
            cameraId[pipelineGroupIndex]        = physicalCameraID;
            pipelineGroup[pipelineGroupIndex++] = pPipelines[index];
            index++;

            if (TRUE == m_pUsecase->IsJPEGSnapshotStream())
            {
                CHX_LOG("Register Jpeg pipeline as blob stream is configured");

                pPipelines[index]                   = AdvancedPipelineType::InternalZSLYuv2JpegMFNRType;
                cameraId[pipelineGroupIndex]        = physicalCameraID;
                pipelineGroup[pipelineGroupIndex++] = pPipelines[index];
                index++;
            }

            // group the offline session together and get the session id
            m_offline          = m_pUsecase->GetUniqueSessionId(&pipelineGroup[0], pipelineGroupIndex, m_physicalCameraIndex);
            m_pUsecase->SetPipelineCameraId(&pipelineGroup[0], &cameraId[0], pipelineGroupIndex);
            pipelineGroupIndex = 0;

            if (TRUE == m_noiseReprocessEnable)
            {
                pPipelines[index]                   = AdvancedPipelineType::OfflineNoiseReprocessType;
                cameraId[pipelineGroupIndex]        = physicalCameraID;
                pipelineGroup[pipelineGroupIndex++] = pPipelines[index];
                index++;

                m_offlineNoiseReprocess = m_pUsecase->GetUniqueSessionId(&pipelineGroup[0], pipelineGroupIndex, m_physicalCameraIndex);
                m_pUsecase->SetPipelineCameraId(&pipelineGroup[0], &cameraId[0], pipelineGroupIndex);
                pipelineGroupIndex = 0;
            }

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

                cameraId[pipelineGroupIndex]        = physicalCameraID;
                pipelineGroup[pipelineGroupIndex++] = pPipelines[index];
                // get the preview session id
                m_realtime          = m_pUsecase->GetUniqueSessionId(&pipelineGroup[0], pipelineGroupIndex, m_physicalCameraIndex);
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

    CHX_LOG("FeatureMFNR::GetRequiredPipelines, required pipeline count:%d", count);

    CHX_ASSERT(pipelineCount == count);

    return count;
}

UINT FeatureMFNR::GetPipelineIndex(
    MFNRStage pipelineStage)
{
    UINT index = pipelineStage;

    CHX_LOG("pipeline index %d for stage %d", index, pipelineStage);

    return index;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UsecaseZSLCommonReprocess::RequestThread
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID* FeatureMFNR::RequestThread(
    VOID* pThreadData)
{
    PerThreadData* pPerThreadData = reinterpret_cast<PerThreadData*>(pThreadData);
    FeatureMFNR*   pCameraUsecase = reinterpret_cast<FeatureMFNR*>(pPerThreadData->pPrivateData);

    pCameraUsecase->RequestThreadProcessing();

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FeatureMfnr::RequestThreadProcessing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::RequestThreadProcessing()
{
    CHX_LOG_INFO("MFNR Request Thread Processing Entered");

    while (TRUE)
    {
        m_pOfflineRequestMutex->Lock();

        /// @todo (CAMX-4025): Code below appeared to handle 2|3 aspects of the problem
        ///                    at the same time.
        ///
        ///                    1. Termination of a thread upon receiving a signal after
        ///                       setting m_offlineRequestProcessTerminate to TRUE
        ///                    2. Processing of requests while termination of thread is
        ///                       not yet requested
        ///                    3. Skip processing of frames, when thread termination is
        ///                       requested
        ///
        ///                    However it, does not handle spurious wakeups allowed for
        ///                    wait on conditional variables, also skips wait() thus it
        ///                    would be lead to the incorrrect invocation of the method
        ///                    SubmitOfflineMfnrRequest() with possible junk data.
        ///
        ///                    Instead it should wait for an event in a loop, until the
        ///                    event signal is ready positively.  i.e., signal() should
        ///                    match a wait() in progress.
        if (FALSE == m_offlineRequestProcessTerminate)
        {
            m_pOfflineRequestAvailable->Wait(m_pOfflineRequestMutex->GetNativeHandle());
        }

        // m_offlineRequestProcessTerminate could be set to TRUE while waiting
        if (TRUE == m_offlineRequestProcessTerminate)
        {
            m_pOfflineRequestMutex->Unlock();
            break;
        }

        /// @todo (CAMX-4025): Code below has to process  MFNR requests in a while loop
        ///                    as long as there are pending frames, then wait for a new
        ///                    signal at the beginning of our loop; i.e., while (TRUE).

        SubmitOfflineMfnrRequest(m_requestFrameNumber, &m_captureRequest);
        SetFeatureStatus(FeatureStatus::READY);
        m_maxSnapshotReqId++;

        m_pOfflineRequestMutex->Unlock();
    }

    CHX_LOG_INFO("MFNR Thread Exited");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMfnr::CalculateMFNRTotalFramesByGain
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID FeatureMFNR::CalculateMFNRTotalFramesByGain()
{
    UINT             totalMFNRNumFrames      = MfnrDefaultInputFrames;
    UINT32           metadataAECFrameControl = 0;
    ChiMetadata*     pMeta                   = m_pUsecase->GetLastReadyRDIMetadata(m_physicalCameraIndex);
    AECFrameControl* pFrameCtrl              = NULL;

    if (NULL != pMeta)
    {
        pFrameCtrl = static_cast<AECFrameControl*>(pMeta->GetTag("org.quic.camera2.statsconfigs",
                                                                 "AECFrameControl"));
        if (NULL != pFrameCtrl)
        {
            FLOAT realGain = pFrameCtrl->exposureInfo[ExposureIndexSafe].linearGain;

            CHX_LOG_INFO("AEC Gain received = %f", realGain);

            // Need update from latest tuning xml file
            if (realGain <= 2.0f)
            {
                totalMFNRNumFrames = 3;
            }
            else if (realGain <= 4.0f)
            {
                totalMFNRNumFrames = 4;
            }
            else if (realGain <= 8.0f)
            {
                totalMFNRNumFrames = 5;
            }
            else if (realGain <= 16.0f)
            {
                totalMFNRNumFrames = 6;
            }
            else if (realGain <= 32.0f)
            {
                totalMFNRNumFrames = 7;
            }
            else
            {
                totalMFNRNumFrames = 8;
            }
        }
    }
    else
    {
        CHX_LOG_ERROR("Not able to obtain AEC gain for calculation of Total MFNR frames");
    }

    m_mfnrTotalNumFrames = totalMFNRNumFrames;
    CHX_LOG_INFO("Total number of MFNR Frames = %d", totalMFNRNumFrames);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMfnr::PublishMFNRTotalFrames
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::PublishMFNRTotalFrames(
    ChiMetadata* pMeta)
{
    CDKResult result = pMeta->SetTag("org.quic.camera2.mfnrconfigs", "MFNRTotalNumFrames",
                                     &m_mfnrTotalNumFrames, 1);
    if (CDKResultSuccess != result)
    {
        CHX_LOG_ERROR("Cannot set MFNR Total Number of Frames info into metadata");
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FeatureMfnr::PublishDisableZoomCrop
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult FeatureMFNR::PublishDisableZoomCrop(
    ChiMetadata* pMeta)
{
    BOOL bDisableZoomCrop = TRUE;

    CDKResult result = pMeta->SetTag("org.quic.camera2.ref.cropsize", "DisableZoomCrop",
                                     &bDisableZoomCrop, 1);
    if (CDKResultSuccess != result)
    {
        CHX_LOG_ERROR("Cannot set disable Zoom crop into metadata");
    }

    return result;
}
