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
#include "qcxlog.h"
#define QCX_AIDL_ERRORMSG(_fmt_, ...) \
    QCX_LOG(CLIENT_MGR, ERROR,  "QCX_AIDL_CLIENT :%d %s " _fmt_ "\n",__LINE__, g_OSALLevelStringLUT[QCX_LOG_LVL_ERROR], ##__VA_ARGS__)

#define QCX_AIDL_DBGMSG(_fmt_, ...) \
    QCX_LOG(CLIENT_MGR, DEBUG,  "QCX_AIDL_CLIENT :%d %s " _fmt_ "\n", __LINE__, g_OSALLevelStringLUT[QCX_LOG_LVL_DEBUG], ##__VA_ARGS__)

#define QCX_AIDL_INFOMSG(_fmt_, ...) \
    QCX_LOG(CLIENT_MGR, MEDIUM,  "QCX_AIDL_CLIENT :%d %s " _fmt_ "\n", __LINE__, g_OSALLevelStringLUT[QCX_LOG_LVL_MEDIUM], ##__VA_ARGS__)

//qcarcam HAL datatypes
using namespace ::aidl::vendor::qti::automotive::qcarcam2;

using ::android::sp;
using namespace android;

class qcx_aidl_client final : public BnQcarCameraStreamCB
{
public:
    qcx_aidl_client(std::shared_ptr<IQcarCamera>service);
    ~qcx_aidl_client();
    QCarCamRet_e qcx_client_open_stream(const QCarCamOpen_t* pOpenParams);
    QCarCamRet_e qcx_client_close_stream();
    QCarCamRet_e qcx_client_s_param(QCarCamParamType_e param, const void* p_value, uint32_t size);
    QCarCamRet_e qcx_client_set_cb(QCarCamEventCallback_t cb_ptr, void *pPrivateData);
    QCarCamRet_e qcx_client_unset_cb();
    QCarCamRet_e qcx_client_s_buffer(const QCarCamBufferList_t* p_buffers);
    QCarCamRet_e qcx_client_start_stream(QCarCamHndl_t q_hndl);
    QCarCamRet_e qcx_client_stop_stream();
    QCarCamRet_e qcx_client_get_frame(QCarCamFrameInfo_t* pqFrameInfo,
        unsigned long long int timeout, unsigned int flags);
    QCarCamRet_e qcx_client_release_frame(uint32_t id, uint32_t bufferIdx);
    QCarCamRet_e qcx_client_pause_stream();
    QCarCamRet_e qcx_client_resume_stream();
    QCarCamRet_e qcx_client_reserve_stream();
    QCarCamRet_e qcx_client_release_stream();
    QCarCamRet_e qcx_client_submitRequest(const QCarCamRequest_t* pRequest);
    QCarCamRet_e qcx_client_g_param(QCarCamParamType_e param, const void* p_value, uint32_t size);
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

    void copyPlayloadData(QCarCamEventPayload_t *pqPayload, const QCarCamEventPayload *pPayload);
    // Callback function defined in IQcarCameraStreamCB
    void *mpPrivateData;
    ::ndk::ScopedAStatus qcarcam_event_callback(int32_t EventType,
        const QCarCamEventPayload& Payload) override;
};

