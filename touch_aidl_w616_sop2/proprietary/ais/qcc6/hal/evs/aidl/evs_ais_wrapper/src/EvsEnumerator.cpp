/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a contribution.
 *
 * Copyright (C) 2022 The Android Open Source Project
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

#include "EvsEnumerator.h"
#include "EvsAISCamera.h"
#include "EvsGlDisplay.h"
#include "ConfigManager.h"
#include "ais_log.h"

#include <aidl/android/hardware/automotive/evs/Rotation.h>
#include <aidl/android/hardware/graphics/common/BufferUsage.h>

#include <dirent.h>
#include <hardware_legacy/uevent.h>
#include <sys/inotify.h>
#include <string_view>
#include <cutils/android_filesystem_config.h>
#ifdef HAL_CAMERA_CPMS_SUPPORT
#include <cutils/properties.h>
#endif

#ifdef HAL_CAMERA_CPMS_SUPPORT
#define HAL_SLEEP_USEC 1000000
#define HAL_DEFAULT_SLEEP_USEC 2
#endif

namespace {
using ::aidl::android::hardware::automotive::evs::DeviceStatusType;
using ::aidl::android::hardware::automotive::evs::EvsResult;
using ::aidl::android::hardware::automotive::evs::Rotation;
using ::aidl::android::hardware::graphics::common::BufferUsage;
using ::ndk::ScopedAStatus;
using std::chrono_literals::operator""s;
/* check if nececessary
using ::android::base::EqualsIgnoreCase;
using ::android::base::StringPrintf;
using ::android::base::WriteStringToFd;
*/
// Constants
/*
constexpr std::chrono::seconds kEnumerationTimeout = 10s;
constexpr std::string_view kDevicePath = "/dev/";
constexpr std::string_view kPrefix = "video";
constexpr size_t kEventBufferSize = 512;
*/
const std::set<uid_t> kAllowedUids = {AID_AUTOMOTIVE_EVS, AID_SYSTEM, AID_ROOT};
constexpr uint64_t kInvalidDisplayId = std::numeric_limits<uint64_t>::max();

} //namespace

