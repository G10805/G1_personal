/* ===========================================================================
 * Copyright (c) 2019-2020,2024 Qualcomm Technologies, Inc.
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
namespace V1_0 {
namespace implementation {

// Global Stream Array from ais_hidl_camera.cpp
extern gQcarCamClientData gQcarCamClientArray[MAX_AIS_CLIENTS];
extern std::mutex gMultiStreamQLock;
extern std::condition_variable gClientNotifier;


// Event callback from qcarcam engine
void ais_hidl_stream::qcarcam_event_cb(qcarcam_hndl_t hndl, qcarcam_event_t event_id, qcarcam_event_payload_t *p_payload)
{
    AIS_HIDL_INFOMSG("qcarcam_event_cb called");

    // Find the current stream Q
    gQcarCamClientData *pRecord = findEventQByHndl(hndl);
    if (!pRecord) {
        AIS_HIDL_ERRORMSG("ERROR::Requested qcarcam_hndl %p not found", hndl);
        return;
    }
    AIS_HIDL_INFOMSG("Found qcarcam_hndl in the list");

    // Push the event into queue
    if (pRecord)
    {
        qcarcam_event_payload_t payload;
        payload.uint_payload = p_payload->uint_payload;
        memcpy(payload.array, p_payload->array, sizeof(payload.array));
        std::unique_lock<std::mutex> lk(pRecord->gEventQLock);
        pRecord->sQcarcamList.emplace_back(event_id, payload);
        lk.unlock();
        pRecord->gCV.notify_one();
        AIS_HIDL_INFOMSG("Pushed event id %d to Event Queue", (int)event_id);
    }
}

void ais_hidl_stream::destroyNativeHandle(native_handle_t** handle)
{
    if (!handle || !(*handle))
        return;

    native_handle_close(*handle);
    native_handle_delete(*handle);

    *handle = nullptr;
}

//Defalut constructors
ais_hidl_stream::ais_hidl_stream(QcarcamInputDesc Desc, gQcarCamClientData *pHead, qcarcam_hndl_t hndl)
{
    AIS_HIDL_INFOMSG("ais_hidl_stream Constructor");
    mStreamObj = nullptr;
    mStreamObj2 = nullptr;
    mDesc = Desc;
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
    qcarcam_ret_t ret;
    if (true == mStreaming) {
        AIS_HIDL_ERRORMSG("Its a abrupt exit, Calling qcarcam stop");
        ret = qcarcam_stop(mQcarcamHndl);
        if (ret != QCARCAM_RET_OK) {
            AIS_HIDL_ERRORMSG("qcarcam_stop failed with error %d for hndl %p", ret, mQcarcamHndl);
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
    ret = qcarcam_close(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        AIS_HIDL_ERRORMSG("qcarcam_close failed with error %d for hndl %p", ret, mQcarcamHndl);
    }

    // Close and delete native handle
    destroyNativeHandle(&cloneHandle);

    // Find and delete mQcarcamHndl
    gMultiStreamQLock.lock();
    AIS_HIDL_INFOMSG("Removing qcarcam_context = %p from Q",mQcarcamHndl);
    gQcarCamClientData *pRecord = ais_hidl_stream::findEventQByHndl(mQcarcamHndl);
    if (nullptr != pRecord) {
        pRecord->qcarcam_context = NOT_IN_USE;
    } else
        AIS_HIDL_ERRORMSG("Qcarcam context not found"); // Ignoring this error
    gMultiStreamQLock.unlock();

    // Notify client monitor thread for calling qcarcam_uninitialize()
    gClientNotifier.notify_one();

    mStreamObj = nullptr;
    mStreamObj2 = nullptr;
    pCurrentRecord = nullptr;
    mStreaming = false;
}

gQcarCamClientData* ais_hidl_stream::findEventQByHndl(const qcarcam_hndl_t qcarcam_hndl)
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
            vendor::qti::automotive::qcarcam::V1_1::QcarcamEvent EventType;
            vendor::qti::automotive::qcarcam::V1_1::QcarcamEventPayload Payload;
            EventType = static_cast<vendor::qti::automotive::qcarcam::V1_1::QcarcamEvent>(qFront.event_id);
            memcpy(&Payload.array, &qFront.payload.array, sizeof(Payload.array));
            this->pCurrentRecord->sQcarcamList.pop_front();
            lk.unlock();
            std::unique_lock<std::mutex> api_lk(mApiLock);
            if (mStreamObj) {
                // In HIDL its mandatory to check return status otherwise hidl
                // generate fake crash
                auto result = mStreamObj->qcarcam_event_callback_1_1(EventType, Payload);
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
                vendor::qti::automotive::qcarcam::V1_1::QcarcamEvent EventType;
                vendor::qti::automotive::qcarcam::V1_1::QcarcamEventPayload Payload;
                EventType = static_cast<vendor::qti::automotive::qcarcam::V1_1::QcarcamEvent>(qFront.event_id);
                memcpy(&Payload.array, &qFront.payload.array, sizeof(Payload.array));
                this->pCurrentRecord->sQcarcamList.pop_front();
                lk.unlock();
                std::unique_lock<std::mutex> api_lk(mApiLock);
                if (mStreamObj) {
                    // In HIDL its mandatory to check return status otherwise hidl
                    // generate fake crash
                    auto result = mStreamObj->qcarcam_event_callback_1_1(EventType, Payload);
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


Return<QcarcamError> ais_hidl_stream::configureStream(QcarcamStreamParam Param, const QcarcamStreamConfigData& data)  {
    AIS_HIDL_INFOMSG("Entry configureStream");
    QcarcamError Error = QcarcamError::QCARCAM_OK;
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    qcarcam_param_t q_param_type;
    qcarcam_param_value_t q_param_value;
    uint32_t p_value[128];
    if (data.gamma_config.config_type == static_cast<QcarcamGammaType>(QCARCAM_GAMMA_KNEEPOINTS))
        q_param_value.gamma_config.gamma.table.p_value = p_value;

    // Convert input into qcarcam type
    Error = get_qcarcam_params(Param, data, &q_param_type, &q_param_value, NULL);

    if (QcarcamError::QCARCAM_OK == Error) {
        // Call qcarcam API
        ret = qcarcam_s_param(mQcarcamHndl, q_param_type, &q_param_value);
        if (ret != QCARCAM_RET_OK) {
            AIS_HIDL_ERRORMSG("qcarcam_s_param failed with error %d for param %d", ret, (int)q_param_type);
            Error = QcarcamError::QCARCAM_FAILED;
        }
    }

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    return Error;
}

Return<QcarcamError> ais_hidl_stream::configureStream_1_1(vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam Param, const vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamConfigData& data)  {
    AIS_HIDL_INFOMSG("Entry configureStream_1_1");
    QcarcamError Error = QcarcamError::QCARCAM_OK;
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    qcarcam_param_t q_param_type;
    qcarcam_param_value_t q_param_value;
    uint32_t p_value[128];
    if (data.gamma_config.config_type == static_cast<QcarcamGammaType>(QCARCAM_GAMMA_KNEEPOINTS))
        q_param_value.gamma_config.gamma.table.p_value = p_value;

    // Convert input into qcarcam type
    Error = get_qcarcam_params_1_1(Param, data, &q_param_type, &q_param_value, NULL);

    if (QcarcamError::QCARCAM_OK == Error) {
        // Call qcarcam API
        ret = qcarcam_s_param(mQcarcamHndl, q_param_type, &q_param_value);
        if (ret != QCARCAM_RET_OK) {
            AIS_HIDL_ERRORMSG("qcarcam_s_param failed with error %d for param %d", ret, (int)q_param_type);
            Error =  (QcarcamError)ret;
        }
    }

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    return Error;
}

Return<QcarcamError> ais_hidl_stream::setStreamBuffers(const hidl_handle& hndl, const QcarcamBuffersInfo& info)  {
    AIS_HIDL_INFOMSG("Entry setStreamBuffers");

    QcarcamError Error = QcarcamError::QCARCAM_OK;
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    if (!hndl.getNativeHandle()) {
        AIS_HIDL_ERRORMSG("Native handle is null!! sending Error status %d", static_cast<int>(Error));
        Error = QcarcamError::QCARCAM_FAILED;
        return Error;
    }

    if(cloneHandle)
    {
       //Handle previously cloned, so this is new buffers/resolution. delete previous cloneHandle.
       destroyNativeHandle(&cloneHandle);
    }

    cloneHandle = native_handle_clone(hndl.getNativeHandle());
    if (!cloneHandle) {
        AIS_HIDL_ERRORMSG("Unable to clone native handle sending Error status %d", static_cast<int>(Error));
        Error = QcarcamError::QCARCAM_FAILED;
        return Error;
    }

    qcarcam_buffers_t p_buffers;
    memset(&p_buffers, 0, sizeof(qcarcam_buffers_t));
    p_buffers.n_buffers = cloneHandle->numFds;
    p_buffers.color_fmt = static_cast<qcarcam_color_fmt_t>(info.color_fmt);
    p_buffers.buffers = (qcarcam_buffer_t *)calloc(p_buffers.n_buffers, sizeof(*p_buffers.buffers));
    if (!p_buffers.buffers) {
        AIS_HIDL_ERRORMSG("Buffers are null!! sending Error status %d", static_cast<int>(Error));
        Error = QcarcamError::QCARCAM_FAILED;
        return Error;
    }

    AIS_HIDL_INFOMSG("Received %d FDs",p_buffers.n_buffers);
    for (int i=0; i<cloneHandle->numFds; i++)
    {
        p_buffers.buffers[i].n_planes = info.n_planes;
        AIS_HIDL_INFOMSG("Fd[%d]=%d", i, cloneHandle->data[i]);
        for(int j=0; j< info.n_planes && j< MAX_NUM_PLANES; j++) {
            p_buffers.buffers[i].planes[j].width = info.planes[j].width;
            p_buffers.buffers[i].planes[j].height = info.planes[j].height;
            p_buffers.buffers[i].planes[j].stride = info.planes[j].stride;
            p_buffers.buffers[i].planes[j].size = info.planes[j].size;
        }
        p_buffers.buffers[i].planes[0].p_buf = (void*)(uintptr_t)cloneHandle->data[i];
    }
    //Fill buffer flag info
    p_buffers.flags = info.flags;

    // Call qcarcam API
    ret = qcarcam_s_buffers(mQcarcamHndl, &p_buffers);
    if (ret != QCARCAM_RET_OK) {
        AIS_HIDL_ERRORMSG("qcarcam_s_buffers failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = QcarcamError::QCARCAM_FAILED;
    }

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    return Error;
}

Return<QcarcamError> ais_hidl_stream::setStreamBuffers_1_1(const hidl_handle& hndl, const vendor::qti::automotive::qcarcam::V1_1::QcarcamBuffersInfoList& info)  {
    AIS_HIDL_INFOMSG("Entry setStreamBuffers_1_1");

    QcarcamError Error = QcarcamError::QCARCAM_OK;
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    if (!hndl.getNativeHandle()) {
        AIS_HIDL_ERRORMSG("Native handle is null!! sending Error status %d", static_cast<int>(Error));
        Error = QcarcamError::QCARCAM_FAILED;
        return Error;
    }

    if(cloneHandle)
    {
        //Handle previously cloned, so this is new buffers/resolution. delete previous cloneHandle.
        destroyNativeHandle(&cloneHandle);
    }

    cloneHandle = native_handle_clone(hndl.getNativeHandle());
    if (!cloneHandle) {
        AIS_HIDL_ERRORMSG("Unable to clone native handle sending Error status %d", static_cast<int>(Error));
        Error = QcarcamError::QCARCAM_FAILED;
        return Error;
    }

    qcarcam_bufferlist_t p_buffers;
    memset(&p_buffers, 0, sizeof(qcarcam_bufferlist_t));
    p_buffers.n_buffers = cloneHandle->numFds;
    p_buffers.buffers = (qcarcam_buffer_v2_t *)calloc(p_buffers.n_buffers, sizeof(*p_buffers.buffers));
    if (!p_buffers.buffers) {
        AIS_HIDL_ERRORMSG("Buffers are null!! sending Error status %d", static_cast<int>(Error));
        Error = QcarcamError::QCARCAM_FAILED;
        return Error;
    }
    p_buffers.id = static_cast<unsigned int>(info.id);
    AIS_HIDL_INFOMSG("Received %d FDs",p_buffers.n_buffers);
    for (int i=0; i<cloneHandle->numFds; i++)
    {
        p_buffers.buffers[i].n_planes = info.BuffersInfo[i].n_planes;
        p_buffers.color_fmt = static_cast<qcarcam_color_fmt_t>(info.BuffersInfo[i].color_fmt);
        AIS_HIDL_INFOMSG("Fd[%d]=%d", i, cloneHandle->data[i]);
        for(int j=0; j< info.BuffersInfo[i].n_planes && j< MAX_NUM_PLANES; j++) {
            p_buffers.buffers[i].planes[j].width = info.BuffersInfo[i].planes[j].width;
            p_buffers.buffers[i].planes[j].height = info.BuffersInfo[i].planes[j].height;
            p_buffers.buffers[i].planes[j].stride = info.BuffersInfo[i].planes[j].stride;
            p_buffers.buffers[i].planes[j].size = info.BuffersInfo[i].planes[j].size;
            p_buffers.buffers[i].planes[j].hndl = info.BuffersInfo[i].planes[j].hndl;
            p_buffers.buffers[i].planes[j].offset = info.BuffersInfo[i].planes[j].offset;
            p_buffers.buffers[i].planes[j].hndl = (unsigned long long)cloneHandle->data[i];
        }
        p_buffers.flags = info.BuffersInfo[i].flags;
    }
    //Fill buffer flag info

    // Call qcarcam API
    ret = qcarcam_s_buffers_v2(mQcarcamHndl, &p_buffers);
    if (ret != QCARCAM_RET_OK) {
        AIS_HIDL_ERRORMSG("qcarcam_s_buffers failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = QcarcamError::QCARCAM_FAILED;
    }

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    return Error;
}

Return<QcarcamError> ais_hidl_stream::startStream(const sp<IQcarCameraStreamCB>& streamObj)  {
    AIS_HIDL_INFOMSG("Entry startStream");

    QcarcamError Error = QcarcamError::QCARCAM_OK;
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    // Store the stream object for sending callback
    mStreamObj2 = streamObj;

    // Register a callback function if streamObj is valid
    if (mStreamObj) {
        qcarcam_param_value_t param;
        param.ptr_value = (void *)&ais_hidl_stream::qcarcam_event_cb;
        ret = qcarcam_s_param(mQcarcamHndl, QCARCAM_PARAM_EVENT_CB, &param);
        if (ret != QCARCAM_RET_OK)
            AIS_HIDL_ERRORMSG("qcarcam_s_param failed for callback setting, proceeding without callback");
    }

    // Call qcarcam API
    ret = qcarcam_start(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        AIS_HIDL_ERRORMSG("qcarcam_start failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = QcarcamError::QCARCAM_FAILED;
        return Error;
    }
    mStreaming = true;

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    return Error;
}

Return<QcarcamError> ais_hidl_stream::startStream_1_1(const sp<vendor::qti::automotive::qcarcam::V1_1::IQcarCameraStreamCB>& streamObj)  {
    AIS_HIDL_INFOMSG("Entry startStream");

    QcarcamError Error = QcarcamError::QCARCAM_OK;
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    // Store the stream object for sending callback
    mStreamObj = streamObj;

    // Register a callback function if streamObj is valid
    if (mStreamObj) {
        qcarcam_param_value_t param;
        param.ptr_value = (void *)&ais_hidl_stream::qcarcam_event_cb;
        ret = qcarcam_s_param(mQcarcamHndl, QCARCAM_PARAM_EVENT_CB, &param);
        if (ret != QCARCAM_RET_OK)
            AIS_HIDL_ERRORMSG("qcarcam_s_param failed for callback setting, proceeding without callback");
    }

    // Call qcarcam API
    ret = qcarcam_start(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        AIS_HIDL_ERRORMSG("qcarcam_start failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = QcarcamError::QCARCAM_FAILED;
        return Error;
    }
    mStreaming = true;

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    return Error;
}

Return<QcarcamError> ais_hidl_stream::stopStream()  {
    AIS_HIDL_INFOMSG("Entry stopStream");

    QcarcamError Error = QcarcamError::QCARCAM_OK;
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    std::unique_lock<std::mutex> api_lk(mApiLock);

    // Call qcarcam API
    ret = qcarcam_stop(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        AIS_HIDL_ERRORMSG("qcarcam_stop failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = QcarcamError::QCARCAM_FAILED;
    }
    mStreaming = false;

    api_lk.unlock();

    // Send back the results
    AIS_HIDL_DBGMSG("sending Error status %d for hndl %p", static_cast<int>(Error), mQcarcamHndl);
    return Error;
}

Return<QcarcamError> ais_hidl_stream::stopStream_1_1()  {
    AIS_HIDL_INFOMSG("Entry stopStream");

    QcarcamError Error = QcarcamError::QCARCAM_OK;
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    std::unique_lock<std::mutex> api_lk(mApiLock);

    // Call qcarcam API
    ret = qcarcam_stop(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        AIS_HIDL_ERRORMSG("qcarcam_stop failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = QcarcamError::QCARCAM_FAILED;
    }
    mStreaming = false;

    api_lk.unlock();

    // Send back the results
    AIS_HIDL_DBGMSG("sending Error status %d for hndl %p", static_cast<int>(Error), mQcarcamHndl);
    return Error;
}

Return<void> ais_hidl_stream::getFrame(uint64_t timeout, uint32_t flags, getFrame_cb _hidl_cb)  {
    AIS_HIDL_INFOMSG("Entry getFrame");

    QcarcamError Error = QcarcamError::QCARCAM_OK;
    QcarcamFrameInfo Info;
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    qcarcam_frame_info_t frame_info;
    memset(&Info, 0x0, sizeof(Info));

    // Call qcarcam API
    ret = qcarcam_get_frame(mQcarcamHndl, &frame_info, timeout, flags);
    if (ret != QCARCAM_RET_OK) {
        AIS_HIDL_ERRORMSG("qcarcam_get_frame failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = QcarcamError::QCARCAM_FAILED;
    } else {
        // Build up QcarcamFrameInfo for return
        Info.idx = frame_info.idx;
        Info.flags = frame_info.flags;
        Info.seq_no = frame_info.seq_no;
        Info.timestamp = frame_info.timestamp;
        Info.timestamp_system = frame_info.timestamp_system;
        Info.field_type = static_cast<QcarcamFrameField>(frame_info.field_type);
    }

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    _hidl_cb(Error, Info);

    // HIDL convention says we return Void if we sent our result back via callback
    return Void();
}

Return<void> ais_hidl_stream::getFrame_1_1(uint64_t timeout, uint32_t flags, getFrame_1_1_cb _hidl_cb)  {
    AIS_HIDL_INFOMSG("Entry getFrame_1_1");

    QcarcamError Error = QcarcamError::QCARCAM_OK;
    vendor::qti::automotive::qcarcam::V1_1::QcarcamFrameInfov2 Info = {};
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    qcarcam_frame_info_v2_t frame_info;

    // Call qcarcam API
    ret = qcarcam_get_frame_v2(mQcarcamHndl, &frame_info, timeout, flags);
    if (ret != QCARCAM_RET_OK) {
        AIS_HIDL_ERRORMSG("qcarcam_get_frame failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = QcarcamError::QCARCAM_FAILED;
    } else {
        // Build up QcarcamFrameInfo for return
        Info.id = frame_info.id;
        Info.idx = frame_info.idx;
        Info.flags = frame_info.flags;
        for(unsigned int i = 0; i < QCARCAM_MAX_BATCH_FRAMES; i++) {
            Info.seq_no[i] = frame_info.seq_no[i];
            Info.sof_qtimestamp[i] = frame_info.sof_qtimestamp[i];
        }
        Info.timestamp = frame_info.timestamp;
        Info.timestamp_system = frame_info.timestamp_system;
        Info.field_type = static_cast<QcarcamFrameField>(frame_info.field_type);
    }

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    _hidl_cb(Error, Info);

    // HIDL convention says we return Void if we sent our result back via callback
    return Void();
}

Return<QcarcamError> ais_hidl_stream::releaseFrame(uint32_t idx)  {
    AIS_HIDL_INFOMSG("Entry releaseFrame");

    QcarcamError Error = QcarcamError::QCARCAM_OK;
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    // Call qcarcam API
    ret = qcarcam_release_frame(mQcarcamHndl, idx);
    if (ret != QCARCAM_RET_OK) {
        AIS_HIDL_ERRORMSG("qcarcam_release_frame failed with error %d for hndl %p idx %u", ret, mQcarcamHndl, idx);
        Error = QcarcamError::QCARCAM_FAILED;
    }

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    return Error;
}

Return<QcarcamError> ais_hidl_stream::releaseFrame_1_1(uint32_t id,uint32_t idx)  {
    AIS_HIDL_INFOMSG("Entry releaseFrame_1_1");

    QcarcamError Error = QcarcamError::QCARCAM_OK;
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    // Call qcarcam API
    ret = qcarcam_release_frame_v2(mQcarcamHndl,id,idx);
    if (ret != QCARCAM_RET_OK) {
        AIS_HIDL_ERRORMSG("qcarcam_release_frame failed with error %d for hndl %p idx %u", ret, mQcarcamHndl, idx);
        Error = QcarcamError::QCARCAM_FAILED;
    }

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    return Error;
}

Return<QcarcamError> ais_hidl_stream::pauseStream()  {
    AIS_HIDL_INFOMSG("Entry pauseStream");

    QcarcamError Error = QcarcamError::QCARCAM_OK;
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    // Call qcarcam API
    ret = qcarcam_pause(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        AIS_HIDL_ERRORMSG("qcarcam_pause failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = QcarcamError::QCARCAM_FAILED;
        return Error;
    }
    mStreaming = false;

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    return Error;
}

Return<QcarcamError> ais_hidl_stream::resumeStream()  {
    AIS_HIDL_INFOMSG("Entry resumeStream");

    QcarcamError Error = QcarcamError::QCARCAM_OK;
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    // Call qcarcam API
    ret = qcarcam_resume(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        if (ret == QCARCAM_RET_BADSTATE && mStreaming == true)
        {
            AIS_HIDL_ERRORMSG("The streams have already been resumed !");
            return QcarcamError::QCARCAM_BADSTATE;
        } else {
            mStreaming = false;
            AIS_HIDL_ERRORMSG("qcarcam_resume failed with error %d for hndl %p", ret, mQcarcamHndl);
            Error = QcarcamError::QCARCAM_FAILED;
            return Error;
        }
    }
    mStreaming = true;

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    return Error;
}

Return<void> ais_hidl_stream::getStreamConfig(QcarcamStreamParam Param, const QcarcamStreamConfigData& data, getStreamConfig_cb _hidl_cb)  {
    AIS_HIDL_INFOMSG("Entry getStreamConfig");

    QcarcamError Error = QcarcamError::QCARCAM_OK;
    QcarcamStreamConfigData StreamData = data;
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    qcarcam_param_t q_param_type;
    qcarcam_param_value_t q_param_value;
    unsigned int *gamma_table = NULL;

    // Convert input into qcarcam type
    Error = get_qcarcam_params(Param, StreamData, &q_param_type, &q_param_value, &gamma_table);
    if (QcarcamError::QCARCAM_OK == Error) {
        // Call qcarcam API
        ret = qcarcam_g_param(mQcarcamHndl, q_param_type, &q_param_value);
        if (ret != QCARCAM_RET_OK) {
            AIS_HIDL_ERRORMSG("qcarcam_g_param failed with error %d for param %d", ret, (int)q_param_type);
            Error = QcarcamError::QCARCAM_FAILED;
        } else {
            // Copy the results to hidl type for retuning
            Error = get_ais_hidl_params(q_param_type, &q_param_value, Param, StreamData, gamma_table);
        }
    }

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    _hidl_cb(Error, StreamData);

    // HIDL convention says we return Void if we sent our result back via callback
    return Void();
}

Return<void> ais_hidl_stream::getStreamConfig_1_1(vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam Param, const vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamConfigData& data, getStreamConfig_1_1_cb _hidl_cb)  {
    AIS_HIDL_INFOMSG("Entry getStreamConfig");

    QcarcamError Error = QcarcamError::QCARCAM_OK;
    vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamConfigData StreamData = data;
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    qcarcam_param_t q_param_type;
    qcarcam_param_value_t q_param_value;
    unsigned int *gamma_table = NULL;

    // Convert input into qcarcam type
    Error = get_qcarcam_params_1_1(Param, StreamData, &q_param_type, &q_param_value, &gamma_table);
    if (QcarcamError::QCARCAM_OK == Error) {
        // Call qcarcam API
        ret = qcarcam_g_param(mQcarcamHndl, q_param_type, &q_param_value);
        if (ret != QCARCAM_RET_OK) {
            AIS_HIDL_ERRORMSG("qcarcam_g_param failed with error %d for param %d", ret, (int)q_param_type);
            Error =  (QcarcamError)ret;
        } else {
            // Copy the results to hidl type for retuning
            Error = get_ais_hidl_params_1_1(q_param_type, &q_param_value, Param, StreamData, gamma_table);
        }
    }

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    _hidl_cb(Error, StreamData);

    // HIDL convention says we return Void if we sent our result back via callback
    return Void();
}

// Utility functions
QcarcamError ais_hidl_stream::get_qcarcam_params(QcarcamStreamParam Param,
        const QcarcamStreamConfigData& data,
        qcarcam_param_t* param_type,
        qcarcam_param_value_t* param_value,
        unsigned int **gamma_table)
{
    QcarcamError Error = QcarcamError::QCARCAM_OK;
    switch (Param)
    {
        case QcarcamStreamParam::QCARCAM_CB_EVENT_MASK:
            {
                *param_type = QCARCAM_PARAM_EVENT_MASK;
                if (param_value)
                    param_value->uint_value = static_cast<unsigned int>(data.uint_value);
            }
            break;
        case QcarcamStreamParam::QCARCAM_COLOR_FMT:
            {
                *param_type = QCARCAM_PARAM_COLOR_FMT;
                if (param_value)
                    param_value->color_value = static_cast<qcarcam_color_fmt_t>(data.color_value);
            }
            break;
        case QcarcamStreamParam::QCARCAM_RESOLUTION:
            {
                *param_type = QCARCAM_PARAM_RESOLUTION;
                if (param_value) {
                    param_value->res_value.width = static_cast<unsigned int>(data.res_value.width);
                    param_value->res_value.height = static_cast<unsigned int>(data.res_value.height);
                    param_value->res_value.fps = static_cast<float>(data.res_value.fps);
                }
            }
            break;
        case QcarcamStreamParam::QCARCAM_BRIGHTNESS:
            {
                *param_type = QCARCAM_PARAM_BRIGHTNESS;
            }
            break;
        case QcarcamStreamParam::QCARCAM_CONTRAST:
            {
                *param_type = QCARCAM_PARAM_CONTRAST;
            }
            break;
        case QcarcamStreamParam::QCARCAM_MIRROR_H:
            {
                *param_type = QCARCAM_PARAM_MIRROR_H;
            }
            break;
        case QcarcamStreamParam::QCARCAM_MIRROR_V:
            {
                *param_type = QCARCAM_PARAM_MIRROR_V;
            }
            break;
        case QcarcamStreamParam::QCARCAM_FRAME_RATE:
            {
                *param_type = QCARCAM_PARAM_FRAME_RATE;
                if (param_value) {
                    param_value->frame_rate_config.frame_drop_mode = static_cast<qcarcam_frame_drop_mode_t>(data.frame_rate_config.frame_drop_mode);
                    param_value->frame_rate_config.frame_drop_period = static_cast<unsigned char>(data.frame_rate_config.frame_drop_period);
                    param_value->frame_rate_config.frame_drop_pattern = static_cast<unsigned int>(data.frame_rate_config.frame_drop_pattern);
                }
            }
            break;
        case QcarcamStreamParam::QCARCAM_VID_STD:
            {
                *param_type = QCARCAM_PARAM_VID_STD;
            }
            break;
        case QcarcamStreamParam::QCARCAM_CURRENT_VID_STD:
            {
                *param_type = QCARCAM_PARAM_CURRENT_VID_STD;
            }
            break;
        case QcarcamStreamParam::QCARCAM_STATUS:
            {
                *param_type = QCARCAM_PARAM_STATUS;
            }
            break;
        case QcarcamStreamParam::QCARCAM_LATENCY_MAX:
            {
                *param_type = QCARCAM_PARAM_LATENCY_MAX;
            }
            break;
        case QcarcamStreamParam::QCARCAM_LATENCY_REDUCE_RATE:
            {
                *param_type = QCARCAM_PARAM_LATENCY_REDUCE_RATE;
            }
            break;
        case QcarcamStreamParam::QCARCAM_PRIVATE_DATA:
            {
                *param_type = QCARCAM_PARAM_PRIVATE_DATA;
                if (param_value)
                    param_value->ptr_value = (void *)data.ptr_value;
            }
            break;
        case QcarcamStreamParam::QCARCAM_INJECTION_START:
            {
                *param_type = QCARCAM_PARAM_INJECTION_START;
                if (param_value)
                    param_value->uint_value = static_cast<unsigned int>(data.uint_value);
            }
            break;
        case QcarcamStreamParam::QCARCAM_EXPOSURE:
            {
                *param_type = QCARCAM_PARAM_EXPOSURE;
                if (param_value) {
                    param_value->exposure_config.exposure_mode_type = static_cast<qcarcam_exposure_mode_t>(data.exposure_config.exposure_mode_type);
                    param_value->exposure_config.exposure_time = static_cast<float>(data.exposure_config.exposure_time);
                    param_value->exposure_config.gain = static_cast<float>(data.exposure_config.gain);
                    param_value->exposure_config.target = static_cast<float>(data.exposure_config.target);
                    param_value->exposure_config.lux_index = static_cast<float>(data.exposure_config.lux_index);
                }
            }
            break;
        case QcarcamStreamParam::QCARCAM_HDR_EXPOSURE:
            {
                *param_type = QCARCAM_PARAM_HDR_EXPOSURE;
                if (param_value) {
                    param_value->hdr_exposure_config.exposure_mode_type = static_cast<qcarcam_exposure_mode_t>(data.hdr_exposure_config.exposure_mode_type);
                    param_value->hdr_exposure_config.hdr_mode = static_cast<uint32_t>(data.hdr_exposure_config.hdr_mode);
                    param_value->hdr_exposure_config.target = static_cast<float>(data.hdr_exposure_config.target);
                    param_value->hdr_exposure_config.lux_index = static_cast<float>(data.hdr_exposure_config.lux_index);
                    param_value->hdr_exposure_config.num_exposures = static_cast<uint32_t>(data.hdr_exposure_config.num_exposures);
                    for (uint32_t i=0; i<param_value->hdr_exposure_config.num_exposures; i++) {
                        param_value->hdr_exposure_config.exposure_time[i] = static_cast<float>(data.hdr_exposure_config.exposure_time[i]);
                        param_value->hdr_exposure_config.exposure_ratio[i] = static_cast<float>(data.hdr_exposure_config.exposure_ratio[i]);
                        param_value->hdr_exposure_config.gain[i] = static_cast<float>(data.hdr_exposure_config.gain[i]);
                    }
                }
            }
            break;
        case QcarcamStreamParam::QCARCAM_HUE:
            {
                *param_type = QCARCAM_PARAM_HUE;
                if (param_value)
                    param_value->float_value = static_cast<float>(data.float_value);
            }
            break;
        case QcarcamStreamParam::QCARCAM_SATURATION:
            {
                *param_type = QCARCAM_PARAM_SATURATION;
                if (param_value)
                    param_value->float_value = static_cast<float>(data.float_value);
            }
            break;
        case QcarcamStreamParam::QCARCAM_GAMMA:
            {
                *param_type = QCARCAM_PARAM_GAMMA;
                if (param_value) {
                    if (data.gamma_config.config_type == static_cast<QcarcamGammaType>(QCARCAM_GAMMA_EXPONENT)) {
                        param_value->gamma_config.gamma.f_value = static_cast<float>(data.gamma_config.gamma.f_value);
                    }
                    else if (data.gamma_config.config_type == static_cast<QcarcamGammaType>(QCARCAM_GAMMA_KNEEPOINTS)) {
                        param_value->gamma_config.gamma.table.length = static_cast<uint32_t>(data.gamma_config.gamma.table.length);
                        if (param_value->gamma_config.gamma.table.length <= QCARCAM_MAX_GAMMA_TABLE) {
                            if (gamma_table == NULL) {
                                /* gamma_table eq NULL then call is for SET gamma user defined params */
                                memcpy(param_value->gamma_config.gamma.table.p_value, &data.gamma_config.gamma.table.arr_gamma, param_value->gamma_config.gamma.table.length * sizeof(unsigned int));
                            }
                            else {
                                unsigned int *gamma_values = (unsigned int *)calloc(param_value->gamma_config.gamma.table.length, sizeof(unsigned int));

                                if (NULL == gamma_values) {
                                    AIS_HIDL_ERRORMSG("Failed to allocate table of size %d", param_value->gamma_config.gamma.table.length);
                                    Error = QcarcamError::QCARCAM_FAILED;
                                }
                                param_value->gamma_config.gamma.table.p_value = gamma_values;
                                *gamma_table = gamma_values;
                            }
                        }
                    }
                    param_value->gamma_config.config_type = static_cast<qcarcam_gamma_type_t>(data.gamma_config.config_type);
                }
            }
            break;
        case QcarcamStreamParam::QCARCAM_OPMODE:
            {
                *param_type = QCARCAM_PARAM_OPMODE;
                if (param_value)
                    param_value->uint_value = static_cast<unsigned int>(data.uint_value);
            }
            break;
        case QcarcamStreamParam::QCARCAM_RECOVERY:
            {
                *param_type = QCARCAM_PARAM_RECOVERY;
                if (param_value)
                    param_value->uint_value = static_cast<unsigned int>(data.uint_value);
            }
            break;
        case QcarcamStreamParam::QCARCAM_ISP_CTRLS:
            {
                *param_type = QCARCAM_PARAM_ISP_CTRLS;
                if (param_value)
                {
                    param_value->isp_ctrls.param_mask = data.isp_ctrls.param_mask;
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_AE_MODE))
                        param_value->isp_ctrls.ae_mode = static_cast<qcarcam_ctrl_ae_mode_t>(data.isp_ctrls.ae_mode);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_AE_LOCK))
                        param_value->isp_ctrls.ae_lock = static_cast<qcarcam_ctrl_ae_lock_t>(data.isp_ctrls.ae_lock);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_AWB_MODE))
                        param_value->isp_ctrls.awb_mode = static_cast<qcarcam_ctrl_awb_mode_t>(data.isp_ctrls.awb_mode);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_AWB_LOCK))
                        param_value->isp_ctrls.awb_lock = static_cast<qcarcam_ctrl_awb_lock_t>(data.isp_ctrls.awb_lock);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_EFFECT_MODE))
                        param_value->isp_ctrls.effect_mode = static_cast<qcarcam_ctrl_control_effect_mode_t>(data.isp_ctrls.effect_mode);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_MODE))
                        param_value->isp_ctrls.ctrl_mode = static_cast<qcarcam_ctrl_control_mode_t>(data.isp_ctrls.ctrl_mode);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_SCENE_MODE))
                        param_value->isp_ctrls.scene_mode = static_cast<qcarcam_ctrl_control_scene_mode_t>(data.isp_ctrls.scene_mode);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_AE_ANTIBANDING_MODE))
                        param_value->isp_ctrls.ae_antibanding_mode = static_cast<qcarcam_ctrl_ae_antibanding_mode_t>(data.isp_ctrls.ae_antibanding_mode);
                }
            }
            break;
        default:
            AIS_HIDL_ERRORMSG("Invalid config param %d", static_cast<int>(Param));
            Error = QcarcamError::QCARCAM_FAILED;
            break;
    }
    return Error;
}

