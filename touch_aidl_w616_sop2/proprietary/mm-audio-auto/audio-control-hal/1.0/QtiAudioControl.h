/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#ifndef QTI_AUDIO_CONTROL_H
#define QTI_AUDIO_CONTROL_H

#include <android/hardware/automotive/audiocontrol/1.0/IAudioControl.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace automotive {
namespace audiocontrol {
namespace V1_0 {


using namespace android::hardware::automotive::audiocontrol::V1_0;

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct QtiAudioControl : public IAudioControl {
public:
    // Methods from ::android::hardware::automotive::audiocontrol::V1_0::IAudioControl follow.
    Return<int32_t> getBusForContext(ContextNumber contextNumber) override;
    Return<void> setBalanceTowardRight(float value) override;
    Return<void> setFadeTowardFront(float value) override;

    // Implementation details
    QtiAudioControl();
};

}  // namespace V1_0
}  // namespace audiocontrol
}  // namespace automotive
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif  // QTI_AUDIO_CONTROL_H
