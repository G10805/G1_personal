/* ===========================================================================
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */


#include <stdio.h>

#include <aidl/vendor/qti/automotive/qcarcam2/IQcarCamera.h>
#include <aidl/vendor/qti/automotive/qcarcam2/IQcarCameraStream.h>
#include <aidl/vendor/qti/automotive/qcarcam2/IQcarCameraStreamCB.h>
#include <aidl/vendor/qti/automotive/qcarcam2/BnQcarCameraStreamCB.h>
#include <aidl/vendor/qti/automotive/qcarcam2/QcarcamFrameField.h>

#include <cutils/native_handle.h>

#include "qcarcam_types.h"
#include "qcarcam_diag_types.h"
#include "qcarcam.h"

//log
#include "ais_log.h"
#define AIS_AIDL_ERRORMSG(_fmt_, ...) \
    ais_log(AIS_MOD_ID_AIDL_SERVICE, AIS_LOG_LVL_ERROR,  "AIS_AIDL_CLIENT %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_ERROR], ##__VA_ARGS__)

#define AIS_AIDL_DBGMSG(_fmt_, ...) \
    ais_log(AIS_MOD_ID_AIDL_SERVICE, AIS_LOG_LVL_DBG,  "AIS_AIDL_CLIENT %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_DBG], ##__VA_ARGS__)

#define AIS_AIDL_INFOMSG(_fmt_, ...) \
    ais_log(AIS_MOD_ID_AIDL_SERVICE, AIS_LOG_LVL_MED,  "AIS_AIDL_CLIENT %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_MED], ##__VA_ARGS__)

//qcarcam HAL datatypes
using namespace ::aidl::vendor::qti::automotive::qcarcam2;

class ais_aidl_client : public BnQcarCameraStreamCB
{
public:
    ais_aidl_client(std::shared_ptr<IQcarCamera>service);
    ~ais_aidl_client();
    QCarCamRet_e ais_client_open_stream(const QCarCamOpen_t* pOpenParams);
    QCarCamRet_e ais_client_close_stream();
    QCarCamRet_e ais_client_s_param(QCarCamParamType_e param, const void* p_value, uint32_t size);
    QCarCamRet_e ais_client_set_cb(QCarCamEventCallback_t cb_ptr, void *pPrivateData);
    QCarCamRet_e ais_client_unset_cb();
    QCarCamRet_e ais_client_s_buffer(const QCarCamBufferList_t* p_buffers);
    QCarCamRet_e ais_client_start_stream(QCarCamHndl_t q_hndl);
    QCarCamRet_e ais_client_stop_stream();
    QCarCamRet_e ais_client_get_frame(QCarCamFrameInfo_t* pqFrameInfo,
        unsigned long long int timeout, unsigned int flags);
    QCarCamRet_e ais_client_release_frame(uint32_t id, uint32_t bufferIdx);
    QCarCamRet_e ais_client_pause_stream();
    QCarCamRet_e ais_client_resume_stream();
    QCarCamRet_e ais_client_reserve_stream();
    QCarCamRet_e ais_client_release_stream();
    QCarCamRet_e ais_client_g_param(QCarCamParamType_e param, const void* p_value, uint32_t size);

    //App callback pointer
    QCarCamEventCallback_t app_cb;

    std::atomic<bool> mInitialized;

private:
    std::mutex mLock;
    QCarCamHndl_t mQcarcamHndl;
    struct native_handle* mNativeHndl;
    std::shared_ptr <IQcarCamera> mpService;
    std::shared_ptr <IQcarCameraStream> mpStream;
    uint32_t mStreamId;
    std::atomic<bool> mStreaming;
    QCarCamError config_aidl_params(QCarCamParamType_e qParam,
        const void *pqConfigData, bool setParam);

    static void copyPayloadData(QCarCamEventPayload_t *pqPayload, const QCarCamEventPayload *pPayload);
    // Callback function defined in IQcarCameraStreamCB
    ::ndk::ScopedAStatus qcarcam_event_callback(int32_t EventType,
            const QCarCamEventPayload& Payload) override;
    void *mpPrivateData;
};
