/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *Not a contribution.
 *
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VENDOR_QTI_CAMERA_DEVICE_CAMERADEVICE_H
#define VENDOR_QTI_CAMERA_DEVICE_CAMERADEVICE_H

#include <aidl/android/hardware/camera/device/BnCameraDevice.h>
#include <aidl/android/hardware/camera/device/ICameraDevice.h>
#include <aidl/android/hardware/camera/common/Status.h>
#include <aidl/android/hardware/camera/common/TorchModeStatus.h>
#include <utils/Mutex.h>

#include "camera_device_session.h"
#include "CameraMetadata.h"
#include "CameraModule.h"
#include "convert.h"

namespace android {
namespace hardware {
namespace camera {
namespace device {
namespace implementation {

using ::aidl::android::hardware::camera::common::CameraResourceCost;
using ::aidl::android::hardware::camera::common::Status;
using ::aidl::android::hardware::camera::device::BnCameraDevice;
using ::aidl::android::hardware::camera::device::CameraMetadata;
using ::aidl::android::hardware::camera::device::ICameraDeviceCallback;
using ::aidl::android::hardware::camera::device::ICameraDeviceSession;
using ::aidl::android::hardware::camera::device::ICameraInjectionSession;
using ::aidl::android::hardware::camera::device::StreamConfiguration;
using ::android::hardware::camera::common::V1_0::helper::CameraModule;

/*
 * The camera device HAL implementation is opened lazily (via the open call)
 */
struct CameraDevice : public BnCameraDevice {
    // Called by provider HAL. Provider HAL must ensure the uniqueness of
    // CameraDevice object per cameraId, or there could be multiple CameraDevice
    // trying to access the same physical camera.
    // Also, provider will have to keep track of all CameraDevice objects in
    // order to notify CameraDevice when the underlying camera is detached
    CameraDevice(std::shared_ptr<CameraModule> module,
                 const std::string& cameraId,
                 const SortedVector<std::pair<std::string, std::string>>& cameraDeviceNames);
    virtual ~CameraDevice();



    // Caller must use this method to check if CameraDevice ctor failed
    bool isInitFailed() { return mInitFail; }
    // Used by provider HAL to signal external camera disconnected
    void setConnectionStatus(bool connected);

    /* Methods from ::aidl::android::hardware::camera::device::ICameraDevice follow. */
    // The following method can be called without opening the actual camera device
    ::ndk::ScopedAStatus getResourceCost(CameraResourceCost* resource_cost) override;
    ::ndk::ScopedAStatus getCameraCharacteristics(CameraMetadata* characteristics_ret) override;
    ::ndk::ScopedAStatus setTorchMode(bool in_on) override;

    // Open the device HAL and also return a default capture session
    ::ndk::ScopedAStatus open(
        const std::shared_ptr<ICameraDeviceCallback>& in_callback,
        std::shared_ptr<ICameraDeviceSession>* session_ret) override;


    // Forward the dump call to the opened session, or do nothing
    binder_status_t dump(int in_fd, const char** args, uint32_t numArgs) override;

    ::ndk::ScopedAStatus openInjectionSession(
        const std::shared_ptr<ICameraDeviceCallback>& in_callback,
        std::shared_ptr<ICameraInjectionSession>* session) override;

    ::ndk::ScopedAStatus turnOnTorchWithStrengthLevel(int32_t torch_strength) override;

    ::ndk::ScopedAStatus getTorchStrengthLevel(int32_t* strength_level) override;

    /* End of Methods from ::aidl::android::hardware::camera::device::ICameraDevice */

protected:

    virtual std::shared_ptr<CameraDeviceSession> createSession(
        camera3_device_t*,
        const camera_metadata_t* deviceInfo,
        const std::shared_ptr<ICameraDeviceCallback>&);

    ::ndk::ScopedAStatus getPhysicalCameraCharacteristics(
        const std::string& in_physicalCameraId,
        CameraMetadata* physicalCameraCharacteristics) override;

    ::ndk::ScopedAStatus isStreamCombinationSupported(const StreamConfiguration& in_streams, bool* isSupported) override;

    const std::shared_ptr<CameraModule> mModule;
    const std::string mCameraId;
    // const after ctor
    int   mCameraIdInt;
    int   mDeviceVersion;
    bool  mInitFail = false;
    // Set by provider (when external camera is connected/disconnected)
    bool  mDisconnected;

    std::weak_ptr<CameraDeviceSession> mSession;

    const SortedVector<std::pair<std::string, std::string>>& mCameraDeviceNames;

    // gating access to mSession and mDisconnected
    mutable Mutex mLock;

    // convert conventional HAL status to AIDL Status
    static Status getAidlStatus(int);

    Status initStatus() const;

private:

};

}  // namespace implementation
}  // namespace device
}  // namespace camera
}  // namespace hardware
}  // namespace android

#endif  // VENDOR_QTI_CAMERA_DEVICE_CAMERADEVICE_H