namespace aidl::android::hardware::automotive::evs::implementation {


// NOTE:  All members values are static so that all clients operate on the same state
//        That is to say, this is effectively a singleton despite the fact that HIDL
//        constructs a new instance for each client.
std::unordered_map<std::string, EvsEnumerator::CameraRecord> EvsEnumerator::sCameraList;
std::weak_ptr<EvsGlDisplay>                                  EvsEnumerator::sActiveDisplay;
std::unique_ptr<ConfigManager>                               EvsEnumerator::sConfigManager;
sp<IAutomotiveDisplayProxyService>                           EvsEnumerator::sDisplayProxy;
uint64_t                                                     EvsEnumerator::sInternalDisplayId;
std::unordered_map<uint8_t, uint64_t>                        EvsEnumerator::sDisplayPortList;
std::mutex EvsEnumerator::sLock;

#ifdef HAL_CAMERA_CPMS_SUPPORT
std::mutex gSuspendLock;                // The Lock for the Suspend entry and exit Control
pm_event_t gPMStatus;
std::shared_ptr<PowerPolicyService> gPowerService;

static void PowerEventThread (void *args) {
    LOG(DEBUG) << "CameraPowerEventThread PowerEventInit";
    gPowerService = PowerEventInit(EvsEnumerator::PowerEventCb, args);
}

int EvsEnumerator::PowerEventCb(pm_event_t eventId, void* pUsrCtxt) {
    LOG(DEBUG) << "Entry PowerEventCb" << eventId;
    std::unordered_map<std::string, EvsEnumerator::CameraRecord> ::iterator itr = sCameraList.begin();
    EvsEnumerator *pEvs = (EvsEnumerator*)pUsrCtxt;

    char value[PROPERTY_VALUE_MAX];
    property_get("vendor.qcom.ais.power.sleep", value, "0");

    int sleepTime = 0;
    sleepTime = atoi(value);

    int sleepTimeUs = ((sleepTime > 0 && sleepTime <= 3) ?
        sleepTime : HAL_DEFAULT_SLEEP_USEC) * HAL_SLEEP_USEC;
    LOG(DEBUG) << "sleep Value : " << sleepTimeUs;

    std::unique_lock<std::mutex> lock(gSuspendLock);
    switch (eventId)
    {
        case AIS_PM_SUSPEND:
        {
            LOG(ERROR) << "PowerEventCb AIS_PM_SUSPEND ";
            if (gPMStatus == AIS_PM_SUSPEND) {
                LOG(ERROR) << "Status is already in the Suspend";
                break;
            }

            LOG(ERROR) << "PowerEventCb Starting sleep to allow App to close sessions";
            usleep(sleepTimeUs);
            LOG(ERROR) << "PowerEventCb Checking for any resource still alive and closing them";

            while(itr != sCameraList.end()) {
                CameraRecord *pRecord = &itr->second;
                LOG(ERROR) << "Going for the stop of the Camera :" << itr->first << pRecord;

                if(pRecord == nullptr || pRecord->activeInstance.expired()) {
                    LOG(ERROR) << "Current Camera instance is already closed by App" << pRecord;
                    itr++;
                    continue;
                }

                std::shared_ptr<EvsAISCamera> pActiveCamera = pRecord->activeInstance.lock();
                if (pActiveCamera != nullptr) {
                     LOG(ERROR) << "Found the record, stopping the camera";
                     //Going with this even on App Crash we will goto the Suspend but App need to establish new Connection on resume.
                     pEvs->closeCamera(pActiveCamera);
                } else {
                     LOG(ERROR) << "EVS App has been crashed or killed improperly";
                     std::unique_lock<std::mutex> lock(pEvs->mClientCntLock);
                     pEvs->mClientCnt = (pEvs->mClientCnt > 0)?(pEvs->mClientCnt-1):0;

                     if (0 == pEvs->mClientCnt) {
                         pEvs->evsQcarcamDeInit();
                     }
                     lock.unlock();
                }
                itr++;
            }

            if (sActiveDisplay.expired()) {
                 LOG(ERROR) << "Display is already Closed by App";
            } else {
                std::shared_ptr<EvsGlDisplay> pActiveDisplay = sActiveDisplay.lock();
                if (pActiveDisplay != nullptr) {
                     LOG(ERROR) << "Setting display State to DEAD for Suspend";
                     pEvs->closeDisplay(pActiveDisplay);
                }
            }

            gPMStatus = AIS_PM_SUSPEND;
            break;
        }
        case AIS_PM_RESUME:
        {
            LOG(ERROR) << "PowerEventCb AIS_PM_RESUME";
            if (gPMStatus == AIS_PM_RESUME) {
                LOG(ERROR) << "Status is already in the Resume";
                break;
            }
            pEvs->enumerateCameras(1); //lpm = 1 for enumerating the cameras back in the List

            gPMStatus = AIS_PM_RESUME;
            break;
        }
    }

    lock.unlock();
    LOG(DEBUG) << "Exit PowerEventCb " << eventId;
    return 0;
}
#endif

EvsEnumerator::EvsEnumerator(sp<IAutomotiveDisplayProxyService> proxyService) {
    LOG(DEBUG) << "EvsEnumerator is created.";

    if (sConfigManager == nullptr) {
        /* loads and initializes ConfigManager in a separate thread */
        sConfigManager =
            ConfigManager::Create("/vendor/etc/automotive/evs/evs_sample_configuration_ais.xml");
    }

    if (sDisplayProxy == nullptr) {
        /* sets a car-window service handle */
        sDisplayProxy = proxyService;
    }

    mQcarcamInitialized = false;
    enumerateCameras(1); //lpm==1
    enumerateDisplays();
    mClientCnt = 0;
    mC2dInitStatus = false;
#ifndef CPU_CONVERSION
    C2D_STATUS c2d_status = c2dDriverInit(NULL);
    if(c2d_status != C2D_STATUS_OK)
        LOG(ERROR) << "C2DDriverInit failed!!!!";
    else
        mC2dInitStatus = true;
#endif

#ifdef HAL_CAMERA_CPMS_SUPPORT
    /* On Startup of the EVS expected the Status be Resume */
    gPMStatus = AIS_PM_RESUME;
    mPowerEventThread = std::thread(PowerEventThread, this);
#endif

    int ready_status = 1;
    std::string ready_property_name = "vendor.evs.qcc6.hal.ready";
    int rc = 0;

    rc = property_set(ready_property_name.c_str(),std::to_string(ready_status).c_str());
    if (rc != 0)
    {
        LOG(ERROR) << "Not able to set vendor.evs.qcc6.hal.ready, evsmanagerd will not be started";
    }
    else
    {
        LOG(INFO) << "set vendor.evs.ais.qcc6.ready to 1, evsmanagerd will be launched";
    }
}
/* todo: check if applicable with AIS
void EvsEnumerator::EvsHotplugThread(std::shared_ptr<EvsEnumerator> service,
                                     std::atomic<bool>& running) {
    // Watch new video devices
    if (!service) {
        LOG(ERROR) << "EvsEnumerator is invalid";
        return;
    }

    auto notifyFd = inotify_init();
    if (notifyFd < 0) {
        LOG(ERROR) << "Failed to initialize inotify.  Exiting a thread loop";
        return;
    }

    int watchFd = inotify_add_watch(notifyFd, kDevicePath.data(), IN_CREATE | IN_DELETE);
    if (watchFd < 0) {
        LOG(ERROR) << "Failed to add a watch.  Exiting a thread loop";
        return;
    }

    LOG(INFO) << "Start monitoring new V4L2 devices";

    char eventBuf[kEventBufferSize] = {};
    while (running) {
        size_t len = read(notifyFd, eventBuf, sizeof(eventBuf));
        if (len < sizeof(struct inotify_event)) {
            // We have no valid event.
            continue;
        }

        size_t offset = 0;
        while (offset < len) {
            struct inotify_event* event =
                    reinterpret_cast<struct inotify_event*>(&eventBuf[offset]);
            offset += sizeof(struct inotify_event) + event->len;
            if (event->wd != watchFd || strncmp(kPrefix.data(), event->name, kPrefix.size())) {
                continue;
            }

            std::string deviceName = std::string(kDevicePath) + std::string(event->name);
            if (event->mask & IN_CREATE) {
                if (addCaptureDevice(deviceName)) {
                    service->notifyDeviceStatusChange(deviceName,
                                                      DeviceStatusType::CAMERA_AVAILABLE);
                }
            }

            if (event->mask & IN_DELETE) {
                if (removeCaptureDevice(deviceName)) {
                    service->notifyDeviceStatusChange(deviceName,
                                                      DeviceStatusType::CAMERA_NOT_AVAILABLE);
                }
            }
        }
    }
}
bool EvsEnumerator::addCaptureDevice(const std::string& deviceName) {
    if (!qualifyCaptureDevice(deviceName.data())) {
        LOG(DEBUG) << deviceName << " is not qualified for this EVS HAL implementation";
        return false;
    }

    CameraRecord cam(deviceName.data());
    if (sConfigManager) {
        std::unique_ptr<ConfigManager::CameraInfo>& camInfo =
                sConfigManager->getCameraInfo(deviceName);
        if (camInfo) {
            uint8_t* ptr = reinterpret_cast<uint8_t*>(camInfo->characteristics);
            const size_t len = get_camera_metadata_size(camInfo->characteristics);
            cam.desc.metadata.insert(cam.desc.metadata.end(), ptr, ptr + len);
        }
    }

    {
        std::lock_guard lock(sLock);
        // insert_or_assign() returns std::pair<std::unordered_map<>, bool>
        auto result = sCameraList.insert_or_assign(deviceName, std::move(cam));
        LOG(INFO) << deviceName << (std::get<1>(result) ? " is added" : " is modified");
    }

    return true;
}

bool EvsEnumerator::removeCaptureDevice(const std::string& deviceName) {
    std::lock_guard lock(sLock);
    if (sCameraList.erase(deviceName) != 0) {
        LOG(INFO) << deviceName << " is removed";
        return true;
    }

    return false;
}
*/


bool EvsEnumerator::checkPermission() {
    const auto uid = AIBinder_getCallingUid();
    if (kAllowedUids.find(uid) == kAllowedUids.end()) {
        LOG(ERROR) << "EVS access denied: "
                   << "pid = " << AIBinder_getCallingPid() << ", uid = " << uid;
        return false;
    }

    return true;
}



void EvsEnumerator::enumerateCameras(int lpm) {

    int ret = 0;
    ret = evsQcarcamInit();
    if (ret == QCARCAM_RET_OK)
    {
        LOG(DEBUG) << "evsQcarcamInit successfully";
        ret = evsQcarcamQueryInputs();
        if (ret != QCARCAM_RET_OK)
            LOG(ERROR) << "evsQcarcamQueryInputs failed ret = "<< ret;
    }
    else
        LOG(ERROR) << "evsQcarcamInit failed "<< ret;
    // Deinit for LPM
    if (lpm)
        evsQcarcamDeInit();
}


uint64_t EvsEnumerator::enumerateDisplays() {
    LOG(INFO) << __FUNCTION__ << ": Starting display enumeration";
    uint64_t internalDisplayId = kInvalidDisplayId;
    if (!sDisplayProxy) {
        LOG(ERROR) << "IAutomotiveDisplayProxyService is not available!";
        return internalDisplayId;
    }

    sDisplayProxy->getDisplayIdList(
      [](const auto& displayIds) {
          // The first entry of the list is the internal display.  See
          // SurfaceFlinger::getPhysicalDisplayIds() implementation.
          if (displayIds.size() > 0) {
              sInternalDisplayId = displayIds[0];
              for (const auto& id : displayIds) {
                  const auto port = id & 0xF;
                  LOG(INFO) << "Display " << std::hex << id
                            << " is detected on the port, " << port;
                  sDisplayPortList.insert_or_assign(port, id);
              }
          }
      }
    );

    LOG(INFO) << "Found " << sDisplayPortList.size() << " displays";
    return sInternalDisplayId;
}


// Methods from ::android::hardware::automotive::evs::IEvsEnumerator follow.
ScopedAStatus EvsEnumerator::getCameraList(std::vector<CameraDesc>* _aidl_return) {
    LOG(DEBUG) << __FUNCTION__;
    if (!checkPermission()) {
        return ScopedAStatus::fromServiceSpecificError(
                static_cast<int>(EvsResult::PERMISSION_DENIED));
    }

    {
        if (sCameraList.size() < 1) {
            // Retry enumarating cameras
            enumerateCameras(1); //lpm==1
        }
    }


    // Build up a packed array of CameraDesc for return
    _aidl_return->resize(sCameraList.size());
    unsigned i = 0;
    for (const auto& [key, cam] : sCameraList) {
        (*_aidl_return)[i++] = cam.desc;
    }

    if (sConfigManager && sCameraList.size() > 0) {
        // Adding camera groups that represent logical camera devices
        auto camGroups = sConfigManager->getCameraGroupIdList();
        for (auto&& id : camGroups) {
            if (sCameraList.find(id) != sCameraList.end()) {
                // Already exists in the _aidl_return
                continue;
            }

            std::unique_ptr<ConfigManager::CameraGroupInfo>& tempInfo =
                    sConfigManager->getCameraGroupInfo(id);
            CameraRecord cam(id.data());
            if (tempInfo) {
                uint8_t* ptr = reinterpret_cast<uint8_t*>(tempInfo->characteristics);
                const size_t len = get_camera_metadata_size(tempInfo->characteristics);
                cam.desc.metadata.insert(cam.desc.metadata.end(), ptr, ptr + len);
            }

            sCameraList.insert_or_assign(id, cam);
            _aidl_return->push_back(cam.desc);
        }
    }

    // Send back the results
    LOG(DEBUG) << "Reporting " << sCameraList.size() << " cameras available";
    return (sCameraList.size() > 0) ? (ScopedAStatus::ok())
    :(ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::INVALID_ARG)));
}

