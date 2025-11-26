////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxfeaturemultiframe.h
/// @brief CHX feature multiframe class declarations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CHXFEATUREMULTIFRAME_H
#define CHXFEATUREMULTIFRAME_H

#include <assert.h>

#include "chxincs.h"
#include "chxfeature.h"
#include "chxusecase.h"
#include "chxusecaseutils.h"

// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files

static const UINT32 MaxMultiFrameRequests = 8;              ///< maximum snapshot that can be triggered back2back
static const UINT   MaxInputPorts         = 10;             ///< Max Input Port Numbers for HDR Node
static const UINT   MaxMultiFrames        = MaxInputPorts;  ///< Max Input Frame Numbers for HDR Node

/// @brief Structure that represents all info required to submit capture request offline
typedef struct ChiOfflinelineRequest
 {
    CHICAPTUREREQUEST       request;
    CHIPIPELINEREQUEST      submitRequest;
    CHISTREAMBUFFER         outputBuffer;
    CHISTREAMBUFFER         inputBuffer[MaxMultiFrames];
    ChiMetadata*            pRequestMetadata;
 } CHXOFFLINEREQUEST;

class FeatureMultiframe : public Feature
{
public:
    static  FeatureMultiframe* Create(
        AdvancedCameraUsecase* pUsecase,
        UINT32                 rtIndex);

    CDKResult           Initialize(
        AdvancedCameraUsecase* pUsecase);

    virtual VOID        Destroy(BOOL isForced);
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

    virtual CDKResult   ExecuteFastShutterSnapshotRequest(
        camera3_capture_request_t*  pRequest,
        ChiMetadata*                pInputMeta,
        UINT                        snapshotCount);

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
        return FeatureType::SWMF;
    }

    virtual BOOL StreamIsInternal(
        ChiStream* pStream)
    {
        BOOL isInternal = ((pStream == m_pRdiStream) || (pStream == m_pFdStream));
        return isInternal;
    }

    virtual INT32 GetRequiredPipelines(
        AdvancedPipelineType* pPipelines,
        INT32 size);

    virtual UINT32 GetRequiredFramesForSnapshot(
        const camera_metadata_t *pMetadata)
    {
        (VOID)pMetadata;
        return m_numMultiFramesRequired;
    }
    virtual UINT32 GetMaxRequiredFramesForSnapshot(
        const camera_metadata_t *pMetadata)
    {
        (VOID)pMetadata;
        return m_numMultiFramesRequired;
    }

