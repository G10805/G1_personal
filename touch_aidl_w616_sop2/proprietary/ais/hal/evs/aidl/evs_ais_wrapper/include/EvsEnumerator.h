/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a contribution.

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

#ifndef CPP_EVS_SAMPLEDRIVER_AIDL_INCLUDE_EVSENUMERATOR_H
#define CPP_EVS_SAMPLEDRIVER_AIDL_INCLUDE_EVSENUMERATOR_H

#include "ConfigManager.h"
#include "EvsGlDisplay.h"
#include "EvsAISCamera.h"
#include "c2d2.h"
#ifdef HAL_CAMERA_CPMS_SUPPORT
#include "PowerEvent.h"
#include "PowerPolicyService.h"
#endif

#include <android/frameworks/automotive/display/1.0/IAutomotiveDisplayProxyService.h>
#include <aidl/android/hardware/automotive/evs/BnEvsEnumerator.h>
#include <aidl/android/hardware/automotive/evs/CameraDesc.h>
#include <aidl/android/hardware/automotive/evs/DeviceStatusType.h>
#include <aidl/android/hardware/automotive/evs/IEvsCamera.h>
#include <aidl/android/hardware/automotive/evs/IEvsEnumeratorStatusCallback.h>
#include <aidl/android/hardware/automotive/evs/Stream.h>
#include <utils/Thread.h>

#include <atomic>
#include <thread>
#include <unordered_map>

#define TARGET_SCISSOR 0
#define TARGET_MASK_ID 0
#define TARGET_COLOR_KEY 0
#define NO_OF_OBJECTS 1
#define KGSL_USER_MEM_TYPE_ION 3
using android::frameworks::automotive::display::V1_0::IAutomotiveDisplayProxyService;

namespace aidl::android::hardware::automotive::evs::implementation {

namespace aidlevs = ::aidl::android::hardware::automotive::evs;

class EvsEnumerator final : public ::aidl::android::hardware::automotive::evs::BnEvsEnumerator {

public:
    // Methods from ::aidl::android::hardware::automotive::evs::IEvsEnumerator
    ::ndk::ScopedAStatus isHardware(bool* flag) override;
    ::ndk::ScopedAStatus openCamera(const std::string& cameraId,
                                    const aidlevs::Stream& streamConfig,
                                    std::shared_ptr<aidlevs::IEvsCamera>* obj) override;
    ::ndk::ScopedAStatus closeCamera(const std::shared_ptr<aidlevs::IEvsCamera>& obj) override;
    ::ndk::ScopedAStatus getCameraList(std::vector<aidlevs::CameraDesc>* _aidl_return) override;
    ::ndk::ScopedAStatus getStreamList(const aidlevs::CameraDesc& desc,
                                       std::vector<aidlevs::Stream>* _aidl_return) override;
    ::ndk::ScopedAStatus openDisplay(int32_t displayId,
                                     std::shared_ptr<aidlevs::IEvsDisplay>* obj) override;
    ::ndk::ScopedAStatus closeDisplay(const std::shared_ptr<aidlevs::IEvsDisplay>& obj) override;
    ::ndk::ScopedAStatus getDisplayIdList(std::vector<uint8_t>* list) override;
    ::ndk::ScopedAStatus getDisplayState(aidlevs::DisplayState* state) override;
    ::ndk::ScopedAStatus registerStatusCallback(
            const std::shared_ptr<aidlevs::IEvsEnumeratorStatusCallback>& callback) override;
    ::ndk::ScopedAStatus openUltrasonicsArray(
            const std::string& id, std::shared_ptr<aidlevs::IEvsUltrasonicsArray>* obj) override;
    ::ndk::ScopedAStatus closeUltrasonicsArray(
            const std::shared_ptr<aidlevs::IEvsUltrasonicsArray>& obj) override;
    ::ndk::ScopedAStatus getUltrasonicsArrayList(
            std::vector<aidlevs::UltrasonicsArrayDesc>* list) override;

    // Methods from ndk::ICInterface
//    binder_status_t dump(int fd, const char** args, uint32_t numArgs) override;

    // Implementation details
    EvsEnumerator(sp<IAutomotiveDisplayProxyService> proxyService = nullptr);
    ~EvsEnumerator();

#ifdef HAL_CAMERA_CPMS_SUPPORT
    static int PowerEventCb(pm_event_t eventId, void* pUsrCtxt);
#endif

    void notifyDeviceStatusChange(const std::string_view& deviceName,
                                  aidlevs::DeviceStatusType type);

    // Monitor hotplug devices
    static void EvsHotplugThread(std::shared_ptr<EvsEnumerator> service,
                                 std::atomic<bool>& running);

private:
    struct CameraRecord {
        aidlevs::CameraDesc desc;
        std::weak_ptr<EvsAISCamera> activeInstance;

        CameraRecord(const char* cameraId) : desc() { desc.id = cameraId; }
    };

    bool checkPermission();
    void closeCamera_impl(const std::shared_ptr<aidlevs::IEvsCamera>& pCamera);

    static bool qualifyCaptureDevice(const char* deviceName);
    static CameraRecord* findCameraById(const std::string& cameraId);
    void enumerateCameras(int lpm);
    static bool addCaptureDevice(const std::string& deviceName);
    static bool removeCaptureDevice(const std::string& deviceName);
    // Enumerate available displays and return an id of the internal display
    static uint64_t enumerateDisplays();

    // Qcarcam related functions
    int evsQcarcamInit(void);
    void evsQcarcamDeInit(void);
    int evsQcarcamQueryInputs(void);
   
    // NOTE:  All members values are static so that all clients operate on the same state
    //        That is to say, this is effectively a singleton despite the fact that HIDL
    //        constructs a new instance for each client.
    //        Because our server has a single thread in the thread pool, these values are
    //        never accessed concurrently despite potentially having multiple instance objects
    //        using them.
    static std::unordered_map<std::string, CameraRecord> sCameraList;
    static std::weak_ptr<EvsGlDisplay> sActiveDisplay;     // Weak pointer.
                                                           // Object destructs if client dies.
    static std::mutex sLock;                               // Mutex on shared camera device list.
    static std::condition_variable sCameraSignal;          // Signal on camera device addition.
    static std::unique_ptr<ConfigManager> sConfigManager;  // ConfigManager
    static sp<IAutomotiveDisplayProxyService> sDisplayProxy;
    static std::unordered_map<uint8_t, uint64_t> sDisplayPortList;

    static uint64_t sInternalDisplayId;
    std::shared_ptr<aidlevs::IEvsEnumeratorStatusCallback> mCallback;

    // Qcarcam variables
    bool mQcarcamInitialized;
    uint32_t mNumAisInputs;
    qcarcam_input_t *m_pAisInputList;
    std::mutex mInitStatusLock;
    std::mutex mClientCntLock;
    uint32_t mClientCnt;
    bool mC2dInitStatus;
#ifdef HAL_CAMERA_CPMS_SUPPORT
    std::thread mPowerEventThread;    // The Thread for the CPMS power Events
#endif
};

}  // namespace aidl::android::hardware::automotive::evs::implementation

#endif  // CPP_EVS_SAMPLEDRIVER_AIDL_INCLUDE_EVSENUMERATOR_H