ScopedAStatus EvsEnumerator::getStreamList(const CameraDesc& desc,
                                           std::vector<Stream>* _aidl_return) {
    using AidlPixelFormat = ::aidl::android::hardware::graphics::common::PixelFormat;

    camera_metadata_t* pMetadata = const_cast<camera_metadata_t*>(
            reinterpret_cast<const camera_metadata_t*>(desc.metadata.data()));
    camera_metadata_entry_t streamConfig;
    if (!find_camera_metadata_entry(pMetadata, ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS,
                                    &streamConfig)) {
        const unsigned numStreamConfigs = streamConfig.count / sizeof(StreamConfiguration);
        _aidl_return->resize(numStreamConfigs);
        const StreamConfiguration* pCurrentConfig =
                reinterpret_cast<StreamConfiguration*>(streamConfig.data.i32);
        for (unsigned i = 0; i < numStreamConfigs; ++i, ++pCurrentConfig) {
            // Build ::aidl::android::hardware::automotive::evs::Stream from
            // StreamConfiguration.
            Stream current = {
                    .id = pCurrentConfig->inputId,
                    .streamType = pCurrentConfig->type ==
                                    ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_INPUT
                            ? StreamType::INPUT
                            : StreamType::OUTPUT,
                    .width = pCurrentConfig->width,
                    .height = pCurrentConfig->height,
                    .format = static_cast<AidlPixelFormat>(pCurrentConfig->format),
                    .usage = BufferUsage::CAMERA_INPUT,
                    .rotation = Rotation::ROTATION_0,
            };

            (*_aidl_return)[i] = std::move(current);
        }
    }

    return ScopedAStatus::ok();
}

