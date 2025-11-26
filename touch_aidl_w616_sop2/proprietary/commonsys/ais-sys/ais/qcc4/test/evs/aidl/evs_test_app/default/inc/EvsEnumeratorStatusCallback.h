/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef CPP_EVS_SAMPLEDRIVER_AIDL_INCLUDE_EVSENUMERATORSTATUSCALLBACK_H
#define CPP_EVS_SAMPLEDRIVER_AIDL_INCLUDE_EVSENUMERATORSTATUSCALLBACK_H

#include <aidl/android/hardware/automotive/evs/IEvsEnumeratorStatusCallback.h>
#include <aidl/android/hardware/automotive/evs/BnEvsEnumeratorStatusCallback.h>

#include <vector>

using namespace ::aidl::android::hardware::automotive::evs;

class EvsEnumeratorStatusCallback : public BnEvsEnumeratorStatusCallback {
public:
    ::ndk::ScopedAStatus deviceStatusChanged(const std::vector<::aidl::android::hardware::automotive::evs::DeviceStatus>& in_status) override;

private:
    std::vector<DeviceStatus> mDeviceStatus;
    std::mutex mLock;
};


#endif  // CPP_EVS_SAMPLEDRIVER_AIDL_INCLUDE_EVSENUMERATORSTATUSCALLBACK_H
