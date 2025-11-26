/* ===========================================================================
 * Copyright (c) 2019-2020,2023-2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#include <utils/Log.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <type_traits>
#include <NativeHandle.h>
#include "ais_aidl_stream.h"

#define CHECK_BIT(num, pos) ((num) & (0x1<<(pos)))

namespace aidl {
namespace vendor {
namespace qti {
namespace automotive {
namespace qcarcam2 {

// Global Stream Array from ais_aidl_camera.cpp
extern gQcarCamClientData gQcarCamClientArray[MAX_AIS_CLIENTS];
extern std::mutex gMultiStreamQLock;
extern std::condition_variable gClientNotifier;

// Event callback from qcarcam engine
QCarCamRet_e ais_aidl_stream::qcarcam_event_cb(const QCarCamHndl_t hndl, const uint32_t eventId,
		const QCarCamEventPayload_t *pPayload, void  *pPrivateData)
{
    AIS_AIDL_INFOMSG("qcarcam_event_cb called");

    // Find the current stream Q
    gQcarCamClientData *pRecord = ais_aidl_stream::findEventQByHndl(hndl);
    if (!pRecord) {
        AIS_AIDL_ERRORMSG("ERROR::Requested qcarcam_hndl %p not found", hndl);
        // Return success to engine for next cb
        return QCARCAM_RET_OK;
    }
    AIS_AIDL_INFOMSG("Found qcarcam_hndl in the list");

    // Push the event into queue
    if (pRecord)
    {
        QCarCamEventPayload_t payload;
        memcpy(&payload, pPayload, sizeof(QCarCamEventPayload_t));
        std::unique_lock<std::mutex> lk(pRecord->gEventQLock);
        pRecord->sQcarcamList.emplace_back(eventId, payload);
        lk.unlock();
        pRecord->gCV.notify_one();
        AIS_AIDL_INFOMSG("Pushed event id %d to Event Queue", (int)eventId);
    }
    return QCARCAM_RET_OK;
}

//Default constructors
ais_aidl_stream::ais_aidl_stream(uint32_t inputId, gQcarCamClientData *pHead, QCarCamHndl_t hndl)
{
    AIS_AIDL_INFOMSG("ais_aidl_stream Constructor");
    mStreamObj = nullptr;
    mInputId = inputId;
    pCurrentRecord = pHead;
    mQcarcamHndl = hndl;
    mRunMode = STOPPED;
    mStreaming = false;
    cloneHandle = nullptr;
    if (mRunMode == RUN) {
        // The background thread is already running, so we can't start a new stream
        AIS_AIDL_ERRORMSG("eventDispatcherThread already in RUN state!!");
    } else {
        // Fire up a thread to receive and dispatch the qcarcam frames
        mRunMode = RUN;
        mCaptureThread = std::thread([this](){ eventDispatcherThread(); });
    }
}

//Default destructor
ais_aidl_stream::~ais_aidl_stream()
{
    QCarCamRet_e ret;
    if (true == mStreaming) {
        AIS_AIDL_ERRORMSG("Its a abrupt exit, Calling QCarCamStop");
        ret = QCarCamStop(mQcarcamHndl);
        if (ret != QCARCAM_RET_OK) {
            AIS_AIDL_ERRORMSG("QCarCamStop failed with error %d for hndl %p", ret, mQcarcamHndl);
        }
    }

    // Signal the eventDispatcherThread thread to stop
    if (mRunMode == RUN) {
        mRunMode = STOPPED;
        pCurrentRecord->gCV.notify_one();
        // Block until the background thread is stopped
        if (mCaptureThread.joinable()) {
            mCaptureThread.join();
        }
        AIS_AIDL_INFOMSG("eventDispatcherThread thread stopped.");
    }

    // Close the current stream
    ret = QCarCamClose(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        AIS_AIDL_ERRORMSG("QCarCamClose failed with error %d for hndl %p", ret, mQcarcamHndl);
    }

    // Close and delete native handle
    native_handle_close(cloneHandle);
    native_handle_delete(cloneHandle);
    cloneHandle = nullptr;

    // Find and delete mQcarcamHndl
    gMultiStreamQLock.lock();
    AIS_AIDL_INFOMSG("Removing qcarcam_context = %p from Q",mQcarcamHndl);
    gQcarCamClientData *pRecord = ais_aidl_stream::findEventQByHndl(mQcarcamHndl);
    if (nullptr != pRecord) {
        pRecord->qcarcam_context = NOT_IN_USE;
    } else
        AIS_AIDL_ERRORMSG("Qcarcam context not found"); // Ignoring this error
    gMultiStreamQLock.unlock();

    // Notify client monitor thread for calling QcarcamUninitialize()
    gClientNotifier.notify_one();

    mStreamObj = nullptr;
    pCurrentRecord = nullptr;
    mStreaming = false;
}

gQcarCamClientData* ais_aidl_stream::findEventQByHndl(const QCarCamHndl_t qcarcam_hndl)
{
    AIS_AIDL_INFOMSG("E");
    // Find the named camera
    int i = 0;
    for (i = 0; i < MAX_AIS_CLIENTS; i++) {
        if (gQcarCamClientArray[i].qcarcam_context == qcarcam_hndl) {
            // Found a match!
            return &gQcarCamClientArray[i];
        }
    }
    AIS_AIDL_ERRORMSG("X qcarcam_context = %p\n",qcarcam_hndl);

    // We didn't find a match
    return nullptr;
}

void ais_aidl_stream::copyPayloadData(QCarCamEventPayload *pPayload, QCarCamEventPayload_t *pqPayload)
{
    int i = 0;
    AIS_AIDL_INFOMSG("E");
    if (pqPayload && pPayload) {
        pPayload->set<QCarCamEventPayload::u32Data>(static_cast<int32_t>(pqPayload->u32Data));
        //QCarCamErrorInfo_t temp_errinfo;
        class QCarCamErrorInfo temp_errinfo;
        temp_errinfo.errorId = static_cast<QCarCamErrorEvent>(pqPayload->errInfo.errorId);
        temp_errinfo.errorCode = pqPayload->errInfo.errorCode;
        temp_errinfo.frameId = pqPayload->errInfo.frameId;
        temp_errinfo.timestamp = pqPayload->errInfo.timestamp;
        pPayload->set<QCarCamEventPayload::errInfo>(temp_errinfo);

        class QCarCamHWTimestamp temp_hwtimestamp;
        temp_hwtimestamp.timestamp = pqPayload->hwTimestamp.timestamp;
        temp_hwtimestamp.timestampGPTP = pqPayload->hwTimestamp.timestampGPTP;
        pPayload->set<QCarCamEventPayload::hwTimestamp>(temp_hwtimestamp);

        class QCarCamVendorParam temp_vendorparam;
        for (i=0; i<QCARCAM_MAX_VENDOR_PAYLOAD_SIZE; i++)
            temp_vendorparam.data[i] = pqPayload->vendorData.data[i];
        pPayload->set<QCarCamEventPayload::vendorData>(temp_vendorparam);

        class QCarCamFrameInfo temp_frameinfo;
        temp_frameinfo.id = pqPayload->frameInfo.id;
        temp_frameinfo.bufferIndex = pqPayload->frameInfo.bufferIndex;
        temp_frameinfo.seqNo = pqPayload->frameInfo.seqNo;
        temp_frameinfo.timestamp = pqPayload->frameInfo.timestamp;
        temp_frameinfo.sofTimestamp.timestamp = pqPayload->frameInfo.sofTimestamp.timestamp;
        temp_frameinfo.sofTimestamp.timestampGPTP = pqPayload->frameInfo.sofTimestamp.timestampGPTP;
        temp_frameinfo.flags = pqPayload->frameInfo.flags;
        temp_frameinfo.fieldType = static_cast<QCarCamInterlaceField>(pqPayload->frameInfo.fieldType);
        temp_frameinfo.requestId = pqPayload->frameInfo.requestId;
        temp_frameinfo.inputMetaIdx = pqPayload->frameInfo.inputMetaIdx;

        for (i=0; i<QCARCAM_MAX_BATCH_FRAMES; i++) {
            temp_frameinfo.batchFramesInfo[i].seqNo = pqPayload->frameInfo.batchFramesInfo[i].seqNo;
            temp_frameinfo.batchFramesInfo[i].timestamp.timestamp = pqPayload->frameInfo.batchFramesInfo[i].timestamp.timestamp;
            temp_frameinfo.batchFramesInfo[i].timestamp.timestampGPTP =
                pqPayload->frameInfo.batchFramesInfo[i].timestamp.timestampGPTP;
        }
        pPayload->set<QCarCamEventPayload::frameInfo>(temp_frameinfo);

        class QCarCamRecovery temp_recovery;
        temp_recovery.msg = static_cast<QCarCamEventRecoveryMsg>(pqPayload->recovery.msg);
        temp_recovery.error.errorId = static_cast<QCarCamErrorEvent>(pqPayload->recovery.error.errorId);
        temp_recovery.error.errorCode = pqPayload->recovery.error.errorCode;
        temp_recovery.error.frameId = pqPayload->recovery.error.frameId;
        temp_recovery.error.timestamp = pqPayload->recovery.error.timestamp;
        pPayload->set<QCarCamEventPayload::recovery>(temp_recovery);

        std::array<uint8_t, QCARCAM_MAX_PAYLOAD_SIZE> arr;
        for (i=0; i<QCARCAM_MAX_PAYLOAD_SIZE; i++)
            arr[i]  = pqPayload->array[i];
        pPayload->set<QCarCamEventPayload::array>(arr);
    }
    AIS_AIDL_INFOMSG("X");
}

void ais_aidl_stream::eventDispatcherThread()
{
    AIS_AIDL_INFOMSG("E");
    int32_t q_size = 0;

    // pCurrentRecord should be filled in constructor
    if (!this->pCurrentRecord) {
        AIS_AIDL_ERRORMSG("ERROR::Requested qcarcam_hndl not found in the list eventDispatcherThread exiting");
        mRunMode = STOPPED;
        return;
    }
    // Run until our atomic signal is cleared
    while (mRunMode == RUN && this->pCurrentRecord->qcarcam_context == mQcarcamHndl) {
        std::unique_lock<std::mutex> lk(this->pCurrentRecord->gEventQLock);
        // fetch events from event Q
        q_size = this->pCurrentRecord->sQcarcamList.size();
        if (q_size) {
            AIS_AIDL_INFOMSG("Found %d events in the Event Queue", q_size);
            QEventDesc qFront = this->pCurrentRecord->sQcarcamList.front();
            // Convert event to aidl type and call app cb
            uint32_t EventType;
            QCarCamEventPayload Payload;
            EventType = static_cast<uint32_t>(qFront.eventId);
            ais_aidl_stream::copyPayloadData(&Payload, &qFront.payload);
            this->pCurrentRecord->sQcarcamList.pop_front();
            lk.unlock();
            std::unique_lock<std::mutex> api_lk(mApiLock);
            if (mStreamObj) {
                // In AIDL its mandatory to check return status otherwise aidl
                // generate fake crash
                auto result = mStreamObj->qcarcam_event_callback(EventType, Payload);
                if (!result.isOk()){
                    AIS_AIDL_DBGMSG("Event delivery call failed in the transport layer.");}
                else if(result.isOk()){
                    AIS_AIDL_DBGMSG("qcarcam_event_callback: Event delivery call success");}
            }
            api_lk.unlock();
        }
        else {
            // Wait for a buffer to be ready
            this->pCurrentRecord->gCV.wait(lk);

            if (mRunMode == STOPPED)
            {
                AIS_AIDL_ERRORMSG("ERROR::Exiting eventDispatcherThread as user has called stop");
                lk.unlock();
                break;
            }

            // fetch events from event Q
            q_size = this->pCurrentRecord->sQcarcamList.size();
            if (q_size) {
                AIS_AIDL_INFOMSG("Found %d events in the Event Queue", q_size);
                QEventDesc qFront = this->pCurrentRecord->sQcarcamList.front();
                // Convert event to aidl type and call app cb
                uint32_t EventType;
                QCarCamEventPayload Payload;
                EventType = static_cast<uint32_t>(qFront.eventId);
                Payload.set<QCarCamEventPayload::u32Data>(static_cast<int32_t>(qFront.payload.u32Data));
                ais_aidl_stream::copyPayloadData(&Payload, &qFront.payload);
                this->pCurrentRecord->sQcarcamList.pop_front();
                lk.unlock();
                std::unique_lock<std::mutex> api_lk(mApiLock);
                if (mStreamObj) {
                    // In AIDL its mandatory to check return status otherwise aidl
                    // generate fake crash
                    auto result = mStreamObj->qcarcam_event_callback(EventType, Payload);
                    if (!result.isOk()){
                        AIS_AIDL_DBGMSG("Event delivery call failed in the transport layer.");}
                    else if (result.isOk()){
                        AIS_AIDL_DBGMSG("qcarcam_event_callback Event delivery call success");}

                }
                api_lk.unlock();
            }
            else {
                lk.unlock();
                AIS_AIDL_ERRORMSG("Event Queue is Empty");
            }
        }
    }

    // Mark that thread stopped
    AIS_AIDL_INFOMSG("X eventDispatcherThread thread exiting for context %p", mQcarcamHndl);
    mRunMode = STOPPED;
}

::ndk::ScopedAStatus ais_aidl_stream::configureStream(::aidl::vendor::qti::automotive::qcarcam2::QCarCamParamType in_Param, const ::aidl::vendor::qti::automotive::qcarcam2::QCarCamStreamConfigData& in_data, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) {
    AIS_AIDL_INFOMSG("E");
    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamStreamConfigData StreamData = in_data;
    bool setParam = true;

    // Convert input into qcarcam type
    Error = config_qcarcam_params(in_Param, StreamData, setParam);
    // Send back the results
    AIS_AIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    _aidl_return = &Error;
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_stream::setStreamBuffers(const ::aidl::android::hardware::common::NativeHandle& in_hndl, const ::aidl::vendor::qti::automotive::qcarcam2::QcarcamBuffersInfo& in_info, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) {
    AIS_AIDL_INFOMSG("E");

    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamRet_e ret = QCARCAM_RET_OK;
    struct native_handle* aNativeHndl = nullptr;

    if ((aNativeHndl = ::android::dupFromAidl(in_hndl)) == nullptr) {
        AIS_AIDL_ERRORMSG("Native handle is null!! sending Error status %d", static_cast<int>(Error));
        Error = QCarCamError::QCARCAM_RET_FAILED;
        _aidl_return = &Error;
        return ndk::ScopedAStatus::ok();
    }

    cloneHandle = native_handle_clone(aNativeHndl);
    if (!cloneHandle) {
        AIS_AIDL_ERRORMSG("Unable to clone native handle sending Error status %d", static_cast<int>(Error));
        Error = QCarCamError::QCARCAM_RET_FAILED;
        _aidl_return = &Error;
        return ndk::ScopedAStatus::ok();
    }

    QCarCamBufferList_t p_buffers;
    memset(&p_buffers, 0, sizeof(QCarCamBufferList_t));
    p_buffers.nBuffers = in_info.nBuffers;
    p_buffers.colorFmt = static_cast<QCarCamColorFmt_e>(in_info.colorFmt);
    p_buffers.pBuffers = (QCarCamBuffer_t *)calloc(p_buffers.nBuffers, sizeof(*p_buffers.pBuffers));
    if (!p_buffers.pBuffers) {
        AIS_AIDL_ERRORMSG("Buffers are null!! sending Error status %d", static_cast<int>(Error));
        Error = QCarCamError::QCARCAM_RET_FAILED;
        _aidl_return = &Error;
        return ndk::ScopedAStatus::ok();
    }

    AIS_AIDL_INFOMSG("Received %d FDs",p_buffers.nBuffers);
    for (int i=0; i<p_buffers.nBuffers; i++)
    {
        p_buffers.pBuffers[i].numPlanes = in_info.numPlanes;
        AIS_AIDL_INFOMSG("Fd[%d]=%d", i, cloneHandle->data[i]);
        for(int j = 0; j < in_info.numPlanes && j < QCARCAM_MAX_NUM_PLANES; j++) {
            p_buffers.pBuffers[i].planes[j].width = in_info.planes[j].width;
            p_buffers.pBuffers[i].planes[j].height = in_info.planes[j].height;
            p_buffers.pBuffers[i].planes[j].stride = in_info.planes[j].stride;
            p_buffers.pBuffers[i].planes[j].size = in_info.planes[j].size;
            p_buffers.pBuffers[i].planes[j].memHndl = (uint64_t)(uintptr_t)cloneHandle->data[i];
            p_buffers.pBuffers[i].planes[j].offset = in_info.planes[j].offset;
        }
    }
    // TODO::Need to handle multiplane for bayer sensors
    //Fill buffer flag in_info
    p_buffers.flags = in_info.flags;

    // Call qcarcam API
    ret = QCarCamSetBuffers(mQcarcamHndl, &p_buffers);
    if (ret != QCARCAM_RET_OK) {
        AIS_AIDL_ERRORMSG("QCarCamSetBuffers() failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = static_cast<QCarCamError>(ret);
    }

    // Send back the results
    AIS_AIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    _aidl_return = &Error;
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_stream::startStream(const std::shared_ptr<::aidl::vendor::qti::automotive::qcarcam2::IQcarCameraStreamCB>& in_streamObj, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) {
    AIS_AIDL_INFOMSG("E");

    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamRet_e ret = QCARCAM_RET_OK;

    // Store the stream object for sending callback
    mStreamObj = in_streamObj;

    // Register a callback function if in_streamObj is valid
    if (mStreamObj) {
        ret = QCarCamRegisterEventCallback(mQcarcamHndl, &ais_aidl_stream::qcarcam_event_cb, NULL);
        if (ret != QCARCAM_RET_OK)
            AIS_AIDL_ERRORMSG("QCarCamRegisterEventCallback() failed for callback setting, proceeding without callback");
    }

    // Call qcarcam API
    ret = QCarCamStart(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        AIS_AIDL_ERRORMSG("QCarCamStart() failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = static_cast<QCarCamError>(ret);
        return ndk::ScopedAStatus::ok();
    }
    mStreaming = true;

    // Send back the results
    AIS_AIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    _aidl_return = &Error;
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_stream::stopStream(::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) {
    AIS_AIDL_INFOMSG("E");

    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamRet_e ret = QCARCAM_RET_OK;

    std::unique_lock<std::mutex> api_lk(mApiLock);

    // Call qcarcam API
    ret = QCarCamStop(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        AIS_AIDL_ERRORMSG("QCarCamStop() failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = static_cast<QCarCamError>(ret);
    }
    mStreaming = false;

    api_lk.unlock();

    // Send back the results
    AIS_AIDL_DBGMSG("sending Error status %d for hndl %p", static_cast<int>(Error), mQcarcamHndl);
    _aidl_return = &Error;
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_stream::getFrame(int64_t in_timeout, int32_t in_flags, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamFrameInfo* out_Info, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) {
    AIS_AIDL_INFOMSG("E");

    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    //QCarCamFrameInfo Info;
    QCarCamRet_e ret = QCARCAM_RET_OK;
    QCarCamFrameInfo_t frame_info;

    // Call qcarcam API
    ret = QCarCamGetFrame(mQcarcamHndl, &frame_info, in_timeout, in_flags);
    if (ret != QCARCAM_RET_OK) {
        AIS_AIDL_ERRORMSG("QCarCamGetFrame() failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = static_cast<QCarCamError>(ret);
    } else {
        // Build up QCarCamFrameInfo for return
        out_Info->id = frame_info.id;
        out_Info->bufferIndex = frame_info.bufferIndex;
        out_Info->seqNo = frame_info.seqNo;
        out_Info->timestamp = frame_info.timestamp;
        out_Info->sofTimestamp.timestamp = frame_info.sofTimestamp.timestamp;
        out_Info->sofTimestamp.timestampGPTP = frame_info.sofTimestamp.timestampGPTP;
        out_Info->flags = frame_info.flags;
        out_Info->fieldType = static_cast<QCarCamInterlaceField>(frame_info.fieldType);
        out_Info->requestId = frame_info.requestId;
        out_Info->inputMetaIdx = frame_info.inputMetaIdx;
        for (int i=0; i<QCARCAM_MAX_BATCH_FRAMES; i++) {
            out_Info->batchFramesInfo[i].seqNo = frame_info.batchFramesInfo[i].seqNo;
            out_Info->batchFramesInfo[i].timestamp.timestamp = frame_info.batchFramesInfo[i].timestamp.timestamp;
            out_Info->batchFramesInfo[i].timestamp.timestampGPTP = frame_info.batchFramesInfo[i].timestamp.timestampGPTP;
        }
    }

    // Send back the results
    AIS_AIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));

    // AIDL convention says we return Void if we sent our result back via callback
    _aidl_return = &Error;
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_stream::releaseFrame(int32_t in_id, int32_t in_bufferIdx, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) {
    AIS_AIDL_INFOMSG("E");

    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamRet_e ret = QCARCAM_RET_OK;

    // Call qcarcam API
    ret = QCarCamReleaseFrame(mQcarcamHndl, in_id, in_bufferIdx);
    if (ret != QCARCAM_RET_OK) {
        AIS_AIDL_ERRORMSG("QCarCamReleaseFrame failed with error %d for hndl %p bufferIdx %u", ret, mQcarcamHndl, in_bufferIdx);
        Error = static_cast<QCarCamError>(ret);
    }

    // Send back the results
    AIS_AIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    _aidl_return = &Error;
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_stream::pauseStream(::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) {
    AIS_AIDL_INFOMSG("E");

    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamRet_e ret = QCARCAM_RET_OK;

    // Call qcarcam API
    ret = QCarCamPause(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        AIS_AIDL_ERRORMSG("QCarCamPause() failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = static_cast<QCarCamError>(ret);
        return ndk::ScopedAStatus::ok();
    }
    mStreaming = false;

    // Send back the results
    AIS_AIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    _aidl_return = &Error;
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_stream::resumeStream(::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) {
    AIS_AIDL_INFOMSG("E");

    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamRet_e ret = QCARCAM_RET_OK;

    // Call qcarcam API
    ret = QCarCamResume(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        if (ret == QCARCAM_RET_BADSTATE && mStreaming == true)
        {
            AIS_AIDL_ERRORMSG("The streams have already been resumed !");
        } else {
            mStreaming = false;
            AIS_AIDL_ERRORMSG("QCarCamResume() failed with error %d for hndl %p", ret, mQcarcamHndl);
        }
        Error = static_cast<QCarCamError>(ret);
        _aidl_return = &Error;
        return ndk::ScopedAStatus::ok();
    }
    mStreaming = true;

    // Send back the results
    AIS_AIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    _aidl_return = &Error;
    return ndk::ScopedAStatus::ok();
}


::ndk::ScopedAStatus ais_aidl_stream::reserveStream(::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) {
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_stream::releaseStream(::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) {
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_stream::submitRequest(const ::aidl::vendor::qti::automotive::qcarcam2::QCarCamRequest& in_Info, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) {
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_stream::getStreamConfig(::aidl::vendor::qti::automotive::qcarcam2::QCarCamParamType in_Param, const ::aidl::vendor::qti::automotive::qcarcam2::QCarCamStreamConfigData& in_data, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamStreamConfigData* out_datatype, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) {
    AIS_AIDL_INFOMSG("E");

    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamStreamConfigData StreamData = in_data;
    bool setParam = false;

    // Convert input into qcarcam type
    Error = config_qcarcam_params(in_Param, StreamData, setParam);

    switch (in_Param)
    {
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK:
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_SET_CROP:
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_INPUT_MODE:
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_MASTER:
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_MIRROR_H:
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_MIRROR_V:
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_VID_STD:
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_CURRENT_VID_STD:
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_SUBSCRIBE:
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_UNSUBSCRIBE:
            {
                out_datatype->set<QCarCamStreamConfigData::uint64Value>(static_cast<uint64_t>(StreamData.uint64Value));
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_LATENCY_CONTROL:
            {
                QCarCamLatencyControl tmp_latencyParam;
                tmp_latencyParam.latencyMax = StreamData.get<QCarCamStreamConfigData::latencyParam>().latencyMax;
                tmp_latencyParam.latencyReduceRate = StreamData.get<QCarCamStreamConfigData::latencyParam>().latencyReduceRate;
                out_datatype->set<QCarCamStreamConfigData::latencyParam>(tmp_latencyParam);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL:
            {
                QCarCamFrameDropConfig tmp_frameDropConfig;
                tmp_frameDropConfig.frameDropPeriod = StreamData.get<QCarCamStreamConfigData::frameDropConfig>().frameDropPeriod;
                tmp_frameDropConfig.frameDropPattern = StreamData.get<QCarCamStreamConfigData::frameDropConfig>().frameDropPattern;
                out_datatype->set<QCarCamStreamConfigData::frameDropConfig>(tmp_frameDropConfig);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE:
            {
                QCarCamBatchConfig tmp_batchConfig;
                tmp_batchConfig.mode = StreamData.get<QCarCamStreamConfigData::batchConfig>().mode;
                tmp_batchConfig.numBatchFrames = StreamData.get<QCarCamStreamConfigData::batchConfig>().numBatchFrames;
                tmp_batchConfig.frameIncrement = StreamData.get<QCarCamStreamConfigData::batchConfig>().frameIncrement;
                tmp_batchConfig.detectFirstPhaseTimer = StreamData.get<QCarCamStreamConfigData::batchConfig>().detectFirstPhaseTimer;
                out_datatype->set<QCarCamStreamConfigData::batchConfig>(tmp_batchConfig);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE:
            {
                QCarCamIspUsecaseConfig tmp_ispConfig;
                tmp_ispConfig.id = StreamData.get<QCarCamStreamConfigData::ispConfig>().id;
                tmp_ispConfig.cameraId = StreamData.get<QCarCamStreamConfigData::ispConfig>().cameraId;
                tmp_ispConfig.usecaseId = StreamData.get<QCarCamStreamConfigData::ispConfig>().usecaseId;
                tmp_ispConfig.tuningMode = StreamData.get<QCarCamStreamConfigData::ispConfig>().tuningMode;
                out_datatype->set<QCarCamStreamConfigData::ispConfig>(tmp_ispConfig);
            }
            break;
         case QCarCamParamType::QCARCAM_SENSOR_PARAM_SIGNAL_STATUS:
            {
                out_datatype->set<QCarCamStreamConfigData::inputSignal>(static_cast<QCarCamInputSignal>(StreamData.inputSignal));
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_EXPOSURE:
            {
                QCarCamExposureConfig tmp_exposureConfig;
                tmp_exposureConfig.mode = StreamData.get<QCarCamStreamConfigData::exposureConfig>().mode;
                tmp_exposureConfig.hdrMode = StreamData.get<QCarCamStreamConfigData::exposureConfig>().hdrMode;
                tmp_exposureConfig.numExposures = StreamData.get<QCarCamStreamConfigData::exposureConfig>().numExposures;
                for (int i = 0; i < 4; i++){
                    tmp_exposureConfig.exposureTime[i] = StreamData.get<QCarCamStreamConfigData::exposureConfig>().exposureTime[i];
                    tmp_exposureConfig.exposureRatio[i] = StreamData.get<QCarCamStreamConfigData::exposureConfig>().exposureRatio[i];
                    tmp_exposureConfig.gain[i] = StreamData.get<QCarCamStreamConfigData::exposureConfig>().gain[i];
                }
                out_datatype->set<QCarCamStreamConfigData::exposureConfig>(tmp_exposureConfig);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_GAMMA:
            {
                QCarCamGammaConfig tmp_gammaConfig;
                tmp_gammaConfig.type = StreamData.get<QCarCamStreamConfigData::gammaConfig>().type;
                tmp_gammaConfig.fpValue = StreamData.get<QCarCamStreamConfigData::gammaConfig>().fpValue;
                tmp_gammaConfig.tableSize = StreamData.get<QCarCamStreamConfigData::gammaConfig>().tableSize;
                for (int i = 0; i < tmp_gammaConfig.tableSize; i++)
                    tmp_gammaConfig.table[i] = StreamData.get<QCarCamStreamConfigData::gammaConfig>().table[i];
                out_datatype->set<QCarCamStreamConfigData::gammaConfig>(tmp_gammaConfig);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_BRIGHTNESS:
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_CONTRAST:
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_HUE:
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_SATURATION:
            {
                out_datatype->set<QCarCamStreamConfigData::floatValue>(static_cast<float>(StreamData.floatValue));
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_COLOR_SPACE:
            {
                out_datatype->set<QCarCamStreamConfigData::colorSpace>(static_cast<QCarCamColorSpace>(StreamData.colorSpace));
            }
            break;
        case QCarCamParamType::QCARCAM_VENDOR_PARAM:
            {
                QCarCamVendorParam tmp_vendorParam;
                for (int i = 0; i < QCARCAM_MAX_VENDOR_PAYLOAD_SIZE; i++)
                    tmp_vendorParam.data[i] = StreamData.get<QCarCamStreamConfigData::vendorParam>().data[i];
                out_datatype->set<QCarCamStreamConfigData::vendorParam>(tmp_vendorParam);
            }
            break;
        default:
            AIS_AIDL_ERRORMSG("Invalid config param %d", static_cast<int>(in_Param));
            Error = QCarCamError::QCARCAM_RET_FAILED;
            break;
    }

    // Send back the results
    AIS_AIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));

    // AIDL convention says we return Void if we sent our result back via callback
    _aidl_return = &Error;
    return ndk::ScopedAStatus::ok();
}

QCarCamError ais_aidl_stream::config_qcarcam_params(QCarCamParamType Param,
        QCarCamStreamConfigData& streamConfigData, bool setParam)
{
    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamRet_e ret = QCARCAM_RET_OK;
    QCarCamParamType_e param_type;

    //qcarcam param types
    uint32_t q_uint32Val = 0;
    uint32_t q_uint64Val = 0;
    float q_floatVal = 0.0;
    QCarCamExposureConfig_t q_exposureConfig;
    QCarCamFrameDropConfig_t q_frameDropConfig;
    QCarCamGammaConfig_t q_gammaConfig;
    QCarCamVendorParam_t q_vendorParam;
    QCarCamLatencyControl_t q_latencyParam;
    QCarCamBatchConfig_t q_batchConfig;
    QCarCamIspUsecaseConfig_t q_ispConfig;
    QCarCamInputSignal_e q_inputSignal;
    QCarCamColorSpace_e q_colorSpace;

    // general data to hold qcarcam types
    void *param = NULL;
    uint32_t param_size = 0;

    switch (Param)
    {
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK:
            {
                param_type = QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK;
                q_uint32Val = static_cast<uint32_t>(streamConfigData.get<QCarCamStreamConfigData::uint64Value>());
                param = (void*)&q_uint32Val;
                param_size = sizeof(q_uint32Val);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_SET_CROP:
            {
                param_type = QCARCAM_STREAM_CONFIG_PARAM_SET_CROP;
                q_uint32Val = static_cast<uint32_t>(streamConfigData.get<QCarCamStreamConfigData::uint64Value>());
                param = (void*)&q_uint32Val;
                param_size = sizeof(q_uint32Val);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_LATENCY_CONTROL:
            {
                param_type = QCARCAM_STREAM_CONFIG_PARAM_LATENCY_CONTROL;
                q_latencyParam.latencyMax = streamConfigData.get<QCarCamStreamConfigData::latencyParam>().latencyMax;
                q_latencyParam.latencyReduceRate = streamConfigData.get<QCarCamStreamConfigData::latencyParam>().latencyReduceRate;
                param = (void*)&q_latencyParam;
                param_size = sizeof(q_latencyParam);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL:
            {
                param_type = QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL;
                q_frameDropConfig.frameDropPeriod = streamConfigData.get<QCarCamStreamConfigData::frameDropConfig>().frameDropPeriod;
                q_frameDropConfig.frameDropPattern = streamConfigData.get<QCarCamStreamConfigData::frameDropConfig>().frameDropPattern;
                param = (void*)&q_frameDropConfig;
                param_size = sizeof(q_frameDropConfig);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_INPUT_MODE:
            {
                param_type = QCARCAM_STREAM_CONFIG_PARAM_INPUT_MODE;
                q_uint32Val = static_cast<uint32_t>(streamConfigData.get<QCarCamStreamConfigData::uint64Value>());
                param = (void*)&q_uint32Val;
                param_size = sizeof(q_uint32Val);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_MASTER:
            {
                param_type = QCARCAM_STREAM_CONFIG_PARAM_MASTER;
                q_uint32Val = static_cast<uint32_t>(streamConfigData.get<QCarCamStreamConfigData::uint64Value>());
                param = (void*)&q_uint32Val;
                param_size = sizeof(q_uint32Val);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_SUBSCRIBE:
            {
                param_type = QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_SUBSCRIBE;
                q_uint64Val = static_cast<uint32_t>(streamConfigData.get<QCarCamStreamConfigData::uint64Value>());
                param = (void*)&q_uint64Val;
                param_size = sizeof(q_uint64Val);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_UNSUBSCRIBE:
            {
                param_type = QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_UNSUBSCRIBE;
                q_uint64Val = static_cast<uint32_t>(streamConfigData.get<QCarCamStreamConfigData::uint64Value>());
                param = (void*)&q_uint64Val;
                param_size = sizeof(q_uint64Val);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE:
            {
                param_type = QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE;
                q_batchConfig.mode = static_cast<QCarCamBatchMode_e>(streamConfigData.get<QCarCamStreamConfigData::batchConfig>().mode);
                q_batchConfig.numBatchFrames = streamConfigData.get<QCarCamStreamConfigData::batchConfig>().numBatchFrames;
                q_batchConfig.frameIncrement = streamConfigData.get<QCarCamStreamConfigData::batchConfig>().frameIncrement;
                q_batchConfig.detectFirstPhaseTimer = streamConfigData.get<QCarCamStreamConfigData::batchConfig>().detectFirstPhaseTimer;
                param = (void*)&q_batchConfig;
                param_size = sizeof(q_batchConfig);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE:
            {
                param_type = QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE;
                q_ispConfig.id = streamConfigData.get<QCarCamStreamConfigData::ispConfig>().id;
                q_ispConfig.cameraId = streamConfigData.get<QCarCamStreamConfigData::ispConfig>().cameraId;
                q_ispConfig.usecaseId = static_cast<QCarCamIspUsecase_e>(streamConfigData.get<QCarCamStreamConfigData::ispConfig>().usecaseId);
                q_ispConfig.tuningMode = streamConfigData.get<QCarCamStreamConfigData::ispConfig>().tuningMode;
                param = (void*)&q_ispConfig;
                param_size = sizeof(q_ispConfig);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_BASE:
            {
                param_type = QCARCAM_SENSOR_PARAM_BASE;
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_MIRROR_H:
            {
                param_type = QCARCAM_SENSOR_PARAM_MIRROR_H;
                q_uint32Val = static_cast<uint32_t>(streamConfigData.get<QCarCamStreamConfigData::uint64Value>());
                param = (void*)&q_uint32Val;
                param_size = sizeof(q_uint32Val);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_MIRROR_V:
            {
                param_type = QCARCAM_SENSOR_PARAM_MIRROR_V;
                q_uint32Val = static_cast<uint32_t>(streamConfigData.get<QCarCamStreamConfigData::uint64Value>());
                param = (void*)&q_uint32Val;
                param_size = sizeof(q_uint32Val);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_VID_STD:
            {
                param_type = QCARCAM_SENSOR_PARAM_VID_STD;
                q_uint32Val = static_cast<uint32_t>(streamConfigData.get<QCarCamStreamConfigData::uint64Value>());
                param = (void*)&q_uint32Val;
                param_size = sizeof(q_uint32Val);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_CURRENT_VID_STD:
            {
                param_type = QCARCAM_SENSOR_PARAM_CURRENT_VID_STD;
                q_uint32Val = static_cast<uint32_t>(streamConfigData.get<QCarCamStreamConfigData::uint64Value>());
                param = (void*)&q_uint32Val;
                param_size = sizeof(q_uint32Val);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_SIGNAL_STATUS:
            {
                param_type = QCARCAM_SENSOR_PARAM_SIGNAL_STATUS;
                q_inputSignal = static_cast<QCarCamInputSignal_e>(streamConfigData.get<QCarCamStreamConfigData::inputSignal>());
                param = (void*)&q_inputSignal;
                param_size = sizeof(q_inputSignal);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_EXPOSURE:
            {
                param_type = QCARCAM_SENSOR_PARAM_EXPOSURE;
                q_exposureConfig.mode = static_cast<QCarCamExposureMode_e>(streamConfigData.get<QCarCamStreamConfigData::exposureConfig>().mode);
                q_exposureConfig.hdrMode = streamConfigData.get<QCarCamStreamConfigData::exposureConfig>().hdrMode;
                q_exposureConfig.numExposures = streamConfigData.get<QCarCamStreamConfigData::exposureConfig>().numExposures;
                for (int i=0; i<4; i++) {
                    q_exposureConfig.exposureTime[i] = streamConfigData.get<QCarCamStreamConfigData::exposureConfig>().exposureTime[i];
                    q_exposureConfig.exposureRatio[i] = streamConfigData.get<QCarCamStreamConfigData::exposureConfig>().exposureRatio[i];
                    q_exposureConfig.gain[i] = streamConfigData.get<QCarCamStreamConfigData::exposureConfig>().gain[i];
                }
                param = (void*)&q_exposureConfig;
                param_size = sizeof(q_exposureConfig);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_GAMMA:
            {
                param_type = QCARCAM_SENSOR_PARAM_GAMMA;
                q_gammaConfig.type = static_cast<QCarCamGammaType_e>(streamConfigData.get<QCarCamStreamConfigData::gammaConfig>().type);
                q_gammaConfig.fpValue = streamConfigData.get<QCarCamStreamConfigData::gammaConfig>().fpValue;
                q_gammaConfig.tableSize = streamConfigData.get<QCarCamStreamConfigData::gammaConfig>().tableSize;
                for (int i=0; i < streamConfigData.get<QCarCamStreamConfigData::gammaConfig>().tableSize; i++)
                    q_gammaConfig.table[i] = streamConfigData.get<QCarCamStreamConfigData::gammaConfig>().table[i];
                param = (void*)&q_gammaConfig;
                param_size = sizeof(q_gammaConfig);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_BRIGHTNESS:
            {
                param_type = QCARCAM_SENSOR_PARAM_BRIGHTNESS;
                q_floatVal = static_cast<float>(streamConfigData.get<QCarCamStreamConfigData::floatValue>());
                param = (void*)&q_floatVal;
                param_size = sizeof(q_floatVal);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_CONTRAST:
            {
                param_type = QCARCAM_SENSOR_PARAM_CONTRAST;
                q_floatVal = static_cast<float>(streamConfigData.get<QCarCamStreamConfigData::floatValue>());
                param = (void*)&q_floatVal;
                param_size = sizeof(q_floatVal);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_HUE:
            {
                param_type = QCARCAM_SENSOR_PARAM_HUE;
                q_floatVal = static_cast<float>(streamConfigData.get<QCarCamStreamConfigData::floatValue>());
                param = (void*)&q_floatVal;
                param_size = sizeof(q_floatVal);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_SATURATION:
            {
                param_type = QCARCAM_SENSOR_PARAM_SATURATION;
                q_floatVal = static_cast<float>(streamConfigData.get<QCarCamStreamConfigData::floatValue>());
                param = (void*)&q_floatVal;
                param_size = sizeof(q_floatVal);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_COLOR_SPACE:
            {
                param_type = QCARCAM_SENSOR_PARAM_COLOR_SPACE;
                q_colorSpace = static_cast<QCarCamColorSpace_e>(streamConfigData.get<QCarCamStreamConfigData::colorSpace>());
                //q_colorSpace = static_cast<QCarCamColorSpace_e>(streamConfigData.colorSpace);
                param = (void*)&q_colorSpace;
                param_size = sizeof(q_colorSpace);
            }
            break;
        case QCarCamParamType::QCARCAM_VENDOR_PARAM_BASE:
            {
                param_type = QCARCAM_VENDOR_PARAM_BASE;
            }
            break;
        case QCarCamParamType::QCARCAM_VENDOR_PARAM:
            {
                param_type = QCARCAM_VENDOR_PARAM;
                for (int i=0; i<QCARCAM_MAX_VENDOR_PAYLOAD_SIZE; i++)
                    q_vendorParam.data[i] = streamConfigData.get<QCarCamStreamConfigData::vendorParam>().data[i];
                param = (void*)&q_vendorParam;
                param_size = sizeof(q_vendorParam);
            }
            break;
        default:
            AIS_AIDL_ERRORMSG("Invalid config param %d", static_cast<int>(Param));
            Error = QCarCamError::QCARCAM_RET_FAILED;
            break;
    }

    if (true == setParam) {
        if (QCarCamError::QCARCAM_RET_OK == Error) {
            // Call qcarcam API
            ret = QCarCamSetParam(mQcarcamHndl, param_type, param, param_size);
            if (ret != QCARCAM_RET_OK) {
                AIS_AIDL_ERRORMSG("QCarCamSetParam() failed with error %d for param %d", ret, (int)param_type);
                Error = static_cast<QCarCamError>(ret);
            }
        }
    } else {
        if (QCarCamError::QCARCAM_RET_OK == Error) {
            // Call qcarcam API
            ret = QCarCamGetParam(mQcarcamHndl, param_type, param, param_size);
            if (ret != QCARCAM_RET_OK) {
                AIS_AIDL_ERRORMSG("QCarCamSetParam() failed with error %d for param %d", ret, (int)param_type);
                Error = static_cast<QCarCamError>(ret);
            }
        }
        if (QCarCamError::QCARCAM_RET_OK == Error) {
            switch(Param)
            {
                case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK:
                case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_SET_CROP:
                case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_INPUT_MODE:
                case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_MASTER:
                case QCarCamParamType::QCARCAM_SENSOR_PARAM_MIRROR_H:
                case QCarCamParamType::QCARCAM_SENSOR_PARAM_MIRROR_V:
                case QCarCamParamType::QCARCAM_SENSOR_PARAM_VID_STD:
                case QCarCamParamType::QCARCAM_SENSOR_PARAM_CURRENT_VID_STD:
                    {
                        streamConfigData.set<QCarCamStreamConfigData::uint64Value>(static_cast<uint64_t>(q_uint32Val));
                    }
                    break;
                case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_LATENCY_CONTROL:
                    {
                        streamConfigData.get<QCarCamStreamConfigData::latencyParam>().latencyMax = q_latencyParam.latencyMax;
                        streamConfigData.get<QCarCamStreamConfigData::latencyParam>().latencyReduceRate = q_latencyParam.latencyReduceRate;
                    }
                    break;
                case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL:
                    {
                        streamConfigData.get<QCarCamStreamConfigData::frameDropConfig>().frameDropPeriod = q_frameDropConfig.frameDropPeriod;
                        streamConfigData.get<QCarCamStreamConfigData::frameDropConfig>().frameDropPattern = q_frameDropConfig.frameDropPattern;
                    }
                    break;
                case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_SUBSCRIBE:
                case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_UNSUBSCRIBE:
                    {
                        streamConfigData.set<QCarCamStreamConfigData::uint64Value>(static_cast<uint64_t>(q_uint64Val));
                    }
                    break;
                case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE:
                    {
                        streamConfigData.get<QCarCamStreamConfigData::batchConfig>().mode = static_cast<QCarCamBatchMode>(q_batchConfig.mode);
                        streamConfigData.get<QCarCamStreamConfigData::batchConfig>().numBatchFrames = q_batchConfig.numBatchFrames;
                        streamConfigData.get<QCarCamStreamConfigData::batchConfig>().frameIncrement = q_batchConfig.frameIncrement;
                        streamConfigData.get<QCarCamStreamConfigData::batchConfig>().detectFirstPhaseTimer = q_batchConfig.detectFirstPhaseTimer;
                    }
                    break;
                case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE:
                    {
                        streamConfigData.get<QCarCamStreamConfigData::ispConfig>().id = q_ispConfig.id;
                        streamConfigData.get<QCarCamStreamConfigData::ispConfig>().cameraId = q_ispConfig.cameraId;
                        streamConfigData.get<QCarCamStreamConfigData::ispConfig>().usecaseId = static_cast<QCarCamIspUsecase>(q_ispConfig.usecaseId);
                        streamConfigData.get<QCarCamStreamConfigData::ispConfig>().tuningMode = q_ispConfig.tuningMode;
                    }
                    break;
                case QCarCamParamType::QCARCAM_SENSOR_PARAM_SIGNAL_STATUS:
                    {
                        streamConfigData.set<QCarCamStreamConfigData::inputSignal>(static_cast<QCarCamInputSignal>(q_inputSignal));
                    }
                    break;
                case QCarCamParamType::QCARCAM_SENSOR_PARAM_EXPOSURE:
                    {
                        streamConfigData.get<QCarCamStreamConfigData::exposureConfig>().mode = static_cast<QCarCamExposureMode>(q_exposureConfig.mode);
                        streamConfigData.get<QCarCamStreamConfigData::exposureConfig>().hdrMode = q_exposureConfig.hdrMode;
                        streamConfigData.get<QCarCamStreamConfigData::exposureConfig>().numExposures = q_exposureConfig.numExposures;

                        for (int i=0; i<4; i++) {
                            streamConfigData.get<QCarCamStreamConfigData::exposureConfig>().exposureTime[i] = q_exposureConfig.exposureTime[i];
                            streamConfigData.get<QCarCamStreamConfigData::exposureConfig>().exposureRatio[i] = q_exposureConfig.exposureRatio[i];
                            streamConfigData.get<QCarCamStreamConfigData::exposureConfig>().gain[i] = q_exposureConfig.gain[i];
                        }
                    }
                    break;
                case QCarCamParamType::QCARCAM_SENSOR_PARAM_GAMMA:
                    {
                        streamConfigData.get<QCarCamStreamConfigData::gammaConfig>().type = static_cast<QCarCamGammaType>(q_gammaConfig.type);
                        streamConfigData.get<QCarCamStreamConfigData::gammaConfig>().fpValue = q_gammaConfig.fpValue;
                        streamConfigData.get<QCarCamStreamConfigData::gammaConfig>().tableSize = q_gammaConfig.tableSize;
                        for (int i=0; i<q_gammaConfig.tableSize; i++)
                            streamConfigData.get<QCarCamStreamConfigData::gammaConfig>().table[i] = q_gammaConfig.table[i];
                    }
                    break;
                case QCarCamParamType::QCARCAM_SENSOR_PARAM_BRIGHTNESS:
                case QCarCamParamType::QCARCAM_SENSOR_PARAM_CONTRAST:
                case QCarCamParamType::QCARCAM_SENSOR_PARAM_HUE:
                case QCarCamParamType::QCARCAM_SENSOR_PARAM_SATURATION:
                    {
                        streamConfigData.set<QCarCamStreamConfigData::floatValue>(static_cast<float>(q_floatVal));
                    }
                    break;
                case QCarCamParamType::QCARCAM_SENSOR_PARAM_COLOR_SPACE:
                    {
                        streamConfigData.set<QCarCamStreamConfigData::colorSpace>(static_cast<QCarCamColorSpace>(q_colorSpace));
                    }
                    break;
                case QCarCamParamType::QCARCAM_VENDOR_PARAM:
                    {
                        for (int i=0; i<QCARCAM_MAX_VENDOR_PAYLOAD_SIZE; i++)
                            streamConfigData.get<QCarCamStreamConfigData::vendorParam>().data[i] = q_vendorParam.data[i];
                    }
                    break;
                default:
                    AIS_AIDL_ERRORMSG("Invalid config param %d", static_cast<int>(Param));
                    Error = QCarCamError::QCARCAM_RET_FAILED;
                    break;
            }
        }
    }
    return Error;
}

} // qcarcam2
} // automotive
} // qti
} // vendor
} // aidl

