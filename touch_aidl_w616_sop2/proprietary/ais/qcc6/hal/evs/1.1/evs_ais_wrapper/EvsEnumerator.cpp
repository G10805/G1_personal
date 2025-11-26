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

#include "EvsEnumerator.h"
#include "EvsAISCamera.h"
#include "EvsGlDisplay.h"
#include "ConfigManager.h"
#include "ais_log.h"

#include <dirent.h>
#include <hardware_legacy/uevent.h>
#include <hwbinder/IPCThreadState.h>
#include <cutils/android_filesystem_config.h>

using namespace std::chrono_literals;
using CameraDesc_1_0 = ::android::hardware::automotive::evs::V1_0::CameraDesc;
using CameraDesc_1_1 = ::android::hardware::automotive::evs::V1_1::CameraDesc;

namespace android {
namespace hardware {
namespace automotive {
namespace evs {
namespace V1_1 {
namespace implementation {


// NOTE:  All members values are static so that all clients operate on the same state
//        That is to say, this is effectively a singleton despite the fact that HIDL
//        constructs a new instance for each client.
std::unordered_map<std::string, EvsEnumerator::CameraRecord> EvsEnumerator::sCameraList;
wp<EvsGlDisplay>                                             EvsEnumerator::sActiveDisplay;
std::unique_ptr<ConfigManager>                               EvsEnumerator::sConfigManager;
sp<IAutomotiveDisplayProxyService>                           EvsEnumerator::sDisplayProxy;
std::unordered_map<uint8_t, uint64_t>                        EvsEnumerator::sDisplayPortList;
uint64_t                                                     EvsEnumerator::sInternalDisplayId;



bool EvsEnumerator::checkPermission() {
    hardware::IPCThreadState *ipc = hardware::IPCThreadState::self();
    if (AID_AUTOMOTIVE_EVS != ipc->getCallingUid() &&
        AID_ROOT != ipc->getCallingUid()) {
        LOG(ERROR) << "EVS access denied: "
                   << "pid = " << ipc->getCallingPid()
                   << ", uid = " << ipc->getCallingUid();
        return false;
    }

    return true;
}


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


void EvsEnumerator::enumerateDisplays() {
    LOG(INFO) << __FUNCTION__
              << ": Starting display enumeration";
    if (!sDisplayProxy) {
        LOG(ERROR) << "AutomotiveDisplayProxyService is not available!";
        return;
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
}


// Methods from ::android::hardware::automotive::evs::V1_0::IEvsEnumerator follow.
Return<void> EvsEnumerator::getCameraList(getCameraList_cb _hidl_cb)  {
    LOG(DEBUG) << __FUNCTION__;
    hidl_vec<CameraDesc_1_0> hidlCameras;
    if (!checkPermission()) {
        _hidl_cb(hidlCameras);
        return Void();
    }

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
    for (const auto& [key, cam] : sCameraList) {
        hidlCameras[i++] = cam.desc.v1;
    }

    // Send back the results
    LOG(DEBUG) << "Reporting " << hidlCameras.size() << " cameras available";
    _hidl_cb(hidlCameras);

    // HIDL convention says we return Void if we sent our result back via callback
    return Void();
}


Return<sp<IEvsCamera_1_0>> EvsEnumerator::openCamera(const hidl_string& cameraId) {
    LOG(DEBUG) << __FUNCTION__ <<" "<< cameraId;
    if (!checkPermission()) {
        return nullptr;
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
                return nullptr;
            }
        }
    }

    // Is this a recognized camera id?
    CameraRecord *pRecord = findCameraById(cameraId);
    if (pRecord == nullptr) {
        LOG(ERROR) << cameraId << " does not exist!";
        // Uninitialize qcarcam only if its first client
        if (0 == mClientCnt)
            evsQcarcamDeInit();
        lock.unlock();
        return nullptr;
    }

