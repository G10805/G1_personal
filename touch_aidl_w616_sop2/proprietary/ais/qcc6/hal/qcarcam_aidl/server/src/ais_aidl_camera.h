/* ===========================================================================
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#ifndef AIS_AIDL_CAMERA_H
#define AIS_AIDL_CAMERA_H

#include <aidl/vendor/qti/automotive/qcarcam2/IQcarCamera.h>
#include <aidl/vendor/qti/automotive/qcarcam2/IQcarCameraStream.h>
#include <aidl/vendor/qti/automotive/qcarcam2/BnQcarCamera.h>

#include "qcarcam_types.h"
#ifdef HAL_CAMERA_CPMS_SUPPORT
#include "PowerEvent.h"
#include "PowerPolicyService.h"
#endif
#include <thread>

namespace aidl {
namespace vendor {
namespace qti {
namespace automotive {
namespace qcarcam2 {

class ais_aidl_stream; // from ais_aidl_stream.h

class ais_aidl_camera : public BnQcarCamera {
public:

    ais_aidl_camera();
    ~ais_aidl_camera();

    ::ndk::ScopedAStatus openStream(const ::aidl::vendor::qti::automotive::qcarcam2::QCarCamOpenParam& in_OpenParam, std::shared_ptr<::aidl::vendor::qti::automotive::qcarcam2::IQcarCameraStream>* _aidl_return) override;
    ::ndk::ScopedAStatus getInputStreamList(std::vector<::aidl::vendor::qti::automotive::qcarcam2::QCarCamInput>* out_inputs, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) override;
    ::ndk::ScopedAStatus getInputStreamMode(int32_t in_inputId, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamInputModes* out_modes, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) override;
    ::ndk::ScopedAStatus closeStream(const std::shared_ptr<::aidl::vendor::qti::automotive::qcarcam2::IQcarCameraStream>& in_camStream, ::aidl::vendor::qti::automotive::qcarcam2::QCarCamError* _aidl_return) override;

#ifdef HAL_CAMERA_CPMS_SUPPORT
    static int PowerEventCb(pm_event_t power_event_id, void* pUsrCtxt);
#endif

private:
    int mQcarcamClientCnt;
    std::mutex mClientCntLock;
    std::thread mMonitorThread;
    void clientMonitorThread();
    std::atomic<bool> mMonitorThreadExit;
    uint32_t mNumInputs;
    QCarCamInput_t *mpQcarcamInput;
    QCarCamInputModes_t *mpQarcamModes;
#ifdef HAL_CAMERA_CPMS_SUPPORT
    std::thread mPowerEventThread;    // The Thread for the CPMS power Events
#endif
};

} // qcarcam2
} // automotive
} // qti
} // vendor
} // aidl

#endif //AIS_AIDL_CAMERA_H
