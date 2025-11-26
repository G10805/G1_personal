/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef QTI_AUDIO_CONTROL_H
#define QTI_AUDIO_CONTROL_H

#include <AudioControl.h>
#include <binder/IBinder.h>
#ifdef ANDROID_AUDIO_CONTROL_HAL_AIDL
#include <android/binder_manager.h>
#include <binder/IServiceManager.h>
#endif

#ifdef ANDROID_U_HAL7
#include <android/hardware/audio/7.0/IDevice.h>
#include <android/hardware/audio/7.0/IDevicesFactory.h>
#include <android/hardware/audio/7.0/types.h>
#else
#include <android/hardware/audio/6.0/IDevice.h>
#include <android/hardware/audio/6.0/IDevicesFactory.h>
#include <android/hardware/audio/6.0/types.h>
#endif

#ifdef ANDROID_AUDIO_CONTROL_HAL_AIDL
#include <aidl/android/hardware/audio/core/IModule.h>
#include <aidl/qti/audio/core/VString.h>
#endif

using namespace android;
using namespace aidl::android::hardware::automotive::audiocontrol;
using std::string;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_handle;
using ::android::hardware::hidl_death_recipient;

#ifdef ANDROID_U_HAL7
using ::android::hardware::audio::V7_0::IDevice;
using ::android::hardware::audio::V7_0::IDevicesFactory;
using ::android::hardware::audio::V7_0::Result;
using ::android::hardware::audio::V7_0::ParameterValue;
#else
using ::android::hardware::audio::V6_0::IDevice;
using ::android::hardware::audio::V6_0::IDevicesFactory;
using ::android::hardware::audio::V6_0::Result;
using ::android::hardware::audio::V6_0::ParameterValue;
#endif

#ifdef ANDROID_AUDIO_CONTROL_HAL_AIDL
using aidl::android::hardware::audio::core::IModule;
using aidl::android::hardware::audio::core::VendorParameter;
using aidl::qti::audio::core::VString;
#endif

namespace vendor {
namespace qti {
namespace hardware {
namespace automotive {
namespace audiocontrol {
namespace aidl {

struct QtiAudioControl : public AudioControl, public IBinder::DeathRecipient {

    class AudioHalDeathRecipient : public hidl_death_recipient {
    public:
        AudioHalDeathRecipient(const sp<QtiAudioControl> audioControl) :
                mAudioControl(audioControl) {}
        virtual void serviceDied(uint64_t cookie, const wp<hidl::base::V1_0::IBase>& who);

    private:
        sp<QtiAudioControl> mAudioControl;
    };

    virtual void binderDied(const wp<IBinder> &the_late_who);

private:
#ifndef ANDROID_AUDIO_CONTROL_HAL_AIDL
    status_t parametersFromStr(const string& key,
                        const string& value,
                        hidl_vec<ParameterValue> *hidlParams);
#else
    status_t parametersFromStr(const std::string& key,
                        const std::string& value,
                        VendorParameter *param);
#endif
    status_t getDeviceInstance();
    void notifyAudioDevice(string& key, string& value);
    void handleAudioHalDeath();

        sp<IDevicesFactory> mDevicesFactory;
        sp<IDevice> mDevice;
        sp<AudioHalDeathRecipient> mAudioHalDeathRecipient;
#ifdef ANDROID_AUDIO_CONTROL_HAL_AIDL
        std::shared_ptr<IModule> mModule;
#endif

public:
    // Methods from ::android::hardware::automotive::audiocontrol::audioControl follow.
    ndk::ScopedAStatus onDevicesToDuckChange(const std::vector<DuckingInfo>& in_duckingInfos) override;
    ndk::ScopedAStatus onDevicesToMuteChange(const std::vector<MutingInfo>& in_mutingInfos) override;

    // Implementation details
    QtiAudioControl() :
        mAudioHalDeathRecipient(new AudioHalDeathRecipient(this)) {}

};

}  // namespace aidl
}  // namespace audiocontrol
}  // namespace automotive
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif  // QTI_AUDIO_CONTROL_H
