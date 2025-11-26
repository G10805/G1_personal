/* ===========================================================================
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#ifndef AIS_AIDL_STREAM_H
#define AIS_AIDL_STREAM_H

#include <cutils/native_handle.h>
#include <aidl/vendor/qti/automotive/qcarcam/IQcarCamera.h>
#include <aidl/vendor/qti/automotive/qcarcam/IQcarCameraStreamCB.h>
#include <aidl/vendor/qti/automotive/qcarcam/BnQcarCameraStream.h>
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


namespace aidl {
namespace vendor {
namespace qti {
namespace automotive {
namespace qcarcam {

using namespace android;

class ais_aidl_stream : public BnQcarCameraStream {

private:
    QcarcamInputDesc mDesc;
    qcarcam_hndl_t mQcarcamHndl;
    std::shared_ptr <IQcarCameraStreamCB> mStreamObj;  // The callback used to deliver each frame
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
    static void copyPlayloadData(QcarcamEventPayload *pPayload, qcarcam_event_payload_t *pqPayload);
    void eventDispatcherThread();
    QcarcamError get_qcarcam_params(QcarcamStreamParam Param, const QcarcamStreamConfigData& data,
            qcarcam_param_t* param_type, qcarcam_param_value_t* param_value, unsigned int **gamma_table=nullptr);
    QcarcamError get_ais_aidl_params(qcarcam_param_t param_type, qcarcam_param_value_t* param_value,
            QcarcamStreamParam& Param, QcarcamStreamConfigData& data, unsigned int *gamma_table=nullptr);

    // Event callback from qcarcam engine
    static void qcarcam_event_cb(qcarcam_hndl_t hndl, qcarcam_event_t event_id, qcarcam_event_payload_t *p_payload);

public:

    ais_aidl_stream(QcarcamInputDesc Desc, gQcarCamClientData *pHead, qcarcam_hndl_t hndl);
    virtual ~ais_aidl_stream() override;

    ::ndk::ScopedAStatus configureStream_1_1(QcarcamStreamParam Param, const QcarcamStreamConfigData& data, QcarcamError* _aidl_return) override; //Deprecated
    ::ndk::ScopedAStatus configureStream(QcarcamStreamParam Param, const QcarcamStreamConfigData& data, QcarcamError* _aidl_return) override;
    ::ndk::ScopedAStatus getFrame_1_1(int64_t timeout, int32_t flags, QcarcamFrameInfov2* Info, QcarcamError* _aidl_return) override; //Deprecated
    ::ndk::ScopedAStatus getFrame(int64_t timeout, int32_t flags, QcarcamFrameInfov2* Info, QcarcamError* _aidl_return) override;
    ::ndk::ScopedAStatus getStreamConfig_1_1(QcarcamStreamParam Param,
            QcarcamStreamConfigData* data, QcarcamError* _aidl_return) override; //Deprecated
    ::ndk::ScopedAStatus getStreamConfig(QcarcamStreamParam Param, const QcarcamStreamConfigData& in_data,
            QcarcamStreamConfigData* data, QcarcamError* _aidl_return) override;
    ::ndk::ScopedAStatus pauseStream(QcarcamError* _aidl_return) override;
    ::ndk::ScopedAStatus resumeStream(QcarcamError* _aidl_return) override;
    ::ndk::ScopedAStatus releaseFrame_1_1(int32_t id, int32_t idx, QcarcamError* _aidl_return) override; //Deprecated
    ::ndk::ScopedAStatus releaseFrame(int32_t id, int32_t idx, QcarcamError* _aidl_return) override;
    ::ndk::ScopedAStatus startStream_1_1(const std::shared_ptr<IQcarCameraStreamCB>& streamObj, QcarcamError* _aidl_return) override; //Deprecated
    ::ndk::ScopedAStatus startStream(const std::shared_ptr<IQcarCameraStreamCB>& streamObj, QcarcamError* _aidl_return) override;
    ::ndk::ScopedAStatus stopStream_1_1(QcarcamError* _aidl_return) override; //Deprecated
    ::ndk::ScopedAStatus stopStream(QcarcamError* _aidl_return) override;
    ::ndk::ScopedAStatus setStreamBuffers_1_1(const ::aidl::android::hardware::common::NativeHandle& hndl,
        const QcarcamBuffersInfoList& info, QcarcamError* _aidl_return) override; //Deprecated
    ::ndk::ScopedAStatus setStreamBuffers(const ::aidl::android::hardware::common::NativeHandle& hndl,
        const QcarcamBuffersInfoList& info, QcarcamError* _aidl_return) override;

    // Public Utility functions
    static gQcarCamClientData* findEventQByHndl(const qcarcam_hndl_t qcarcam_hndl);

};

} // qcarcam
} // automotive
} // qti
} // vendor
} // aidl

#endif //AIS_AIDL_STREAM_H