ScopedAStatus EvsEnumerator::openCamera(const std::string& id, [[maybe_unused]] const Stream& cfg,
                                        std::shared_ptr<IEvsCamera>* obj) {
    LOG(DEBUG) << __FUNCTION__ <<" "<< id;
    if (!checkPermission()) {
        return ScopedAStatus::fromServiceSpecificError(
                static_cast<int>(EvsResult::PERMISSION_DENIED));
    }

    int ret = 0;

    std::unique_lock<std::mutex> lock(mClientCntLock);
    // Initialize qcarcam only if its first client
    if (0 == mClientCnt) {
        if (sCameraList.size() < 1) {
            // Retry enumarating cameras
            enumerateCameras(0); //lpm==0 as its first client
        } else {
            ret = evsQcarcamInit();
            if (ret == QCARCAM_RET_OK)
                LOG(DEBUG) << "evsQcarcamInit successfully";
            else {
                LOG(ERROR) << "evsQcarcamInit failed ret = "<<ret;
                lock.unlock();
                return ScopedAStatus::fromServiceSpecificError(
                    static_cast<int>(EvsResult::UNDERLYING_SERVICE_ERROR));
            }
        }
    }

    // Is this a recognized camera id?
    CameraRecord *pRecord = findCameraById(id);
    if (pRecord == nullptr) {
        LOG(ERROR) << id << " does not exist!";
        // Uninitialize qcarcam only if its first client
        if (0 == mClientCnt)
            evsQcarcamDeInit();
        lock.unlock();
        return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::INVALID_ARG));
    }

    // Has this camera already been instantiated by another caller?
    std::shared_ptr<EvsAISCamera> pActiveCamera = pRecord->activeInstance.lock();
    if (pActiveCamera != nullptr) {
        LOG(WARNING) << "Killing previous camera because of new caller";
        mClientCnt = (mClientCnt > 0)?(mClientCnt-1):0;
        closeCamera_impl(pActiveCamera);
    }

    // Construct a camera instance for the caller
    if (sConfigManager == nullptr) {
        LOG(ERROR) << "ConfigManager is not available.  Given stream configuration is ignored.";
        pActiveCamera = EvsAISCamera::Create(id.data(),
                                             (void*)mpStreamConfigs,
                                             mNumAisInputs,
                                             mC2dInitStatus, this);
    } else {
        pActiveCamera = EvsAISCamera::Create(id.data(),
                                             (void*)mpStreamConfigs,
                                             mNumAisInputs,
                                             mC2dInitStatus,
                                             sConfigManager->getCameraInfo(id), this);
    }

    pRecord->activeInstance = pActiveCamera;
    if (pActiveCamera == nullptr) {
        LOG(ERROR) << "Failed to create new EvsAISCamera object for " << id;
        // Uninitialize qcarcam only if its first client
        if (0 == mClientCnt)
            evsQcarcamDeInit();
        lock.unlock();
        return ScopedAStatus::fromServiceSpecificError(
                static_cast<int>(EvsResult::UNDERLYING_SERVICE_ERROR));
    }

    mClientCnt++;
    lock.unlock();
    *obj = pActiveCamera;
    return ScopedAStatus::ok();
}


