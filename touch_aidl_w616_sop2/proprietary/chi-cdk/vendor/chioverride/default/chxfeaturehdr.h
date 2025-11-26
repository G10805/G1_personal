////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxfeaturehdr.h
/// @brief CHX feature HDR class declarations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CHXFEATUREHDR_H
#define CHXFEATUREHDR_H

#include <assert.h>

#include "chxincs.h"
#include "chxfeature.h"
#include "chxusecase.h"
#include "chxusecaseutils.h"

// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files

struct internalResultDescriptor
{
    CHISTREAMBUFFER buffer;
    const VOID*     pMetadata;
    UINT32          internalRequestId;
    UINT32          flag;
};

struct Bayer2YuvResultQ
{
    UINT32                   frameNumber;                         ///< Frame number
    UINT32                   validFrames;
    internalResultDescriptor resultDescriptor[10];
};


class FeatureHDR : public Feature
{
public:
    static  FeatureHDR* Create(
        AdvancedCameraUsecase* pUsecase,
        UINT32 realtimePipelineIndex);

    CDKResult           Initialize(
        AdvancedCameraUsecase* pUsecase);

    virtual VOID        Destroy(BOOL isForced);

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

    virtual VOID        Pause(BOOL isForced);

    virtual VOID        ProcessMessage(
        const CHIMESSAGEDESCRIPTOR* pMessageDescriptor,
        VOID*                       pPrivateCallbackData);

    virtual CDKResult   GetRequestInfo(
        camera3_capture_request_t*  pRequest,
        FeatureRequestInfo*         pOutputRequests,
        FeatureRequestType          requestType);

    virtual FeatureType GetFeatureType()
    {
        return FeatureType::HDR;
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

    FeatureHDR() = default;
    virtual ~FeatureHDR() = default;


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

    ChiMetadata*        m_pApplicationInputMeta;                             ///< Reference for app metadata for snapshot
    CHISTREAMBUFFER     m_snapshotBuffers[MaxOutstandingRequests][2];        ///< Result buffers from app for snapshot
    UINT                m_snapshotBufferNum[MaxOutstandingRequests];         ///< Buffer count

    BOOL                m_isSnapshotFrame[MaxOutstandingRequests];           ///< Is a snapshot request

    CHIPRIVDATA         m_realtimePrivData[MaxOutstandingRequests];          ///< Private data for each realtime request
    CHIPRIVDATA         m_offlinePrivData[MaxOutstandingRequests];           ///< Private data for each offline request
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

    static const UINT   MaxInputPorts            = 10;                       ///< Max Input Port Numbers for HDR Node
    static const UINT   MaxMultiFrames           = MaxInputPorts;            ///< Max Input Frame Numbers for HDR Node
    static const UINT   MinOutputBuffers         = 3;                        ///< Minimum number of Chi output buffers

    // Multiframe stage-1
    ChiStream*          m_pBayer2YuvStream;                                  ///< Tracking of internal stream used for stage-1
    UINT                m_Bayer2YuvStreamIndex;                              ///< Stream/target index for the stage-1 stream
    TargetBuffer*       m_pBayer2YuvTargetBuffer;                            ///< TargetBuffer in the CameraUsecaseBase
    // Multiframe stage-2
    ChiStream*          m_pMergeYuvStream;                                   ///< Tracking of internal stream used for stage-2
    UINT                m_mergeStreamIndex;                                  ///< Stream/target index for the stage-2 stream
    TargetBuffer*       m_pMergeTargetBuffer;                                ///< TargetBuffer in the CameraUsecaseBase
    ChiTarget           m_mergePipelineInputTargets[MaxInputPorts];          ///< InputTargets for HDR merge pipeline
    ChiTarget           m_mergePipelineOutputTarget;                         ///< OutputTarget for HDR merge pipeline
    CHIBufferManager*   m_pMergeOutputBufferManager;                         ///< BufferManager for HDR merge pipeline output
    TargetBuffer        m_mergeTargetBuffers;                                ///< TargetBuffers for HDR merge pipeline output

    CHIBufferManager*   m_pBayer2YuvOutputBufferManager;                     ///< BufferManager for BayertoYuv pipeline output
    TargetBuffer        m_bayer2YuvTargetBuffers;                            ///< TargetBuffers for BayertoYuv pipeline output

    ChiStream*          m_pJPEGInputStream;                                  ///< Common JPEG input stream

    INT32               m_expValues[5];                                      ///< Different exposure values for HDR
    UINT32              m_numAeBracketFrames;                                ///< Number of AE Bracket frames for HDR
    CHITAGSOPS          m_vendorTagOps;                                      ///< Vendor Tag Ops
    UINT32              m_previewRawReqId;                                   ///< Internal request id for realtime pipeline
    UINT32              m_lastShutterFrameNum;                               ///< Latest received shutter frame number
    UINT32              m_numBayer2YuvFrames;                                ///< Currently received bayer2yuv frames
    ChiStream*          m_pMergePipelineInputStreams[MaxInputPorts];         ///< Input streams for HDR merge pipeline
    ChiMetadata*        m_pOverrideAppSetting[MaxMultiFrames];               ///< Overrde EV settings for realtime pipeline
    UINT32              m_firstNormalExpIdx;                                 ///< First normal exposure in AE bracket array
    UINT32              m_snapshotAppFrameNum;                               ///< App frame number for snapshot

    UINT32              m_masterCameraId;                                    ///< Master Camera Id
    UINT32              m_activePipelineID;                                  ///< Active Pipeline ID
    BOOL                m_isSkipPreview[MaxOutstandingRequests];             ///< Skip preview processing
    UINT32              m_internalFrameNum;                                  ///< Internal frame number of HDR request
private:
    // Do not allow the copy constructor or assignment operator
    FeatureHDR(const FeatureHDR&) = delete;
    FeatureHDR& operator= (const FeatureHDR&) = delete;

    // Check if JPEG output stream is required
    BOOL isJPEGOutputRequired() const;

    // Configure feature output target stream
    CDKResult ConfigureTargetStream();

    // Dump Debug/Tuning data
    DebugData           m_debugDataOffline;                             ///< Offline copy for debug-data

    // Dump hdr meta
    VOID DumpMeta(
       ChiMetadata* pChiMeta,                                           ///< Point to the metadata
       UINT index);                                                     ///< HDR request index

    // Helper to submit request
    VOID SubmitRequest(
       UINT32 sessionIdx,
       UINT             frameNumber,
       TargetBuffer*    pTargetBuffer,
       CHISTREAMBUFFER* pOutputBuffer,
       UINT32           inputPipelineReqId,
       BOOL             canInvalidate,
       const CHAR*      identifierString);

    // Generates AE bracket requests for single camera and submits to the session
    CDKResult GenerateAEBracketRequest(
       camera3_capture_request_t* pRequest);

    // Generates AE bracket requests for dual camera and submits to the session
    CDKResult GenerateAEBracketSettings(
       camera3_capture_request_t* pRequest);
};

#endif // CHXFEATUREHDR_H
