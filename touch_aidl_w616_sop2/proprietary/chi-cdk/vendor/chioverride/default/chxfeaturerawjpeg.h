////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxfeaturerawjpeg.h
/// @brief CHX feature raw + jpeg(simultaneous raw and jpeg capture) class declarations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CHXFEATURERAWJPEG_H
#define CHXFEATURERAWJPEG_H

#include <assert.h>

#include "chxincs.h"
#include "chxfeature.h"
#include "chxusecaseutils.h"

// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files

class FeatureRawJPEG : public Feature
{
public:
    static  FeatureRawJPEG* Create(
        AdvancedCameraUsecase* pUsecase);

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

    // We need one internal stream to handle Raw+JPEG.
    BOOL StreamIsInternal(
        ChiStream* pStream)
    {
        return (pStream == m_pRdiStream || pStream == m_pFdStream) ? TRUE : FALSE;
    }

protected:
    FeatureRawJPEG() = default;
    virtual ~FeatureRawJPEG() = default;
    BOOL IsPseudoZSL(ChiMetadata* pMetadata);


    ChiUsecase*        m_pChiUsecase;                                     ///< Copy of RawJPEG usecase that gets adjusted
    ChiStream*         m_pRdiStream;                                      ///< Allocated internal RDI stream
    ChiStream*         m_pFdStream;                                       ///< Allocated internal FD stream
    ChiStream*         m_pRdiFrameworkStream;                             ///< Framework RDI output stream
    ChiStream*         m_pPreviewStream;                                  ///< Tracking of the stream used for preview
    ChiStream*         m_pSnapshotStream;                                 ///< Tracking of the stream used for snapshot
    UINT               m_rdiStreamIndex;                                  ///< Stream/target index for the RDI stream
    UINT               m_fdStreamIndex;                                   ///< Stream/target index for the FD   stream
    TargetBuffer*      m_pRdiTargetBuffer;                                ///< TargetBuffer in the CameraUsecase
    TargetBuffer*      m_pFdTargetBuffer;                                 ///< FD  TargetBuffer in the CameraUsecase
    UINT64             m_shutterTimestamp[MaxOutstandingRequests];        ///< Tracking of shutter timestamp for each request
    UINT32             m_maxSnapshotReqId;                                ///< Last valid ID to move snapshotReqId to
                                                                          ///  Owned by the main thread
    UINT32             m_snapshotReqId;                                   ///< Next ID to provide snapshot request
    UINT32             m_snapshotReqIdToFrameNum[MaxOutstandingRequests]; ///< Mapping of snapshotReqId to framework frame

    ChiMetadata*       m_pSnapshotInputMeta[MaxOutstandingRequests];      ///< The metadata for the request
    CHISTREAMBUFFER    m_snapshotBuffers[MaxOutstandingRequests][2];      ///< Result buffers from app for snapshot
    UINT               m_snapshotBufferNum[MaxOutstandingRequests];       ///< Buffer count

    BOOL               m_isSnapshotFrame[MaxOutstandingRequests];         ///< Is a snapshot request
    BOOL               m_isRdiFrameRequested[MaxOutstandingRequests];     ///< Is Rdi frame requested from frameworks
    BOOL               m_isZSLSnapshotRequested[MaxOutstandingRequests];  ///< Is ZSL snapshot requested
    Mutex*             m_pResultMutex;                                    ///< Result mutex

    /// Offline Request thread info
    PerThreadData      m_offlineRequestProcessThread;                     ///< Thread to process the results
    Mutex*             m_pOfflineRequestMutex;                            ///< App Result mutex
    Condition*         m_pOfflineRequestAvailable;                        ///< Wait till SensorListener results
                                                                          ///  are available
    volatile BOOL      m_offlineRequestProcessTerminate;                  ///< Indication to SensorListener result
                                                                          ///  thread to terminate itself
    volatile UINT32    m_aPauseInProgress;                                ///< Is pause in progress

    BOOL               m_useDummyPreview;                                 ///< Whether to use internal dummy preview

    static const UINT  PreviewPipelineIndex  = 1;
    static const UINT  SnapshotPipelineIndex = 0;

    static const UINT32 DefaultPreviewWidth  = 640;                       /// default width for dummy preview stream
    static const UINT32 DefaultPreviewHeight = 480;                       /// default height for dummy preview stream
    static const UINT32 MaxStreamBuffers     = 8;                         /// Max number of stream buffers

    camera3_stream_buffer_t m_pRdiFrameworkStreamBuffer[MaxOutstandingRequests];    ///< Rdi buffer provided by framework
    BOOL                    m_isRdiFormatRaw16;                                     ///< Is Rdi format raw16

    CHIPRIVDATA             m_privData[MaxOutstandingRequests];            ///< Result private data
    CHIPRIVDATA             m_offlinePrivData[MaxOutstandingRequests];     ///< Result private data for offline pipeline

private:
    // Do not allow the copy constructor or assignment operator
    FeatureRawJPEG(const FeatureRawJPEG&) = delete;
    FeatureRawJPEG& operator= (const FeatureRawJPEG&) = delete;

    /// Main entry function for the offline Request thread
    static VOID* RequestThread(VOID* pArg);
    VOID RequestThreadProcessing();
};

#endif // CHXFEATURERAWJPEG_H
