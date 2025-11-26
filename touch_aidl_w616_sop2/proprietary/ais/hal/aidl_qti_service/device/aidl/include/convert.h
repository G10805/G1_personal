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

#ifndef VENDOR_QTI_CAMERA_DEVICE_DEFAULT_INCLUDE_CONVERT_H_
#define VENDOR_QTI_CAMERA_DEVICE_DEFAULT_INCLUDE_CONVERT_H_

#include <aidl/android/hardware/camera/common/Status.h>
#include <aidl/android/hardware/camera/device/BufferStatus.h>
#include <aidl/android/hardware/camera/device/CameraMetadata.h>
#include <aidl/android/hardware/camera/device/HalStream.h>
#include <aidl/android/hardware/camera/device/NotifyMsg.h>
#include <aidl/android/hardware/camera/device/Stream.h>
#include <aidl/android/hardware/common/NativeHandle.h>
#include <android/binder_interface_utils.h>
#include <set>

#include "camera3.h"

namespace android {
namespace hardware {
namespace camera {
namespace device {
namespace implementation {

using ::aidl::android::hardware::camera::common::Status;
using ::aidl::android::hardware::camera::device::BufferStatus;
using ::aidl::android::hardware::camera::device::CameraMetadata;
using ::aidl::android::hardware::camera::device::HalStream;
using ::aidl::android::hardware::camera::device::NotifyMsg;
using ::aidl::android::hardware::camera::device::Stream;
using ::aidl::android::hardware::common::NativeHandle;

// The camera3_stream_t sent to conventional HAL. Added mId fields to enable stream ID lookup
// fromt a downcasted camera3_stream
struct Camera3Stream : public camera3_stream {
    int mId;
};

// *dst will point to the data owned by src, but src still owns the data after this call returns.
bool convertFromAidl(const CameraMetadata &src, const camera_metadata_t** dst);
void convertToAidl(const camera_metadata_t* src, CameraMetadata* dst);

void convertFromAidl(const Stream &src, Camera3Stream* dst);
void convertToAidl(const Camera3Stream* src, HalStream* dst);

void convertFromAidl(
        buffer_handle_t*, BufferStatus, camera3_stream_t*, int acquireFence, // inputs
        camera3_stream_buffer_t* dst);

void convertToAidl(const camera3_stream_configuration_t& src, std::vector<HalStream>& dst);

// The camera3_stream_t* in src must be the same as what wrapper HAL passed to conventional
// HAL, or the ID lookup will return garbage. Caller should validate the ID in ErrorMsg is
// indeed one of active stream IDs
void convertToAidl(const camera3_notify_msg* src, NotifyMsg* dst);

// Convert NativeHandle to native_handle_t*, return nullptr if NativeHandle is null
native_handle_t* convertFromAidl(const NativeHandle& src);

NativeHandle convertToAidl(const native_handle_t* src);

::ndk::ScopedAStatus convertToScopedAStatus(Status status);

}  // namespace implementation
}  // namespace device
}  // namespace camera
}  // namespace hardware
}  // namespace android

#endif  // VENDOR_QTI_CAMERA_DEVICE_DEFAULT_INCLUDE_CONVERT_H_
