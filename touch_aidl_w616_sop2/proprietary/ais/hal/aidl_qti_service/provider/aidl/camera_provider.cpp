/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *Not a contribution.
 *
 * Copyright 2016 The Android Open Source Project
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

#define LOG_TAG "AidlService.cameraProvider"
#include <android/log.h>

#if ENABLE_CAM_DEVICE_1_0
#include "CameraDevice_1_0.h"
#endif // ENABLE_CAM_DEVICE_1_0
#include "camera_device.h"


#include "camera_provider.h"
#include <cutils/properties.h>
#include <regex>
#include <string.h>
#include <utils/Trace.h>


namespace android {
namespace hardware {
namespace camera {
namespace provider {
namespace implementation {

using aidl::android::hardware::camera::provider::ICameraProvider;
using aidl::android::hardware::camera::common::CameraMetadataType;
using aidl::android::hardware::camera::common::Status;
using aidl::android::hardware::camera::provider::ICameraProvider;
using ::android::sp;

namespace {
// "device@<version>/vendor_qti/<id>"
const std::regex kDeviceNameRE("device@([0-9]+\\.[0-9]+)/vendor_qti/(.+)");
const char *kHALAIDL1_1 = "1.1";

const int kMaxCameraDeviceNameLen = 128;
const int kMaxCameraIdLen = 16;

bool matchDeviceName(const std::string& deviceName, std::string* deviceVersion,
                     std::string* cameraId) {
    std::string deviceNameStd(deviceName.c_str());
    std::smatch sm;
    if (std::regex_match(deviceNameStd, sm, kDeviceNameRE)) {
        if (deviceVersion != nullptr) {
            *deviceVersion = sm[1];
        }
        if (cameraId != nullptr) {
            *cameraId = sm[2];
        }
        return true;
    }
    return false;
}

} // anonymous namespace


::ndk::ScopedAStatus CameraProvider::convertToScopedAStatus(Status status)
{
    if (status == Status::OK) {
        return ndk::ScopedAStatus::ok();
        }

    return ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(
        static_cast<int32_t>(status),
        ::aidl::android::hardware::camera::common::toString(status).c_str());
}

void CameraProvider::addDeviceNames(int camera_id, CameraDeviceStatus status, bool cam_new)
{

    char cameraId[kMaxCameraIdLen];
    snprintf(cameraId, sizeof(cameraId), "%d", camera_id);
    std::string cameraIdStr(cameraId);

    mCameraIds.add(cameraIdStr);

    // initialize mCameraDeviceNames and mOpenLegacySupported
    mOpenLegacySupported[cameraIdStr] = false;
    int deviceVersion = mModule->getDeviceVersion(camera_id);
    auto deviceNamePair = std::make_pair(cameraIdStr,
                                         getAidlDeviceName(cameraIdStr, deviceVersion));
    mCameraDeviceNames.add(deviceNamePair);
    if (cam_new) {
       auto result = mCallbacks->cameraDeviceStatusChange(deviceNamePair.second, status);
       if (!result.isOk()) {
          ALOGW("%s: failed to send cameraDeviceStatusChange", __FUNCTION__);
       }
    }
    if (deviceVersion >= CAMERA_DEVICE_API_VERSION_3_2 &&
            mModule->isOpenLegacyDefined()) {
        // try open_legacy to see if it actually works
        if ((property_get_bool("ro.config.low_ram", /*default*/ false))) {
            deviceNamePair = std::make_pair(cameraIdStr,
                            getAidlDeviceName(cameraIdStr, CAMERA_DEVICE_API_VERSION_1_0));
            mCameraDeviceNames.add(deviceNamePair);
            if (cam_new) {
                auto result = mCallbacks->cameraDeviceStatusChange(deviceNamePair.second, status);
                if (!result.isOk()) {
                 ALOGW("%s: failed to send cameraDeviceStatusChange", __FUNCTION__);
                }
            }
        }
        else {
            struct hw_device_t* halDev = nullptr;
            int ret = mModule->openLegacy(cameraId, CAMERA_DEVICE_API_VERSION_1_0, &halDev);
            if (ret == 0) {
               mOpenLegacySupported[cameraIdStr] = true;
               halDev->close(halDev);
               deviceNamePair = std::make_pair(cameraIdStr,
                                getAidlDeviceName(cameraIdStr, CAMERA_DEVICE_API_VERSION_1_0));
               mCameraDeviceNames.add(deviceNamePair);
               if (cam_new) {
                 auto result = mCallbacks->cameraDeviceStatusChange(deviceNamePair.second, status);
                 if (!result.isOk()) {
                    ALOGW("%s: failed to send cameraDeviceStatusChange", __FUNCTION__);
                 }
              }
            } else if (ret == -EBUSY || ret == -EUSERS) {
            // Looks like this provider instance is not initialized during
            // system startup and there are other camera users already.
            // Not a good sign but not fatal.
              ALOGW("%s: open_legacy try failed!", __FUNCTION__);
            }
        }
    }
}

void CameraProvider::removeDeviceNames(int camera_id)
{
    std::string cameraIdStr = std::to_string(camera_id);

    mCameraIds.remove(cameraIdStr);

    int deviceVersion = mModule->getDeviceVersion(camera_id);
    auto deviceNamePair = std::make_pair(cameraIdStr,
                                         getAidlDeviceName(cameraIdStr, deviceVersion));
    mCameraDeviceNames.remove(deviceNamePair);
    auto result = mCallbacks->cameraDeviceStatusChange(deviceNamePair.second, CameraDeviceStatus::NOT_PRESENT);
    if (!result.isOk()) {
       ALOGW("%s: failed to send cameraDeviceStatusChange", __FUNCTION__);
    }
    if (deviceVersion >= CAMERA_DEVICE_API_VERSION_3_2 &&
        mModule->isOpenLegacyDefined() && mOpenLegacySupported[cameraIdStr]) {

        deviceNamePair = std::make_pair(cameraIdStr,
                            getAidlDeviceName(cameraIdStr, CAMERA_DEVICE_API_VERSION_1_0));
        mCameraDeviceNames.remove(deviceNamePair);
        auto result = mCallbacks->cameraDeviceStatusChange(deviceNamePair.second,
                                             CameraDeviceStatus::NOT_PRESENT);
        if (!result.isOk()) {
           ALOGW("%s: failed to send cameraDeviceStatusChange", __FUNCTION__);
        }
    }

    mModule->removeCamera(camera_id);
}

/**
 * static callback forwarding methods from HAL to instance
 */
void CameraProvider::sCameraDeviceStatusChange(
        const struct camera_module_callbacks* callbacks,
        int camera_id,
        int new_status) {
    CameraProvider* cp = const_cast<CameraProvider*>(
            static_cast<const CameraProvider*>(callbacks));
    if (cp == nullptr) {
        ALOGE("%s: callback ops is null", __FUNCTION__);
        return;
    }

    Mutex::Autolock _l(cp->mCbLock);
    char cameraId[kMaxCameraIdLen];
    snprintf(cameraId, sizeof(cameraId), "%d", camera_id);
    std::string cameraIdStr(cameraId);
    cp->mCameraStatusMap[cameraIdStr] = (camera_device_status_t) new_status;

    if (cp->mCallbacks == nullptr) {
        // For camera connected before mCallbacks is set, the corresponding
        // addDeviceNames() would be called later in setCallbacks().
        return;
    }

    bool found = false;
    CameraDeviceStatus status = (CameraDeviceStatus)new_status;
    for (auto const& deviceNamePair : cp->mCameraDeviceNames) {
        if (cameraIdStr.compare(deviceNamePair.first) == 0) {
            auto result = cp->mCallbacks->cameraDeviceStatusChange(deviceNamePair.second, status);
             if (!result.isOk()) {
                ALOGE("%s: failed to send cameraDeviceStatusChange", __FUNCTION__);
             }
            found = true;
        }
    }

    switch (status) {
        case CameraDeviceStatus::PRESENT:
        case CameraDeviceStatus::ENUMERATING:
            if (!found) {
                cp->addDeviceNames(camera_id, status, true);
            }
            break;
        case CameraDeviceStatus::NOT_PRESENT:
            if (found) {
                cp->removeDeviceNames(camera_id);
            }
    }
}

void CameraProvider::sTorchModeStatusChange(
        const struct camera_module_callbacks* callbacks,
        const char* camera_id,
        int new_status) {
    CameraProvider* cp = const_cast<CameraProvider*>(
            static_cast<const CameraProvider*>(callbacks));

    if (cp == nullptr) {
        ALOGE("%s: callback ops is null", __FUNCTION__);
        return;
    }

    Mutex::Autolock _l(cp->mCbLock);
    if (cp->mCallbacks != nullptr) {
        std::string cameraIdStr(camera_id);
        TorchModeStatus status = (TorchModeStatus) new_status;
        for (auto const& deviceNamePair : cp->mCameraDeviceNames) {
            if (cameraIdStr.compare(deviceNamePair.first) == 0) {
                auto result = cp->mCallbacks->torchModeStatusChange(
                                deviceNamePair.second, status);
                if (!result.isOk()) {
                    ALOGE("%s: failed to send torch status", __FUNCTION__);
                }
            }
        }
    }
}

Status CameraProvider::getAidlStatus(int status) {
    switch (status) {
        case 0: return Status::OK;
        case -ENODEV: return Status::INTERNAL_ERROR;
        case -EINVAL: return Status::ILLEGAL_ARGUMENT;
        default:
            ALOGE("%s: unknown HAL status code %d", __FUNCTION__, status);
            return Status::INTERNAL_ERROR;
    }
}

std::string CameraProvider::getAidlDeviceName(
        std::string cameraId, int deviceVersion) {

    int versionMajor = 1;
    int versionMinor = 1;
    char deviceName[kMaxCameraDeviceNameLen];
    snprintf(deviceName, sizeof(deviceName), "device@%d.%d/vendor_qti/%s",
            versionMajor, versionMinor, cameraId.c_str());
    return deviceName;
}

CameraProvider::CameraProvider() :
        camera_module_callbacks_t({sCameraDeviceStatusChange,
                                   sTorchModeStatusChange}) {
    mInitFailed = initialize();
}

CameraProvider::~CameraProvider() {

}

bool CameraProvider::initialize() {
    camera_module_t *rawModule;
    int err = hw_get_module(CAMERA_HARDWARE_MODULE_ID,
            (const hw_module_t **)&rawModule);
    if (err < 0) {
        ALOGE("Could not load camera HAL module: %d (%s)", err, strerror(-err));
        return true;
    }

    mModule = std::make_shared<CameraModule>(rawModule);
    err = mModule->init();
    if (err != OK) {
        ALOGE("Could not initialize camera HAL module: %d (%s)", err, strerror(-err));
        mModule.reset();
        return true;
    }
    ALOGI("Loaded \"%s\" camera module", mModule->getModuleName());

    // Setup vendor tags here so HAL can setup vendor keys in camera characteristics
    VendorTagDescriptor::clearGlobalVendorTagDescriptor();
    if (!setUpVendorTags()) {
        ALOGE("%s: Vendor tag setup failed, will not be available.", __FUNCTION__);
    }

    // Setup callback now because we are going to try openLegacy next
    err = mModule->setCallbacks(this);
    if (err != OK) {
        ALOGE("Could not set camera module callback: %d (%s)", err, strerror(-err));
        mModule.reset();
        return true;
    }

    mPreferredHal3MinorVersion =
        property_get_int32("ro.vendor.camera.wrapper.hal3TrebleMinorVersion", 3);
    ALOGV("Preferred HAL 3 minor version is %d", mPreferredHal3MinorVersion);
    switch(mPreferredHal3MinorVersion) {
        case 2:
        case 3:
            // OK
            break;
        default:
            ALOGW("Unknown minor camera device HAL version %d in property "
                    "'camera.wrapper.hal3TrebleMinorVersion', defaulting to 3",
                    mPreferredHal3MinorVersion);
            mPreferredHal3MinorVersion = 3;
    }

    mNumberOfLegacyCameras = mModule->getNumberOfCameras();
    ALOGE("Num of cameras identified: %d",mNumberOfLegacyCameras);
    for (int i = 0; i < mNumberOfLegacyCameras; i++) {
        uint32_t device_version;
        auto rc = mModule->getCameraDeviceVersion(i, &device_version);
        if (rc != NO_ERROR) {
            ALOGE("%s: Camera device version query failed!", __func__);
            mModule.reset();
            return true;
        }

        if (checkCameraVersion(i, device_version) != OK) {
            ALOGE("%s: Camera version check failed!", __func__);
            mModule.reset();
            return true;
        }

        char cameraId[kMaxCameraIdLen];
        snprintf(cameraId, sizeof(cameraId), "%d", i);
        std::string cameraIdStr(cameraId);
        mCameraStatusMap[cameraIdStr] = CAMERA_DEVICE_STATUS_PRESENT;

        addDeviceNames(i);
    }
    return false; // mInitFailed
}

/**
 * Check that the device HAL version is still in supported.
 */
int CameraProvider::checkCameraVersion(int id, uint32_t device_version) {
    if (mModule == nullptr) {
        return NO_INIT;
    }

    // device_version undefined in CAMERA_MODULE_API_VERSION_1_0,
    // All CAMERA_MODULE_API_VERSION_1_0 devices are backward-compatible
    uint16_t moduleVersion = mModule->getModuleApiVersion();
    if (moduleVersion >= CAMERA_MODULE_API_VERSION_2_0) {
        // Verify the device version is in the supported range
        switch (device_version) {
            case CAMERA_DEVICE_API_VERSION_1_0:
            case CAMERA_DEVICE_API_VERSION_3_2:
            case CAMERA_DEVICE_API_VERSION_3_3:
            case CAMERA_DEVICE_API_VERSION_3_4:
            case CAMERA_DEVICE_API_VERSION_3_5:
                // in support
                break;
            case CAMERA_DEVICE_API_VERSION_3_6:
            case CAMERA_DEVICE_API_VERSION_3_7:
                /**
                 * ICameraDevice@3.5 contains APIs from both
                 * CAMERA_DEVICE_API_VERSION_3_6 and CAMERA_MODULE_API_VERSION_2_5
                 * so we require HALs to uprev both for simplified supported combinations.
                 * HAL can still opt in individual new APIs indepedently.
                 */
                if (moduleVersion < CAMERA_MODULE_API_VERSION_2_5) {
                    ALOGE("%s: Device %d has unsupported version combination:"
                            "HAL version %x and module version %x",
                            __FUNCTION__, id, device_version, moduleVersion);
                    return NO_INIT;
                }
                break;
            case CAMERA_DEVICE_API_VERSION_2_0:
            case CAMERA_DEVICE_API_VERSION_2_1:
            case CAMERA_DEVICE_API_VERSION_3_0:
            case CAMERA_DEVICE_API_VERSION_3_1:
                // no longer supported
            default:
                ALOGE("%s: Device %d has HAL version %x, which is not supported", __FUNCTION__, id,
                      device_version);
                return NO_INIT;
        }
    }
    return OK;
}

bool CameraProvider::setUpVendorTags() {
    ATRACE_CALL();
    vendor_tag_ops_t vOps = vendor_tag_ops_t();

    // Check if vendor operations have been implemented
    if (!mModule->isVendorTagDefined()) {
        ALOGI("%s: No vendor tags defined for this device.", __FUNCTION__);
        return true;
    }

    mModule->getVendorTagOps(&vOps);

    // Ensure all vendor operations are present
    if (vOps.get_tag_count == nullptr || vOps.get_all_tags == nullptr ||
            vOps.get_section_name == nullptr || vOps.get_tag_name == nullptr ||
            vOps.get_tag_type == nullptr) {
        ALOGE("%s: Vendor tag operations not fully defined. Ignoring definitions."
               , __FUNCTION__);
        return false;
    }

    // Read all vendor tag definitions into a descriptor
    sp<VendorTagDescriptor> desc;
    status_t res;
    if ((res = VendorTagDescriptor::createDescriptorFromOps(&vOps, /*out*/desc))
            != OK) {
        ALOGE("%s: Could not generate descriptor from vendor tag operations,"
              "received error %s (%d). Camera clients will not be able to use"
              "vendor tags", __FUNCTION__, strerror(res), res);
        return false;
    }

    // Set the global descriptor to use with camera metadata
    VendorTagDescriptor::setAsGlobalVendorTagDescriptor(desc);
    const SortedVector<String8>* sectionNames = desc->getAllSectionNames();
    size_t numSections = sectionNames->size();
    std::vector<std::vector<VendorTag>> tagsBySection(numSections);
    int tagCount = desc->getTagCount();
    std::vector<uint32_t> tags(tagCount);
    desc->getTagArray(tags.data());
    for (int i = 0; i < tagCount; i++) {
        VendorTag vt;
        vt.tagId = tags[i];
        vt.tagName = desc->getTagName(tags[i]);
        vt.tagType = (CameraMetadataType) desc->getTagType(tags[i]);
        ssize_t sectionIdx = desc->getSectionIndex(tags[i]);
        tagsBySection[sectionIdx].push_back(vt);
    }
    mVendorTagSections.resize(numSections);
    for (size_t s = 0; s < numSections; s++) {
#ifdef __ANDROID_V__
        mVendorTagSections[s].sectionName = (*sectionNames)[s].c_str();
#else
        mVendorTagSections[s].sectionName = (*sectionNames)[s].string();
#endif
        mVendorTagSections[s].tags = tagsBySection[s];
    }
    return true;
}

// Methods from aidl::android::hardware::camera::provider::ICameraProvider follow.
::ndk::ScopedAStatus CameraProvider::setCallback(
        const std::shared_ptr<ICameraProviderCallback>& callback) {
    Mutex::Autolock _l(mCbLock);
    if (callback == nullptr) {
        ALOGE("%s: callback is nullptr. ", __FUNCTION__);
        return convertToScopedAStatus(Status::ILLEGAL_ARGUMENT);
    }

    mCallbacks = callback;
    // Add and report all presenting external cameras.
    for (auto const& statusPair : mCameraStatusMap) {
        int id = std::stoi(statusPair.first);
        auto status = static_cast<CameraDeviceStatus>(statusPair.second);
        if (id >= mNumberOfLegacyCameras && status != CameraDeviceStatus::NOT_PRESENT) {
            addDeviceNames(id, status, true);
        }
    }
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus CameraProvider::getVendorTags(
        std::vector<VendorTagSection>* vts) {
    if (vts == nullptr) {
        ALOGE("%s: vts is nullptr. ", __FUNCTION__);
        return convertToScopedAStatus(Status::ILLEGAL_ARGUMENT);
    }
    vts->clear();
    if(!mVendorTagSections.empty())
    {
        *vts = mVendorTagSections;
        ALOGI("%s: size:%zu, deviceName:%s", __FUNCTION__, (*vts).size(),
            (*vts)[0].sectionName.c_str());
    }


    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus CameraProvider::getCameraIdList(
        std::vector<std::string>* camera_ids_ret) {
    if (camera_ids_ret == nullptr) {
        return convertToScopedAStatus(Status::ILLEGAL_ARGUMENT);
    }
    camera_ids_ret->clear();
    for (auto const& deviceNamePair : mCameraDeviceNames) {
        if (std::stoi(deviceNamePair.first) >= mNumberOfLegacyCameras) {
            // External camera devices must be reported through the device status change callback,
            // not in this list.
            continue;
        }
        if (mCameraStatusMap[deviceNamePair.first] == CAMERA_DEVICE_STATUS_PRESENT) {
            (*camera_ids_ret).push_back(deviceNamePair.second);
        }
    }

    if (camera_ids_ret->size() > 0)
    {
        for (uint32_t i = 0; i < camera_ids_ret->size(); i++) {
                ALOGI("%s: size:%zu, deviceName:%s", __FUNCTION__, camera_ids_ret->size(),
                    (*camera_ids_ret)[i].c_str());
        }
    }
    else
    {
        ALOGI("%s: size: 0", __FUNCTION__);
    }

    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus CameraProvider::getCameraDeviceInterface(
        const std::string& in_cameraDeviceName,
        std::shared_ptr<ICameraDevice>* device) {
   if (device == nullptr) {
     ALOGE("%s: device is nullptr. ", __FUNCTION__);
     return convertToScopedAStatus(Status::ILLEGAL_ARGUMENT);
   }

    std::string cameraId, deviceVersion;
    bool match = matchDeviceName(in_cameraDeviceName, &deviceVersion, &cameraId);
    if (!match) {
        ALOGE("%s: Device name match fail. in_cameraDeviceName:%s", __FUNCTION__, in_cameraDeviceName.c_str());
        device = nullptr;
        return convertToScopedAStatus(Status::ILLEGAL_ARGUMENT);
    }

    std::string deviceName(in_cameraDeviceName.c_str());
    ssize_t index = mCameraDeviceNames.indexOf(std::make_pair(cameraId, deviceName));
    if (index == NAME_NOT_FOUND) { // Either an illegal name or a device version mismatch
        Status status = Status::OK;
        ssize_t idx = mCameraIds.indexOf(cameraId);
        if (idx == NAME_NOT_FOUND) {
            ALOGE("%s: cannot find camera %s!", __FUNCTION__, cameraId.c_str());
            status = Status::ILLEGAL_ARGUMENT;
        } else { // invalid version
            ALOGE("%s: camera device %s does not support version %s!",
                    __FUNCTION__, cameraId.c_str(), deviceVersion.c_str());
            status = Status::OPERATION_NOT_SUPPORTED;
        }
        device = nullptr;
        return convertToScopedAStatus(status);
    }

    if (mCameraStatusMap.count(cameraId) == 0 ||
            mCameraStatusMap[cameraId] != CAMERA_DEVICE_STATUS_PRESENT) {
        ALOGE("%s: AIDL , Status:%d!",
            __FUNCTION__, mCameraStatusMap[cameraId]);
        device = nullptr;
        return convertToScopedAStatus(Status::ILLEGAL_ARGUMENT);
    }

    // ICameraDevice 1.1 for AIDL
    if (deviceVersion >= kHALAIDL1_1) {
        ALOGI("Constructing v1.1+ AIDL camera device");
        *device =
            ndk::SharedRefBase::make<android::hardware::camera::device::implementation::CameraDevice>(mModule, cameraId, mCameraDeviceNames);

        if ((*device) == nullptr) {
            ALOGE("%s: Creating CameraDevice %s failed!", __FUNCTION__, cameraId.c_str());
            device = nullptr;
            return convertToScopedAStatus(Status::INTERNAL_ERROR);
        }
    }
    else
    {
        ALOGE("%s: AIDL camera device %s does not support version %s!",
            __FUNCTION__, cameraId.c_str(), deviceVersion.c_str());
        device = nullptr;
        return convertToScopedAStatus(Status::ILLEGAL_ARGUMENT);
    }
    return convertToScopedAStatus(Status::OK);
}

::ndk::ScopedAStatus CameraProvider::notifyDeviceStateChange(
    int64_t in_deviceState) {
    ALOGI("%s: New device state: 0x%" PRIx64, __FUNCTION__, in_deviceState);
    uint64_t state = static_cast<uint64_t>(in_deviceState);
    mModule->notifyDeviceStateChange(state);
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus CameraProvider::getConcurrentCameraIds(
    std::vector<ConcurrentCameraIdCombination>* concurrent_camera_ids) {
    if (concurrent_camera_ids == nullptr) {
        ALOGE("%s: concurrent_camera_ids is nullptr. ", __FUNCTION__);
        return convertToScopedAStatus(Status::ILLEGAL_ARGUMENT);
    }
    concurrent_camera_ids->clear();

    concurrent_camera_combination_t*  pConcCamArray      = NULL;
    uint32_t                          concCamArrayLength = 0;
    Status                            status             = Status::OPERATION_NOT_SUPPORTED;

    ALOGI("%s Calling getConcurrentStreamingCameraIds", __FUNCTION__);
    int res = mModule->getConcurrentStreamingCameraIds(&concCamArrayLength, &pConcCamArray);

    if ((OK == res) && (0 != concCamArrayLength) && (NULL != pConcCamArray))
    {
        (*concurrent_camera_ids).resize(concCamArrayLength);
        int camIdComboIndex = 0;

        for (int camIdComboIndex = 0; camIdComboIndex < concCamArrayLength; ++camIdComboIndex)
        {
            std::vector<std::string> aidlCombination(pConcCamArray[camIdComboIndex].numCameras);

            for (int camIndex = 0; camIndex < pConcCamArray[camIdComboIndex].numCameras; ++camIndex)
            {
                aidlCombination[camIndex] =
                    std::to_string(pConcCamArray[camIdComboIndex].concurrentCamArray[camIndex]);
            }
            (*concurrent_camera_ids)[camIdComboIndex].combination = aidlCombination;
        }

        // Memory allocated by caller needs to be freed
        delete [] pConcCamArray;

        status = Status::OK;
    }

    ALOGI("%s called, number of concurrent combinations supported is %d", __FUNCTION__, concCamArrayLength);

    return convertToScopedAStatus(status);
}


::ndk::ScopedAStatus CameraProvider::isConcurrentStreamCombinationSupported(
        const std::vector<CameraIdAndStreamCombination>& in_configs,
        bool* support)
{
    Status status      = Status::OK;
    bool   queryStatus = false;
    size_t numEntries  = in_configs.size();
    int    res         = NO_ERROR;

    std::vector<cameraid_stream_combination_t> cameraIdStreamComboVec;

    for (const auto &camIdStreamCombo : in_configs)
    {
        cameraid_stream_combination_t cameraIdStreamComboEntry = {};

        cameraIdStreamComboEntry.streamConfig = new camera_stream_combination_t;

        camera_stream_combination_t* pStreamComb = cameraIdStreamComboEntry.streamConfig;

        if (NULL == pStreamComb)
        {
            res = NO_MEMORY;
            break;
        }

        pStreamComb->operation_mode = static_cast<uint32_t> (camIdStreamCombo.streamConfiguration.operationMode);
        pStreamComb->num_streams    = camIdStreamCombo.streamConfiguration.streams.size();
        pStreamComb->streams        = new camera_stream_t[pStreamComb->num_streams];

        if (NULL == pStreamComb->streams)
        {
            delete pStreamComb;
            res = NO_MEMORY;
            break;
        }

        camera_stream_t* pStreamBuffer = pStreamComb->streams;
        size_t           strmIndex     = 0;

        for (const auto &it : camIdStreamCombo.streamConfiguration.streams)
        {
            pStreamBuffer[strmIndex].stream_type         = static_cast<int> (it.streamType);
            pStreamBuffer[strmIndex].width               = it.width;
            pStreamBuffer[strmIndex].height              = it.height;
            pStreamBuffer[strmIndex].format              = static_cast<int> (it.format);
            pStreamBuffer[strmIndex].data_space          = static_cast<android_dataspace_t> (it.dataSpace);
            pStreamBuffer[strmIndex].usage               = static_cast<uint32_t> (it.usage);
            pStreamBuffer[strmIndex].physical_camera_id  = it.physicalCameraId.c_str();
            pStreamBuffer[strmIndex++].rotation          = static_cast<int> (it.rotation);
        }

        cameraIdStreamComboVec.push_back(cameraIdStreamComboEntry);
    }

    if (NO_ERROR == res)
    {
        res = mModule->isConcurrentStreamCombinationSupported(cameraIdStreamComboVec);
    }

    switch (res)
    {
        case NO_ERROR:
            queryStatus = true;
            status      = Status::OK;
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

    for (auto &camIdStreamCombo : cameraIdStreamComboVec)
    {
        delete [] camIdStreamCombo.streamConfig->streams;
        delete camIdStreamCombo.streamConfig;
    }

    *support = queryStatus;
    ALOGI("%s called, queryStatus %d, status %d, number of concurrent Cameras %zu",
          __FUNCTION__, queryStatus, status, numEntries);
    return convertToScopedAStatus(status);
}


}  // namespace implementation
}  // namespace provider
}  // namespace camera
}  // namespace hardware
}  // namespace android
