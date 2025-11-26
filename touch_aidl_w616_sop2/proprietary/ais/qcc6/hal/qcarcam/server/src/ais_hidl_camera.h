/* ===========================================================================
 * Copyright (c) 2019, 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#ifndef AIS_HIDL_CAMERA_H
#define AIS_HIDL_CAMERA_H

#include <vendor/qti/automotive/qcarcam/2.0/IQcarCamera.h>
#include <vendor/qti/automotive/qcarcam/2.0/IQcarCameraStream.h>
#include <thread>
#include "qcarcam_types.h"

namespace vendor {
namespace qti {
namespace automotive {
namespace qcarcam {
namespace V2_0 {
namespace implementation {

using vendor::qti::automotive::qcarcam::V2_0::QCarCamError;
using vendor::qti::automotive::qcarcam::V2_0::QCarCamOpenParam;
using android::hardware::Return;

class ais_hidl_stream; // from ais_hidl_stream.h

class ais_hidl_camera : public IQcarCamera {
public:

    ais_hidl_camera();
    ~ais_hidl_camera();

    Return<void> getInputStreamList(getInputStreamList_cb _hidl_cb)  override;
    Return<void> getInputStreamMode(uint32_t inputId, getInputStreamMode_cb _hidl_cb) override;
    Return<void> openStream(const QCarCamOpenParam& openParam, openStream_cb _hidl_cb)  override;
    Return<QCarCamError> closeStream(const ::android::sp<IQcarCameraStream>& camStream)  override;

private:
    int mQcarcamClientCnt;
    std::mutex mClientCntLock;
    std::thread mMonitorThread;
    void clientMonitorThread();
    std::atomic<bool> mMonitorThreadExit;
    uint32_t mNumInputs;
    QCarCamInput_t *mpQcarcamInput;
    QCarCamInputModes_t *mpQarcamModes;
};

} //implementation
} //V2_0
} //qcarcam
} //automotive
} //qti
} //vendor

#endif //AIS_HIDL_CAMERA_H