QcarcamError ais_hidl_stream::get_ais_hidl_params(qcarcam_param_t param_type, qcarcam_param_value_t* param_value,
        QcarcamStreamParam& Param, QcarcamStreamConfigData& data, unsigned int *gamma_table)
{
    QcarcamError Error = QcarcamError::QCARCAM_OK;
    switch (param_type)
    {
        case QCARCAM_PARAM_EVENT_MASK:
            {
                Param = QcarcamStreamParam::QCARCAM_CB_EVENT_MASK;
                if (param_value)
                    data.uint_value = static_cast<uint32_t>(param_value->uint_value);
            }
            break;
        case QCARCAM_PARAM_COLOR_FMT:
            {
                Param = QcarcamStreamParam::QCARCAM_COLOR_FMT;
                if (param_value)
                    data.color_value = static_cast<QcarcamColorFmt>(param_value->color_value);
            }
            break;
        case QCARCAM_PARAM_RESOLUTION:
            {
                Param = QcarcamStreamParam::QCARCAM_RESOLUTION;
                if (param_value) {
                    data.res_value.width = static_cast<uint32_t>(param_value->res_value.width);
                    data.res_value.height = static_cast<uint32_t>(param_value->res_value.height);
                    data.res_value.fps = static_cast<float>(param_value->res_value.fps);
                }
            }
            break;
        case QCARCAM_PARAM_BRIGHTNESS:
            {
                Param = QcarcamStreamParam::QCARCAM_BRIGHTNESS;
            }
            break;
        case QCARCAM_PARAM_CONTRAST:
            {
                Param = QcarcamStreamParam::QCARCAM_CONTRAST;
            }
            break;
        case QCARCAM_PARAM_MIRROR_H:
            {
                Param = QcarcamStreamParam::QCARCAM_MIRROR_H;
            }
            break;
        case QCARCAM_PARAM_MIRROR_V:
            {
                Param = QcarcamStreamParam::QCARCAM_MIRROR_V;
            }
            break;
        case QCARCAM_PARAM_FRAME_RATE:
            {
                Param = QcarcamStreamParam::QCARCAM_FRAME_RATE;
                if (param_value) {
                    data.frame_rate_config.frame_drop_mode = static_cast<QcarcamFrameDropMode>(param_value->frame_rate_config.frame_drop_mode);
                    data.frame_rate_config.frame_drop_period = static_cast<uint16_t>(param_value->frame_rate_config.frame_drop_period);
                    data.frame_rate_config.frame_drop_pattern = static_cast<uint32_t>(param_value->frame_rate_config.frame_drop_pattern);
                }
            }
            break;
        case QCARCAM_PARAM_VID_STD:
            {
                Param = QcarcamStreamParam::QCARCAM_VID_STD;
            }
            break;
        case QCARCAM_PARAM_CURRENT_VID_STD:
            {
                Param = QcarcamStreamParam::QCARCAM_CURRENT_VID_STD;
            }
            break;
        case QCARCAM_PARAM_STATUS:
            {
                Param = QcarcamStreamParam::QCARCAM_STATUS;
            }
            break;
        case QCARCAM_PARAM_LATENCY_MAX:
            {
                Param = QcarcamStreamParam::QCARCAM_LATENCY_MAX;
            }
            break;
        case QCARCAM_PARAM_LATENCY_REDUCE_RATE:
            {
                Param = QcarcamStreamParam::QCARCAM_LATENCY_REDUCE_RATE;
            }
            break;
        case QCARCAM_PARAM_PRIVATE_DATA:
            {
                Param = QcarcamStreamParam::QCARCAM_PRIVATE_DATA;
                if (param_value)
                    data.ptr_value = (uint64_t)param_value->ptr_value;
            }
            break;
        case QCARCAM_PARAM_INJECTION_START:
            {
                Param = QcarcamStreamParam::QCARCAM_INJECTION_START;
                if (param_value)
                    data.uint_value = static_cast<uint32_t>(param_value->uint_value);
            }
            break;
        case QCARCAM_PARAM_EXPOSURE:
            {
                Param = QcarcamStreamParam::QCARCAM_EXPOSURE;
                if (param_value) {
                    data.exposure_config.exposure_mode_type = static_cast<QcarcamExposureMode>(param_value->exposure_config.exposure_mode_type);
                    data.exposure_config.exposure_time = static_cast<float>(param_value->exposure_config.exposure_time);
                    data.exposure_config.gain = static_cast<float>(param_value->exposure_config.gain);
                    data.exposure_config.target = static_cast<float>(param_value->exposure_config.target);
                    data.exposure_config.lux_index = static_cast<float>(param_value->exposure_config.lux_index);
                }
            }
            break;
        case QCARCAM_PARAM_HDR_EXPOSURE:
            {
                Param = QcarcamStreamParam::QCARCAM_HDR_EXPOSURE;
                if (param_value) {
                    data.hdr_exposure_config.exposure_mode_type = static_cast<QcarcamExposureMode>(param_value->hdr_exposure_config.exposure_mode_type);
                    data.hdr_exposure_config.hdr_mode = static_cast<uint32_t>(param_value->hdr_exposure_config.hdr_mode);
                    data.hdr_exposure_config.target = static_cast<float>(param_value->hdr_exposure_config.target);
                    data.hdr_exposure_config.lux_index = static_cast<float>(param_value->hdr_exposure_config.lux_index);
                    data.hdr_exposure_config.num_exposures = static_cast<uint32_t>(param_value->hdr_exposure_config.num_exposures);
                    for (uint32_t i=0; i<data.hdr_exposure_config.num_exposures; i++) {
                        data.hdr_exposure_config.exposure_time[i] = static_cast<float>(param_value->hdr_exposure_config.exposure_time[i]);
                        data.hdr_exposure_config.exposure_ratio[i] = static_cast<float>(param_value->hdr_exposure_config.exposure_ratio[i]);
                        data.hdr_exposure_config.gain[i] = static_cast<float>(param_value->hdr_exposure_config.gain[i]);
                    }
                }
            }
            break;
        case QCARCAM_PARAM_HUE:
            {
                Param = QcarcamStreamParam::QCARCAM_HUE;
                if (param_value)
                    data.float_value = static_cast<float>(param_value->float_value);
            }
            break;
        case QCARCAM_PARAM_SATURATION:
            {
                Param = QcarcamStreamParam::QCARCAM_SATURATION;
                if (param_value)
                    data.float_value = static_cast<float>(param_value->float_value);
            }
            break;
        case QCARCAM_PARAM_GAMMA:
            {
                Param = QcarcamStreamParam::QCARCAM_GAMMA;
                if (param_value) {
                    if (param_value->gamma_config.config_type == QCARCAM_GAMMA_EXPONENT) {
                        data.gamma_config.gamma.f_value = static_cast<float>(param_value->gamma_config.gamma.f_value);
                    }
                    else if (param_value->gamma_config.config_type == QCARCAM_GAMMA_KNEEPOINTS) {
                        data.gamma_config.gamma.table.length = static_cast<uint32_t>(param_value->gamma_config.gamma.table.length);
                        if (param_value->gamma_config.gamma.table.length <= QCARCAM_MAX_GAMMA_TABLE) {
                            if (gamma_table != NULL) {
                                memcpy(&data.gamma_config.gamma.table.arr_gamma, gamma_table, param_value->gamma_config.gamma.table.length * sizeof(unsigned int));
                                free(gamma_table);
                            }
                        }
                    }
                    data.gamma_config.config_type = static_cast<QcarcamGammaType>(param_value->gamma_config.config_type);
                }

            }
            break;
        case QCARCAM_PARAM_ISP_CTRLS:
            {
                Param = vendor::qti::automotive::qcarcam::V1_0::QcarcamStreamParam::QCARCAM_ISP_CTRLS;
                if (param_value)
                {
                    data.isp_ctrls.param_mask = param_value->isp_ctrls.param_mask;
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_AE_MODE))
                        data.isp_ctrls.ae_mode = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamCtrlAEMode>(param_value->isp_ctrls.ae_mode);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_AE_LOCK))
                        data.isp_ctrls.ae_lock = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamCtrlAELock>(param_value->isp_ctrls.ae_lock);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_AWB_MODE))
                        data.isp_ctrls.awb_mode = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamCtrlAWBMode>(param_value->isp_ctrls.awb_mode);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_AWB_LOCK))
                        data.isp_ctrls.awb_lock = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamCtrlAWBLock>(param_value->isp_ctrls.awb_lock);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_EFFECT_MODE))
                        data.isp_ctrls.effect_mode = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamCtrlControlEffectMode>(param_value->isp_ctrls.effect_mode);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_MODE))
                        data.isp_ctrls.ctrl_mode = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamCtrlControlMode>(param_value->isp_ctrls.ctrl_mode);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_SCENE_MODE))
                        data.isp_ctrls.scene_mode = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamCtrlControlSceneMode>(param_value->isp_ctrls.scene_mode);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_AE_ANTIBANDING_MODE))
                        data.isp_ctrls.ae_antibanding_mode = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamCtrlAEAntibandingMode>(param_value->isp_ctrls.ae_antibanding_mode);
                }
            }
            break;
        default: AIS_HIDL_ERRORMSG("Invalid config param %d",static_cast<int>(param_type));
                 Error = QcarcamError::QCARCAM_FAILED;
    }
    return Error;
}

