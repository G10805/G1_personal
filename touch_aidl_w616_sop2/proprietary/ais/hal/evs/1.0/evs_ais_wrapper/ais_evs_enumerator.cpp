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

#include "ais_evs_enumerator.h"
#include "ais_evs_gldisplay.h"
#include <gralloc_priv.h>

#include <dirent.h>
#include <stdio.h>


namespace android {
namespace hardware {
namespace automotive {
namespace evs {
namespace V1_0 {
namespace implementation {


// NOTE:  All members values are static so that all clients operate on the same state
//        That is to say, this is effectively a singleton despite the fact that HIDL
//        constructs a new instance for each client.
std::list<EvsAISEnumerator::CameraRecord>   EvsAISEnumerator::sCameraList;
wp<EvsGlDisplay>                           EvsAISEnumerator::sActiveDisplay;


EvsAISEnumerator::EvsAISEnumerator() {
    ALOGD("EvsAISEnumerator created");
    mQcarcamInitialized = false;
    enumerateCameras(1); //lpm==1
    mClientCnt = 0;
#ifndef CPU_CONVERSION
    C2D_STATUS c2d_status = c2dDriverInit(NULL);

    if(c2d_status != C2D_STATUS_OK) {
        ALOGE("\nC2DDriverInit failed \n");
    }
    ALOGE("C2D driver loaded Successfully\n");
#endif

}


// Methods from ::android::hardware::automotive::evs::V1_0::IEvsEnumerator follow.
Return<void> EvsAISEnumerator::getCameraList(getCameraList_cb _hidl_cb)  {
    ALOGD("getCameraList");

    hidl_vec<CameraDesc> hidlCameras;
    {
        if (sCameraList.size() < 1) {
            // Retry enumarating cameras
            enumerateCameras(1); //lpm==1
        }
    }
    const unsigned numCameras = sCameraList.size();

    // Build up a packed array of CameraDesc for return
    hidlCameras.resize(numCameras);
    unsigned i = 0;
    for (const auto& cam : sCameraList) {
        hidlCameras[i++] = cam.desc;
    }

    // Send back the results
    ALOGD("reporting %zu cameras available", hidlCameras.size());
    _hidl_cb(hidlCameras);

    // HIDL convention says we return Void if we sent our result back via callback
    return Void();
}


Return<sp<IEvsCamera>> EvsAISEnumerator::openCamera(const hidl_string& cameraId) {
    ALOGD("openCamera");

    uint32_t frameWidth = DEFAULT_FRAME_WIDTH;
    uint32_t frameHeight = DEFAULT_FRAME_HEIGHT;
    uint32_t frameFormat = DEFAULT_FRAME_FORMAT;
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
                ALOGD("qcarcam_initialize successfully");
            else {
                ALOGE("qcarcam_initialize failed ret = %d",ret);
                lock.unlock();
                return nullptr;
            }
        }
    }

    // Is this a recognized camera id?
    CameraRecord *pRecord = findCameraById(cameraId);
    if (!pRecord) {
        ALOGE("Requested camera %s not found", cameraId.c_str());
        // Uninitialize qcarcam only if its first client
        if (0 == mClientCnt)
            evsQcarcamDeInit();
        lock.unlock();
        return nullptr;
    }

    // Has this camera already been instantiated by another caller?
    sp<EvsAISCamera> pActiveCamera = pRecord->activeInstance.promote();
    if (pActiveCamera != nullptr) {
        ALOGW("Killing previous camera because of new caller");
        mClientCnt = (mClientCnt > 0)?(mClientCnt-1):0;
        closeCamera_impl(pActiveCamera);
    }

    if (!m_pAisInputList) {
        ALOGE("m_pAisInputList is not initialized");
        if (0 == mClientCnt)
            evsQcarcamDeInit();
        lock.unlock();
        return nullptr;
    }

    int stream_id;
    sscanf(cameraId.c_str(), "%d", &stream_id);
    for (uint32_t i = 0; i < mNumAisInputs; i++) {
        if (stream_id == m_pAisInputList[i].desc) {
            frameWidth = m_pAisInputList[i].res[0].width;
            frameHeight = m_pAisInputList[i].res[0].height;
            frameFormat = getQcarcamToHalFormat(m_pAisInputList[i].color_fmt[0]);
        }
    }

    // Construct a camera instance for the caller
    pActiveCamera = new EvsAISCamera(cameraId.c_str(), frameWidth, frameHeight, frameFormat);
    pRecord->activeInstance = pActiveCamera;
    if (pActiveCamera == nullptr) {
        ALOGE("Failed to allocate new EvsAISCamera object for %s\n", cameraId.c_str());
        // Uninitialize qcarcam only if its first client
        if (0 == mClientCnt)
            evsQcarcamDeInit();
        lock.unlock();
        return nullptr;
    }

