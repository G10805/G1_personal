////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxfeaturemfsr.h
/// @brief CHX feature mfsr class declarations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CHXFEATUREMFSR_H
#define CHXFEATUREMFSR_H

#include <assert.h>

#include "chi.h"
#include "chxincs.h"
#include "chxfeature.h"
#include "chxusecaseutils.h"

// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files

class FeatureMFSR : public Feature
{
public:
    static  FeatureMFSR* Create(
        AdvancedCameraUsecase* pUsecase);

    CDKResult           Initialize(
        AdvancedCameraUsecase* pUsecase);

    virtual VOID        Destroy(
        BOOL isForced);

    virtual VOID    Pause(
        BOOL isForced);

    virtual ChiUsecase* OverrideUsecase(
        LogicalCameraInfo*              pCameraInfo,
        camera3_stream_configuration_t* pStreamConfig);

    virtual VOID        PipelineCreated(
        UINT32 sessionId,
        UINT32 pipelineId);

    virtual CDKResult   ExecuteProcessRequest(
        camera3_capture_request_t*  pRequest);

    virtual VOID        ProcessResult(
        CHICAPTURERESULT*           pResult,
        VOID*                       pPrivateCallbackData);

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

    virtual VOID        ProcessMessage(
        const CHIMESSAGEDESCRIPTOR* pMessageDescriptor,
        VOID*                       pPrivateCallbackData);

    virtual FeatureType GetFeatureType()
    {
        return FeatureType::MFSR;
    }

    virtual BOOL StreamIsInternal(
        ChiStream* pStream)
    {
        BOOL isInternal = (pStream != NULL &&
            (pStream == m_pRdiStream[CameraType::Wide] || pStream == m_pRdiStream[CameraType::Tele]
                || pStream == m_pFdStream));
        return isInternal;
    }

    virtual INT32 GetRequiredPipelines(
        AdvancedPipelineType* pPipelines,
        INT32 size);

    virtual CDKResult   GetRequestInfo(
        camera3_capture_request_t*  pRequest,
        FeatureRequestInfo*         pOutputRequests,
        FeatureRequestType          requestType);

    static const UINT32 MaxFrameCntNeededForMFSR = 8;

protected:
    FeatureMFSR() = default;
    virtual ~FeatureMFSR() = default;

    struct MFSRInputInfo
    {
        CHISTREAMBUFFER bufferArray[BufferQueueDepth];        ///< Chi Stream buffer will be as MFSR buffer input
        CHISTREAMBUFFER fdbufferArray[BufferQueueDepth];      ///< CHI FD Stream buffer will be as MFNR buffer input
        VOID*           pMetadata[BufferQueueDepth];          ///< Metadata array will be as MFSR pipeline metadata input
        UINT32          numOfBuffers;                         ///< number of buffers needed for this MFSR process
    };

    CDKResult CreateMFSRInputInfo(
        MFSRInputInfo* pInputInfo,
        camera3_capture_request_t* pRequest);

    virtual UINT32 GetRequiredFramesForSnapshot(
        const camera_metadata_t* pMetadata)
    {
        (VOID)pMetadata;

        return m_mfsrTotalNumFrames;
    }

    virtual UINT32 GetMaxRequiredFramesForSnapshot(
        const camera_metadata_t *pMetadata)
    {
        (VOID)pMetadata;
        return MaxFrameCntNeededForMFSR;
    }


    ChiUsecase*         m_pChiUsecase;                                      ///< Copy of ZSL usecase that gets adjusted
    ChiStream*          m_pRdiStream[CameraType::Count];                    ///< Allocated internal RDI stream
    ChiStream*          m_pFdStream;                                        ///< Allocated internal FD stream
    ChiStream*          m_pPreviewStream;                                   ///< Tracking of the stream used for preview
    ChiStream*          m_pSnapshotStream;                                  ///< Tracking of the stream used for snapshot
    UINT                m_rdiStreamIndex;                                   ///< Stream/target index for the RDI stream
    UINT                m_fdStreamIndex;                                    ///< Stream/target index for the FD  stream
    TargetBuffer*       m_pRdiTargetBuffer;                                 ///< TargetBuffer in the CameraUsecase
    TargetBuffer*       m_pFdTargetBuffer;                                  ///< TargetBuffer in the CameraUsecase
    UINT32              m_maxSnapshotReqId;                                 ///< Last valid ID to move snapshotReqId to
                                                                            ///  Owned by the main thread
    UINT32              m_snapshotReqIdToFrameNum[MaxOutstandingRequests];  ///< Mapping of snapshotReqId to framework frame

    VOID*               m_pSnapshotInputMeta[MaxOutstandingRequests];       ///< The metadata for the request
    CHISTREAMBUFFER     m_snapshotBuffers[MaxOutstandingRequests][2];       ///< Result buffers from app for snapshot
    UINT                m_snapshotBufferNum[MaxOutstandingRequests];        ///< Buffer count

