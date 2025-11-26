/* ===========================================================================
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
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
namespace qcarcam {

// Global Stream Array from ais_aidl_camera.cpp
extern gQcarCamClientData gQcarCamClientArray[MAX_AIS_CLIENTS];
extern std::mutex gMultiStreamQLock;
extern std::condition_variable gClientNotifier;

// Event callback from qcarcam engine
void ais_aidl_stream::qcarcam_event_cb(qcarcam_hndl_t hndl, qcarcam_event_t event_id, qcarcam_event_payload_t *p_payload)
{
    AIS_AIDL_INFOMSG("qcarcam_event_cb called");

    // Find the current stream Q
    gQcarCamClientData *pRecord = findEventQByHndl(hndl);
    if (!pRecord) {
        AIS_AIDL_ERRORMSG("ERROR::Requested qcarcam_hndl %p not found", hndl);
        return;
    }
    AIS_AIDL_INFOMSG("Found qcarcam_hndl in the list");

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
        AIS_AIDL_INFOMSG("Pushed event id %d to Event Queue", (int)event_id);
    }
}

//Defalut constructors
ais_aidl_stream::ais_aidl_stream(QcarcamInputDesc Desc, gQcarCamClientData *pHead, qcarcam_hndl_t hndl)
{
    AIS_AIDL_ERRORMSG("ais_aidl_stream Constructor hndl: %p", hndl);
    mStreamObj.reset();
    mDesc = Desc;
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

//Defalut destructor
ais_aidl_stream::~ais_aidl_stream()
{
    qcarcam_ret_t ret;
    if (true == mStreaming) {
        AIS_AIDL_ERRORMSG("Its a abrupt exit, Calling qcarcam stop");
        ret = qcarcam_stop(mQcarcamHndl);
        if (ret != QCARCAM_RET_OK) {
            AIS_AIDL_ERRORMSG("qcarcam_stop failed with error %d for hndl %p", ret, mQcarcamHndl);
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
    ret = qcarcam_close(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        AIS_AIDL_ERRORMSG("qcarcam_close failed with error %d for hndl %p", ret, mQcarcamHndl);
    }

    // Close and delete native handle
    native_handle_close(cloneHandle);
    native_handle_delete(cloneHandle);
    cloneHandle = nullptr;

    // Find and delete mQcarcamHndl
    gMultiStreamQLock.lock();
    AIS_AIDL_ERRORMSG("Removing qcarcam_context = %p from Q",mQcarcamHndl);
    gQcarCamClientData *pRecord = ais_aidl_stream::findEventQByHndl(mQcarcamHndl);
    if (nullptr != pRecord) {
        pRecord->qcarcam_context = NOT_IN_USE;
    } else
        AIS_AIDL_ERRORMSG("Qcarcam context not found"); // Ignoring this error
    gMultiStreamQLock.unlock();

    // Notify client monitor thread for calling qcarcam_uninitialize()
    gClientNotifier.notify_one();

    mStreamObj.reset();
    pCurrentRecord = nullptr;
    mStreaming = false;
}

gQcarCamClientData* ais_aidl_stream::findEventQByHndl(const qcarcam_hndl_t qcarcam_hndl)
{
    // Find the named camera
    int i = 0;
    for (i=0; i<MAX_AIS_CLIENTS; i++) {
        if (gQcarCamClientArray[i].qcarcam_context == qcarcam_hndl) {
            // Found a match!
            return &gQcarCamClientArray[i];
        }
    }
    AIS_AIDL_INFOMSG("findEventQByHndl::ERROR qcarcam_context = %p did not match!!\n",qcarcam_hndl);

    // We didn't find a match
    return nullptr;
}

void ais_aidl_stream::copyPlayloadData(QcarcamEventPayload *pPayload, qcarcam_event_payload_t *pqPayload)
{
    QcarcamEventPayload::Tag tagIndex = pPayload->getTag();
    if(tagIndex == QcarcamEventPayload::Tag::uint_payload)
    {
        memcpy(&pPayload->get<QcarcamEventPayload::Tag::uint_payload>(), &pqPayload->uint_payload, sizeof(pPayload->uint_payload));
    }
    else if(tagIndex == QcarcamEventPayload::Tag::sof_timestamp)
    {
        memcpy(&pPayload->get<QcarcamEventPayload::Tag::sof_timestamp>(), &pqPayload->sof_timestamp, sizeof(pPayload->sof_timestamp));
    }
	else if(tagIndex == QcarcamEventPayload::Tag::frame_freeze)
    {
        memcpy(&pPayload->get<QcarcamEventPayload::Tag::frame_freeze>(), &pqPayload->frame_freeze, sizeof(pPayload->frame_freeze));
    }
    else if(tagIndex == QcarcamEventPayload::Tag::vendor_data)
    {
        memcpy(&pPayload->get<QcarcamEventPayload::Tag::vendor_data>(), &pqPayload->frame_freeze, sizeof(pPayload->vendor_data));
    }
    else if(tagIndex == QcarcamEventPayload::Tag::array)
    {
        memcpy(&pPayload->get<QcarcamEventPayload::Tag::array>(), &pqPayload->array, sizeof(pPayload->array));
    }
}

void ais_aidl_stream::eventDispatcherThread()
{
    int32_t q_size = 0;
    int i = 0;

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
            //AIS_AIDL_ERRORMSG("Found %d events in the Event Queue", q_size);
            QEventDesc qFront = this->pCurrentRecord->sQcarcamList.front();
            // Convert event to aidl type and call app cb
            QcarcamEvent EventType;
            QcarcamEventPayload Payload;
            EventType = static_cast<QcarcamEvent>(qFront.event_id);
            ais_aidl_stream::copyPlayloadData(&Payload, &qFront.payload);
            this->pCurrentRecord->sQcarcamList.pop_front();
            lk.unlock();
            std::unique_lock<std::mutex> api_lk(mApiLock);
            if (mStreamObj != nullptr) {
                // In AIDL its mandatory to check return status otherwise aidl
                // generate fake crash
                auto result = mStreamObj->qcarcam_event_callback(EventType, Payload);
                if (!result.isOk())
                    AIS_AIDL_DBGMSG("Event delivery call failed in the transport layer.");
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
                QEventDesc qFront = this->pCurrentRecord->sQcarcamList.front();
                // Convert event to aidl type and call app cb
                QcarcamEvent EventType;
                QcarcamEventPayload Payload;
                EventType = static_cast<QcarcamEvent>(qFront.event_id);
                ais_aidl_stream::copyPlayloadData(&Payload, &qFront.payload);
                this->pCurrentRecord->sQcarcamList.pop_front();
                lk.unlock();
                std::unique_lock<std::mutex> api_lk(mApiLock);
                if (mStreamObj != nullptr) {
                    // In AIDL its mandatory to check return status otherwise aidl
                    // generate fake crash
                    auto result = mStreamObj->qcarcam_event_callback(EventType, Payload);
                    if (!result.isOk())
                        AIS_AIDL_ERRORMSG("Event delivery call failed in the transport layer. mStreamObj: %#x", mStreamObj.get());
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
    AIS_AIDL_ERRORMSG("eventDispatcherThread thread exiting for context %p", mQcarcamHndl);
    mRunMode = STOPPED;
}

::ndk::ScopedAStatus ais_aidl_stream::configureStream_1_1(QcarcamStreamParam Param, const QcarcamStreamConfigData& data, QcarcamError* _aidl_return) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

::ndk::ScopedAStatus ais_aidl_stream::configureStream(QcarcamStreamParam Param, const QcarcamStreamConfigData& data, QcarcamError* _aidl_return) {
    AIS_AIDL_ERRORMSG("Entry configureStream");

    QcarcamError &Error = *_aidl_return;
    Error = QcarcamError::QCARCAM_OK;

    qcarcam_ret_t ret = QCARCAM_RET_OK;
    qcarcam_param_t q_param_type;
    qcarcam_param_value_t q_param_value;
    uint32_t p_value[128];
    QcarcamStreamConfigData::Tag tagIndex = data.getTag();
    if (tagIndex == QcarcamStreamConfigData::gamma_config) {
        if ((data.get<QcarcamStreamConfigData::gamma_config>()).config_type == static_cast<QcarcamGammaType>(QCARCAM_GAMMA_KNEEPOINTS))
            q_param_value.gamma_config.gamma.table.p_value = p_value;
    }

    // Convert input into qcarcam type
    Error = get_qcarcam_params(Param, data, &q_param_type, &q_param_value, NULL);

    if (QcarcamError::QCARCAM_OK == Error) {
        // Call qcarcam API
        ret = qcarcam_s_param(mQcarcamHndl, q_param_type, &q_param_value);
        if (ret != QCARCAM_RET_OK) {
            AIS_AIDL_ERRORMSG("qcarcam_s_param failed with error %d for param %d", ret, (int)q_param_type);
            Error =  (QcarcamError)ret;
        }
    }

    // Send back the results
    AIS_AIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_stream::setStreamBuffers_1_1(const ::aidl::android::hardware::common::NativeHandle& hndl,
    const QcarcamBuffersInfoList& info, QcarcamError* _aidl_return) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

::ndk::ScopedAStatus ais_aidl_stream::setStreamBuffers(const ::aidl::android::hardware::common::NativeHandle& hndl,
    const QcarcamBuffersInfoList& info, QcarcamError* _aidl_return) {
    AIS_AIDL_ERRORMSG("Entry setStreamBuffers");

    QcarcamError &Error = *_aidl_return;
    Error = QcarcamError::QCARCAM_OK;
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    struct native_handle* aNativeHndl = nullptr;

    if ((aNativeHndl = ::android::dupFromAidl(hndl)) == nullptr) {
        AIS_AIDL_ERRORMSG("Native handle is null!! sending Error status %d", static_cast<int>(Error));
        Error = QcarcamError::QCARCAM_FAILED;
        return ndk::ScopedAStatus::ok();
    }

    cloneHandle = native_handle_clone(aNativeHndl);
    if (!cloneHandle) {
        AIS_AIDL_ERRORMSG("Unable to clone native handle sending Error status %d", static_cast<int>(Error));
        Error = QcarcamError::QCARCAM_FAILED;
        return ndk::ScopedAStatus::ok();
    }

    qcarcam_bufferlist_t p_buffers;
    memset(&p_buffers, 0, sizeof(qcarcam_bufferlist_t));
    p_buffers.n_buffers = cloneHandle->numFds;
    p_buffers.buffers = (qcarcam_buffer_v2_t *)calloc(p_buffers.n_buffers, sizeof(*p_buffers.buffers));
    if (!p_buffers.buffers) {
        AIS_AIDL_ERRORMSG("Buffers are null!! sending Error status %d", static_cast<int>(Error));
        Error = QcarcamError::QCARCAM_FAILED;
        return ndk::ScopedAStatus::ok();
    }
    p_buffers.id = static_cast<unsigned int>(info.id);
    AIS_AIDL_ERRORMSG("Received %d FDs",p_buffers.n_buffers);
    for (int i=0; i<cloneHandle->numFds; i++)
    {
        p_buffers.buffers[i].n_planes = info.BuffersInfo[i].n_planes;
        p_buffers.color_fmt = static_cast<qcarcam_color_fmt_t>(info.BuffersInfo[i].color_fmt);
        AIS_AIDL_ERRORMSG("Fd[%d]=%d", i, cloneHandle->data[i]);
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
    AIS_AIDL_ERRORMSG("calling qcarcam_s_buffers_v2 mQcarcamHndl: %#x", mQcarcamHndl);
    ret = qcarcam_s_buffers_v2(mQcarcamHndl, &p_buffers);
    if (ret != QCARCAM_RET_OK) {
        AIS_AIDL_ERRORMSG("qcarcam_s_buffers failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = QcarcamError::QCARCAM_FAILED;
    }

    // Send back the results
    AIS_AIDL_ERRORMSG("sending Error status %d", static_cast<int>(Error));
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_stream::startStream_1_1(const std::shared_ptr<IQcarCameraStreamCB>& streamObj, QcarcamError* _aidl_return) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

::ndk::ScopedAStatus ais_aidl_stream::startStream(const std::shared_ptr<IQcarCameraStreamCB>& streamObj, QcarcamError* _aidl_return) {
    AIS_AIDL_ERRORMSG("Entry startStream: streamObj: %#x", streamObj.get());

    QcarcamError &Error = *_aidl_return;
    Error = QcarcamError::QCARCAM_OK;

    qcarcam_ret_t ret = QCARCAM_RET_OK;

    // Store the stream object for sending callback
    mStreamObj = streamObj;

    // Register a callback function if streamObj is valid
    if (mStreamObj != nullptr) {
        qcarcam_param_value_t param;
        param.ptr_value = (void *)&ais_aidl_stream::qcarcam_event_cb;
        ret = qcarcam_s_param(mQcarcamHndl, QCARCAM_PARAM_EVENT_CB, &param);
        if (ret != QCARCAM_RET_OK)
            AIS_AIDL_ERRORMSG("qcarcam_s_param failed for callback setting, proceeding without callback");
    }

    // Call qcarcam API
    ret = qcarcam_start(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        AIS_AIDL_ERRORMSG("qcarcam_start failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = QcarcamError::QCARCAM_FAILED;
        return ndk::ScopedAStatus::ok();
    }
    mStreaming = true;

    // Send back the results
    AIS_AIDL_ERRORMSG("sending Error status %d", static_cast<int>(Error));
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_stream::stopStream_1_1(QcarcamError* _aidl_return) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

::ndk::ScopedAStatus ais_aidl_stream::stopStream(QcarcamError* _aidl_return) {
    AIS_AIDL_INFOMSG("Entry stopStream");

    QcarcamError &Error = *_aidl_return;
    Error = QcarcamError::QCARCAM_OK;
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    std::unique_lock<std::mutex> api_lk(mApiLock);

    // Call qcarcam API
    ret = qcarcam_stop(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        AIS_AIDL_ERRORMSG("qcarcam_stop failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = QcarcamError::QCARCAM_FAILED;
    }
    mStreaming = false;

    api_lk.unlock();

    // Send back the results
    AIS_AIDL_ERRORMSG("sending Error status %d for hndl %p", static_cast<int>(Error), mQcarcamHndl);
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_stream::getFrame_1_1(int64_t timeout, int32_t flags, QcarcamFrameInfov2* aInfo, QcarcamError* _aidl_return) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

::ndk::ScopedAStatus ais_aidl_stream::getFrame(int64_t timeout, int32_t flags, QcarcamFrameInfov2* aInfo, QcarcamError* _aidl_return) {
    QcarcamError &Error = *_aidl_return;
    Error = QcarcamError::QCARCAM_OK;

    QcarcamFrameInfov2 &Info = *aInfo;
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    qcarcam_frame_info_v2_t frame_info;

    // Call qcarcam API
    ret = qcarcam_get_frame_v2(mQcarcamHndl, &frame_info, timeout, flags);
    if (ret != QCARCAM_RET_OK) {
        AIS_AIDL_ERRORMSG("qcarcam_get_frame failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = QcarcamError::QCARCAM_FAILED;
    } else {
        // Build up QcarcamFrameInfo for return
        Info.id = frame_info.id;
        Info.idx = frame_info.idx;
        Info.flags = frame_info.flags;
        for(unsigned int i=0; i < QCARCAM_MAX_BATCH_FRAMES; i++)
        Info.seq_no[i] = frame_info.seq_no[i];
        Info.timestamp = frame_info.timestamp;
        Info.timestamp_system = frame_info.timestamp_system;
        Info.field_type = static_cast<QcarcamFrameField>(frame_info.field_type);
    }

    // Send back the results
    //AIS_AIDL_ERRORMSG("sending Error status %d", static_cast<int>(Error));

    // AIDL convention says we return Void if we sent our result back via callback
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_stream::releaseFrame_1_1(int32_t id, int32_t idx, QcarcamError* _aidl_return) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

::ndk::ScopedAStatus ais_aidl_stream::releaseFrame(int32_t id, int32_t idx, QcarcamError* _aidl_return) {
    QcarcamError &Error = *_aidl_return;
    Error = QcarcamError::QCARCAM_OK;
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    // Call qcarcam API
    ret = qcarcam_release_frame_v2(mQcarcamHndl,id,idx);
    if (ret != QCARCAM_RET_OK) {
        AIS_AIDL_ERRORMSG("qcarcam_release_frame failed with error %d for hndl %p idx %u", ret, mQcarcamHndl, idx);
        Error = QcarcamError::QCARCAM_FAILED;
    }

    // Send back the results
    //AIS_AIDL_ERRORMSG("sending Error status %d", static_cast<int>(Error));
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_stream::pauseStream(QcarcamError* _aidl_return) {
    AIS_AIDL_ERRORMSG("Entry pauseStream");

    QcarcamError &Error = *_aidl_return;
    Error = QcarcamError::QCARCAM_OK;
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    // Call qcarcam API
    ret = qcarcam_pause(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        AIS_AIDL_ERRORMSG("qcarcam_pause failed with error %d for hndl %p", ret, mQcarcamHndl);
        Error = QcarcamError::QCARCAM_FAILED;
        return ndk::ScopedAStatus::ok();
    }
    mStreaming = false;

    // Send back the results
    AIS_AIDL_ERRORMSG("sending Error status %d", static_cast<int>(Error));
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_stream::resumeStream(QcarcamError* _aidl_return) {
    AIS_AIDL_ERRORMSG("Entry resumeStream");

    QcarcamError &Error = *_aidl_return;
    Error = QcarcamError::QCARCAM_OK;
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    // Call qcarcam API
    ret = qcarcam_resume(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        if (ret == QCARCAM_RET_BADSTATE && mStreaming == true)
        {
            AIS_AIDL_ERRORMSG("The streams have already been resumed !");
            Error = QcarcamError::QCARCAM_BADSTATE;
            return ndk::ScopedAStatus::ok();
        } else {
            mStreaming = false;
            AIS_AIDL_ERRORMSG("qcarcam_resume failed with error %d for hndl %p", ret, mQcarcamHndl);
            Error = QcarcamError::QCARCAM_FAILED;
            return ndk::ScopedAStatus::ok();
        }
    }
    mStreaming = true;

    // Send back the results
    AIS_AIDL_ERRORMSG("sending Error status %d", static_cast<int>(Error));
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_stream::getStreamConfig_1_1(QcarcamStreamParam Param,
    QcarcamStreamConfigData* data, QcarcamError* _aidl_return) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

::ndk::ScopedAStatus ais_aidl_stream::getStreamConfig(QcarcamStreamParam Param, const QcarcamStreamConfigData& in_data,
    QcarcamStreamConfigData* data, QcarcamError* _aidl_return) {
    AIS_AIDL_ERRORMSG("Entry getStreamConfig");

    QcarcamError &Error = *_aidl_return;
    Error = QcarcamError::QCARCAM_OK;

    QcarcamStreamConfigData StreamData = in_data;
    QcarcamStreamConfigData &out_StreamData = *data;
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
            AIS_AIDL_ERRORMSG("qcarcam_g_param failed with error %d for param %d", ret, (int)q_param_type);
            Error =  (QcarcamError)ret;
        } else {
            // Copy the results to aidl type for retuning
            Error = get_ais_aidl_params(q_param_type, &q_param_value, Param, out_StreamData, gamma_table);
        }
    }

    // Send back the results
    AIS_AIDL_ERRORMSG("sending Error status %d", static_cast<int>(Error));

    // AIDL convention says we return Void if we sent our result back via callback
    return ndk::ScopedAStatus::ok();
}

QcarcamError ais_aidl_stream::get_qcarcam_params(QcarcamStreamParam Param,
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
                    param_value->uint_value = static_cast<unsigned int>(data.get<QcarcamStreamConfigData::uint_value>());
            }
            break;
        case QcarcamStreamParam::QCARCAM_COLOR_FMT:
            {
                *param_type = QCARCAM_PARAM_COLOR_FMT;
                if (param_value)
                    param_value->color_value = static_cast<qcarcam_color_fmt_t>(data.get<QcarcamStreamConfigData::color_value>());
            }
            break;
        case QcarcamStreamParam::QCARCAM_RESOLUTION:
            {
                *param_type = QCARCAM_PARAM_RESOLUTION;
                if (param_value) {
                    param_value->res_value.width = static_cast<unsigned int>(data.get<QcarcamStreamConfigData::res_value>().width);
                    param_value->res_value.height = static_cast<unsigned int>(data.get<QcarcamStreamConfigData::res_value>().height);
                    param_value->res_value.fps = static_cast<float>(data.get<QcarcamStreamConfigData::res_value>().fps);
                }
            }
            break;
        case QcarcamStreamParam::QCARCAM_BRIGHTNESS:
            {
                *param_type = QCARCAM_PARAM_BRIGHTNESS;
                 if(param_value)
                     param_value->float_value = static_cast<float>(data.get<QcarcamStreamConfigData::float_value>());
            }
            break;
        case QcarcamStreamParam::QCARCAM_VENDOR:
            {
                *param_type = QCARCAM_PARAM_VENDOR;
                if(param_value) {
                    for(unsigned int i = 0; i < QCARCAM_MAX_VENDOR_PAYLOAD_SIZE; i++){
                        param_value->vendor_param.data[i] = static_cast<uint32_t>(data.get<QcarcamStreamConfigData::vendor_param>().data[i]);
                  }
                }
            }
            break;
        case QcarcamStreamParam::QCARCAM_INPUT_MODE:
            {
                *param_type = QCARCAM_PARAM_INPUT_MODE;
                if(param_value)
                 param_value->uint_value =  static_cast<uint32_t>(data.get<QcarcamStreamConfigData::uint_value>());
            }
            break;
        case QcarcamStreamParam::QCARCAM_MASTER:
            {
                *param_type = QCARCAM_PARAM_MASTER;
                if(param_value)
                 param_value->uint_value =  static_cast<uint32_t>(data.get<QcarcamStreamConfigData::uint_value>());
            }
            break;
        case QcarcamStreamParam::QCARCAM_EVENT_CHANGE_SUBSCRIBE:
            {
                *param_type = QCARCAM_PARAM_EVENT_CHANGE_SUBSCRIBE;
                if(param_value)
                 param_value->uint64_value =  static_cast<uint64_t>(data.get<QcarcamStreamConfigData::uint64_value>());
            }
            break;
        case QcarcamStreamParam::QCARCAM_EVENT_CHANGE_UNSUBSCRIBE:
            {
                *param_type = QCARCAM_PARAM_EVENT_CHANGE_UNSUBSCRIBE;
                if(param_value)
                 param_value->uint64_value =  static_cast<uint64_t>(data.get<QcarcamStreamConfigData::uint64_value>());
            }
            break;
        case QcarcamStreamParam::QCARCAM_BATCH_MODE:
            {
                *param_type = QCARCAM_PARAM_BATCH_MODE;
                if(param_value)
                param_value->batch_config.num_batch_frames = static_cast<uint32_t>(data.get<QcarcamStreamConfigData::batch_config>().num_batch_frames);
            }
            break;
        case QcarcamStreamParam::QCARCAM_ISP_USECASE:
            {
                *param_type = QCARCAM_PARAM_ISP_USECASE;
                if(param_value) {
                   param_value->uint_value =  static_cast<uint32_t>(data.get<QcarcamStreamConfigData::uint_value>());
                   param_value->isp_config.id = data.get<QcarcamStreamConfigData::isp_config>().id;
                   param_value->isp_config.camera_id = data.get<QcarcamStreamConfigData::isp_config>().camera_id;
                   param_value->isp_config.use_case = static_cast<qcarcam_isp_usecase_t>(data.get<QcarcamStreamConfigData::isp_config>().use_case);
                }
            }
            break;
        case QcarcamStreamParam::QCARCAM_CONTRAST:
            {
                *param_type = QCARCAM_PARAM_CONTRAST;
                if(param_value)
                 param_value->float_value = static_cast<float>(data.get<QcarcamStreamConfigData::float_value>());
            }
            break;
        case QcarcamStreamParam::QCARCAM_MIRROR_H:
            {
                *param_type = QCARCAM_PARAM_MIRROR_H;
                if(param_value)
                param_value->uint_value =  static_cast<uint32_t>(data.get<QcarcamStreamConfigData::uint_value>());
            }
            break;
        case QcarcamStreamParam::QCARCAM_MIRROR_V:
            {
                *param_type = QCARCAM_PARAM_MIRROR_V;
                if(param_value)
                param_value->uint_value =  static_cast<uint32_t>(data.get<QcarcamStreamConfigData::uint_value>());
            }
            break;
        case QcarcamStreamParam::QCARCAM_FRAME_RATE:
            {
                *param_type = QCARCAM_PARAM_FRAME_RATE;
                if (param_value) {
                    param_value->frame_rate_config.frame_drop_mode = static_cast<qcarcam_frame_drop_mode_t>(data.get<QcarcamStreamConfigData::frame_rate_config>().frame_drop_mode);
                    param_value->frame_rate_config.frame_drop_period = static_cast<unsigned char>(data.get<QcarcamStreamConfigData::frame_rate_config>().frame_drop_period);
                    param_value->frame_rate_config.frame_drop_pattern = static_cast<unsigned int>(data.get<QcarcamStreamConfigData::frame_rate_config>().frame_drop_pattern);
                }
            }
            break;
        case QcarcamStreamParam::QCARCAM_VID_STD:
            {
                *param_type = QCARCAM_PARAM_VID_STD;
                if(param_value)
                param_value->uint_value =  static_cast<uint32_t>(data.get<QcarcamStreamConfigData::uint_value>());
            }
            break;
        case QcarcamStreamParam::QCARCAM_CURRENT_VID_STD:
            {
                *param_type = QCARCAM_PARAM_CURRENT_VID_STD;
                if(param_value)
                param_value->uint_value =  static_cast<uint32_t>(data.get<QcarcamStreamConfigData::uint_value>());
            }
            break;
        case QcarcamStreamParam::QCARCAM_STATUS:
            {
                *param_type = QCARCAM_PARAM_STATUS;
                if(param_value)
                param_value->uint_value =  static_cast<uint32_t>(data.get<QcarcamStreamConfigData::uint_value>());
            }
            break;
        case QcarcamStreamParam::QCARCAM_LATENCY_MAX:
            {
                *param_type = QCARCAM_PARAM_LATENCY_MAX;
                if(param_value)
                param_value->uint_value =  static_cast<uint32_t>(data.get<QcarcamStreamConfigData::uint_value>());
            }
            break;
        case QcarcamStreamParam::QCARCAM_LATENCY_REDUCE_RATE:
            {
                *param_type = QCARCAM_PARAM_LATENCY_REDUCE_RATE;
                if(param_value)
                param_value->uint_value =  static_cast<uint32_t>(data.get<QcarcamStreamConfigData::uint_value>());
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
                    param_value->uint_value = static_cast<unsigned int>(data.get<QcarcamStreamConfigData::uint_value>());
            }
            break;
        case QcarcamStreamParam::QCARCAM_EXPOSURE:
            {
                *param_type = QCARCAM_PARAM_EXPOSURE;
                if (param_value) {
                    param_value->exposure_config.exposure_mode_type = static_cast<qcarcam_exposure_mode_t>(data.get<QcarcamStreamConfigData::exposure_config>().exposure_mode_type);
                    param_value->exposure_config.exposure_time = static_cast<float>(data.get<QcarcamStreamConfigData::exposure_config>().exposure_time);
                    param_value->exposure_config.gain = static_cast<float>(data.get<QcarcamStreamConfigData::exposure_config>().gain);
                    param_value->exposure_config.target = static_cast<float>(data.get<QcarcamStreamConfigData::exposure_config>().target);
                    param_value->exposure_config.lux_index = static_cast<float>(data.get<QcarcamStreamConfigData::exposure_config>().lux_index);
                }
            }
            break;
        case QcarcamStreamParam::QCARCAM_HDR_EXPOSURE:
            {
                *param_type = QCARCAM_PARAM_HDR_EXPOSURE;
                if (param_value) {
                    param_value->hdr_exposure_config.exposure_mode_type = static_cast<qcarcam_exposure_mode_t>(data.get<QcarcamStreamConfigData::hdr_exposure_config>().exposure_mode_type);
                    param_value->hdr_exposure_config.hdr_mode = static_cast<uint32_t>(data.get<QcarcamStreamConfigData::hdr_exposure_config>().hdr_mode);
                    param_value->hdr_exposure_config.target = static_cast<float>(data.get<QcarcamStreamConfigData::hdr_exposure_config>().target);
                    param_value->hdr_exposure_config.lux_index = static_cast<float>(data.get<QcarcamStreamConfigData::hdr_exposure_config>().lux_index);
                    param_value->hdr_exposure_config.num_exposures = static_cast<uint32_t>(data.get<QcarcamStreamConfigData::hdr_exposure_config>().num_exposures);
                    for (uint32_t i=0; i<param_value->hdr_exposure_config.num_exposures; i++) {
                        param_value->hdr_exposure_config.exposure_time[i] = static_cast<float>(data.get<QcarcamStreamConfigData::hdr_exposure_config>().exposure_time[i]);
                        param_value->hdr_exposure_config.exposure_ratio[i] = static_cast<float>(data.get<QcarcamStreamConfigData::hdr_exposure_config>().exposure_ratio[i]);
                        param_value->hdr_exposure_config.gain[i] = static_cast<float>(data.get<QcarcamStreamConfigData::hdr_exposure_config>().gain[i]);
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
                    if (data.get<QcarcamStreamConfigData::gamma_config>().config_type == static_cast<QcarcamGammaType>(QCARCAM_GAMMA_EXPONENT)) {
                        param_value->gamma_config.gamma.f_value = static_cast<float>(data.get<QcarcamStreamConfigData::gamma_config>().gamma.f_value);
                    }
                    else if (data.get<QcarcamStreamConfigData::gamma_config>().config_type == static_cast<QcarcamGammaType>(QCARCAM_GAMMA_KNEEPOINTS)) {
                        param_value->gamma_config.gamma.table.length = static_cast<uint32_t>(data.get<QcarcamStreamConfigData::gamma_config>().gamma.get<QcarcamGamma::table>().length);
                        if (param_value->gamma_config.gamma.table.length <= QCARCAM_MAX_GAMMA_TABLE) {
                            if (gamma_table == NULL) {
                                /* gamma_table eq NULL then call is for SET gamma user defined params */
                                memcpy(param_value->gamma_config.gamma.table.p_value, &data.get<QcarcamStreamConfigData::gamma_config>().gamma.get<QcarcamGamma::table>().arr_gamma, param_value->gamma_config.gamma.table.length * sizeof(unsigned int));
                            }
                            else {
                                unsigned int *gamma_values = (unsigned int *)calloc(param_value->gamma_config.gamma.table.length, sizeof(unsigned int));

                                if (NULL == gamma_values) {
                                    AIS_AIDL_ERRORMSG("Failed to allocate table of size %d", param_value->gamma_config.gamma.table.length);
                                    Error = QcarcamError::QCARCAM_FAILED;
                                }
                                param_value->gamma_config.gamma.table.p_value = gamma_values;
                                *gamma_table = gamma_values;
                            }
                        }
                    }
                    param_value->gamma_config.config_type = static_cast<qcarcam_gamma_type_t>(data.get<QcarcamStreamConfigData::gamma_config>().config_type);
                }
            }
            break;
        case QcarcamStreamParam::QCARCAM_OPMODE:
            {
                *param_type = QCARCAM_PARAM_OPMODE;
                if (param_value)
                    param_value->uint_value = static_cast<unsigned int>(data.get<QcarcamStreamConfigData::uint_value>());
            }
            break;
        case QcarcamStreamParam::QCARCAM_RECOVERY:
            {
                *param_type = QCARCAM_PARAM_RECOVERY;
                if (param_value)
                    param_value->uint_value = static_cast<unsigned int>(data.get<QcarcamStreamConfigData::uint_value>());
            }
            break;
        case QcarcamStreamParam::QCARCAM_ISP_CTRLS:
            {
                *param_type = QCARCAM_PARAM_ISP_CTRLS;
                if (param_value)
                {
                    param_value->isp_ctrls.param_mask = data.get<QcarcamStreamConfigData::isp_ctrls>().param_mask;
                    if (CHECK_BIT(data.get<QcarcamStreamConfigData::isp_ctrls>().param_mask, QCARCAM_CONTROL_AE_MODE))
                        param_value->isp_ctrls.ae_mode = static_cast<qcarcam_ctrl_ae_mode_t>(data.get<QcarcamStreamConfigData::isp_ctrls>().ae_mode);
                    if (CHECK_BIT(data.get<QcarcamStreamConfigData::isp_ctrls>().param_mask, QCARCAM_CONTROL_AE_LOCK))
                        param_value->isp_ctrls.ae_lock = static_cast<qcarcam_ctrl_ae_lock_t>(data.get<QcarcamStreamConfigData::isp_ctrls>().ae_lock);
                    if (CHECK_BIT(data.get<QcarcamStreamConfigData::isp_ctrls>().param_mask, QCARCAM_CONTROL_AWB_MODE))
                        param_value->isp_ctrls.awb_mode = static_cast<qcarcam_ctrl_awb_mode_t>(data.get<QcarcamStreamConfigData::isp_ctrls>().awb_mode);
                    if (CHECK_BIT(data.get<QcarcamStreamConfigData::isp_ctrls>().param_mask, QCARCAM_CONTROL_AWB_LOCK))
                        param_value->isp_ctrls.awb_lock = static_cast<qcarcam_ctrl_awb_lock_t>(data.get<QcarcamStreamConfigData::isp_ctrls>().awb_lock);
                    if (CHECK_BIT(data.get<QcarcamStreamConfigData::isp_ctrls>().param_mask, QCARCAM_CONTROL_EFFECT_MODE))
                        param_value->isp_ctrls.effect_mode = static_cast<qcarcam_ctrl_control_effect_mode_t>(data.get<QcarcamStreamConfigData::isp_ctrls>().effect_mode);
                    if (CHECK_BIT(data.get<QcarcamStreamConfigData::isp_ctrls>().param_mask, QCARCAM_CONTROL_MODE))
                        param_value->isp_ctrls.ctrl_mode = static_cast<qcarcam_ctrl_control_mode_t>(data.get<QcarcamStreamConfigData::isp_ctrls>().ctrl_mode);
                    if (CHECK_BIT(data.get<QcarcamStreamConfigData::isp_ctrls>().param_mask, QCARCAM_CONTROL_SCENE_MODE))
                        param_value->isp_ctrls.scene_mode = static_cast<qcarcam_ctrl_control_scene_mode_t>(data.get<QcarcamStreamConfigData::isp_ctrls>().scene_mode);
                    if (CHECK_BIT(data.get<QcarcamStreamConfigData::isp_ctrls>().param_mask, QCARCAM_CONTROL_AE_ANTIBANDING_MODE))
                        param_value->isp_ctrls.ae_antibanding_mode = static_cast<qcarcam_ctrl_ae_antibanding_mode_t>(data.get<QcarcamStreamConfigData::isp_ctrls>().ae_antibanding_mode);
                }
            }
            break;
        default:
            AIS_AIDL_ERRORMSG("Invalid config param %d", static_cast<int>(Param));
            Error = QcarcamError::QCARCAM_FAILED;
            break;
    }
    return Error;
}

