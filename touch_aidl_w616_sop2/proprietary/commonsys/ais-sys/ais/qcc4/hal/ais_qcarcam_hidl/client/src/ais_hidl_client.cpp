/* ===========================================================================
 * Copyright (c) 2019-2020,2022,2024 Qualcomm Technologies, Inc.
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
#define QCARCAMBUFFERSINFOV2_SIZE 20

using namespace std;

using vendor::qti::automotive::qcarcam::V1_1::IQcarCamera;
using vendor::qti::automotive::qcarcam::V1_1::IQcarCameraStreamCB;
using ::android::hardware::hidl_vec;
using ::memcpy;
using ::android::sp;

// Default constructors
ais_hidl_client::ais_hidl_client(android::sp<vendor::qti::automotive::qcarcam::V1_1::IQcarCamera> service): mpService(service)
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

// Override callback method
Return<void> ais_hidl_client::qcarcam_event_callback(vendor::qti::automotive::qcarcam::V1_0::QcarcamEvent EventType,
        const vendor::qti::automotive::qcarcam::V1_0::QcarcamEventPayload& Payload)
{
    std::unique_lock<std::mutex> lock(mLock);
    AIS_HIDL_DBGMSG("Received qcarcam_event_callback");
    qcarcam_event_t event_id;
    qcarcam_event_payload_t event_payload;
    event_id = static_cast<qcarcam_event_t>(EventType);
    memcpy(&event_payload.vendor_data, &Payload.vendor_data, sizeof(event_payload.vendor_data));
    lock.unlock();
    if (app_cb)
        app_cb(mQcarcamHndl, event_id, &event_payload);
    return Void();
}

// Override callback method
Return<void> ais_hidl_client::qcarcam_event_callback_1_1(vendor::qti::automotive::qcarcam::V1_1::QcarcamEvent EventType,
        const vendor::qti::automotive::qcarcam::V1_1::QcarcamEventPayload& Payload)
{
    std::unique_lock<std::mutex> lock(mLock);
    AIS_HIDL_DBGMSG("Received qcarcam_event_callback");
    qcarcam_event_t event_id;
    qcarcam_event_payload_t event_payload;
    event_id = static_cast<qcarcam_event_t>(EventType);
    memcpy(&event_payload.vendor_data, &Payload.vendor_data, sizeof(event_payload.vendor_data));
    lock.unlock();
    if (app_cb)
        app_cb(mQcarcamHndl, event_id, &event_payload);
    return Void();
}

// Public methods
qcarcam_ret_t ais_hidl_client::ais_client_open_stream(qcarcam_input_desc_t desc)
{
    vendor::qti::automotive::qcarcam::V1_0::QcarcamError Error;

    mStreamId = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamInputDesc>(desc);
    if (mpService) {
        auto ret = mpService->openStream_1_1(mStreamId, [&](const auto& Stream, const auto& tmpError) {

                Error = tmpError;
                if (vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_OK == tmpError) {
                    AIS_HIDL_INFOMSG("openStream success for stream id %d", static_cast<int>(mStreamId));
                    mpStream = Stream;
                } else {
                    AIS_HIDL_ERRORMSG("openStream Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(tmpError));
                }
                } );
        if (!ret.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport error, openStream() failed for stream id %d", static_cast<int>(mStreamId));
            Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
            mpService = nullptr;
        }
    } else {
        AIS_HIDL_ERRORMSG("Service obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
    }

    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_hidl_client::ais_client_close_stream()
{
    vendor::qti::automotive::qcarcam::V1_0::QcarcamError Error;

    if (true == mStreaming)
        ais_client_stop_stream();

    if (mpStream && mpService) {
        auto ret = mpService->closeStream_1_1(mpStream);
        if (!ret.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport layer error, closeStream() failed for stream id %d", static_cast<int>(mStreamId));
            Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
        } else {
            Error = ret;
            if (vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_OK == Error) {
                AIS_HIDL_INFOMSG("closeStream success for stream id %d", static_cast<int>(mStreamId));
                mpStream = nullptr;
            }
            else
                AIS_HIDL_ERRORMSG("closeStream Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(Error));
        }
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
    }

    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_hidl_client::ais_client_s_param(qcarcam_param_t param, const qcarcam_param_value_t* p_value)
{
    vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam Param;
    vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamConfigData data = {};
    vendor::qti::automotive::qcarcam::V1_0::QcarcamError Error;

    // Convert input into hidl structure
    get_ais_hidl_params(param, p_value, Param, data);
    if (mpStream) {
        auto ret = mpStream->configureStream_1_1(Param, data);
        if (!ret.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport layer error, configureStream() failed for stream id %d", static_cast<int>(mStreamId));
            Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
        } else {
            Error = ret;
            if (vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_OK == Error)
                AIS_HIDL_INFOMSG("configureStream success for stream id %d", static_cast<int>(mStreamId));
            else
                AIS_HIDL_ERRORMSG("configureStream Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(Error));
        }
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
    }

    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_hidl_client::ais_client_s_cb(void *cb_ptr)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    // Store apps callback and return
    app_cb = (qcarcam_event_cb_t)cb_ptr;
    return ret;
}

qcarcam_ret_t ais_hidl_client::ais_client_s_buffer(qcarcam_buffers_t* p_buffers)
{
    vendor::qti::automotive::qcarcam::V1_0::QcarcamBuffersInfo BuffInfo;
    vendor::qti::automotive::qcarcam::V1_0::QcarcamError Error;
    // Fill BuffInfo with buffer data
    BuffInfo.color_fmt = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamColorFmt>(p_buffers->color_fmt);
    BuffInfo.n_planes = p_buffers->buffers[0].n_planes;
    for (int j=0; j<BuffInfo.n_planes; j++) {
        BuffInfo.planes[j].width = static_cast<uint32_t>(p_buffers->buffers[0].planes[j].width);
        BuffInfo.planes[j].height = static_cast<uint32_t>(p_buffers->buffers[0].planes[j].height);
        BuffInfo.planes[j].stride = static_cast<uint32_t>(p_buffers->buffers[0].planes[j].stride);
        BuffInfo.planes[j].size = static_cast<uint32_t>(p_buffers->buffers[0].planes[j].size);
    }
    //Fill buffer flag info
    BuffInfo.flags = static_cast<uint32_t>(p_buffers->flags);
    // Fill native handle with FD details
    NATIVE_HANDLE_DECLARE_STORAGE(dvrHandle, p_buffers->n_buffers, 0);
    mNativeHndl = native_handle_init(dvrHandle, p_buffers->n_buffers, 0);
    if (mNativeHndl == NULL) {
        AIS_HIDL_ERRORMSG("Native handle is null!! sending error status");
        Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
        return static_cast<qcarcam_ret_t>(Error);
    }
    mNativeHndl->numFds = p_buffers->n_buffers;
    for (unsigned int i=0; i<p_buffers->n_buffers; i++) {
        mNativeHndl->data[i] = (uint64_t)p_buffers->buffers[i].planes[0].p_buf;
    }

    if (mpStream) {
        auto ret = mpStream->setStreamBuffers(mNativeHndl, BuffInfo);
        if (!ret.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport layer error, setStreamBuffers() failed for stream id %d", static_cast<int>(mStreamId));
            Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
        } else {
            Error = ret;
            if (vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_OK == Error)
                AIS_HIDL_INFOMSG("setStreamBuffers success for stream id %d", static_cast<int>(mStreamId));
            else
                AIS_HIDL_ERRORMSG("setStreamBuffers Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(Error));
        }
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
    }

    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_hidl_client::ais_client_s_buffer_v2(qcarcam_bufferlist_t* p_buffers)
{
    vendor::qti::automotive::qcarcam::V1_1::QcarcamBuffersInfoList BuffListInfo;
    vendor::qti::automotive::qcarcam::V1_0::QcarcamError Error;
    // Fill BuffListInfo with buffer data
    BuffListInfo.id = static_cast<uint32_t>(p_buffers->id);
    BuffListInfo.n_buffers = static_cast<uint32_t>(p_buffers->n_buffers);
    for(int i=0;i<BuffListInfo.n_buffers && i<QCARCAMBUFFERSINFOV2_SIZE;i++)
    {
        BuffListInfo.BuffersInfo[i].color_fmt = static_cast<vendor::qti::automotive::qcarcam::V1_1::QcarcamColorFmt>(p_buffers->color_fmt);
        BuffListInfo.BuffersInfo[i].n_planes = p_buffers->buffers[i].n_planes;
         for (int j=0; j<BuffListInfo.BuffersInfo[i].n_planes; j++) {
         BuffListInfo.BuffersInfo[i].planes[j].width = static_cast<uint32_t>(p_buffers->buffers[i].planes[j].width);
         BuffListInfo.BuffersInfo[i].planes[j].height = static_cast<uint32_t>(p_buffers->buffers[i].planes[j].height);
         BuffListInfo.BuffersInfo[i].planes[j].stride = static_cast<uint32_t>(p_buffers->buffers[i].planes[j].stride);
         BuffListInfo.BuffersInfo[i].planes[j].size = static_cast<uint32_t>(p_buffers->buffers[i].planes[j].size);
        }
        BuffListInfo.BuffersInfo[i].flags = static_cast<uint32_t>(p_buffers->flags);
    }
    //Fill buffer flag info
    // BuffInfo.flags = static_cast<uint32_t>(p_buffers->flags);
    // Fill native handle with FD details
    NATIVE_HANDLE_DECLARE_STORAGE(dvrHandle, p_buffers->n_buffers, 0);
    mNativeHndl = native_handle_init(dvrHandle, p_buffers->n_buffers, 0);
    if (mNativeHndl == NULL) {
        AIS_HIDL_ERRORMSG("Native handle is null!! sending error status");
        Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
        return static_cast<qcarcam_ret_t>(Error);
    }
    mNativeHndl->numFds = p_buffers->n_buffers;
    for (unsigned int i=0; i<p_buffers->n_buffers; i++) {
        mNativeHndl->data[i] = (uint64_t)p_buffers->buffers[i].planes[0].hndl;
    }

    if (mpStream) {
        Error = mpStream->setStreamBuffers_1_1(mNativeHndl, BuffListInfo);
        if (vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_OK == Error)
            AIS_HIDL_INFOMSG("setStreamBuffers success for stream id %d", static_cast<int>(mStreamId));
        else
            AIS_HIDL_ERRORMSG("setStreamBuffers Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(Error));
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
    }

    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_hidl_client::ais_client_start_stream(qcarcam_hndl_t q_hndl)
{
    vendor::qti::automotive::qcarcam::V1_0::QcarcamError Error;

    //Save qcarcam hndl for callback
    mQcarcamHndl = q_hndl;

    // Call start stream with current class obj for callback
    if (mpStream) {
        auto ret = mpStream->startStream_1_1(this);
        if (!ret.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport layer error, startStream() failed for stream id %d", static_cast<int>(mStreamId));
            Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
            mStreaming = false;
        } else {
            Error = ret;
            if (vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_OK == Error) {
                AIS_HIDL_INFOMSG("startStream success for stream id %d", static_cast<int>(mStreamId));
                mStreaming = true;
            } else {
                AIS_HIDL_ERRORMSG("startStream Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(Error));
                mStreaming = false;
            }
        }
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
    }

    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_hidl_client::ais_client_stop_stream()
{
    vendor::qti::automotive::qcarcam::V1_0::QcarcamError Error;
    std::unique_lock<std::mutex> lock(mLock);

    if (mpStream) {
        auto ret = mpStream->stopStream_1_1();
        if (!ret.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport layer error, stopStream() failed for stream id %d",static_cast<int32_t>(mStreamId));
            Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
        } else {
            Error = ret;
            if (vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_OK == Error) {
                AIS_HIDL_INFOMSG("stopStream success for stream id %d", static_cast<int>(mStreamId));
                mStreaming = false;
            } else
                AIS_HIDL_ERRORMSG("stopStream Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(Error));
        }
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
    }
    lock.unlock();

    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_hidl_client::ais_client_get_frame(qcarcam_frame_info_t* p_frame_info,
        unsigned long long int timeout, unsigned int flags)
{
    vendor::qti::automotive::qcarcam::V1_0::QcarcamError Error;

    if (mpStream) {
        auto ret = mpStream->getFrame(timeout, flags, [&](const auto& tmpError, const auto& Info) {
                Error = tmpError;
                if (vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_OK == tmpError) {
                    AIS_HIDL_INFOMSG("getFrame success for stream id %d idx %u", static_cast<int>(mStreamId), Info.idx);
                // Copy the result into qcarcam structure from hidl cb
                p_frame_info->idx = Info.idx;
                p_frame_info->flags = Info.flags;
                p_frame_info->seq_no = Info.seq_no;
                p_frame_info->timestamp = Info.timestamp;
                p_frame_info->timestamp_system = Info.timestamp_system;
                p_frame_info->field_type = static_cast<qcarcam_field_t>(Info.field_type);
                } else {
                    AIS_HIDL_ERRORMSG("getFrame Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(tmpError));
                }
                } );
        if (!ret.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport layer error, getFrame() failed for stream id %d", static_cast<int32_t>(mStreamId));
            Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
        }
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
    }

    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_hidl_client::ais_client_get_frame_v2(qcarcam_frame_info_v2_t* p_frame_info,
        unsigned long long int timeout, unsigned int flags)
{
    vendor::qti::automotive::qcarcam::V1_0::QcarcamError Error;

    if (mpStream) {
        mpStream->getFrame_1_1(timeout, flags, [&](const auto& tmpError, const auto& Info) {
                Error = tmpError;
                if (vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_OK == tmpError) {
                AIS_HIDL_INFOMSG("getFrame success for stream id %d idx %u", static_cast<int>(mStreamId), Info.idx);
                // Copy the result into qcarcam structure from hidl cb
                p_frame_info->id = Info.id;
                p_frame_info->idx = Info.idx;
                p_frame_info->flags = Info.flags;
                for(unsigned int i = 0; i < QCARCAM_MAX_BATCH_FRAMES ; i++){
                   p_frame_info->seq_no[i] = Info.seq_no[i];
                   p_frame_info->sof_qtimestamp[i] = Info.sof_qtimestamp[i];
                }
                p_frame_info->timestamp = Info.timestamp;
                p_frame_info->timestamp_system = Info.timestamp_system;
                p_frame_info->field_type = static_cast<qcarcam_field_t>(Info.field_type);
                } else {
                AIS_HIDL_ERRORMSG("getFrame Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(tmpError));
                }
                } );
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
    }

    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_hidl_client::ais_client_release_frame(unsigned int idx)
{
    vendor::qti::automotive::qcarcam::V1_0::QcarcamError Error;

    if (mpStream) {
        auto ret = mpStream->releaseFrame(idx);
        if (!ret.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport layer error, releaseFrame() failed for stream id %d", static_cast<int32_t>(mStreamId));
            Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
        }
        else {
            Error = ret;
            if (vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_OK == Error)
                AIS_HIDL_INFOMSG("releaseFrame success for stream id %d idx %u", static_cast<int>(mStreamId), idx);
            else
                AIS_HIDL_ERRORMSG("releaseFrame Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(Error));
        }
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
    }

    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_hidl_client::ais_client_release_frame_v2(unsigned int id,unsigned int idx)
{
    vendor::qti::automotive::qcarcam::V1_0::QcarcamError Error;

    if (mpStream) {
        Error = mpStream->releaseFrame_1_1(id,idx);
        if (vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_OK == Error)
            AIS_HIDL_INFOMSG("releaseFrame success for stream id %d idx %u", static_cast<int>(mStreamId), idx);
        else
            AIS_HIDL_ERRORMSG("releaseFrame Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(Error));
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
    }

    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_hidl_client::ais_client_pause_stream()
{
    vendor::qti::automotive::qcarcam::V1_0::QcarcamError Error;

    if (mpStream) {
        auto ret = mpStream->pauseStream();
        if (!ret.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport layer error, pauseStream() failed for stream id %d", static_cast<int32_t>(mStreamId));
            Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
        }
        else {
            Error = ret;
            if (vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_OK == Error) {
                AIS_HIDL_INFOMSG("pauseStream success for stream id %d", static_cast<int>(mStreamId));
                mStreaming = false;
            } else
                AIS_HIDL_ERRORMSG("pauseStream Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(Error));
        }
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
    }
    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_hidl_client::ais_client_resume_stream()
{
    vendor::qti::automotive::qcarcam::V1_0::QcarcamError Error;

    if (mpStream) {
        auto ret = mpStream->resumeStream();
        if (!ret.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport layer error, resumeStream() failed for stream id %d", static_cast<int32_t>(mStreamId));
            Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
        }
        else {
            Error = ret;
            if (vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_OK == Error) {
                AIS_HIDL_INFOMSG("resumeStream success for stream id %d", static_cast<int>(mStreamId));
                mStreaming = true;
            } else
                AIS_HIDL_ERRORMSG("resumeStream Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(Error));
        }
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
    }
    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_hidl_client::ais_client_g_param(qcarcam_param_t param, qcarcam_param_value_t* p_value)
{

    vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam Param;
    vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamConfigData data = {};
    vendor::qti::automotive::qcarcam::V1_0::QcarcamError Error;

    // Convert param into hidl type
    get_ais_hidl_params(param, p_value, Param, data);

    if (mpStream) {
        auto ret = mpStream->getStreamConfig_1_1(Param, data, [&](const auto& tmpError, const auto& StreamData) {
                Error = tmpError;
                if (vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_OK == tmpError) {
                AIS_HIDL_INFOMSG("getStreamConfig success for stream id %d", static_cast<int>(mStreamId));
                // Copy the result into qcarcam structure from hidl cb
                get_qcarcam_params(Param, StreamData, &param, p_value);
                } else {
                AIS_HIDL_ERRORMSG("getStreamConfig Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(tmpError));
                }
                } );
        if (!ret.isOk()) {
            AIS_HIDL_ERRORMSG("Hidl transport error; getStreamConfig() failed for stream id %d",static_cast<int32_t>(mStreamId));
            Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
        }
    } else {
        AIS_HIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = vendor::qti::automotive::qcarcam::V1_0::QcarcamError::QCARCAM_FAILED;
    }

    return static_cast<qcarcam_ret_t>(Error);
}


// Utility Functions
void ais_hidl_client::get_ais_hidl_params(qcarcam_param_t param_type, const qcarcam_param_value_t* param_value,
        vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam& Param,
        vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamConfigData& data)
{
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
                    data.frame_rate_config.frame_drop_mode = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamFrameDropMode>(param_value->frame_rate_config.frame_drop_mode);
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
                    data.exposure_config.exposure_mode_type = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamExposureMode>(param_value->exposure_config.exposure_mode_type);
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
                    data.hdr_exposure_config.exposure_mode_type = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamExposureMode>(param_value->hdr_exposure_config.exposure_mode_type);
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
                     else if(param_value->gamma_config.config_type == QCARCAM_GAMMA_KNEEPOINTS) {
                         data.gamma_config.gamma.table.length = static_cast<uint32_t>(param_value->gamma_config.gamma.table.length);
                        memcpy(&data.gamma_config.gamma.table.arr_gamma,param_value->gamma_config.gamma.table.p_value,data.gamma_config.gamma.table.length*sizeof(unsigned int));
                     }
                    data.gamma_config.config_type = static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamGammaType>(param_value->gamma_config.config_type);
                }
            }
            break;
        case QCARCAM_PARAM_OPMODE:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_OPMODE;
                if (param_value)
                    data.uint_value = static_cast<uint32_t>(param_value->uint_value);
            }
            break;
        case QCARCAM_PARAM_RECOVERY:
            {
                Param = vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_RECOVERY;
                if (param_value)
                    data.uint_value = static_cast<uint32_t>(param_value->uint_value);
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
        default:
            AIS_HIDL_ERRORMSG("Invalid config param %d", param_type);
            break;
    }
}

void ais_hidl_client::get_qcarcam_params(vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam Param,
        const vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamConfigData& data,
        qcarcam_param_t* param_type,
        qcarcam_param_value_t* param_value)
{
    switch (Param)
    {
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_CB_EVENT_MASK:
            {
                *param_type = QCARCAM_PARAM_EVENT_MASK;
                param_value->uint_value = static_cast<unsigned int>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_COLOR_FMT:
            {
                *param_type = QCARCAM_PARAM_COLOR_FMT;
                param_value->color_value = static_cast<qcarcam_color_fmt_t>(data.color_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_RESOLUTION:
            {
                *param_type = QCARCAM_PARAM_RESOLUTION;
                param_value->res_value.width = static_cast<unsigned int>(data.res_value.width);
                param_value->res_value.height = static_cast<unsigned int>(data.res_value.height);
                param_value->res_value.fps = static_cast<float>(data.res_value.fps);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_BRIGHTNESS:
            {
                *param_type = QCARCAM_PARAM_BRIGHTNESS;
                param_value->float_value = static_cast<float>(data.float_value);
            }
            break;
        case  vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_VENDOR:
            {
                *param_type = QCARCAM_PARAM_VENDOR;
                 for(unsigned int i=0; i < QCARCAM_MAX_VENDOR_PAYLOAD_SIZE; i++){
                     param_value->vendor_param.data[i] = static_cast<uint32_t>(data.vendor_param.data[i]);
                 }
            }
            break;
        case  vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_INPUT_MODE:
            {
                *param_type = QCARCAM_PARAM_INPUT_MODE;
                param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
            }
            break;
        case  vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_MASTER:
            {
                *param_type = QCARCAM_PARAM_MASTER;
                param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
            }
            break;
        case  vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_EVENT_CHANGE_SUBSCRIBE:
            {
                *param_type = QCARCAM_PARAM_EVENT_CHANGE_SUBSCRIBE;
                param_value->uint64_value =  static_cast<uint64_t>(data.uint64_value);
            }
            break;
        case  vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_EVENT_CHANGE_UNSUBSCRIBE:
            {
                *param_type = QCARCAM_PARAM_EVENT_CHANGE_UNSUBSCRIBE;
                param_value->uint64_value =  static_cast<uint64_t>(data.uint64_value);
            }
            break;
        case  vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_BATCH_MODE:
            {
                *param_type = QCARCAM_PARAM_BATCH_MODE;
                 param_value->batch_config.num_batch_frames = static_cast<uint32_t>(data.batch_config.num_batch_frames);
            }
            break;
        case  vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_ISP_USECASE:
            {
                *param_type = QCARCAM_PARAM_ISP_USECASE;
                param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
                param_value->isp_config.id = data.isp_config.id;
                param_value->isp_config.camera_id = data.isp_config.camera_id;
                param_value->isp_config.use_case = static_cast<qcarcam_isp_usecase_t>(data.isp_config.use_case);
            }
            break;
        case  vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_OPMODE:
            {
                *param_type = QCARCAM_PARAM_OPMODE;
                param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
            }
            break;
        case  vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_RECOVERY:
            {
                *param_type = QCARCAM_PARAM_RECOVERY;
                param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_CONTRAST:
            {
                *param_type = QCARCAM_PARAM_CONTRAST;
                 param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_MIRROR_H:
            {
                *param_type = QCARCAM_PARAM_MIRROR_H;
                param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_MIRROR_V:
            {
                *param_type = QCARCAM_PARAM_MIRROR_V;
                param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_FRAME_RATE:
            {
                *param_type = QCARCAM_PARAM_FRAME_RATE;
                param_value->frame_rate_config.frame_drop_mode = static_cast<qcarcam_frame_drop_mode_t>(data.frame_rate_config.frame_drop_mode);
                param_value->frame_rate_config.frame_drop_period = static_cast<unsigned char>(data.frame_rate_config.frame_drop_period);
                param_value->frame_rate_config.frame_drop_pattern = static_cast<unsigned int>(data.frame_rate_config.frame_drop_pattern);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_VID_STD:
            {
                *param_type = QCARCAM_PARAM_VID_STD;
                param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_CURRENT_VID_STD:
            {
                *param_type = QCARCAM_PARAM_CURRENT_VID_STD;
                param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_STATUS:
            {
                *param_type = QCARCAM_PARAM_STATUS;
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_LATENCY_MAX:
            {
                *param_type = QCARCAM_PARAM_LATENCY_MAX;
                param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_LATENCY_REDUCE_RATE:
            {
                *param_type = QCARCAM_PARAM_LATENCY_REDUCE_RATE;
                param_value->uint_value =  static_cast<uint32_t>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_PRIVATE_DATA:
            {
                *param_type = QCARCAM_PARAM_PRIVATE_DATA;
                param_value->ptr_value = (void *)data.ptr_value;
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_INJECTION_START:
            {
                *param_type = QCARCAM_PARAM_INJECTION_START;
                param_value->uint_value = static_cast<unsigned int>(data.uint_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_EXPOSURE:
            {
                *param_type = QCARCAM_PARAM_EXPOSURE;
                param_value->exposure_config.exposure_mode_type = static_cast<qcarcam_exposure_mode_t>(data.exposure_config.exposure_mode_type);
                param_value->exposure_config.exposure_time = static_cast<float>(data.exposure_config.exposure_time);
                param_value->exposure_config.gain = static_cast<float>(data.exposure_config.gain);
                param_value->exposure_config.target = static_cast<float>(data.exposure_config.target);
                param_value->exposure_config.lux_index = static_cast<float>(data.exposure_config.lux_index);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_HDR_EXPOSURE:
            {
                *param_type = QCARCAM_PARAM_HDR_EXPOSURE;
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
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_HUE:
            {
                *param_type = QCARCAM_PARAM_HUE;
                param_value->float_value = static_cast<float>(data.float_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_SATURATION:
            {
                *param_type = QCARCAM_PARAM_SATURATION;
                param_value->float_value = static_cast<float>(data.float_value);
            }
            break;
        case vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam::QCARCAM_GAMMA:
            {
                *param_type = QCARCAM_PARAM_GAMMA;
                if (data.gamma_config.config_type == static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamGammaType>(QCARCAM_GAMMA_EXPONENT)) {
                    param_value->gamma_config.gamma.f_value = static_cast<float>(data.gamma_config.gamma.f_value);
                }
                else if (data.gamma_config.config_type == static_cast<vendor::qti::automotive::qcarcam::V1_0::QcarcamGammaType>(QCARCAM_GAMMA_KNEEPOINTS)) {
                    param_value->gamma_config.gamma.table.length = static_cast<uint32_t>(data.gamma_config.gamma.table.length);
                    if (param_value->gamma_config.gamma.table.length < 0 || param_value->gamma_config.gamma.table.length > QCARCAM_MAX_GAMMA_TABLE)
                    {
                        AIS_HIDL_ERRORMSG("Invalid length");
                        break;
                    }
                    for (int i=0; i<param_value->gamma_config.gamma.table.length; i++)
                        param_value->gamma_config.gamma.table.p_value[i] = static_cast<uint32_t>(data.gamma_config.gamma.table.arr_gamma[i]);
                }
                param_value->gamma_config.config_type = static_cast<qcarcam_gamma_type_t>(data.gamma_config.config_type);
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
        default: AIS_HIDL_ERRORMSG("Invalid config param");
    }
}
