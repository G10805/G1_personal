/* ===========================================================================
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */


#include <stdio.h>

#include <vendor/qti/automotive/qcarcam/1.1/types.h>
#include <vendor/qti/automotive/qcarcam/1.1/IQcarCamera.h>
#include <vendor/qti/automotive/qcarcam/1.1/IQcarCameraStream.h>
#include <vendor/qti/automotive/qcarcam/1.1/IQcarCameraStreamCB.h>
#include <cutils/native_handle.h>

#include "qcarcam_types.h"
#include "qcarcam.h"

//log
#include "ais_log.h"
#define AIS_HIDL_ERRORMSG(_fmt_, ...) \
    ais_log(AIS_MOD_ID_HIDL_SERVICE, AIS_LOG_LVL_ERROR,  "AIS_HIDL_CLIENT %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_ERROR], ##__VA_ARGS__)

#define AIS_HIDL_DBGMSG(_fmt_, ...) \
    ais_log(AIS_MOD_ID_HIDL_SERVICE, AIS_LOG_LVL_DBG,  "AIS_HIDL_CLIENT %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_DBG], ##__VA_ARGS__)

#define AIS_HIDL_INFOMSG(_fmt_, ...) \
    ais_log(AIS_MOD_ID_HIDL_SERVICE, AIS_LOG_LVL_MED,  "AIS_HIDL_CLIENT %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_MED], ##__VA_ARGS__)

using vendor::qti::automotive::qcarcam::V1_1::IQcarCamera;
using vendor::qti::automotive::qcarcam::V1_1::IQcarCameraStream;
using vendor::qti::automotive::qcarcam::V1_1::IQcarCameraStreamCB;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::android::hardware::hidl_vec;

class ais_hidl_client : public IQcarCameraStreamCB
{
public:
    ais_hidl_client(android::sp<vendor::qti::automotive::qcarcam::V1_1::IQcarCamera>service);
    ~ais_hidl_client();
    qcarcam_ret_t ais_client_open_stream(qcarcam_input_desc_t desc);
    qcarcam_ret_t ais_client_close_stream();
    qcarcam_ret_t ais_client_s_param(qcarcam_param_t param, const qcarcam_param_value_t* p_value);
    qcarcam_ret_t ais_client_s_cb(void *cb_ptr);
    qcarcam_ret_t ais_client_s_buffer(qcarcam_buffers_t* p_buffers);
    qcarcam_ret_t ais_client_s_buffer_v2(qcarcam_bufferlist_t* p_buffers);
    qcarcam_ret_t ais_client_start_stream(qcarcam_hndl_t q_hndl);
    qcarcam_ret_t ais_client_stop_stream();
    qcarcam_ret_t ais_client_get_frame(qcarcam_frame_info_t* p_frame_info,
        unsigned long long int timeout, unsigned int flags);
    qcarcam_ret_t ais_client_get_frame_v2(qcarcam_frame_info_v2_t* p_frame_info,
        unsigned long long int timeout, unsigned int flags);
    qcarcam_ret_t ais_client_release_frame(unsigned int idx);
    qcarcam_ret_t ais_client_release_frame_v2(unsigned int id,unsigned int idx);
    qcarcam_ret_t ais_client_pause_stream();
    qcarcam_ret_t ais_client_resume_stream();
    qcarcam_ret_t ais_client_g_param(qcarcam_param_t param, qcarcam_param_value_t* p_value);

    //App callback pointer
    qcarcam_event_cb_t app_cb;

    std::atomic<bool> mInitialized;

private:
    std::mutex mLock;
    qcarcam_hndl_t mQcarcamHndl;
    struct native_handle* mNativeHndl;
    android::sp <IQcarCamera> mpService;
    android::sp <IQcarCameraStream> mpStream;
    vendor::qti::automotive::qcarcam::V1_0::QcarcamInputDesc mStreamId;
    std::atomic<bool> mStreaming;
    void get_ais_hidl_params(qcarcam_param_t param_type, const qcarcam_param_value_t* param_value,
        vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam& Param,
        vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamConfigData& data);
    void get_qcarcam_params(vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam Param,
        const vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamConfigData& data,
        qcarcam_param_t* param_type,
        qcarcam_param_value_t* param_value);

    // Callback function defined in IQcarCameraStreamCB
    Return<void> qcarcam_event_callback(vendor::qti::automotive::qcarcam::V1_0::QcarcamEvent EventType,
            const vendor::qti::automotive::qcarcam::V1_0::QcarcamEventPayload& Payload) override;

    // Callback function defined in IQcarCameraStreamCB
    Return<void> qcarcam_event_callback_1_1(vendor::qti::automotive::qcarcam::V1_1::QcarcamEvent EventType,
            const vendor::qti::automotive::qcarcam::V1_1::QcarcamEventPayload& Payload) override;
};
