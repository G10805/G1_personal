/* ===========================================================================
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#ifndef AIS_HIDL_STREAM_H
#define AIS_HIDL_STREAM_H

#include <vendor/qti/automotive/qcarcam/1.1/types.h>
#include <vendor/qti/automotive/qcarcam/1.1/IQcarCamera.h>
#include <vendor/qti/automotive/qcarcam/1.1/IQcarCameraStreamCB.h>
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
    qcarcam_event_t event_id;
    qcarcam_event_payload_t payload;

    QEventDesc(qcarcam_event_t event , qcarcam_event_payload_t Payload) {event_id = event; payload = Payload;}
};

typedef struct QcarcamCamera {
    qcarcam_hndl_t qcarcam_context;
    std::mutex gEventQLock;
    std::condition_variable gCV;
    std::list<QEventDesc> sQcarcamList;
}gQcarCamClientData;


namespace vendor {
namespace qti {
namespace automotive {
namespace qcarcam {
namespace V1_0 {
namespace implementation {

using android::hardware::Return;
using android::hardware::hidl_handle;
using android::hardware::hidl_vec;
using android::sp;
using namespace android::hardware;
using vendor::qti::automotive::qcarcam::V1_1::IQcarCameraStream;
using vendor::qti::automotive::qcarcam::V1_0::IQcarCameraStreamCB;
// From ais_hidl_camera.h
class ais_hidl_camera;

class ais_hidl_stream : public IQcarCameraStream {

private:
    QcarcamInputDesc mDesc;
    qcarcam_hndl_t mQcarcamHndl;
    sp <vendor::qti::automotive::qcarcam::V1_1::IQcarCameraStreamCB> mStreamObj = nullptr;  // The callback used to deliver each frame
    sp <IQcarCameraStreamCB> mStreamObj2 = nullptr;  // The callback used to deliver each frame
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
    void eventDispatcherThread();
    QcarcamError get_qcarcam_params(QcarcamStreamParam Param, const QcarcamStreamConfigData& data,
            qcarcam_param_t* param_type, qcarcam_param_value_t* param_value, unsigned int **gamma_table=nullptr);
    QcarcamError get_ais_hidl_params(qcarcam_param_t param_type, qcarcam_param_value_t* param_value,
            QcarcamStreamParam& Param, QcarcamStreamConfigData& data, unsigned int *gamma_table=nullptr);
    QcarcamError get_qcarcam_params_1_1(vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam Param, const vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamConfigData& data,
            qcarcam_param_t* param_type, qcarcam_param_value_t* param_value, unsigned int **gamma_table=nullptr);
    QcarcamError get_ais_hidl_params_1_1(qcarcam_param_t param_type, qcarcam_param_value_t* param_value,
            vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam& Param, vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamConfigData& data, unsigned int *gamma_table=nullptr);

    // Event callback from qcarcam engine
    static void qcarcam_event_cb(qcarcam_hndl_t hndl, qcarcam_event_t event_id, qcarcam_event_payload_t *p_payload);

     /*
     * @brief Destroy a native handle
     *
     * Helper function which will close the file descripters
     * contained in native handle and frees the memory allocated
     * for the handle.
     *
     * @param handle native handle to destroy
     */
    void destroyNativeHandle(native_handle_t** handle);

public:

    ais_hidl_stream(QcarcamInputDesc Desc, gQcarCamClientData *pHead, qcarcam_hndl_t hndl);
    virtual ~ais_hidl_stream() override;

    Return<QcarcamError> configureStream(QcarcamStreamParam Param, const QcarcamStreamConfigData& data)  override;
    Return<QcarcamError> configureStream_1_1(vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam Param, const vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamConfigData& data)  override;
    Return<QcarcamError> setStreamBuffers(const hidl_handle& hndl, const QcarcamBuffersInfo& info)  override;
    Return<QcarcamError> setStreamBuffers_1_1(const hidl_handle& hndl, const vendor::qti::automotive::qcarcam::V1_1::QcarcamBuffersInfoList& info)  override;
    Return<QcarcamError> startStream(const sp<IQcarCameraStreamCB>& streamObj)  override;
    Return<QcarcamError> startStream_1_1(const sp<vendor::qti::automotive::qcarcam::V1_1::IQcarCameraStreamCB>& streamObj)  override;
    Return<QcarcamError> stopStream()  override;
    Return<QcarcamError> stopStream_1_1()  override;
    Return<void> getFrame(uint64_t timeout, uint32_t flags, getFrame_cb _hidl_cb)  override;
    Return<void> getFrame_1_1(uint64_t timeout, uint32_t flags, getFrame_1_1_cb _hidl_cb)  override;
    Return<QcarcamError> releaseFrame(uint32_t idx)  override;
    Return<QcarcamError> releaseFrame_1_1(uint32_t id,uint32_t idx)  override;
    Return<QcarcamError> pauseStream()  override;
    Return<QcarcamError> resumeStream()  override;
    Return<void> getStreamConfig(QcarcamStreamParam Param, const QcarcamStreamConfigData& data, getStreamConfig_cb _hidl_cb)  override;
    Return<void> getStreamConfig_1_1(vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamParam Param, const vendor::qti::automotive::qcarcam::V1_1::QcarcamStreamConfigData& data, getStreamConfig_1_1_cb _hidl_cb)  override;

    // Public Utility functions
    static gQcarCamClientData* findEventQByHndl(const qcarcam_hndl_t qcarcam_hndl);

};

} //implementation
} //V1_0
} //qcarcam
} //automotive
} //qti
} //vendor

#endif //AIS_HIDL_STREAM_H
