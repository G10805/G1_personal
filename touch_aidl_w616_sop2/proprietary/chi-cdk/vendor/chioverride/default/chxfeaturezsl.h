// NOWHINE FILE PR007b: Whiner incorrectly concludes as non-library files////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxfeaturezsl.h
/// @brief CHX feature zsl class declarations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CHXFEATUREZSL_H
#define CHXFEATUREZSL_H

#include <assert.h>

#include "chxincs.h"
#include "chxfeature.h"

// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files

static const UINT InvalidFrameNum   = 0xFFFFFFFF;

struct TargetBuffer;

class FeatureZSL : public Feature
{
public:
    static  FeatureZSL* Create(
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
        return FeatureType::ZSL;
    }

    virtual BOOL StreamIsInternal(
        ChiStream* pStream)
    {
        return (pStream == m_pRdiStream || pStream == m_pFdStream) ? TRUE : FALSE;
    }

    virtual INT32 GetRequiredPipelines(
        AdvancedPipelineType* pPipelines,
        INT32 size);

protected:
    FeatureZSL() = default;
    virtual ~FeatureZSL() = default;

    // Checks if Psuedo ZSL is enabled
    BOOL IsPseudoZSL(
       ChiMetadata* pMetadata);


    ChiUsecase*        m_pChiUsecase;                                     ///< Copy of ZSL usecase that gets adjusted
    ChiStream*         m_pRdiStream;                                      ///< Allocated internal RDI stream
    ChiStream*         m_pFdStream;                                       ///< Allocated internal FD stream
    ChiStream*         m_pPreviewStream;                                  ///< Tracking of the stream used for preview
    ChiStream*         m_pSnapshotStream;                                 ///< Tracking of the stream used for snapshot
    ChiStream*         m_pVideoStream;                                    ///< Tracking of the stream used for video
    UINT               m_rdiStreamIndex;                                  ///< Stream/target index for the RDI stream
    UINT               m_fdStreamIndex;                                   ///< Stream/target index for the FD   stream
    TargetBuffer*      m_pRdiTargetBuffer;                                ///< RDI TargetBuffer in the CameraUsecase
    TargetBuffer*      m_pFdTargetBuffer;                                 ///< FD  TargetBuffer in the CameraUsecase
    UINT32             m_maxSnapshotReqId;                                ///< Last valid ID to move snapshotReqId to
                                                                          ///  Owned by the main thread
    UINT32             m_snapshotReqId;                                   ///< Next ID to provide snapshot request
    UINT32             m_snapshotReqIdToFrameNum[MaxOutstandingRequests]; ///< Mapping of snapshotReqId to framework frame
    UINT32             m_ZSLInputRDIReqId[MaxOutstandingRequests];        ///< Mapping of snapshotReqId to the RDI ReqId


    ChiMetadata*       m_pSnapshotInputMeta[MaxOutstandingRequests];      ///< The metadata for the request
    CHISTREAMBUFFER    m_snapshotBuffers[MaxOutstandingRequests][2];      ///< Result buffers from app for snapshot
    UINT               m_snapshotBufferNum[MaxOutstandingRequests];       ///< Buffer count

    CHIPRIVDATA        m_privData[MaxOutstandingRequests];

    BOOL               m_isSnapshotFrame[MaxOutstandingRequests];         ///< Is a snapshot request
    Mutex*             m_pResultMutex;                                    ///< Result mutex
    BOOL               m_isFlashRequired;                                 ///< Indicates if Flash is required for snapshot
    BOOL               m_isFlashFired;                                      ///< Indicates if Flash is fired or not

    /// Offline Request thread info
    PerThreadData      m_offlineRequestProcessThread;                     ///< Thread to process the results
    Mutex*             m_pOfflineRequestMutex;                            ///< App Result mutex
    Condition*         m_pOfflineRequestAvailable;                        ///< Wait till SensorListener results
                                                                          ///  are available
    volatile BOOL      m_offlineRequestProcessTerminate;                  ///< Indication to SensorListener result
                                                                          ///  thread to terminate itself
    volatile UINT32    m_aPauseInProgress;                                ///< Is flush in progress

    UINT32 m_previewPipelineIndex;
    UINT32 m_snapshotPipelineIndex;
    UINT32 m_previewSessionId;
    UINT32 m_snapshotSessionId;
    BOOL   m_continuousRdiCapture;

    UINT32 m_realtimeMetaClientId;                                        ///< Metadata client ID for the realtime pipeline
    UINT32 m_offlineMetaClientId;                                         ///< Metadata client ID for the offline pipeline

    BOOL   m_isSkipPreview[MaxOutstandingRequests];                       ///< If skip preview frame or not
    BOOL   m_isFutureFrameSnapshot[MaxOutstandingRequests];               ///< Flag to indicate to use future RDI for snapshot or not


private:
    // Do not allow the copy constructor or assignment operator
    FeatureZSL(const FeatureZSL&) = delete;
    FeatureZSL& operator= (const FeatureZSL&) = delete;

    /// Main entry function for the offline Request thread
    static VOID* RequestThread(VOID* pArg);

    VOID RequestThreadProcessing();

    VOID GetResultFrameInfo(
        UINT32    sessionId,
        UINT32 captureResultframeworkFrameNum,
        UINT32 *pResultFrameNumber,
        UINT32 *pResultFrameIndex);

    VOID*      m_EmptyMetaData;         ///< Empty MetaData
    UINT32     m_lastSnapshotInput;
};

#endif // CHXFEATUREZSL_H
