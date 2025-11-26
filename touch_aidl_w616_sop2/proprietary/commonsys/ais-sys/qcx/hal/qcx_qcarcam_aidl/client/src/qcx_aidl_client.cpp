/* ===========================================================================
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#include <binder/TextOutput.h>
#include <binder/IInterface.h>
#include <binder/IBinder.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>

#include <NativeHandle.h>
#include "qcx_aidl_client.h"

#define CHECK_BIT(num, pos) ((num) & (0x1<<(pos)))

//std::shared_ptr <IQcarCameraStream> mpStream;
using namespace std;

using ::memcpy;
using ::android::sp;

// Default constructors
qcx_aidl_client::qcx_aidl_client(std::shared_ptr<IQcarCamera> service): mpService(service)
{
    mInitialized = false;
    mStreaming = false;
}

qcx_aidl_client::~qcx_aidl_client()
{
    QCX_AIDL_DBGMSG("Destructor called for qcx_aidl_client hndl %p",mQcarcamHndl);
    mInitialized = false;
    mStreaming = false;
}

void qcx_aidl_client::copyPlayloadData(QCarCamEventPayload_t *pqPayload, const QCarCamEventPayload *pPayload)
{
    int i=0;

    if (pqPayload) {
        switch (pPayload->getTag())
        {
            case QCarCamEventPayload::Tag::u32Data:
                pqPayload->u32Data = pPayload->get<QCarCamEventPayload::Tag::u32Data>();
                break;
            case QCarCamEventPayload::Tag::errInfo:
                pqPayload->errInfo.errorCode = pPayload->get<QCarCamEventPayload::errInfo>().errorCode;
                    pqPayload->errInfo.frameId = pPayload->get<QCarCamEventPayload::errInfo>().frameId;
                    pqPayload->errInfo.timestamp = pPayload->get<QCarCamEventPayload::errInfo>().timestamp;

                break;
            case QCarCamEventPayload::Tag::hwTimestamp:
                pqPayload->hwTimestamp.timestamp = pPayload->get<QCarCamEventPayload::hwTimestamp>().timestamp;
                    pqPayload->hwTimestamp.timestampGPTP = pPayload->get<QCarCamEventPayload::hwTimestamp>().timestampGPTP;
                break;
            case QCarCamEventPayload::Tag::vendorData:
                for (i = 0; i < QCARCAM_MAX_VENDOR_PAYLOAD_SIZE; i++)
                        pqPayload->vendorData.data[i] = pPayload->get<QCarCamEventPayload::vendorData>().data[i];

                break;
            case QCarCamEventPayload::Tag::frameInfo:
                pqPayload->frameInfo.id = pPayload->get<QCarCamEventPayload::frameInfo>().id;
                    pqPayload->frameInfo.bufferIndex = pPayload->get<QCarCamEventPayload::frameInfo>().bufferIndex;
                    pqPayload->frameInfo.seqNo = pPayload->get<QCarCamEventPayload::frameInfo>().seqNo;
                    pqPayload->frameInfo.timestamp = pPayload->get<QCarCamEventPayload::frameInfo>().timestamp;
                    pqPayload->frameInfo.sofTimestamp.timestamp = pPayload->get<QCarCamEventPayload::frameInfo>().sofTimestamp.timestamp;
                    pqPayload->frameInfo.sofTimestamp.timestampGPTP = pPayload->get<QCarCamEventPayload::frameInfo>().sofTimestamp.timestampGPTP;
                    pqPayload->frameInfo.flags = pPayload->get<QCarCamEventPayload::frameInfo>().flags;
                    pqPayload->frameInfo.fieldType = static_cast<QCarCamInterlaceField_e>(pPayload->get<QCarCamEventPayload::frameInfo>().fieldType);
                    pqPayload->frameInfo.requestId = pPayload->get<QCarCamEventPayload::frameInfo>().requestId;
                    pqPayload->frameInfo.inputMetaIdx = pPayload->get<QCarCamEventPayload::frameInfo>().inputMetaIdx;
                    for (i = 0; i < QCARCAM_MAX_BATCH_FRAMES; i++) {
                            pqPayload->frameInfo.batchFramesInfo[i].seqNo = pPayload->get<QCarCamEventPayload::frameInfo>().batchFramesInfo[i].seqNo;
                            pqPayload->frameInfo.batchFramesInfo[i].timestamp.timestamp = pPayload->get<QCarCamEventPayload::frameInfo>().batchFramesInfo[i].timestamp.timestamp;
                            pqPayload->frameInfo.batchFramesInfo[i].timestamp.timestampGPTP = pPayload->get<QCarCamEventPayload::frameInfo>().batchFramesInfo[i].timestamp.timestampGPTP;
                    }
                break;
            case QCarCamEventPayload::Tag::recovery:
                pqPayload->recovery.msg = static_cast<QCarCamEventRecoveryMsg_e>(pPayload->get<QCarCamEventPayload::recovery>().msg);
                    pqPayload->recovery.error.errorId = static_cast<QCarCamErrorEvent_e>(pPayload->get<QCarCamEventPayload::recovery>().error.errorId);
                    pqPayload->recovery.error.errorCode = pPayload->get<QCarCamEventPayload::recovery>().error.errorCode;
                    pqPayload->recovery.error.frameId = pPayload->get<QCarCamEventPayload::recovery>().error.frameId;
                    pqPayload->recovery.error.timestamp = pPayload->get<QCarCamEventPayload::recovery>().error.timestamp;

                break;
            case QCarCamEventPayload::Tag::array:
                for (i=0; i<QCARCAM_MAX_PAYLOAD_SIZE; i++)
                            pqPayload->array[i] = pPayload->get<QCarCamEventPayload::array>()[i];
                break;
            default:
                QCX_AIDL_ERRORMSG("Unsupported QCarCamEventPayload Tag");
                break;
        }
    }
}

// Override callback method
::ndk::ScopedAStatus qcx_aidl_client::qcarcam_event_callback(int32_t EventType,
        const QCarCamEventPayload& Payload)
{
    std::unique_lock<std::mutex> lock(mLock);
    QCX_AIDL_INFOMSG("Received qcarcam_event_callback");
    uint32_t event_id;
    QCarCamEventPayload_t qPayload;
    memset(&qPayload, 0, sizeof(QCarCamEventPayload_t));
    event_id = EventType;
    copyPlayloadData(&qPayload, &Payload);
    lock.unlock();
    if (app_cb){
        app_cb(mQcarcamHndl, event_id, &qPayload, mpPrivateData);
    }
    return ndk::ScopedAStatus::ok();
}

// Public methods
QCarCamRet_e qcx_aidl_client::qcx_client_open_stream(const QCarCamOpen_t* pOpenParams)
{
    QCarCamError Error;
    QCarCamOpenParam openParams = {};

    //Copy qcarcam params
    openParams.opMode = static_cast<QCarCamOpmode>(pOpenParams->opMode);
    openParams.priority = pOpenParams->priority;
    openParams.numInputs = pOpenParams->numInputs;
    openParams.flags = pOpenParams->flags;
    openParams.clientId = pOpenParams->clientId;

    for (int i=0; i<openParams.numInputs; i++) {
        openParams.inputs[i].inputId = pOpenParams->inputs[i].inputId;
        openParams.inputs[i].srcId = pOpenParams->inputs[i].srcId;
        openParams.inputs[i].inputMode = pOpenParams->inputs[i].inputMode;
    }

    if (mpService) {
        std::shared_ptr<::aidl::vendor::qti::automotive::qcarcam2::IQcarCameraStream> Stream;
        auto ret = mpService->openStream(openParams, &Stream);
        if (ret.isOk()) {
            QCX_AIDL_INFOMSG("openStream success for stream id %u", mStreamId);
            mpStream = Stream;
            Error = QCarCamError::QCARCAM_RET_OK;
        } else {
            Error = QCarCamError::QCARCAM_RET_FAILED;
            QCX_AIDL_ERRORMSG("openStream Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
            mpService.reset();
        }
    } else {
        QCX_AIDL_ERRORMSG("Service obj already destroyed for stream id %u", mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }

    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e qcx_aidl_client::qcx_client_close_stream()
{
    QCarCamError Error;

    if (true == mStreaming)
        qcx_client_stop_stream();

    if (mpStream.get() && mpService) {
        auto ret = mpService->closeStream(mpStream, &Error);
        if (!ret.isOk()) {
            QCX_AIDL_ERRORMSG("Aidl transport layer error, closeStream() failed for stream id %u", mStreamId);
            Error = QCarCamError::QCARCAM_RET_FAILED;
        } else {
            if (QCarCamError::QCARCAM_RET_OK == Error) {
                QCX_AIDL_INFOMSG("closeStream success for stream id %u", mStreamId);
                mpStream.reset();
            }
            else
                QCX_AIDL_ERRORMSG("closeStream Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
        }
    } else {
        QCX_AIDL_ERRORMSG("Stream obj already destroyed for stream id %u", mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }

    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e qcx_aidl_client::qcx_client_set_cb(QCarCamEventCallback_t cb_ptr, void *pPrivateData)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    // Store apps callback and return
    app_cb = (QCarCamEventCallback_t)cb_ptr;
    mpPrivateData = pPrivateData;
    return ret;
}

QCarCamRet_e qcx_aidl_client::qcx_client_unset_cb()
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    app_cb = NULL;
    mpPrivateData = NULL;
    return ret;
}

QCarCamRet_e qcx_aidl_client::qcx_client_s_param(QCarCamParamType_e param, const void* p_value, uint32_t size)
{
    QCarCamError Error;
    bool setParam = true;
    (void)size;

    // Convert input into aidl structure
    if (mpStream.get()) {
        Error = config_aidl_params(param, p_value, setParam);
    } else {
        QCX_AIDL_ERRORMSG("Stream obj already destroyed for stream id %u", mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }
    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e qcx_aidl_client::qcx_client_g_param(QCarCamParamType_e param, const void* p_value, uint32_t size)
{
    QCarCamError Error;
    bool setParam = false;
    (void)size;

    // Convert input into aidl structure
    if (mpStream.get()) {
        Error = config_aidl_params(param, p_value, setParam);
    } else {
        QCX_AIDL_ERRORMSG("Stream obj already destroyed for stream id %u", mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }

    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e qcx_aidl_client::qcx_client_s_buffer(const QCarCamBufferList_t* p_buffers)
{
    QcarcamBuffersInfo BuffInfo;
    QCarCamError Error;
    // Fill BuffInfo with buffer data
    BuffInfo.nBuffers = p_buffers->nBuffers;
    BuffInfo.colorFmt = static_cast<QCarCamColorFmt>(p_buffers->colorFmt);
    BuffInfo.flags = p_buffers->flags;
    BuffInfo.id = p_buffers->id;
    for (int i=0; i<p_buffers->nBuffers; i++)
    {
        BuffInfo.numPlanes = p_buffers->pBuffers[i].numPlanes;
        for(int j=0; j< BuffInfo.numPlanes && j<QCARCAM_MAX_NUM_PLANES; j++) {
            BuffInfo.planes[j].width = p_buffers->pBuffers[i].planes[j].width;
            BuffInfo.planes[j].height = p_buffers->pBuffers[i].planes[j].height;
            BuffInfo.planes[j].stride = p_buffers->pBuffers[i].planes[j].stride;
            BuffInfo.planes[j].size = p_buffers->pBuffers[i].planes[j].size;
            BuffInfo.planes[j].offset = p_buffers->pBuffers[i].planes[j].offset;
            QCX_AIDL_INFOMSG("setStreamBuffers w %d h %d stride %d size %d offset %d", BuffInfo.planes[j].width, BuffInfo.planes[j].height, BuffInfo.planes[j].stride, BuffInfo.planes[j].size, BuffInfo.planes[j].offset = p_buffers->pBuffers[i].planes[j].offset);
        }
    }


    // Fill native handle with FD details
    // TODO::Need to handle multiplane for bayer sensors
    NATIVE_HANDLE_DECLARE_STORAGE(dvrHandle, p_buffers->nBuffers, 0);
    mNativeHndl = native_handle_init(dvrHandle, p_buffers->nBuffers, 0);
    if (mNativeHndl == NULL) {
        QCX_AIDL_ERRORMSG("Native handle is null!! sending error status");
        Error = QCarCamError::QCARCAM_RET_FAILED;
        return static_cast<QCarCamRet_e>(Error);
    }
    mNativeHndl->numFds = p_buffers->nBuffers;
    for (unsigned int i=0; i<p_buffers->nBuffers; i++) {
        mNativeHndl->data[i] = (uint64_t)p_buffers->pBuffers[i].planes[0].bufHndl.bufFd;
    }

    if (mpStream.get()) {
        ::aidl::android::hardware::common::NativeHandle aNativeHndl = ::android::dupToAidl(mNativeHndl);
        auto ret = mpStream->setStreamBuffers(aNativeHndl, BuffInfo, &Error);
        if (!ret.isOk()) {
            QCX_AIDL_ERRORMSG("Aidl transport layer error, setStreamBuffers() failed for stream id %u", mStreamId);
        } else {
            if (QCarCamError::QCARCAM_RET_OK == Error)
                QCX_AIDL_INFOMSG("setStreamBuffers success for stream id %u", mStreamId);
            else
                QCX_AIDL_ERRORMSG("setStreamBuffers Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
        }
    } else {
        QCX_AIDL_ERRORMSG("Stream obj already destroyed for stream id %u", mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }

    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e qcx_aidl_client::qcx_client_start_stream(QCarCamHndl_t q_hndl)
{
    QCarCamError Error;

    //Save qcarcam hndl for callback
    mQcarcamHndl = q_hndl;
    // Call start stream with current class obj for callback
    if (mpStream.get()) {

        auto ret = mpStream->startStream(ref<qcx_aidl_client>(),  &Error);
        if (!ret.isOk()) {
            QCX_AIDL_ERRORMSG("Aidl transport layer error, startStream() failed for stream id %u", mStreamId);
            mStreaming = false;
        } else {
            if (QCarCamError::QCARCAM_RET_OK == Error) {
                QCX_AIDL_INFOMSG("startStream success for stream id %u", mStreamId);
                mStreaming = true;
            } else {
                QCX_AIDL_ERRORMSG("startStream Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
                mStreaming = false;
            }
        }
    } else {
        QCX_AIDL_ERRORMSG("Stream obj already destroyed for stream id %u", mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }
    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e qcx_aidl_client::qcx_client_stop_stream()
{
    QCarCamError Error;
    std::unique_lock<std::mutex> lock(mLock);

    if (mpStream.get()) {
        auto ret = mpStream->stopStream(&Error);
        if (!ret.isOk()) {
            QCX_AIDL_ERRORMSG("Aidl transport layer error, stopStream() failed for stream id %u", mStreamId);
        } else {
            if (QCarCamError::QCARCAM_RET_OK == Error) {
                QCX_AIDL_INFOMSG("stopStream success for stream id %u", mStreamId);
                mStreaming = false;
            } else
                QCX_AIDL_ERRORMSG("stopStream Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
        }
    } else {
        QCX_AIDL_ERRORMSG("Stream obj already destroyed for stream id %u", mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }
    lock.unlock();

    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e qcx_aidl_client::qcx_client_get_frame(QCarCamFrameInfo_t* pqFrameInfo,
        unsigned long long int timeout, unsigned int flags)
{
    QCarCamError Error;

    if (mpStream.get()) {
        QCarCamFrameInfo Info;
        auto ret = mpStream->getFrame(timeout, flags, &Info, &Error);
        if (ret.isOk()) {
            if (QCarCamError::QCARCAM_RET_OK == Error) {
                QCX_AIDL_INFOMSG("getFrame success for stream id %u id %u", mStreamId, Info.id);
                // Copy the result into qcarcam structure from aidl cb
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
            }
            else
                QCX_AIDL_ERRORMSG("qcx_client_get_frame Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
        } else {
            QCX_AIDL_ERRORMSG("getFrame Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error)/*static_cast<int32_t>(tmpError)*/);
        }
    } else {
        QCX_AIDL_ERRORMSG("Stream obj already destroyed for stream id %u", mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }

    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e qcx_aidl_client::qcx_client_submitRequest(const QCarCamRequest_t* pRequest)
{
    QCarCamError Error;

    if (mpStream.get()) {
        QCarCamRequest Info;
        Info.requestId = pRequest->requestId;
        Info.numStreamRequests = pRequest->numStreamRequests;
        for (uint32_t i = 0; i < Info.numStreamRequests; i++)
        {
            Info.streamRequests[i].bufferIdx = pRequest->streamRequests[i].bufferIdx;
            Info.streamRequests[i].bufferlistId = pRequest->streamRequests[i].bufferlistId;
        }
        auto ret = mpStream->submitRequest(Info, &Error);
        if (ret.isOk()) {
            QCX_AIDL_INFOMSG("submitRequest success for stream id %u id %u", mStreamId, Info.requestId);
        } else {
            QCX_AIDL_ERRORMSG("submitRequest Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error)/*static_cast<int32_t>(tmpError)*/);
        }
    } else {
        QCX_AIDL_ERRORMSG("Stream obj already destroyed for stream id %u", mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }

    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e qcx_aidl_client::qcx_client_release_frame(uint32_t id, uint32_t bufferIdx)
{
    QCarCamError Error;

    if (mpStream.get()) {
        auto ret = mpStream->releaseFrame(id, bufferIdx, &Error);
        if (!ret.isOk()) {
            QCX_AIDL_ERRORMSG("Aidl transport layer error, releaseFrame() failed for stream id %u", mStreamId);
        }
        else {
            if (QCarCamError::QCARCAM_RET_OK == Error)
                QCX_AIDL_INFOMSG("releaseFrame success for stream id %u idx %u", mStreamId, bufferIdx);
            else
                QCX_AIDL_ERRORMSG("releaseFrame Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
        }
    } else {
        QCX_AIDL_ERRORMSG("Stream obj already destroyed for stream id %u",mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }

    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e qcx_aidl_client::qcx_client_pause_stream()
{
    QCarCamError Error;

    if (mpStream.get()) {
        auto ret = mpStream->pauseStream(&Error);
        if (!ret.isOk()) {
            QCX_AIDL_ERRORMSG("Aidl transport layer error, pauseStream() failed for stream id %u", mStreamId);
        }
        else {
            if (QCarCamError::QCARCAM_RET_OK == Error) {
                QCX_AIDL_INFOMSG("pauseStream success for stream id %u", mStreamId);
                mStreaming = false;
            } else
                QCX_AIDL_ERRORMSG("pauseStream Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
        }
    } else {
        QCX_AIDL_ERRORMSG("Stream obj already destroyed for stream id %u",mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }
    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e qcx_aidl_client::qcx_client_resume_stream()
{
    QCarCamError Error;

    if (mpStream.get()) {
        auto ret = mpStream->resumeStream(&Error);
        if (!ret.isOk()) {
            QCX_AIDL_ERRORMSG("Aidl transport layer error, resumeStream() failed for stream id %u", mStreamId);
            Error = QCarCamError::QCARCAM_RET_FAILED;
        }
        else {
            if (QCarCamError::QCARCAM_RET_OK == Error) {
                QCX_AIDL_INFOMSG("resumeStream success for stream id %u", mStreamId);
                mStreaming = true;
            } else
                QCX_AIDL_ERRORMSG("resumeStream Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
        }
    } else {
        QCX_AIDL_ERRORMSG("Stream obj already destroyed for stream id %u",mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }
    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e qcx_aidl_client::qcx_client_reserve_stream()
{
    QCarCamError Error;

    if (mpStream.get()) {
        auto ret = mpStream->reserveStream(&Error);
        if (!ret.isOk()) {
            QCX_AIDL_ERRORMSG("Aidl transport layer error, reserveStream() failed for stream id %u", mStreamId);
            Error = QCarCamError::QCARCAM_RET_FAILED;
        }
        else {
            if (QCarCamError::QCARCAM_RET_OK == Error) {
                QCX_AIDL_INFOMSG("reserveStream success for stream id %u", mStreamId);
                mStreaming = true;
            } else
                QCX_AIDL_ERRORMSG("reserveStream Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
        }
    } else {
        QCX_AIDL_ERRORMSG("Stream obj already destroyed for stream id %u",mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }
    return static_cast<QCarCamRet_e>(Error);
}

QCarCamRet_e qcx_aidl_client::qcx_client_release_stream()
{
    QCarCamError Error;

    if (mpStream.get()) {
        auto ret = mpStream->releaseStream(&Error);
        if (!ret.isOk()) {
            QCX_AIDL_ERRORMSG("Aidl transport layer error, releaseStream() failed for stream id %u", mStreamId);
            Error = QCarCamError::QCARCAM_RET_FAILED;
        }
        else {
            if (QCarCamError::QCARCAM_RET_OK == Error) {
                QCX_AIDL_INFOMSG("releaseStream success for stream id %u", mStreamId);
                mStreaming = true;
            } else
                QCX_AIDL_ERRORMSG("releaseStream Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
        }
    } else {
        QCX_AIDL_ERRORMSG("Stream obj already destroyed for stream id %u",mStreamId);
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }
    return static_cast<QCarCamRet_e>(Error);
}

// Utility functions
QCarCamError qcx_aidl_client::config_aidl_params(QCarCamParamType_e qParam,
        const void *pqConfigData, bool setParam)
{
    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamParamType aidlParamType;
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
    QCarCamRegion_t *pcropRegion;

    switch (qParam)
    {
        case QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK:
            {
                aidlParamType = QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK;
                puint32Val = (uint32_t*)pqConfigData;
                streamConfigData.set<QCarCamStreamConfigData::uint64Value>(static_cast<uint64_t>(*puint32Val));
                //streamConfigData.uint64Value = static_cast<uint64_t>(*puint32Val);
            }
            break;
        case QCARCAM_STREAM_CONFIG_PARAM_SET_CROP:
            {
                aidlParamType = QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_SET_CROP;
                pcropRegion = (QCarCamRegion_t*)pqConfigData;
                class QCarCamCropRegion dummy_cropRegion;
                dummy_cropRegion.x = pcropRegion->x;
                dummy_cropRegion.y = pcropRegion->y;
                dummy_cropRegion.width = pcropRegion->width;
                dummy_cropRegion.height = pcropRegion->height;
                streamConfigData.set<QCarCamStreamConfigData::cropRegion>(dummy_cropRegion);
            }
            break;
        case QCARCAM_STREAM_CONFIG_PARAM_LATENCY_CONTROL:
            {
                aidlParamType = QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_LATENCY_CONTROL;
                platencyParam = (QCarCamLatencyControl_t*)pqConfigData;
                class QCarCamLatencyControl dummy_latencycontrol;
                dummy_latencycontrol.latencyMax = platencyParam->latencyMax;
                dummy_latencycontrol.latencyReduceRate = platencyParam->latencyReduceRate;
                streamConfigData.set<QCarCamStreamConfigData::latencyParam>(dummy_latencycontrol);
            }
            break;
        case QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL:
            {
                aidlParamType = QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL;
                pframeDropConfig = (QCarCamFrameDropConfig_t*)pqConfigData;
                class QCarCamFrameDropConfig dummy_frame_dropconfig;
                dummy_frame_dropconfig.frameDropPeriod = pframeDropConfig->frameDropPeriod;
                dummy_frame_dropconfig.frameDropPattern = pframeDropConfig->frameDropPattern;
                streamConfigData.set<QCarCamStreamConfigData::frameDropConfig>(dummy_frame_dropconfig);
            }
            break;
        case QCARCAM_STREAM_CONFIG_PARAM_INPUT_MODE:
            {
                aidlParamType = QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_INPUT_MODE;
                puint32Val = (uint32_t*)pqConfigData;
                streamConfigData.set<QCarCamStreamConfigData::uint64Value>(static_cast<uint64_t>(*puint32Val));
            }
            break;
        case QCARCAM_STREAM_CONFIG_PARAM_MASTER:
            {
                aidlParamType = QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_MASTER;
                puint32Val = (uint32_t*)pqConfigData;
                streamConfigData.set<QCarCamStreamConfigData::uint64Value>(static_cast<uint64_t>(*puint32Val));
            }
            break;
        case QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_SUBSCRIBE:
            {
                aidlParamType = QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_SUBSCRIBE;
                puint64Val = (uint64_t*)pqConfigData;
                streamConfigData.set<QCarCamStreamConfigData::uint64Value>(static_cast<uint64_t>(*puint32Val));
            }
            break;
        case QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_UNSUBSCRIBE:
            {
                aidlParamType = QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_UNSUBSCRIBE;
                puint64Val = (uint64_t*)pqConfigData;
                streamConfigData.set<QCarCamStreamConfigData::uint64Value>(static_cast<uint64_t>(*puint32Val));
            }
            break;
        case QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE:
            {
                aidlParamType = QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE;
                pbatchConfig = (QCarCamBatchConfig_t*)pqConfigData;
                class QCarCamBatchConfig dummy_varible;
                dummy_varible.mode                  = static_cast<QCarCamBatchMode>(pbatchConfig->mode);
                dummy_varible.numBatchFrames        = pbatchConfig->numBatchFrames;
                dummy_varible.frameIncrement        = pbatchConfig->frameIncrement;
                dummy_varible.detectFirstPhaseTimer = pbatchConfig->detectFirstPhaseTimer;
            }
            break;
        case QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE:
            {
                aidlParamType = QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE;
                pispConfig = (QCarCamIspUsecaseConfig_t*)pqConfigData;
                class QCarCamIspUsecaseConfig dummy_varible;
                dummy_varible.id = pispConfig->id;
                dummy_varible.cameraId = pispConfig->cameraId;
                dummy_varible.usecaseId = static_cast<QCarCamIspUsecase>(pispConfig->usecaseId);
                dummy_varible.tuningMode = pispConfig->tuningMode;
                streamConfigData.set<QCarCamStreamConfigData::ispConfig>(dummy_varible);
            }
            break;
        case QCARCAM_SENSOR_PARAM_BASE:
            {
                aidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_BASE;
            }
            break;
        case QCARCAM_SENSOR_PARAM_MIRROR_H:
            {
                aidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_MIRROR_H;
                puint32Val = (uint32_t*)pqConfigData;
                streamConfigData.set<QCarCamStreamConfigData::uint64Value>(static_cast<uint64_t>(*puint32Val));
            }
            break;
        case QCARCAM_SENSOR_PARAM_MIRROR_V:
            {
                aidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_MIRROR_V;
                puint32Val = (uint32_t*)pqConfigData;
                streamConfigData.set<QCarCamStreamConfigData::uint64Value>(static_cast<uint64_t>(*puint32Val));
            }
            break;
        case QCARCAM_SENSOR_PARAM_VID_STD:
            {
                aidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_VID_STD;
                puint32Val = (uint32_t*)pqConfigData;
                streamConfigData.set<QCarCamStreamConfigData::uint64Value>(static_cast<uint64_t>(*puint32Val));
            }
            break;
        case QCARCAM_SENSOR_PARAM_CURRENT_VID_STD:
            {
                aidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_CURRENT_VID_STD;
                puint32Val = (uint32_t*)pqConfigData;
                streamConfigData.set<QCarCamStreamConfigData::uint64Value>(static_cast<uint64_t>(*puint32Val));
            }
            break;
        case QCARCAM_SENSOR_PARAM_SIGNAL_STATUS:
            {
                aidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_SIGNAL_STATUS;
                pinputSignal = (QCarCamInputSignal_e*)pqConfigData;
                streamConfigData.set<QCarCamStreamConfigData::uint64Value>(static_cast<uint64_t>(*puint32Val));
            }
            break;
        case QCARCAM_SENSOR_PARAM_EXPOSURE:
            {
                aidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_EXPOSURE;
                pexposureConfig = (QCarCamExposureConfig_t*)pqConfigData;
                class QCarCamExposureConfig dummy_exposureconfig;
                dummy_exposureconfig.mode = static_cast<QCarCamExposureMode>(pexposureConfig->mode);
                dummy_exposureconfig.hdrMode = pexposureConfig->hdrMode;
                dummy_exposureconfig.numExposures = pexposureConfig->numExposures;
                for (int i=0; i<4; i++) {
                    dummy_exposureconfig.exposureTime[i] = pexposureConfig->exposureTime[i];
                    dummy_exposureconfig.exposureRatio[i] = pexposureConfig->exposureRatio[i];
                    dummy_exposureconfig.gain[i] = pexposureConfig->gain[i];
                }
                streamConfigData.set<QCarCamStreamConfigData::exposureConfig>(dummy_exposureconfig);
            }
            break;
        case QCARCAM_SENSOR_PARAM_GAMMA:
            {
                aidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_GAMMA;
                pgammaConfig = (QCarCamGammaConfig_t*)pqConfigData;

                class QCarCamGammaConfig dummy_gammaconfig;
                dummy_gammaconfig.type = static_cast<QCarCamGammaType>(pgammaConfig->type);
                dummy_gammaconfig.fpValue = pgammaConfig->fpValue;
                dummy_gammaconfig.tableSize = pgammaConfig->tableSize;
                for (int i=0; i<pgammaConfig->tableSize; i++)
                    dummy_gammaconfig.table[i] = pgammaConfig->table[i];

                streamConfigData.set<QCarCamStreamConfigData::gammaConfig>(dummy_gammaconfig);
            }
            break;
        case QCARCAM_SENSOR_PARAM_BRIGHTNESS:
            {
                aidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_BRIGHTNESS;
                pfloatVal = (float*)pqConfigData;
                streamConfigData.set<QCarCamStreamConfigData::floatValue>(static_cast<float>(*pfloatVal));
                //streamConfigData.floatValue = static_cast<float>(*pfloatVal);
            }
            break;
        case QCARCAM_SENSOR_PARAM_CONTRAST:
            {
                aidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_CONTRAST;
                pfloatVal = (float*)pqConfigData;
                streamConfigData.set<QCarCamStreamConfigData::floatValue>(static_cast<float>(*pfloatVal));
                //streamConfigData.floatValue = static_cast<float>(*pfloatVal);
            }
            break;
        case QCARCAM_SENSOR_PARAM_HUE:
            {
                aidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_HUE;
                pfloatVal = (float*)pqConfigData;
                streamConfigData.set<QCarCamStreamConfigData::floatValue>(static_cast<float>(*pfloatVal));
            }
            break;
        case QCARCAM_SENSOR_PARAM_SATURATION:
            {
                aidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_SATURATION;
                pfloatVal = (float*)pqConfigData;
                streamConfigData.set<QCarCamStreamConfigData::floatValue>(static_cast<float>(*pfloatVal));
            }
            break;
        case QCARCAM_SENSOR_PARAM_COLOR_SPACE:
            {
                aidlParamType = QCarCamParamType::QCARCAM_SENSOR_PARAM_COLOR_SPACE;
                pcolorSpace = (QCarCamColorSpace_e*)pqConfigData;
                streamConfigData.set<QCarCamStreamConfigData::colorSpace>(static_cast<QCarCamColorSpace>(*pcolorSpace));
                //streamConfigData.colorSpace = static_cast<QCarCamColorSpace>(*pcolorSpace);
            }
            break;
        case QCARCAM_VENDOR_PARAM_BASE:
            {
                aidlParamType = QCarCamParamType::QCARCAM_VENDOR_PARAM_BASE;
            }
            break;
        case QCARCAM_VENDOR_PARAM:
            {
                aidlParamType = QCarCamParamType::QCARCAM_VENDOR_PARAM;
                pvendorParam = (QCarCamVendorParam_t*)pqConfigData;
                QCarCamVendorParam dummy_vendorParam;
                for (int i=0; i<QCARCAM_MAX_VENDOR_PAYLOAD_SIZE; i++)
                    dummy_vendorParam.data[i] = pvendorParam->data[i];
                    streamConfigData.set<QCarCamStreamConfigData::vendorParam>(dummy_vendorParam);
            }
            break;
        default:
            QCX_AIDL_ERRORMSG("Invalid config param %d", static_cast<int>(qParam));
            Error = QCarCamError::QCARCAM_RET_FAILED;
            break;
    }

    if (true == setParam) {
        if (QCarCamError::QCARCAM_RET_OK == Error) {
            // Call aidl API
            if (mpStream.get()) {
                auto ret = mpStream->configureStream(aidlParamType, streamConfigData, &Error);
                if (!ret.isOk()) {
                    QCX_AIDL_ERRORMSG("Aidl transport layer error, configureStream() failed for stream id %u", mStreamId);
                } else {
                    if (QCarCamError::QCARCAM_RET_OK == Error)
                        QCX_AIDL_INFOMSG("configureStream success for stream id %u", mStreamId);
                    else
                        QCX_AIDL_ERRORMSG("configureStream Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
                }
            } else {
                QCX_AIDL_ERRORMSG("Stream obj already destroyed for stream id %u",mStreamId);
                Error = QCarCamError::QCARCAM_RET_FAILED;
            }
        }
    } else {
        if (QCarCamError::QCARCAM_RET_OK == Error) {
            // Call aidl API
            QCX_AIDL_ERRORMSG("mpStream.get() %x",mpStream.get());
            if (mpStream.get()) {
                QCarCamStreamConfigData tmpData;
                auto ret = mpStream->getStreamConfig(aidlParamType, streamConfigData, &tmpData, &Error);
                if (QCarCamError::QCARCAM_RET_OK == Error) {
                    QCX_AIDL_INFOMSG("getStreamConfig success for stream id %u", mStreamId);
                    // Copy the result from tmpData
                    std::memcpy(&streamConfigData, &tmpData, sizeof(tmpData));
                } else {
                    QCX_AIDL_ERRORMSG("getStreamConfig Failed for stream id %u with Error %d", mStreamId, static_cast<int32_t>(Error));
                }
                if (!ret.isOk()) {
                    QCX_AIDL_ERRORMSG("Aidl transport error; getStreamConfig() failed for stream id %u",mStreamId);
                }
            } else {
                QCX_AIDL_ERRORMSG("Stream obj already destroyed for stream id %u",mStreamId);
                Error = QCarCamError::QCARCAM_RET_FAILED;
            }
        }
        if (QCarCamError::QCARCAM_RET_OK == Error) {
            switch(qParam)
            {
                case QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK:
                case QCARCAM_STREAM_CONFIG_PARAM_INPUT_MODE:
                case QCARCAM_STREAM_CONFIG_PARAM_MASTER:
                case QCARCAM_SENSOR_PARAM_MIRROR_H:
                case QCARCAM_SENSOR_PARAM_MIRROR_V:
                case QCARCAM_SENSOR_PARAM_VID_STD:
                case QCARCAM_SENSOR_PARAM_CURRENT_VID_STD:
                    {
                        puint32Val = (uint32_t*)pqConfigData;
                        *puint32Val = static_cast<uint32_t>(streamConfigData.get<QCarCamStreamConfigData::uint64Value>());
                    }
                    break;
                case QCARCAM_STREAM_CONFIG_PARAM_SET_CROP:
                    {
                        pcropRegion = (QCarCamRegion_t *)pqConfigData;
                        pcropRegion->x = streamConfigData.get<QCarCamStreamConfigData::cropRegion>().x;
                        pcropRegion->y = streamConfigData.get<QCarCamStreamConfigData::cropRegion>().y;
                        pcropRegion->width = streamConfigData.get<QCarCamStreamConfigData::cropRegion>().width;
                        pcropRegion->height = streamConfigData.get<QCarCamStreamConfigData::cropRegion>().height;

                    }
                case QCARCAM_STREAM_CONFIG_PARAM_LATENCY_CONTROL:
                    {
                        platencyParam = (QCarCamLatencyControl_t*)pqConfigData;
                        platencyParam->latencyMax = streamConfigData.get<QCarCamStreamConfigData::latencyParam>().latencyMax;
                        platencyParam->latencyReduceRate = streamConfigData.get<QCarCamStreamConfigData::latencyParam>().latencyReduceRate;
                    }
                    break;
                case QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL:
                    {
                        pframeDropConfig = (QCarCamFrameDropConfig_t*)pqConfigData;
                        pframeDropConfig->frameDropPeriod = streamConfigData.get<QCarCamStreamConfigData::frameDropConfig>().frameDropPeriod;
                        pframeDropConfig->frameDropPattern = streamConfigData.get<QCarCamStreamConfigData::frameDropConfig>().frameDropPattern;
                    }
                    break;
                case QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_SUBSCRIBE:
                case QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_UNSUBSCRIBE:
                    {
                        puint64Val = (uint64_t*)pqConfigData;
                        *puint64Val = static_cast<uint32_t>(streamConfigData.get<QCarCamStreamConfigData::uint64Value>());
                    }
                    break;
                case QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE:
                    {
                        pbatchConfig = (QCarCamBatchConfig_t*)pqConfigData;
                        pbatchConfig->mode = static_cast<QCarCamBatchMode_e>(streamConfigData.get<QCarCamStreamConfigData::batchConfig>().mode);
                        pbatchConfig->numBatchFrames = streamConfigData.get<QCarCamStreamConfigData::batchConfig>().numBatchFrames;
                        pbatchConfig->frameIncrement = streamConfigData.get<QCarCamStreamConfigData::batchConfig>().frameIncrement;
                        pbatchConfig->detectFirstPhaseTimer = streamConfigData.get<QCarCamStreamConfigData::batchConfig>().detectFirstPhaseTimer;
                    }
                    break;
                case QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE:
                    {
                        pispConfig = (QCarCamIspUsecaseConfig_t*)pqConfigData;
                        pispConfig->id = streamConfigData.get<QCarCamStreamConfigData::ispConfig>().id;
                        pispConfig->cameraId = streamConfigData.get<QCarCamStreamConfigData::ispConfig>().cameraId;
                        pispConfig->usecaseId = static_cast<QCarCamIspUsecase_e>(streamConfigData.get<QCarCamStreamConfigData::ispConfig>().usecaseId);
                        pispConfig->tuningMode = streamConfigData.get<QCarCamStreamConfigData::ispConfig>().tuningMode;
                    }
                    break;
                case QCARCAM_SENSOR_PARAM_SIGNAL_STATUS:
                    {
                        pinputSignal = (QCarCamInputSignal_e*)pqConfigData;
                        *pinputSignal = static_cast<QCarCamInputSignal_e>(streamConfigData.get<QCarCamStreamConfigData::inputSignal>());
                    }
                    break;
                case QCARCAM_SENSOR_PARAM_EXPOSURE:
                    {
                        pexposureConfig = (QCarCamExposureConfig_t*)pqConfigData;
                        pexposureConfig->mode = static_cast<QCarCamExposureMode_e>(streamConfigData.get<QCarCamStreamConfigData::exposureConfig>().mode);
                        pexposureConfig->hdrMode = streamConfigData.get<QCarCamStreamConfigData::exposureConfig>().hdrMode;
                        pexposureConfig->numExposures = streamConfigData.get<QCarCamStreamConfigData::exposureConfig>().numExposures;
                        for (int i=0; i<4; i++) {
                            pexposureConfig->exposureTime[i] = streamConfigData.get<QCarCamStreamConfigData::exposureConfig>().exposureTime[i];
                            pexposureConfig->exposureRatio[i] = streamConfigData.get<QCarCamStreamConfigData::exposureConfig>().exposureRatio[i];
                            pexposureConfig->gain[i] = streamConfigData.get<QCarCamStreamConfigData::exposureConfig>().gain[i];
                        }
                    }
                    break;
                case QCARCAM_SENSOR_PARAM_GAMMA:
                    {
                        pgammaConfig = (QCarCamGammaConfig_t*)pqConfigData;
                        pgammaConfig->type = static_cast<QCarCamGammaType_e>(streamConfigData.get<QCarCamStreamConfigData::gammaConfig>().type);
                        pgammaConfig->fpValue = streamConfigData.get<QCarCamStreamConfigData::gammaConfig>().fpValue;
                        pgammaConfig->tableSize = streamConfigData.get<QCarCamStreamConfigData::gammaConfig>().tableSize;
                        for (int i=0; i<pgammaConfig->tableSize; i++)
                            pgammaConfig->table[i] = streamConfigData.get<QCarCamStreamConfigData::gammaConfig>().table[i];
                    }
                    break;
                case QCARCAM_SENSOR_PARAM_BRIGHTNESS:
                case QCARCAM_SENSOR_PARAM_CONTRAST:
                case QCARCAM_SENSOR_PARAM_HUE:
                case QCARCAM_SENSOR_PARAM_SATURATION:
                    {
                        pfloatVal = (float*)pqConfigData;
                        *pfloatVal = static_cast<float>(streamConfigData.get<QCarCamStreamConfigData::floatValue>());
                    }
                    break;
                case QCARCAM_SENSOR_PARAM_COLOR_SPACE:
                    {
                        pcolorSpace = (QCarCamColorSpace_e*)pqConfigData;
                        *pcolorSpace = static_cast<QCarCamColorSpace_e>(streamConfigData.get<QCarCamStreamConfigData::colorSpace>());
                    }
                    break;
                case QCARCAM_VENDOR_PARAM:
                    {
                        pvendorParam = (QCarCamVendorParam_t*)pqConfigData;
                        for (int i=0; i<QCARCAM_MAX_VENDOR_PAYLOAD_SIZE; i++)
                            pvendorParam->data[i] = streamConfigData.get<QCarCamStreamConfigData::vendorParam>().data[i];
                    }
                    break;
                default:
                    QCX_AIDL_ERRORMSG("Invalid config param %d", static_cast<int>(qParam));
                    Error = QCarCamError::QCARCAM_RET_FAILED;
                    break;
            }
        }
    }
    return Error;
}
