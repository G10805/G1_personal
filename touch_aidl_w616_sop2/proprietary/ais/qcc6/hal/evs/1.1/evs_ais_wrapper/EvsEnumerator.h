/*
 * Copyright (c) 2020, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a contribution.
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

#ifndef ANDROID_HARDWARE_AUTOMOTIVE_EVS_V1_1_EVSCAMERAENUMERATOR_H
#define ANDROID_HARDWARE_AUTOMOTIVE_EVS_V1_1_EVSCAMERAENUMERATOR_H

#include <android/hardware/automotive/evs/1.1/IEvsEnumerator.h>
#include <android/hardware/automotive/evs/1.1/IEvsCamera.h>
#include <android/hardware/camera/device/3.2/ICameraDevice.h>
#include <android/frameworks/automotive/display/1.0/IAutomotiveDisplayProxyService.h>

#include <unordered_map>
#include <thread>
#include <atomic>

#include <QtiGralloc.h>
#include "ConfigManager.h"

#define DEFAULT_FRAME_WIDTH     1920
#define DEFAULT_FRAME_HEIGHT    1020
#define DEFAULT_FRAME_FORMAT    HAL_PIXEL_FORMAT_CbYCrY_422_I

#include "c2d2.h"
#define TARGET_SCISSOR 0
#define TARGET_MASK_ID 0
#define TARGET_COLOR_KEY 0
#define NO_OF_OBJECTS 1
#define KGSL_USER_MEM_TYPE_ION 3

using ::android::hardware::camera::device::V3_2::Stream;
using EvsDisplayState = ::android::hardware::automotive::evs::V1_0::DisplayState;
using IEvsCamera_1_0  = ::android::hardware::automotive::evs::V1_0::IEvsCamera;
using IEvsCamera_1_1  = ::android::hardware::automotive::evs::V1_1::IEvsCamera;
using IEvsDisplay_1_0  = ::android::hardware::automotive::evs::V1_0::IEvsDisplay;
using IEvsDisplay_1_1  = ::android::hardware::automotive::evs::V1_1::IEvsDisplay;
using android::frameworks::automotive::display::V1_0::IAutomotiveDisplayProxyService;

namespace android {
namespace hardware {
namespace automotive {
namespace evs {
namespace V1_1 {
namespace implementation {


class EvsAISCamera;    // from EvsCamera.h
class EvsGlDisplay;    // from EvsGlDisplay.h

class EvsEnumerator : public IEvsEnumerator {
public:
    // Methods from ::android::hardware::automotive::evs::V1_0::IEvsEnumerator follow.
    Return<void>                getCameraList(getCameraList_cb _hidl_cb)  override;
    Return<sp<IEvsCamera_1_0>>  openCamera(const hidl_string& cameraId) override;
    Return<void>                closeCamera(const ::android::sp<IEvsCamera_1_0>& pCamera)  override;
    Return<sp<IEvsDisplay_1_0>> openDisplay()  override;
    Return<void>                closeDisplay(const ::android::sp<IEvsDisplay_1_0>& display)  override;
    Return<EvsDisplayState>     getDisplayState()  override;

    // Methods from ::android::hardware::automotive::evs::V1_1::IEvsEnumerator follow.
    Return<void>                getCameraList_1_1(getCameraList_1_1_cb _hidl_cb) override;
    Return<sp<IEvsCamera_1_1>>  openCamera_1_1(const hidl_string& cameraId,
                                               const Stream& streamCfg) override;
    Return<bool>                isHardware() override { return true; }
    Return<void>                getDisplayIdList(getDisplayIdList_cb _list_cb) override;
    Return<sp<IEvsDisplay_1_1>> openDisplay_1_1(uint8_t port) override;
    Return<void> getUltrasonicsArrayList(getUltrasonicsArrayList_cb _hidl_cb) override;
    Return<sp<IEvsUltrasonicsArray>> openUltrasonicsArray(
            const hidl_string& ultrasonicsArrayId) override;
    Return<void> closeUltrasonicsArray(
            const ::android::sp<IEvsUltrasonicsArray>& evsUltrasonicsArray) override;

    // Implementation details
    Return<std::string>        deviceIdToQcarcamId(std::string deviceID);
    EvsEnumerator(sp<IAutomotiveDisplayProxyService> proxyService = nullptr);
    ~EvsEnumerator();

private:
    struct CameraRecord {
        CameraDesc          desc;
        wp<EvsAISCamera>    activeInstance;

        CameraRecord(const char *cameraId) : desc() { desc.v1.cameraId = cameraId; }
    };

    bool checkPermission();

    static CameraRecord* findCameraById(const std::string& cameraId);
    void enumerateCameras(int lpm);
    static void enumerateDisplays();

    void closeCamera_impl(const sp<IEvsCamera_1_0>& pCamera);
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
    static std::unordered_map<std::string,
                              CameraRecord> sCameraList;

    static wp<EvsGlDisplay>                 sActiveDisplay; // Weak pointer.
                                                            // Object destructs if client dies.

    static std::unique_ptr<ConfigManager>   sConfigManager; // ConfigManager

    static sp<IAutomotiveDisplayProxyService> sDisplayProxy;
    static std::unordered_map<uint8_t,
                              uint64_t>       sDisplayPortList;
    static uint64_t                           sInternalDisplayId;
    // Qcarcam variables
    bool mQcarcamInitialized;
    uint32_t mNumAisInputs;
    StreamConfigs_t *mpStreamConfigs;
    std::mutex mInitStatusLock;
    std::mutex mClientCntLock;
    uint32_t mClientCnt;
    bool mC2dInitStatus;

    //DeviceIDToQcarcamIDList has key Value as Device ID that stores Incrementing values 0,1,2,3,4,5,6,7 .....12,13,14,15
    //Mapped Value is Qcarcam ID's and it takes Actual Input Port ID values 0,1,2,3,4,5,8,9,12,13,14,15,16,17,18,19
    std::unordered_map<std::string, std::string> DeviceIDToQcarcamIDList;
};

} // namespace implementation
} // namespace V1_1
} // namespace evs
} // namespace automotive
} // namespace hardware
} // namespace android

#endif  // ANDROID_HARDWARE_AUTOMOTIVE_EVS_V1_1_EVSCAMERAENUMERATOR_H
