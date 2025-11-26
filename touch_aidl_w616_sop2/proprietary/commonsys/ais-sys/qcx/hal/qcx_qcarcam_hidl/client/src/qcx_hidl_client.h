/* ===========================================================================
 * Copyright (c) 2019-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */


#include <stdio.h>

#include <vendor/qti/automotive/qcarcam/2.0/types.h>
#include <vendor/qti/automotive/qcarcam/2.0/IQcarCamera.h>
#include <vendor/qti/automotive/qcarcam/2.0/IQcarCameraStream.h>
#include <vendor/qti/automotive/qcarcam/2.0/IQcarCameraStreamCB.h>
#include <cutils/native_handle.h>

#include "qcarcam_types.h"
#include "qcarcam_diag_types.h"
#include "qcarcam.h"

//log
#include "qcxlog.h"
#define QCX_HIDL_ERRORMSG(_fmt_, ...) \
    QCX_LOG(CLIENT_MGR, ERROR,  "QCX_HIDL_CLIENT :%d %s " _fmt_ "\n", __LINE__, g_OSALLevelStringLUT[QCX_LOG_LVL_ERROR], ##__VA_ARGS__)

#define QCX_HIDL_DBGMSG(_fmt_, ...) \
    QCX_LOG(CLIENT_MGR, DEBUG,  "QCX_HIDL_CLIENT :%d %s " _fmt_ "\n", __LINE__, g_OSALLevelStringLUT[QCX_LOG_LVL_DEBUG], ##__VA_ARGS__)

#define QCX_HIDL_INFOMSG(_fmt_, ...) \
    QCX_LOG(CLIENT_MGR, MEDIUM,  "QCX_HIDL_CLIENT :%d %s " _fmt_ "\n", __LINE__, g_OSALLevelStringLUT[QCX_LOG_LVL_MEDIUM], ##__VA_ARGS__)

 #define UNUSED_PARAM(param) ((void)(param))

//qcarcam HAL datatypes
using vendor::qti::automotive::qcarcam::V2_0::IQcarCamera;
using vendor::qti::automotive::qcarcam::V2_0::IQcarCameraStream;
using vendor::qti::automotive::qcarcam::V2_0::IQcarCameraStreamCB;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamError;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamOpenParam;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamOpmode;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamInputStream;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamColorFmt;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamInputSrc;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamMode;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamInput;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamParamType;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamExposureMode;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamExposureConfig;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamFrameDropMode;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamGammaType;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamFrameDropConfig;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamGammaConfig;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamLatencyControl;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamBatchMode;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamIspUsecase;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamIspUsecaseConfig;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamInputSignal;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamColorSpace;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamStreamConfigData;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamPlane;
using vendor::qti::automotive::qcarcam::V2_0::QcarcamBuffersInfo;
using vendor::qti::automotive::qcarcam::V2_0::QcarcamFrameField;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamErrorEvent;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamErrorInfo;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamHWTimestamp;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamVendorParam;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamInterlaceField;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamBatchFramesInfo;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamFrameInfo;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamEventRecoveryMsg;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamRecovery;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamEventPayload;

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::android::hardware::hidl_vec;

class qcx_hidl_client : public IQcarCameraStreamCB
{
public:
    qcx_hidl_client(android::sp<IQcarCamera>service);
    ~qcx_hidl_client();
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
    QCarCamRet_e qcx_client_g_param(QCarCamParamType_e param, const void* p_value, uint32_t size);
    QCarCamRet_e qcx_client_abort_due_to_death();

    //App callback pointer
    QCarCamEventCallback_t app_cb;

    std::atomic<bool> mInitialized;

private:
    std::mutex mLock;
    QCarCamHndl_t mQcarcamHndl;
    struct native_handle* mNativeHndl;
    android::sp <IQcarCamera> mpService;
    android::sp <IQcarCameraStream> mpStream;
    uint32_t mStreamId;
    std::atomic<bool> mStreaming;
    QCarCamError config_hidl_params(QCarCamParamType_e qParam,
        const void *pqConfigData, bool setParam);

    static void copyPlayloadData(QCarCamEventPayload_t *pqPayload, const QCarCamEventPayload *pPayload);
    // Callback function defined in IQcarCameraStreamCB
    Return<void> qcarcam_event_callback(uint32_t EventType,
            const QCarCamEventPayload& Payload) override;
    void *mpPrivateData;
};
