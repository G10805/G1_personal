/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
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

#ifndef VENDOR_QTI_CAMERA_PROVIDER_CAMERAPROVIDER_H
#define VENDOR_QTI_CAMERA_PROVIDER_CAMERAPROVIDER_H

#include <aidl/android/hardware/camera/provider/ICameraProvider.h>
#include <aidl/android/hardware/camera/provider/ICameraProviderCallback.h>
#include <aidl/android/hardware/camera/provider/ConcurrentCameraIdCombination.h>
#include <aidl/android/hardware/camera/common/Status.h>
#include <aidl/android/hardware/camera/provider/BnCameraProvider.h>
#include <aidl/android/hardware/camera/common/CameraMetadataType.h>
#include <map>

#include "camera_common.h"
#include "utils/Mutex.h"
#include "utils/SortedVector.h"

#include "CameraModule.h"
#include "VendorTagDescriptor.h"


namespace android {
namespace hardware {
namespace camera {
namespace provider {
namespace implementation {


using aidl::android::hardware::camera::common::Status;


// Default recommended RPC thread count for camera provider implementations
const int HWBINDER_THREAD_COUNT = 6;

using namespace ::android::hardware::camera::provider;

using ::aidl::android::hardware::camera::common::CameraDeviceStatus;
using ::aidl::android::hardware::camera::common::Status;
using ::aidl::android::hardware::camera::common::TorchModeStatus;
using ::aidl::android::hardware::camera::common::VendorTag;
using ::aidl::android::hardware::camera::common::VendorTagSection;
using ::aidl::android::hardware::camera::provider::ICameraProvider;
using ::aidl::android::hardware::camera::provider::BnCameraProvider;
using ::aidl::android::hardware::camera::provider::ICameraProviderCallback;
using ::aidl::android::hardware::camera::provider::ConcurrentCameraIdCombination;
using ::aidl::android::hardware::camera::device::ICameraDevice;
using ::aidl::android::hardware::camera::provider::CameraIdAndStreamCombination;
using ::android::hardware::camera::common::V1_0::helper::CameraModule;
using ::android::hardware::camera::common::V1_0::helper::VendorTagDescriptor;
using ::aidl::android::hardware::camera::common::CameraMetadataType;

using ::android::sp;
using ::android::Mutex;

/**
 * The implementation of legacy wrapper CameraProvider, separated
 * from the AIDL interface layer to allow for implementation reuse by later
 * provider versions.
 *
 * This implementation supports cameras implemented via the legacy libhardware
 * camera HAL definitions.
 */
struct CameraProvider : public camera_module_callbacks_t, public BnCameraProvider {
    CameraProvider();
    ~CameraProvider();

    static std::shared_ptr<CameraProvider> Create()
    {
        std::shared_ptr<CameraProvider> provider =
            ndk::SharedRefBase::make<CameraProvider>();

        return provider;
    }

    // Caller must use this method to check if CameraProvider ctor failed
    bool isInitFailed() { return mInitFailed; }

    // Methods from aidl::android::hardware::camera::provider::ICameraProvider follow.
    ::ndk::ScopedAStatus setCallback(const std::shared_ptr<ICameraProviderCallback>& in_callback) override;
    ::ndk::ScopedAStatus getVendorTags(std::vector<VendorTagSection>* _aidl_return) override;
    ::ndk::ScopedAStatus getCameraIdList(std::vector<std::string>* _aidl_return) override;

    ::ndk::ScopedAStatus getCameraDeviceInterface(const std::string& in_cameraDeviceName,
        std::shared_ptr<ICameraDevice>* device) override;

    ::ndk::ScopedAStatus notifyDeviceStateChange(int64_t in_deviceState) override;
    ::ndk::ScopedAStatus getConcurrentCameraIds(
        std::vector<ConcurrentCameraIdCombination>* concurrent_camera_ids) override;

    ::ndk::ScopedAStatus isConcurrentStreamCombinationSupported(
        const std::vector<CameraIdAndStreamCombination>& in_configs,
        bool* support) override;

protected:
    Mutex mCbLock;
    std::shared_ptr<ICameraProviderCallback> mCallbacks = nullptr;
    std::shared_ptr<CameraModule> mModule;

    int mNumberOfLegacyCameras;
    std::map<std::string, camera_device_status_t> mCameraStatusMap; // camera id -> status
    std::map<std::string, bool> mOpenLegacySupported; // camera id -> open_legacy HAL1.0 supported
    SortedVector<std::string> mCameraIds; // the "0"/"1" legacy camera Ids
    // (cameraId string, aidl device version) pairs
    SortedVector<std::pair<std::string, std::string>> mCameraDeviceNames;

    int mPreferredHal3MinorVersion;

    // Must be queried before using any APIs.
    // APIs will only work when this returns true
    bool mInitFailed;
    bool initialize();
    std::vector<VendorTagSection> mVendorTagSections;

    bool setUpVendorTags();
    int checkCameraVersion(int id, uint32_t device_version);

    // create AIDL device name from camera ID and device version
    std::string getAidlDeviceName(std::string cameraId, int deviceVersion);

    // convert conventional HAL status to AIDL Status
    static Status getAidlStatus(int);

    // static callback forwarding methods
    static void sCameraDeviceStatusChange(
        const struct camera_module_callbacks* callbacks,
        int camera_id,
        int new_status);
    static void sTorchModeStatusChange(
        const struct camera_module_callbacks* callbacks,
        const char* camera_id,
        int new_status);

    void addDeviceNames(int camera_id, CameraDeviceStatus status = CameraDeviceStatus::PRESENT,
                        bool cam_new = false);
    void removeDeviceNames(int camera_id);

    ::ndk::ScopedAStatus convertToScopedAStatus(Status status);
};


}  // namespace implementation
}  // namespace provider
}  // namespace camera
}  // namespace hardware
}  // namespace android

#endif  // VENDOR_QTI_CAMERA_PROVIDER_CAMERAPROVIDER_H
