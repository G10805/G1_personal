////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxfeaturemfnr.h
/// @brief CHX feature mfnr class declarations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CHXFEATUREMFNR_H
#define CHXFEATUREMFNR_H

#if defined (_LINUX)
#include <sys/mman.h>               // memory management
#endif

#include <assert.h>

#include "chi.h"
#include "chinode.h"
#include "chxadvancedcamerausecase.h"
#include "chxfeature.h"
#include "chxincs.h"
#include "chxusecaseutils.h"

// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files

#define CHX_STRINGIZE(arg) #arg

////////////////////////
/// Forward Declarations
////////////////////////
class AdvancedCameraUsecase;
struct SnapshotFeatureInfo;

class FeatureMFNR : public Feature
{
public:
    static FeatureMFNR* Create(
        AdvancedCameraUsecase* pUsecase,
        UINT32 rtPipelineIndex);

    CDKResult Initialize(
        AdvancedCameraUsecase* pUsecase);

    virtual VOID Destroy(
        BOOL isForced);

    virtual VOID Pause(
        BOOL isForced);

    virtual ChiUsecase* OverrideUsecase(
        LogicalCameraInfo*              pCameraInfo,
        camera3_stream_configuration_t* pStreamConfig);

    virtual VOID PipelineCreated(
        UINT sessionId,
        UINT pipelineId);

    virtual VOID PipelineDestroyed(
        UINT32 sessionId,
        UINT32 pipelineId);

    virtual CDKResult ExecuteProcessRequest(
        camera3_capture_request_t* pRequest);

