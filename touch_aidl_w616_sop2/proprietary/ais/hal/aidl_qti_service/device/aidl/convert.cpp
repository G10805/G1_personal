/*
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


#define LOG_TAG "AidlService.Convert"
#include <log/log.h>

#include "include/convert.h"
#include <aidl/android/hardware/camera/device/ErrorCode.h>
#include <aidl/android/hardware/camera/device/ErrorMsg.h>
#include <aidl/android/hardware/camera/device/ShutterMsg.h>
#include <aidlcommonsupport/NativeHandle.h>

#include <cstdint>

namespace android {
namespace hardware {
namespace camera {
namespace device {
namespace implementation {

using ::aidl::android::hardware::graphics::common::PixelFormat;
using ::aidl::android::hardware::graphics::common::BufferUsage;

bool convertFromAidl(const CameraMetadata &src, const camera_metadata_t** dst) {
    if (src.metadata.size() == 0) {
        // Special case for null metadata
        *dst = nullptr;
        return true;
    }

    const uint8_t* data = src.metadata.data();
    // check that the size of CameraMetadata match underlying camera_metadata_t
    if (get_camera_metadata_size((camera_metadata_t*)data) != src.metadata.size()) {
        *dst = nullptr;
        ALOGE("%s: input CameraMetadata is corrupt!", __FUNCTION__);
        return false;
    }

    *dst = (camera_metadata_t*) data;
    return true;
}

// Note: Existing data in dst will be gone. Caller still owns the memory of src.
// Data is copied from src to dst.
void convertToAidl(const camera_metadata_t *src, CameraMetadata* dst) {
    if (src == nullptr) {
        return;
    }

    size_t size = get_camera_metadata_size(src);

    // Create a vector with copied src data
    std::vector<uint8_t> temp(
        &(reinterpret_cast<const uint8_t*>(src)[0]),
        &(reinterpret_cast<const uint8_t*>(src)[size]));

    // Move data to dst
    dst->metadata.swap(temp);
    return;
}

void convertFromAidl(const Stream &src, Camera3Stream* dst) {
    dst->mId = src.id;
    dst->stream_type = (int) src.streamType;
    dst->width = src.width;
    dst->height = src.height;
    dst->format = (int) src.format;
    dst->data_space = (android_dataspace_t) src.dataSpace;
    dst->rotation = (int) src.rotation;
    dst->usage = (uint32_t) src.usage;
    dst->stream_use_case = (uint64_t) src.useCase;
    dst->reserved[0] = NULL;
    dst->reserved[1] = NULL;

    dst->dynamic_range_profile = (int)src.dynamicRangeProfile;

    // Fields to be filled by HAL (max_buffers, priv) are initialized to 0
    dst->max_buffers = 0;
    dst->priv = 0;
    dst->physical_camera_id = nullptr;
    return;
}

void convertToAidl(const Camera3Stream* src, HalStream* dst) {
    dst->overrideDataSpace = ::aidl::android::hardware::graphics::common::Dataspace(src->data_space);
    dst->id = src->mId;
    dst->overrideFormat = (PixelFormat) src->format;
    dst->maxBuffers = src->max_buffers;
    if (src->stream_type == CAMERA3_STREAM_OUTPUT) {
        dst->consumerUsage = BufferUsage(0);
        dst->producerUsage = BufferUsage(src->usage);
    } else if (src->stream_type == CAMERA3_STREAM_INPUT) {
        dst->producerUsage = BufferUsage(0);
        dst->consumerUsage = BufferUsage(src->usage);
    } else {
        //Should not reach here per current AIDL spec, but we might end up adding
        // bi-directional stream to AIDL.
        ALOGW("%s: Stream type %d is not currently supported!",
                __FUNCTION__, src->stream_type);
    }

    HalStream* halStream = NULL;
    if (src->reserved[0] != NULL) {
        halStream = (HalStream*)(src->reserved[0]);
    } else if (src->reserved[1] != NULL) {
        halStream = (HalStream*)(src->reserved[1]);
    }

    // Check if overrideFormat is set and honor it
    if (halStream != NULL) {
        dst->overrideFormat = (PixelFormat) halStream->overrideFormat;
        if (src->stream_type == CAMERA3_STREAM_OUTPUT) {
            dst->producerUsage = BufferUsage(halStream->producerUsage);
        } else if (src->stream_type == CAMERA3_STREAM_INPUT) {
            dst->consumerUsage = BufferUsage(halStream->consumerUsage);
        }
    }
    dst->physicalCameraId = src->physical_camera_id;
}

void convertToAidl(const camera3_stream_configuration_t& src, std::vector<HalStream>& dst) {
    dst.resize(src.num_streams);
    for (uint32_t i = 0; i < src.num_streams; i++) {
        convertToAidl(static_cast<Camera3Stream*>(src.streams[i]), &dst[i]);
    }
    return;
}

void convertFromAidl(
        buffer_handle_t* bufPtr, BufferStatus status, camera3_stream_t* stream, int acquireFence,
        camera3_stream_buffer_t* dst) {
    dst->stream = stream;
    dst->buffer = bufPtr;
    dst->status = (int) status;
    dst->acquire_fence = acquireFence;
    dst->release_fence = -1; // meant for HAL to fill in
}

void convertToAidl(const camera3_notify_msg* src, NotifyMsg* dst) {
    switch (src->type) {
        case CAMERA3_MSG_ERROR:
            {
                // The camera3_stream_t* must be the same as what wrapper HAL passed to conventional
                // HAL, or the ID lookup will return garbage. Caller should validate the ID here is
                // indeed one of active stream IDs
                Camera3Stream* stream = static_cast<Camera3Stream*>(
                        src->message.error.error_stream);
                ::aidl::android::hardware::camera::device::ErrorMsg msg;
                msg.frameNumber = src->message.error.frame_number;
                msg.errorStreamId = (stream != nullptr) ? stream->mId : -1;
                msg.errorCode = (::aidl::android::hardware::camera::device::ErrorCode) src->message.error.error_code;
                dst->set<NotifyMsg::Tag::error>(msg);
            }
            break;
        case CAMERA3_MSG_SHUTTER:
        {
            ::aidl::android::hardware::camera::device::ShutterMsg msg;
            msg.frameNumber = src->message.shutter.frame_number;
            msg.timestamp = src->message.shutter.timestamp;
            msg.readoutTimestamp = src->message.shutter.readoutTimestamp;
            dst->set<NotifyMsg::Tag::shutter>(msg);
        }
            break;
        default:
            ALOGE("%s: AIDL type converion failed. Unknown msg type 0x%x",
                    __FUNCTION__, src->type);
    }
    return;
}

native_handle_t* convertFromAidl(const NativeHandle& src) {
    return ((src.fds.size() == 0) && (src.ints.size() == 0)) ? nullptr : ::android::makeFromAidl(src);
}

NativeHandle convertToAidl(const native_handle_t* src) {
    return ::android::makeToAidl(src);
}


::ndk::ScopedAStatus convertToScopedAStatus(Status status) {
    if (status == Status::OK) {
        return ndk::ScopedAStatus::ok();
    }

    return ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(
        static_cast<int32_t>(status),
        ::aidl::android::hardware::camera::common::toString(status).c_str());
}

}  // namespace implementation
}  // namespace device
}  // namespace camera
}  // namespace hardware
}  // namespace android