ScopedAStatus EvsEnumerator::closeCamera(const std::shared_ptr<IEvsCamera>& cameraObj) {
    LOG(DEBUG) << __FUNCTION__;

    if (!cameraObj) {
        LOG(ERROR) << "Ignoring call to closeCamera with null camera ptr";
        return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::INVALID_ARG));
    }

    closeCamera_impl(cameraObj);

    std::unique_lock<std::mutex> lock(mClientCntLock);
    mClientCnt = (mClientCnt > 0)?(mClientCnt-1):0;
    // Uninitialize qcarcam only if its last client
    if (0 == mClientCnt) {
        evsQcarcamDeInit();
    }
    lock.unlock();

    return ScopedAStatus::ok();
}


ScopedAStatus EvsEnumerator::openDisplay(int32_t id, std::shared_ptr<IEvsDisplay>* displayObj) {
    LOG(DEBUG) << __FUNCTION__;
    if (!checkPermission()) {
        return ScopedAStatus::fromServiceSpecificError(
                static_cast<int>(EvsResult::PERMISSION_DENIED));
    }

    // If we already have a display active, then we need to shut it down so we can
    // give exclusive access to the new caller.
    std::shared_ptr<EvsGlDisplay> pActiveDisplay = sActiveDisplay.lock();
    if (pActiveDisplay != nullptr) {
        LOG(WARNING) << "Killing previous display because of new caller";
        closeDisplay(pActiveDisplay);
    }

    // Create a new display interface and return it
    if (sDisplayPortList.find(id) == sDisplayPortList.end()) {
        LOG(ERROR) << "No display is available on the port " << static_cast<int32_t>(id);
        return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::INVALID_ARG));
    }

    // Create a new display interface and return it.
    pActiveDisplay = ::ndk::SharedRefBase::make<EvsGlDisplay>(sDisplayProxy, sInternalDisplayId);
    sActiveDisplay = pActiveDisplay;

    LOG(DEBUG) << "Returning new EvsGlDisplay object " << pActiveDisplay.get();
    *displayObj = pActiveDisplay;
    return ScopedAStatus::ok();
}