protected:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetOutputBufferManager
    ///
    /// @brief  The implementation of this function obtains the ptr of the buffer manager for the given
    ///         output buffer. Must be implemented per feature.
    ///
    /// @param  pOutputBuffer Output Buffer pointer
    ///
    /// @return CHIBufferManager* The buffer manager of the output buffer
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CHIBufferManager* GetOutputBufferManager(
        CHISTREAMBUFFER* pOutputBuffer);

    FeatureMultiframe() = default;
    virtual ~FeatureMultiframe() = default;


    ChiUsecase*         m_pChiUsecase;                                       ///< Copy of ZSL usecase that gets adjusted
    ChiStream*          m_pRdiStream;                                        ///< Allocated internal RDI stream
    ChiStream*          m_pFdStream;                                         ///< Allocated internal FD stream
    ChiStream*          m_pPreviewStream;                                    ///< Tracking of the stream used for preview
    ChiStream*          m_pSnapshotStream;                                   ///< Tracking of the stream used for snapshot
    UINT                m_rdiStreamIndex;                                    ///< Stream/target index for the RDI stream
    UINT                m_fdStreamIndex;                                     ///< Stream/target index for the FD  stream
    TargetBuffer*       m_pRdiTargetBuffer;                                  ///< TargetBuffer in the CameraUsecase
    TargetBuffer*       m_pFdTargetBuffer;                                   ///< TargetBuffer in the CameraUsecase
    UINT64              m_shutterTimestamp[MaxOutstandingRequests];          ///< Tracking of shutter timestamp for each request
    UINT32              m_maxSnapshotReqId;                                  ///< Last valid ID to move snapshotReqId to
                                                                             ///  Owned by the main thread
    UINT32              m_snapshotReqIdToFrameNum[MaxOutstandingRequests];   ///< Mapping of snapshotReqId to framework frame

    ChiMetadata*        m_pSnapshotInputMeta[MaxOutstandingRequests];        ///< The metadata for the request
    CHISTREAMBUFFER     m_snapshotBuffers[MaxOutstandingRequests][2];        ///< Result buffers from app for snapshot
    UINT                m_snapshotBufferNum[MaxOutstandingRequests];         ///< Buffer count

    BOOL                m_isSnapshotFrame[MaxOutstandingRequests];           ///< Is a snapshot request
    CHIPRIVDATA         m_privData[MaxOutstandingRequests];                  ///< Private data for each request

    CHIPRIVDATA         m_realtimePrivData[MaxOutstandingRequests];          ///< Private data for each request
    CHIPRIVDATA         m_offlinePrivData[MaxOutstandingRequests];           ///< Private data for each request
                                                                             ///  Having two private data arrays to prevent data being
                                                                             ///  overwritten between pipelines when the reqIDs are the same

    UINT32              m_snapshotPipelineIndex;                             ///< Snapshot Pipeline Index
    UINT32              m_mergePipelineIndex;                                ///< HDR Merge Pipeline Index
    UINT32              m_bayer2YuvPipelineIndex;                            ///< Bayer2Yuv Pipeline Index
    UINT32              m_previewPipelineIndex;                              ///< Preivew Pipeline Index
    UINT32              m_snapshotSessionId;                                 ///< Snapshot Session Id
    UINT32              m_mergeSessionId;                                    ///< HDR Merge Session Id
    UINT32              m_bayer2YuvSessionId;                                ///< Bayer2Yuv Session Id
    UINT32              m_previewSessionId;                                  ///< Preview Session Id

    static const UINT   DefaultSWMFNumFrames     = 3;                        ///< Default number of frames required for SWMF
    static const UINT   MinOutputBuffers         = 8;                        ///< Minimum number of Chi output buffers
    static const UINT   m_SWMFNumFramesforFS     = 5;                        ///< num of frames for SWMF in FastShutter mode
    static const UINT   WaitAllPreviewResultTime = 500;                      ///< Wait time for all preview results

    // Multiframe stage-1
    ChiStream*          m_pBayer2YuvStream;                                  ///< Tracking of internal stream used for stage-1
    UINT                m_Bayer2YuvStreamIndex;                              ///< Stream/target index for the stage-1 stream
    TargetBuffer*       m_pBayer2YuvTargetBuffer;                            ///< TargetBuffer in the CameraUsecaseBase
    ChiMetadata*        m_pBayer2YuvInputMeta[MaxOutstandingRequests];       ///< The metadata for the request
    // Multiframe stage-2
    ChiStream*          m_pMergeYuvStream;                                   ///< Tracking of internal stream used for stage-2
    UINT                m_mergeStreamIndex;                                  ///< Stream/target index for the stage-2 stream
    TargetBuffer*       m_pMergeTargetBuffer;                                ///< TargetBuffer in the CameraUsecaseBase
    ChiMetadata*        m_pMergeInputMeta[MaxOutstandingRequests];    ///< The metadata for the request
    ChiTarget           m_mergePipelineInputTargets[MaxInputPorts];          ///< InputTargets for HDR merge pipeline
    ChiTarget           m_mergePipelineOutputTarget;                         ///< OutputTarget for HDR merge pipeline
    CHIBufferManager*   m_pMergeOutputBufferManager;                         ///< BufferManager for HDR merge pipeline output
    TargetBuffer        m_mergeTargetBuffers;                                ///< TargetBuffers for HDR merge pipeline output

    CHIBufferManager*   m_pBayer2YuvOutputBufferManager;                     ///< BufferManager for BayertoYuv pipeline output
    TargetBuffer        m_bayer2YuvTargetBuffers;                            ///< TargetBuffers for BayertoYuv pipeline output

    ChiStream*          m_pJPEGInputStream;                                  ///< Common JPEG input stream

    UINT32              m_numMultiFramesRequired;                            ///< Number of yuv frames required
    CHITAGSOPS          m_vendorTagOps;                                      ///< Vendor Tag Ops
    UINT32              m_lastShutterFrameNum;                               ///< Latest received shutter frame number
    UINT32              m_numBayer2YuvFrames;                                ///< Currently received bayer2yuv frames
    UINT32              m_numSnapshotRdiFramesreceived;                      ///< Currently received Snapshot Rdi frames for FS2
    ChiStream*          m_pMergePipelineInputStreams[MaxInputPorts];         ///< Input streams for HDR merge pipeline
    UINT32              m_snapshotAppFrameNum;                               ///< App frame number for snapshot

    UINT32              m_masterCameraId;                                    ///< Master Camera Id
    UINT32              m_activePipelineID;                                  ///< Active Pipeline ID

    /// Offline Request thread info
    PerThreadData      m_offlineRequestProcessThread;                        ///< Thread to process the results
    Mutex*             m_pOfflineRequestMutex;                               ///< App Result mutex
    Condition*         m_pOfflineRequestAvailable;                           ///< Wait till SensorListener results
                                                                             ///  are available
    volatile BOOL      m_offlineRequestProcessTerminate;                     ///< Indication to SensorListener result
                                                                             ///  thread to terminate itself
    CHXOFFLINEREQUEST  m_offlineRequestData[MaxMultiFrameRequests];
    UINT32             m_offlineThreadCaptureRequestId;
    UINT32             m_offlineThreadCaptureRequestSubmitId;
    volatile UINT32    m_aPauseInProgress;                                   ///< Is pause in progress
    UINT32             m_internalFrameNum;                                   ///< Internal frame number of HDR request
    UINT32             m_lastPreviewFrameNum;                                ///< Last preview frame before FS2 snapshot request.
    Mutex*             m_pLastPreviewRequestMutex;                           ///< Last preview result mutex
    Condition*         m_pLastPreviewResultAvailable;                        ///< Wait till Last preview results

private:
    // Do not allow the copy constructor or assignment operator
    FeatureMultiframe(const FeatureMultiframe&) = delete;
    FeatureMultiframe& operator= (const FeatureMultiframe&) = delete;

    // Check if JPEG output stream is required
    BOOL isJPEGOutputRequired() const;

    // Configure feature output target stream
    CDKResult ConfigureTargetStream();


    // Dump Debug/Tuning data
    DebugData           m_debugDataOffline;                             ///< Offline copy for debug-data

    // Helper to submit request
    VOID SubmitRequest(
        UINT32              sessionIdx,
        UINT                frameNumber,
        TargetBuffer*       pTargetBuffer,
        CHISTREAMBUFFER*    pOutputBuffer,
        UINT32              inputPipelineReqId,
        UINT32              bufferIndex,
        BOOL                isBatched,
        const CHAR*         identifierData);
    /// Main entry function for the offline Request thread
    static VOID* RequestThread(VOID* pArg);
    VOID RequestThreadProcessing();
};

#endif // CHXFEATUREMULTIFRAME_H