    BOOL                m_isSnapshotFrame[MaxOutstandingRequests];          ///< Is a snapshot request
    BOOL                m_isSnapshotMetadataNeeded;                         ///< Is snapshot metadata needed.
    UINT32              m_sensorModeIndex;                                  ///< sensorModeIndex
    CHIPRIVDATA         m_privData[MaxOutstandingRequests];

    UINT32              m_snapshotPipelineIndex;
    UINT32              m_prefilterPipelineIndex;
    UINT32              m_blendPipelineIndex;
    UINT32              m_postfilterPipelineIndex;
    UINT32              m_previewPipelineIndex;

    UINT32              m_realtime;
    UINT32              m_offline;
    UINT32              m_requestFrameNumber;
    UINT32              m_internalRequestId;
    UINT32              m_activeCameraId;
    UINT32              m_activePipelineIndex;
    CHIPRIVDATA         m_realtimePrivData[MaxOutstandingRequests];          ///< Private data for each realtime request
    enum MFSRStage
    {
        MfsrStageSnapshot = 0,
        MfsrStagePrefilter,
        MfsrStageBlend,
        MfsrStagePostfilter,
        MfsrStageMax
    };

    enum MFSRReference
    {
        MfsrReferenceFull = 0,
        MfsrReferenceDS4,
        MfsrReferenceDS16,
        MfsrReferenceDS64,
        MfsrReferenceMax
    };

    const CHAR* mfsrBufferManagerNames[4] =
    {
        "MfsrReferenceFullBufferManager",
        "MfsrReferenceDS4BufferManager",
        "MfsrReferenceDS16BufferManager",
        "MfsrReferenceDS64BufferManager",
    };

    static const UINT MaxChiStreamBuffers          = 8;
    static const UINT MfsrMaxPreFilterStageBuffers = 2;
    static const UINT MfsrMaxBpsRegOutBuffers      = 1;
    static const UINT MfsrDefaultInputFrames       = 5;
    static const UINT MfsrMaxInputRDIFrames        = 12;

    UINT32             m_mfsrTotalNumFrames;                                           ///< Total num of MFSR frames
    ChiStream*         m_pReferenceOutStream[CameraType::Count][MfsrReferenceMax];     ///< Prefilter out stream
    ChiStream*         m_pPrefilterInStream[CameraType::Count];                        ///< Prefilter in stream
    ChiStream*         m_pReferenceInStream[CameraType::Count][MfsrReferenceMax];      ///< Blend in stream
    ChiStream*         m_pBpsRegOutStream[CameraType::Count];                          ///< Reg out stream
    ChiStream*         m_pBpsRegInStream[CameraType::Count];                           ///< Reg in stream
    ChiStream*         m_pPostFilterOutStream[CameraType::Count];                      ///< Post filter out stream

    camera_metadata_t* m_pInterStageMetadata;
    camera_metadata_t* m_pApplicationInputMeta;

    CHIBufferManager*  m_pMfsrBpsRegOutBufferManager[CameraType::Count];                  ///< Mfnr BPS Reg out Buffer manager
    UINT               m_mfsrSessionReqId;
    CHIBufferManager*  m_pMfsrBufferManager[CameraType::Count][MfsrReferenceMax];         ///< Mfnr Buffer Managers

    UINT               m_mfsrReqIdToFrameNum[MaxOutstandingRequests];
    Mutex*             m_pMfsrResultMutex;                             ///< Blend Result mutex
    Condition*         m_pMfsrResultAvailable;                         ///< Wait till all results are available
    volatile BOOL      m_resultsAvailable;
    BOOL               m_triggerMFSRReprocess[MaxOutstandingRequests]; ///< If trigger reprocess for a give internal Req ID
    Mutex*             m_pRDIResultMutex;                              ///< RDI result mutex
    Condition*         m_pRDIResultAvailable;                          ///< Condition to wait all RDI buffers are ready
    BOOL               m_allRDIResultsAvaliable;                       ///< ALL required RDI buffers and metadatas are available
    BOOL               m_allFDResultsAvaliable;                        ///< ALL required FD buffers are available
    BOOL               m_allResultsAvaliable;                          ///< ALL required  buffers and metadatas are available
    BOOL               m_isLLSSnapshot;                                ///< Flag to indicate LLS snapshot or not

    BOOL               m_blockPreviewForSnapshot;                      ///< If block up-coming preview during snapshot request
    Mutex*             m_pSnapshotResultMutex;                         ///< Snapshot result mutex
    Condition*         m_pSnapshotResultAvailable;                     ///< Condition to wait final snapshot buffer ready
    BOOL               m_snapshotResultAvailable;                      ///< Flag to inidicate if final snapshot result availabe
    volatile UINT32    m_aPauseInProgress;                             ///< Is pause in progress

