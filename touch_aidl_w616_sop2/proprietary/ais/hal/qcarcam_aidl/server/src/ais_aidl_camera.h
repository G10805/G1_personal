/* ===========================================================================
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#ifndef AIS_AIDL_CAMERA_H
#define AIS_AIDL_CAMERA_H

#include <aidl/vendor/qti/automotive/qcarcam/IQcarCamera.h>
#include <aidl/vendor/qti/automotive/qcarcam/IQcarCameraStream.h>
#include <aidl/vendor/qti/automotive/qcarcam/BnQcarCamera.h>
#include <thread>
#ifdef HAL_CAMERA_CPMS_SUPPORT
#include "PowerEvent.h"
#include "PowerPolicyService.h"
#endif

namespace aidl {
namespace vendor {
namespace qti {
namespace automotive {
namespace qcarcam {

class ais_aidl_stream; // from ais_aidl_stream.h

class ais_aidl_camera : public BnQcarCamera {
public:

    ais_aidl_camera();
    ~ais_aidl_camera();

    ::ndk::ScopedAStatus openStream_1_1(QcarcamInputDesc Desc, std::shared_ptr<IQcarCameraStream>* _aidl_return) override; //Deprecated
    ::ndk::ScopedAStatus openStream(QcarcamInputDesc Desc, std::shared_ptr<IQcarCameraStream>* _aidl_return) override;
    ::ndk::ScopedAStatus getInputStreamList_1_1(std::vector<QcarcamInputInfov2>* inputs, QcarcamError* _aidl_return) override; //Deprecated
    ::ndk::ScopedAStatus getInputStreamList(std::vector<QcarcamInputInfov2>* inputs, QcarcamError* _aidl_return) override;
    ::ndk::ScopedAStatus closeStream_1_1(const std::shared_ptr<IQcarCameraStream>& camStream, QcarcamError* _aidl_return) override; //Deprecated
    ::ndk::ScopedAStatus closeStream(const std::shared_ptr<IQcarCameraStream>& camStream, QcarcamError* _aidl_return) override;
#ifdef HAL_CAMERA_CPMS_SUPPORT
    static int PowerEventCb(pm_event_t power_event_id, void* pUsrCtxt);
#endif

private:
    int mQcarcamClientCnt;
    std::mutex mClientCntLock;
    std::thread mMonitorThread;
    void clientMonitorThread();
    std::atomic<bool> mMonitorThreadExit;
#ifdef HAL_CAMERA_CPMS_SUPPORT
    std::thread mPowerEventThread;    // The Thread for the CPMS power Events
#endif
};

} // qcarcam
} // automotive
} // qti
} // vendor
} // aidl

#endif //AIS_AIDL_CAMERA_H