QcarcamError ais_aidl_stream::get_ais_aidl_params(qcarcam_param_t param_type, qcarcam_param_value_t* param_value,
        QcarcamStreamParam& Param, QcarcamStreamConfigData& data, unsigned int *gamma_table)
{
    QcarcamError Error = QcarcamError::QCARCAM_OK;
    switch (param_type)
    {
        case QCARCAM_PARAM_EVENT_MASK:
            {
                Param = QcarcamStreamParam::QCARCAM_CB_EVENT_MASK;
                if (param_value)
                    data.set<QcarcamStreamConfigData::uint_value>(static_cast<uint32_t>(param_value->uint_value));
            }
            break;
        case QCARCAM_PARAM_COLOR_FMT:
            {
                Param = QcarcamStreamParam::QCARCAM_COLOR_FMT;
                if (param_value)
                    data.set<QcarcamStreamConfigData::color_value>(static_cast<QcarcamColorFmt>(param_value->color_value));
            }
            break;
        case QCARCAM_PARAM_RESOLUTION:
            {
                Param = QcarcamStreamParam::QCARCAM_RESOLUTION;
                if (param_value) {
                    QcarcamResolution tmp_res_value;
                    tmp_res_value.width = static_cast<uint32_t>(param_value->res_value.width);
                    tmp_res_value.height = static_cast<uint32_t>(param_value->res_value.height);
                    tmp_res_value.fps = static_cast<float>(param_value->res_value.fps);
                    data.set<QcarcamStreamConfigData::res_value>(tmp_res_value);
                }
            }
            break;
        case QCARCAM_PARAM_BRIGHTNESS:
            {
                Param = QcarcamStreamParam::QCARCAM_BRIGHTNESS;
                if(param_value){
                    data.set<QcarcamStreamConfigData::float_value>(static_cast<float>(param_value->float_value));
                }
            }
            break;
                case QCARCAM_PARAM_VENDOR:
            {
                Param = QcarcamStreamParam::QCARCAM_VENDOR;
                if(param_value){
                     QcarcamVendorParam tmp_vendor_param;
                     for(unsigned int i = 0; i < QCARCAM_MAX_VENDOR_PAYLOAD_SIZE; i++) {
                         tmp_vendor_param.data[i] = static_cast<uint32_t>(param_value->vendor_param.data[i]);
                     }
                     data.set<QcarcamStreamConfigData::vendor_param>(tmp_vendor_param);
                }
            }
            break;
        case QCARCAM_PARAM_INPUT_MODE:
            {
                Param = QcarcamStreamParam::QCARCAM_INPUT_MODE;
                 if(param_value){
                    data.set<QcarcamStreamConfigData::uint_value>(static_cast<uint32_t>(param_value->uint_value));
                }
            }
            break;
        case QCARCAM_PARAM_MASTER:
            {
                Param = QcarcamStreamParam::QCARCAM_MASTER;
                 if(param_value){
                    data.set<QcarcamStreamConfigData::uint_value>(static_cast<uint32_t>(param_value->uint_value));
                }
            }
            break;
        case QCARCAM_PARAM_EVENT_CHANGE_SUBSCRIBE:
            {
                Param = QcarcamStreamParam::QCARCAM_EVENT_CHANGE_SUBSCRIBE;
                if(param_value){
                    data.set<QcarcamStreamConfigData::uint64_value>(static_cast<uint64_t>(param_value->uint64_value));
                }
            }
            break;
        case QCARCAM_PARAM_EVENT_CHANGE_UNSUBSCRIBE:
            {
                Param = QcarcamStreamParam::QCARCAM_EVENT_CHANGE_UNSUBSCRIBE;
                if(param_value){
                    data.set<QcarcamStreamConfigData::uint64_value>(static_cast<uint64_t>(param_value->uint64_value));
                }
            }
            break;
        case QCARCAM_PARAM_BATCH_MODE:
            {
                Param = QcarcamStreamParam::QCARCAM_BATCH_MODE;
                if(param_value){
                  data.get<QcarcamStreamConfigData::batch_config>().num_batch_frames = static_cast<uint32_t>(param_value->batch_config.num_batch_frames);
                }
            }
            break;
        case QCARCAM_PARAM_ISP_USECASE:
            {
                Param = QcarcamStreamParam::QCARCAM_ISP_USECASE;
                if(param_value){
                    QcarcamIspUsecaseConfig tmp_isp_config;
                    //data.set<QcarcamStreamConfigData::uint_value>(static_cast<uint32_t>(param_value->uint_value));
                    tmp_isp_config.id = param_value->isp_config.id;
                    tmp_isp_config.camera_id = param_value->isp_config.camera_id;
                    tmp_isp_config.use_case = static_cast<QcarcamIspUsecase>(param_value->isp_config.use_case);
                    data.set<QcarcamStreamConfigData::isp_config>(tmp_isp_config);
                }
            }
            break;
        case QCARCAM_PARAM_CONTRAST:
            {
                Param = QcarcamStreamParam::QCARCAM_CONTRAST;
                 if(param_value){
                    data.set<QcarcamStreamConfigData::float_value>(static_cast<float>(param_value->float_value));
                }
            }
            break;
        case QCARCAM_PARAM_MIRROR_H:
            {
                Param = QcarcamStreamParam::QCARCAM_MIRROR_H;
                if(param_value){
                    data.set<QcarcamStreamConfigData::uint_value>(static_cast<uint32_t>(param_value->uint_value));
                }
            }
            break;
        case QCARCAM_PARAM_MIRROR_V:
            {
                Param = QcarcamStreamParam::QCARCAM_MIRROR_V;
                if(param_value){
                    data.set<QcarcamStreamConfigData::uint_value>(static_cast<uint32_t>(param_value->uint_value));
                }
            }
            break;
        case QCARCAM_PARAM_FRAME_RATE:
            {
                Param = QcarcamStreamParam::QCARCAM_FRAME_RATE;
                if (param_value) {
                    QcarcamFrameRate tmp_frame_rate_config;
                    tmp_frame_rate_config.frame_drop_mode = static_cast<QcarcamFrameDropMode>(param_value->frame_rate_config.frame_drop_mode);
                    tmp_frame_rate_config.frame_drop_period = static_cast<uint16_t>(param_value->frame_rate_config.frame_drop_period);
                    tmp_frame_rate_config.frame_drop_pattern = static_cast<uint32_t>(param_value->frame_rate_config.frame_drop_pattern);
                    data.set<QcarcamStreamConfigData::frame_rate_config>(tmp_frame_rate_config);
                }
            }
            break;
        case QCARCAM_PARAM_VID_STD:
            {
                Param = QcarcamStreamParam::QCARCAM_VID_STD;
                 if(param_value){
                    data.set<QcarcamStreamConfigData::uint_value>(static_cast<uint32_t>(param_value->uint_value));
                }
            }
            break;
        case QCARCAM_PARAM_CURRENT_VID_STD:
            {
                Param = QcarcamStreamParam::QCARCAM_CURRENT_VID_STD;
                 if(param_value){
                    data.set<QcarcamStreamConfigData::uint_value>(static_cast<uint32_t>(param_value->uint_value));
                }
            }
            break;
        case QCARCAM_PARAM_STATUS:
            {
                Param = QcarcamStreamParam::QCARCAM_STATUS;
                 if(param_value){
                    data.set<QcarcamStreamConfigData::uint_value>(static_cast<uint32_t>(param_value->uint_value));
                }
            }
            break;
        case QCARCAM_PARAM_LATENCY_MAX:
            {
                Param = QcarcamStreamParam::QCARCAM_LATENCY_MAX;
                 if(param_value){
                    data.set<QcarcamStreamConfigData::uint_value>(static_cast<uint32_t>(param_value->uint_value));
                }
            }
            break;
        case QCARCAM_PARAM_LATENCY_REDUCE_RATE:
            {
                Param = QcarcamStreamParam::QCARCAM_LATENCY_REDUCE_RATE;
                 if(param_value){
                    data.set<QcarcamStreamConfigData::uint_value>(static_cast<uint32_t>(param_value->uint_value));
                }
            }
            break;
        case QCARCAM_PARAM_OPMODE:
            {
                Param = QcarcamStreamParam::QCARCAM_OPMODE;
                 if(param_value){
                    data.set<QcarcamStreamConfigData::uint_value>(static_cast<uint32_t>(param_value->uint_value));
                }
            }
            break;
        case QCARCAM_PARAM_RECOVERY:
            {
                Param = QcarcamStreamParam::QCARCAM_RECOVERY;
                 if(param_value){
                    data.set<QcarcamStreamConfigData::uint_value>(static_cast<uint32_t>(param_value->uint_value));
                }
            }
            break;
        case QCARCAM_PARAM_PRIVATE_DATA:
            {
                Param = QcarcamStreamParam::QCARCAM_PRIVATE_DATA;
                if (param_value)
                    data.set<QcarcamStreamConfigData::ptr_value>((uint64_t)param_value->ptr_value);
            }
            break;
        case QCARCAM_PARAM_INJECTION_START:
            {
                Param = QcarcamStreamParam::QCARCAM_INJECTION_START;
                if (param_value)
                    data.set<QcarcamStreamConfigData::uint_value>(static_cast<uint32_t>(param_value->uint_value));
            }
            break;
        case QCARCAM_PARAM_EXPOSURE:
            {
                Param = QcarcamStreamParam::QCARCAM_EXPOSURE;
                if (param_value) {
                    QcarcamExposureConfig tmp_exposure_config;
                    tmp_exposure_config.exposure_mode_type = static_cast<QcarcamExposureMode>(param_value->exposure_config.exposure_mode_type);
                    tmp_exposure_config.exposure_time = static_cast<float>(param_value->exposure_config.exposure_time);
                    tmp_exposure_config.gain = static_cast<float>(param_value->exposure_config.gain);
                    tmp_exposure_config.target = static_cast<float>(param_value->exposure_config.target);
                    tmp_exposure_config.lux_index = static_cast<float>(param_value->exposure_config.lux_index);
                    data.set<QcarcamStreamConfigData::exposure_config>(tmp_exposure_config);
                }
            }
            break;
        case QCARCAM_PARAM_HDR_EXPOSURE:
            {
                Param = QcarcamStreamParam::QCARCAM_HDR_EXPOSURE;
                if (param_value) {
                    QcarcamHDRExposureConfig tmp_hdr_exposure_config;
                    tmp_hdr_exposure_config.exposure_mode_type = static_cast<QcarcamExposureMode>(param_value->hdr_exposure_config.exposure_mode_type);
                    tmp_hdr_exposure_config.hdr_mode = static_cast<uint32_t>(param_value->hdr_exposure_config.hdr_mode);
                    tmp_hdr_exposure_config.target = static_cast<float>(param_value->hdr_exposure_config.target);
                    tmp_hdr_exposure_config.lux_index = static_cast<float>(param_value->hdr_exposure_config.lux_index);
                    tmp_hdr_exposure_config.num_exposures = static_cast<uint32_t>(param_value->hdr_exposure_config.num_exposures);
                    for (uint32_t i = 0; i < tmp_hdr_exposure_config.num_exposures; i++) {
                        tmp_hdr_exposure_config.exposure_time[i] = static_cast<float>(param_value->hdr_exposure_config.exposure_time[i]);
                        tmp_hdr_exposure_config.exposure_ratio[i] = static_cast<float>(param_value->hdr_exposure_config.exposure_ratio[i]);
                        tmp_hdr_exposure_config.gain[i] = static_cast<float>(param_value->hdr_exposure_config.gain[i]);
                    }
                    data.set<QcarcamStreamConfigData::hdr_exposure_config>(tmp_hdr_exposure_config);
                }
            }
            break;
        case QCARCAM_PARAM_HUE:
            {
                Param = QcarcamStreamParam::QCARCAM_HUE;
                if (param_value)
                    data.set<QcarcamStreamConfigData::float_value>(static_cast<float>(param_value->float_value));
            }
            break;
        case QCARCAM_PARAM_SATURATION:
            {
                Param = QcarcamStreamParam::QCARCAM_SATURATION;
                if (param_value)
                    data.set<QcarcamStreamConfigData::float_value>(static_cast<float>(param_value->float_value));
            }
            break;
        case QCARCAM_PARAM_GAMMA:
            {
                Param = QcarcamStreamParam::QCARCAM_GAMMA;
                if (param_value) {
                    QcarcamGammaConfig tmp_gamma_config;
                    data.set<QcarcamStreamConfigData::gamma_config>(tmp_gamma_config);
                    if (param_value->gamma_config.config_type == QCARCAM_GAMMA_EXPONENT) {
                        data.get<QcarcamStreamConfigData::gamma_config>().gamma.set<QcarcamGamma::f_value>(static_cast<float>(param_value->gamma_config.gamma.f_value));
                    }
                    else if (param_value->gamma_config.config_type == QCARCAM_GAMMA_KNEEPOINTS) {
                        data.get<QcarcamStreamConfigData::gamma_config>().gamma.get<QcarcamGamma::table>().length = static_cast<uint32_t>(param_value->gamma_config.gamma.table.length);
                        if (param_value->gamma_config.gamma.table.length <= QCARCAM_MAX_GAMMA_TABLE) {
                            if (gamma_table != NULL) {
                                memcpy(&data.get<QcarcamStreamConfigData::gamma_config>().gamma.get<QcarcamGamma::table>().arr_gamma, gamma_table, param_value->gamma_config.gamma.table.length * sizeof(unsigned int));
                                free(gamma_table);
                            }
                        }
                    }
                    data.get<QcarcamStreamConfigData::gamma_config>().config_type = static_cast<QcarcamGammaType>(param_value->gamma_config.config_type);
                }

            }
            break;
        case QCARCAM_PARAM_ISP_CTRLS:
            {
                Param = QcarcamStreamParam::QCARCAM_ISP_CTRLS;
                if (param_value)
                {
                    QcarcamIspCtrls tmp_isp_ctrls;
                    tmp_isp_ctrls.param_mask = param_value->isp_ctrls.param_mask;
                    if (CHECK_BIT(tmp_isp_ctrls.param_mask, QCARCAM_CONTROL_AE_MODE))
                        tmp_isp_ctrls.ae_mode = static_cast<QcarcamCtrlAEMode>(param_value->isp_ctrls.ae_mode);
                    if (CHECK_BIT(tmp_isp_ctrls.param_mask, QCARCAM_CONTROL_AE_LOCK))
                        tmp_isp_ctrls.ae_lock = static_cast<QcarcamCtrlAELock>(param_value->isp_ctrls.ae_lock);
                    if (CHECK_BIT(tmp_isp_ctrls.param_mask, QCARCAM_CONTROL_AWB_MODE))
                        tmp_isp_ctrls.awb_mode = static_cast<QcarcamCtrlAWBMode>(param_value->isp_ctrls.awb_mode);
                    if (CHECK_BIT(tmp_isp_ctrls.param_mask, QCARCAM_CONTROL_AWB_LOCK))
                        tmp_isp_ctrls.awb_lock = static_cast<QcarcamCtrlAWBLock>(param_value->isp_ctrls.awb_lock);
                    if (CHECK_BIT(tmp_isp_ctrls.param_mask, QCARCAM_CONTROL_EFFECT_MODE))
                        tmp_isp_ctrls.effect_mode = static_cast<QcarcamCtrlControlEffectMode>(param_value->isp_ctrls.effect_mode);
                    if (CHECK_BIT(tmp_isp_ctrls.param_mask, QCARCAM_CONTROL_MODE))
                        tmp_isp_ctrls.ctrl_mode = static_cast<QcarcamCtrlControlMode>(param_value->isp_ctrls.ctrl_mode);
                    if (CHECK_BIT(tmp_isp_ctrls.param_mask, QCARCAM_CONTROL_SCENE_MODE))
                        tmp_isp_ctrls.scene_mode = static_cast<QcarcamCtrlControlSceneMode>(param_value->isp_ctrls.scene_mode);
                    if (CHECK_BIT(tmp_isp_ctrls.param_mask, QCARCAM_CONTROL_AE_ANTIBANDING_MODE))
                        tmp_isp_ctrls.ae_antibanding_mode = static_cast<QcarcamCtrlAEAntibandingMode>(param_value->isp_ctrls.ae_antibanding_mode);
                    data.set<QcarcamStreamConfigData::isp_ctrls>(tmp_isp_ctrls);
                }
            }
            break;
        default: AIS_AIDL_ERRORMSG("Invalid config param %d",static_cast<int>(param_type));
                 Error = QcarcamError::QCARCAM_FAILED;
    }
    return Error;
}

} // qcarcam
} // automotive
} // qti
} // vendor
} // aidl

