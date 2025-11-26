/* ===========================================================================
 * Copyright (c) 2019-2020, 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#include <binder/TextOutput.h>
#include <binder/IInterface.h>
#include <binder/IBinder.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>

#include "ais_hidl_client.h"

#define CHECK_BIT(num, pos) ((num) & (0x1<<(pos)))

using namespace std;

using ::android::hardware::hidl_vec;
using ::memcpy;
using ::android::sp;

// Default constructors
ais_hidl_client::ais_hidl_client(android::sp<IQcarCamera> service): mpService(service)
{
    mInitialized = false;
    mStreaming = false;
}
ais_hidl_client::~ais_hidl_client()
{
    AIS_HIDL_DBGMSG("Destructor called for ais_hidl_client hndl %p",mQcarcamHndl);
    mInitialized = false;
    mStreaming = false;
}

void ais_hidl_client::copyPlayloadData(QCarCamEventPayload_t *pqPayload, const QCarCamEventPayload *pPayload)
{
    int i=0;
    if (pqPayload && pPayload) {
        pqPayload->u32Data = pPayload->u32Data;
        pqPayload->errInfo.errorId = static_cast<QCarCamErrorEvent_e>(pPayload->errInfo.errorId);
        pqPayload->errInfo.errorCode = pPayload->errInfo.errorCode;
        pqPayload->errInfo.frameId = pPayload->errInfo.frameId;
        pqPayload->errInfo.timestamp = pPayload->errInfo.timestamp;
        pqPayload->hwTimestamp.timestamp = pPayload->hwTimestamp.timestamp;
        pqPayload->hwTimestamp.timestampGPTP = pPayload->hwTimestamp.timestampGPTP;
        for (i=0; i<QCARCAM_MAX_VENDOR_PAYLOAD_SIZE; i++)
            pqPayload->vendorData.data[i] = pPayload->vendorData.data[i];
        pqPayload->frameInfo.id = pPayload->frameInfo.id;
        pqPayload->frameInfo.bufferIndex = pPayload->frameInfo.bufferIndex;
        pqPayload->frameInfo.seqNo = pPayload->frameInfo.seqNo;
        pqPayload->frameInfo.timestamp = pPayload->frameInfo.timestamp;
        pqPayload->frameInfo.sofTimestamp.timestamp = pPayload->frameInfo.sofTimestamp.timestamp;
        pqPayload->frameInfo.sofTimestamp.timestampGPTP = pPayload->frameInfo.sofTimestamp.timestampGPTP;
        pqPayload->frameInfo.flags = pPayload->frameInfo.flags;
        pqPayload->frameInfo.fieldType = static_cast<QCarCamInterlaceField_e>(pPayload->frameInfo.fieldType);
        pqPayload->frameInfo.requestId = pPayload->frameInfo.requestId;
        pqPayload->frameInfo.inputMetaIdx = pPayload->frameInfo.inputMetaIdx;
        for (i=0; i<QCARCAM_MAX_BATCH_FRAMES; i++) {
            pqPayload->frameInfo.batchFramesInfo[i].seqNo = pPayload->frameInfo.batchFramesInfo[i].seqNo;
            pqPayload->frameInfo.batchFramesInfo[i].timestamp.timestamp = pPayload->frameInfo.batchFramesInfo[i].timestamp.timestamp;
            pqPayload->frameInfo.batchFramesInfo[i].timestamp.timestampGPTP = pPayload->frameInfo.batchFramesInfo[i].timestamp.timestampGPTP;
        }
        pqPayload->recovery.msg = static_cast<QCarCamEventRecoveryMsg_e>(pPayload->recovery.msg);
        pqPayload->recovery.error.errorId = static_cast<QCarCamErrorEvent_e>(pPayload->recovery.error.errorId);
        pqPayload->recovery.error.errorCode = pPayload->recovery.error.errorCode;
        pqPayload->recovery.error.frameId = pPayload->recovery.error.frameId;
        pqPayload->recovery.error.timestamp = pPayload->recovery.error.timestamp;
        for (i=0; i<QCARCAM_MAX_PAYLOAD_SIZE; i++)
            pqPayload->array[i] = pPayload->array[i];
    }
}

// Override callback method
Return<void> ais_hidl_client::qcarcam_event_callback(uint32_t EventType,
        const QCarCamEventPayload& Payload)
{
    std::unique_lock<std::mutex> lock(mLock);
    AIS_HIDL_DBGMSG("Received qcarcam_event_callback");
    uint32_t event_id;
    QCarCamEventPayload_t qPayload;
    event_id = EventType;
    ais_hidl_client::copyPlayloadData(&qPayload, &Payload);
    lock.unlock();
    if (app_cb)
        app_cb(mQcarcamHndl, event_id, &qPayload, mpPrivateData);
    return Void();
}

// Public methods
QCarCamRet_e ais_hidl_client::ais_client_open_stream(const QCarCamOpen_t* pOpenParams)
{
    QCarCamError Error;
    QCarCamOpenParam openParams = {};

    //Copy qcarcam params
    openParams.opMode = static_cast<QCarCamOpmode>(pOpenParams->opMode);
    openParams.priority = pOpenParams->priority;
    openParams.numInputs = pOpenParams->numInputs;
    for (int i=0; i<openParams.numInputs; i++) {
        openParams.inputs[i].inputId = pOpenParams->inputs[i].inputId;
        openParams.inputs[i].srcId = pOpenParams->inputs[i].srcId;
        openParams.inputs[i].inputMode = pOpenParams->inputs[i].inputMode;
    }

    if (mpService) {
        auto ret = mpService->openStream(openParams, [&](const auto& Stream, const auto& tmpError) {
                Error = tmpError;
                if (QCarCamError::QCARCAM_RET_OK == tmpError) {
                    AIS_HIDL_INFOMSG("openStream success for stream id %u", mStreamId);
                    mpStream = Stream;
                } else {
                    AIS_HIDL_ERRORMSG("openStream Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(tmpError));
                }
                } );
        if (!ret.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport error, openStream() failed for stream id %u", mStreamId);
            Error = QCarCamError::QCARCAM_RET_FAILED;
            mpService = nullptr;
        }
    } else {
        AIS_HIDL_ERRORMSG("Service obj already destroyed for stream id %u", mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }

    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e ais_hidl_client::ais_client_close_stream()
{
    QCarCamError Error;

    if (true == mStreaming)
        ais_client_stop_stream();

    if (mpStream && mpService) {
        auto ret = mpService->closeStream(mpStream);
        if (!ret.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport layer error, closeStream() failed for stream id %u", mStreamId);
            Error = QCarCamError::QCARCAM_RET_FAILED;
        } else {
            Error = ret;
            if (QCarCamError::QCARCAM_RET_OK == Error) {
                AIS_HIDL_INFOMSG("closeStream success for stream id %u", mStreamId);
                mpStream = nullptr;
            }
            else
                AIS_HIDL_ERRORMSG("closeStream Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
        }
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %u", mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }

    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e ais_hidl_client::ais_client_set_cb(QCarCamEventCallback_t cb_ptr, void *pPrivateData)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    // Store apps callback and return
    app_cb = (QCarCamEventCallback_t)cb_ptr;
    mpPrivateData = pPrivateData;
    return ret;
}

QCarCamRet_e ais_hidl_client::ais_client_unset_cb()
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    app_cb = NULL;
    mpPrivateData = NULL;
    return ret;
}

QCarCamRet_e ais_hidl_client::ais_client_s_param(QCarCamParamType_e param, const void* p_value, uint32_t size)
{
    QCarCamError Error;
    bool setParam = true;
    (void)size;

    // Convert input into hidl structure
    if (mpStream) {
        Error = config_hidl_params(param, p_value, setParam);
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %u", mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }

    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e ais_hidl_client::ais_client_g_param(QCarCamParamType_e param, const void* p_value, uint32_t size)
{
    QCarCamError Error;
    bool setParam = false;
    (void)size;

    // Convert input into hidl structure
    if (mpStream) {
        Error = config_hidl_params(param, p_value, setParam);
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %u", mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }

    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e ais_hidl_client::ais_client_s_buffer(const QCarCamBufferList_t* p_buffers)
{
    QcarcamBuffersInfo BuffInfo;
    QCarCamError Error;
    // Fill BuffInfo with buffer data
    BuffInfo.nBuffers = p_buffers->nBuffers;
    BuffInfo.colorFmt = static_cast<QCarCamColorFmt>(p_buffers->colorFmt);
    BuffInfo.flags = p_buffers->flags;
    for (int i=0; i<p_buffers->nBuffers; i++)
    {
        BuffInfo.numPlanes = p_buffers->pBuffers[i].numPlanes;
        for(int j=0; j< BuffInfo.numPlanes && j<QCARCAM_MAX_NUM_PLANES; j++) {
            BuffInfo.planes[j].width = p_buffers->pBuffers[i].planes[j].width;
            BuffInfo.planes[j].height = p_buffers->pBuffers[i].planes[j].height;
            BuffInfo.planes[j].stride = p_buffers->pBuffers[i].planes[j].stride;
            BuffInfo.planes[j].size = p_buffers->pBuffers[i].planes[j].size;
            BuffInfo.planes[j].offset = p_buffers->pBuffers[i].planes[j].offset;
        }
    }
    // Fill native handle with FD details
    // TODO::Need to handle multiplane for bayer sensors
    NATIVE_HANDLE_DECLARE_STORAGE(dvrHandle, p_buffers->nBuffers, 0);
    mNativeHndl = native_handle_init(dvrHandle, p_buffers->nBuffers, 0);
    if (mNativeHndl == NULL) {
        AIS_HIDL_ERRORMSG("Native handle is null!! sending error status");
        Error = QCarCamError::QCARCAM_RET_FAILED;
        return static_cast<QCarCamRet_e>(Error);
    }
    mNativeHndl->numFds = p_buffers->nBuffers;
    for (unsigned int i=0; i<p_buffers->nBuffers; i++) {
        mNativeHndl->data[i] = (uint64_t)p_buffers->pBuffers[i].planes[0].memHndl;
    }

    if (mpStream) {
        auto ret = mpStream->setStreamBuffers(mNativeHndl, BuffInfo);
        if (!ret.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport layer error, setStreamBuffers() failed for stream id %u", mStreamId);
            Error = QCarCamError::QCARCAM_RET_FAILED;
        } else {
            Error = ret;
            if (QCarCamError::QCARCAM_RET_OK == Error)
                AIS_HIDL_INFOMSG("setStreamBuffers success for stream id %u", mStreamId);
            else
                AIS_HIDL_ERRORMSG("setStreamBuffers Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
        }
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %u", mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }

    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e ais_hidl_client::ais_client_start_stream(QCarCamHndl_t q_hndl)
{
    QCarCamError Error;

    //Save qcarcam hndl for callback
    mQcarcamHndl = q_hndl;

    // Call start stream with current class obj for callback
    if (mpStream) {
        auto ret = mpStream->startStream(this);
        if (!ret.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport layer error, startStream() failed for stream id %u", mStreamId);
            Error = QCarCamError::QCARCAM_RET_FAILED;
            mStreaming = false;
        } else {
            Error = ret;
            if (QCarCamError::QCARCAM_RET_OK == Error) {
                AIS_HIDL_INFOMSG("startStream success for stream id %u", mStreamId);
                mStreaming = true;
            } else {
                AIS_HIDL_ERRORMSG("startStream Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
                mStreaming = false;
            }
        }
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %u", mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }

    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e ais_hidl_client::ais_client_stop_stream()
{
    QCarCamError Error;
    std::unique_lock<std::mutex> lock(mLock);

    if (mpStream) {
        auto ret = mpStream->stopStream();
        if (!ret.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport layer error, stopStream() failed for stream id %u", mStreamId);
            Error = QCarCamError::QCARCAM_RET_FAILED;
        } else {
            Error = ret;
            if (QCarCamError::QCARCAM_RET_OK == Error) {
                AIS_HIDL_INFOMSG("stopStream success for stream id %u", mStreamId);
                mStreaming = false;
            } else
                AIS_HIDL_ERRORMSG("stopStream Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
        }
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %u", mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }
    lock.unlock();

    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e ais_hidl_client::ais_client_get_frame(QCarCamFrameInfo_t* pqFrameInfo,
        unsigned long long int timeout, unsigned int flags)
{
    QCarCamError Error;

    if (mpStream) {
        auto ret = mpStream->getFrame(timeout, flags, [&](const auto& tmpError, const auto& Info) {
                Error = tmpError;
                if (QCarCamError::QCARCAM_RET_OK == tmpError) {
                AIS_HIDL_INFOMSG("getFrame success for stream id %u id %u", mStreamId, Info.id);
                // Copy the result into qcarcam structure from hidl cb
                pqFrameInfo->id = Info.id;
                pqFrameInfo->bufferIndex = Info.bufferIndex;
                pqFrameInfo->seqNo = Info.seqNo;
                pqFrameInfo->timestamp = Info.timestamp;
                pqFrameInfo->sofTimestamp.timestamp = Info.sofTimestamp.timestamp;
                pqFrameInfo->sofTimestamp.timestampGPTP = Info.sofTimestamp.timestampGPTP;
                pqFrameInfo->flags = Info.flags;
                pqFrameInfo->fieldType = static_cast<QCarCamInterlaceField_e>(Info.fieldType);
                pqFrameInfo->requestId = Info.requestId;
                pqFrameInfo->inputMetaIdx = Info.inputMetaIdx;
                for (int i=0; i<QCARCAM_MAX_BATCH_FRAMES; i++) {
                    pqFrameInfo->batchFramesInfo[i].seqNo = Info.batchFramesInfo[i].seqNo;
                    pqFrameInfo->batchFramesInfo[i].timestamp.timestamp = Info.batchFramesInfo[i].timestamp.timestamp;
                    pqFrameInfo->batchFramesInfo[i].timestamp.timestampGPTP = Info.batchFramesInfo[i].timestamp.timestampGPTP;
                }
                } else {
                    AIS_HIDL_ERRORMSG("getFrame Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(tmpError));
                }
        } );
        if (!ret.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport layer error, getFrame() failed for stream id %u", mStreamId);
            Error = QCarCamError::QCARCAM_RET_FAILED;
        }
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %u", mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }

    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e ais_hidl_client::ais_client_release_frame(uint32_t id, uint32_t bufferIdx)
{
    QCarCamError Error;

    if (mpStream) {
        auto ret = mpStream->releaseFrame(id, bufferIdx);
        if (!ret.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport layer error, releaseFrame() failed for stream id %u", mStreamId);
            Error = QCarCamError::QCARCAM_RET_FAILED;
        }
        else {
            Error = ret;
            if (QCarCamError::QCARCAM_RET_OK == Error)
                AIS_HIDL_INFOMSG("releaseFrame success for stream id %u idx %u", mStreamId, bufferIdx);
            else
                AIS_HIDL_ERRORMSG("releaseFrame Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
        }
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %u",mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }

    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e ais_hidl_client::ais_client_pause_stream()
{
    QCarCamError Error;

    if (mpStream) {
        auto ret = mpStream->pauseStream();
        if (!ret.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport layer error, pauseStream() failed for stream id %u", mStreamId);
            Error = QCarCamError::QCARCAM_RET_FAILED;
        }
        else {
            Error = ret;
            if (QCarCamError::QCARCAM_RET_OK == Error) {
                AIS_HIDL_INFOMSG("pauseStream success for stream id %u", mStreamId);
                mStreaming = false;
            } else
                AIS_HIDL_ERRORMSG("pauseStream Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
        }
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %u",mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }
    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e ais_hidl_client::ais_client_resume_stream()
{
    QCarCamError Error;

    if (mpStream) {
        auto ret = mpStream->resumeStream();
        if (!ret.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport layer error, resumeStream() failed for stream id %u", mStreamId);
            Error = QCarCamError::QCARCAM_RET_FAILED;
        }
        else {
            Error = ret;
            if (QCarCamError::QCARCAM_RET_OK == Error) {
                AIS_HIDL_INFOMSG("resumeStream success for stream id %u", mStreamId);
                mStreaming = true;
            } else
                AIS_HIDL_ERRORMSG("resumeStream Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
        }
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %u",mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }
    return static_cast<QCarCamRet_e>(Error);
}

// Utility functions
QCarCamError ais_hidl_client::config_hidl_params(QCarCamParamType_e qParam,
        const void *pqConfigData, bool setParam)
{
    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamParamType hidlParamType;
    QCarCamStreamConfigData streamConfigData;

    //qcarcam param pointer types
    uint32_t *puint32Val = 0;
    uint64_t *puint64Val = 0;
    float *pfloatVal = 0;
    QCarCamExposureConfig_t *pexposureConfig;
    QCarCamFrameDropConfig_t *pframeDropConfig;
    QCarCamGammaConfig_t *pgammaConfig;
    QCarCamVendorParam_t *pvendorParam;
    QCarCamLatencyControl_t *platencyParam;
    QCarCamBatchConfig_t *pbatchConfig;
    QCarCamIspUsecaseConfig_t *pispConfig;
    QCarCamInputSignal_e *pinputSignal;
    QCarCamColorSpace_e *pcolorSpace;

    switch (qParam)
    {
        case QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK:
            {
                hidlParamType = QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK;
                puint32Val = (uint32_t*)pqConfigData;
                streamConfigData.uint64Value = static_cast<uint64_t>(*puint32Val);
            }
            break;
        case QCARCAM_STREAM_CONFIG_PARAM_SET_CROP:
            {
                hidlParamType = QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_SET_CROP;
                puint32Val = (uint32_t*)pqConfigData;
                streamConfigData.uint64Value = static_cast<uint64_t>(*puint32Val);
            }
            break;
        case QCARCAM_STREAM_CONFIG_PARAM_LATENCY_CONTROL:
            {
                hidlParamType = QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_LATENCY_CONTROL;
                platencyParam = (QCarCamLatencyControl_t*)pqConfigData;
                streamConfigData.latencyParam.latencyMax = platencyParam->latencyMax;
                streamConfigData.latencyParam.latencyReduceRate = platencyParam->latencyReduceRate;
            }
            break;
        case QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL:
            {
                hidlParamType = QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL;
                pframeDropConfig = (QCarCamFrameDropConfig_t*)pqConfigData;
                streamConfigData.frameDropConfig.frameDropPeriod = pframeDropConfig->frameDropPeriod;
                streamConfigData.frameDropConfig.frameDropPattern = pframeDropConfig->frameDropPattern;
            }
            break;
        case QCARCAM_STREAM_CONFIG_PARAM_INPUT_MODE:
            {
                hidlParamType = QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_INPUT_MODE;
                puint32Val = (uint32_t*)pqConfigData;
                streamConfigData.uint64Value = static_cast<uint64_t>(*puint32Val);
            }
            break;
        case QCARCAM_STREAM_CONFIG_PARAM_MASTER:
            {
                hidlParamType = QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_MASTER;
                puint32Val = (uint32_t*)pqConfigData;
                streamConfigData.uint64Value = static_cast<uint64_t>(*puint32Val);
            }
            break;
        case QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_SUBSCRIBE:
            {
                hidlParamType = QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_SUBSCRIBE;
                puint64Val = (uint64_t*)pqConfigData;
                streamConfigData.uint64Value = static_cast<uint64_t>(*puint64Val);
            }
            break;
        case QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_UNSUBSCRIBE:
            {
                hidlParamType = QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_UNSUBSCRIBE;
                puint64Val = (uint64_t*)pqConfigData;
                streamConfigData.uint64Value = static_cast<uint64_t>(*puint64Val);
            }
            break;
        case QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE:
            {
                hidlParamType = QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE;
                pbatchConfig = (QCarCamBatchConfig_t*)pqConfigData;
                streamConfigData.batchConfig.mode = static_cast<QCarCamBatchMode>(pbatchConfig->mode);
                streamConfigData.batchConfig.numBatchFrames = pbatchConfig->numBatchFrames;
                streamConfigData.batchConfig.frameIncrement = pbatchConfig->frameIncrement;
                streamConfigData.batchConfig.detectFirstPhaseTimer = pbatchConfig->detectFirstPhaseTimer;
            }
            break;
        case QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE:
            {
                hidlParamType = QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE;
                pispConfig = (QCarCamIspUsecaseConfig_t*)pqConfigData;
                streamConfigData.ispConfig.id = pispConfig->id;
                streamConfigData.ispConfig.cameraId = pispConfig->cameraId;
                streamConfigData.ispConfig.usecaseId = static_cast<QCarCamIspUsecase>(pispConfig->usecaseId);
                streamConfigData.ispConfig.tuningMode = pispConfig->tuningMode;
            }
            break;
        case QCARCAM_SENSOR_PARAM_BASE:
            {
                hidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_BASE;
            }
            break;
        case QCARCAM_SENSOR_PARAM_MIRROR_H:
            {
                hidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_MIRROR_H;
                puint32Val = (uint32_t*)pqConfigData;
                streamConfigData.uint64Value = static_cast<uint64_t>(*puint32Val);
            }
            break;
        case QCARCAM_SENSOR_PARAM_MIRROR_V:
            {
                hidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_MIRROR_V;
                puint32Val = (uint32_t*)pqConfigData;
                streamConfigData.uint64Value = static_cast<uint64_t>(*puint32Val);
            }
            break;
        case QCARCAM_SENSOR_PARAM_VID_STD:
            {
                hidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_VID_STD;
                puint32Val = (uint32_t*)pqConfigData;
                streamConfigData.uint64Value = static_cast<uint64_t>(*puint32Val);
            }
            break;
        case QCARCAM_SENSOR_PARAM_CURRENT_VID_STD:
            {
                hidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_CURRENT_VID_STD;
                puint32Val = (uint32_t*)pqConfigData;
                streamConfigData.uint64Value = static_cast<uint64_t>(*puint32Val);
            }
            break;
        case QCARCAM_SENSOR_PARAM_SIGNAL_STATUS:
            {
                hidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_SIGNAL_STATUS;
                pinputSignal = (QCarCamInputSignal_e*)pqConfigData;
                streamConfigData.inputSignal = static_cast<QCarCamInputSignal>(*pinputSignal);
            }
            break;
        case QCARCAM_SENSOR_PARAM_EXPOSURE:
            {
                hidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_EXPOSURE;
                pexposureConfig = (QCarCamExposureConfig_t*)pqConfigData;
                streamConfigData.exposureConfig.mode = static_cast<QCarCamExposureMode>(pexposureConfig->mode);
                streamConfigData.exposureConfig.hdrMode = pexposureConfig->hdrMode;
                streamConfigData.exposureConfig.numExposures = pexposureConfig->numExposures;
                for (int i=0; i<4; i++) {
                    streamConfigData.exposureConfig.exposureTime[i] = pexposureConfig->exposureTime[i];
                    streamConfigData.exposureConfig.exposureRatio[i] = pexposureConfig->exposureRatio[i];
                    streamConfigData.exposureConfig.gain[i] = pexposureConfig->gain[i];
                }
            }
            break;
        case QCARCAM_SENSOR_PARAM_GAMMA:
            {
                hidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_GAMMA;
                pgammaConfig = (QCarCamGammaConfig_t*)pqConfigData;
                streamConfigData.gammaConfig.type = static_cast<QCarCamGammaType>(pgammaConfig->type);
                streamConfigData.gammaConfig.fpValue = pgammaConfig->fpValue;
                if(pgammaConfig->tableSize > QCARCAM_MAX_GAMMA_TABLE)
                {
                    pgammaConfig->tableSize = QCARCAM_MAX_GAMMA_TABLE;
                }
                streamConfigData.gammaConfig.tableSize = pgammaConfig->tableSize;
                for (int i=0; i<pgammaConfig->tableSize && i<QCARCAM_MAX_GAMMA_TABLE; i++)
                     streamConfigData.gammaConfig.table[i] = pgammaConfig->table[i];
            }
            break;
        case QCARCAM_SENSOR_PARAM_BRIGHTNESS:
            {
                hidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_BRIGHTNESS;
                pfloatVal = (float*)pqConfigData;
                streamConfigData.floatValue = static_cast<float>(*pfloatVal);
            }
            break;
        case QCARCAM_SENSOR_PARAM_CONTRAST:
            {
                hidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_CONTRAST;
                pfloatVal = (float*)pqConfigData;
                streamConfigData.floatValue = static_cast<float>(*pfloatVal);
            }
            break;
        case QCARCAM_SENSOR_PARAM_HUE:
            {
                hidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_HUE;
                pfloatVal = (float*)pqConfigData;
                streamConfigData.floatValue = static_cast<float>(*pfloatVal);
            }
            break;
        case QCARCAM_SENSOR_PARAM_SATURATION:
            {
                hidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_SATURATION;
                pfloatVal = (float*)pqConfigData;
                streamConfigData.floatValue = static_cast<float>(*pfloatVal);
            }
            break;
        case QCARCAM_SENSOR_PARAM_COLOR_SPACE:
            {
                hidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_COLOR_SPACE;
                pcolorSpace = (QCarCamColorSpace_e*)pqConfigData;
                streamConfigData.colorSpace = static_cast<QCarCamColorSpace>(*pcolorSpace);
            }
            break;
        case QCARCAM_VENDOR_PARAM_BASE:
            {
                hidlParamType = QCarCamParamType::QCARCAM_VENDOR_PARAM_BASE;
            }
            break;
        case QCARCAM_VENDOR_PARAM:
            {
                hidlParamType = QCarCamParamType::QCARCAM_VENDOR_PARAM;
                pvendorParam = (QCarCamVendorParam_t*)pqConfigData;
                for (int i=0; i<QCARCAM_MAX_VENDOR_PAYLOAD_SIZE; i++)
                    streamConfigData.vendorParam.data[i] = pvendorParam->data[i];
            }
            break;
        default:
            AIS_HIDL_ERRORMSG("Invalid config param %d", static_cast<int>(qParam));
            Error = QCarCamError::QCARCAM_RET_FAILED;
            break;
    }

    if (true == setParam) {
        if (QCarCamError::QCARCAM_RET_OK == Error) {
            // Call hidl API
            if (mpStream) {
                auto ret = mpStream->configureStream(hidlParamType, streamConfigData);
                if (!ret.isOk()) {
                    AIS_HIDL_ERRORMSG("Hidl transport layer error, configureStream() failed for stream id %u", mStreamId);
                    Error = QCarCamError::QCARCAM_RET_FAILED;
                } else {
                    Error = ret;
                    if (QCarCamError::QCARCAM_RET_OK == Error)
                        AIS_HIDL_INFOMSG("configureStream success for stream id %u", mStreamId);
                    else
                        AIS_HIDL_ERRORMSG("configureStream Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
                }
            } else {
                AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %u",mStreamId);
                Error = QCarCamError::QCARCAM_RET_FAILED;
            }
        }
    } else {
        if (QCarCamError::QCARCAM_RET_OK == Error) {
            // Call hidl API
            if (mpStream) {
                auto ret = mpStream->getStreamConfig(hidlParamType, streamConfigData, [&](const auto& tmpError, const auto& tmpData) {
                        Error = tmpError;
                        if (QCarCamError::QCARCAM_RET_OK == tmpError) {
                        AIS_HIDL_INFOMSG("getStreamConfig success for stream id %u", mStreamId);
                        // Copy the result from tmpData
                        std::memcpy(&streamConfigData, &tmpData, sizeof(tmpData));
                        } else {
                        AIS_HIDL_ERRORMSG("getStreamConfig Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(tmpError));
                        }
                        } );
                if (!ret.isOk()) {
                    AIS_HIDL_ERRORMSG("Hidl transport error; getStreamConfig() failed for stream id %u",mStreamId);
                    Error = QCarCamError::QCARCAM_RET_FAILED;
                }
            } else {
                AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %u",mStreamId);
                Error = QCarCamError::QCARCAM_RET_FAILED;
            }
        }
        if (QCarCamError::QCARCAM_RET_OK == Error) {
            switch(qParam)
            {
                case QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK:
                case QCARCAM_STREAM_CONFIG_PARAM_SET_CROP:
                case QCARCAM_STREAM_CONFIG_PARAM_INPUT_MODE:
                case QCARCAM_STREAM_CONFIG_PARAM_MASTER:
                case QCARCAM_SENSOR_PARAM_MIRROR_H:
                case QCARCAM_SENSOR_PARAM_MIRROR_V:
                case QCARCAM_SENSOR_PARAM_VID_STD:
                case QCARCAM_SENSOR_PARAM_CURRENT_VID_STD:
                    {
                        puint32Val = (uint32_t*)pqConfigData;
                        *puint32Val = static_cast<uint32_t>(streamConfigData.uint64Value);
                    }
                    break;
                case QCARCAM_STREAM_CONFIG_PARAM_LATENCY_CONTROL:
                    {
                        platencyParam = (QCarCamLatencyControl_t*)pqConfigData;
                        platencyParam->latencyMax = streamConfigData.latencyParam.latencyMax;
                        platencyParam->latencyReduceRate = streamConfigData.latencyParam.latencyReduceRate;
                    }
                    break;
                case QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL:
                    {
                        pframeDropConfig = (QCarCamFrameDropConfig_t*)pqConfigData;
                        pframeDropConfig->frameDropPeriod = streamConfigData.frameDropConfig.frameDropPeriod;
                        pframeDropConfig->frameDropPattern = streamConfigData.frameDropConfig.frameDropPattern;
                    }
                    break;
                case QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_SUBSCRIBE:
                case QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_UNSUBSCRIBE:
                    {
                        puint64Val = (uint64_t*)pqConfigData;
                        *puint64Val = static_cast<uint64_t>(streamConfigData.uint64Value);
                    }
                    break;
                case QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE:
                    {
                        pbatchConfig = (QCarCamBatchConfig_t*)pqConfigData;
                        pbatchConfig->mode = static_cast<QCarCamBatchMode_e>(streamConfigData.batchConfig.mode);
                        pbatchConfig->numBatchFrames = streamConfigData.batchConfig.numBatchFrames;
                        pbatchConfig->frameIncrement = streamConfigData.batchConfig.frameIncrement;
                        pbatchConfig->detectFirstPhaseTimer = streamConfigData.batchConfig.detectFirstPhaseTimer;
                    }
                    break;
                case QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE:
                    {
                        pispConfig = (QCarCamIspUsecaseConfig_t*)pqConfigData;
                        pispConfig->id = streamConfigData.ispConfig.id;
                        pispConfig->cameraId = streamConfigData.ispConfig.cameraId;
                        pispConfig->usecaseId = static_cast<QCarCamIspUsecase_e>(streamConfigData.ispConfig.usecaseId);
                        pispConfig->tuningMode = streamConfigData.ispConfig.tuningMode;
                    }
                    break;
                case QCARCAM_SENSOR_PARAM_SIGNAL_STATUS:
                    {
                        pinputSignal = (QCarCamInputSignal_e*)pqConfigData;
                        *pinputSignal = static_cast<QCarCamInputSignal_e>(streamConfigData.inputSignal);
                    }
                    break;
                case QCARCAM_SENSOR_PARAM_EXPOSURE:
                    {
                        pexposureConfig = (QCarCamExposureConfig_t*)pqConfigData;
                        pexposureConfig->mode = static_cast<QCarCamExposureMode_e>(streamConfigData.exposureConfig.mode);
                        pexposureConfig->hdrMode = streamConfigData.exposureConfig.hdrMode;
                        pexposureConfig->numExposures = streamConfigData.exposureConfig.numExposures;
                        for (int i=0; i<4; i++) {
                            pexposureConfig->exposureTime[i] = streamConfigData.exposureConfig.exposureTime[i];
                            pexposureConfig->exposureRatio[i] = streamConfigData.exposureConfig.exposureRatio[i];
                            pexposureConfig->gain[i] = streamConfigData.exposureConfig.gain[i];
                        }
                    }
                    break;
                case QCARCAM_SENSOR_PARAM_GAMMA:
                    {
                        pgammaConfig = (QCarCamGammaConfig_t*)pqConfigData;
                        pgammaConfig->type = static_cast<QCarCamGammaType_e>(streamConfigData.gammaConfig.type);
                        pgammaConfig->fpValue = streamConfigData.gammaConfig.fpValue;
                        pgammaConfig->tableSize = streamConfigData.gammaConfig.tableSize;
                        for (int i=0; i<pgammaConfig->tableSize; i++)
                            pgammaConfig->table[i] = streamConfigData.gammaConfig.table[i];
                    }
                    break;
                case QCARCAM_SENSOR_PARAM_BRIGHTNESS:
                case QCARCAM_SENSOR_PARAM_CONTRAST:
                case QCARCAM_SENSOR_PARAM_HUE:
                case QCARCAM_SENSOR_PARAM_SATURATION:
                    {
                        pfloatVal = (float*)pqConfigData;
                        *pfloatVal = static_cast<float>(streamConfigData.floatValue);
                    }
                    break;
                case QCARCAM_SENSOR_PARAM_COLOR_SPACE:
                    {
                        pcolorSpace = (QCarCamColorSpace_e*)pqConfigData;
                        *pcolorSpace = static_cast<QCarCamColorSpace_e>(streamConfigData.colorSpace);
                    }
                    break;
                case QCARCAM_VENDOR_PARAM:
                    {
                        pvendorParam = (QCarCamVendorParam_t*)pqConfigData;
                        for (int i=0; i<QCARCAM_MAX_VENDOR_PAYLOAD_SIZE; i++)
                            pvendorParam->data[i] = streamConfigData.vendorParam.data[i];
                    }
                    break;
                default:
                    AIS_HIDL_ERRORMSG("Invalid config param %d", static_cast<int>(qParam));
                    Error = QCarCamError::QCARCAM_RET_FAILED;
                    break;
            }
        }
    }
    return Error;
}
