/* ===========================================================================
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#ifndef AIS_HIDL_STREAM_H
#define AIS_HIDL_STREAM_H

#include <vendor/qti/automotive/qcarcam/2.0/types.h>
#include <vendor/qti/automotive/qcarcam/2.0/IQcarCamera.h>
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
#define AIS_HIDL_ERRORMSG(_fmt_, ...) \
    ais_log(AIS_MOD_ID_HIDL_SERVICE, AIS_LOG_LVL_ERROR,  "AIS_HIDL_SERVER %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_ERROR], ##__VA_ARGS__)

#define AIS_HIDL_DBGMSG(_fmt_, ...) \
    ais_log(AIS_MOD_ID_HIDL_SERVICE, AIS_LOG_LVL_DBG,  "AIS_HIDL_SERVER %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_DBG], ##__VA_ARGS__)

#define AIS_HIDL_INFOMSG(_fmt_, ...) \
    ais_log(AIS_MOD_ID_HIDL_SERVICE, AIS_LOG_LVL_MED,  "AIS_HIDL_SERVER %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_MED], ##__VA_ARGS__)

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


namespace vendor {
namespace qti {
namespace automotive {
namespace qcarcam {
namespace V2_0 {
namespace implementation {

//qcarcam HAL datatypes
using vendor::qti::automotive::qcarcam::V2_0::QCarCamError;
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

using android::hardware::Return;
using android::hardware::hidl_handle;
using android::hardware::hidl_vec;
using android::sp;
using namespace android::hardware;

// From ais_hidl_camera.h
class ais_hidl_camera;

class ais_hidl_stream : public IQcarCameraStream {

private:
    uint32_t mInputId;
    QCarCamHndl_t mQcarcamHndl;
    sp <IQcarCameraStreamCB> mStreamObj = nullptr;  // The callback used to deliver each frame
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
    static void copyPlayloadData(QCarCamEventPayload *pPayload, QCarCamEventPayload_t *pqPayload);
    void eventDispatcherThread();
    QCarCamError config_qcarcam_params(QCarCamParamType Param,
        QCarCamStreamConfigData& streamConfigData, bool setParam);

    // Event callback from qcarcam engine
    static QCarCamRet_e qcarcam_event_cb(const QCarCamHndl_t hndl,
            const uint32_t eventId, const QCarCamEventPayload_t *pPayload, void  *pPrivateData);

public:

    ais_hidl_stream(uint32_t inputId, gQcarCamClientData *pHead, QCarCamHndl_t hndl);
    virtual ~ais_hidl_stream() override;

    Return<QCarCamError> configureStream(QCarCamParamType Param, const QCarCamStreamConfigData& data)  override;
    Return<QCarCamError> setStreamBuffers(const hidl_handle& hndl, const QcarcamBuffersInfo& info)  override;
    Return<QCarCamError> startStream(const sp<IQcarCameraStreamCB>& streamObj)  override;
    Return<QCarCamError> stopStream()  override;
    Return<void> getFrame(uint64_t timeout, uint32_t flags, getFrame_cb _hidl_cb)  override;
    Return<QCarCamError> releaseFrame(uint32_t id, uint32_t bufferIdx)  override;
    Return<QCarCamError> pauseStream()  override;
    Return<QCarCamError> resumeStream()  override;
    Return<QCarCamError> reserveStream()  override; //Dummy Implementation to avoid unimplemented pure virtual method error
    Return<QCarCamError> releaseStream()  override; //Dummy Implementation to avoid unimplemented pure virtual method error
    Return<void> getStreamConfig(QCarCamParamType Param, const QCarCamStreamConfigData& data, getStreamConfig_cb _hidl_cb)  override;

    // Public Utility functions
    static gQcarCamClientData* findEventQByHndl(const QCarCamHndl_t qcarcam_hndl);

};

} //implementation
} //V2_0
} //qcarcam
} //automotive
} //qti
} //vendor

#endif //AIS_HIDL_STREAM_H
