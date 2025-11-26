////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxfeaturequadcfa.h
/// @brief CHX feature quadcfa class declarations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CHXFEATUREQUADCFA_H
#define CHXFEATUREQUADCFA_H

#include <assert.h>

#include "chistatspropertydefines.h"
#include "chxincs.h"
#include "chxfeature.h"
#include "chxusecaseutils.h"

// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files

struct TargetBuffer;

class FeatureQuadCFA : public Feature
{
public:
    static  FeatureQuadCFA* Create(
        AdvancedCameraUsecase* pUsecase);

    CDKResult           Initialize(
        AdvancedCameraUsecase* pUsecase);

    virtual VOID        Destroy(BOOL isForced);
    virtual VOID        Pause(BOOL isForced);

    virtual ChiUsecase* OverrideUsecase(
        LogicalCameraInfo*              pCameraInfo,
        camera3_stream_configuration_t* pStreamConfig);

    virtual VOID        PipelineCreated(
        UINT32 sessionId,
        UINT32 pipelineIndex);

    virtual CDKResult   ExecuteProcessRequest(
        camera3_capture_request_t*  pRequest);

    virtual VOID        ProcessResult(
        CHICAPTURERESULT*           pResult,
        VOID*                       pPrivateCallbackData);

    virtual VOID        ProcessMessage(
        const CHIMESSAGEDESCRIPTOR* pMessageDescriptor,
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
    /// @param  frameNum  Frame number for which the CHI Partial data should be populated
    ///
    /// @return VOID
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual VOID ProcessCHIPartialData(
        UINT32    frameNum,
        UINT32    sessionId);

    virtual FeatureType GetFeatureType()
    {
        return FeatureType::QuadCFA;
    }

    virtual BOOL StreamIsInternal(
        ChiStream* pStream)
    {
        return ((pStream == m_pFullSizeRawStream) || (pStream == m_pFdStream) || (pStream == m_pRemosaicRawStream) ||
                ((FALSE == IsYuvSnapshotFromAPP()) && (pStream == m_pQcfaBayer2YuvStream))) ? TRUE : FALSE;
    }

    virtual INT32 GetRequiredPipelines(
        AdvancedPipelineType* pPipelines,
        INT32 size);

    CHX_INLINE BOOL IsYuvSnapshotFromAPP()
    {
        return m_isYuvSnapshotFromApp;
    }

    CHX_INLINE BOOL IsJPEGOutputRequired()
    {
        return ((FALSE == IsYuvSnapshotFromAPP()) && (FALSE == m_isSeparateFeatureForJpeg));
    }

protected:
    FeatureQuadCFA() = default;
    virtual ~FeatureQuadCFA() = default;

    static const UINT   InvalidStreamIndex   = 0xFFFFFFFF;                ///< Invalid Stream idx
    static const UINT   InvalidSessionId     = 0xFFFFFFFF;                ///< Invalid session id
    static const UINT   InvalidPipelineIndex = 0xFFFFFFFF;                ///< Invalid pipeline index

    ChiUsecase*        m_pChiUsecase;                                     ///< Copy of ZSL usecase that gets adjusted
    ChiStream*         m_pRdiStream;                                      ///< Allocated internal RDI stream
    ChiStream*         m_pFdStream;                                       ///< Allocated internal FD stream
    ChiStream*         m_pFullSizeRawStream;                              ///< Internal full size raw stream
    ChiStream*         m_pRemosaicRawStream;                              ///< Internal Remosaic Raw stream
    ChiStream*         m_pPreviewStream;                                  ///< Tracking of the stream used for preview
    ChiStream*         m_pDummyPreviewStream;                             ///< Dummy preview stream for full size raw pipeline
    ChiStream*         m_pSnapshotStream;                                 ///< Tracking of the stream used for snapshot
    ChiStream*         m_pQcfaBayer2YuvStream;                            ///< Internal full size bayer2yuv stream
    ChiStream*         m_pJPEGInputStream;                                ///< Internal Jpeg input stream
    ChiStream*         m_pVideoStream;                                    ///< Tracking of the stream used for video
    UINT               m_rdiStreamIndex;                                  ///< Stream/target index for the RDI stream
    UINT               m_fdStreamIndex;                                   ///< Stream/target index for the FD   stream
    TargetBuffer*      m_pRdiTargetBuffer;                                ///< TargetBuffer in the CameraUsecase
    TargetBuffer*      m_pFdTargetBuffer;                                 ///< FD  TargetBuffer in the CameraUsecase
    UINT               m_fullSizeRawStreamIdx;                            ///< Stream/target index for the RDI stream
    TargetBuffer*      m_pFullSizeRawTargetBuffer;                        ///< TargetBuffer in the CameraUsecase
    UINT               m_remosaicRawStreamIdx;                            ///< Stream/target index for the Remosaic RDI stream
    TargetBuffer*      m_pRemosaicRawTargetBuffer;                        ///< Remosaic TargetBuffer in the CameraUsecase
    UINT               m_bayer2YuvStreamIdx;                              ///< Stream/target index for the internal bayer2yuv stream
    TargetBuffer*      m_pBayer2YuvTargetBuffer;                          ///< TargetBuffer for internal bayer2yuv stream
    UINT32             m_maxSnapshotReqId;                                ///< Last valid ID to move snapshotReqId to
                                                                          ///  Owned by the main thread
    UINT32             m_snapshotReqId;                                   ///< Next ID to provide snapshot request
    UINT32             m_snapshotReqIdToFrameNum[MaxOutstandingRequests]; ///< Mapping of snapshotReqId to framework frame

    ChiMetadata*       m_pSnapshotInputMeta[MaxOutstandingRequests];      ///< The metadata for the request
    CHISTREAMBUFFER    m_snapshotBuffers[MaxOutstandingRequests][2];      ///< Result buffers from app for snapshot
    UINT               m_snapshotBufferNum[MaxOutstandingRequests];       ///< Buffer count
    UINT               m_snapshotInputNum[MaxOutstandingRequests];        ///< Input frame number needed for snapshot.

    CHIPRIVDATA        m_privData[MaxOutstandingRequests];

    BOOL               m_isSnapshotFrame[MaxOutstandingRequests];         ///< Is a snapshot request
    Mutex*             m_pResultMutex;                                    ///< Result mutex
    BOOL               m_isFlashRequired;                                 ///< Indicates if Flash is required for snapshot

    Mutex*             m_pSnapshotRDIResultMutex;                         ///< Snapshot RDI Result availability mutex
    Condition*         m_pSnapshotRDIResultAvailable;                     ///< Wait till all realtime results are available

    UINT32             m_snapshotAppFrameNumber;                          ///< Snapshot Frame number

    UINT               m_lastRdiFrameAvailable;                           ///< Last valid frame number of
                                                                          ///  available rdi buffer
    UINT               m_lastRealtimeMetadataAvailable;                   ///< Last valid frame number of
                                                                          ///  available metadata from realtime
    UINT               m_offlineRequestWaitingForFrame;                   ///< Real time frame number for which offline request
                                                                          ///  is waiting(for rdi and meta availability).
    BOOL               m_isMultiFramesSnapshot;                           ///< If multi frame snapshot or not
    BOOL               m_isYuvSnapshotFromApp;                            ///< If APP config yuv_420_888 as snpashot format
    BOOL               m_isSeparateFeatureForJpeg;                        ///< If Jpeg pipeline is in a separate Feature
    UINT32             m_registeredPipelineCount;                         ///< Total pipelines created in this Feature

    /// Offline Request thread info
    PerThreadData      m_offlineRequestProcessThread;                     ///< Thread to process the results
    Mutex*             m_pOfflineRequestMutex;                            ///< App Result mutex
    Condition*         m_pOfflineRequestAvailable;                        ///< Wait till SensorListener results
                                                                          ///  are available
    volatile BOOL      m_offlineRequestProcessTerminate;                  ///< Indication to SensorListener result
                                                                          ///  thread to terminate itself
    volatile UINT32    m_aPauseInProgress;                                ///< Is flush in progress

    VOID*              m_EmptyMetaData;                                   ///< Empty MetaData

    UINT32             m_previewPipelineIndex;
    UINT32             m_fullSizeRawPipelineIndex;
    UINT32             m_remosaicPipelineIndex;
    UINT32             m_bayer2YuvPipelineIndex;
    UINT32             m_snapshotPipelineIndex;
    UINT32             m_previewSessionId;
    UINT32             m_fullSizeRawSessionId;
    UINT32             m_remosaicSessionId;
    UINT32             m_bayer2YuvSessionId;
    UINT32             m_snapshotSessionId;
    BOOL               m_continuousRdiCapture;
    CHIREMOSAICTYPE    m_RemosaicType;
    BOOL               m_isSkipPreview[MaxOutstandingRequests];          ///< If skip preview frame or not
    DebugData          m_localDebugData;                                 ///< Local copy of debug data for offline sessions

private:
    // Do not allow the copy constructor or assignment operator
    FeatureQuadCFA(const FeatureQuadCFA&) = delete;
    FeatureQuadCFA& operator= (const FeatureQuadCFA&) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// SubmitRequestToSession
    ///
    /// @brief  Helper function to sumibt CHICAPTUREREQUEST to camx session
    ///
    /// @param  sessionId         Session id
    /// @param  pSession          Session object
    /// @param  numRequests       Number of requests
    /// @param  pCaptureRequests  Detail info for requests, including input/output metadatas and buffers, etc.
    ///
    /// @return CDKResult
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CDKResult SubmitRequestToSession(
        UINT32                   sessionId,
        const Session*           pSession,
        UINT32                   numRequests,
        const CHICAPTUREREQUEST* pCaptureRequests);

    /// Main entry function for the offline Request thread
    static VOID* RequestThread(VOID* pArg);

    VOID RequestThreadProcessing();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// AllocateLocalDebugDataBuffer
    ///
    /// @brief  Allocate local debug data buffer
    ///
    /// @param  size  the size of debug data buffer
    ///
    /// @return CDKResultSuccess if buffer is allocated successfully, CDKResultEFailed otherwise
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CDKResult AllocateLocalDebugDataBuffer(SIZE_T size);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DestroyLocalDebugData
    ///
    /// @brief  Destroy the local debug data buffer
    ///
    /// @return none
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID DestroyLocalDebugData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// SaveDebugDataToLocal
    ///
    /// @brief  Save a local copy of debug data in CHI,
    ///
    /// @param  pChiMeta    Chi metadata buffer
    ///
    /// @return CDKResult
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CDKResult SaveDebugDataToLocal(ChiMetadata* pChiMeta);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// UpdateDebugDataPtr
    ///
    /// @brief  Save a local copy of debug data in CHI if needed,
    ///         and update debug data pointer in metadata to the local saved copy
    ///
    /// @param  pChiMeta    Chi metadata buffer
    /// @param  saveOrigin  save the original debug data in the metadata to local copy
    ///
    /// @return CDKResult
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CDKResult UpdateDebugDataPtr(
        ChiMetadata* pChiMeta,
        BOOL         saveOrigin);

    /// check if there is snapshot stream for this request
    BOOL IsSnapshotRequest(
        camera3_capture_request_t* pRequest) const
    {
        BOOL isSnapshotRequest = FALSE;
        for (UINT i = 0; i < pRequest->num_output_buffers; i++)
        {
            if ((NULL != m_pSnapshotStream) &&
                (CHISTREAM *)pRequest->output_buffers[i].stream == m_pSnapshotStream)
            {
                isSnapshotRequest = TRUE;
                break;
            }
        }
        return isSnapshotRequest;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// RegisterPipelinesToUsecase
    ///
    /// @brief  Register pipelines to usecase, and return the unique session id for this pipeline group,
    ///         all the pipelines in the same group share the same session id
    ///
    /// @param  numPipelinesInGroup   Number of pipelines in this group
    /// @param  pipelineGroup         Pipeline types in this group
    /// @param  cameraIds             Camera IDs for the pipelines in this group
    /// @param  pRegisteredPipelines  Output param to store the registered pipelines
    ///
    /// @return Unique session id
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT32 RegisterPipelinesToUsecase(
        UINT32                numPipelinesInGroup,
        AdvancedPipelineType* pipelineGroup,
        UINT32*               cameraIds,
        AdvancedPipelineType* pRegisteredPipelines);

    CDKResult   WaitFullsizeRDIReady(
        UINT32 snapshotRDISeqId);

    CDKResult   WaitRemosaicRDIReady(
        UINT32 snapshotRDISeqId);

    CDKResult   StartFullsizeRDISession(
        BOOL restartOfflineSessions);

    CDKResult   ReStartPreviewSession(
        BOOL restartOfflineSessions);

    CDKResult   HandlePreviewRequest(
        camera3_capture_request_t*  pRequest);

    CDKResult   HandleSnapshotRequest(
        camera3_capture_request_t*  pRequest);

    /// For multi frame snapshot, need call ExecuteProcessRequest to another Feature
    CDKResult   HandleMultiFramesSnapshot(
        camera3_capture_request_t*  pRequest);

    CDKResult   HandleSingleFrameSnapshot(
        camera3_capture_request_t*  pRequest);

    /// Returns RemosaicType for QuadCFA sensor, UnKnown is not support
    CHIREMOSAICTYPE GetRemosaicType(
        const LogicalCameraInfo* pCamInfo);

    CDKResult GenerateFullsizeRDIRequest(
        camera3_capture_request_t*  pRequest,
        ChiMetadata*                pRTInputMetadata);

    CDKResult GenerateRemosaicRequest(
        UINT               frameNumber,
        UINT               requestIdIndex,
        UINT32             snapshotReqId,
        ChiMetadata*       pInputMetadata,
        ChiMetadata*       pOutputMetadata,
        CHISTREAMBUFFER*   pInputBuffer);

    CDKResult GenerateSnapshotJpegRequest(
        UINT               frameNumber,
        UINT               requestIdIndex,
        UINT32             snapshotReqId,
        ChiMetadata*       pInputMetadata,
        ChiMetadata*       pOutputMetadata,
        CHISTREAMBUFFER*   pInputBuffer,
        CHIBufferManager*  pInputBufferManager);
};

#endif // CHXFEATUREQUADCFA_H
