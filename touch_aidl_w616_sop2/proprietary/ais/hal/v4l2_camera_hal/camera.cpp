/*
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
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

// Modified from hardware/libhardware/modules/camera/Camera.cpp

//#define LOG_NDEBUG 0
#define MODULE_NAME "Camera"

#include <cstdlib>
#include <memory>
#include <vector>
#include <stdio.h>
#include <hardware/camera3.h>
#include <sync/sync.h>
#include <system/camera_metadata.h>
#include <system/graphics.h>
#include <utils/Mutex.h>
#include <time.h>

#include "common.h"
#include "metadata/metadata_common.h"

#define ATRACE_TAG (ATRACE_TAG_CAMERA | ATRACE_TAG_HAL)
#include <utils/Trace.h>

#include "camera.h"

#define CAMERA_SYNC_TIMEOUT 5000 // in msecs
#ifdef FPS_PRINT
static void GetSystemTime(unsigned long long* pnCurrentTime) {
    struct timespec t;
    if (pnCurrentTime) {
        clock_gettime(CLOCK_MONOTONIC, &t);
        // convert milliseconds
        *pnCurrentTime = (unsigned long long)((t.tv_sec * 1000) + (t.tv_nsec / 1000000));
    }
}
unsigned long long t_start = 0;
#endif

extern "C" {

    // Get handle to camera from device priv data
    default_camera_hal::Camera *camdev_to_camera(const camera3_device_t *dev)
    {
        return reinterpret_cast<default_camera_hal::Camera*>(dev->priv);
    }

    int initialize(const camera3_device_t *dev,
            const camera3_callback_ops_t *callback_ops)
    {
        return camdev_to_camera(dev)->initialize(callback_ops);
    }

    int configure_streams(const camera3_device_t *dev,
            camera3_stream_configuration_t *stream_list)
    {
        return camdev_to_camera(dev)->configureStreams(stream_list);
    }

    const camera_metadata_t *construct_default_request_settings(
            const camera3_device_t *dev, int type)
    {
        return camdev_to_camera(dev)->constructDefaultRequestSettings(type);
    }

    int process_capture_request(const camera3_device_t *dev,
            camera3_capture_request_t *request)
    {
        return camdev_to_camera(dev)->processCaptureRequest(request);
    }

    void dump(const camera3_device_t *dev, int fd)
    {
        camdev_to_camera(dev)->dump(fd);
    }

    int flush(const camera3_device_t *dev)
    {
        return camdev_to_camera(dev)->flush();
    }

} // extern "C"

namespace default_camera_hal {

extern "C" {
// Shim passed to the framework to close an opened device.
static int close_device(hw_device_t* dev)
{
    camera3_device_t* cam_dev = reinterpret_cast<camera3_device_t*>(dev);
    Camera* cam = static_cast<Camera*>(cam_dev->priv);
    return cam->close();
}
} // extern "C"

Camera::Camera(const config_port_metadata& config)
  : mId(config.port_idx),
    mconfig(config),
    mSettingsSet(false),
    mBusy(false),
    mCallbackOps(NULL),
    mInFlightTracker(new RequestTracker)
{
    memset(&mTemplates, 0, sizeof(mTemplates));
    memset(&mDevice, 0, sizeof(mDevice));
    mDevice.common.tag    = HARDWARE_DEVICE_TAG;
#if defined(AIDL_ANDROID_API)
    mDevice.common.version = CAMERA_DEVICE_API_VERSION_3_7;
#else
    mDevice.common.version = CAMERA_DEVICE_API_VERSION_3_4;
#endif
    mDevice.common.close  = close_device;
    mDevice.ops           = const_cast<camera3_device_ops_t*>(&sOps);
    mDevice.priv          = this;
#ifdef FPS_PRINT
    mFrameCnt = 0;
    GetSystemTime(&t_start);
#endif
}

Camera::~Camera()
{
}

int Camera::openDevice(const hw_module_t *module, hw_device_t **device)
{
    HAL_LOGV("Id:%d:Opening camera device", mId);
    ATRACE_CALL();
    android::Mutex::Autolock al(mDeviceLock);

    if (mBusy) {
        HAL_LOGE("Id:%d: Error! Camera device already opened", mId);
        return -EBUSY;
    }

    int connectResult = connect();
    if (connectResult != 0) {
      disconnect();
      return connectResult;
    }
    mBusy = true;
    mDevice.common.module = const_cast<hw_module_t*>(module);
    *device = &mDevice.common;
    HAL_LOGV("---------------x Id:%d", mId);
    return 0;
}

int Camera::getInfo(struct camera_info *info)
{
    info->device_version = mDevice.common.version;
    initDeviceInfo(info);
    if (!mStaticInfo) {
        int res = loadStaticInfo();
        if (res) {
            HAL_LOGE("Failed to load static info");
            return res;
        }
    }
    info->static_camera_characteristics = mStaticInfo->raw_metadata();
    info->facing = mStaticInfo->facing();
    info->orientation = mStaticInfo->orientation();
    HAL_LOGD("facing = %d, orientation = %d", info->facing, info->orientation);

    return 0;
}

int Camera::loadStaticInfo() {
  HAL_LOGE("----E");
  // Using a lock here ensures |mStaticInfo| will only ever be set once,
  // even in concurrent situations.
  android::Mutex::Autolock al(mStaticInfoLock);

  if (mStaticInfo) {
    return 0;
  }

  std::unique_ptr<android::hardware::camera::common::V1_0::helper::CameraMetadata> static_metadata =
      std::make_unique<android::hardware::camera::common::V1_0::helper::CameraMetadata>();
  int res = initStaticInfo(static_metadata.get());
  if (res) {
    HAL_LOGE("Id:%d: Failed to get static info from device.", mId);
    return res;
  }

  mStaticInfo.reset(StaticProperties::NewStaticProperties(
      std::move(static_metadata)));
  if (!mStaticInfo) {
    HAL_LOGE("Id:%d: Failed to initialize static properties from device metadata.",
          mId);
    return -ENODEV;
  }

  return 0;
}

int Camera::close()
{
    HAL_LOGE("Id:%d: Closing camera device", mId);
    ATRACE_CALL();
    android::Mutex::Autolock al(mDeviceLock);

    if (!mBusy) {
        HAL_LOGE("Id:%d: Error! Camera device not open", mId);
        return -EINVAL;
    }

    flush();
    disconnect();
    mBusy = false;
    return 0;
}

int Camera::initialize(const camera3_callback_ops_t *callback_ops)
{
    int res;

    HAL_LOGE("Id:%d: callback_ops=%p", mId, callback_ops);
    mCallbackOps = callback_ops;
    // per-device specific initialization
    res = initDevice();
    if (res != 0) {
        HAL_LOGE("Id:%d: Failed to initialize device!", mId);
        return res;
    }
    return 0;
}

int Camera::configureStreams(camera3_stream_configuration_t *stream_config)
{
    android::Mutex::Autolock al(mDeviceLock);

    HAL_LOGE("Id:%d: stream_config=%p", mId, stream_config);
    ATRACE_CALL();

    // Check that there are no in-flight requests.
    if (!mInFlightTracker->Empty()) {
        HAL_LOGE("Id:%d: Can't configure streams while frames are in flight.",
              mId);
        return -EINVAL;
    }

    // Verify the set of streams in aggregate, and perform configuration if valid.
    int res = validateStreamConfiguration(stream_config);
    if (res) {
        HAL_LOGE("Id:%d: Failed to validate stream set", mId);
    } else {
        // Set up all streams. Since they've been validated,
        // this should only result in fatal (-ENODEV) errors.
        // This occurs after validation to ensure that if there
        // is a non-fatal error, the stream configuration doesn't change states.
        res = setupStreams(stream_config);
        if (res) {
            HAL_LOGE("Id:%d: Failed to setup stream set", mId);
        }
    }

    // Set trackers based on result.
    if (!res) {
        // Success, set up the in-flight trackers for the new streams.
        mInFlightTracker->SetStreamConfiguration(*stream_config);
        // Must provide new settings for the new configuration.
        mSettingsSet = false;
    } else if (res != -EINVAL) {
        // Fatal error, the old configuration is invalid.
        mInFlightTracker->ClearStreamConfiguration();
    }
    // On a non-fatal error the old configuration, if any, remains valid.
    return res;
}

int Camera::validateStreamConfiguration(
    const camera3_stream_configuration_t* stream_config)
{
    HAL_LOGE("Id:%d E", mId);
    // Check that the configuration is well-formed.
    if (stream_config == nullptr) {
        HAL_LOGE("Id:%d: NULL stream configuration array", mId);
        return -EINVAL;
    } else if (stream_config->num_streams == 0) {
        HAL_LOGE("Id:%d: Empty stream configuration array", mId);
        return -EINVAL;
    } else if (stream_config->streams == nullptr) {
        HAL_LOGE("Id:%d: NULL stream configuration streams", mId);
        return -EINVAL;
    }

    // Check that the configuration is supported.
    // Make sure static info has been initialized before trying to use it.
    if (!mStaticInfo) {
        int res = loadStaticInfo();
        if (res) {
            return res;
        }
    }
    if (!mStaticInfo->StreamConfigurationSupported(stream_config)) {
        HAL_LOGE("Id:%d: Stream configuration does not match static "
              "metadata restrictions.", mId);
        return -EINVAL;
    }

    // Dataspace support is poorly documented - unclear if the expectation
    // is that a device supports ALL dataspaces that could match a given
    // format. For now, defer to child class implementation.
    // Rotation support isn't described by metadata, so must defer to device.
    if (!validateDataspacesAndRotations(stream_config)) {
        HAL_LOGE("Id:%d: Device can not handle configuration "
              "dataspaces or rotations.", mId);
        return -EINVAL;
    }

    return 0;
}

bool Camera::isValidTemplateType(int type)
{
    return type > 0 && type < CAMERA3_TEMPLATE_COUNT;
}

const camera_metadata_t* Camera::constructDefaultRequestSettings(int type)
{
    HAL_LOGE("Id:%d: type=%d", mId, type);

    if (!isValidTemplateType(type)) {
        HAL_LOGE("Id:%d: Invalid template request type: %d", mId, type);
        return NULL;
    }

    if (!mTemplates[type]) {
        // Check if the device has the necessary features
        // for the requested template. If not, don't bother.
        if (!mStaticInfo->TemplateSupported(type)) {
            HAL_LOGW("Id:%d: Camera does not support template type %d",
                  mId, type);
            return NULL;
        }

        // Initialize this template if it hasn't been initialized yet.
        std::unique_ptr<android::hardware::camera::common::V1_0::helper::CameraMetadata> new_template =
            std::make_unique<android::hardware::camera::common::V1_0::helper::CameraMetadata>();
        int res = initTemplate(type, new_template.get());
        if (res || !new_template) {
            HAL_LOGE("Id:%d: Failed to generate template of type: %d",
                  mId, type);
            return NULL;
        }
        mTemplates[type] = std::move(new_template);
    }

    // The "locking" here only causes non-const methods to fail,
    // which is not a problem since the CameraMetadata being locked
    // is already const. Destructing automatically "unlocks".
    return mTemplates[type]->getAndLock();
}

int Camera::processCaptureRequest(camera3_capture_request_t *temp_request)
{
    int res;
    // TODO(b/32917568): A capture request submitted or ongoing during a flush
    // should be returned with an error; for now they are mutually exclusive.
    android::Mutex::Autolock al(mFlushLock);

    ATRACE_CALL();

    if (temp_request == NULL) {
        HAL_LOGE("Id:%d: NULL request recieved", mId);
        return -EINVAL;
    }

    // Make a persistent copy of request, since otherwise it won't live
    // past the end of this method.
    std::shared_ptr<CaptureRequest> request = std::make_shared<CaptureRequest>(temp_request);

    HAL_LOGV("Id:%d: frame: %d", mId, request->frame_number);

    if (!mInFlightTracker->CanAddRequest(*request)) {
        // Streams are full or frame number is not unique.
        HAL_LOGE("Id:%d: Can not add request.", mId);
        return -EINVAL;
    }

    // Null/Empty indicates use last settings
    if (request->settings.isEmpty() && !mSettingsSet) {
        HAL_LOGE("Id:%d: NULL settings without previous set Frame:%d",
              mId, request->frame_number);
        return -EINVAL;
    }

    if (request->input_buffer != NULL) {
        HAL_LOGV("Id:%d: Reprocessing input buffer %p", mId,
              request->input_buffer.get());
    } else {
        HAL_LOGV("Id:%d: Capturing new frame.", mId);
    }

    if (!isValidRequestSettings(request->settings)) {
        HAL_LOGE("Id:%d: Invalid request settings.", mId);
        return -EINVAL;
    }

    // Pre-process output buffers.
    if (request->output_buffers.size() <= 0) {
        HAL_LOGE("Id:%d: Invalid number of output buffers: %zu", mId,
              request->output_buffers.size());
        return -EINVAL;
    }
    for (auto& output_buffer : request->output_buffers) {
        res = preprocessCaptureBuffer(&output_buffer);
        if (res) {
            completeRequestWithError(request);
            return -ENODEV;
        }
    }

    // Add the request to tracking.
    if (!mInFlightTracker->Add(request)) {
        HAL_LOGE("Id:%d: Failed to track request for frame %d.",
              mId, request->frame_number);
        return -ENODEV;
    }

    // Valid settings have been provided (mSettingsSet is a misnomer;
    // all that matters is that a previous request with valid settings
    // has been passed to the device, not that they've been set).
    mSettingsSet = true;

    // Send the request off to the device for completion.
    int ret = enqueueRequest(request);
    if (ret != 0) {
        mInFlightTracker->Remove(request);
        completeRequestWithError(request);
    }

    // Request is now in flight. The device will call completeRequest
    // asynchronously when it is done filling buffers and metadata.
    return 0;
}

void Camera::completeRequest(std::shared_ptr<CaptureRequest> request, int err)
{
    if (!mInFlightTracker->Remove(request)) {
        HAL_LOGE("Id:%d: Completed request %p is not being tracked. "
              "It may have been cleared out during a flush.",
              mId, request.get());
        return;
    }

    // Since |request| has been removed from the tracking, this method
    // MUST call sendResult (can still return a result in an error state, e.g.
    // through completeRequestWithError) so the frame doesn't get lost.

    if (err) {
      HAL_LOGE("Id:%d: Error completing request for frame %d.",
            mId, request->frame_number);
      completeRequestWithError(request);
      return;
    }

    // Notify the framework with the shutter time (extracted from the result).
    int64_t timestamp = 0;
    // TODO(b/31360070): The general metadata methods should be part of the
    // default_camera_hal namespace, not the v4l2_camera_hal namespace.
    int res = v4l2_camera_hal::SingleTagValue(
        request->settings, ANDROID_SENSOR_TIMESTAMP, &timestamp);
    if (res) {
        HAL_LOGE("Id:%d: Request for frame %d is missing required metadata.",
              mId, request->frame_number);
        // TODO(b/31653322): Send RESULT error.
        // For now sending REQUEST error instead.
        completeRequestWithError(request);
        return;
    }
    notifyShutter(request->frame_number, timestamp);

    for (unsigned int i = 0; i < request->output_buffers.size(); i++) {
        request->output_buffers[i].status = CAMERA3_BUFFER_STATUS_OK;
    }

    // TODO(b/31653322): Check all returned buffers for errors
    // (if any, send BUFFER error).

    sendResult(request);
}

int Camera::flush()
{
    HAL_LOGE("Id:%d: Flushing.", mId);
    // TODO(b/32917568): Synchronization. Behave "appropriately"
    // (i.e. according to camera3.h) if process_capture_request()
    // is called concurrently with this (in either order).
    // Since the callback to completeRequest also may happen on a separate
    // thread, this function should behave nicely concurrently with that too.
    android::Mutex::Autolock al(mFlushLock);

    flushBuffers();

    std::set<std::shared_ptr<CaptureRequest>> requests;
    mInFlightTracker->Clear(&requests);
    std::set<std::shared_ptr<CaptureRequest>, CaptureRequestComp> tmp(requests.begin(), requests.end());
    for (auto& request : tmp) {
        // TODO(b/31653322): See camera3.h. Should return different error
        // depending on status of the request.
        completeRequestWithError(request);
    }

    HAL_LOGV("Id:%d: Flushed %zu requests.", mId, requests.size());

    return 0;
}

int Camera::preprocessCaptureBuffer(camera3_stream_buffer_t *buffer)
{
    int res = 0;
    HAL_LOGV("Id:%d E", mId);
    // TODO(b/29334616): This probably should be non-blocking; part
    // of the asynchronous request processing.
    if (buffer->acquire_fence != -1) {
        res = sync_wait(buffer->acquire_fence, CAMERA_SYNC_TIMEOUT);
        if (res != 0) {
            if (errno == ETIME) {
                HAL_LOGE("Id:%d: Timeout waiting on buffer acquire fence", mId);
            } else {
                HAL_LOGE("Id:%d:Error waiting on buffer acquire fence: %s(%d), res:%d",
                                mId, strerror(errno), errno, res);
            }
            ::close(buffer->acquire_fence);
            return -1;
        }
        ::close(buffer->acquire_fence);
    }

    // Acquire fence has been waited upon.
    buffer->acquire_fence = -1;
    // No release fence waiting unless the device sets it.
    buffer->release_fence = -1;

    buffer->status = CAMERA3_BUFFER_STATUS_OK;
    return 0;
}

void Camera::notifyShutter(uint32_t frame_number, uint64_t timestamp)
{
    camera3_notify_msg_t message;
    memset(&message, 0, sizeof(message));
    message.type = CAMERA3_MSG_SHUTTER;
    message.message.shutter.frame_number = frame_number;
    message.message.shutter.timestamp = timestamp;
    mCallbackOps->notify(mCallbackOps, &message);
}

void Camera::completeRequestWithError(std::shared_ptr<CaptureRequest> request)
{
    // Send an error notification.
    camera3_notify_msg_t message;
    memset(&message, 0, sizeof(message));
    message.type = CAMERA3_MSG_ERROR;
    message.message.error.frame_number = request->frame_number;
    message.message.error.error_stream = nullptr;
    if (request->output_buffers[0].status & CAMERA3_BUFFER_STATUS_DEVICE_ERROR) {
        message.message.error.error_code = CAMERA3_MSG_ERROR_DEVICE;
    } else {
        message.message.error.error_code = CAMERA3_MSG_ERROR_REQUEST;
    }
    mCallbackOps->notify(mCallbackOps, &message);

    // TODO(b/31856611): Ensure all the buffers indicate their error status.
    for (unsigned int i = 0; i < request->output_buffers.size(); i++) {
        request->output_buffers[i].status = CAMERA3_BUFFER_STATUS_ERROR;
    }

    // Send the errored out result.
    sendResult(request);
}

void Camera::sendResult(std::shared_ptr<CaptureRequest> request) {
    // Fill in the result struct
    // (it only needs to live until the end of the framework callback).
    camera3_capture_result_t result {
        request->frame_number,
        request->settings.getAndLock(),
        static_cast<uint32_t>(request->output_buffers.size()),
        request->output_buffers.data(),
        request->input_buffer.get(),
        1  // Total result; only 1 part.
    };
    // Make the framework callback.
    mCallbackOps->process_capture_result(mCallbackOps, &result);
#ifdef FPS_PRINT
    if (request->output_buffers[0].status != CAMERA3_BUFFER_STATUS_ERROR) {
        uint32_t mAisFrameFPS = 30, diff_msec;
        unsigned long long cur_msec = 0;
        if (0 == mFrameCnt) {
            GetSystemTime(&mPrevMsec);
        }
        mFrameCnt++;
        // Will calculate number of frames for 5 seconds
        uint32_t numFrames = (uint32_t)mAisFrameFPS * 5;
        if (numFrames == mFrameCnt) {
            GetSystemTime(&cur_msec);
            diff_msec = cur_msec - mPrevMsec;
            if (diff_msec > (unsigned long long)5000) {
                double total_frame_rendered = (static_cast<double>(diff_msec)) * (static_cast<double>(mAisFrameFPS)/1000);
                double extra_frames = total_frame_rendered / static_cast<double>(numFrames);
                double fps = (double)mAisFrameFPS / extra_frames;
                HAL_LOGE("-----------FPS Report - %.1f sec-----------\n", ((float)(cur_msec - t_start) / 1000));
                HAL_LOGE("| id | qid |  fps |\n");
                HAL_LOGE("| %2u | %2u  | %4.2f |\n", mconfig.device_idx, mconfig.port_idx, fps);
            }
            else
                HAL_LOGE(" FPS is 30\n");
            // Reset frame count
            mFrameCnt = 0;
            // Save current time
            mPrevMsec = cur_msec;
        }
    }
#endif //FPS_PRINT
}

void Camera::dump(int fd)
{
    HAL_LOGV("Id:%d: Dumping to fd %d", mId, fd);
    ATRACE_CALL();
    android::Mutex::Autolock al(mDeviceLock);

    dprintf(fd, "Camera ID: %d (Busy: %d)\n", mId, mBusy);

    // TODO: dump all settings
}

const char* Camera::templateToString(int type)
{
    switch (type) {
    case CAMERA3_TEMPLATE_PREVIEW:
        return "CAMERA3_TEMPLATE_PREVIEW";
    case CAMERA3_TEMPLATE_STILL_CAPTURE:
        return "CAMERA3_TEMPLATE_STILL_CAPTURE";
    case CAMERA3_TEMPLATE_VIDEO_RECORD:
        return "CAMERA3_TEMPLATE_VIDEO_RECORD";
    case CAMERA3_TEMPLATE_VIDEO_SNAPSHOT:
        return "CAMERA3_TEMPLATE_VIDEO_SNAPSHOT";
    case CAMERA3_TEMPLATE_ZERO_SHUTTER_LAG:
        return "CAMERA3_TEMPLATE_ZERO_SHUTTER_LAG";
    }
    // TODO: support vendor templates
    return "Invalid template type!";
}

const camera3_device_ops_t Camera::sOps = {
    .initialize = ::initialize,
    .configure_streams = ::configure_streams,
    .register_stream_buffers = nullptr,
    .construct_default_request_settings
        = ::construct_default_request_settings,
    .process_capture_request = ::process_capture_request,
    .get_metadata_vendor_tag_ops = nullptr,
    .dump = ::dump,
    .flush = ::flush,
    .reserved = {0},
};

}  // namespace default_camera_hal
