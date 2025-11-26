/* ===========================================================================
 * Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#include <utils/Log.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "ais_hidl_stream.h"

#define CHECK_BIT(num, pos) ((num) & (0x1<<(pos)))

namespace vendor {
namespace qti {
namespace automotive {
namespace qcarcam {
namespace V2_0 {
namespace implementation {

// Global Stream Array from ais_hidl_camera.cpp
extern gQcarCamClientData gQcarCamClientArray[MAX_AIS_CLIENTS];
extern std::mutex gMultiStreamQLock;
extern std::condition_variable gClientNotifier;


// Event callback from qcarcam engine
QCarCamRet_e ais_hidl_stream::qcarcam_event_cb(const QCarCamHndl_t hndl,
            const uint32_t eventId, const QCarCamEventPayload_t *pPayload, void  *pPrivateData)
{
    AIS_HIDL_INFOMSG("qcarcam_event_cb called");

    // Find the current stream Q
    gQcarCamClientData *pRecord = ais_hidl_stream::findEventQByHndl(hndl);
    if (!pRecord) {
        AIS_HIDL_ERRORMSG("ERROR::Requested qcarcam_hndl %p not found", hndl);
        // Return success to engine for next cb
        return QCARCAM_RET_OK;
    }
    AIS_HIDL_INFOMSG("Found qcarcam_hndl in the list");

    // Push the event into queue
    if (pRecord)
    {
        QCarCamEventPayload_t payload;
        memcpy(&payload, pPayload, sizeof(QCarCamEventPayload_t));
        std::unique_lock<std::mutex> lk(pRecord->gEventQLock);
        pRecord->sQcarcamList.emplace_back(eventId, payload);
        lk.unlock();
        pRecord->gCV.notify_one();
        AIS_HIDL_INFOMSG("Pushed event id %d to Event Queue", (int)eventId);
    }
    return QCARCAM_RET_OK;
}

//Defalut constructors
ais_hidl_stream::ais_hidl_stream(uint32_t inputId, gQcarCamClientData *pHead, QCarCamHndl_t hndl)
{
    AIS_HIDL_INFOMSG("ais_hidl_stream Constructor");
    mStreamObj = nullptr;
    mInputId = inputId;
    pCurrentRecord = pHead;
    mQcarcamHndl = hndl;
    mRunMode = STOPPED;
    mStreaming = false;
    cloneHandle = nullptr;
    if (mRunMode == RUN) {
        // The background thread is already running, so we can't start a new stream
        AIS_HIDL_ERRORMSG("eventDispatcherThread already in RUN state!!");
    } else {
        // Fire up a thread to receive and dispatch the qcarcam frames
        mRunMode = RUN;
        mCaptureThread = std::thread([this](){ eventDispatcherThread(); });
    }
}

//Defalut destructor
ais_hidl_stream::~ais_hidl_stream()
{
    QCarCamRet_e ret;
    if (true == mStreaming) {
        AIS_HIDL_ERRORMSG("Its a abrupt exit, Calling qcarcam stop");
        ret = QCarCamStop(mQcarcamHndl);
        if (ret != QCARCAM_RET_OK) {
            AIS_HIDL_ERRORMSG("QCarCamStop failed with error %d for hndl %p", ret, mQcarcamHndl);
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
        AIS_HIDL_INFOMSG("eventDispatcherThread thread stopped.");
    }

    // Close the current stream
    ret = QCarCamClose(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        AIS_HIDL_ERRORMSG("QCarCamClose failed with error %d for hndl %p", ret, mQcarcamHndl);
    }

    // Close and delete native handle
    native_handle_close(cloneHandle);
    native_handle_delete(cloneHandle);
    cloneHandle = nullptr;

    // Find and delete mQcarcamHndl
    gMultiStreamQLock.lock();
    AIS_HIDL_INFOMSG("Removing qcarcam_context = %p from Q",mQcarcamHndl);
    gQcarCamClientData *pRecord = ais_hidl_stream::findEventQByHndl(mQcarcamHndl);
    if (nullptr != pRecord) {
        pRecord->qcarcam_context = NOT_IN_USE;
    } else
        AIS_HIDL_ERRORMSG("Qcarcam context not found"); // Ignoring this error
    gMultiStreamQLock.unlock();

    // Notify client monitor thread for calling QcarcamUninitialize()
    gClientNotifier.notify_one();

    mStreamObj = nullptr;
    pCurrentRecord = nullptr;
    mStreaming = false;
}

gQcarCamClientData* ais_hidl_stream::findEventQByHndl(const QCarCamHndl_t qcarcam_hndl)
{
    AIS_HIDL_INFOMSG("ais_hidl_stream findEventQByHndl");
    // Find the named camera
    int i = 0;
    for (i=0; i<MAX_AIS_CLIENTS; i++) {
        if (gQcarCamClientArray[i].qcarcam_context == qcarcam_hndl) {
            // Found a match!
            return &gQcarCamClientArray[i];
        }
    }
    AIS_HIDL_ERRORMSG("findEventQByHndl::ERROR qcarcam_context = %p did not match!!\n",qcarcam_hndl);

    // We didn't find a match
    return nullptr;
}

void ais_hidl_stream::copyPlayloadData(QCarCamEventPayload *pPayload, QCarCamEventPayload_t *pqPayload)
{
    int i=0;
    if (pqPayload && pPayload) {
        pPayload->u32Data = pqPayload->u32Data;
        pPayload->errInfo.errorId = static_cast<QCarCamErrorEvent>(pqPayload->errInfo.errorId);
        pPayload->errInfo.errorCode = pqPayload->errInfo.errorCode;
        pPayload->errInfo.frameId = pqPayload->errInfo.frameId;
        pPayload->errInfo.timestamp = pqPayload->errInfo.timestamp;
        pPayload->hwTimestamp.timestamp = pqPayload->hwTimestamp.timestamp;
        pPayload->hwTimestamp.timestampGPTP = pqPayload->hwTimestamp.timestampGPTP;
        for (i=0; i<QCARCAM_MAX_VENDOR_PAYLOAD_SIZE; i++)
            pPayload->vendorData.data[i] = pqPayload->vendorData.data[i];
        pPayload->frameInfo.id = pqPayload->frameInfo.id;
        pPayload->frameInfo.bufferIndex = pqPayload->frameInfo.bufferIndex;
        pPayload->frameInfo.seqNo = pqPayload->frameInfo.seqNo;
        pPayload->frameInfo.timestamp = pqPayload->frameInfo.timestamp;
        pPayload->frameInfo.sofTimestamp.timestamp = pqPayload->frameInfo.sofTimestamp.timestamp;
        pPayload->frameInfo.sofTimestamp.timestampGPTP = pqPayload->frameInfo.sofTimestamp.timestampGPTP;
        pPayload->frameInfo.flags = pqPayload->frameInfo.flags;
        pPayload->frameInfo.fieldType = static_cast<QCarCamInterlaceField>(pqPayload->frameInfo.fieldType);
        pPayload->frameInfo.requestId = pqPayload->frameInfo.requestId;
        pPayload->frameInfo.inputMetaIdx = pqPayload->frameInfo.inputMetaIdx;
        for (i=0; i<QCARCAM_MAX_BATCH_FRAMES; i++) {
            pPayload->frameInfo.batchFramesInfo[i].seqNo = pqPayload->frameInfo.batchFramesInfo[i].seqNo;
            pPayload->frameInfo.batchFramesInfo[i].timestamp.timestamp = pqPayload->frameInfo.batchFramesInfo[i].timestamp.timestamp;
            pPayload->frameInfo.batchFramesInfo[i].timestamp.timestampGPTP = pqPayload->frameInfo.batchFramesInfo[i].timestamp.timestampGPTP;
        }
        pPayload->recovery.msg = static_cast<QCarCamEventRecoveryMsg>(pqPayload->recovery.msg);
        pPayload->recovery.error.errorId = static_cast<QCarCamErrorEvent>(pqPayload->recovery.error.errorId);
        pPayload->recovery.error.errorCode = pqPayload->recovery.error.errorCode;
        pPayload->recovery.error.frameId = pqPayload->recovery.error.frameId;
        pPayload->recovery.error.timestamp = pqPayload->recovery.error.timestamp;
        for (i=0; i<QCARCAM_MAX_PAYLOAD_SIZE; i++)
            pPayload->array[i] = pqPayload->array[i];
    }
}

void ais_hidl_stream::eventDispatcherThread()
{
    AIS_HIDL_INFOMSG("ais_hidl_stream::eventDispatcherThread starting");
    int32_t q_size = 0;

    // pCurrentRecord should be filled in constructor
    if (!this->pCurrentRecord) {
        AIS_HIDL_ERRORMSG("ERROR::Requested qcarcam_hndl not found in the list eventDispatcherThread exiting");
        mRunMode = STOPPED;
        return;
    }
    // Run until our atomic signal is cleared
    while (mRunMode == RUN && this->pCurrentRecord->qcarcam_context == mQcarcamHndl) {
        std::unique_lock<std::mutex> lk(this->pCurrentRecord->gEventQLock);
        // fetch events from event Q
        q_size = this->pCurrentRecord->sQcarcamList.size();
        if (q_size) {
            AIS_HIDL_INFOMSG("Found %d events in the Event Queue", q_size);
            QEventDesc qFront = this->pCurrentRecord->sQcarcamList.front();
            // Convert event to hidl type and call app cb
            uint32_t EventType;
            QCarCamEventPayload Payload;
            EventType = static_cast<uint32_t>(qFront.eventId);
            ais_hidl_stream::copyPlayloadData(&Payload, &qFront.payload);
            this->pCurrentRecord->sQcarcamList.pop_front();
            lk.unlock();
            std::unique_lock<std::mutex> api_lk(mApiLock);
            if (mStreamObj) {
                // In HIDL its mandatory to check return status otherwise hidl
                // generate fake crash
                auto result = mStreamObj->qcarcam_event_callback(EventType, Payload);
                if (!result.isOk())
                    AIS_HIDL_DBGMSG("Event delivery call failed in the transport layer.");
            }
            api_lk.unlock();
        }
        else {
            // Wait for a buffer to be ready
            this->pCurrentRecord->gCV.wait(lk);

            if (mRunMode == STOPPED)
            {
                AIS_HIDL_ERRORMSG("ERROR::Exiting eventDispatcherThread as user has called stop");
                lk.unlock();
                break;
            }

            // fetch events from event Q
            q_size = this->pCurrentRecord->sQcarcamList.size();
            if (q_size) {
                AIS_HIDL_INFOMSG("Found %d events in the Event Queue", q_size);
                QEventDesc qFront = this->pCurrentRecord->sQcarcamList.front();
                // Convert event to hidl type and call app cb
                uint32_t EventType;
                QCarCamEventPayload Payload;
                EventType = static_cast<uint32_t>(qFront.eventId);
                Payload.u32Data = qFront.payload.u32Data;
                ais_hidl_stream::copyPlayloadData(&Payload, &qFront.payload);
                this->pCurrentRecord->sQcarcamList.pop_front();
                lk.unlock();
                std::unique_lock<std::mutex> api_lk(mApiLock);
                if (mStreamObj) {
                    // In HIDL its mandatory to check return status otherwise hidl
                    // generate fake crash
                    auto result = mStreamObj->qcarcam_event_callback(EventType, Payload);
                    if (!result.isOk())
                        AIS_HIDL_DBGMSG("Event delivery call failed in the transport layer.");
                }
                api_lk.unlock();
            }
            else {
                lk.unlock();
                AIS_HIDL_ERRORMSG("Event Queue is Empty");
            }
        }
    }

    // Mark that thread stopped
    AIS_HIDL_INFOMSG("eventDispatcherThread thread exiting for context %p", mQcarcamHndl);
    mRunMode = STOPPED;
}

Return<QCarCamError> ais_hidl_stream::configureStream(QCarCamParamType Param, const QCarCamStreamConfigData& data)  {
    AIS_HIDL_INFOMSG("Entry configureStream");
    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamStreamConfigData StreamData = data;
    bool setParam = true;

    // Convert input into qcarcam type
    Error = config_qcarcam_params(Param, StreamData, setParam);
    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    return Error;
}

Return<QCarCamError> ais_hidl_stream::setStreamBuffers(const hidl_handle& hndl, const QcarcamBuffersInfo& info)  {
    AIS_HIDL_INFOMSG("Entry setStreamBuffers");

    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamRet_e ret = QCARCAM_RET_OK;

    if (!hndl.getNativeHandle()) {
        AIS_HIDL_ERRORMSG("Native handle is null!! sending Error status %d", static_cast<int>(Error));
        Error = QCarCamError::QCARCAM_RET_FAILED;
        return Error;
    }

    cloneHandle = native_handle_clone(hndl.getNativeHandle());
    if (!cloneHandle) {
        AIS_HIDL_ERRORMSG("Unable to clone native handle sending Error status %d", static_cast<int>(Error));
        Error = QCarCamError::QCARCAM_RET_FAILED;
        return Error;
    }

    QCarCamBufferList_t p_buffers;
    memset(&p_buffers, 0, sizeof(QCarCamBufferList_t));
    p_buffers.nBuffers = info.nBuffers;
    p_buffers.colorFmt = static_cast<QCarCamColorFmt_e>(info.colorFmt);
    p_buffers.pBuffers = (QCarCamBuffer_t *)calloc(p_buffers.nBuffers, sizeof(*p_buffers.pBuffers));
    if (!p_buffers.pBuffers) {
        AIS_HIDL_ERRORMSG("Buffers are null!! sending Error status %d", static_cast<int>(Error));
        Error = QCarCamError::QCARCAM_RET_FAILED;
        return Error;
    }

    AIS_HIDL_INFOMSG("Received %d FDs",p_buffers.nBuffers);
    for (int i=0; i<p_buffers.nBuffers; i++)
    {
        p_buffers.pBuffers[i].numPlanes = info.numPlanes;
        AIS_HIDL_INFOMSG("Fd[%d]=%d", i, cloneHandle->data[i]);
        for(int j=0; j< info.numPlanes && j<QCARCAM_MAX_NUM_PLANES; j++) {
            p_buffers.pBuffers[i].planes[j].width = info.planes[j].width;
            p_buffers.pBuffers[i].planes[j].height = info.planes[j].height;
            p_buffers.pBuffers[i].planes[j].stride = info.planes[j].stride;
            p_buffers.pBuffers[i].planes[j].size = info.planes[j].size;
            p_buffers.pBuffers[i].planes[j].memHndl = (uint64_t)(uintptr_t)cloneHandle->data[i];
            p_buffers.pBuffers[i].planes[j].offset = info.planes[j].offset;
        }
    }
    // TODO::Need to handle multiplane for bayer sensors
    //Fill buffer flag info
    p_buffers.flags = info.flags;

    // Call qcarcam API
    ret = QCarCamSetBuffers(mQcarcamHndl, &p_buffers);
    if (ret != QCARCAM_RET_OK) {
        AIS_HIDL_ERRORMSG("QCarCamSetBuffers() failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = static_cast<QCarCamError>(ret);
    }

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    return Error;
}

Return<QCarCamError> ais_hidl_stream::startStream(const sp<IQcarCameraStreamCB>& streamObj)  {
    AIS_HIDL_INFOMSG("Entry startStream");

    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamRet_e ret = QCARCAM_RET_OK;

    // Store the stream object for sending callback
    mStreamObj = streamObj;

    // Register a callback function if streamObj is valid
    if (mStreamObj) {
        ret = QCarCamRegisterEventCallback(mQcarcamHndl, &ais_hidl_stream::qcarcam_event_cb, NULL);
        if (ret != QCARCAM_RET_OK)
            AIS_HIDL_ERRORMSG("QCarCamRegisterEventCallback() failed for callback setting, proceeding without callback");
    }

    // Call qcarcam API
    ret = QCarCamStart(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        AIS_HIDL_ERRORMSG("QCarCamStart() failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = static_cast<QCarCamError>(ret);
        return Error;
    }
    mStreaming = true;

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    return Error;
}

Return<QCarCamError> ais_hidl_stream::stopStream()  {
    AIS_HIDL_INFOMSG("Entry stopStream");

    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamRet_e ret = QCARCAM_RET_OK;

    std::unique_lock<std::mutex> api_lk(mApiLock);

    // Call qcarcam API
    ret = QCarCamStop(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        AIS_HIDL_ERRORMSG("QCarCamStop() failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = static_cast<QCarCamError>(ret);
    }
    mStreaming = false;

    api_lk.unlock();

    // Send back the results
    AIS_HIDL_DBGMSG("sending Error status %d for hndl %p", static_cast<int>(Error), mQcarcamHndl);
    return Error;
}

Return<void> ais_hidl_stream::getFrame(uint64_t timeout, uint32_t flags, getFrame_cb _hidl_cb)  {
    AIS_HIDL_INFOMSG("Entry getFrame");

    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamFrameInfo Info;
    QCarCamRet_e ret = QCARCAM_RET_OK;
    QCarCamFrameInfo_t frame_info;

    // Call qcarcam API
    ret = QCarCamGetFrame(mQcarcamHndl, &frame_info, timeout, flags);
    if (ret != QCARCAM_RET_OK) {
        AIS_HIDL_ERRORMSG("QCarCamGetFrame() failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = static_cast<QCarCamError>(ret);
    } else {
        // Build up QCarCamFrameInfo for return
        Info.id = frame_info.id;
        Info.bufferIndex = frame_info.bufferIndex;
        Info.seqNo = frame_info.seqNo;
        Info.timestamp = frame_info.timestamp;
        Info.sofTimestamp.timestamp = frame_info.sofTimestamp.timestamp;
        Info.sofTimestamp.timestampGPTP = frame_info.sofTimestamp.timestampGPTP;
        Info.flags = frame_info.flags;
        Info.fieldType = static_cast<QCarCamInterlaceField>(frame_info.fieldType);
        Info.requestId = frame_info.requestId;
        Info.inputMetaIdx = frame_info.inputMetaIdx;
        for (int i=0; i<QCARCAM_MAX_BATCH_FRAMES; i++) {
            Info.batchFramesInfo[i].seqNo = frame_info.batchFramesInfo[i].seqNo;
            Info.batchFramesInfo[i].timestamp.timestamp = frame_info.batchFramesInfo[i].timestamp.timestamp;
            Info.batchFramesInfo[i].timestamp.timestampGPTP = frame_info.batchFramesInfo[i].timestamp.timestampGPTP;
        }
    }

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    _hidl_cb(Error, Info);

    // HIDL convention says we return Void if we sent our result back via callback
    return Void();
}

Return<QCarCamError> ais_hidl_stream::releaseFrame(uint32_t id, uint32_t bufferIdx)  {
    AIS_HIDL_INFOMSG("Entry releaseFrame");

    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamRet_e ret = QCARCAM_RET_OK;

    // Call qcarcam API
    ret = QCarCamReleaseFrame(mQcarcamHndl, id, bufferIdx);
    if (ret != QCARCAM_RET_OK) {
        AIS_HIDL_ERRORMSG("QCarCamReleaseFrame() failed with error %d for hndl %p bufferIdx %u", ret, mQcarcamHndl, bufferIdx);
        Error = static_cast<QCarCamError>(ret);
    }

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    return Error;
}

Return<QCarCamError> ais_hidl_stream::pauseStream()  {
    AIS_HIDL_INFOMSG("Entry pauseStream");

    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamRet_e ret = QCARCAM_RET_OK;

    // Call qcarcam API
    ret = QCarCamPause(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        AIS_HIDL_ERRORMSG("QCarCamPause() failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = static_cast<QCarCamError>(ret);
        return Error;
    }
    mStreaming = false;

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    return Error;
}

Return<QCarCamError> ais_hidl_stream::resumeStream()  {
    AIS_HIDL_INFOMSG("Entry resumeStream");

    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamRet_e ret = QCARCAM_RET_OK;

    // Call qcarcam API
    ret = QCarCamResume(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        if (ret == QCARCAM_RET_BADSTATE && mStreaming == true)
        {
            AIS_HIDL_ERRORMSG("The streams have already been resumed !");
        } else {
            mStreaming = false;
            AIS_HIDL_ERRORMSG("QCarCamResume() failed with error %d for hndl %p", ret, mQcarcamHndl);
        }
        Error = static_cast<QCarCamError>(ret);
        return Error;
    }
    mStreaming = true;

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    return Error;
}

//Dummy Implementation to avoid unimplemented pure virtual method error.
Return<QCarCamError>  ais_hidl_stream::reserveStream() {
    AIS_HIDL_INFOMSG("Dummy reserveStream");

    QCarCamError ret = QCarCamError::QCARCAM_RET_OK;
    return ret;
}

//Dummy Implementation to avoid unimplemented pure virtual method error.
Return<QCarCamError>  ais_hidl_stream::releaseStream() {
    AIS_HIDL_INFOMSG("Dummy releaseStream");

    QCarCamError ret = QCarCamError::QCARCAM_RET_OK;
    return ret;
}

Return<void> ais_hidl_stream::getStreamConfig(QCarCamParamType Param, const QCarCamStreamConfigData &data, getStreamConfig_cb _hidl_cb)  {
    AIS_HIDL_INFOMSG("Entry getStreamConfig");

    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamStreamConfigData StreamData = data;
    bool setParam = false;

    // Convert input into qcarcam type
    Error = config_qcarcam_params(Param, StreamData, setParam);

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    _hidl_cb(Error, StreamData);

    // HIDL convention says we return Void if we sent our result back via callback
    return Void();
}

// Utility functions
QCarCamError ais_hidl_stream::config_qcarcam_params(QCarCamParamType Param,
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
                q_uint32Val = static_cast<uint32_t>(streamConfigData.uint64Value);
                param = (void*)&q_uint32Val;
                param_size = sizeof(q_uint32Val);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_SET_CROP:
            {
                param_type = QCARCAM_STREAM_CONFIG_PARAM_SET_CROP;
                q_uint32Val = static_cast<uint32_t>(streamConfigData.uint64Value);
                param = (void*)&q_uint32Val;
                param_size = sizeof(q_uint32Val);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_LATENCY_CONTROL:
            {
                param_type = QCARCAM_STREAM_CONFIG_PARAM_LATENCY_CONTROL;
                q_latencyParam.latencyMax = streamConfigData.latencyParam.latencyMax;
                q_latencyParam.latencyReduceRate = streamConfigData.latencyParam.latencyReduceRate;
                param = (void*)&q_latencyParam;
                param_size = sizeof(q_latencyParam);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL:
            {
                param_type = QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL;
                q_frameDropConfig.frameDropPeriod = streamConfigData.frameDropConfig.frameDropPeriod;
                q_frameDropConfig.frameDropPattern = streamConfigData.frameDropConfig.frameDropPattern;
                param = (void*)&q_frameDropConfig;
                param_size = sizeof(q_frameDropConfig);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_INPUT_MODE:
            {
                param_type = QCARCAM_STREAM_CONFIG_PARAM_INPUT_MODE;
                q_uint32Val = static_cast<uint32_t>(streamConfigData.uint64Value);
                param = (void*)&q_uint32Val;
                param_size = sizeof(q_uint32Val);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_MASTER:
            {
                param_type = QCARCAM_STREAM_CONFIG_PARAM_MASTER;
                q_uint32Val = static_cast<uint32_t>(streamConfigData.uint64Value);
                param = (void*)&q_uint32Val;
                param_size = sizeof(q_uint32Val);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_SUBSCRIBE:
            {
                param_type = QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_SUBSCRIBE;
                q_uint64Val = static_cast<uint64_t>(streamConfigData.uint64Value);
                param = (void*)&q_uint64Val;
                param_size = sizeof(q_uint64Val);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_UNSUBSCRIBE:
            {
                param_type = QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_UNSUBSCRIBE;
                q_uint64Val = static_cast<uint64_t>(streamConfigData.uint64Value);
                param = (void*)&q_uint64Val;
                param_size = sizeof(q_uint64Val);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE:
            {
                param_type = QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE;
                q_batchConfig.mode = static_cast<QCarCamBatchMode_e>(streamConfigData.batchConfig.mode);
                q_batchConfig.numBatchFrames = streamConfigData.batchConfig.numBatchFrames;
                q_batchConfig.frameIncrement = streamConfigData.batchConfig.frameIncrement;
                q_batchConfig.detectFirstPhaseTimer = streamConfigData.batchConfig.detectFirstPhaseTimer;
                param = (void*)&q_batchConfig;
                param_size = sizeof(q_batchConfig);
            }
            break;
        case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE:
            {
                param_type = QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE;
                q_ispConfig.id = streamConfigData.ispConfig.id;
                q_ispConfig.cameraId = streamConfigData.ispConfig.cameraId;
                q_ispConfig.usecaseId = static_cast<QCarCamIspUsecase_e>(streamConfigData.ispConfig.usecaseId);
                q_ispConfig.tuningMode = streamConfigData.ispConfig.tuningMode;
                param = (void*)&q_batchConfig;
                param_size = sizeof(q_batchConfig);
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
                q_uint32Val = static_cast<uint32_t>(streamConfigData.uint64Value);
                param = (void*)&q_uint32Val;
                param_size = sizeof(q_uint32Val);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_MIRROR_V:
            {
                param_type = QCARCAM_SENSOR_PARAM_MIRROR_V;
                q_uint32Val = static_cast<uint32_t>(streamConfigData.uint64Value);
                param = (void*)&q_uint32Val;
                param_size = sizeof(q_uint32Val);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_VID_STD:
            {
                param_type = QCARCAM_SENSOR_PARAM_VID_STD;
                q_uint32Val = static_cast<uint32_t>(streamConfigData.uint64Value);
                param = (void*)&q_uint32Val;
                param_size = sizeof(q_uint32Val);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_CURRENT_VID_STD:
            {
                param_type = QCARCAM_SENSOR_PARAM_CURRENT_VID_STD;
                q_uint32Val = static_cast<uint32_t>(streamConfigData.uint64Value);
                param = (void*)&q_uint32Val;
                param_size = sizeof(q_uint32Val);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_SIGNAL_STATUS:
            {
                param_type = QCARCAM_SENSOR_PARAM_SIGNAL_STATUS;
                q_inputSignal = static_cast<QCarCamInputSignal_e>(streamConfigData.inputSignal);
                param = (void*)&q_inputSignal;
                param_size = sizeof(q_inputSignal);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_EXPOSURE:
            {
                param_type = QCARCAM_SENSOR_PARAM_EXPOSURE;
                q_exposureConfig.mode = static_cast<QCarCamExposureMode_e>(streamConfigData.exposureConfig.mode);
                q_exposureConfig.hdrMode = streamConfigData.exposureConfig.hdrMode;
                q_exposureConfig.numExposures = streamConfigData.exposureConfig.numExposures;
                for (int i=0; i<q_exposureConfig.numExposures; i++) {
                    q_exposureConfig.exposureTime[i] = streamConfigData.exposureConfig.exposureTime[i];
                    q_exposureConfig.exposureRatio[i] = streamConfigData.exposureConfig.exposureRatio[i];
                    q_exposureConfig.gain[i] = streamConfigData.exposureConfig.gain[i];
                }
                param = (void*)&q_exposureConfig;
                param_size = sizeof(q_exposureConfig);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_GAMMA:
            {
                param_type = QCARCAM_SENSOR_PARAM_GAMMA;
                q_gammaConfig.type = static_cast<QCarCamGammaType_e>(streamConfigData.gammaConfig.type);
                q_gammaConfig.fpValue = streamConfigData.gammaConfig.fpValue;
                q_gammaConfig.tableSize = streamConfigData.gammaConfig.tableSize;
                for (int i=0; i<q_gammaConfig.tableSize; i++)
                    q_gammaConfig.table[i] = streamConfigData.gammaConfig.table[i];
                param = (void*)&q_gammaConfig;
                param_size = sizeof(q_gammaConfig);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_BRIGHTNESS:
            {
                param_type = QCARCAM_SENSOR_PARAM_BRIGHTNESS;
                q_floatVal = static_cast<float>(streamConfigData.floatValue);
                param = (void*)&q_floatVal;
                param_size = sizeof(q_floatVal);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_CONTRAST:
            {
                param_type = QCARCAM_SENSOR_PARAM_CONTRAST;
                q_floatVal = static_cast<float>(streamConfigData.floatValue);
                param = (void*)&q_floatVal;
                param_size = sizeof(q_floatVal);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_HUE:
            {
                param_type = QCARCAM_SENSOR_PARAM_HUE;
                q_floatVal = static_cast<float>(streamConfigData.floatValue);
                param = (void*)&q_floatVal;
                param_size = sizeof(q_floatVal);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_SATURATION:
            {
                param_type = QCARCAM_SENSOR_PARAM_SATURATION;
                q_floatVal = static_cast<float>(streamConfigData.floatValue);
                param = (void*)&q_floatVal;
                param_size = sizeof(q_floatVal);
            }
            break;
        case QCarCamParamType::QCARCAM_SENSOR_PARAM_COLOR_SPACE:
            {
                param_type = QCARCAM_SENSOR_PARAM_COLOR_SPACE;
                q_colorSpace = static_cast<QCarCamColorSpace_e>(streamConfigData.colorSpace);
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
                    q_vendorParam.data[i] = streamConfigData.vendorParam.data[i];
                param = (void*)&q_vendorParam;
                param_size = sizeof(q_vendorParam);
            }
            break;
        default:
            AIS_HIDL_ERRORMSG("Invalid config param %d", static_cast<int>(Param));
            Error = QCarCamError::QCARCAM_RET_FAILED;
            break;
    }

    if (true == setParam) {
        if (QCarCamError::QCARCAM_RET_OK == Error) {
            // Call qcarcam API
            ret = QCarCamSetParam(mQcarcamHndl, param_type, param, param_size);
            if (ret != QCARCAM_RET_OK) {
                AIS_HIDL_ERRORMSG("QCarCamSetParam() failed with error %d for param %d", ret, (int)param_type);
                Error = static_cast<QCarCamError>(ret);
            }
        }
    } else {
        if (QCarCamError::QCARCAM_RET_OK == Error) {
            // Call qcarcam API
            ret = QCarCamGetParam(mQcarcamHndl, param_type, param, param_size);
            if (ret != QCARCAM_RET_OK) {
                AIS_HIDL_ERRORMSG("QCarCamSetParam() failed with error %d for param %d", ret, (int)param_type);
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
                        streamConfigData.uint64Value = (uint64_t)q_uint32Val;
                    }
                    break;
                case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_LATENCY_CONTROL:
                    {
                        streamConfigData.latencyParam.latencyMax = q_latencyParam.latencyMax;
                        streamConfigData.latencyParam.latencyReduceRate = q_latencyParam.latencyReduceRate;
                    }
                    break;
                case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_FRAME_DROP_CONTROL:
                    {
                        streamConfigData.frameDropConfig.frameDropPeriod = q_frameDropConfig.frameDropPeriod;
                        streamConfigData.frameDropConfig.frameDropPattern = q_frameDropConfig.frameDropPattern;
                    }
                    break;
                case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_SUBSCRIBE:
                case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_EVENT_CHANGE_UNSUBSCRIBE:
                    {
                        streamConfigData.uint64Value = (uint64_t)q_uint64Val;
                    }
                    break;
                case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_BATCH_MODE:
                    {
                        streamConfigData.batchConfig.mode = static_cast<QCarCamBatchMode>(q_batchConfig.mode);
                        streamConfigData.batchConfig.numBatchFrames = q_batchConfig.numBatchFrames;
                        streamConfigData.batchConfig.frameIncrement = q_batchConfig.frameIncrement;
                        streamConfigData.batchConfig.detectFirstPhaseTimer = q_batchConfig.detectFirstPhaseTimer;
                    }
                    break;
                case QCarCamParamType::QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE:
                    {
                        streamConfigData.ispConfig.id = q_ispConfig.id;
                        streamConfigData.ispConfig.cameraId = q_ispConfig.cameraId;
                        streamConfigData.ispConfig.usecaseId = static_cast<QCarCamIspUsecase>(q_ispConfig.usecaseId);
                        streamConfigData.ispConfig.tuningMode = q_ispConfig.tuningMode;
                    }
                    break;
                case QCarCamParamType::QCARCAM_SENSOR_PARAM_SIGNAL_STATUS:
                    {
                        streamConfigData.inputSignal = static_cast<QCarCamInputSignal>(q_inputSignal);
                    }
                    break;
                case QCarCamParamType::QCARCAM_SENSOR_PARAM_EXPOSURE:
                    {
                        streamConfigData.exposureConfig.mode = static_cast<QCarCamExposureMode>(q_exposureConfig.mode);
                        streamConfigData.exposureConfig.hdrMode = q_exposureConfig.hdrMode;
                        streamConfigData.exposureConfig.numExposures = q_exposureConfig.numExposures;
                        for (int i=0; i<streamConfigData.exposureConfig.numExposures; i++) {
                            streamConfigData.exposureConfig.exposureTime[i] = q_exposureConfig.exposureTime[i];
                            streamConfigData.exposureConfig.exposureRatio[i] = q_exposureConfig.exposureRatio[i];
                            streamConfigData.exposureConfig.gain[i] = q_exposureConfig.gain[i];
                        }
                    }
                    break;
                case QCarCamParamType::QCARCAM_SENSOR_PARAM_GAMMA:
                    {
                        streamConfigData.gammaConfig.type = static_cast<QCarCamGammaType>(q_gammaConfig.type);
                        streamConfigData.gammaConfig.fpValue = q_gammaConfig.fpValue;
                        streamConfigData.gammaConfig.tableSize = q_gammaConfig.tableSize;
                        if (streamConfigData.gammaConfig.tableSize > QCARCAM_MAX_GAMMA_TABLE)
                            streamConfigData.gammaConfig.tableSize = QCARCAM_MAX_GAMMA_TABLE;
                        for (int i=0; i<q_gammaConfig.tableSize; i++)
                            streamConfigData.gammaConfig.table[i] = q_gammaConfig.table[i];
                    }
                    break;
                case QCarCamParamType::QCARCAM_SENSOR_PARAM_BRIGHTNESS:
                case QCarCamParamType::QCARCAM_SENSOR_PARAM_CONTRAST:
                case QCarCamParamType::QCARCAM_SENSOR_PARAM_HUE:
                case QCarCamParamType::QCARCAM_SENSOR_PARAM_SATURATION:
                    {
                        streamConfigData.floatValue = q_floatVal;
                    }
                    break;
                case QCarCamParamType::QCARCAM_SENSOR_PARAM_COLOR_SPACE:
                    {
                        streamConfigData.colorSpace = static_cast<QCarCamColorSpace>(q_colorSpace);
                    }
                    break;
                case QCarCamParamType::QCARCAM_VENDOR_PARAM:
                    {
                        for (int i=0; i<QCARCAM_MAX_VENDOR_PAYLOAD_SIZE; i++)
                            streamConfigData.vendorParam.data[i] = q_vendorParam.data[i];
                    }
                    break;
                default:
                    AIS_HIDL_ERRORMSG("Invalid config param %d", static_cast<int>(Param));
                    Error = QCarCamError::QCARCAM_RET_FAILED;
                    break;
            }
        }
    }
    return Error;
}

} //implementation
} //V2_0
} //qcarcam
} //automotive
} //qti
} //vendor

