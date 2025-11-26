/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "EvsEnumeratorStatusCallback.h"
#include <android-base/logging.h>

using ::ndk::ScopedAStatus;

::ndk::ScopedAStatus EvsEnumeratorStatusCallback::deviceStatusChanged(const std::vector<::aidl::android::hardware::automotive::evs::DeviceStatus>& in_status) {

    {
        std::lock_guard<std::mutex> lock(mLock);
        mDeviceStatus.clear();

        for (int i=0; i < (int)in_status.size(); i++) {
            mDeviceStatus.push_back(in_status[i]);

            // as of now its just informational log, customer can modify this part of the code to draw any overlays as per there requirement
            LOG(INFO) << "deviceStatusChanged :: Device Name = " << in_status[i].id << " status = " << (int)(in_status[i].status);
        }
    }

    return ScopedAStatus::ok();
}