    virtual VOID ProcessResult(
        CHICAPTURERESULT* pResult,
        VOID*             pPrivateCallbackData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ProcessDriverPartialCaptureResult
    ///
    /// @brief  This will be called by the usecase when Partial Result is available from the driver
    ///
    /// @param  pResult                 Partial result from the driver
    /// @param  pPrivateCallbackData    Private Data managed by the client
    ///
    /// @return VOID
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual VOID ProcessDriverPartialCaptureResult(
        CHIPARTIALCAPTURERESULT*    pResult,
        VOID*                       pPrivateCallbackData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ProcessCHIPartialData
    ///
    /// @brief  This will be called by the usecase if CombinedPartialMeta is supported
    ///         Here all the CHI Partial Metadata should be populated and sent to framework as required
    ///
    /// @param  frameNum   Frame number for which the CHI Partial data should be populated
    /// @param  sessionId  Corresponding Session Id
    ///
    /// @return VOID
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual VOID ProcessCHIPartialData(
        UINT32    frameNum,
        UINT32    sessionId);

    virtual VOID ProcessMessage(
        const CHIMESSAGEDESCRIPTOR* pMessageDescriptor,
        VOID*                       pPrivateCallbackData);

    virtual FeatureType GetFeatureType()
    {
        return FeatureType::MFNR;
    }

    virtual BOOL StreamIsInternal(
        ChiStream* pStream)
    {
        BOOL isInternal = ((NULL != pStream)  &&
                           (pStream == m_pRdiStream ||
                            pStream == m_pFdStream));
        return isInternal;
    }

    virtual INT32 GetRequiredPipelines(
        AdvancedPipelineType* pPipelines,
        INT32                 size);

    virtual CDKResult GetRequestInfo(
        camera3_capture_request_t* pRequest,
        FeatureRequestInfo*        pOutputRequests,
        FeatureRequestType         requestType);

    static const UINT32 MaxFrameCntNeededForMFNR = 8;
protected:
    FeatureMFNR() = default;
    virtual ~FeatureMFNR() = default;

    struct MFNRInputInfo
    {
        CHISTREAMBUFFER bufferArray[BufferQueueDepth];          ///< CHI Stream buffer will be as MFNR buffer input
        CHISTREAMBUFFER fdbufferArray[BufferQueueDepth];        ///< CHI FD Stream buffer will be as MFNR buffer input
        ChiMetadata*    pChiMetadata[BufferQueueDepth];         ///< Metadata array will be as MFNR pipeline metadata input
        UINT32          numOfBuffers;                           ///< number of buffers needed for this MFNR process
        UINT32          realtimeFrameNum[BufferQueueDepth];     ///< Is shutter message sent
        UINT32          targetBufferQIdx;                       ///< Index of input target buffer
    };

    UINT32 m_mfnrZslQAnchorIndices[BufferQueueDepth];     ///< Best to least good anchor frames
    UINT32 m_nextStageAnchorIndex;                        ///< Next stage anchor index

    static const UINT ZSLInputFrameOffset = 2;            ///< ZSL Input frame to select

    enum class AnchorFrameSelectionMode
    {
         TimeStamp,                                       ///< Will use the desired image as anchor; process others in increasing temporal difference
         Sharpness,                                       ///< Process images in order of decreasing sharpness (focus value)
         Lighting                                         ///< Process images with similar lighting first
    };

    enum class AnchorFrameSelectionAlgorithm
    {
         None,                                            ///< As per current implementation
         Fixed,                                           ///< Apply fixed known order
         NonFixed                                         ///< Apply non-fixed order based on AnchorFrameSelectionMode
    };

    struct AnchorFrameSelectionData
    {
         FLOAT                    focusValue[BufferQueueDepth];
         UINT32*                  histogram[BufferQueueDepth];
         UINT32                   minHistrogramBin;
         UINT32                   maxHistrogramBin;
         UINT64                   timestamps[BufferQueueDepth];
         UINT32                   numOfImagesToBlend;
         AnchorFrameSelectionMode anchorFrameSelectionMode;
         UINT32                   desiredAnchorFrameIndex;
         UINT64                   anchorFrameTimeRange;
         UINT32                   brightnessTolerance;
         BOOL                     removeExpectedBadImages;
         UINT32                   numImagesAllowedAsAnchor; // Anchor image will be chosen from first N images
         UINT32                   numSharpnessImages;       // Limits the number of images for which sharpness can be calculated
         UINT32                   sharpnessBlockSize;       // A parameter for calculating sharpness
         float                    sharpnessRankValue;       // A parameter for calculating sharpness
    };

    CDKResult FetchVendorTagsForAnchorFrameSelection(
        UINT32* pVendorTagLocationFocusValue,
        UINT32* pVendorTagLocationBuckets,
        UINT32* pVendorTagLocationStats);

    CDKResult PopulateAnchorFrameSelectionDataFromVTags(
        AnchorFrameSelectionData* pAnchorFrameSelectionData,
        MFNRInputInfo*            pMfnrInputInfo);

    UINT32 CalculateBrightness(
        UINT32* histogram,
        UINT    minHistrogramBin,
        UINT    maxHistrogramBin);

    FLOAT CalculateSharpness(
        CHISTREAMBUFFER*   InputFDBuffer,
        UINT32             sharpnessBlockSize,
        float              sharpnessRankValue);

    BOOL IsCandidateForAnchor(
        UINT32 numImagesAllowedAsAnchor,
        UINT32 imageIndex);

    CHX_INLINE UINT32 getZSLInputFrameOffset()
    {
        return ZSLInputFrameOffset;
    }

    CDKResult SelectFixedFrameOrder(
        UINT* pFrameNumber,
        UINT  totalNumberOfFrames);

    CDKResult SelectNonFixedFrameOrder(
        MFNRInputInfo*   pMfnrInputInfo,
        UINT* pFrameNumber);

    CDKResult SelectMFNRAnchorFrameAndFrameOrder(
        MFNRInputInfo*                 pMfnrInputInfo,
        CHISTREAMBUFFER*               selectionInputBuffer,
        size_t                         selectionInputBufferSize,
        ChiMetadata**                  ppMergedMetadata,
        AnchorFrameSelectionAlgorithm* anchorSelectionAlgorithm);

    CDKResult CreateMFNRInputInfo(
        MFNRInputInfo*             pInputInfo,
        camera3_capture_request_t* pRequest,
        SnapshotFeatureInfo*       pFeatureInfo);

    CDKResult PrepareMFNRInputMetadata(
        MFNRInputInfo*             pInputInfo);

    CDKResult PerformAnchorImagePicking(
        MFNRInputInfo*                 pMfnrInputInfo,
        AnchorFrameSelectionData*      pAnchorFrameSelectionData,
        UINT*                          pFrameNumber);

    virtual UINT32 GetRequiredFramesForSnapshot(
        const camera_metadata* pMetadata);

    virtual UINT32 GetMaxRequiredFramesForSnapshot(
        const camera_metadata_t *pMetadata)
    {
        (VOID)pMetadata;

        return MaxFrameCntNeededForMFNR;
    }

    BOOL IsJPEGSnapshotConfigured()
    {
        return m_isJPEGSnapshotConfigured;
    }

    CAMX_INLINE BOOL IsIHDRSnapshotEnabled()
    {
        return ((IHDR_START == m_stateIHDRSnapshot)||(IHDR_ENABLED == m_stateIHDRSnapshot))? TRUE: FALSE;
    }

    ChiUsecase*                m_pChiUsecase;                                      ///< Copy of ZSL usecase that gets adjusted
    ChiStream*                 m_pRdiStream;                                       ///< Allocated internal RDI stream
    ChiStream*                 m_pFdStream;                                        ///< Allocated internal FD stream
    ChiStream*                 m_pPreviewStream;                                   ///< Tracking of the stream used for preview
    ChiStream*                 m_pSnapshotStream;                                  ///< Tracking of the stream used for snapshot
    UINT32                     m_rdiStreamIndex;                                   ///< Stream/target index for RDI stream
    UINT32                     m_fdStreamIndex;                                    ///< Stream/target index for RDI stream
    TargetBuffer*              m_pRdiTargetBuffer;                                 ///< TargetBuffer in the CameraUsecase
    TargetBuffer*              m_pFdTargetBuffer;                                  ///< TargetBuffer in the CameraUsecase

    UINT32                     m_maxSnapshotReqId;                                 ///< Last valid ID to move snapshotReqId to
                                                                                   ///  Owned by the main thread
    UINT32                     m_snapshotReqIdToFrameNum[MaxOutstandingRequests];  ///< snapshotReqId to framework frame mapping
    CHISTREAMBUFFER            m_snapshotBuffers[MaxOutstandingRequests][2];       ///< Result buffers from app for snapshot
    UINT                       m_snapshotBufferNum[MaxOutstandingRequests];        ///< Buffer count
    BOOL                       m_isSnapshotFrame[MaxOutstandingRequests];          ///< Is a snapshot request
    BOOL                       m_isSnapshotMetadataNeeded;                         ///< Is snapshot metadata needed.

    CHIPRIVDATA                m_privData[MaxOutstandingRequests];                      ///< Private data for real-time pipeline
    CHIPRIVDATA                m_offlinePrivData[MaxOutstandingRequests];               ///< Private data for offline pipelines
    CHIPRIVDATA                m_offlineNoiseReprocessPrivData[MaxOutstandingRequests]; ///< Private data for offline pipelines

    UINT32                     m_sensorModeIndex;                                  ///< sensorModeIndex
    UINT32                     m_snapshotPipelineIndex;
    UINT32                     m_prefilterPipelineIndex;
    UINT32                     m_blendPipelineIndex;
    UINT32                     m_postfilterPipelineIndex;
    UINT32                     m_scalePipelineIndex;
    UINT32                     m_previewPipelineIndex;

    UINT32                     m_realtime;
    UINT32                     m_offline;
    UINT32                     m_offlineNoiseReprocess;
    UINT32                     m_requestFrameNumber;
    UINT32                     m_internalRequestId;
    UINT32                     m_activeCameraId;
    UINT32                     m_activePipelineIndex;

    enum MFNRStage
    {
        MfnrStagePrefilter = 0,
        MfnrStageBlend,
        MfnrStagePostfilter,
        MfnrStageSnapshot,
        MfnrStageNoiseReprocess,
        MfnrStageMax
    };

    enum MFNRReference
    {
        MfnrReferenceFull = 0,
        MfnrReferenceDS4,
        MfnrReferenceMax
    };

    const CHAR* MFNRStageNames[MfnrStageMax] =
    {
        CHX_STRINGIZE(MfnrStagePrefilter),
        CHX_STRINGIZE(MfnrStageBlend),
        CHX_STRINGIZE(MfnrStagePostfilter),
        CHX_STRINGIZE(MfnrStageSnapshot),
        CHX_STRINGIZE(MfnrStageNoiseReprocess)
    };

    const CHAR* PipelineType[PipelineCount] =
    {
        CHX_STRINGIZE(ZSLSnapshotJpegType),
        CHX_STRINGIZE(ZSLSnapshotYUVType),
        CHX_STRINGIZE(InternalZSLYuv2JpegType),
        CHX_STRINGIZE(InternalZSLYuv2JpegMFNRType),
        CHX_STRINGIZE(Merge3YuvCustomTo1YuvType),
        CHX_STRINGIZE(ZSLPreviewRawType),
        CHX_STRINGIZE(ZSLPreviewRawYUVType),
        CHX_STRINGIZE(MFNRPrefilterType),
        CHX_STRINGIZE(MFNRBlendType),
        CHX_STRINGIZE(MFNRPostFilterType),
        CHX_STRINGIZE(SWMFMergeYuvType),
        CHX_STRINGIZE(ZSLSnapshotYUVAuxType),
        CHX_STRINGIZE(InternalZSLYuv2JpegMFNRAuxType),
        CHX_STRINGIZE(MFNRPrefilterAuxType),
        CHX_STRINGIZE(MFNRBlendAuxType),
        CHX_STRINGIZE(MFNRPostFilterAuxType),
        CHX_STRINGIZE(ZSLYuv2YuvType),
        CHX_STRINGIZE(ZSLSnapshotJpegGPUType),
        CHX_STRINGIZE(NoiseReprocessType),
    };

    const CHAR* mfnrBufferManagerNames[4] =
    {
        "MfnrReferenceFullBufferManager",
        "MfnrReferenceDS4BufferManager",
        "MfnrReferenceDS16BufferManager",
        "MfnrReferenceDS64BufferManager",
    };

    static const UINT MaxChiStreamBuffers          = 8;
    static const UINT MfnrMaxPreFilterStageBuffers = (2 + 2);
    static const UINT MfnrMaxBpsRegOutBuffers      = 2;
    static const UINT MfnrDefaultInputFrames       = 5;
    static const UINT MfnrMaxInputRDIFrames        = 12;
    static const UINT MfnrMaxMetadata              = MfnrMaxInputRDIFrames+1;

    static const UINT MfnrNumFramesforFS           = 8;                         ///< num of frames for SWMF in FastShutter mode

    UINT32                     m_mfnrTotalNumFrames;                            ///< Total num of MFNR frames [3..5]
    UINT32                     m_autoMfnr;
    ChiStream*                 m_pPrefilterOutStream[MfnrReferenceMax];         ///< Prefilter out stream
    ChiStream*                 m_pBlendOutStream[MfnrReferenceMax];             ///< Blend out stream
    ChiStream*                 m_pBlendInStream[MfnrReferenceMax];              ///< Blend in stream
    ChiStream*                 m_pScaleInStream[MfnrReferenceMax];              ///< Scale in stream
    ChiStream*                 m_pMfnrBpsRegOutStream;                          ///< Reg out stream
    ChiStream*                 m_pMfnrBpsRegInStream;                           ///< Reg in stream
    ChiStream*                 m_pMfnrPostFilterOutStream;                      ///< Post filter out stream
    ChiStream*                 m_pNoiseReprocessOutStream;                      ///< Noise Reprocess out stream
    ChiStream*                 m_pNoiseReprocessInStream;                       ///< Noise Reprocess in stream
    ChiStream*                 m_pJPEGInputStream;
    ChiMetadata*               m_pApplicationInputMeta;                         ///< Reference for app metadata
    ChiMetadata*               m_pInterStageInputMetadata[MfnrMaxMetadata];     ///< References of inter stage input metadata
    ChiMetadata*               m_pInterStageOutputMetadata[MfnrMaxMetadata];    ///< References of inter stage output metadata
    UINT32                     m_metadataIndex;                                 ///< Current index of the metadata buffer

    CHIBufferManager*          m_pMfnrBpsRegOutBufferManager;                        ///< MFNR BPS Reg out Buffer manager
    UINT                       m_mfnrSessionReqId;                                   ///< Chi request ID for MFNR
    UINT                       m_mfnrReqIdToFrameNum[MaxOutstandingRequests];        ///< Chi req Id to framework frame num
    CHIBufferManager*          m_pMfnrBufferManager[MfnrStageMax][MfnrReferenceMax]; ///< MFNR Buffer Managers
    CHIBufferManager*          m_pOfflineNoiseReprocessBufferManager;                ///< Offline Reprocess Buffer manager

    Mutex*                     m_pMfnrResultMutex;                              ///< Blend Result mutex
    Condition*                 m_pMfnrResultAvailable;                          ///< Wait till all results are available
    volatile BOOL              m_resultsAvailable;
    UINT                       m_processingOrder[BufferQueueDepth /* - 1 */];   ///< Max images to Blend
    UINT                       m_numOfImagesCaptured;
    BOOL                       m_triggerMFNRReprocess[MaxOutstandingRequests];  ///< If trigger reprocess for a give internal Req ID
    Mutex*                     m_pRDIResultMutex;                               ///< RDI result mutex
    Condition*                 m_pRDIResultAvailable;                           ///< Condition to wait all RDI buffers are ready
    BOOL                       m_allRDIResultsAvaliable;                        ///< ALL required RDI buffers and metadatas are available
    BOOL                       m_allFDResultsAvaliable;                         ///< ALL required FD buffers are available
    BOOL                       m_allResultsAvaliable;                           ///< ALL required buffers and metadatas are available
    BOOL                       m_isLLSSnapshot;                                 ///< Flag to indicate LLS snapshot or not
    BOOL                       m_isIHDRSnapshot;                                ///< Flag to indicate IHDR snapshot or not
    BOOL                       m_blockPreviewForSnapshot;                       ///< If block up-coming preview during snapshot request
    BOOL                       m_snapshotResultAvailable;                       ///< Flag to inidicate if final snapshot result availabe
    BOOL                       m_noiseReprocessEnable;                          ///< If offline noise reprocess pipeline enabled or not
    BOOL                       m_isIHDRSnapshotEnable;                          ///< If IHDR was enabled in override settings
    IHDRSnapshotState          m_stateIHDRSnapshot;                             ///< State of IHDR snapshot
    Mutex*                     m_pSnapshotResultMutex;                          ///< Snapshot result mutex
    Condition*                 m_pSnapshotResultAvailable;                      ///< Condition to wait final snapshot buffer ready
    volatile UINT32            m_aPauseInProgress;                              ///< Is pause in progress

    struct MfnrStageResult
    {
        ChiMetadata*     pChiMetadata;                                          ///< Metadata
        CHISTREAMBUFFER  refOutputBuffer[MfnrReferenceMax];                     ///< Ref buffer placeholder
    };

    UINT32                     m_numExpectedStageBuffers;
    MfnrStageResult            m_preFilterStageResult;
    MfnrStageResult            m_scaleStageResult;
    MfnrStageResult            m_blendStageResult;
    MfnrStageResult            m_postFilterStageResult;
    MfnrStageResult            m_noiseReprocessStageResult;

    CHISTREAMBUFFER            m_preFilterAnchorFrameRegResultBuffer;           ///< Anchor Frame/Image Registration Output Buffer at BPS

    UINT32                     m_remainingPrefilterStageResults;
    UINT32                     m_remainingScaleStageResults;
    UINT32                     m_remainingBlendStageResults;
    UINT32                     m_remainingPostFilterStageResults;
    UINT32                     m_remainingSnapshotStageResults;
    UINT32                     m_remainingNoiseReprocessStageResults;

    // Request thread info
    PerThreadData              m_offlineRequestProcessThread;                   ///< Thread to process the results
    Mutex*                     m_pOfflineRequestMutex;                          ///< App Result mutex
    Condition*                 m_pOfflineRequestAvailable;                      ///< Wait till SensorListener results
                                                                                ///  are available
    volatile BOOL              m_offlineRequestProcessTerminate;                ///< Indication to SensorListener result
                                                                                ///  thread to terminate itself
    camera3_capture_request_t  m_captureRequest;

    BOOL                       m_isJPEGSnapshotConfigured;                      ///< Flag to indicate whether JPEG snapshot
                                                                                ///  or YUV snapshot.

    // Dump Debug/Tuning data
    DebugData                  m_debugDataOffline;                              ///< Offline copy for debug-data
    DebugData                  m_debugDataOfflineSnapshot;                      ///< Snapshot copy for debug-data

private:
    // Do not allow the copy constructor or assignment operator
    FeatureMFNR(const FeatureMFNR&) = delete;
    FeatureMFNR& operator=(const FeatureMFNR&) = delete;

    // Disallow the move constructor or assignment operator
    FeatureMFNR(const FeatureMFNR&&) = delete;
    FeatureMFNR& operator=(const FeatureMFNR&&) = delete;

    /// @brief Define blend stage processing FSM states
    enum class BlendStageParallelRequestFSMState
    {
         Init,
         PrepareAndSubmitRequest0,                        ///< Prepare & Submit #1 (ParReq#0)
         PrepareAndSubmitRequest1AndOrProcessRequest0,    ///< Prepare & Submit #2 (ParReq#1) &/| Process #1 (ParReq#0)
         PrepareAndSubmitRequest0AndOrProcessRequest1,    ///< Prepare & Submit #1 (ParReq#0) &/| Process #2 (ParReq#1)
         Exit,                                            ///< Final state
         Done,                                            ///< Terminal state
         Fail                                             ///< Failed/Error state
    };

    static constexpr UINT SizeofBlendStageParallelRequestFSMState =
        (static_cast<UINT>(BlendStageParallelRequestFSMState::Fail) + 1);

    const CHAR* BlendStageParallelRequestFSMStateAsString[SizeofBlendStageParallelRequestFSMState] =
    {
        CHX_STRINGIZE(Init),
        CHX_STRINGIZE(PrepareAndSubmitRequest0),
        CHX_STRINGIZE(PrepareAndSubmitRequest1AndOrProcessRequest0),
        CHX_STRINGIZE(PrepareAndSubmitRequest0AndOrProcessRequest1),
        CHX_STRINGIZE(Exit),
        CHX_STRINGIZE(Done),
        CHX_STRINGIZE(Fail)
    };

    /// @brief Define CHX/CSL fence signal status codes
    typedef enum
    {
        CHXFenceResultSuccess = 0,  ///< Fence signaled with success
        CHXFenceResultFailed        ///< Fence signaled with failure
    } CHXFenceResult;

    CAMX_INLINE CDKResult SetupStageChiInputFences(
        UINT32           numInputs,
        CHISTREAMBUFFER* pInputBuffers,
        BOOL             skipRefFences = FALSE);

    CAMX_INLINE CDKResult SetupStageChiOutputFences(
        UINT32           numOutputs,
        CHISTREAMBUFFER* pOutputBuffers);

    CAMX_INLINE CDKResult SetupStageChiFences(
        UINT32           numInputs,
        CHISTREAMBUFFER* pInputBuffers,
        UINT32           numOutputs,
        CHISTREAMBUFFER* pOutputBuffers);

    CAMX_INLINE CDKResult SignalStageChiInputFences(
        UINT32           numInputs,
        CHISTREAMBUFFER* pInputBuffers,
        BOOL             skipBPSFences = FALSE,
        BOOL             skipRefFences = FALSE);

    CAMX_INLINE CDKResult SignalStageChiOutputFences(
        UINT32           numOutputs,
        CHISTREAMBUFFER* pOutputBuffers);

    CAMX_INLINE CDKResult SignalStageChiFences(
        UINT32           numInputs,
        CHISTREAMBUFFER* pInputBuffers,
        UINT32           numOutputs,
        CHISTREAMBUFFER* pOutputBuffers);

    CAMX_INLINE CDKResult ClearStageChiInputFences(
        UINT32           numInputs,
        CHISTREAMBUFFER* pInputBuffers,
        BOOL             skipRefFences = FALSE);

    CAMX_INLINE CDKResult ClearStageChiOutputFences(
        UINT32           numOutputs,
        CHISTREAMBUFFER* pOutputBuffers);

    CAMX_INLINE CDKResult ClearStageChiFences(
        UINT32           numInputs,
        CHISTREAMBUFFER* pInputBuffers,
        UINT32           numOutputs,
        CHISTREAMBUFFER* pOutputBuffers);

    CDKResult ActivateOfflinePipeline(
        MFNRStage pipelineStage);

    CDKResult DeactivateOfflinePipeline(
        MFNRStage pipelineStage);

    CDKResult ActivateOfflineNoiseReprocessPipeline();

    CDKResult DeactivateOfflineNoiseReprocessPipeline();

    CDKResult SubmitOfflinePreFilterStageRequest(
        UINT32                     appFrameNumber,
        camera3_capture_request_t* pRequest,
        MFNRInputInfo*             pMfnrInputInfo);

    CDKResult PrepareOfflineBlendStageParallelRequest(
        MFNRInputInfo*   pMfnrInputInfo,
        UINT             processingOrderIndex,
        UINT32           activeBlendParallelRequest,
        UINT32           numInputs,
        CHISTREAMBUFFER* pInputBuffers,
        UINT32           numOutputs,
        CHISTREAMBUFFER  (*pOutputBuffers)[MaxChiStreamBuffers],                  // [2][8]
        BOOL*            pPrefilterStageOutputBuffersAsInput);

    CDKResult SubmitOfflineBlendStageParallelRequest(
        UINT32           appFrameNumber,
        UINT32           activeBlendParallelRequest,
        UINT32           numInputs,
        CHISTREAMBUFFER* pInputBuffers,
        UINT32           numOutputs,
        CHISTREAMBUFFER  (*pOutputBuffers)[MaxChiStreamBuffers],                  // [2][8]
        BOOL             previousBlendParallelResultExpected);

    CDKResult ProcessOfflineBlendStageParallelRequest(
        UINT32           activeBlendParallelRequest,
        UINT32           numInputs,
        CHISTREAMBUFFER  (*pInputBuffers)[MaxChiStreamBuffers],                   // [2][8]
        UINT32           numOutputs,
        CHISTREAMBUFFER  (*pOutputBuffers)[MaxChiStreamBuffers],                  // [2][8]
        BOOL             previousBlendParallelResultExpected,
        BOOL             nextBlendParallelRequestExpected,
        BOOL             releasePrefilterStageOutputBuffers);

    CDKResult PrepareAndSubmitNextRequestAndOrProcessCurrentRequest(
        INT*                               pActiveBlendParallelRequest,
        UINT32                             appFrameNumber,
        camera3_capture_request_t*         pRequest,
        MFNRInputInfo*                     pMfnrInputInfo,
        CHISTREAMBUFFER                    inputBuffers[][MaxChiStreamBuffers],   // [2][8]
        CHISTREAMBUFFER                    outputBuffers[][MaxChiStreamBuffers],  // [2][8]
        UINT32                             numStageInputs,
        UINT32                             numStageOutputs,
        UINT                               numberBlendRequests,
        UINT*                              pRemainingBlendRequests,
        UINT*                              pCurrentBlendRequest,
        BlendStageParallelRequestFSMState  nextState,
        BlendStageParallelRequestFSMState* pUpdatedState,
        BOOL*                              pPrefilterStageOutputBuffersAsInput,
        BOOL*                              releasePrefilterStageOutputBuffers);

    VOID ReleaseOfflineBlendStageOutputBuffers(
        UINT32          numOutputs,
        CHISTREAMBUFFER (*pOutputBuffers)[MaxChiStreamBuffers]);                  // [2][8]

    CDKResult SubmitOfflineBlendStageRequest(
        UINT32                     appFrameNumber,
        camera3_capture_request_t* pRequest,
        MFNRInputInfo*             pMfnrInputInfo);

    VOID DumpMFNRStageResult(
        MfnrStageResult& stageResult);

    CDKResult SubmitOfflinePostfilterStageRequest(
        UINT32                     appFrameNumber,
        camera3_capture_request_t* pRequest,
        MFNRInputInfo*             pMfnrInputInfo);

    CDKResult SubmitOfflineSnapshotStageRequest(
        UINT32                     appFrameNumber,
        camera3_capture_request_t* pRequest,
        MFNRInputInfo*             pMfnrInputInfo);

    CDKResult SubmitOfflineNoiseReprocessStageRequest(
        UINT32                     appFrameNumber,
        camera3_capture_request_t* pRequest,
        MFNRInputInfo*             pMfnrInputInfo);

    CDKResult SubmitOfflineMfnrRequest(
        UINT32                     frameNumber,
        camera3_capture_request_t* pRequest);

    CDKResult SubmitOfflineNoiseReprocessRequest(
        UINT32              frameNumber,
        UINT32&             numInputs,
        CHISTREAMBUFFER*    pInputBuffer,
        UINT32&             numOutputs,
        CHISTREAMBUFFER*    pOutputBuffer,
        ChiMetadata*        pChiInputMetadata,
        ChiMetadata*        pChiOutputMetadata);

    CDKResult TriggerInternalLLSRequests(
        camera3_capture_request_t* pRequest);

    CDKResult TriggerInternalIHDRRequests(
        camera3_capture_request_t*  pRequest);

    CDKResult GenerateLLSRequestSettings(
        const camera_metadata_t*  pInputSetting,
        UINT32                    numFrames,
        const camera_metadata_t** pOutputSettingArray);

    VOID CleanupOfflineMetadata(
        MFNRInputInfo* pMfnrInputInfo);

    VOID ProcessPreFilterStageResults(
        const CHISTREAMBUFFER* pChiStreamBuffer);

    VOID ProcessBlendStageResults(
        const CHISTREAMBUFFER* pChiStreamBuffer);

    VOID ProcessPostFilterStageResults(
        const CHISTREAMBUFFER* pChiStreamBuffer);

    VOID ProcessSnapshotStageResults(
        const CHISTREAMBUFFER*    pChiStreamBuffer,
        camera3_capture_result_t* pUsecaseResult);

    VOID ProcessOfflineNoiseReprocessResults(
        const CHISTREAMBUFFER* pChiStreamBuffer);

    VOID ProcessNormalResults(
        const CHISTREAMBUFFER*    pChiStreamBuffer,
        camera3_capture_result_t* pUsecaseResult);

    CDKResult WaitForRDIResultsReady();

    CDKResult GetOutputBuffer(
        CHIBufferManager* pBufferManager,
        ChiStream*        pChiStream,
        CHISTREAMBUFFER*  pOutputBuffer);

    CDKResult ExecuteMfnrRequest(
        MFNRStage           pipelineStage,
        UINT32              frameNumber,
        UINT32              numOutputs,
        CHISTREAMBUFFER*    pOutputBuffers,
        UINT32              numInputs,
        CHISTREAMBUFFER*    pInputBuffers,
        ChiMetadata*        pInputMetadata,
        ChiMetadata*        pOutputMetadata);

    VOID CalculateMFNRTotalFramesByGain();

    CDKResult PublishMFNRTotalFrames(
        ChiMetadata* pMeta);

    CDKResult PublishDisableZoomCrop(
        ChiMetadata* pMeta);

    VOID InitializeInternalStreams();

    VOID ConfigureInternalStreams();

    VOID SetupInternalPipelines();

    VOID SetupInternalMFNRPreFilterPipeline();

    VOID SetupInternalMFNRBlendPipeline();

    VOID SetupInternalMFNRPostFilterPipeline();

    VOID SetupInternalMFNRSnapshotPipeline();

    VOID SetupOfflineNoiseReprocessPipeline();

    UINT GetPipelineIndex(
        MFNRStage pipelineStage);

    INT GetTargetIndex(
        ChiTargetPortDescriptorInfo* pTargetsInfo,
        const char*                  pTargetName);

    /// Main entry function for the Request thread
    static VOID* RequestThread(
        VOID* pArg);

    VOID  RequestThreadProcessing();

    // get pipeline stage based on index
    CHX_INLINE enum MFNRStage GetPipelineStage(UINT32 index)
    {
        MFNRStage pipelineStage;
        // set pipeline stage
        if (0 == index)
        {
            pipelineStage = MfnrStagePrefilter;
        }
        else if (m_mfnrTotalNumFrames == index + 1)
        {
            pipelineStage = MfnrStagePostfilter;
        }
        else if (m_mfnrTotalNumFrames == index)
        {
            pipelineStage = MfnrStageSnapshot;
        }
        else
        {
            pipelineStage = MfnrStageBlend;
        }
        return pipelineStage;
    }

    // get stage index as string
    CHX_INLINE const CHAR* GetUniqueStageName(
       UINT32       index       = 0,
       const CHAR*  pStagename = "")
    {
        snprintf(stageNameString, sizeof(stageNameString), "mfnr_%s_%d", pStagename, index);
        return stageNameString;
    }

    CHAR stageNameString[MaxFileLen]; ///< temporary memory to format string
};

#endif // CHXFEATUREMFNR_H