    // Has this camera already been instantiated by another caller?
    sp<EvsAISCamera> pActiveCamera = pRecord->activeInstance.promote();
    if (pActiveCamera != nullptr) {
        LOG(WARNING) << "Killing previous camera because of new caller";
        mClientCnt = (mClientCnt > 0)?(mClientCnt-1):0;
        closeCamera_impl(pActiveCamera);
    }
    std::string qcarcam_idx = deviceIdToQcarcamId(cameraId);
    // Construct a camera instance for the caller
    if (sConfigManager == nullptr) {
        LOG(ERROR) << "ConfigManager is not available.  Given stream configuration is ignored.";
        pActiveCamera = EvsAISCamera::Create(cameraId.c_str(),
                                             (void*)mpStreamConfigs,
                                             mNumAisInputs,
                                             mC2dInitStatus,
                                             this);
    } else {
        pActiveCamera = EvsAISCamera::Create(cameraId.c_str(),
                                             (void*)mpStreamConfigs,
                                             mNumAisInputs,
                                             mC2dInitStatus,
                                             sConfigManager->getCameraInfo(qcarcam_idx),
                                             this);
    }

    pRecord->activeInstance = pActiveCamera;
    if (pActiveCamera == nullptr) {
        LOG(ERROR) << "Failed to create new EvsAISCamera object for " << cameraId;
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


Return<void> EvsEnumerator::closeCamera(const ::android::sp<IEvsCamera_1_0>& pCamera) {
    LOG(DEBUG) << __FUNCTION__;

    if (pCamera == nullptr) {
        LOG(ERROR) << "Ignoring call to closeCamera with null camera ptr";
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


Return<sp<IEvsDisplay_1_0>> EvsEnumerator::openDisplay() {
    LOG(DEBUG) << __FUNCTION__;
    if (!checkPermission()) {
        return nullptr;
    }

    // If we already have a display active, then we need to shut it down so we can
    // give exclusive access to the new caller.
    sp<EvsGlDisplay> pActiveDisplay = sActiveDisplay.promote();
    if (pActiveDisplay != nullptr) {
        LOG(WARNING) << "Killing previous display because of new caller";
        closeDisplay(pActiveDisplay);
    }

    // Create a new display interface and return it.
    pActiveDisplay = new EvsGlDisplay(sDisplayProxy, sInternalDisplayId);
    sActiveDisplay = pActiveDisplay;

    LOG(DEBUG) << "Returning new EvsGlDisplay object " << pActiveDisplay.get();
    return pActiveDisplay;
}


Return<void> EvsEnumerator::closeDisplay(const ::android::sp<IEvsDisplay_1_0>& pDisplay) {
    LOG(DEBUG) << __FUNCTION__;

    // Do we still have a display object we think should be active?
    sp<EvsGlDisplay> pActiveDisplay = sActiveDisplay.promote();
    if (pActiveDisplay == nullptr) {
        LOG(ERROR) << "Somehow a display is being destroyed "
                   << "when the enumerator didn't know one existed";
    } else if (sActiveDisplay != pDisplay) {
        LOG(WARNING) << "Ignoring close of previously orphaned display - why did a client steal?";
    } else {
        // Drop the active display
        pActiveDisplay->forceShutdown();
        sActiveDisplay = nullptr;
    }

    return Void();
}


Return<EvsDisplayState> EvsEnumerator::getDisplayState()  {
    LOG(DEBUG) << __FUNCTION__;
    if (!checkPermission()) {
        return EvsDisplayState::DEAD;
    }

    // Do we still have a display object we think should be active?
    sp<IEvsDisplay_1_0> pActiveDisplay = sActiveDisplay.promote();
    if (pActiveDisplay != nullptr) {
        return pActiveDisplay->getDisplayState();
    } else {
        return EvsDisplayState::NOT_OPEN;
    }
}


// Methods from ::android::hardware::automotive::evs::V1_1::IEvsEnumerator follow.
Return<void> EvsEnumerator::getCameraList_1_1(getCameraList_1_1_cb _hidl_cb)  {
    LOG(DEBUG) << __FUNCTION__;
    std::vector<CameraDesc_1_1> hidlCameras;
    if (!checkPermission()) {
        _hidl_cb(hidlCameras);
        return Void();
    }

    {
        if (sCameraList.size() < 1) {
            // Retry enumarating cameras
            enumerateCameras(1); //lpm==1
        }
    }

    if (sConfigManager == nullptr) {
        auto numCameras = sCameraList.size();

        // Build up a packed array of CameraDesc for return
        hidlCameras.resize(numCameras);
        unsigned i = 0;
        for (auto&& [key, cam] : sCameraList) {
            hidlCameras[i++] = cam.desc;
        }
    } else {
        // Build up a packed array of CameraDesc for return
        for (auto&& [key, cam] : sCameraList) {
            unique_ptr<ConfigManager::CameraInfo> &tempInfo =
                sConfigManager->getCameraInfo(key);
            if (tempInfo != nullptr) {
                cam.desc.metadata.setToExternal(
                    (uint8_t *)tempInfo->characteristics,
                     get_camera_metadata_size(tempInfo->characteristics)
                );
            }

            hidlCameras.emplace_back(cam.desc);
        }

        // Adding camera groups that represent logical camera devices
        auto camGroups = sConfigManager->getCameraGroupIdList();
        for (auto&& id : camGroups) {
            if (sCameraList.find(id) != sCameraList.end()) {
                // Already exists in the list
                continue;
            }

            unique_ptr<ConfigManager::CameraGroupInfo> &tempInfo =
                sConfigManager->getCameraGroupInfo(id);
            CameraRecord cam(id.c_str());
            if (tempInfo != nullptr) {
                cam.desc.metadata.setToExternal(
                    (uint8_t *)tempInfo->characteristics,
                     get_camera_metadata_size(tempInfo->characteristics)
                );
            }

            sCameraList.emplace(id, cam);
            hidlCameras.emplace_back(cam.desc);
        }
    }

    // Send back the results
    _hidl_cb(hidlCameras);

    // HIDL convention says we return Void if we sent our result back via callback
    return Void();
}

Return<std::string> EvsEnumerator::deviceIdToQcarcamId(std::string deviceID)
{
    std::string result = "";
    auto iter = DeviceIDToQcarcamIDList.find(deviceID);
    if (iter == DeviceIDToQcarcamIDList.end()) {
      LOG(ERROR)<<"ERROR: device id does not exist: deviceID" <<deviceID;
      return result;
    }
    std::string qcarcam_idx = iter->second;
    return qcarcam_idx;
}

Return<sp<IEvsCamera_1_1>> EvsEnumerator::openCamera_1_1(const hidl_string& cameraId,
                                                         const Stream& streamCfg) {
    LOG(DEBUG) << __FUNCTION__ <<" "<< cameraId;
    if (!checkPermission()) {
        return nullptr;
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
                LOG(ERROR) << "evsQcarcamInit failed ret = " << ret;
                lock.unlock();
                return nullptr;
            }
        }
    }

    // Is this a recognized camera id?
    CameraRecord *pRecord = findCameraById(cameraId);
    if (pRecord == nullptr) {
        LOG(ERROR) << cameraId << " does not exist!";
        // Uninitialize qcarcam only if its first client
        if (0 == mClientCnt)
            evsQcarcamDeInit();
        lock.unlock();
        return nullptr;
    }

    // Has this camera already been instantiated by another caller?
    sp<EvsAISCamera> pActiveCamera = pRecord->activeInstance.promote();
    if (pActiveCamera != nullptr) {
        LOG(WARNING) << "Killing previous camera because of new caller";
        mClientCnt = (mClientCnt > 0)?(mClientCnt-1):0;
        closeCamera_impl(pActiveCamera);
    }
    std::string qcarcam_idx = deviceIdToQcarcamId(cameraId);
    // Construct a camera instance for the caller
    if (sConfigManager == nullptr) {
        LOG(WARNING) << "ConfigManager is not available.  "
                     << "Given stream configuration is ignored.";
        pActiveCamera = EvsAISCamera::Create(cameraId.c_str(),
                                             (void*)mpStreamConfigs,
                                             mNumAisInputs,
                                             mC2dInitStatus,
                                             this);
    } else {
        pActiveCamera = EvsAISCamera::Create(cameraId.c_str(),
                                             (void*)mpStreamConfigs,
                                             mNumAisInputs,
                                             mC2dInitStatus,
                                             sConfigManager->getCameraInfo(qcarcam_idx),
                                             this,
                                             &streamCfg);
    }
    pRecord->activeInstance = pActiveCamera;
    if (pActiveCamera == nullptr) {
        LOG(ERROR) << "Failed to create new EvsV4lCamera object for " << cameraId;
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


Return<void> EvsEnumerator::getDisplayIdList(getDisplayIdList_cb _list_cb) {
    hidl_vec<uint8_t> ids;

    if (sDisplayPortList.size() > 0) {
        ids.resize(sDisplayPortList.size());
        unsigned i = 0;
        ids[i++] = sInternalDisplayId & 0xF;
        for (const auto& [port, id] : sDisplayPortList) {
            if (sInternalDisplayId != id) {
                ids[i++] = port;
            }
        }
    }

    _list_cb(ids);
    return Void();
}


Return<sp<IEvsDisplay_1_1>> EvsEnumerator::openDisplay_1_1(uint8_t port) {
    LOG(DEBUG) << __FUNCTION__;
    if (!checkPermission()) {
        return nullptr;
    }

    // If we already have a display active, then we need to shut it down so we can
    // give exclusive access to the new caller.
    sp<EvsGlDisplay> pActiveDisplay = sActiveDisplay.promote();
    if (pActiveDisplay != nullptr) {
        LOG(WARNING) << "Killing previous display because of new caller";
        closeDisplay(pActiveDisplay);
    }

    // Create a new display interface and return it
    pActiveDisplay = new EvsGlDisplay(sDisplayProxy, sDisplayPortList[port]);
    sActiveDisplay = pActiveDisplay;

    LOG(DEBUG) << "Returning new EvsGlDisplay object " << pActiveDisplay.get();
    return pActiveDisplay;
}


void EvsEnumerator::closeCamera_impl(const sp<IEvsCamera_1_0>& pCamera) {
    // Get the camera id so we can find it in our list
    std::string cameraId;
    pCamera->getCameraInfo([&cameraId](CameraDesc_1_0 desc) {
                               cameraId = desc.cameraId;
                           }
    );
    LOG(DEBUG) << __FUNCTION__ <<" "<< cameraId;
    // Find the named camera
    CameraRecord *pRecord = findCameraById(cameraId);

    // Is the display being destroyed actually the one we think is active?
    if (!pRecord) {
        LOG(ERROR) << "Asked to close a camera whose name isn't recognized";
    } else {
        sp<EvsAISCamera> pActiveCamera = pRecord->activeInstance.promote();

        if (pActiveCamera == nullptr) {
            LOG(ERROR) << "Somehow a camera is being destroyed "
                       << "when the enumerator didn't know one existed";
        } else if (pActiveCamera != pCamera) {
            // This can happen if the camera was aggressively reopened,
            // orphaning this previous instance
            LOG(WARNING) << "Ignoring close of previously orphaned camera "
                         << "- why did a client steal?";
        } else {
            // Drop the active camera
            pActiveCamera->shutdown();
            pRecord->activeInstance = nullptr;
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


// TODO(b/149874793): Add implementation for EVS Manager and Sample driver
Return<void> EvsEnumerator::getUltrasonicsArrayList(getUltrasonicsArrayList_cb _hidl_cb) {
    hidl_vec<UltrasonicsArrayDesc> ultrasonicsArrayDesc;
    _hidl_cb(ultrasonicsArrayDesc);
    return Void();
}


// TODO(b/149874793): Add implementation for EVS Manager and Sample driver
Return<sp<IEvsUltrasonicsArray>> EvsEnumerator::openUltrasonicsArray(
        const hidl_string& ultrasonicsArrayId) {
    (void)ultrasonicsArrayId;
    return sp<IEvsUltrasonicsArray>();
}


// TODO(b/149874793): Add implementation for EVS Manager and Sample driver
Return<void> EvsEnumerator::closeUltrasonicsArray(
        const ::android::sp<IEvsUltrasonicsArray>& evsUltrasonicsArray)  {
    (void)evsUltrasonicsArray;
    return Void();
}

EvsEnumerator::~EvsEnumerator() {
    LOG(DEBUG) << "EvsAISEnumerator destructor";
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
    if (QCARCAM_RET_OK != ret || queryNumInputs == 0) {
        LOG(ERROR) << "Failed QCarCamQueryInputs number of inputs="<<queryNumInputs<<" ret="<<ret;
    } else {
        pAisInputList = (QCarCamInput_t *)calloc(queryNumInputs, sizeof(*pAisInputList));
        mpStreamConfigs = (StreamConfigs_t *)calloc(queryNumInputs, sizeof(*mpStreamConfigs));
        if (pAisInputList && mpStreamConfigs) {
            ret = QCarCamQueryInputs(pAisInputList, queryNumInputs, &mNumAisInputs);
            if (QCARCAM_RET_OK != ret || mNumAisInputs != queryNumInputs)
                LOG(ERROR) <<"Failed QCarCamQueryInputs with ret "<<ret;
            else {
                LOG(ERROR) <<"--- QCarCam Queried Inputs ----";
                for (i = 0; i < (int)mNumAisInputs; i++) {

                    QCarCamRet_e ret = QCARCAM_RET_OK;
                    QCarCamMode_t *pModes = (QCarCamMode_t *)calloc(pAisInputList[i].numModes, sizeof(QCarCamMode_t));
                    if (pModes == NULL)
                    {
                        LOG(ERROR) <<"Failed to allocate memory for QCarCamMode";
                        return QCARCAM_RET_FAILED;
                    }
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

                        LOG(ERROR) <<i<<" input_id = "<<pAisInputList[i].inputId
                        <<" inputName = "<<pAisInputList[i].inputName
                        <<" flags = "<< pAisInputList[i].flags
                        <<" numModes = "<<pAisInputList[i].numModes
                        <<" width = "<<mpStreamConfigs[i].width
                        <<" height = "<<mpStreamConfigs[i].height
                        <<" colorFmt = "<<mpStreamConfigs[i].colorFmt
                        <<" fps = "<<mpStreamConfigs[i].fps;

                        snprintf(camera_name, sizeof(camera_name), "%d",i);
                        CameraRecord cam(camera_name);
                        if (sConfigManager != nullptr) {
                            if (sConfigManager->externalMetadataEnabled()) {
                                unique_ptr<ConfigManager::CameraInfo> &camInfo =
                                    sConfigManager->getCameraInfo(camera_name);
                                if (camInfo != nullptr) {
                                    cam.desc.metadata.setToExternal(
                                            (uint8_t *)camInfo->characteristics,
                                            get_camera_metadata_size(camInfo->characteristics)
                                            );
                                }
                            } else {
                                LOG(ERROR) << "Storing qcarcam camera info as stream metadata";
                                unique_ptr<ConfigManager::CameraInfo> &camInfo =
                                    sConfigManager->setCameraInfo(camera_name, &mpStreamConfigs[i]);
                                if (camInfo != nullptr) {
                                    cam.desc.metadata.setToExternal(
                                            (uint8_t *)camInfo->characteristics,
                                            get_camera_metadata_size(camInfo->characteristics)
                                            );
                                } else
                                    LOG(ERROR) << "Unable create camera stream metadata";
                            }
                        }
                        sCameraList.emplace(camera_name, cam);
                        DeviceIDToQcarcamIDList.emplace(std::to_string(i),std::to_string(pAisInputList[i].inputId));
                        captureCount++;
                    } else
                        LOG(ERROR) << "QCarCamQueryInputModes failed with ret "<<ret;
                }
            }
        }
    }
    LOG(ERROR) << "Found "<<captureCount<<" qualified video capture devices found";
    if (pAisInputList)
        free(pAisInputList);
    return ret;
}

} // namespace implementation
} // namespace V1_1
} // namespace evs
} // namespace automotive
} // namespace hardware
} // namespace android
