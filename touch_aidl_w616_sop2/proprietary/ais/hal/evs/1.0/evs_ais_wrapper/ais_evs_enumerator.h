/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * Not a contribution.
 *
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef EVSAISENUMERATOR_H
#define EVSAISENUMERATOR_H

#include <android/hardware/automotive/evs/1.0/IEvsEnumerator.h>
#include <android/hardware/automotive/evs/1.0/IEvsCamera.h>

#include <list>
#include "ais_evs_camera.h"
#include "c2d2.h"
#define DEFAULT_FRAME_WIDTH 1280
#define DEFAULT_FRAME_HEIGHT 720
#define DEFAULT_FRAME_FORMAT HAL_PIXEL_FORMAT_CbYCrY_422_I

namespace android {
namespace hardware {
namespace automotive {
namespace evs {
namespace V1_0 {
namespace implementation {


class EvsAISCamera;    // from ais_evs_camera.h
class EvsGlDisplay;    // from ais_evs_gldisplay.h


class EvsAISEnumerator : public IEvsEnumerator {
public:
    // Methods from ::android::hardware::automotive::evs::V1_0::IEvsEnumerator follow.
    Return<void> getCameraList(getCameraList_cb _hidl_cb)  override;
    Return<sp<IEvsCamera>> openCamera(const hidl_string& cameraId) override;
    Return<void> closeCamera(const ::android::sp<IEvsCamera>& carCamera)  override;
    Return<sp<IEvsDisplay>> openDisplay()  override;
    Return<void> closeDisplay(const ::android::sp<IEvsDisplay>& display)  override;
    Return<DisplayState> getDisplayState()  override;

    // Implementation details
    EvsAISEnumerator();
    ~EvsAISEnumerator();

private:
    struct CameraRecord {
        CameraDesc          desc;
        wp<EvsAISCamera>    activeInstance;

        CameraRecord(const char *cameraId) : desc() { desc.cameraId = cameraId; }
    };

    uint32_t getQcarcamToHalFormat(qcarcam_color_fmt_t qcarcamFormat);

    static CameraRecord* findCameraById(const std::string& cameraId);

    void enumerateCameras(int lpm);
    void closeCamera_impl(const sp<IEvsCamera>& pCamera);
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
    static std::list<CameraRecord> sCameraList;

    static wp<EvsGlDisplay>          sActiveDisplay; // Weak pointer. Object destructs if client dies.
    bool mQcarcamInitialized;
    uint32_t mNumAisInputs;
    qcarcam_input_t *m_pAisInputList;
    std::mutex mInitStatusLock;
    std::mutex mClientCntLock;
    uint32_t mClientCnt;
};

} // namespace implementation
} // namespace V1_0
} // namespace evs
} // namespace automotive
} // namespace hardware
} // namespace android

#endif  // EVSAISENUMERATOR_H