ScopedAStatus EvsEnumerator::closeDisplay(const std::shared_ptr<IEvsDisplay>& obj) {
    LOG(DEBUG) << __FUNCTION__;

    // Do we still have a display object we think should be active?
    std::shared_ptr<EvsGlDisplay> pActiveDisplay = sActiveDisplay.lock();
    if (!pActiveDisplay) {
        LOG(ERROR) << "Somehow a display is being destroyed "
                   << "when the enumerator didn't know one existed";
        return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::OWNERSHIP_LOST));
    } else if (pActiveDisplay != obj) {
        LOG(WARNING) << "Ignoring close of previously orphaned display - why did a client steal?";
    } else {
        // Drop the active display
        pActiveDisplay->forceShutdown();
    }

    return ScopedAStatus::ok();
}


ScopedAStatus EvsEnumerator::getDisplayState(DisplayState* state) {
    LOG(DEBUG) << __FUNCTION__;
    if (!checkPermission()) {
        *state = DisplayState::DEAD;
        return ScopedAStatus::fromServiceSpecificError(
                static_cast<int>(EvsResult::PERMISSION_DENIED));
    }

    // Do we still have a display object we think should be active?
    std::shared_ptr<IEvsDisplay> pActiveDisplay = sActiveDisplay.lock();
    if (pActiveDisplay) {
        return pActiveDisplay->getDisplayState(state);
    } else {
        *state = DisplayState::NOT_OPEN;
        return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::OWNERSHIP_LOST));
    }
}


ScopedAStatus EvsEnumerator::getDisplayIdList(std::vector<uint8_t>* list) {
    std::vector<uint8_t>& output = *list;

    if (sDisplayPortList.size() > 0) {
        output.resize(sDisplayPortList.size());
        unsigned i = 0;
        output[i++] = sInternalDisplayId & 0xF;
        for (const auto& [port, id] : sDisplayPortList) {
            if (sInternalDisplayId != id) {
                output[i++] = port;
            }
        }
    }

    return ScopedAStatus::ok();
}

ScopedAStatus EvsEnumerator::isHardware(bool* flag) {
    *flag = true;
    return ScopedAStatus::ok();
}

void EvsEnumerator::notifyDeviceStatusChange(const std::string_view& deviceName,
                                             DeviceStatusType type) {
    std::lock_guard lock(sLock);
    if (!mCallback) {
        return;
    }

    std::vector<DeviceStatus> status {{ .id = std::string(deviceName), .status = type }};
    if (!mCallback->deviceStatusChanged(status).isOk()) {
        LOG(WARNING) << "Failed to notify a device status change, name = " << deviceName
                     << ", type = " << static_cast<int>(type);
    }
}

ScopedAStatus EvsEnumerator::registerStatusCallback(
        const std::shared_ptr<IEvsEnumeratorStatusCallback>& callback) {
    std::lock_guard lock(sLock);
    if (mCallback) {
        LOG(INFO) << "Replacing an existing device status callback";
    }
    mCallback = callback;
    return ScopedAStatus::ok();
}



