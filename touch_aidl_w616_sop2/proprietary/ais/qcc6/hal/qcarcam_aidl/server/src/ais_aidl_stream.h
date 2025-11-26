/* ===========================================================================
 * Copyright (c) 2021,2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#ifndef AIS_AIDL_STREAM_H
#define AIS_AIDL_STREAM_H

#include <cutils/native_handle.h>
#include <aidl/vendor/qti/automotive/qcarcam2/IQcarCamera.h>
#include <aidl/vendor/qti/automotive/qcarcam2/IQcarCameraStreamCB.h>
#include <aidl/vendor/qti/automotive/qcarcam2/BnQcarCameraStream.h>
#include <aidl/vendor/qti/automotive/qcarcam2/QcarcamFrameField.h>
#include <thread>
#include <functional>
#include <list>
#include <atomic>
#include <thread>
#include "qcarcam_types.h"
#include "qcarcam.h"

#define MAX_AIS_CLIENTS 32
#define NOT_IN_USE 0
#define MAX_NUM_PLANES 3

//log
#include "ais_log.h"
#define AIS_AIDL_ERRORMSG(_fmt_, ...) \
    ais_log(AIS_MOD_ID_AIDL_SERVICE, AIS_LOG_LVL_ERROR,  "AIS_AIDL_SERVER %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_ERROR], ##__VA_ARGS__)

#define AIS_AIDL_DBGMSG(_fmt_, ...) \
    ais_log(AIS_MOD_ID_AIDL_SERVICE, AIS_LOG_LVL_DBG,  "AIS_AIDL_SERVER %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_DBG], ##__VA_ARGS__)

#define AIS_AIDL_INFOMSG(_fmt_, ...) \
    ais_log(AIS_MOD_ID_AIDL_SERVICE, AIS_LOG_LVL_MED,  "AIS_AIDL_SERVER %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_MED], ##__VA_ARGS__)

// Global declarations
struct QEventDesc {
    uint32_t eventId;
    QCarCamEventPayload_t payload;

    QEventDesc(uint32_t event , QCarCamEventPayload_t Payload) {eventId = event; payload = Payload;}
};

typedef struct QcarcamCamera {
    QCarCamHndl_t qcarcam_context;
    std::mutex gEventQLock;
    std::condition_variable gCV;
    std::list<QEventDesc> sQcarcamList;
}gQcarCamClientData;


namespace aidl {
namespace vendor {
namespace qti {
namespace automotive {
namespace qcarcam2 {

using vendor::qti::automotive::qcarcam2::QCarCamError;
using vendor::qti::automotive::qcarcam2::QCarCamOpmode;
using vendor::qti::automotive::qcarcam2::QCarCamInputStream;
using vendor::qti::automotive::qcarcam2::QCarCamColorFmt;
using vendor::qti::automotive::qcarcam2::QCarCamInputSrc;
using vendor::qti::automotive::qcarcam2::QCarCamMode;
using vendor::qti::automotive::qcarcam2::QCarCamInput;
using vendor::qti::automotive::qcarcam2::QCarCamParamType;
using vendor::qti::automotive::qcarcam2::QCarCamExposureMode;
using vendor::qti::automotive::qcarcam2::QCarCamExposureConfig;
using vendor::qti::automotive::qcarcam2::QCarCamFrameDropConfig;
using vendor::qti::automotive::qcarcam2::QCarCamGammaType;
using vendor::qti::automotive::qcarcam2::QCarCamFrameDropConfig;
using vendor::qti::automotive::qcarcam2::QCarCamGammaConfig;
using vendor::qti::automotive::qcarcam2::QCarCamLatencyControl;
using vendor::qti::automotive::qcarcam2::QCarCamBatchMode;
using vendor::qti::automotive::qcarcam2::QCarCamIspUsecase;
using vendor::qti::automotive::qcarcam2::QCarCamIspUsecaseConfig;
using vendor::qti::automotive::qcarcam2::QCarCamInputSignal;
using vendor::qti::automotive::qcarcam2::QCarCamColorSpace;
using vendor::qti::automotive::qcarcam2::QCarCamStreamConfigData;
using vendor::qti::automotive::qcarcam2::QCarCamPlane;
using vendor::qti::automotive::qcarcam2::QcarcamBuffersInfo;
using vendor::qti::automotive::qcarcam2::QcarcamFrameField;
using vendor::qti::automotive::qcarcam2::QCarCamErrorEvent;
using vendor::qti::automotive::qcarcam2::QCarCamErrorInfo;
using vendor::qti::automotive::qcarcam2::QCarCamHWTimestamp;
using vendor::qti::automotive::qcarcam2::QCarCamVendorParam;
using vendor::qti::automotive::qcarcam2::QCarCamInterlaceField;
using vendor::qti::automotive::qcarcam2::QCarCamBatchFramesInfo;
using vendor::qti::automotive::qcarcam2::QCarCamFrameInfo;
using vendor::qti::automotive::qcarcam2::QCarCamEventRecoveryMsg;
using vendor::qti::automotive::qcarcam2::QCarCamRecovery;
using vendor::qti::automotive::qcarcam2::QCarCamEventPayload;

using ::android::sp;
using namespace android;

class ais_aidl_stream : public BnQcarCameraStream {

private:
    uint32_t mInputId;
    QCarCamHndl_t mQcarcamHndl;
    std::shared_ptr <::aidl::vendor::qti::automotive::qcarcam2::IQcarCameraStreamCB> mStreamObj;  // The callback used to deliver each frame
    gQcarCamClientData *pCurrentRecord;             // Pointer to current stream Event Q
    std::thread mCaptureThread;                     // The thread we'll use to dispatch events
    std::atomic<int> mRunMode;                      // Used to signal the frame loop (see RunModes below)
    enum RunModes {
        STOPPED     = 0,
        RUN         = 1,
        STOPPING    = 2,
    };
    std::atomic<bool> mStreaming;
    std::mutex mApiLock;
    native_handle_t* cloneHandle;

    // Private Utility functions
    static void findEventQByHndl(void);
    static void copyPayloadData(QCarCamEventPayload *pPayload, QCarCamEventPayload_t *pqPayload);
    void eventDispatcherThread();
    QCarCamError config_qcarcam_params(QCarCamParamType Param, QCarCamStreamConfigData& streamConfigData, bool setParam);

    // Event callback from qcarcam engine
    static QCarCamRet_e qcarcam_event_cb(const QCarCamHndl_t hndl,
            const uint32_t eventId, const QCarCamEventPayload_t *pPayload, void  *pPrivateData);

public:

    ais_aidl_stream(uint32_t inputId, gQcarCamClientData *pHead, QCarCamHndl_t hndl);
    virtual ~ais_aidl_stream() override;

    ::ndk::ScopedAStatus configureStream(::aidl::vendor::qti::automotive::qcarcam2::QCarCamParamType in_Param, const ::aidl::vendor::qti::automotive::qcarcam2::QCarCamStreamConfigData& in_data, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) override;
    ::ndk::ScopedAStatus getFrame(int64_t in_timeout, int32_t in_flags, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamFrameInfo* out_Info, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) override;
    ::ndk::ScopedAStatus getStreamConfig(::aidl::vendor::qti::automotive::qcarcam2::QCarCamParamType in_Param, const ::aidl::vendor::qti::automotive::qcarcam2::QCarCamStreamConfigData& in_data, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamStreamConfigData* out_datatype, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) override;
    ::ndk::ScopedAStatus pauseStream(::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) override;
    ::ndk::ScopedAStatus releaseFrame(int32_t in_id, int32_t in_bufferIdx, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) override;
    ::ndk::ScopedAStatus resumeStream(::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) override;
    ::ndk::ScopedAStatus startStream(const std::shared_ptr<::aidl::vendor::qti::automotive::qcarcam2::IQcarCameraStreamCB>& in_streamObj, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) override;
    ::ndk::ScopedAStatus stopStream(::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) override;
    ::ndk::ScopedAStatus setStreamBuffers(const ::aidl::android::hardware::common::NativeHandle& in_hndl, const ::aidl::vendor::qti::automotive::qcarcam2::QcarcamBuffersInfo& in_info, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) override;
    ::ndk::ScopedAStatus reserveStream(::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) override;
    ::ndk::ScopedAStatus releaseStream(::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) override;
    ::ndk::ScopedAStatus submitRequest(const ::aidl::vendor::qti::automotive::qcarcam2::QCarCamRequest& in_Info, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) override;
    // Public Utility functions
    static gQcarCamClientData* findEventQByHndl(const QCarCamHndl_t qcarcam_hndl);

};

} // qcarcam2
} // automotive
} // qti
} // vendor
} // aidl

#endif //AIS_AIDL_STREAM_H
