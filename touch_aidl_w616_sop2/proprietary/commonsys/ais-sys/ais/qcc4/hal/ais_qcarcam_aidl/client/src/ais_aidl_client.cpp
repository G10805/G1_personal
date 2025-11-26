/* ===========================================================================
 * Copyright (c) 2019-2020, 2022, 2024 Qualcomm Technologies, Inc.
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
#include "ais_aidl_client.h"

#define CHECK_BIT(num, pos) ((num) & (0x1<<(pos)))
#define QCARCAMBUFFERSINFOV2_SIZE 20

#define UNUSED_VAR(x) (void)x;

using namespace std;

using ::memcpy;

// Default constructors
ais_aidl_client::ais_aidl_client(std::shared_ptr<IQcarCamera> service): mpService(service)
{
    mInitialized = false;
    mStreaming = false;
}
ais_aidl_client::~ais_aidl_client()
{
    AIS_AIDL_DBGMSG("Destructor called for ais_aidl_client hndl %p",mQcarcamHndl);
    mInitialized = false;
    mStreaming = false;
}

// Override callback method
// qcarcam_event_callback_1_1 API is deprecated.
::ndk::ScopedAStatus ais_aidl_client::qcarcam_event_callback_1_1(QcarcamEvent EventType,
        const QcarcamEventPayload& Payload)
{
    UNUSED_VAR(EventType);
    UNUSED_VAR(Payload);
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

::ndk::ScopedAStatus ais_aidl_client::qcarcam_event_callback(QcarcamEvent EventType,
        const QcarcamEventPayload& Payload)
{
    std::unique_lock<std::mutex> lock(mLock);
    AIS_AIDL_DBGMSG("Received qcarcam_event_callback");
    qcarcam_event_t event_id;
    qcarcam_event_payload_t event_payload;
    event_id = static_cast<qcarcam_event_t>(EventType);
    memcpy(&event_payload.vendor_data, &Payload.vendor_data, sizeof(event_payload.vendor_data));
    lock.unlock();
    if (app_cb)
        app_cb(mQcarcamHndl, event_id, &event_payload);
    return ndk::ScopedAStatus::ok();
}

// Public methods
qcarcam_ret_t ais_aidl_client::ais_client_open_stream(qcarcam_input_desc_t desc)
{
    QcarcamError Error;

    mStreamId = static_cast<QcarcamInputDesc>(desc);
    if (mpService) {
        auto ret = mpService->openStream(mStreamId, &mpStream);
        if (!ret.isOk()) {
            AIS_AIDL_ERRORMSG("openStream Failed for stream id %d", static_cast<int32_t>(mStreamId));
            Error = QcarcamError::QCARCAM_FAILED;
            mpService = nullptr;
            mpStream.reset();
        } else {
            AIS_AIDL_INFOMSG("openStream success for stream id %d", static_cast<int>(mStreamId));
            Error = QcarcamError::QCARCAM_OK;
        }
    } else {
        AIS_AIDL_ERRORMSG("Service obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = QcarcamError::QCARCAM_FAILED;
    }

    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_aidl_client::ais_client_close_stream()
{
    QcarcamError Error;

    if (true == mStreaming)
        ais_client_stop_stream();

    if (mpStream && mpService) {
        auto ret = mpService->closeStream(mpStream, &Error);
        if (!ret.isOk()) {
            AIS_AIDL_ERRORMSG("aidl transport layer error, closeStream() failed for stream id %d", static_cast<int>(mStreamId));
        } else {
            if (QcarcamError::QCARCAM_OK == Error) {
                AIS_AIDL_INFOMSG("closeStream success for stream id %d", static_cast<int>(mStreamId));
                mpStream.reset();
            }
            else
                AIS_AIDL_ERRORMSG("closeStream Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(Error));
        }
    } else {
        AIS_AIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = QcarcamError::QCARCAM_FAILED;
    }

    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_aidl_client::ais_client_s_param(qcarcam_param_t param, const qcarcam_param_value_t* p_value)
{
    QcarcamStreamParam Param;
    QcarcamStreamConfigData data = {};
    QcarcamError Error;

    // Convert input into aidl structure
    get_ais_aidl_params(param, p_value, Param, data);
    if (mpStream) {
        auto ret = mpStream->configureStream(Param, data, &Error);
        if (!ret.isOk()) {
            AIS_AIDL_ERRORMSG("aidl transport layer error, ais_client_s_param() failed for stream id %d", static_cast<int>(mStreamId));
        } else {
            if (QcarcamError::QCARCAM_OK == Error)
                AIS_AIDL_INFOMSG("ais_client_s_param success for stream id %d", static_cast<int>(mStreamId));
            else
                AIS_AIDL_ERRORMSG("ais_client_s_param Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(Error));
        }
    } else {
        AIS_AIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = QcarcamError::QCARCAM_FAILED;
    }

    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_aidl_client::ais_client_s_cb(void *cb_ptr)
{
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    // Store apps callback and return
    app_cb = (qcarcam_event_cb_t)cb_ptr;
    return ret;
}

qcarcam_ret_t ais_aidl_client::ais_client_s_buffer_v2(qcarcam_bufferlist_t* p_buffers)
{
    QcarcamBuffersInfoList BuffListInfo;
    QcarcamError Error;
    // Fill BuffListInfo with buffer data
    BuffListInfo.id = static_cast<uint32_t>(p_buffers->id);
    BuffListInfo.n_buffers = static_cast<uint32_t>(p_buffers->n_buffers);
    for(int i = 0; i < BuffListInfo.n_buffers && i < QCARCAMBUFFERSINFOV2_SIZE; i++)
    {
        BuffListInfo.BuffersInfo[i].color_fmt = static_cast<QcarcamColorFmt>(p_buffers->color_fmt);
        BuffListInfo.BuffersInfo[i].n_planes = p_buffers->buffers[i].n_planes;
         for (int j = 0; j < BuffListInfo.BuffersInfo[i].n_planes; j++) {
         BuffListInfo.BuffersInfo[i].planes[j].width = static_cast<uint32_t>(p_buffers->buffers[i].planes[j].width);
         BuffListInfo.BuffersInfo[i].planes[j].height = static_cast<uint32_t>(p_buffers->buffers[i].planes[j].height);
         BuffListInfo.BuffersInfo[i].planes[j].stride = static_cast<uint32_t>(p_buffers->buffers[i].planes[j].stride);
         BuffListInfo.BuffersInfo[i].planes[j].size = static_cast<uint32_t>(p_buffers->buffers[i].planes[j].size);
         BuffListInfo.BuffersInfo[i].planes[j].offset = static_cast<uint32_t>(p_buffers->buffers[i].planes[j].offset);
         BuffListInfo.BuffersInfo[i].planes[j].hndl = static_cast<uint64_t>(p_buffers->buffers[i].planes[j].hndl);
        }
        BuffListInfo.BuffersInfo[i].flags = static_cast<uint32_t>(p_buffers->flags);
    }
    // Fill native handle with FD details
    NATIVE_HANDLE_DECLARE_STORAGE(dvrHandle, p_buffers->n_buffers, 0);
    mNativeHndl = native_handle_init(dvrHandle, p_buffers->n_buffers, 0);
    if (mNativeHndl == NULL) {
        AIS_AIDL_ERRORMSG("Native handle is null!! sending error status");
        Error = QcarcamError::QCARCAM_FAILED;
        return static_cast<qcarcam_ret_t>(Error);
    }
    mNativeHndl->numFds = p_buffers->n_buffers;
    for (unsigned int i = 0; i < p_buffers->n_buffers; i++) {
        for(unsigned int j = 0; j < p_buffers->buffers[i].n_planes; j++)
        mNativeHndl->data[i] = (uint64_t)p_buffers->buffers[i].planes[j].hndl;
    }

    if (mpStream) {
        ::aidl::android::hardware::common::NativeHandle aNativeHndl = ::android::dupToAidl(mNativeHndl);
        auto ret = mpStream->setStreamBuffers(aNativeHndl, BuffListInfo, &Error);
        if (!ret.isOk()) {
            AIS_AIDL_ERRORMSG("aidl transport layer error, setStreamBuffers() failed for stream id %d", static_cast<int>(mStreamId));
        } else {
            if (QcarcamError::QCARCAM_OK == Error)
                AIS_AIDL_INFOMSG("setStreamBuffers success for stream id %d", static_cast<int>(mStreamId));
            else
                AIS_AIDL_ERRORMSG("setStreamBuffers Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(Error));
        }
    } else {
        AIS_AIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = QcarcamError::QCARCAM_FAILED;
    }

    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_aidl_client::ais_client_start_stream(qcarcam_hndl_t q_hndl)
{
    QcarcamError Error;

    //Save qcarcam hndl for callback
    mQcarcamHndl = q_hndl;

    // Call start stream with current class obj for callback
    if (mpStream) {
        auto ret = mpStream->startStream(ref<ais_aidl_client>(), &Error);
        if (!ret.isOk()) {
            AIS_AIDL_ERRORMSG("aidl transport layer error, startStream() failed for stream id %d", static_cast<int>(mStreamId));
            mStreaming = false;
        } else {
            if (QcarcamError::QCARCAM_OK == Error) {
                AIS_AIDL_INFOMSG("startStream success for stream id %d", static_cast<int>(mStreamId));
                mStreaming = true;
            } else {
                AIS_AIDL_ERRORMSG("startStream Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(Error));
                mStreaming = false;
            }
        }
    } else {
        AIS_AIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = QcarcamError::QCARCAM_FAILED;
    }

    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_aidl_client::ais_client_stop_stream()
{
    QcarcamError Error;
    std::unique_lock<std::mutex> lock(mLock);

    if (mpStream) {
        auto ret = mpStream->stopStream(&Error);
        if (!ret.isOk()) {
            AIS_AIDL_ERRORMSG("aidl transport layer error, stopStream() failed for stream id %d",static_cast<int32_t>(mStreamId));
        } else {
            if (QcarcamError::QCARCAM_OK == Error) {
                AIS_AIDL_INFOMSG("stopStream success for stream id %d", static_cast<int>(mStreamId));
                mStreaming = false;
            } else
                AIS_AIDL_ERRORMSG("stopStream Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(Error));
        }
    } else {
        AIS_AIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = QcarcamError::QCARCAM_FAILED;
    }
    lock.unlock();

    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_aidl_client::ais_client_get_frame_v2(qcarcam_frame_info_v2_t* p_frame_info,
        unsigned long long int timeout, unsigned int flags)
{
    QcarcamError Error;
    QcarcamFrameInfov2 Info;

    if (mpStream) {
        auto ret = mpStream->getFrame(timeout, flags, &Info, &Error);
        if (!ret.isOk()) {
            AIS_AIDL_ERRORMSG("aidl transport layer error, getFrame() failed for stream id %d",static_cast<int32_t>(mStreamId));
        } else {
            if (QcarcamError::QCARCAM_OK == Error) {
                AIS_AIDL_INFOMSG("getFrame success for stream id %d idx %u", static_cast<int>(mStreamId), Info.idx);
                // Copy the result into qcarcam structure from aidl cb
                p_frame_info->id = Info.id;
                p_frame_info->idx = Info.idx;
                p_frame_info->flags = Info.flags;
                for(unsigned int i=0; i < QCARCAM_MAX_BATCH_FRAMES ; i++) {
                   p_frame_info->seq_no[i] = Info.seq_no[i];
                }
                p_frame_info->timestamp = Info.timestamp;
                p_frame_info->timestamp_system = Info.timestamp_system;
                p_frame_info->field_type = static_cast<qcarcam_field_t>(Info.field_type);
            } else {
                AIS_AIDL_ERRORMSG("getFrame Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(Error));
            }
        }
    } else {
        AIS_AIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = QcarcamError::QCARCAM_FAILED;
    }

    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_aidl_client::ais_client_release_frame_v2(unsigned int id,unsigned int idx)
{
    QcarcamError Error;

    if (mpStream) {
        auto ret = mpStream->releaseFrame(id, idx, &Error);
        if (!ret.isOk()) {
            AIS_AIDL_ERRORMSG("aidl transport layer error, releaseFrame() failed for stream id %d",static_cast<int32_t>(mStreamId));
        } else {
            if (QcarcamError::QCARCAM_OK == Error)
                AIS_AIDL_INFOMSG("releaseFrame success for stream id %d idx %u", static_cast<int>(mStreamId), idx);
            else
                AIS_AIDL_ERRORMSG("releaseFrame Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(Error));
        }
    } else {
        AIS_AIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = QcarcamError::QCARCAM_FAILED;
    }
    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_aidl_client::ais_client_pause_stream()
{
    QcarcamError Error;

    if (mpStream) {
        auto ret = mpStream->pauseStream(&Error);
        if (!ret.isOk()) {
            AIS_AIDL_ERRORMSG("aidl transport layer error, pauseStream() failed for stream id %d", static_cast<int32_t>(mStreamId));
        }
        else {
            if (QcarcamError::QCARCAM_OK == Error) {
                AIS_AIDL_INFOMSG("pauseStream success for stream id %d", static_cast<int>(mStreamId));
                mStreaming = false;
            } else
                AIS_AIDL_ERRORMSG("pauseStream Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(Error));
        }
    } else {
        AIS_AIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = QcarcamError::QCARCAM_FAILED;
    }
    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_aidl_client::ais_client_resume_stream()
{
    QcarcamError Error;

    if (mpStream) {
        auto ret = mpStream->resumeStream(&Error);
        if (!ret.isOk()) {
            AIS_AIDL_ERRORMSG("aidl transport layer error, resumeStream() failed for stream id %d", static_cast<int32_t>(mStreamId));
        }
        else {
            if (QcarcamError::QCARCAM_OK == Error) {
                AIS_AIDL_INFOMSG("resumeStream success for stream id %d", static_cast<int>(mStreamId));
                mStreaming = true;
            } else
                AIS_AIDL_ERRORMSG("resumeStream Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(Error));
        }
    } else {
        AIS_AIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = QcarcamError::QCARCAM_FAILED;
    }
    return static_cast<qcarcam_ret_t>(Error);
}

qcarcam_ret_t ais_aidl_client::ais_client_g_param(qcarcam_param_t param, qcarcam_param_value_t* p_value)
{

    QcarcamStreamParam Param;
    QcarcamStreamConfigData data = {};
    QcarcamError Error;

    // Convert param into aidl type
    get_ais_aidl_params(param, p_value, Param, data);

    if (mpStream) {
        QcarcamStreamConfigData cb_data;
        auto ret = mpStream->getStreamConfig(Param, data, &cb_data, &Error);
        if (!ret.isOk()) {
            AIS_AIDL_ERRORMSG("aidl transport layer error, ais_client_g_param() failed for stream id %d", static_cast<int32_t>(mStreamId));
        } else {
                if (QcarcamError::QCARCAM_OK == Error) {
                    AIS_AIDL_INFOMSG("getStreamConfig success for stream id %d", static_cast<int>(mStreamId));
                    // Copy the result into qcarcam structure from aidl cb
                    get_qcarcam_params(Param, cb_data, &param, p_value);
                } else {
                    AIS_AIDL_ERRORMSG("getStreamConfig Failed for stream id %d with Error %d", static_cast<int32_t>(mStreamId), static_cast<int32_t>(Error));
                }
        }
    } else {
        AIS_AIDL_ERRORMSG("Stream obj already destroyed for stream id %d",static_cast<int32_t>(mStreamId));
        Error = QcarcamError::QCARCAM_FAILED;
    }

    return static_cast<qcarcam_ret_t>(Error);
}


// Utility Functions
void ais_aidl_client::get_ais_aidl_params(qcarcam_param_t param_type, const qcarcam_param_value_t* param_value,
        QcarcamStreamParam& Param,
        QcarcamStreamConfigData& data)
{
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
                     for(unsigned int i = 0; i < QCARCAM_MAX_VENDOR_PAYLOAD_SIZE; i++){
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
                    QcarcamBatchModeConfig tmp_batch_config;
                    tmp_batch_config.num_batch_frames = static_cast<uint32_t>(param_value->batch_config.num_batch_frames);
                    data.set<QcarcamStreamConfigData::batch_config>(tmp_batch_config);
                }
            }
            break;
        case QCARCAM_PARAM_ISP_USECASE:
            {
                Param = QcarcamStreamParam::QCARCAM_ISP_USECASE;
                if(param_value){
                    QcarcamIspUsecaseConfig tmp_isp_config;
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
                        memcpy(&data.get<QcarcamStreamConfigData::gamma_config>().gamma.get<QcarcamGamma::table>().arr_gamma,
                            param_value->gamma_config.gamma.table.p_value, param_value->gamma_config.gamma.table.length * sizeof(unsigned int));
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
    }
}

void ais_aidl_client::get_qcarcam_params(QcarcamStreamParam Param,
        const QcarcamStreamConfigData& data,
        qcarcam_param_t* param_type,
        qcarcam_param_value_t* param_value)
{
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
        case  QcarcamStreamParam::QCARCAM_VENDOR:
            {
                *param_type = QCARCAM_PARAM_VENDOR;
                if(param_value) {
                   for(unsigned int i=0; i < QCARCAM_MAX_VENDOR_PAYLOAD_SIZE; i++){
                     param_value->vendor_param.data[i] = static_cast<uint32_t>(data.get<QcarcamStreamConfigData::vendor_param>().data[i]);
                  }
                }
            }
            break;
        case  QcarcamStreamParam::QCARCAM_INPUT_MODE:
            {
                *param_type = QCARCAM_PARAM_INPUT_MODE;
                if(param_value)
                 param_value->uint_value =  static_cast<uint32_t>(data.get<QcarcamStreamConfigData::uint_value>());
            }
            break;
        case  QcarcamStreamParam::QCARCAM_MASTER:
            {
                *param_type = QCARCAM_PARAM_MASTER;
                if(param_value)
                 param_value->uint_value =  static_cast<uint32_t>(data.get<QcarcamStreamConfigData::uint_value>());
            }
            break;
        case  QcarcamStreamParam::QCARCAM_EVENT_CHANGE_SUBSCRIBE:
            {
                *param_type = QCARCAM_PARAM_EVENT_CHANGE_SUBSCRIBE;
                if(param_value)
                 param_value->uint64_value =  static_cast<uint64_t>(data.get<QcarcamStreamConfigData::uint64_value>());
            }
            break;
        case  QcarcamStreamParam::QCARCAM_EVENT_CHANGE_UNSUBSCRIBE:
            {
                *param_type = QCARCAM_PARAM_EVENT_CHANGE_UNSUBSCRIBE;
                if(param_value)
                 param_value->uint64_value =  static_cast<uint64_t>(data.get<QcarcamStreamConfigData::uint64_value>());
            }
            break;
        case  QcarcamStreamParam::QCARCAM_BATCH_MODE:
            {
                *param_type = QCARCAM_PARAM_BATCH_MODE;
                if(param_value)
                param_value->batch_config.num_batch_frames = static_cast<uint32_t>(data.get<QcarcamStreamConfigData::batch_config>().num_batch_frames);
            }
            break;
        case  QcarcamStreamParam::QCARCAM_ISP_USECASE:
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
                if (data.get<QcarcamStreamConfigData::gamma_config>().config_type == static_cast<QcarcamGammaType>(QCARCAM_GAMMA_EXPONENT)) {
                    param_value->gamma_config.gamma.f_value = static_cast<float>(data.get<QcarcamStreamConfigData::gamma_config>().gamma.f_value);
                }
                else if (data.get<QcarcamStreamConfigData::gamma_config>().config_type == static_cast<QcarcamGammaType>(QCARCAM_GAMMA_KNEEPOINTS)) {
                    param_value->gamma_config.gamma.table.length = static_cast<uint32_t>(data.get<QcarcamStreamConfigData::gamma_config>().gamma.get<QcarcamGamma::table>().length);
                    if (param_value->gamma_config.gamma.table.length < 0 || param_value->gamma_config.gamma.table.length > QCARCAM_MAX_GAMMA_TABLE)
                    {
                        AIS_AIDL_ERRORMSG("Invalid length");
                        break;
                    }
                    for (int i=0; i<param_value->gamma_config.gamma.table.length; i++)
                        param_value->gamma_config.gamma.table.p_value[i] = static_cast<uint32_t>(data.get<QcarcamStreamConfigData::gamma_config>().gamma.get<QcarcamGamma::table>().arr_gamma[i]);
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
        default: AIS_AIDL_ERRORMSG("Invalid config param");
    }
}

