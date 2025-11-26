/* ===========================================================================
 * Copyright (c) 2019, 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */


#include <stdio.h>
#include <memory>

#include <aidl/vendor/qti/automotive/qcarcam/IQcarCamera.h>
#include <aidl/vendor/qti/automotive/qcarcam/IQcarCameraStream.h>
#include <aidl/vendor/qti/automotive/qcarcam/IQcarCameraStreamCB.h>
#include <aidl/vendor/qti/automotive/qcarcam/BnQcarCameraStreamCB.h>
#include <cutils/native_handle.h>

#include "qcarcam_types.h"
#include "qcarcam.h"

//log
#include "ais_log.h"
#define AIS_AIDL_ERRORMSG(_fmt_, ...) \
    ais_log(AIS_MOD_ID_HIDL_SERVICE, AIS_LOG_LVL_ERROR,  "AIS_AIDL_CLIENT %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_ERROR], ##__VA_ARGS__)

#define AIS_AIDL_DBGMSG(_fmt_, ...) \
    ais_log(AIS_MOD_ID_HIDL_SERVICE, AIS_LOG_LVL_DBG,  "AIS_AIDL_CLIENT %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_DBG], ##__VA_ARGS__)

#define AIS_AIDL_INFOMSG(_fmt_, ...) \
    ais_log(AIS_MOD_ID_HIDL_SERVICE, AIS_LOG_LVL_MED,  "AIS_AIDL_CLIENT %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_MED], ##__VA_ARGS__)

using namespace ::aidl::vendor::qti::automotive::qcarcam;

class ais_aidl_client : public BnQcarCameraStreamCB
{
public:
    ais_aidl_client(std::shared_ptr<IQcarCamera>service);
    ~ais_aidl_client();
    qcarcam_ret_t ais_client_open_stream(qcarcam_input_desc_t desc);
    qcarcam_ret_t ais_client_close_stream();
    qcarcam_ret_t ais_client_s_param(qcarcam_param_t param, const qcarcam_param_value_t* p_value);
    qcarcam_ret_t ais_client_s_cb(void *cb_ptr);
    qcarcam_ret_t ais_client_s_buffer_v2(qcarcam_bufferlist_t* p_buffers);
    qcarcam_ret_t ais_client_start_stream(qcarcam_hndl_t q_hndl);
    qcarcam_ret_t ais_client_stop_stream();
    qcarcam_ret_t ais_client_get_frame_v2(qcarcam_frame_info_v2_t* p_frame_info,
        unsigned long long int timeout, unsigned int flags);
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
    std::shared_ptr <IQcarCamera> mpService;
    std::shared_ptr <IQcarCameraStream> mpStream;
    QcarcamInputDesc mStreamId;
    std::atomic<bool> mStreaming;
    void get_ais_aidl_params(qcarcam_param_t param_type, const qcarcam_param_value_t* param_value,
        QcarcamStreamParam& Param,
        QcarcamStreamConfigData& data);
    void get_qcarcam_params(QcarcamStreamParam Param,
        const QcarcamStreamConfigData& data,
        qcarcam_param_t* param_type,
        qcarcam_param_value_t* param_value);

    // Callback function defined in IQcarCameraStreamCB
    ::ndk::ScopedAStatus qcarcam_event_callback_1_1(QcarcamEvent EventType,
        const QcarcamEventPayload& Payload) override; //Deprecated
    ::ndk::ScopedAStatus qcarcam_event_callback(QcarcamEvent EventType,
        const QcarcamEventPayload& Payload) override;
};