    mClientCnt++;
    lock.unlock();
    return pActiveCamera;
}


Return<void> EvsAISEnumerator::closeCamera(const ::android::sp<IEvsCamera>& pCamera) {
    ALOGD("closeCamera");

    if (pCamera == nullptr) {
        ALOGE("Ignoring call to closeCamera with null camera ptr");
        return Void();
    }

    closeCamera_impl(pCamera);

    std::unique_lock<std::mutex> lock(mClientCntLock);
    mClientCnt = (mClientCnt > 0)?(mClientCnt-1):0;
    // Uninitialize qcarcam only if its last client
    if (0 == mClientCnt) {
        evsQcarcamDeInit();
    }
    lock.unlock();

    return Void();
}


Return<sp<IEvsDisplay>> EvsAISEnumerator::openDisplay() {
    ALOGD("openDisplay");

    // If we already have a display active, then we need to shut it down so we can
    // give exclusive access to the new caller.
    sp<EvsGlDisplay> pActiveDisplay = sActiveDisplay.promote();
    if (pActiveDisplay != nullptr) {
        ALOGW("Killing previous display because of new caller");
        closeDisplay(pActiveDisplay);
    }

    // Create a new display interface and return it
    pActiveDisplay = new EvsGlDisplay();
    sActiveDisplay = pActiveDisplay;

    ALOGD("Returning new EvsGlDisplay object %p", pActiveDisplay.get());
    return pActiveDisplay;
}


Return<void> EvsAISEnumerator::closeDisplay(const ::android::sp<IEvsDisplay>& pDisplay) {
    ALOGD("closeDisplay");

    // Do we still have a display object we think should be active?
    sp<EvsGlDisplay> pActiveDisplay = sActiveDisplay.promote();
    if (pActiveDisplay == nullptr) {
        ALOGE("Somehow a display is being destroyed when the enumerator didn't know one existed");
    } else if (sActiveDisplay != pDisplay) {
        ALOGW("Ignoring close of previously orphaned display - why did a client steal?");
    } else {
        // Drop the active display
        pActiveDisplay->forceShutdown();
        sActiveDisplay = nullptr;
    }

    return Void();
}


Return<DisplayState> EvsAISEnumerator::getDisplayState()  {
    ALOGD("getDisplayState");

    // Do we still have a display object we think should be active?
    sp<IEvsDisplay> pActiveDisplay = sActiveDisplay.promote();
    if (pActiveDisplay != nullptr) {
        return pActiveDisplay->getDisplayState();
    } else {
        return DisplayState::NOT_OPEN;
    }
}

EvsAISEnumerator::CameraRecord* EvsAISEnumerator::findCameraById(const std::string& cameraId) {
    // Find the named camera
    for (auto &&cam : sCameraList) {
        if (cam.desc.cameraId == cameraId) {
            // Found a match!
            return &cam;
        }
        ALOGE("EVS::cam.desc.cameraId.c_str() = %s\n",cam.desc.cameraId.c_str());
    }

    // We didn't find a match
    return nullptr;
}

EvsAISEnumerator::~EvsAISEnumerator() {
    ALOGD("EvsAISEnumerator destructor");
    evsQcarcamDeInit();
    if (m_pAisInputList)
        free(m_pAisInputList);
#ifndef CPU_CONVERSION
    c2dDriverDeInit();
#endif
}

uint32_t EvsAISEnumerator::getQcarcamToHalFormat(qcarcam_color_fmt_t qcarcamFormat)
{
    switch(qcarcamFormat)
    {
        case QCARCAM_FMT_UYVY_8:
        case QCARCAM_FMT_UYVY_10:
        case QCARCAM_FMT_UYVY_12:
            return HAL_PIXEL_FORMAT_CbYCrY_422_I;
        case QCARCAM_FMT_YUYV_8:
        case QCARCAM_FMT_YUYV_10:
        case QCARCAM_FMT_YUYV_12:
            return HAL_PIXEL_FORMAT_YCBCR_422_I;
        case QCARCAM_FMT_RGB_888:
            return HAL_PIXEL_FORMAT_RGB_888;
        case QCARCAM_FMT_MIPIRAW_8:
            return HAL_PIXEL_FORMAT_RAW8;
        case QCARCAM_FMT_MIPIRAW_10:
            return HAL_PIXEL_FORMAT_RAW10;
        case QCARCAM_FMT_MIPIRAW_12:
            return HAL_PIXEL_FORMAT_RAW12;
        default: ALOGE("Unsupported HAL format");
            return -1;
    }
}

int EvsAISEnumerator::evsQcarcamInit(void)
{
    int ret = 0;
    qcarcam_init_t qcarcam_init={};
    qcarcam_init.version = QCARCAM_VERSION;

    // Initialize qcarcam
    std::unique_lock<std::mutex> lock(mInitStatusLock);
    if (!mQcarcamInitialized) {
        // Initialize qcarcam
        ret = qcarcam_initialize(&qcarcam_init);
        if (ret)
            mQcarcamInitialized = false;
        else
            mQcarcamInitialized = true;
    }
    // return success if its already initialized
    lock.unlock();

    return ret;
}

void EvsAISEnumerator::evsQcarcamDeInit(void)
{
    std::unique_lock<std::mutex> lock(mInitStatusLock);
    if (mQcarcamInitialized) {
        qcarcam_uninitialize();
        mQcarcamInitialized = false;
    }
    lock.unlock();
}

void EvsAISEnumerator::enumerateCameras(int lpm) {

    int ret = 0;
    ret = evsQcarcamInit();
    if (ret == QCARCAM_RET_OK)
    {
        ALOGD("qcarcam_initialize successfully");
        ret = evsQcarcamQueryInputs();
        if (ret != QCARCAM_RET_OK)
            ALOGE("evsQcarcamQueryInputs failed ret = %d",ret);
    }
    else
        ALOGE("qcarcam_initialize failed");
    // Deinit for LPM
    if (lpm)
        evsQcarcamDeInit();
}

int EvsAISEnumerator::evsQcarcamQueryInputs(void)
{
    unsigned captureCount = 0;
    /*query qcarcam*/
    char camera_name[20];
    unsigned int queryNumInputs = 0;
    int i, ret = 0;

    ret = qcarcam_query_inputs(NULL, 0, &queryNumInputs);
    if (QCARCAM_RET_OK != ret || queryNumInputs == 0)
    {
        ALOGE("Failed qcarcam_query_inputs number of inputs with ret %d", ret);
        m_pAisInputList = NULL;
    }
    else
    {
        m_pAisInputList = (qcarcam_input_t *)calloc(queryNumInputs, sizeof(*m_pAisInputList));
        if (m_pAisInputList)
        {
            ret = qcarcam_query_inputs(m_pAisInputList, queryNumInputs, &mNumAisInputs);
            if (QCARCAM_RET_OK != ret || mNumAisInputs != queryNumInputs)
            {
                ALOGE("Failed qcarcam_query_inputs with ret %d", ret);
            }
            else
            {
                ALOGE("--- QCarCam Queried Inputs ----");
                for (i = 0; i < (int)mNumAisInputs; i++)
                {
                    ALOGE("%d: input_id=%d, res=%dx%d fmt=0x%08x", i, m_pAisInputList[i].desc,
                            m_pAisInputList[i].res[0].width, m_pAisInputList[i].res[0].height,
                            m_pAisInputList[i].color_fmt[0]);
                    snprintf(camera_name, sizeof(camera_name), "%d",m_pAisInputList[i].desc);
                    sCameraList.emplace_back(camera_name);
                    captureCount++;
                }
            }
        }
    }
    ALOGE("Found %d qualified video capture devices found\n", captureCount);
    return ret;
}

void EvsAISEnumerator::closeCamera_impl(const ::android::sp<IEvsCamera>& pCamera) {
    ALOGD("closeCamera_impl");

    if (pCamera == nullptr) {
        ALOGE("Ignoring call to closeCamera with null camera ptr");
        return;
    }
        // Get the camera id so we can find it in our list
    std::string cameraId;
    pCamera->getCameraInfo([&cameraId](CameraDesc desc) {
                               cameraId = desc.cameraId;
                           }
    );

    // Find the named camera
    CameraRecord *pRecord = findCameraById(cameraId);

    // Is the display being destroyed actually the one we think is active?
    if (!pRecord) {
        ALOGE("Asked to close a camera whose name isn't recognized");
    } else {
        sp<EvsAISCamera> pActiveCamera = pRecord->activeInstance.promote();

        if (pActiveCamera == nullptr) {
            ALOGE("Somehow a camera is being destroyed when the enumerator didn't know one existed");
        } else if (pActiveCamera != pCamera) {
            // This can happen if the camera was aggressively reopened, orphaning this previous instance
            ALOGW("Ignoring close of previously orphaned camera - why did a client steal?");
        } else {
            // Drop the active camera
            pActiveCamera->shutdown();
            pRecord->activeInstance = nullptr;
        }
    }
}

} // namespace implementation
} // namespace V1_0
} // namespace evs
} // namespace automotive
} // namespace hardware
} // namespace android