QcarcamError ais_hidl_stream::get_qcarcam_params_1_1(vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam Param,
        const vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamConfigData& data,
        qcarcam_param_t* param_type,
        qcarcam_param_value_t* param_value,
        unsigned int **gamma_table)
{
    QcarcamError Error = QcarcamError::QCARCAM_OK;
    switch (Param)
    {
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_CB_EVENT_MASK:
            {
                *param_type = QCARCAM_PARAM_EVENT_MASK;
                if (param_value)
                    param_value->uint_value = static_cast<unsigned int>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_COLOR_FMT:
            {
                *param_type = QCARCAM_PARAM_COLOR_FMT;
                if (param_value)
                    param_value->color_value = static_cast<qcarcam_color_fmt_t>(data.color_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_RESOLUTION:
            {
                *param_type = QCARCAM_PARAM_RESOLUTION;
                if (param_value) {
                    param_value->res_value.width = static_cast<unsigned int>(data.res_value.width);
                    param_value->res_value.height = static_cast<unsigned int>(data.res_value.height);
                    param_value->res_value.fps = static_cast<float>(data.res_value.fps);
                }
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_BRIGHTNESS:
            {
                *param_type = QCARCAM_PARAM_BRIGHTNESS;
                 if(param_value)
                 param_value->float_value = static_cast<float>(data.float_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_VENDOR:
            {
                *param_type = QCARCAM_PARAM_VENDOR;
                if(param_value) {
                   for(unsigned int i=0; i < QCARCAM_MAX_VENDOR_PAYLOAD_SIZE; i++){
                     param_value->vendor_param.data[i] = static_cast<uint32_t>(data.vendor_param.data[i]);
                  }
                }
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_INPUT_MODE:
            {
                *param_type = QCARCAM_PARAM_INPUT_MODE;
                if(param_value)
                 param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_MASTER:
            {
                *param_type = QCARCAM_PARAM_MASTER;
                if(param_value)
                 param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_EVENT_CHANGE_SUBSCRIBE:
            {
                *param_type = QCARCAM_PARAM_EVENT_CHANGE_SUBSCRIBE;
                if(param_value)
                 param_value->uint64_value =  static_cast<uint64_t>(data.uint64_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_EVENT_CHANGE_UNSUBSCRIBE:
            {
                *param_type = QCARCAM_PARAM_EVENT_CHANGE_UNSUBSCRIBE;
                if(param_value)
                 param_value->uint64_value =  static_cast<uint64_t>(data.uint64_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_BATCH_MODE:
            {
                *param_type = QCARCAM_PARAM_BATCH_MODE;
                if(param_value)
                param_value->batch_config.num_batch_frames = static_cast<uint32_t>(data.batch_config.num_batch_frames);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_ISP_USECASE:
            {
                *param_type = QCARCAM_PARAM_ISP_USECASE;
                if(param_value) {
                   param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
                   param_value->isp_config.id = data.isp_config.id;
                   param_value->isp_config.camera_id = data.isp_config.camera_id;
                   param_value->isp_config.use_case = static_cast<qcarcam_isp_usecase_t>(data.isp_config.use_case);
                }
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_CONTRAST:
            {
                *param_type = QCARCAM_PARAM_CONTRAST;
                if(param_value)
                 param_value->float_value = static_cast<float>(data.float_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_MIRROR_H:
            {
                *param_type = QCARCAM_PARAM_MIRROR_H;
                if(param_value)
                param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_MIRROR_V:
            {
                *param_type = QCARCAM_PARAM_MIRROR_V;
                if(param_value)
                param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_FRAME_RATE:
            {
                *param_type = QCARCAM_PARAM_FRAME_RATE;
                if (param_value) {
                    param_value->frame_rate_config.frame_drop_mode = static_cast<qcarcam_frame_drop_mode_t>(data.frame_rate_config.frame_drop_mode);
                    param_value->frame_rate_config.frame_drop_period = static_cast<unsigned char>(data.frame_rate_config.frame_drop_period);
                    param_value->frame_rate_config.frame_drop_pattern = static_cast<unsigned int>(data.frame_rate_config.frame_drop_pattern);
                }
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_VID_STD:
            {
                *param_type = QCARCAM_PARAM_VID_STD;
                if(param_value)
                param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_CURRENT_VID_STD:
            {
                *param_type = QCARCAM_PARAM_CURRENT_VID_STD;
                if(param_value)
                param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_STATUS:
            {
                *param_type = QCARCAM_PARAM_STATUS;
                if(param_value)
                param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_LATENCY_MAX:
            {
                *param_type = QCARCAM_PARAM_LATENCY_MAX;
                if(param_value)
                param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_LATENCY_REDUCE_RATE:
            {
                *param_type = QCARCAM_PARAM_LATENCY_REDUCE_RATE;
                if(param_value)
                param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_PRIVATE_DATA:
            {
                *param_type = QCARCAM_PARAM_PRIVATE_DATA;
                if (param_value)
                    param_value->ptr_value = (void *)data.ptr_value;
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_INJECTION_START:
            {
                *param_type = QCARCAM_PARAM_INJECTION_START;
                if (param_value)
                    param_value->uint_value = static_cast<unsigned int>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_EXPOSURE:
            {
                *param_type = QCARCAM_PARAM_EXPOSURE;
                if (param_value) {
                    param_value->exposure_config.exposure_mode_type = static_cast<qcarcam_exposure_mode_t>(data.exposure_config.exposure_mode_type);
                    param_value->exposure_config.exposure_time = static_cast<float>(data.exposure_config.exposure_time);
                    param_value->exposure_config.gain = static_cast<float>(data.exposure_config.gain);
                    param_value->exposure_config.target = static_cast<float>(data.exposure_config.target);
                    param_value->exposure_config.lux_index = static_cast<float>(data.exposure_config.lux_index);
                }
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_HDR_EXPOSURE:
            {
                *param_type = QCARCAM_PARAM_HDR_EXPOSURE;
                if (param_value) {
                    param_value->hdr_exposure_config.exposure_mode_type = static_cast<qcarcam_exposure_mode_t>(data.hdr_exposure_config.exposure_mode_type);
                    param_value->hdr_exposure_config.hdr_mode = static_cast<uint32_t>(data.hdr_exposure_config.hdr_mode);
                    param_value->hdr_exposure_config.target = static_cast<float>(data.hdr_exposure_config.target);
                    param_value->hdr_exposure_config.lux_index = static_cast<float>(data.hdr_exposure_config.lux_index);
                    param_value->hdr_exposure_config.num_exposures = static_cast<uint32_t>(data.hdr_exposure_config.num_exposures);
                    for (uint32_t i=0; i<param_value->hdr_exposure_config.num_exposures; i++) {
                        param_value->hdr_exposure_config.exposure_time[i] = static_cast<float>(data.hdr_exposure_config.exposure_time[i]);
                        param_value->hdr_exposure_config.exposure_ratio[i] = static_cast<float>(data.hdr_exposure_config.exposure_ratio[i]);
                        param_value->hdr_exposure_config.gain[i] = static_cast<float>(data.hdr_exposure_config.gain[i]);
                    }
                }
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_HUE:
            {
                *param_type = QCARCAM_PARAM_HUE;
                if (param_value)
                    param_value->float_value = static_cast<float>(data.float_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_SATURATION:
            {
                *param_type = QCARCAM_PARAM_SATURATION;
                if (param_value)
                    param_value->float_value = static_cast<float>(data.float_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_GAMMA:
            {
                *param_type = QCARCAM_PARAM_GAMMA;
                if (param_value) {
                    if (data.gamma_config.config_type == static_cast<QcarcamGammaType>(QCARCAM_GAMMA_EXPONENT)) {
                        param_value->gamma_config.gamma.f_value = static_cast<float>(data.gamma_config.gamma.f_value);
                    }
                    else if (data.gamma_config.config_type == static_cast<QcarcamGammaType>(QCARCAM_GAMMA_KNEEPOINTS)) {
                        param_value->gamma_config.gamma.table.length = static_cast<uint32_t>(data.gamma_config.gamma.table.length);
                        if (param_value->gamma_config.gamma.table.length <= QCARCAM_MAX_GAMMA_TABLE) {
                            if (gamma_table == NULL) {
                                /* gamma_table eq NULL then call is for SET gamma user defined params */
                                memcpy(param_value->gamma_config.gamma.table.p_value, &data.gamma_config.gamma.table.arr_gamma, param_value->gamma_config.gamma.table.length * sizeof(unsigned int));
                            }
                            else {
                                unsigned int *gamma_values = (unsigned int *)calloc(param_value->gamma_config.gamma.table.length, sizeof(unsigned int));

                                if (NULL == gamma_values) {
                                    AIS_HIDL_ERRORMSG("Failed to allocate table of size %d", param_value->gamma_config.gamma.table.length);
                                    Error = QcarcamError::QCARCAM_FAILED;
                                }
                                param_value->gamma_config.gamma.table.p_value = gamma_values;
                                *gamma_table = gamma_values;
                            }
                        }
                    }
                    param_value->gamma_config.config_type = static_cast<qcarcam_gamma_type_t>(data.gamma_config.config_type);
                }
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_OPMODE:
            {
                *param_type = QCARCAM_PARAM_OPMODE;
                if (param_value)
                    param_value->uint_value = static_cast<unsigned int>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_RECOVERY:
            {
                *param_type = QCARCAM_PARAM_RECOVERY;
                if (param_value)
                    param_value->uint_value = static_cast<unsigned int>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_ISP_CTRLS:
            {
                *param_type = QCARCAM_PARAM_ISP_CTRLS;
                if (param_value)
                {
                    param_value->isp_ctrls.param_mask = data.isp_ctrls.param_mask;
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_AE_MODE))
                        param_value->isp_ctrls.ae_mode = static_cast<qcarcam_ctrl_ae_mode_t>(data.isp_ctrls.ae_mode);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_AE_LOCK))
                        param_value->isp_ctrls.ae_lock = static_cast<qcarcam_ctrl_ae_lock_t>(data.isp_ctrls.ae_lock);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_AWB_MODE))
                        param_value->isp_ctrls.awb_mode = static_cast<qcarcam_ctrl_awb_mode_t>(data.isp_ctrls.awb_mode);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_AWB_LOCK))
                        param_value->isp_ctrls.awb_lock = static_cast<qcarcam_ctrl_awb_lock_t>(data.isp_ctrls.awb_lock);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_EFFECT_MODE))
                        param_value->isp_ctrls.effect_mode = static_cast<qcarcam_ctrl_control_effect_mode_t>(data.isp_ctrls.effect_mode);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_MODE))
                        param_value->isp_ctrls.ctrl_mode = static_cast<qcarcam_ctrl_control_mode_t>(data.isp_ctrls.ctrl_mode);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_SCENE_MODE))
                        param_value->isp_ctrls.scene_mode = static_cast<qcarcam_ctrl_control_scene_mode_t>(data.isp_ctrls.scene_mode);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_AE_ANTIBANDING_MODE))
                        param_value->isp_ctrls.ae_antibanding_mode = static_cast<qcarcam_ctrl_ae_antibanding_mode_t>(data.isp_ctrls.ae_antibanding_mode);
                }
            }
            break;
        default:
            AIS_HIDL_ERRORMSG("Invalid config param %d", static_cast<int>(Param));
            Error = QcarcamError::QCARCAM_FAILED;
            break;
    }
    return Error;
}

QcarcamError ais_hidl_stream::get_ais_hidl_params_1_1(qcarcam_param_t param_type, qcarcam_param_value_t* param_value,
        vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam& Param, vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamConfigData& data, unsigned int *gamma_table)
{
    QcarcamError Error = QcarcamError::QCARCAM_OK;
    switch (param_type)
    {
        case QCARCAM_PARAM_EVENT_MASK:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_CB_EVENT_MASK;
                if (param_value)
                    data.uint_value = static_cast<uint32_t>(param_value->uint_value);
            }
            break;
        case QCARCAM_PARAM_COLOR_FMT:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_COLOR_FMT;
                if (param_value)
                    data.color_value = static_cast<vendor::qti::automotive::qcarcam::V1_1::QcarcamColorFmt>(param_value->color_value);
            }
            break;
        case QCARCAM_PARAM_RESOLUTION:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_RESOLUTION;
                if (param_value) {
                    data.res_value.width = static_cast<uint32_t>(param_value->res_value.width);
                    data.res_value.height = static_cast<uint32_t>(param_value->res_value.height);
                    data.res_value.fps = static_cast<float>(param_value->res_value.fps);
                }
            }
            break;
        case QCARCAM_PARAM_BRIGHTNESS:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_BRIGHTNESS;
                if(param_value){
                    data.float_value = static_cast<float>(param_value->float_value);
                }
            }
            break;
                case QCARCAM_PARAM_VENDOR:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_VENDOR;
                if(param_value){
                     for(unsigned int i=0; i < QCARCAM_MAX_VENDOR_PAYLOAD_SIZE; i++){
                     data.vendor_param.data[i] = static_cast<uint32_t>(param_value->vendor_param.data[i]);
                  }
                }
            }
            break;
        case QCARCAM_PARAM_INPUT_MODE:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_INPUT_MODE;
                 if(param_value){
                    data.uint_value = static_cast<uint32_t>(param_value->uint_value);
                }
            }
            break;
        case QCARCAM_PARAM_MASTER:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_MASTER;
                 if(param_value){
                    data.uint_value = static_cast<uint32_t>(param_value->uint_value);
                }
            }
            break;
        case QCARCAM_PARAM_EVENT_CHANGE_SUBSCRIBE:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_EVENT_CHANGE_SUBSCRIBE;
                if(param_value){
                    data.uint64_value = static_cast<uint64_t>(param_value->uint64_value);
                }
            }
            break;
        case QCARCAM_PARAM_EVENT_CHANGE_UNSUBSCRIBE:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_EVENT_CHANGE_UNSUBSCRIBE;
                if(param_value){
                    data.uint64_value = static_cast<uint64_t>(param_value->uint64_value);
                }
            }
            break;
        case QCARCAM_PARAM_BATCH_MODE:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_BATCH_MODE;
                if(param_value){
                  data.batch_config.num_batch_frames = static_cast<uint32_t>(param_value->batch_config.num_batch_frames);
                }
            }
            break;
        case QCARCAM_PARAM_ISP_USECASE:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_ISP_USECASE;
                if(param_value){
                    data.uint_value = static_cast<uint32_t>(param_value->uint_value);
                    data.isp_config.id = param_value->isp_config.id;
                    data.isp_config.camera_id = param_value->isp_config.camera_id;
                    data.isp_config.use_case = static_cast<vendor::qti::automotive::qcarcam::V1_1::QcarcamIspUsecase>(param_value->isp_config.use_case);
                }
            }
            break;
        case QCARCAM_PARAM_CONTRAST:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_CONTRAST;
                 if(param_value){
                    data.float_value = static_cast<float>(param_value->float_value);
                }
            }
            break;
        case QCARCAM_PARAM_MIRROR_H:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_MIRROR_H;
                if(param_value){
                    data.uint_value = static_cast<uint32_t>(param_value->uint_value);
                }
            }
            break;
        case QCARCAM_PARAM_MIRROR_V:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_MIRROR_V;
                if(param_value){
                    data.uint_value = static_cast<uint32_t>(param_value->uint_value);
                }
            }
            break;
        case QCARCAM_PARAM_FRAME_RATE:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_FRAME_RATE;
                if (param_value) {
                    data.frame_rate_config.frame_drop_mode = static_cast<QcarcamFrameDropMode>(param_value->frame_rate_config.frame_drop_mode);
                    data.frame_rate_config.frame_drop_period = static_cast<uint16_t>(param_value->frame_rate_config.frame_drop_period);
                    data.frame_rate_config.frame_drop_pattern = static_cast<uint32_t>(param_value->frame_rate_config.frame_drop_pattern);
                }
            }
            break;
        case QCARCAM_PARAM_VID_STD:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_VID_STD;
                 if(param_value){
                    data.uint_value = static_cast<uint32_t>(param_value->uint_value);
                }
            }
            break;
        case QCARCAM_PARAM_CURRENT_VID_STD:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_CURRENT_VID_STD;
                 if(param_value){
                    data.uint_value = static_cast<uint32_t>(param_value->uint_value);
                }
            }
            break;
        case QCARCAM_PARAM_STATUS:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_STATUS;
                 if(param_value){
                    data.uint_value = static_cast<uint32_t>(param_value->uint_value);
                }
            }
            break;
        case QCARCAM_PARAM_LATENCY_MAX:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_LATENCY_MAX;
                 if(param_value){
                    data.uint_value = static_cast<uint32_t>(param_value->uint_value);
                }
            }
            break;
        case QCARCAM_PARAM_LATENCY_REDUCE_RATE:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_LATENCY_REDUCE_RATE;
                 if(param_value){
                    data.uint_value = static_cast<uint32_t>(param_value->uint_value);
                }
            }
            break;
        case QCARCAM_PARAM_OPMODE:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_OPMODE;
                 if(param_value){
                    data.uint_value = static_cast<uint32_t>(param_value->uint_value);
                }
            }
            break;
        case QCARCAM_PARAM_RECOVERY:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_RECOVERY;
                 if(param_value){
                    data.uint_value = static_cast<uint32_t>(param_value->uint_value);
                }
            }
            break;
        case QCARCAM_PARAM_PRIVATE_DATA:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_PRIVATE_DATA;
                if (param_value)
                    data.ptr_value = (uint64_t)param_value->ptr_value;
            }
            break;
        case QCARCAM_PARAM_INJECTION_START:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_INJECTION_START;
                if (param_value)
                    data.uint_value = static_cast<uint32_t>(param_value->uint_value);
            }
            break;
        case QCARCAM_PARAM_EXPOSURE:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_EXPOSURE;
                if (param_value) {
                    data.exposure_config.exposure_mode_type = static_cast<QcarcamExposureMode>(param_value->exposure_config.exposure_mode_type);
                    data.exposure_config.exposure_time = static_cast<float>(param_value->exposure_config.exposure_time);
                    data.exposure_config.gain = static_cast<float>(param_value->exposure_config.gain);
                    data.exposure_config.target = static_cast<float>(param_value->exposure_config.target);
                    data.exposure_config.lux_index = static_cast<float>(param_value->exposure_config.lux_index);
                }
            }
            break;
        case QCARCAM_PARAM_HDR_EXPOSURE:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_HDR_EXPOSURE;
                if (param_value) {
                    data.hdr_exposure_config.exposure_mode_type = static_cast<QcarcamExposureMode>(param_value->hdr_exposure_config.exposure_mode_type);
                    data.hdr_exposure_config.hdr_mode = static_cast<uint32_t>(param_value->hdr_exposure_config.hdr_mode);
                    data.hdr_exposure_config.target = static_cast<float>(param_value->hdr_exposure_config.target);
                    data.hdr_exposure_config.lux_index = static_cast<float>(param_value->hdr_exposure_config.lux_index);
                    data.hdr_exposure_config.num_exposures = static_cast<uint32_t>(param_value->hdr_exposure_config.num_exposures);
                    for (uint32_t i=0; i<data.hdr_exposure_config.num_exposures; i++) {
                        data.hdr_exposure_config.exposure_time[i] = static_cast<float>(param_value->hdr_exposure_config.exposure_time[i]);
                        data.hdr_exposure_config.exposure_ratio[i] = static_cast<float>(param_value->hdr_exposure_config.exposure_ratio[i]);
                        data.hdr_exposure_config.gain[i] = static_cast<float>(param_value->hdr_exposure_config.gain[i]);
                    }
                }
            }
            break;
        case QCARCAM_PARAM_HUE:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_HUE;
                if (param_value)
                    data.float_value = static_cast<float>(param_value->float_value);
            }
            break;
        case QCARCAM_PARAM_SATURATION:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_SATURATION;
                if (param_value)
                    data.float_value = static_cast<float>(param_value->float_value);
            }
            break;
        case QCARCAM_PARAM_GAMMA:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_GAMMA;
                if (param_value) {
                    if (param_value->gamma_config.config_type == QCARCAM_GAMMA_EXPONENT) {
                        data.gamma_config.gamma.f_value = static_cast<float>(param_value->gamma_config.gamma.f_value);
                    }
                    else if (param_value->gamma_config.config_type == QCARCAM_GAMMA_KNEEPOINTS) {
                        data.gamma_config.gamma.table.length = static_cast<uint32_t>(param_value->gamma_config.gamma.table.length);
                        if (param_value->gamma_config.gamma.table.length <= QCARCAM_MAX_GAMMA_TABLE) {
                            if (gamma_table != NULL) {
                                memcpy(&data.gamma_config.gamma.table.arr_gamma, gamma_table, param_value->gamma_config.gamma.table.length * sizeof(unsigned int));
                                free(gamma_table);
                            }
                        }
                    }
                    data.gamma_config.config_type = static_cast<QcarcamGammaType>(param_value->gamma_config.config_type);
                }

            }
            break;
        case QCARCAM_PARAM_ISP_CTRLS:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_ISP_CTRLS;
                if (param_value)
                {
                    data.isp_ctrls.param_mask = param_value->isp_ctrls.param_mask;
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_AE_MODE))
                        data.isp_ctrls.ae_mode = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamCtrlAEMode>(param_value->isp_ctrls.ae_mode);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_AE_LOCK))
                        data.isp_ctrls.ae_lock = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamCtrlAELock>(param_value->isp_ctrls.ae_lock);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_AWB_MODE))
                        data.isp_ctrls.awb_mode = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamCtrlAWBMode>(param_value->isp_ctrls.awb_mode);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_AWB_LOCK))
                        data.isp_ctrls.awb_lock = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamCtrlAWBLock>(param_value->isp_ctrls.awb_lock);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_EFFECT_MODE))
                        data.isp_ctrls.effect_mode = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamCtrlControlEffectMode>(param_value->isp_ctrls.effect_mode);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_MODE))
                        data.isp_ctrls.ctrl_mode = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamCtrlControlMode>(param_value->isp_ctrls.ctrl_mode);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_SCENE_MODE))
                        data.isp_ctrls.scene_mode = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamCtrlControlSceneMode>(param_value->isp_ctrls.scene_mode);
                    if (CHECK_BIT(data.isp_ctrls.param_mask, QCARCAM_CONTROL_AE_ANTIBANDING_MODE))
                        data.isp_ctrls.ae_antibanding_mode = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamCtrlAEAntibandingMode>(param_value->isp_ctrls.ae_antibanding_mode);
                }
            }
            break;
        default: AIS_HIDL_ERRORMSG("Invalid config param %d",static_cast<int>(param_type));
                 Error = QcarcamError::QCARCAM_FAILED;
    }
    return Error;
}

} //implementation
} //V1_0
} //qcarcam
} //automotive
} //qti
} //vendor

