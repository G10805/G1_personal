/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "QtiAudioControl.h"

#include <log/log.h>

#define MAX_SLEEP_RETRY 100
#define AUDIO_CONTROL_SLEEP_WAIT 100 /* 100 ms */

namespace vendor {
namespace qti {
namespace hardware {
namespace automotive {
namespace audiocontrol {
namespace aidl {

#ifndef ANDROID_AUDIO_CONTROL_HAL_AIDL
status_t QtiAudioControl::getDeviceInstance() {

    if (mDevicesFactory == NULL) {
        mDevicesFactory = IDevicesFactory::getService();
        if (mDevicesFactory != NULL) {
            mDevicesFactory->linkToDeath(mAudioHalDeathRecipient, 0);
        } else {
            ALOGE("%s: Failed to obtan IDevicesFactory service", __func__);
            return FAILED_TRANSACTION;
        }
    }

    if (mDevice == 0) {
        Result retval = Result::NOT_INITIALIZED;
        Return<void> ret = mDevicesFactory->openPrimaryDevice(
                [&](Result r, const sp<IDevice>& result) {
                        retval = r;
                        if (retval == Result::OK)
                            mDevice = result;
                });
        if (ret.isOk()) {
            if (retval == Result::OK) return OK;
            else if (retval == Result::INVALID_ARGUMENTS) return BAD_VALUE;
            else return NO_INIT;
        }
        return FAILED_TRANSACTION;
    }

    return OK;
}

status_t QtiAudioControl::parametersFromStr(const string& key,
                                            const string& value,
                                            hidl_vec<ParameterValue> *hidlParams) {
    size_t idx = hidlParams->size();
    hidlParams->resize(idx + 1);
    (*hidlParams)[idx].key = key;
    (*hidlParams)[idx].value = value;
    return OK;
}
#else
status_t QtiAudioControl::getDeviceInstance() {

    auto serviceName = std::string(IModule::descriptor) + "/default";
    mModule = IModule::fromBinder(
            ndk::SpAIBinder(AServiceManager_waitForService(serviceName.c_str())));
    if (mModule == nullptr) {
        ALOGE("%s fromBinder %s failed", __func__, serviceName.c_str());
        return NO_INIT;
    }

    return OK;
}

status_t QtiAudioControl::parametersFromStr(const std::string& key,
                                            const std::string& value,
                                            VendorParameter *param) {
    VString parcel;
    param->id = key;
    parcel.value = value;
    if (param->ext.setParcelable(parcel) != android::OK) {
        ALOGD("%s : failed to set parcel for %s",__func__,parcel.descriptor);
    }
    return OK;
}
#endif

void QtiAudioControl::notifyAudioDevice(string& key, string& value) {
    status_t ret = OK;
    int retryCount = 0;

    do {
        if (getDeviceInstance() == OK) {
            ALOGI("%s: getDeviceInstance() OK", __func__);
            break;
        }
        else {
            if (++retryCount <= MAX_SLEEP_RETRY) {
                ALOGE("%s: Sleeping for 100ms", __func__);
                usleep(AUDIO_CONTROL_SLEEP_WAIT*1000);
            }
            else {
                ALOGE("%s: getDeviceInstance() FAILED", __func__);
                return;
            }
        }
    } while (1);
#ifndef ANDROID_AUDIO_CONTROL_HAL_AIDL
    hidl_vec<ParameterValue> hidlParams;
    ret = parametersFromStr(key, value, &hidlParams);
    if (ret != OK)
        return;
    mDevice->setParameters(NULL, hidlParams);
#else
    VendorParameter parameter;
    parametersFromStr(key,value, &parameter);
    mModule->setVendorParameters({parameter}, false);
#endif
}

void QtiAudioControl::handleAudioHalDeath() {
       int retryCount = 0;

        mDevicesFactory->unlinkToDeath(mAudioHalDeathRecipient);
        mDevicesFactory = NULL;
        mDevice = NULL;

        ALOGI("%s: Reconnecting to Audio HAL", __func__);
        do {
            if (getDeviceInstance() == OK) {
                ALOGI("%s: getDeviceInstance() OK", __func__);
                break;
            } else {
                if (++retryCount <= MAX_SLEEP_RETRY) {
                    ALOGE("%s: Sleeping for 100 ms", __func__);
                    usleep(AUDIO_CONTROL_SLEEP_WAIT*1000);
                } else {
                    ALOGE("%s: getDeviceInstance() FAILED", __func__);
                    return;
                }
            }
        } while (1);
        ALOGI("%s: Audio HAL Reconnected", __func__);
}

 void QtiAudioControl::binderDied(const wp<IBinder> &the_late_who __unused) {
    ALOGE("%s: Binder died, need to clean up", __func__);
}

ndk::ScopedAStatus QtiAudioControl::onDevicesToDuckChange(
        const std::vector<DuckingInfo>& in_duckingInfos) {
    string duckKey = "DevicesToDuck", duckValue = "";
    string unduckKey = "DevicesToUnduck", unduckValue = "";

    ALOGV("%s: Enter", __func__);
    for (const  DuckingInfo& duckingInfo: in_duckingInfos) {
        for (const auto& addressToDuck : duckingInfo.deviceAddressesToDuck) {
            duckValue += addressToDuck;
            duckValue += ",";
        }
        for (const auto& addressToUnduck : duckingInfo.deviceAddressesToUnduck) {
            unduckValue += addressToUnduck;
            unduckValue += ",";
        }
    }

    if (!duckValue.empty()) {
        ALOGI("%s: Addresses to duck: %s", __func__, duckValue.c_str());
        notifyAudioDevice(duckKey, duckValue);
    }
    if (!unduckValue.empty()) {
        ALOGI("%s: Addresses to unduck: %s", __func__, unduckValue.c_str());
        notifyAudioDevice(unduckKey, unduckValue);
    }

    ALOGV("%s: Exit", __func__);

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus QtiAudioControl::onDevicesToMuteChange(
        const std::vector<MutingInfo>& in_mutingInfos) {
    string muteKey = "DevicesToMute", muteValue = "";
    string unmuteKey = "DevicesToUnmute", unmuteValue = "";

    ALOGV("%s: Enter", __func__);
    for (const  MutingInfo& mutingInfo: in_mutingInfos) {
        for (const auto& addressToMute : mutingInfo.deviceAddressesToMute) {
            muteValue += addressToMute;
            muteValue += ",";
        }
        for (const auto& addressToUnmute : mutingInfo.deviceAddressesToUnmute) {
            unmuteValue += addressToUnmute;
            unmuteValue += ",";
        }
    }

    if (!muteValue.empty()) {
        ALOGI("%s: Addresses to mute: %s", __func__, muteValue.c_str());
        notifyAudioDevice(muteKey, muteValue);
    }
    if (!unmuteValue.empty()) {
        ALOGI("%s: Addresses to unmute: %s", __func__, unmuteValue.c_str());
        notifyAudioDevice(unmuteKey, unmuteValue);
    }

    ALOGV("%s: Exit", __func__);

    return ndk::ScopedAStatus::ok();
}

void QtiAudioControl::AudioHalDeathRecipient::serviceDied(uint64_t cookie __unused,
            const wp<hidl::base::V1_0::IBase>& who __unused) {
    ALOGI("%s: Audio HAL died", __func__);
    mAudioControl->handleAudioHalDeath();
}

}  // namespace aidl
}  // namespace audiocontrol
}  // namespace automotive
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