void EvsEnumerator::closeCamera_impl(const std::shared_ptr<IEvsCamera>& pCamera) {
    // Get the camera id so we can find it in our list
    CameraDesc desc;
    auto status = pCamera->getCameraInfo(&desc);
    if (!status.isOk()) {
        LOG(ERROR) << "Failed to read a camera descriptor";
        return;
    }
    auto cameraId = desc.id;
    LOG(DEBUG) << __FUNCTION__ <<" "<< cameraId;

    // Find the named camera
    CameraRecord* pRecord = findCameraById(cameraId);

    // Is the display being destroyed actually the one we think is active?
    if (!pRecord) {
        LOG(ERROR) << "Asked to close a camera whose name isn't recognized";
    } else {
        std::shared_ptr<EvsAISCamera> pActiveCamera = pRecord->activeInstance.lock();
        if (!pActiveCamera) {
            LOG(WARNING) << "Somehow a camera is being destroyed "
                         << "when the enumerator didn't know one existed";
        } else if (pActiveCamera != pCamera) {
            // This can happen if the camera was aggressively reopened,
            // orphaning this previous instance
            LOG(WARNING) << "Ignoring close of previously orphaned camera "
                         << "- why did a client steal?";
        } else {
            // Shutdown the active camera
            pActiveCamera->shutdown();
            pRecord->activeInstance.reset();
        }
    }

    return;
}


EvsEnumerator::CameraRecord* EvsEnumerator::findCameraById(const std::string& cameraId) {
    // Find the named camera
    auto found = sCameraList.find(cameraId);
    if (sCameraList.end() != found) {
        // Found a match!
        return &found->second;
    }

    // We didn't find a match
    return nullptr;
}


ScopedAStatus EvsEnumerator::getUltrasonicsArrayList(
        [[maybe_unused]] std::vector<UltrasonicsArrayDesc>* list) {
    // TODO(b/149874793): Add implementation for EVS Manager and Sample driver
    return ScopedAStatus::ok();
}

ScopedAStatus EvsEnumerator::openUltrasonicsArray(
        [[maybe_unused]] const std::string& id,
        [[maybe_unused]] std::shared_ptr<IEvsUltrasonicsArray>* obj) {
    // TODO(b/149874793): Add implementation for EVS Manager and Sample driver
    return ScopedAStatus::ok();
}

ScopedAStatus EvsEnumerator::closeUltrasonicsArray(
        [[maybe_unused]] const std::shared_ptr<IEvsUltrasonicsArray>& obj) {
    // TODO(b/149874793): Add implementation for EVS Manager and Sample driver
    return ScopedAStatus::ok();
}

EvsEnumerator::~EvsEnumerator() {
    LOG(DEBUG) << "EvsAISEnumerator destructor";
#ifdef HAL_CAMERA_CPMS_SUPPORT
    gPowerService.reset();
    if (mPowerEventThread.joinable())
        mPowerEventThread.join();
#endif
    evsQcarcamDeInit();
    if (mpStreamConfigs)
        free(mpStreamConfigs);
    if (mC2dInitStatus)
        c2dDriverDeInit();
}

// Private functions
int EvsEnumerator::evsQcarcamInit(void)
{
    int ret = 0;
    QCarCamInit_t initParams = {};
    char *processName = (char *)"evs_hal";
    initParams.apiVersion = QCARCAM_VERSION;

    std::unique_lock<std::mutex> lock(mInitStatusLock);
    if (!mQcarcamInitialized) {
        // Initialize qcarcam
        ret = QCarCamInitialize(&initParams);
        if (ret)
            mQcarcamInitialized = false;
        else
            mQcarcamInitialized = true;
        ais_log_init(NULL, processName);
    }
    // return success if its already initialized
    lock.unlock();

    return ret;
}

void EvsEnumerator::evsQcarcamDeInit(void)
{
    std::unique_lock<std::mutex> lock(mInitStatusLock);
    if (mQcarcamInitialized) {
        QCarCamUninitialize();
        mQcarcamInitialized = false;
    }
    lock.unlock();
}