    struct MfsrStageResult
    {
        VOID*            pMetadata;                                                    ///< Metadata
        CHISTREAMBUFFER  refOutputBuffer[MfsrReferenceMax];                            ///< Ref buffer placeholder
    };

    struct MfsrStageSingleResult
    {
        VOID*            pMetadata;                                                    ///< Metadata
        CHISTREAMBUFFER  refOutputBuffer;                                              ///< Ref buffer placeholder
    };

    UINT32                m_numExpectedStageBuffers;
    MfsrStageResult       m_preFilterStageResult;
    MfsrStageSingleResult m_blendStageResult;
    MfsrStageSingleResult m_postFilterStageResult;
    CHISTREAMBUFFER       m_bpsRegResultBuffer;

    UINT32                m_remainingPrefilterStageResults;
    UINT32                m_remainingBlendStageResults;
    UINT32                m_remainingPostFilterStageResults;
    UINT32                m_remainingSnapshotStageResults;

    /// Request thread info
    PerThreadData         m_offlineRequestProcessThread;                   ///< Thread to process the results
    Mutex*                m_pOfflineRequestMutex;                          ///< App Result mutex
    Condition*            m_pOfflineRequestAvailable;                      ///< Wait till SensorListener results
                                                                           ///< are available
    volatile BOOL         m_offlineRequestProcessTerminate;                ///< Indication to SensorListener result
                                                                           ///<  thread to terminate itself
    camera3_capture_request_t  m_captureRequest;

private:
    // Do not allow the copy constructor or assignment operator
    FeatureMFSR(const FeatureMFSR&) = delete;
    FeatureMFSR& operator= (const FeatureMFSR&) = delete;
    INT GetTargetIndex(ChiTargetPortDescriptorInfo* pTargetsInfo, const char* pTargetName);

    CDKResult SubmitOfflineMfsrRequest(
        UINT32                     frameNumber,
        camera3_capture_request_t* pRequest,
        UINT32                     rtPipelineReqId);

    CDKResult SubmitOfflineMfsrPrefilterRequest(
        UINT32                     frameNumber,
        camera3_capture_request_t* pRequest,
        MFSRInputInfo*             pMfsrInputInfo,
        CameraType                 type);

    CDKResult SubmitOfflineMfsrBlendRequest(
        UINT32                     frameNumber,
        camera3_capture_request_t* pRequest,
        MFSRInputInfo*             pMfsrInputInfo,
        CameraType                 type);

    CDKResult SubmitOfflineMfsrPostfilterRequest(
        UINT32                     frameNumber,
        camera3_capture_request_t* pRequest,
        MFSRInputInfo*             pMfsrInputInfo,
        CameraType                 type);

    CDKResult SubmitOfflineMfsrSnapshotRequest(
        UINT32                     frameNumber,
        camera3_capture_request_t* pRequest,
        CameraType                 type);

    CDKResult TriggerInternalLLSRequests(
        camera3_capture_request_t*  pRequest);

    CDKResult GenerateLLSRequestSettings(
        const camera_metadata_t*    pInputSetting,
        UINT32                      numFrames,
        const camera_metadata_t**   pOutputSettingArray);

    CAMX_INLINE UINT32 GetRequiredInputFrames()
    {
        return m_mfsrTotalNumFrames;
    }

    CDKResult WaitForRDIResultsReady();

    CDKResult GetOutputBuffer(
        CHIBufferManager*   pBufferManager,
        ChiStream*          pChiStream,
        CHISTREAMBUFFER*    pOutputBuffer);

    CDKResult ExecuteMfsrRequest(
        MFSRStage           pipelineStage,
        UINT32              frameNumber,
        UINT32              numOutputs,
        CHISTREAMBUFFER*    pOutputBuffers,
        UINT32              numInputs,
        CHISTREAMBUFFER*    pInputBuffers,
        const VOID*         pSettings);

    UINT GetMFSRTotalFramesByGain(
        camera_metadata_t* pMeta);

    CDKResult PublicMFSRTotalFrames(
        camera_metadata_t* pMeta);

    CDKResult CreateInternalBuffers(CameraType type);

    VOID InitializeInternalStreams(CameraType type);
    VOID SetupInternalStreams(CameraType type, UINT32 width, UINT32 height);

    VOID SetupInternalPipelines(CameraType type);
    VOID SetupInternalMFSRPreFilterPipeline(CameraType type);
    VOID SetupInternalMFSRBlendPipeline(CameraType type);
    VOID SetupInternalMFSRPostFilterPipeline(CameraType type);
    VOID SetupInternalMFSRSnapshotPipeline(CameraType type);
    UINT GetPipelineIndex(MFSRStage pipelineStage);

    /// Main entry function for the Request thread
    static VOID*    RequestThread(VOID* pArg);
    VOID            RequestThreadProcessing();
};

#endif // CHXFEATUREMFSR_H
