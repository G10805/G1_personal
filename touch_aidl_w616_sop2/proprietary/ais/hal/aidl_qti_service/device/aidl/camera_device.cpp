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

#define LOG_TAG "AidlService.cameraDevice"
#define LOG_NDEBUG 1
#include <log/log.h>

#include "include/camera_device.h"

#include <aidl/android/hardware/graphics/common/PixelFormat.h>
#include <cutils/native_handle.h>
#include <optional>
#include <utils/Errors.h>
#include <utils/Trace.h>
#include <utils/Vector.h>
#include <vector>

#include "include/convert.h"

namespace android {
namespace hardware {
namespace camera {
namespace device {
namespace implementation {

CameraDevice::CameraDevice(
    std::shared_ptr<CameraModule> module, const std::string& cameraId,
    const SortedVector<std::pair<std::string, std::string>>& cameraDeviceNames) :
        mModule(module),
        mCameraId(cameraId),
        mDisconnected(false),
        mCameraDeviceNames(cameraDeviceNames) {

    ALOGI("%s: cameraId: %s", __FUNCTION__, cameraId.c_str());

    mCameraIdInt = atoi(mCameraId.c_str());
    // Should not reach here as provider also validate ID
    if (mCameraIdInt < 0) {
        ALOGE("%s: Invalid camera id: %s", __FUNCTION__, mCameraId.c_str());
        mInitFail = true;
    } else if (mCameraIdInt >= mModule->getNumberOfCameras()) {
        ALOGI("%s: Adding a new camera id: %s", __FUNCTION__, mCameraId.c_str());
    }

    mDeviceVersion = mModule->getDeviceVersion(mCameraIdInt);
    if (mDeviceVersion < CAMERA_DEVICE_API_VERSION_3_2) {
        ALOGE("%s: Camera id %s does not support HAL3.2+",
                __FUNCTION__, mCameraId.c_str());
        mInitFail = true;
    }
    ALOGI("%s: cameraId: %s, this :%p", __FUNCTION__, cameraId.c_str(), this);

}

CameraDevice::~CameraDevice() {
    ALOGI("%s: ~CameraDevice", __FUNCTION__);
}

Status CameraDevice::initStatus() const {
    Mutex::Autolock _l(mLock);
    Status status = Status::OK;
    if (mInitFail) {
        status = Status::INTERNAL_ERROR;
    } else if (mDisconnected) {
        status = Status::CAMERA_DISCONNECTED;
    }
    return status;
}

void CameraDevice::setConnectionStatus(bool connected) {
    Mutex::Autolock _l(mLock);
    mDisconnected = !connected;
    if (mSession.expired()) {
        return;
    }

    std::shared_ptr<CameraDeviceSession> session = mSession.lock();
    if (session == nullptr) {
        return;
    }

    // Only notify active session disconnect events.
    // Users will need to re-open camera after disconnect event
    if (!connected) {
        session->disconnect();
    }
    return;
}

Status CameraDevice::getAidlStatus(int status) {
    switch (status) {
        case 0: return Status::OK;
        case -ENOSYS: return Status::OPERATION_NOT_SUPPORTED;
        case -EBUSY : return Status::CAMERA_IN_USE;
        case -EUSERS: return Status::MAX_CAMERAS_IN_USE;
        case -ENODEV: return Status::INTERNAL_ERROR;
        case -EINVAL: return Status::ILLEGAL_ARGUMENT;
        default:
            ALOGE("%s: unknown HAL status code %d", __FUNCTION__, status);
            return Status::INTERNAL_ERROR;
    }
}

// Methods from ::aidl::android::hardware::camera::device::ICameraDevice follow.
::ndk::ScopedAStatus CameraDevice::getResourceCost(CameraResourceCost* resource_cost) {
    if (nullptr == resource_cost)
    {
        return convertToScopedAStatus(Status::ILLEGAL_ARGUMENT);
    }

    Status status = initStatus();
    if (status == Status::OK) {
        int cost = 100;
        std::vector<std::string> conflicting_devices;
        struct camera_info info;

        // If using post-2.4 module version, query the cost + conflicting devices from the HAL
        if (mModule->getModuleApiVersion() >= CAMERA_MODULE_API_VERSION_2_4) {
            int ret = mModule->getCameraInfo(mCameraIdInt, &info);
            if (ret == OK) {
                cost = info.resource_cost;
                for (size_t i = 0; i < info.conflicting_devices_length; i++) {
                    std::string cameraId(info.conflicting_devices[i]);
                    for (const auto& pair : mCameraDeviceNames) {
                        if (cameraId == pair.first) {
                            conflicting_devices.push_back(pair.second);
                        }
                    }
                }
            } else {
                status = Status::INTERNAL_ERROR;
            }
        }

        if (status == Status::OK) {
            resource_cost->resourceCost = cost;
            resource_cost->conflictingDevices.resize(conflicting_devices.size());
            for (size_t i = 0; i < conflicting_devices.size(); i++) {
                resource_cost->conflictingDevices[i] = conflicting_devices[i];
                ALOGV("CamDevice %s is conflicting with camDevice %s",
                        mCameraId.c_str(), resource_cost->conflictingDevices[i].c_str());
            }
        }
    }

    return convertToScopedAStatus(status);
}

::ndk::ScopedAStatus CameraDevice::getCameraCharacteristics(CameraMetadata* characteristics_ret) {
    if (nullptr == characteristics_ret)
    {
        return convertToScopedAStatus(Status::ILLEGAL_ARGUMENT);
    }

    Status status = initStatus();
    if (status == Status::OK) {
        //Module 2.1+ codepath.
        struct camera_info info;
        int ret = mModule->getCameraInfo(mCameraIdInt, &info);
        if (ret == OK) {
            convertToAidl(info.static_camera_characteristics, characteristics_ret);
        } else {
            ALOGE("%s: get camera info failed!", __FUNCTION__);
            status = Status::INTERNAL_ERROR;
        }
    }

    return convertToScopedAStatus(status);
}

::ndk::ScopedAStatus CameraDevice::setTorchMode(bool in_on) {
    Status status = initStatus();
    if (!mModule->isSetTorchModeSupported(mCameraIdInt)) {
        status = Status::OPERATION_NOT_SUPPORTED;
    }

    if (status == Status::OK) {
        status = getAidlStatus(mModule->setTorchMode(mCameraId.c_str(), in_on));
    }

    return convertToScopedAStatus(status);
}

::ndk::ScopedAStatus CameraDevice::open(
    const std::shared_ptr<ICameraDeviceCallback>& in_callback,
    std::shared_ptr<ICameraDeviceSession>* session_ret)  {
    Status status = initStatus();
    std::shared_ptr<CameraDeviceSession> session = nullptr;

    if (in_callback == nullptr) {
        ALOGE("%s: cannot open camera %s. callback is null!",
                __FUNCTION__, mCameraId.c_str());

        *session_ret = nullptr;
        status = Status::ILLEGAL_ARGUMENT;
        return convertToScopedAStatus(status);
    }

    if (status != Status::OK) {
        // Provider will never pass initFailed device to client, so
        // this must be a disconnected camera
        ALOGE("%s: cannot open camera %s. camera is disconnected!",
                __FUNCTION__, mCameraId.c_str());

        *session_ret = nullptr;
        status = Status::CAMERA_DISCONNECTED;
        return convertToScopedAStatus(status);
    } else {
        mLock.lock();

        ALOGV("%s: Initializing device for camera %d", __FUNCTION__, mCameraIdInt);
        session = mSession.lock();
        if (session != nullptr && !session->isClosed()) {
            ALOGE("%s: cannot open an already opened camera!", __FUNCTION__);
            mLock.unlock();

            *session_ret = nullptr;
            status = Status::CAMERA_IN_USE;
            return convertToScopedAStatus(status);
        }

        /** Open HAL device */
        status_t res;
        camera3_device_t *device;

        ATRACE_BEGIN("camera3->open");
        res = mModule->open(mCameraId.c_str(),
                reinterpret_cast<hw_device_t**>(&device));
        ATRACE_END();

        if (res != OK) {
            ALOGE("%s: cannot open camera %s!", __FUNCTION__, mCameraId.c_str());
            mLock.unlock();

            *session_ret = nullptr;
            status = getAidlStatus(res);
            return convertToScopedAStatus(status);
        }

        /** Cross-check device version */
        if (device->common.version < CAMERA_DEVICE_API_VERSION_3_2) {
            ALOGE("%s: Could not open camera: "
                    "Camera device should be at least %x, reports %x instead",
                    __FUNCTION__,
                    CAMERA_DEVICE_API_VERSION_3_2,
                    device->common.version);
            device->common.close(&device->common);
            mLock.unlock();

            *session_ret = nullptr;
            status = Status::ILLEGAL_ARGUMENT;
            return convertToScopedAStatus(status);
        }

        struct camera_info info;
        res = mModule->getCameraInfo(mCameraIdInt, &info);
        if (res != OK) {
            ALOGE("%s: Could not open camera: getCameraInfo failed", __FUNCTION__);
            device->common.close(&device->common);
            mLock.unlock();

            *session_ret = nullptr;
            status = Status::ILLEGAL_ARGUMENT;
            return convertToScopedAStatus(status);
        }

        session = createSession(
                device, info.static_camera_characteristics, in_callback);
        if (session == nullptr) {
            ALOGE("%s: camera device session allocation failed", __FUNCTION__);
            mLock.unlock();

            *session_ret = nullptr;
            status = Status::INTERNAL_ERROR;
            return convertToScopedAStatus(status);
        }
        if (session->isInitFailed()) {
            ALOGE("%s: camera device session init failed", __FUNCTION__);
            session = nullptr;
            mLock.unlock();

            *session_ret = nullptr;
            status = Status::INTERNAL_ERROR;
            return convertToScopedAStatus(status);
        }
        mSession = session;

        mLock.unlock();
    }

    *session_ret = session;
    return convertToScopedAStatus(status);
}


binder_status_t CameraDevice::dump(int in_fd, const char** args, uint32_t numArgs) {
    Mutex::Autolock _l(mLock);

    if (mSession.expired()) {
        dprintf(in_fd, "No active camera device session instance\n");
        return STATUS_OK;
    }
    std::shared_ptr<CameraDeviceSession> session = mSession.lock();
    if (session == nullptr) {
        dprintf(in_fd, "No active camera device session instance\n");
        return STATUS_OK;
    }

    session->dumpState(in_fd);

    return STATUS_OK;
}

::ndk::ScopedAStatus CameraDevice::getPhysicalCameraCharacteristics(
    const std::string& in_physicalCameraId,
    CameraMetadata* physicalCameraCharacteristics) {

    Status status = initStatus();

    if(nullptr == physicalCameraCharacteristics){
        status = Status::ILLEGAL_ARGUMENT;
    }

    if (status == Status::OK) {
        // Require module 2.5+ version.
        if (mModule->getModuleApiVersion() < CAMERA_MODULE_API_VERSION_2_5) {
            ALOGE("%s: get_physical_camera_info must be called on camera module 2.5 or newer",
                    __FUNCTION__);
            status = Status::INTERNAL_ERROR;
        } else {
            char *end;
            errno = 0;
            long id = strtol(in_physicalCameraId.c_str(), &end, 0);
            if (id > INT_MAX || (errno == ERANGE && id == LONG_MAX) ||
                    id < INT_MIN || (errno == ERANGE && id == LONG_MIN) ||
                    *end != '\0') {
                ALOGE("%s: Invalid in_physicalCameraId %s", __FUNCTION__, in_physicalCameraId.c_str());
                status = Status::ILLEGAL_ARGUMENT;
            } else {
                camera_metadata_t *physicalInfo = nullptr;
                int ret = mModule->getPhysicalCameraInfo((int)id, &physicalInfo);
                if (ret == OK) {
                    convertToAidl(physicalInfo, physicalCameraCharacteristics);
                } else if (ret == -EINVAL) {
                    ALOGE("%s: %s is not a valid physical camera Id outside of getCameraIdList()",
                            __FUNCTION__, in_physicalCameraId.c_str());
                    status = Status::ILLEGAL_ARGUMENT;
                } else {
                    ALOGE("%s: Failed to get physical camera %s info: %s (%d)!", __FUNCTION__,
                            in_physicalCameraId.c_str(), strerror(-ret), ret);
                    status = Status::INTERNAL_ERROR;
                }
            }
        }
    }

    return convertToScopedAStatus(status);
}

::ndk::ScopedAStatus CameraDevice::isStreamCombinationSupported(const StreamConfiguration& in_streams, bool* isSupported) {
    Status status = Status::OK;
    if (nullptr == isSupported) {
        status = Status::ILLEGAL_ARGUMENT;
    }

    if (Status::OK == status)
    {
        *isSupported = false;

        camera_stream_combination_t streamComb{};
        streamComb.operation_mode = static_cast<uint32_t> (in_streams.operationMode);
        streamComb.num_streams = in_streams.streams.size();

        camera_stream_t *streamBuffer = new camera_stream_t[streamComb.num_streams];
        if (nullptr == streamBuffer)
        {
            ALOGE("%s: Memory allocation failed", __FUNCTION__);
            status = Status::INTERNAL_ERROR;
        }

        if (Status::OK == status)
        {
            size_t i = 0;
            for (const ::aidl::android::hardware::camera::device::Stream &it : in_streams.streams) {
                streamBuffer[i].stream_type = static_cast<int> (it.streamType);
                streamBuffer[i].width = it.width;
                streamBuffer[i].height = it.height;
                streamBuffer[i].format = static_cast<int> (it.format);
                streamBuffer[i].data_space = static_cast<android_dataspace_t> (it.dataSpace);
                streamBuffer[i].usage = static_cast<uint32_t> (it.usage);
                streamBuffer[i].physical_camera_id = it.physicalCameraId.c_str();
                streamBuffer[i].stream_use_case = static_cast<int> (it.useCase);
                streamBuffer[i++].rotation = static_cast<int> (it.rotation);
            }
            streamComb.streams = streamBuffer;
            int res = mModule->isStreamCombinationSupported(mCameraIdInt, &streamComb);
            switch (res) {
                case NO_ERROR:
                    *isSupported = true;
                    status = Status::OK;
                    break;
                case BAD_VALUE:
                    status = Status::OK;
                    break;
                case INVALID_OPERATION:
                    status = Status::OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    ALOGE("%s: Unexpected error: %d", __FUNCTION__, res);
                    status = Status::INTERNAL_ERROR;
            };
            delete [] streamBuffer;
        }
    }

    return convertToScopedAStatus(status);
}

std::shared_ptr<CameraDeviceSession> CameraDevice::createSession(camera3_device_t* device,
        const camera_metadata_t* deviceInfo,
        const std::shared_ptr<ICameraDeviceCallback>& callback) {
    return ndk::SharedRefBase::make<CameraDeviceSession>(device, deviceInfo, callback);
}

::ndk::ScopedAStatus CameraDevice::openInjectionSession(
    const std::shared_ptr<ICameraDeviceCallback>& in_callback,
    std::shared_ptr<ICameraInjectionSession>* session){
    if (nullptr == session) {
        return ::ndk::ScopedAStatus::fromServiceSpecificError(static_cast<int32_t>(Status::ILLEGAL_ARGUMENT));
    }
    *session = nullptr;

    return ::ndk::ScopedAStatus::fromServiceSpecificError(static_cast<int32_t>(Status::OPERATION_NOT_SUPPORTED));
}

::ndk::ScopedAStatus CameraDevice::turnOnTorchWithStrengthLevel(int32_t torch_strength){
    Status status = initStatus();

    if (status == Status::OK) {
        status = getAidlStatus(mModule->turnOnTorchWithStrengthLevel(mCameraId.c_str(), torch_strength));
    }

    return convertToScopedAStatus(status);
}

::ndk::ScopedAStatus CameraDevice::getTorchStrengthLevel(int32_t* strength_level){
    Status status = initStatus();

    if (status == Status::OK) {
        status = getAidlStatus(mModule->getTorchStrengthLevel(mCameraId.c_str(), strength_level));
    }

    return convertToScopedAStatus(status);
}


// End of methods from ::aidl::android::hardware::camera::device::ICameraDevice.

} // namespace implementation
}  // namespace device
}  // namespace camera
}  // namespace hardware
}  // namespace android