int EvsEnumerator::evsQcarcamQueryInputs(void)
{
    unsigned captureCount = 0;
    /*query qcarcam*/
    char camera_name[20];
    unsigned int queryNumInputs = 0;
    QCarCamInput_t *pAisInputList = NULL;
    int i, ret = 0;

    ret = QCarCamQueryInputs(NULL, 0, &queryNumInputs);
    if (QCARCAM_RET_OK != ret || queryNumInputs == 0)
    {
        LOG(ERROR) << "Failed QCarCamQueryInputs number of inputs="<<queryNumInputs<<" ret="<<ret;
    }
    else
    {
        pAisInputList = (QCarCamInput_t *)calloc(queryNumInputs, sizeof(*pAisInputList));
        mpStreamConfigs = (StreamConfiguration *)calloc(queryNumInputs, sizeof(*mpStreamConfigs));
        if (pAisInputList && mpStreamConfigs) {
            ret = QCarCamQueryInputs(pAisInputList, queryNumInputs, &mNumAisInputs);
            if (QCARCAM_RET_OK != ret || mNumAisInputs != queryNumInputs)
                LOG(ERROR) <<"Failed QCarCamQueryInputs with ret "<<ret;
            else {
                LOG(ERROR) <<"--- QCarCam Queried Inputs ----";
                for (i = 0; i < (int)mNumAisInputs; i++) {

                    QCarCamRet_e ret = QCARCAM_RET_OK;
                    QCarCamMode_t *pModes = (QCarCamMode_t *)calloc(pAisInputList[i].numModes, sizeof(QCarCamMode_t));
                    QCarCamInputModes_t modesDesc;
                    modesDesc.numModes = pAisInputList[i].numModes;
                    modesDesc.pModes = pModes;
                    ret = QCarCamQueryInputModes(pAisInputList[i].inputId, &modesDesc);
                    if (QCARCAM_RET_OK == ret) {
                        //TODO::Save all available configurations
                        mpStreamConfigs[i].inputId = pAisInputList[i].inputId;
                        mpStreamConfigs[i].srcId = pModes[0].sources[0].srcId;
                        mpStreamConfigs[i].width = pModes[0].sources[0].width;
                        mpStreamConfigs[i].height = pModes[0].sources[0].height;
                        mpStreamConfigs[i].colorFmt = pModes[0].sources[0].colorFmt;
                        mpStreamConfigs[i].fps = pModes[0].sources[0].fps;

                        LOG(ERROR) << i << " input_id = " << mpStreamConfigs[i].inputId
                        << " inputName = " << pAisInputList[i].inputName
                        << " flags = "<< pAisInputList[i].flags
                        << " numModes = " << pAisInputList[i].numModes
                        << "Src ID = " << mpStreamConfigs[i].srcId
                        << " width = " << mpStreamConfigs[i].width
                        << " height = " << mpStreamConfigs[i].height
                        << " colorFmt = " <<mpStreamConfigs[i].colorFmt
                        << " fps = " << mpStreamConfigs[i].fps;

                        snprintf(camera_name, sizeof(camera_name), "%d",pAisInputList[i].inputId);
                        CameraRecord cam(camera_name);
                        if (sConfigManager != nullptr) {
                            if (sConfigManager->externalMetadataEnabled()) {
                                unique_ptr<ConfigManager::CameraInfo> &camInfo =
                                    sConfigManager->getCameraInfo(camera_name);
                                if (camInfo != nullptr) {
                                    uint8_t* ptr = reinterpret_cast<uint8_t*>(camInfo->characteristics);
                                    const size_t len = get_camera_metadata_size(camInfo->characteristics);
                                    cam.desc.metadata.insert(cam.desc.metadata.end(), ptr, ptr + len);
                                }
                            } else {
                                LOG(ERROR) << "Storing qcarcam camera info as stream metadata";

                                std::unique_ptr<ConfigManager::CameraInfo> &camInfo =
                                    sConfigManager->setCameraInfo(camera_name, &mpStreamConfigs[i]);
                                
                                if (camInfo != nullptr) {
                                    uint8_t* ptr = reinterpret_cast<uint8_t*>(camInfo->characteristics);
                                    const size_t len = get_camera_metadata_size(camInfo->characteristics);
                                    cam.desc.metadata.insert(cam.desc.metadata.end(), ptr, ptr + len);
                                } else
                                    LOG(ERROR) << "Unable create camera stream metadata";
                            }
                        }
                        sCameraList.emplace(camera_name, cam);
                        captureCount++;
                    } 
                    else{
                        LOG(ERROR) << "QCarCamQueryInputModes failed with ret "<<ret;
                    }
                }
            }
        }
    }
    LOG(ERROR) << "Found "<<captureCount<<" qualified video capture devices found";
    if (pAisInputList)
        free(pAisInputList);
    return ret;
}

} // namespace aidl::android::hardware::automotive::evs::implementation

