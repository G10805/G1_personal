/* ===========================================================================
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#ifndef AIS_HIDL_CAMERA_H
#define AIS_HIDL_CAMERA_H

#include <vendor/qti/automotive/qcarcam/1.1/IQcarCamera.h>
#include <vendor/qti/automotive/qcarcam/1.1/IQcarCameraStream.h>
#include <thread>
#ifdef HAL_CAMERA_CPMS_SUPPORT
#include "PowerEvent.h"
#include "PowerPolicyService.h"
#endif

namespace vendor {
namespace qti {
namespace automotive {
namespace qcarcam {
namespace V1_0 {
namespace implementation {

using android::hardware::Return;
using vendor::qti::automotive::qcarcam::V1_1::IQcarCamera;
using IQcarCameraStream_1_0 = ::vendor::qti::automotive::qcarcam::V1_0::IQcarCameraStream;
using IQcarCameraStream_1_1 = ::vendor::qti::automotive::qcarcam::V1_1::IQcarCameraStream;

class ais_hidl_stream; // from ais_hidl_stream.h

class ais_hidl_camera : public IQcarCamera {
public:

    ais_hidl_camera();
    ~ais_hidl_camera();

    Return<void> getInputStreamList(getInputStreamList_cb _hidl_cb)  override;
    Return<void> getInputStreamList_1_1(getInputStreamList_1_1_cb _hidl_cb)  override;
    Return<void> openStream(QcarcamInputDesc Desc, openStream_cb _hidl_cb)  override;
    Return<QcarcamError> closeStream(const ::android::sp<IQcarCameraStream_1_0>& camStream)  override;
    Return<void> openStream_1_1(QcarcamInputDesc Desc, openStream_1_1_cb _hidl_cb)  override;
    Return<QcarcamError> closeStream_1_1(const ::android::sp<IQcarCameraStream_1_1>& camStream)  override;
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

} //implementation
} //V1_0
} //qcarcam
} //automotive
} //qti
} //vendor

#endif //AIS_HIDL_CAMERA_H
