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

#define LOG_TAG "AidlService.cameraDeviceSession"
#include <android/log.h>
#include <cutils/properties.h>
#include <optional>
#include <set>
#include <utils/Trace.h>
#include <vector>
#include "CameraModule.h"
#include "include/camera_device_session.h"

namespace android {
namespace hardware {
namespace camera {
namespace device {
namespace implementation {

using ::aidl::android::hardware::camera::device::ErrorCode;
using ::android::hardware::camera::common::V1_0::helper::CameraModule;

// Size of request metadata fast message queue. Change to 0 to always use hwbinder buffer.
static constexpr int32_t CAMERA_REQUEST_METADATA_QUEUE_SIZE = 1 << 20 /* 1MB */;
// Size of result metadata fast message queue. Change to 0 to always use hwbinder buffer.
static constexpr int32_t CAMERA_RESULT_METADATA_QUEUE_SIZE  = 1 << 20 /* 1MB */;

// Metadata sent by HAL will be replaced by a compact copy
// if their (total size >= compact size + METADATA_SHRINK_ABS_THRESHOLD &&
//           total_size >= compact size * METADATA_SHRINK_REL_THRESHOLD)
// Heuristically picked by size of one page
static constexpr int     METADATA_SHRINK_ABS_THRESHOLD      = 4096;
static constexpr int     METADATA_SHRINK_REL_THRESHOLD      = 2;

HandleImporter CameraDeviceSession::sHandleImporter;
buffer_handle_t CameraDeviceSession::sEmptyBuffer = nullptr;

const int CameraDeviceSession::ResultBatcher::NOT_BATCHED;

CameraDeviceSession::CameraDeviceSession(
    camera3_device_t*                             device,
    const camera_metadata_t*                      deviceInfo,
    const std::shared_ptr<ICameraDeviceCallback>& callback) :
    camera3_callback_ops({&sProcessCaptureResult, &sNotify, nullptr, nullptr}),
        mDevice(device),
        mDeviceVersion(device->common.version),
        mFreeBufEarly(shouldFreeBufEarly()),
        mIsAELockAvailable(false),
        mDerivePostRawSensKey(false),
        mNumPartialResults(1),
        mResultBatcher(callback)
{
    mDeviceInfo = deviceInfo;
    camera_metadata_entry partialResultsCount =
        mDeviceInfo.find(ANDROID_REQUEST_PARTIAL_RESULT_COUNT);

    if (partialResultsCount.count > 0) {
        mNumPartialResults = partialResultsCount.data.i32[0];
    }

    camera_metadata_entry aeLockAvailableEntry = mDeviceInfo.find(
            ANDROID_CONTROL_AE_LOCK_AVAILABLE);
    if (aeLockAvailableEntry.count > 0) {
        mIsAELockAvailable = (aeLockAvailableEntry.data.u8[0] ==
                ANDROID_CONTROL_AE_LOCK_AVAILABLE_TRUE);
    }

    // Determine whether we need to derive sensitivity boost values for older devices.
    // If post-RAW sensitivity boost range is listed, so should post-raw sensitivity control
    // be listed (as the default value 100)
    if (mDeviceInfo.exists(ANDROID_CONTROL_POST_RAW_SENSITIVITY_BOOST_RANGE)) {
        mDerivePostRawSensKey = true;
    }

    mInitFail              = initialize();
    process_capture_result = sProcessCaptureResult;
    notify                 = sNotify;

    if (!mInitFail) {
        mResultBatcher.setResultMetadataQueue(mResultMetadataQueue);
    }

    mResultBatcher.setNumPartialResults(mNumPartialResults);
    // Parse and store current logical camera's physical ids.
    (void)CameraModule::isLogicalMultiCamera(mDeviceInfo, &mPhysicalCameraIds);

    aidl_device_callback    = callback;
    mSupportBufMgr          = false;
    camera_metadata_entry bufMgrVersion = mDeviceInfo.find(
            ANDROID_INFO_SUPPORTED_BUFFER_MANAGEMENT_VERSION);
    if (bufMgrVersion.count > 0) {
        mSupportBufMgr = (bufMgrVersion.data.u8[0] ==
                ANDROID_INFO_SUPPORTED_BUFFER_MANAGEMENT_VERSION_HIDL_DEVICE_3_5);
        if (mSupportBufMgr) {
            request_stream_buffers = sRequestStreamBuffers;
            return_stream_buffers = sReturnStreamBuffers;
        }
    }
}

bool CameraDeviceSession::initialize() {
    /** Initialize device with callback functions */
    ATRACE_BEGIN("camera3->initialize");
    status_t res = mDevice->ops->initialize(mDevice, this);
    ATRACE_END();

    if (res != OK) {
        ALOGE("%s: Unable to initialize HAL device: %s (%d)",
                __FUNCTION__, strerror(-res), res);
        mDevice->common.close(&mDevice->common);
        mClosed = true;
        return true;
    }

    // "ro.camera" properties are no longer supported on vendor side.
    //  Support a fall back for the fmq size override that uses "ro.vendor.camera"
    //  properties.
    int32_t reqFMQSize = property_get_int32("ro.vendor.camera.req.fmq.size", /*default*/-1);

    if (reqFMQSize < 0) {
        reqFMQSize = property_get_int32("ro.camera.req.fmq.size", /*default*/-1);

        if (reqFMQSize < 0) {
            reqFMQSize = CAMERA_REQUEST_METADATA_QUEUE_SIZE;
        } else {
            ALOGV("%s: request FMQ size overridden to %d", __FUNCTION__, reqFMQSize);
        }
    } else {
        ALOGV("%s: request FMQ size overridden to %d via fallback property", __FUNCTION__,
                reqFMQSize);
    }

    mRequestMetadataQueue = std::make_unique<RequestMetadataQueue>(
            static_cast<size_t>(reqFMQSize),
            false /* non blocking */);

    if (!mRequestMetadataQueue->isValid()) {
        ALOGE("%s: invalid request fmq", __FUNCTION__);
        return true;
    }

    // "ro.camera" properties are no longer supported on vendor side.
    //  Support a fall back for the fmq size override that uses "ro.vendor.camera"
    //  properties.
    int32_t resFMQSize = property_get_int32("ro.vendor.camera.res.fmq.size", /*default*/-1);

    if (resFMQSize < 0) {
        resFMQSize = property_get_int32("ro.camera.res.fmq.size", /*default*/-1);

        if (resFMQSize < 0) {
            resFMQSize = CAMERA_RESULT_METADATA_QUEUE_SIZE;
        } else {
            ALOGV("%s: result FMQ size overridden to %d", __FUNCTION__, resFMQSize);
        }
    } else {
        ALOGV("%s: result FMQ size overridden to %d via fallback property", __FUNCTION__,
                resFMQSize);
    }

    mResultMetadataQueue = std::make_shared<ResultMetadataQueue>(
            static_cast<size_t>(resFMQSize),
            false /* non blocking */);

    if (!mResultMetadataQueue->isValid()) {
        ALOGE("%s: invalid result fmq", __FUNCTION__);
        return true;
    }

    mResultBatcher.setResultMetadataQueue(mResultMetadataQueue);

    return false;
}

bool CameraDeviceSession::shouldFreeBufEarly() {
    return property_get_bool("ro.vendor.camera.free_buf_early", 0) == 1;
}

CameraDeviceSession::~CameraDeviceSession() {
    if (!isClosed()) {
        ALOGE("CameraDeviceSession deleted before close!");
        close();
    }
}

bool CameraDeviceSession::isClosed() {
    Mutex::Autolock _l(mStateLock);
    return mClosed;
}

Status CameraDeviceSession::initStatus() const {
    Mutex::Autolock _l(mStateLock);
    Status status = Status::OK;
    if (mInitFail) {
        status = Status::INTERNAL_ERROR;
    } else if (mDisconnected) {
        status = Status::CAMERA_DISCONNECTED;
    } else if (mClosed) {
        status = Status::INTERNAL_ERROR;
    }
    return status;
}

void CameraDeviceSession::disconnect() {
    Mutex::Autolock _l(mStateLock);
    mDisconnected = true;
    ALOGW("%s: Camera device is disconnected. Closing.", __FUNCTION__);
    if (!mClosed) {
        mDevice->common.close(&mDevice->common);
        mClosed = true;
    }
}

void CameraDeviceSession::dumpState(int fd) {
    if (!isClosed()) {
        mDevice->ops->dump(mDevice, fd);
    }
}

/**
 * For devices <= CAMERA_DEVICE_API_VERSION_3_2, AE_PRECAPTURE_TRIGGER_CANCEL is not supported so
 * we need to override AE_PRECAPTURE_TRIGGER_CANCEL to AE_PRECAPTURE_TRIGGER_IDLE and AE_LOCK_OFF
 * to AE_LOCK_ON to start cancelling AE precapture. If AE lock is not available, it still overrides
 * AE_PRECAPTURE_TRIGGER_CANCEL to AE_PRECAPTURE_TRIGGER_IDLE but doesn't add AE_LOCK_ON to the
 * request.
 */
bool CameraDeviceSession::handleAePrecaptureCancelRequestLocked(
        const camera3_capture_request_t&                                   halRequest,
        ::android::hardware::camera::common::V1_0::helper::CameraMetadata* settings /*out*/,
         AETriggerCancelOverride*                                          override /*out*/) {
    if ((mDeviceVersion > CAMERA_DEVICE_API_VERSION_3_2) ||
            (nullptr == halRequest.settings) || (nullptr == settings) ||
            (0 == get_camera_metadata_entry_count(halRequest.settings))) {
        return false;
    }

    settings->clear();
    settings->append(halRequest.settings);
    camera_metadata_entry_t aePrecaptureTrigger =
            settings->find(ANDROID_CONTROL_AE_PRECAPTURE_TRIGGER);
    if (aePrecaptureTrigger.count > 0 &&
            aePrecaptureTrigger.data.u8[0] ==
                    ANDROID_CONTROL_AE_PRECAPTURE_TRIGGER_CANCEL) {
        // Always override CANCEL to IDLE
        uint8_t aePrecaptureTrigger =
                ANDROID_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE;
        settings->update(ANDROID_CONTROL_AE_PRECAPTURE_TRIGGER,
                &aePrecaptureTrigger, 1);
        *override = { false, ANDROID_CONTROL_AE_LOCK_OFF,
                true, ANDROID_CONTROL_AE_PRECAPTURE_TRIGGER_CANCEL };

        if (mIsAELockAvailable == true) {
            camera_metadata_entry_t aeLock = settings->find(
                    ANDROID_CONTROL_AE_LOCK);
            if (aeLock.count == 0 || aeLock.data.u8[0] ==
                    ANDROID_CONTROL_AE_LOCK_OFF) {
                uint8_t aeLock = ANDROID_CONTROL_AE_LOCK_ON;
                settings->update(ANDROID_CONTROL_AE_LOCK, &aeLock, 1);
                override->applyAeLock = true;
                override->aeLock = ANDROID_CONTROL_AE_LOCK_OFF;
            }
        }

        return true;
    }

    return false;
}

Status CameraDeviceSession::importBuffer(int32_t streamId,
        uint64_t bufId, buffer_handle_t buf,
        /*out*/buffer_handle_t** outBufPtr,
        bool allowEmptyBuf) {

    if (buf == nullptr && bufId == BUFFER_ID_NO_BUFFER) {
        if (allowEmptyBuf) {
            *outBufPtr = &sEmptyBuffer;
            return Status::OK;
        } else {
            ALOGE("%s: bufferId %" PRIu64 " has null buffer handle!", __FUNCTION__, bufId);
            return Status::ILLEGAL_ARGUMENT;
        }
    }

    Mutex::Autolock _l(mInflightLock);
    CirculatingBuffers& cbs = mCirculatingBuffers[streamId];
    if (cbs.count(bufId) == 0) {
        // Register a newly seen buffer
        buffer_handle_t importedBuf = sHandleImporter.importBuffer(buf);
        if (importedBuf == nullptr) {
            ALOGE("%s: output buffer for stream %d is invalid!", __FUNCTION__, streamId);
            return Status::INTERNAL_ERROR;
        } else {
            cbs[bufId] = importedBuf;
        }
    }
    *outBufPtr = &cbs[bufId];
    return Status::OK;
}

Status CameraDeviceSession::importRequest(
        const CaptureRequest& request,
        std::vector<buffer_handle_t*>& allBufPtrs,
        std::vector<int>& allFences) {
    if (mSupportBufMgr) {
        return importRequestImpl(request, allBufPtrs, allFences, /*allowEmptyBuf*/ true);
    }
    return importRequestImpl(request, allBufPtrs, allFences, /*allowEmptyBuf*/ false);
}

ndk::ScopedAStatus CameraDeviceSession::signalStreamFlush(
    const std::vector<int32_t>& in_streamIds,
    int32_t in_streamConfigCounter) {
    if (mDevice->ops->signal_stream_flush == nullptr) {
         return convertToScopedAStatus(Status::ILLEGAL_ARGUMENT);
    }

    uint32_t currentCounter = 0;
    {
        Mutex::Autolock _l(mStreamConfigCounterLock);
        currentCounter = mStreamConfigCounter;
    }

    if (in_streamConfigCounter < currentCounter) {
        ALOGV("%s: in_streamConfigCounter %d is stale (current %d), skipping signal_stream_flush call",
                __FUNCTION__, in_streamConfigCounter, mStreamConfigCounter);
         return convertToScopedAStatus(Status::ILLEGAL_ARGUMENT);
    }

    std::vector<camera3_stream_t*> streams(in_streamIds.size());
    {
        Mutex::Autolock _l(mInflightLock);
        for (size_t i = 0; i < in_streamIds.size(); i++) {
            int32_t id = in_streamIds[i];
            if (mStreamMap.count(id) == 0) {
                ALOGE("%s: unknown streamId %d", __FUNCTION__, id);
                 return convertToScopedAStatus(Status::INTERNAL_ERROR);
            }
            streams[i] = &mStreamMap[id];
        }
    }

    mDevice->ops->signal_stream_flush(mDevice, streams.size(), streams.data());
    return ndk::ScopedAStatus::ok();
}

Status CameraDeviceSession::importRequestImpl(
        const CaptureRequest& request,
        std::vector<buffer_handle_t*>& allBufPtrs,
        std::vector<int>& allFences,
        bool allowEmptyBuf) {
    bool hasInputBuf = (request.inputBuffer.streamId != -1 &&
            request.inputBuffer.bufferId != 0);
    size_t numOutputBufs = request.outputBuffers.size();
    size_t numBufs = numOutputBufs + (hasInputBuf ? 1 : 0);
    Status status = Status::OK;

    // Validate all I/O buffers
    std::vector<buffer_handle_t> allBufs;
    std::vector<uint64_t> allBufIds;
    allBufs.resize(numBufs);
    allBufIds.resize(numBufs);
    allBufPtrs.resize(numBufs);
    allFences.resize(numBufs);
    std::vector<int32_t> streamIds(numBufs);

    for (size_t i = 0; i < numOutputBufs; i++) {
        native_handle_t* handle = convertFromAidl(request.outputBuffers[i].buffer);
        allBufs[i] = handle;
        allBufIds[i] = request.outputBuffers[i].bufferId;
        allBufPtrs[i] = &allBufs[i];
        streamIds[i] = request.outputBuffers[i].streamId;
    }
    if (hasInputBuf) {
        native_handle_t* handle = convertFromAidl(request.inputBuffer.buffer);
        allBufs[numOutputBufs] = handle;
        allBufIds[numOutputBufs] = request.inputBuffer.bufferId;
        allBufPtrs[numOutputBufs] = &allBufs[numOutputBufs];
        streamIds[numOutputBufs] = request.inputBuffer.streamId;
    }

    for (size_t i = 0; i < numBufs; i++) {
        status = importBuffer(
                streamIds[i], allBufIds[i], allBufs[i], &allBufPtrs[i],
                // Disallow empty buf for input stream, otherwise follow
                // the allowEmptyBuf argument.
                (hasInputBuf && i == numOutputBufs) ? false : allowEmptyBuf);
        native_handle_delete(const_cast<native_handle_t*>(allBufs[i]));
    }

    if (status == Status::OK) {
        // All buffers are imported. Now validate output buffer acquire fences
        for (size_t i = 0; i < numOutputBufs; i++) {
            native_handle_t* handle = convertFromAidl(request.outputBuffers[i].acquireFence);
            if (!sHandleImporter.importFence(
                    handle, allFences[i])) {
                ALOGE("%s: output buffer %zu acquire fence is invalid", __FUNCTION__, i);
                cleanupInflightFences(allFences, i);
                status = Status::INTERNAL_ERROR;
            }
            native_handle_delete(handle);
        }
    }

    if (status == Status::OK) {
        // Validate input buffer acquire fences
        if (hasInputBuf) {
            native_handle_t* handle = convertFromAidl(request.inputBuffer.acquireFence);
            if (!sHandleImporter.importFence(
                    handle, allFences[numOutputBufs])) {
                ALOGE("%s: input buffer acquire fence is invalid", __FUNCTION__);
                cleanupInflightFences(allFences, numOutputBufs);
                status = Status::INTERNAL_ERROR;
            }
            native_handle_delete(handle);
        }
    }
    return status;
}

void CameraDeviceSession::cleanupInflightFences(
        std::vector<int>& allFences, size_t numFences) {
    for (size_t j = 0; j < numFences; j++) {
        sHandleImporter.closeFence(allFences[j]);
    }
}

// Methods from ::android::hardware::camera::device::ICameraDeviceSession follow.
::ndk::ScopedAStatus CameraDeviceSession::constructDefaultRequestSettings(
    RequestTemplate type,
    CameraMetadata* defaultRequestSettings)  {
    return (nullptr == defaultRequestSettings) ? convertToScopedAStatus(Status::ILLEGAL_ARGUMENT) :
            constructDefaultRequestSettingsRaw( (int) type, defaultRequestSettings);
}

::ndk::ScopedAStatus CameraDeviceSession::constructDefaultRequestSettingsRaw(int type, CameraMetadata *outMetadata) {
    Status status = initStatus();
    const camera_metadata_t *rawRequest;
    if (status == Status::OK) {
        ATRACE_BEGIN("camera3->construct_default_request_settings");
        rawRequest = mDevice->ops->construct_default_request_settings(mDevice, (int) type);
        ATRACE_END();
        if (rawRequest == nullptr) {
            ALOGI("%s: template %d is not supported on this camera device",
                  __FUNCTION__, type);
            status = Status::ILLEGAL_ARGUMENT;
        } else {
            mOverridenRequest.clear();
            mOverridenRequest.append(rawRequest);
            // Derive some new keys for backward compatibility
            if (mDerivePostRawSensKey && !mOverridenRequest.exists(
                    ANDROID_CONTROL_POST_RAW_SENSITIVITY_BOOST)) {
                int32_t defaultBoost[1] = {100};
                mOverridenRequest.update(
                        ANDROID_CONTROL_POST_RAW_SENSITIVITY_BOOST,
                        defaultBoost, 1);
            }
            const camera_metadata_t *metaBuffer =
                    mOverridenRequest.getAndLock();
            convertToAidl(metaBuffer, outMetadata);
            mOverridenRequest.unlock(metaBuffer);
        }
    }
    return convertToScopedAStatus(status);
}

/**
 * Map Android N dataspace definitions back to Android M definitions, for
 * use with HALv3.3 or older.
 *
 * Only map where correspondences exist, and otherwise preserve the value.
 */
android_dataspace CameraDeviceSession::mapToLegacyDataspace(
        android_dataspace dataSpace) const {
    if (mDeviceVersion <= CAMERA_DEVICE_API_VERSION_3_3) {
        switch (dataSpace) {
            case HAL_DATASPACE_V0_SRGB_LINEAR:
                return HAL_DATASPACE_SRGB_LINEAR;
            case HAL_DATASPACE_V0_SRGB:
                return HAL_DATASPACE_SRGB;
            case HAL_DATASPACE_V0_JFIF:
                return HAL_DATASPACE_JFIF;
            case HAL_DATASPACE_V0_BT601_625:
                return HAL_DATASPACE_BT601_625;
            case HAL_DATASPACE_V0_BT601_525:
                return HAL_DATASPACE_BT601_525;
            case HAL_DATASPACE_V0_BT709:
                return HAL_DATASPACE_BT709;
            default:
                return dataSpace;
        }
    }

   return dataSpace;
}

int CameraDeviceSession::getStreamWithInflightBufferInGroup(
    const Camera3Stream* stream,
    uint32_t& frameNumber) {
    int streamId = -1;
    int groupId  = stream->group_id;

    for(auto it = mStreamMap.begin(); it != mStreamMap.end(); it++)
    {
        int streamGroupId = it->second.group_id;

        if (streamGroupId == groupId)
        {
            streamId = it->first;
            auto key = std::make_pair(streamId, frameNumber);
            if (1 == mInflightBuffers.count(key))
            {
                break;
            }
        }
    }
    return streamId;
}

::ndk::ScopedAStatus CameraDeviceSession::configureStreams(
        const StreamConfiguration& requestedConfiguration,
        std::vector<HalStream>* halStreamList)
{
    StreamConfiguration    streamConfig;
    std::vector<Stream>    streams;
    std::vector<int32_t>   groupIds;

    auto processStream = [&](const Stream& stream)
    {
        groupIds.push_back(stream.groupId);

        streams.push_back(stream);
    };

    for_each(requestedConfiguration.streams.begin(), requestedConfiguration.streams.end(), processStream);

    streamConfig.streams       = streams;
    streamConfig.operationMode = requestedConfiguration.operationMode;
    streamConfig.sessionParams = requestedConfiguration.sessionParams;

    return configureStreams_Impl(streamConfig, halStreamList,
            requestedConfiguration.streamConfigCounter, false /*useOverriddenFields*/,
            &groupIds, requestedConfiguration.multiResolutionInputImage);
}

::ndk::ScopedAStatus CameraDeviceSession::configureStreams_Impl(
        const StreamConfiguration& requestedConfiguration,
        std::vector<HalStream>* halStreamList,
        uint32_t streamConfigCounter, bool useOverriddenFields,
        std::vector<int32_t> *groupIds, bool multiResolutionInputImage)  {
    Status status = initStatus();
    if (nullptr == halStreamList)
    {
        return convertToScopedAStatus(Status::ILLEGAL_ARGUMENT);
    }
    halStreamList->clear();

    // hold the inflight lock for entire configureStreams scope since there must not be any
    // inflight request/results during stream configuration.
    Mutex::Autolock _l(mInflightLock);
    if (!mInflightBuffers.empty()) {
        ALOGE("%s: trying to configureStreams while there are still %zu inflight buffers!",
                __FUNCTION__, mInflightBuffers.size());
        return convertToScopedAStatus(Status::INTERNAL_ERROR);
    }

    if (!mInflightAETriggerOverrides.empty()) {
        ALOGE("%s: trying to configureStreams while there are still %zu inflight"
                " trigger overrides!", __FUNCTION__,
                mInflightAETriggerOverrides.size());
        return convertToScopedAStatus(Status::INTERNAL_ERROR);
    }

    if (!mInflightRawBoostPresent.empty()) {
        ALOGE("%s: trying to configureStreams while there are still %zu inflight"
                " boost overrides!", __FUNCTION__,
                mInflightRawBoostPresent.size());
        return convertToScopedAStatus(Status::INTERNAL_ERROR);
    }

    if (status != Status::OK) {
        return  convertToScopedAStatus(status);
    }

    const camera_metadata_t *paramBuffer = nullptr;
    if (0 < requestedConfiguration.sessionParams.metadata.size()) {
        if (!convertFromAidl(requestedConfiguration.sessionParams, &paramBuffer))
        {
            return convertToScopedAStatus(Status::ILLEGAL_ARGUMENT);
        }
    }

    camera3_stream_configuration_t stream_list{};
    // Block reading mStreamConfigCounter until configureStream returns
    Mutex::Autolock _sccl(mStreamConfigCounterLock);
    mStreamConfigCounter = streamConfigCounter;
    std::vector<camera3_stream_t*> streams;
    stream_list.session_parameters = paramBuffer;
    if (!preProcessConfigurationLocked(requestedConfiguration,
            useOverriddenFields, &stream_list, &streams)) {
        return convertToScopedAStatus(Status::INTERNAL_ERROR);
    }

    if (nullptr != groupIds)
    {
        if (groupIds->size() != streams.size())
        {
            ALOGE("%s: stream and groupIds size mismatch", __FUNCTION__);
            return convertToScopedAStatus(Status::INTERNAL_ERROR);
        }
        for (int streamIndex = 0; streamIndex < streams.size(); streamIndex++)
        {
            streams[streamIndex]->group_id = (*groupIds)[streamIndex];
        }
    }
    else
    {
        for (int streamIndex = 0; streamIndex < streams.size(); streamIndex++)
        {
            streams[streamIndex]->group_id = -1;
        }
    }

    stream_list.multi_resolution_input_image = multiResolutionInputImage;

    ATRACE_BEGIN("camera3->configure_streams");

    status_t ret = mDevice->ops->configure_streams(mDevice, &stream_list);
    ATRACE_END();

    // In case Hal returns error most likely it was not able to release
    // the corresponding resources of the deleted streams.
    if (ret == OK) {
        postProcessConfigurationLocked(requestedConfiguration);
    } else {
        postProcessConfigurationFailureLocked(requestedConfiguration);
    }

    if (ret == -EINVAL) {
        status = Status::ILLEGAL_ARGUMENT;
    } else if (ret != OK) {
        status = Status::INTERNAL_ERROR;
    } else {
        convertToAidl(stream_list, *halStreamList);
        mFirstRequest = true;
    }

    return convertToScopedAStatus(status);
}

bool CameraDeviceSession::preProcessConfigurationLocked(
        const StreamConfiguration& requestedConfiguration, bool useOverriddenFields,
        camera3_stream_configuration_t *stream_list /*out*/,
        std::vector<camera3_stream_t*> *streams /*out*/) {

    if ((stream_list == nullptr) || (streams == nullptr)) {
        return false;
    }

    stream_list->operation_mode = (uint32_t) requestedConfiguration.operationMode;
    stream_list->num_streams = requestedConfiguration.streams.size();
    streams->resize(stream_list->num_streams);
    stream_list->streams = streams->data();

    for (uint32_t i = 0; i < stream_list->num_streams; i++) {
        int id = requestedConfiguration.streams[i].id;

        if (mStreamMap.count(id) == 0) {
            Camera3Stream stream;
            convertFromAidl(requestedConfiguration.streams[i], &stream);

            mStreamMap[id] = stream;
            mPhysicalCameraIdMap[id] = requestedConfiguration.streams[i].physicalCameraId;
            mStreamMap[id].data_space = mapToLegacyDataspace(
                    mStreamMap[id].data_space);
            mCirculatingBuffers.emplace(stream.mId, CirculatingBuffers{});
        } else {
            // width/height/format must not change, but usage/rotation might need to change.
            // format and data_space may change.
            if (mStreamMap[id].stream_type !=
                    (int) requestedConfiguration.streams[i].streamType ||
                    mStreamMap[id].width != requestedConfiguration.streams[i].width ||
                    mStreamMap[id].height != requestedConfiguration.streams[i].height ||
                    mPhysicalCameraIdMap[id] != requestedConfiguration.streams[i].physicalCameraId) {
                ALOGE("%s: stream %d configuration changed!", __FUNCTION__, id);
                return false;
            }
            if (useOverriddenFields) {
                android_dataspace_t requestedDataSpace =
                        mapToLegacyDataspace(static_cast<android_dataspace_t>(
                        requestedConfiguration.streams[i].dataSpace));
                if (mStreamMap[id].format != (int) requestedConfiguration.streams[i].format ||
                        mStreamMap[id].data_space != requestedDataSpace) {
                    ALOGE("%s: stream %d configuration changed!", __FUNCTION__, id);
                    return false;
                }
            } else {
                mStreamMap[id].format =
                        (int) requestedConfiguration.streams[i].format;
                mStreamMap[id].data_space = (android_dataspace_t)
                        requestedConfiguration.streams[i].dataSpace;
            }
            mStreamMap[id].rotation = (int) requestedConfiguration.streams[i].rotation;
            mStreamMap[id].usage = (uint32_t) requestedConfiguration.streams[i].usage;
        }

        mStreamMap[id].physical_camera_id = mPhysicalCameraIdMap[id].c_str();

        (*streams)[i] = &mStreamMap[id];
    }

    if (mFreeBufEarly) {
        // Remove buffers of deleted streams
        for(auto it = mStreamMap.begin(); it != mStreamMap.end(); it++) {
            int id = it->first;

            bool found = false;
            for (const auto& stream : requestedConfiguration.streams) {
                if (id == stream.id) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                // Unmap all buffers of deleted stream
                cleanupBuffersLocked(id);
            }
        }
    }
    return true;
}

::ndk::ScopedAStatus CameraDeviceSession::switchToOffline(
    const std::vector<int32_t>& in_streamsToKeep,
    CameraOfflineSessionInfo* out_offlineSessionInfo,
    std::shared_ptr<ICameraOfflineSession>* cameraOfflineSession) {
    if (nullptr == cameraOfflineSession || nullptr == out_offlineSessionInfo) {
        return convertToScopedAStatus(Status::ILLEGAL_ARGUMENT);
    }
    *cameraOfflineSession = nullptr;

    // TBD. Support will be added later
    out_offlineSessionInfo->offlineStreams.resize(0);
    out_offlineSessionInfo->offlineRequests.resize(0);

    return ::ndk::ScopedAStatus::fromServiceSpecificError(
        static_cast<int32_t>(Status::OPERATION_NOT_SUPPORTED));
}

void CameraDeviceSession::postProcessConfigurationLocked(
        const StreamConfiguration& requestedConfiguration) {
    // delete unused streams, note we do this after adding new streams to ensure new stream
    // will not have the same address as deleted stream, and HAL has a chance to reference
    // the to be deleted stream in configure_streams call
    for(auto it = mStreamMap.begin(); it != mStreamMap.end();) {
        int id = it->first;
        bool found = false;
        for (const auto& stream : requestedConfiguration.streams) {
            if (id == stream.id) {
                found = true;
                break;
            }
        }
        if (!found) {
            // Unmap all buffers of deleted stream
            // in case the configuration call succeeds and HAL
            // is able to release the corresponding resources too.
            if (!mFreeBufEarly) {
                cleanupBuffersLocked(id);
            }
            it = mStreamMap.erase(it);
        } else {
            ++it;
        }
    }

    // Track video streams
    mVideoStreamIds.clear();
    for (const auto& stream : requestedConfiguration.streams) {
        if (stream.streamType == StreamType::OUTPUT &&
            static_cast<uint64_t>(stream.usage) & static_cast<uint64_t>(BufferUsage::VIDEO_ENCODER)) {
            mVideoStreamIds.push_back(stream.id);
        }
    }
    mResultBatcher.setBatchedStreams(mVideoStreamIds);
}

void CameraDeviceSession::postProcessConfigurationFailureLocked(
        const StreamConfiguration& requestedConfiguration) {
    if (mFreeBufEarly) {
        // Re-build the buf cache entry for deleted streams
        for(auto it = mStreamMap.begin(); it != mStreamMap.end(); it++) {
            int id = it->first;
            bool found = false;
            for (const auto& stream : requestedConfiguration.streams) {
                if (id == stream.id) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                mCirculatingBuffers.emplace(id, CirculatingBuffers{});
            }
        }
    }
}

::ndk::ScopedAStatus CameraDeviceSession::processCaptureRequest_Impl(
        const std::vector<CaptureRequest>& requests,
        const std::vector<BufferCache>& cachesToRemove,
        int32_t* numRequestProcessed,
        std::vector<uint32_t> *inputWidth, std::vector<uint32_t> *inputHeight)  {
    updateBufferCaches(cachesToRemove);

    *numRequestProcessed = 0;
    Status s = Status::OK;

    for (size_t i = 0; i < requests.size(); i++, (*numRequestProcessed)++) {

        if (nullptr != inputWidth && nullptr != inputHeight)
        {
            s = processOneCaptureRequest(requests[i], (*inputWidth)[i], (*inputHeight)[i]);
        }
        else
        {
            s = processOneCaptureRequest(requests[i]);
        }
        if (s != Status::OK) {
            break;
        }
    }

    if (s == Status::OK && requests.size() > 1) {
        mResultBatcher.registerBatch(requests[0].frameNumber, requests.size());
    }

    return convertToScopedAStatus(s);
}

::ndk::ScopedAStatus CameraDeviceSession::processCaptureRequest(
            const std::vector<CaptureRequest>& requests,
            const std::vector<BufferCache>& cachesToRemove,
            int32_t* numRequestProcessed)
{
    if (nullptr == numRequestProcessed)
    {
        return convertToScopedAStatus(Status::ILLEGAL_ARGUMENT);
    }

    Status ret = Status::INTERNAL_ERROR;

    std::vector<uint32_t> inputWidth;
    std::vector<uint32_t> inputHeight;

    auto processRequest = [&](const CaptureRequest& request)
    {
        inputWidth.push_back(static_cast<uint32_t>(request.inputWidth));
        inputHeight.push_back(static_cast<uint32_t>(request.inputHeight));
    };

    for_each(requests.begin(), requests.end(), processRequest);

    return processCaptureRequest_Impl(requests, cachesToRemove, numRequestProcessed, &inputWidth, &inputHeight);
}

Status CameraDeviceSession::processOneCaptureRequest(const CaptureRequest& request,
    uint32_t inputWidth, uint32_t inputHeight)  {
    Status status = initStatus();
    if (status != Status::OK) {
        ALOGE("%s: camera init failed or disconnected", __FUNCTION__);
        return status;
    }

    camera3_capture_request_t halRequest = {};
    halRequest.frame_number = request.frameNumber;

    bool converted = true;
    CameraMetadata settingsFmq;  // settings from FMQ
    if (request.fmqSettingsSize > 0) {
        // non-blocking read; client must write metadata before calling
        // processOneCaptureRequest
        settingsFmq.metadata.resize(request.fmqSettingsSize);
        bool read = mRequestMetadataQueue->read(reinterpret_cast<int8_t*>(settingsFmq.metadata.data()), request.fmqSettingsSize);
        if (read) {
            converted = convertFromAidl(settingsFmq, &halRequest.settings);
        } else {
            ALOGE("%s: capture request settings metadata couldn't be read from fmq!", __FUNCTION__);
            converted = false;
        }
    } else {
        converted = convertFromAidl(request.settings,
                &halRequest.settings);
    }

    if (!converted) {
        ALOGE("%s: capture request settings metadata is corrupt!", __FUNCTION__);
        return Status::ILLEGAL_ARGUMENT;
    }

    if (mFirstRequest && halRequest.settings == nullptr) {
        ALOGE("%s: capture request settings must not be null for first request!",
                __FUNCTION__);
        return Status::ILLEGAL_ARGUMENT;
    }

    std::vector<buffer_handle_t*> allBufPtrs;
    std::vector<int> allFences;
    bool hasInputBuf = (request.inputBuffer.streamId != -1 &&
            request.inputBuffer.bufferId != 0);
    size_t numOutputBufs = request.outputBuffers.size();
    size_t numBufs = numOutputBufs + (hasInputBuf ? 1 : 0);

    if (numOutputBufs == 0) {
        ALOGE("%s: capture request must have at least one output buffer!", __FUNCTION__);
        return Status::ILLEGAL_ARGUMENT;
    }

    for (const StreamBuffer& outputBuffer : request.outputBuffers)
    {
        if (outputBuffer.streamId == -1) {
            ALOGE("%s: invalid streamId in outputBuffer (%d)", __FUNCTION__, outputBuffer.streamId);
            return Status::ILLEGAL_ARGUMENT;
        }
    }

    status = importRequest(request, allBufPtrs, allFences);
    if (status != Status::OK) {
        return status;
    }

    std::vector<camera3_stream_buffer_t> outHalBufs;
    outHalBufs.resize(numOutputBufs);
    bool aeCancelTriggerNeeded = false;
    ::android::hardware::camera::common::V1_0::helper::CameraMetadata settingsOverride;
    {
        Mutex::Autolock _l(mInflightLock);
        if (hasInputBuf) {
            auto streamId = request.inputBuffer.streamId;
            auto key = std::make_pair(request.inputBuffer.streamId, request.frameNumber);
            auto& bufCache = mInflightBuffers[key] = camera3_stream_buffer_t{};
            convertFromAidl(
                    allBufPtrs[numOutputBufs], request.inputBuffer.status,
                    &mStreamMap[request.inputBuffer.streamId], allFences[numOutputBufs],
                    &bufCache);
            bufCache.stream->physical_camera_id = mPhysicalCameraIdMap[streamId].c_str();
            halRequest.input_buffer = &bufCache;
        } else {
            halRequest.input_buffer = nullptr;
        }

        halRequest.num_output_buffers = numOutputBufs;
        for (size_t i = 0; i < numOutputBufs; i++) {
            auto streamId = request.outputBuffers[i].streamId;
            auto key = std::make_pair(streamId, request.frameNumber);
            auto& bufCache = mInflightBuffers[key] = camera3_stream_buffer_t{};
            convertFromAidl(
                    allBufPtrs[i], request.outputBuffers[i].status,
                    &mStreamMap[streamId], allFences[i],
                    &bufCache);
            bufCache.stream->physical_camera_id = mPhysicalCameraIdMap[streamId].c_str();
            outHalBufs[i] = bufCache;
        }
        halRequest.output_buffers = outHalBufs.data();

        AETriggerCancelOverride triggerOverride;
        aeCancelTriggerNeeded = handleAePrecaptureCancelRequestLocked(
                halRequest, &settingsOverride /*out*/, &triggerOverride/*out*/);
        if (aeCancelTriggerNeeded) {
            mInflightAETriggerOverrides[halRequest.frame_number] =
                    triggerOverride;
            halRequest.settings = settingsOverride.getAndLock();
        }
    }

    std::vector<const char *> physicalCameraIds;
    std::vector<const camera_metadata_t *> physicalCameraSettings;
    std::vector<CameraMetadata> physicalFmq;
    size_t settingsCount = request.physicalCameraSettings.size();
    if (settingsCount > 0) {
        physicalCameraIds.reserve(settingsCount);
        physicalCameraSettings.reserve(settingsCount);
        physicalFmq.reserve(settingsCount);

        for (size_t i = 0; i < settingsCount; i++) {
            uint64_t settingsSize = request.physicalCameraSettings[i].fmqSettingsSize;
            const camera_metadata_t *settings = nullptr;
            if (settingsSize > 0) {
                CameraMetadata cmData = CameraMetadata();
                cmData.metadata.resize(settingsSize);
                physicalFmq.push_back(cmData);
                bool read = mRequestMetadataQueue->read(reinterpret_cast<int8_t*>(physicalFmq[i].metadata.data()), settingsSize);
                if (read) {
                    converted = convertFromAidl(physicalFmq[i], &settings);
                    physicalCameraSettings.push_back(settings);
                } else {
                    ALOGE("%s: physical camera settings metadata couldn't be read from fmq!",
                            __FUNCTION__);
                    converted = false;
                }
            } else {
                converted = convertFromAidl(
                        request.physicalCameraSettings[i].settings, &settings);
                physicalCameraSettings.push_back(settings);
            }

            if (!converted) {
                ALOGE("%s: physical camera settings metadata is corrupt!", __FUNCTION__);
                return Status::ILLEGAL_ARGUMENT;
            }

            if (mFirstRequest && settings == nullptr) {
                ALOGE("%s: Individual request settings must not be null for first request!",
                        __FUNCTION__);
                return Status::ILLEGAL_ARGUMENT;
            }

            physicalCameraIds.push_back(request.physicalCameraSettings[i].physicalCameraId.c_str());
        }
    }
    halRequest.num_physcam_settings = settingsCount;
    halRequest.physcam_id = physicalCameraIds.data();
    halRequest.physcam_settings = physicalCameraSettings.data();

    halRequest.inputWidth  = inputWidth;
    halRequest.inputHeight = inputHeight;

    ATRACE_ASYNC_BEGIN("frame capture", request.frameNumber);
    ATRACE_BEGIN("camera3->process_capture_request");
    status_t ret = mDevice->ops->process_capture_request(mDevice, &halRequest);
    ATRACE_END();
    if (aeCancelTriggerNeeded) {
        settingsOverride.unlock(halRequest.settings);
    }
    if (ret != OK) {
        Mutex::Autolock _l(mInflightLock);
        ALOGE("%s: HAL process_capture_request call failed!", __FUNCTION__);

        cleanupInflightFences(allFences, numBufs);
        if (hasInputBuf) {
            auto key = std::make_pair(request.inputBuffer.streamId, request.frameNumber);
            mInflightBuffers.erase(key);
        }
        for (size_t i = 0; i < numOutputBufs; i++) {
            auto key = std::make_pair(request.outputBuffers[i].streamId,
                    request.frameNumber);
            mInflightBuffers.erase(key);
        }
        if (aeCancelTriggerNeeded) {
            mInflightAETriggerOverrides.erase(request.frameNumber);
        }

        if (ret == BAD_VALUE) {
            return Status::ILLEGAL_ARGUMENT;
        } else {
            return Status::INTERNAL_ERROR;
        }
    }

    mFirstRequest = false;
    return Status::OK;
}

/**
 * Static callback forwarding methods from HAL to instance
 */
void CameraDeviceSession::sProcessCaptureResult(
        const camera3_callback_ops *cb,
        const camera3_capture_result *hal_result) {
    CameraDeviceSession *d =
            const_cast<CameraDeviceSession*>(static_cast<const CameraDeviceSession*>(cb));

    CaptureResult result = {};
    camera3_capture_result shadowResult;
    bool handlePhysCam = (d->mDeviceVersion >= CAMERA_DEVICE_API_VERSION_3_5);
    std::vector<::android::hardware::camera::common::V1_0::helper::CameraMetadata> compactMds;
    std::vector<const camera_metadata_t*> physCamMdArray;
    sShrinkCaptureResult(&shadowResult, hal_result, &compactMds, &physCamMdArray, handlePhysCam);

    status_t ret = d->constructCaptureResult(result, &shadowResult);
    if (ret != OK) {
        return;
    }

    if (handlePhysCam) {
        if (shadowResult.num_physcam_metadata > d->mPhysicalCameraIds.size()) {
            ALOGE("%s: Fatal: Invalid num_physcam_metadata %u", __FUNCTION__,
                    shadowResult.num_physcam_metadata);
            return;
        }
        result.physicalCameraMetadata.resize(shadowResult.num_physcam_metadata);
        for (uint32_t i = 0; i < shadowResult.num_physcam_metadata; i++) {
            std::string physicalId = shadowResult.physcam_ids[i];
            if (d->mPhysicalCameraIds.find(physicalId) == d->mPhysicalCameraIds.end()) {
                ALOGE("%s: Fatal: Invalid physcam_ids[%u]: %s", __FUNCTION__,
                      i, shadowResult.physcam_ids[i]);
                return;
            }
            CameraMetadata physicalMetadata;
            convertToAidl(
                    shadowResult.physcam_metadata[i], &physicalMetadata);
            result.physicalCameraMetadata[i].fmqMetadataSize  = 0;
            result.physicalCameraMetadata[i].physicalCameraId = physicalId;
            result.physicalCameraMetadata[i].metadata         = physicalMetadata;
        }
    }
    d->mResultBatcher.processCaptureResult(result);
}

// Needs to get called after acquiring 'mInflightLock'
void CameraDeviceSession::cleanupBuffersLocked(int id) {
    for (auto& pair : mCirculatingBuffers.at(id)) {
        sHandleImporter.freeBuffer(pair.second);
    }
    mCirculatingBuffers[id].clear();
    mCirculatingBuffers.erase(id);
}

void CameraDeviceSession::updateBufferCaches(const std::vector<BufferCache>& cachesToRemove) {
    Mutex::Autolock _l(mInflightLock);
    for (auto& cache : cachesToRemove) {
        auto cbsIt = mCirculatingBuffers.find(cache.streamId);
        if (cbsIt == mCirculatingBuffers.end()) {
            // The stream could have been removed
            continue;
        }
        CirculatingBuffers& cbs = cbsIt->second;
        auto it = cbs.find(cache.bufferId);
        if (it != cbs.end()) {
            sHandleImporter.freeBuffer(it->second);
            cbs.erase(it);
        } else {
            ALOGE("%s: stream %d buffer %" PRIu64 " is not cached",
                    __FUNCTION__, cache.streamId, cache.bufferId);
        }
    }
}

::ndk::ScopedAStatus CameraDeviceSession::getCaptureRequestMetadataQueue(
      ::aidl::android::hardware::common::fmq::MQDescriptor<
      int8_t, SynchronizedReadWrite>* requestMetadataQuesu) {
    if (nullptr == requestMetadataQuesu){
        return convertToScopedAStatus(Status::ILLEGAL_ARGUMENT);
    }
    *requestMetadataQuesu = mRequestMetadataQueue->dupeDesc();
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus CameraDeviceSession::getCaptureResultMetadataQueue(
      ::aidl::android::hardware::common::fmq::MQDescriptor<
      int8_t, SynchronizedReadWrite>* resultMetadataQuesu) {
    if (nullptr == resultMetadataQuesu){
        return convertToScopedAStatus(Status::ILLEGAL_ARGUMENT);
    }
    *resultMetadataQuesu = mResultMetadataQueue->dupeDesc();
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus CameraDeviceSession::flush()  {
    Status status = initStatus();
    if (status == Status::OK) {
        // Flush is always supported on device 3.1 or later
        status_t ret = mDevice->ops->flush(mDevice);
        if (ret != OK) {
            status = Status::INTERNAL_ERROR;
        }
    }
    return convertToScopedAStatus(status);
}

::ndk::ScopedAStatus CameraDeviceSession::close()  {
    Mutex::Autolock _l(mStateLock);
    if (!mClosed) {
        {
            Mutex::Autolock _l(mInflightLock);
            if (!mInflightBuffers.empty()) {
                ALOGE("%s: trying to close while there are still %zu inflight buffers!",
                        __FUNCTION__, mInflightBuffers.size());
            }
            if (!mInflightAETriggerOverrides.empty()) {
                ALOGE("%s: trying to close while there are still %zu inflight "
                        "trigger overrides!", __FUNCTION__,
                        mInflightAETriggerOverrides.size());
            }
            if (!mInflightRawBoostPresent.empty()) {
                ALOGE("%s: trying to close while there are still %zu inflight "
                        " RAW boost overrides!", __FUNCTION__,
                        mInflightRawBoostPresent.size());
            }

        }

        ATRACE_BEGIN("camera3->close");
        mDevice->common.close(&mDevice->common);
        ATRACE_END();

        // free all imported buffers
        Mutex::Autolock _l(mInflightLock);
        for(auto& pair : mCirculatingBuffers) {
            CirculatingBuffers& buffers = pair.second;
            for (auto& p2 : buffers) {
                sHandleImporter.freeBuffer(p2.second);
            }
            buffers.clear();
        }
        mCirculatingBuffers.clear();

        mClosed = true;
    }

    return ::ndk::ScopedAStatus::ok();
}


void CameraDeviceSession::pushBufferId(
        const buffer_handle_t& buf, uint64_t bufferId, int streamId) {
    std::lock_guard<std::mutex> lock(mBufferIdMapLock);

    // emplace will return existing entry if there is one.
    auto pair = mBufferIdMaps.emplace(streamId, BufferIdMap{});
    BufferIdMap& bIdMap = pair.first->second;
    bIdMap[buf] = bufferId;
}

uint64_t CameraDeviceSession::popBufferId(
        const buffer_handle_t& buf, int streamId) {
    std::lock_guard<std::mutex> lock(mBufferIdMapLock);

    auto streamIt = mBufferIdMaps.find(streamId);
    if (streamIt == mBufferIdMaps.end()) {
        return BUFFER_ID_NO_BUFFER;
    }
    BufferIdMap& bIdMap = streamIt->second;
    auto it = bIdMap.find(buf);
    if (it == bIdMap.end()) {
        return BUFFER_ID_NO_BUFFER;
    }
    uint64_t bufId = it->second;
    bIdMap.erase(it);
    if (bIdMap.empty()) {
        mBufferIdMaps.erase(streamIt);
    }
    return bufId;
}

uint64_t CameraDeviceSession::getCapResultBufferId(const buffer_handle_t& buf, int streamId) {
    if (mSupportBufMgr) {
        return popBufferId(buf, streamId);
    }
    return BUFFER_ID_NO_BUFFER;
}


Camera3Stream* CameraDeviceSession::getStreamPointer(int32_t streamId) {
    Mutex::Autolock _l(mInflightLock);
    if (mStreamMap.count(streamId) == 0) {
        ALOGE("%s: unknown streamId %d", __FUNCTION__, streamId);
        return nullptr;
    }
    return &mStreamMap[streamId];
}

void CameraDeviceSession::cleanupInflightBufferFences(
        std::vector<int>& fences, std::vector<std::pair<buffer_handle_t, int>>& bufs) {
    std::vector<int> hFences = fences;
    cleanupInflightFences(hFences, fences.size());
    for (auto& p : bufs) {
        popBufferId(p.first, p.second);
    }
}

camera3_buffer_request_status_t CameraDeviceSession::requestStreamBuffers(
        uint32_t num_buffer_reqs,
        const camera3_buffer_request_t *buffer_reqs,
        /*out*/uint32_t *num_returned_buf_reqs,
        /*out*/camera3_stream_buffer_ret_t *returned_buf_reqs) {
    ATRACE_CALL();
    *num_returned_buf_reqs = 0;
    std::vector<BufferRequest> hBufReqs(num_buffer_reqs);
    for (size_t i = 0; i < num_buffer_reqs; i++) {
        hBufReqs[i].streamId =
                static_cast<Camera3Stream*>(buffer_reqs[i].stream)->mId;
        hBufReqs[i].numBuffersRequested = buffer_reqs[i].num_buffers_requested;
    }

    ATRACE_BEGIN("AIDL requestStreamBuffers");
    BufferRequestStatus status;
    std::vector<StreamBufferRet> bufRets;
    auto err = aidl_device_callback->requestStreamBuffers(hBufReqs, &bufRets, &status);
    if (!err.isOk()) {
        ALOGE("%s: Transaction error: %s", __FUNCTION__, err.getDescription().c_str());
        return CAMERA3_BUF_REQ_FAILED_UNKNOWN;
    }
    ATRACE_END();

    switch (status) {
        case BufferRequestStatus::FAILED_CONFIGURING:
            return CAMERA3_BUF_REQ_FAILED_CONFIGURING;
        case BufferRequestStatus::FAILED_ILLEGAL_ARGUMENTS:
            return CAMERA3_BUF_REQ_FAILED_ILLEGAL_ARGUMENTS;
        default:
            break; // Other status Handled by following code
    }

    if (status != BufferRequestStatus::OK && status != BufferRequestStatus::FAILED_PARTIAL &&
            status != BufferRequestStatus::FAILED_UNKNOWN) {
        ALOGE("%s: unknown buffer request error code %d", __FUNCTION__, status);
        return CAMERA3_BUF_REQ_FAILED_UNKNOWN;
    }

    // Only OK, FAILED_PARTIAL and FAILED_UNKNOWN reaches here
    if (bufRets.size() != num_buffer_reqs) {
        ALOGE("%s: expect %d buffer requests returned, only got %zu",
                __FUNCTION__, num_buffer_reqs, bufRets.size());
        return CAMERA3_BUF_REQ_FAILED_UNKNOWN;
    }

    *num_returned_buf_reqs = num_buffer_reqs;
    for (size_t i = 0; i < num_buffer_reqs; i++) {
        // maybe we can query all streams in one call to avoid frequent locking device here?
        Camera3Stream* stream = getStreamPointer(bufRets[i].streamId);
        if (stream == nullptr) {
            ALOGE("%s: unknown streamId %d", __FUNCTION__, bufRets[i].streamId);
            return CAMERA3_BUF_REQ_FAILED_UNKNOWN;
        }
        returned_buf_reqs[i].stream = stream;
    }

    // Handle failed streams
    for (size_t i = 0; i < num_buffer_reqs; i++) {
        if (bufRets[i].val.getTag() == StreamBuffersVal::Tag::error) {
            returned_buf_reqs[i].num_output_buffers = 0;
            switch (bufRets[i].val.get<StreamBuffersVal::Tag::error>()) {
                case StreamBufferRequestError::NO_BUFFER_AVAILABLE:
                    returned_buf_reqs[i].status = CAMERA3_PS_BUF_REQ_NO_BUFFER_AVAILABLE;
                    break;
                case StreamBufferRequestError::MAX_BUFFER_EXCEEDED:
                    returned_buf_reqs[i].status = CAMERA3_PS_BUF_REQ_MAX_BUFFER_EXCEEDED;
                    break;
                case StreamBufferRequestError::STREAM_DISCONNECTED:
                    returned_buf_reqs[i].status = CAMERA3_PS_BUF_REQ_STREAM_DISCONNECTED;
                    break;
                case StreamBufferRequestError::UNKNOWN_ERROR:
                    returned_buf_reqs[i].status = CAMERA3_PS_BUF_REQ_UNKNOWN_ERROR;
                    break;
                default:
                    ALOGE("%s: Unknown StreamBufferRequestError %d",
                            __FUNCTION__, bufRets[i].val.get<StreamBuffersVal::Tag::error>());
                    return CAMERA3_BUF_REQ_FAILED_UNKNOWN;
            }
        }
    }

    if (status == BufferRequestStatus::FAILED_UNKNOWN) {
        return CAMERA3_BUF_REQ_FAILED_UNKNOWN;
    }

    // Only BufferRequestStatus::OK and BufferRequestStatus::FAILED_PARTIAL reaches here
    std::vector<int> importedFences;
    std::vector<std::pair<buffer_handle_t, int>> importedBuffers;
    for (size_t i = 0; i < num_buffer_reqs; i++) {
        if (bufRets[i].val.getTag() !=
                StreamBuffersVal::Tag::buffers) {
            continue;
        }
        int streamId = bufRets[i].streamId;
        const std::vector<StreamBuffer>& hBufs =
            bufRets[i].val.get<StreamBuffersVal::Tag::buffers>();
        camera3_stream_buffer_t* outBufs = returned_buf_reqs[i].output_buffers;
        returned_buf_reqs[i].num_output_buffers = hBufs.size();
        for (size_t b = 0; b < hBufs.size(); b++) {
            const StreamBuffer& hBuf = hBufs[b];
            camera3_stream_buffer_t& outBuf = outBufs[b];
            // maybe add importBuffers API to avoid frequent locking device?
            native_handle_t* handleBuf = convertFromAidl(hBuf.buffer);
            Status s = importBuffer(streamId,
                    hBuf.bufferId, handleBuf,
                    /*out*/&(outBuf.buffer),
                    /*allowEmptyBuf*/false);
            // Buffer import should never fail - restart HAL since something is very wrong.
            native_handle_delete(handleBuf);
            LOG_ALWAYS_FATAL_IF(s != Status::OK,
                    "%s: import stream %d bufferId %" PRIu64 " failed!",
                    __FUNCTION__, streamId, hBuf.bufferId);

            pushBufferId(*(outBuf.buffer), hBuf.bufferId, streamId);
            importedBuffers.push_back(std::make_pair(*(outBuf.buffer), streamId));

            native_handle_t* handleAcqFence = convertFromAidl(hBuf.acquireFence);
            bool succ = sHandleImporter.importFence(handleAcqFence ,
                outBuf.acquire_fence);
            // Fence import should never fail - restart HAL since something is very wrong.
            native_handle_delete(handleAcqFence);
            LOG_ALWAYS_FATAL_IF(!succ,
                        "%s: stream %d bufferId %" PRIu64 "acquire fence is invalid",
                        __FUNCTION__, streamId, hBuf.bufferId);
            importedFences.push_back(outBuf.acquire_fence);
            outBuf.stream = returned_buf_reqs[i].stream;
            outBuf.status = CAMERA3_BUFFER_STATUS_OK;
            outBuf.release_fence = -1;
        }
        returned_buf_reqs[i].status = CAMERA3_PS_BUF_REQ_OK;
    }

    return (status == BufferRequestStatus::OK) ?
            CAMERA3_BUF_REQ_OK : CAMERA3_BUF_REQ_FAILED_PARTIAL;
}

void CameraDeviceSession::returnStreamBuffers(
        uint32_t num_buffers,
        const camera3_stream_buffer_t* const* buffers) {
    ATRACE_CALL();
    std::vector<StreamBuffer> hBufs(num_buffers);

    for (size_t i = 0; i < num_buffers; i++) {
        hBufs[i].streamId =
                static_cast<Camera3Stream*>(buffers[i]->stream)->mId;
        hBufs[i].buffer = aidl::android::hardware::common::NativeHandle();
        hBufs[i].bufferId = popBufferId(*(buffers[i]->buffer), hBufs[i].streamId);
        if (hBufs[i].bufferId == BUFFER_ID_NO_BUFFER) {
            ALOGE("%s: unknown buffer is returned to stream %d",
                    __FUNCTION__, hBufs[i].streamId);
        }
        // ERROR since the buffer is not for application to consume
        hBufs[i].status = BufferStatus::ERROR;
        // skip acquire fence since it's of no use to camera service

        if (buffers[i]->release_fence != -1) {
            native_handle_t* handle = native_handle_create(/*numFds*/1, /*numInts*/0);

            if (nullptr != handle) {
                handle->data[0] = buffers[i]->release_fence;
                hBufs[i].releaseFence = convertToAidl(handle);
            }
            native_handle_delete(handle);
        }
    }

    aidl_device_callback->returnStreamBuffers(hBufs);
    return;
}

/**
 * Static callback forwarding methods from HAL to instance
 */
camera3_buffer_request_status_t CameraDeviceSession::sRequestStreamBuffers(
        const struct camera3_callback_ops *cb,
        uint32_t num_buffer_reqs,
        const camera3_buffer_request_t *buffer_reqs,
        /*out*/uint32_t *num_returned_buf_reqs,
        /*out*/camera3_stream_buffer_ret_t *returned_buf_reqs) {
    CameraDeviceSession *d =
            const_cast<CameraDeviceSession*>(static_cast<const CameraDeviceSession*>(cb));

    if (num_buffer_reqs == 0 || buffer_reqs == nullptr || num_returned_buf_reqs == nullptr ||
            returned_buf_reqs == nullptr) {
        ALOGE("%s: bad argument: numBufReq %d, bufReqs %pK, numRetBufReq %pK, retBufReqs %pK",
                __FUNCTION__, num_buffer_reqs, buffer_reqs,
                num_returned_buf_reqs, returned_buf_reqs);
        return CAMERA3_BUF_REQ_FAILED_ILLEGAL_ARGUMENTS;
    }

    return d->requestStreamBuffers(num_buffer_reqs, buffer_reqs,
            num_returned_buf_reqs, returned_buf_reqs);
}

void CameraDeviceSession::sReturnStreamBuffers(
        const struct camera3_callback_ops *cb,
        uint32_t num_buffers,
        const camera3_stream_buffer_t* const* buffers) {
    CameraDeviceSession *d =
            const_cast<CameraDeviceSession*>(static_cast<const CameraDeviceSession*>(cb));

    d->returnStreamBuffers(num_buffers, buffers);
}

::ndk::ScopedAStatus CameraDeviceSession::isReconfigurationRequired(
        const CameraMetadata& oldSessionParams, const CameraMetadata& newSessionParams,
        bool* isReconfigurationRequired) {
    Status status = Status::OK;
    if (mDevice->ops->is_reconfiguration_required != nullptr) {
        const camera_metadata_t *oldParams, *newParams;

        bool convertOldSuccess = convertFromAidl(oldSessionParams, &oldParams);
        bool convertNewSuccess = convertFromAidl(newSessionParams, &newParams);

        if ((true == convertOldSuccess) && (true == convertNewSuccess) && (nullptr != oldParams) && (nullptr != newParams))
        {
            auto ret = mDevice->ops->is_reconfiguration_required(mDevice, oldParams, newParams);
            switch (ret) {
                case 0:
                    *isReconfigurationRequired = true;
                    status = Status::OK;
                    break;
                case -EINVAL:
                    *isReconfigurationRequired = false;
                    status = Status::OK;
                    break;
                case -ENOSYS:
                    *isReconfigurationRequired = true;
                    status = Status::OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    *isReconfigurationRequired = true;
                    status = Status::INTERNAL_ERROR;
                    break;
            };
        }
        else
        {
            *isReconfigurationRequired = true;
            status = Status::ILLEGAL_ARGUMENT;
        }

    } else {
        *isReconfigurationRequired = true;
        status = Status::OPERATION_NOT_SUPPORTED;
    }

    return convertToScopedAStatus(status);
}

status_t CameraDeviceSession::constructCaptureResult(CaptureResult& result,
                                                 const camera3_capture_result *hal_result) {
    uint32_t frameNumber = hal_result->frame_number;
    bool hasInputBuf = (hal_result->input_buffer != nullptr);
    size_t numOutputBufs = hal_result->num_output_buffers;
    size_t numBufs = numOutputBufs + (hasInputBuf ? 1 : 0);
    if (numBufs > 0) {
        Mutex::Autolock _l(mInflightLock);
        if (hasInputBuf) {
            int streamId = static_cast<Camera3Stream*>(hal_result->input_buffer->stream)->mId;
            // validate if buffer is inflight
            auto key = std::make_pair(streamId, frameNumber);
            if (mInflightBuffers.count(key) != 1) {
                ALOGE("%s: input buffer for stream %d frame %d is not inflight!",
                        __FUNCTION__, streamId, frameNumber);
                return -EINVAL;
            }
        }

        for (size_t i = 0; i < numOutputBufs; i++) {
            int streamId = static_cast<Camera3Stream*>(hal_result->output_buffers[i].stream)->mId;
            // validate if buffer is inflight
            auto key = std::make_pair(streamId, frameNumber);
            if ((1 != mInflightBuffers.count(key)) &&
                (-1 == getStreamWithInflightBufferInGroup(reinterpret_cast<Camera3Stream*>(
                    hal_result->output_buffers[i].stream),frameNumber)))
            {
                ALOGE("%s: output buffer for stream %d frame %d is not inflight!",
                        __FUNCTION__, streamId, frameNumber);
                return -EINVAL;
            }
        }
    }
    // We don't need to validate/import fences here since we will be passing them to camera service
    // within the scope of this function
    result.frameNumber = frameNumber;
    result.fmqResultSize = 0;
    result.partialResult = hal_result->partial_result;
    convertToAidl(hal_result->result, &result.result);
    if (nullptr != hal_result->result) {
        bool resultOverriden = false;
        Mutex::Autolock _l(mInflightLock);

        // Derive some new keys for backward compatibility
        if (mDerivePostRawSensKey) {
            camera_metadata_ro_entry entry;
            if (find_camera_metadata_ro_entry(hal_result->result,
                    ANDROID_CONTROL_POST_RAW_SENSITIVITY_BOOST, &entry) == 0) {
                mInflightRawBoostPresent[frameNumber] = true;
            } else {
                auto entry = mInflightRawBoostPresent.find(frameNumber);
                if (mInflightRawBoostPresent.end() == entry) {
                    mInflightRawBoostPresent[frameNumber] = false;
                }
            }

            if ((hal_result->partial_result == mNumPartialResults)) {
                if (!mInflightRawBoostPresent[frameNumber]) {
                    if (!resultOverriden) {
                        mOverridenResult.clear();
                        mOverridenResult.append(hal_result->result);
                        resultOverriden = true;
                    }
                    int32_t defaultBoost[1] = {100};
                    mOverridenResult.update(
                            ANDROID_CONTROL_POST_RAW_SENSITIVITY_BOOST,
                            defaultBoost, 1);
                }

                mInflightRawBoostPresent.erase(frameNumber);
            }
        }

        auto entry = mInflightAETriggerOverrides.find(frameNumber);
        if (mInflightAETriggerOverrides.end() != entry) {
            if (!resultOverriden) {
                mOverridenResult.clear();
                mOverridenResult.append(hal_result->result);
                resultOverriden = true;
            }
            if (hal_result->partial_result == mNumPartialResults) {
                mInflightAETriggerOverrides.erase(frameNumber);
            }
        }

        if (resultOverriden) {
            const camera_metadata_t *metaBuffer =
                    mOverridenResult.getAndLock();
            convertToAidl(metaBuffer, &result.result);
            mOverridenResult.unlock(metaBuffer);
        }
    }
    if (hasInputBuf) {
        result.inputBuffer.streamId =
                static_cast<Camera3Stream*>(hal_result->input_buffer->stream)->mId;
        result.inputBuffer.buffer = aidl::android::hardware::common::NativeHandle();
        result.inputBuffer.status = (BufferStatus) hal_result->input_buffer->status;
        // skip acquire fence since it's no use to camera service
        if (hal_result->input_buffer->release_fence != -1) {
            native_handle_t* handle = native_handle_create(/*numFds*/1, /*numInts*/0);
            if (nullptr != handle) {
                handle->data[0] = hal_result->input_buffer->release_fence;
            }
            result.inputBuffer.releaseFence = convertToAidl(handle);
            native_handle_delete(handle);
        } else {
            result.inputBuffer.releaseFence = aidl::android::hardware::common::NativeHandle();
        }
    } else {
        result.inputBuffer.streamId = -1;
    }

    result.outputBuffers.resize(numOutputBufs);
    for (size_t i = 0; i < numOutputBufs; i++) {
        result.outputBuffers[i].streamId =
                static_cast<Camera3Stream*>(hal_result->output_buffers[i].stream)->mId;
        result.outputBuffers[i].buffer = aidl::android::hardware::common::NativeHandle();
        if (hal_result->output_buffers[i].buffer != nullptr) {
            result.outputBuffers[i].bufferId = getCapResultBufferId(
                    *(hal_result->output_buffers[i].buffer),
                    result.outputBuffers[i].streamId);
        } else {
            result.outputBuffers[i].bufferId = 0;
        }

        result.outputBuffers[i].status = (BufferStatus) hal_result->output_buffers[i].status;
        // skip acquire fence since it's of no use to camera service
        if (hal_result->output_buffers[i].release_fence != -1) {
            native_handle_t* handle = native_handle_create(/*numFds*/1, /*numInts*/0);
            if (nullptr != handle) {
                int dup_fd = dup(hal_result->output_buffers[i].release_fence);
                if (dup_fd < 0)
                {
                    ALOGE("failed to dup fence fd %d", hal_result->output_buffers[i].release_fence);
                    native_handle_delete(handle);
                    return false;
                }
                handle->data[0] = dup_fd;
            }
            result.outputBuffers[i].releaseFence = convertToAidl(handle);
            native_handle_delete(handle);
        } else {
            result.outputBuffers[i].releaseFence = aidl::android::hardware::common::NativeHandle();
        }
    }

    // Free inflight record/fences.
    // Do this before call back to camera service because camera service might jump to
    // configure_streams right after the processCaptureResult call so we need to finish
    // updating inflight queues first
    if (numBufs > 0) {
        Mutex::Autolock _l(mInflightLock);
        if (hasInputBuf) {
            int streamId = static_cast<Camera3Stream*>(hal_result->input_buffer->stream)->mId;
            auto key = std::make_pair(streamId, frameNumber);
            mInflightBuffers.erase(key);
        }

        for (size_t i = 0; i < numOutputBufs; i++) {
            int streamId = static_cast<Camera3Stream*>(hal_result->output_buffers[i].stream)->mId;
            auto key = std::make_pair(streamId, frameNumber);
            if (mInflightBuffers.count(key) == 1)
            {
                mInflightBuffers.erase(key);
            }
            else
            {
                streamId = getStreamWithInflightBufferInGroup(reinterpret_cast<Camera3Stream*>(hal_result->output_buffers[i].stream),
            frameNumber);
                if (streamId != -1)
                {
                    key = std::make_pair(streamId, frameNumber);
                    mInflightBuffers.erase(key);
                }
            }
        }

        if (mInflightBuffers.empty()) {
            ALOGV("%s: inflight buffer queue is now empty!", __FUNCTION__);
        }
    }
    return OK;
}

// Static helper method to copy/shrink capture result metadata sent by HAL
void CameraDeviceSession::sShrinkCaptureResult(
        camera3_capture_result* dst, const camera3_capture_result* src,
        std::vector<::android::hardware::camera::common::V1_0::helper::CameraMetadata>* mds,
        std::vector<const camera_metadata_t*>* physCamMdArray,
        bool handlePhysCam) {
    *dst = *src;
    // Reserve maximum number of entries to avoid metadata re-allocation.
    mds->reserve(1 + (handlePhysCam ? src->num_physcam_metadata : 0));
    if (sShouldShrink(src->result)) {
        mds->emplace_back(sCreateCompactCopy(src->result));
        dst->result = mds->back().getAndLock();
    }

    if (handlePhysCam) {
        // First determine if we need to create new camera_metadata_t* array
        bool needShrink = false;
        for (uint32_t i = 0; i < src->num_physcam_metadata; i++) {
            if (sShouldShrink(src->physcam_metadata[i])) {
                needShrink = true;
            }
        }

        if (!needShrink) return;

        physCamMdArray->reserve(src->num_physcam_metadata);
        dst->physcam_metadata = physCamMdArray->data();
        for (uint32_t i = 0; i < src->num_physcam_metadata; i++) {
            if (sShouldShrink(src->physcam_metadata[i])) {
                mds->emplace_back(sCreateCompactCopy(src->physcam_metadata[i]));
                dst->physcam_metadata[i] = mds->back().getAndLock();
            } else {
                dst->physcam_metadata[i] = src->physcam_metadata[i];
            }
        }
    }
}

bool CameraDeviceSession::sShouldShrink(const camera_metadata_t* md) {
    size_t compactSize = get_camera_metadata_compact_size(md);
    size_t totalSize = get_camera_metadata_size(md);
    if (totalSize >= compactSize + METADATA_SHRINK_ABS_THRESHOLD &&
            totalSize >= compactSize * METADATA_SHRINK_REL_THRESHOLD) {
        ALOGV("Camera metadata should be shrunk from %zu to %zu", totalSize, compactSize);
        return true;
    }
    return false;
}

camera_metadata_t* CameraDeviceSession::sCreateCompactCopy(const camera_metadata_t* src) {
    size_t compactSize = get_camera_metadata_compact_size(src);
    void* buffer = calloc(1, compactSize);
    if (buffer == nullptr) {
        ALOGE("%s: Allocating %zu bytes failed", __FUNCTION__, compactSize);
    }
    return copy_camera_metadata(buffer, compactSize, src);
}

void CameraDeviceSession::sNotify(
        const camera3_callback_ops *cb,
        const camera3_notify_msg *msg) {
    CameraDeviceSession *d =
            const_cast<CameraDeviceSession*>(static_cast<const CameraDeviceSession*>(cb));
    NotifyMsg aidlMsg;
    convertToAidl(msg, &aidlMsg);

    if (aidlMsg.getTag() == NotifyMsg::Tag::error &&
            aidlMsg.get<NotifyMsg::Tag::error>().errorStreamId != -1) {
        if (d->mStreamMap.count(aidlMsg.get<NotifyMsg::Tag::error>().errorStreamId) != 1) {
            ALOGE("%s: unknown stream ID %d reports an error!",
                    __FUNCTION__, aidlMsg.get<NotifyMsg::Tag::error>().errorStreamId);
            return;
        }
    }

    if (aidlMsg.getTag() == NotifyMsg::Tag::error) {
        switch (aidlMsg.get<NotifyMsg::Tag::error>().errorCode) {
            case ErrorCode::ERROR_DEVICE:
            case ErrorCode::ERROR_REQUEST:
            case ErrorCode::ERROR_RESULT: {
                Mutex::Autolock _l(d->mInflightLock);
                auto entry = d->mInflightAETriggerOverrides.find(
                        aidlMsg.get<NotifyMsg::Tag::error>().frameNumber);
                if (d->mInflightAETriggerOverrides.end() != entry) {
                    d->mInflightAETriggerOverrides.erase(
                            aidlMsg.get<NotifyMsg::Tag::error>().frameNumber);
                }

                auto boostEntry = d->mInflightRawBoostPresent.find(
                        aidlMsg.get<NotifyMsg::Tag::error>().frameNumber);
                if (d->mInflightRawBoostPresent.end() != boostEntry) {
                    d->mInflightRawBoostPresent.erase(
                            aidlMsg.get<NotifyMsg::Tag::error>().frameNumber);
                }

            }
                break;
            case ErrorCode::ERROR_BUFFER:
            default:
                break;
        }

    }

    d->mResultBatcher.notify(aidlMsg);
}

CameraDeviceSession::ResultBatcher::ResultBatcher(
        const std::shared_ptr<ICameraDeviceCallback>& callback) : mCallback(callback) {};

void CameraDeviceSession::ResultBatcher::setNumPartialResults(uint32_t n) {
    Mutex::Autolock _l(mLock);
    mNumPartialResults = n;
}

void CameraDeviceSession::ResultBatcher::setBatchedStreams(
        const std::vector<int>& streamsToBatch) {
    Mutex::Autolock _l(mLock);
    mStreamsToBatch = streamsToBatch;
}

void CameraDeviceSession::ResultBatcher::setResultMetadataQueue(
        std::shared_ptr<ResultMetadataQueue> q) {
    Mutex::Autolock _l(mLock);
    mResultMetadataQueue = q;
}

void CameraDeviceSession::ResultBatcher::registerBatch(uint32_t frameNumber, uint32_t batchSize) {
    auto batch = std::make_shared<InflightBatch>();
    batch->mFirstFrame = frameNumber;
    batch->mBatchSize = batchSize;
    batch->mLastFrame = batch->mFirstFrame + batch->mBatchSize - 1;
    batch->mNumPartialResults = mNumPartialResults;
    for (int id : mStreamsToBatch) {
        batch->mBatchBufs.emplace(id, batch->mBatchSize);
    }
    Mutex::Autolock _l(mLock);
    mInflightBatches.push_back(batch);
}

std::pair<int, std::shared_ptr<CameraDeviceSession::ResultBatcher::InflightBatch>>
CameraDeviceSession::ResultBatcher::getBatch(
        uint32_t frameNumber) {
    Mutex::Autolock _l(mLock);
    int numBatches = mInflightBatches.size();
    if (numBatches == 0) {
        return std::make_pair(NOT_BATCHED, nullptr);
    }
    uint32_t frameMin = mInflightBatches[0]->mFirstFrame;
    uint32_t frameMax = mInflightBatches[numBatches - 1]->mLastFrame;
    if (frameNumber < frameMin || frameNumber > frameMax) {
        return std::make_pair(NOT_BATCHED, nullptr);
    }
    for (int i = 0; i < numBatches; i++) {
        if (frameNumber >= mInflightBatches[i]->mFirstFrame &&
                frameNumber <= mInflightBatches[i]->mLastFrame) {
            return std::make_pair(i, mInflightBatches[i]);
        }
    }
    return std::make_pair(NOT_BATCHED, nullptr);
}

void CameraDeviceSession::ResultBatcher::checkAndRemoveFirstBatch() {
    Mutex::Autolock _l(mLock);
    if (mInflightBatches.size() > 0) {
        std::shared_ptr<InflightBatch> batch = mInflightBatches[0];
        bool shouldRemove = false;
        if (batch != nullptr)
        {
            Mutex::Autolock _l(batch->mLock);
            if (batch->allDelivered()) {
                batch->mRemoved = true;
                shouldRemove = true;
            }
        }
        if (shouldRemove) {
            mInflightBatches.pop_front();
        }
    }
}

void CameraDeviceSession::ResultBatcher::sendBatchShutterCbsLocked(
        std::shared_ptr<InflightBatch> batch) {
    if (batch->mShutterDelivered) {
        ALOGW("%s: batch shutter callback already sent!", __FUNCTION__);
        return;
    }

    auto ret = mCallback->notify(batch->mShutterMsgs);
    if (!ret.isOk()) {
        ALOGE("%s: notify shutter transaction failed: %s",
                __FUNCTION__, ret.getDescription().c_str());
    }
    batch->mShutterDelivered = true;
    batch->mShutterMsgs.clear();
}

void CameraDeviceSession::ResultBatcher::freeReleaseFences(std::vector<CaptureResult>& results) {
    for (auto& result : results) {
        native_handle_t* handle = convertFromAidl(result.inputBuffer.releaseFence);
        if (handle != nullptr) {
            native_handle_close(handle);
            native_handle_delete(handle);
        }
        for (auto& buf : result.outputBuffers) {
            native_handle_t* handle = convertFromAidl(buf.releaseFence);
            if (handle != nullptr) {
                native_handle_close(handle);
                native_handle_delete(handle);
            }
        }
    }
}

void CameraDeviceSession::ResultBatcher::moveStreamBuffer(StreamBuffer&& src, StreamBuffer& dst) {
    // Only dealing with releaseFence here. Assume buffer/acquireFence are null
    dst = std::move(src);
    src.releaseFence = aidl::android::hardware::common::NativeHandle();
}

void CameraDeviceSession::ResultBatcher::pushStreamBuffer(
        StreamBuffer&& src, std::vector<StreamBuffer>& dst) {
    // Only dealing with releaseFence here. Assume buffer/acquireFence are null
    dst.push_back(StreamBuffer());
    dst.back() = std::move(src);
    src.releaseFence = aidl::android::hardware::common::NativeHandle();
}

void CameraDeviceSession::ResultBatcher::sendBatchBuffersLocked(
        std::shared_ptr<InflightBatch> batch) {
    sendBatchBuffersLocked(batch, mStreamsToBatch);
}

void CameraDeviceSession::ResultBatcher::sendBatchBuffersLocked(
        std::shared_ptr<InflightBatch> batch, const std::vector<int>& streams) {
    size_t batchSize = 0;
    for (int streamId : streams) {
        auto it = batch->mBatchBufs.find(streamId);
        if (it != batch->mBatchBufs.end()) {
            InflightBatch::BufferBatch& bb = it->second;
            if (bb.mDelivered) {
                continue;
            }
            if (bb.mBuffers.size() > batchSize) {
                batchSize = bb.mBuffers.size();
            }
        } else {
            ALOGE("%s: stream ID %d is not batched!", __FUNCTION__, streamId);
            return;
        }
    }

    if (batchSize == 0) {
        ALOGW("%s: there is no buffer to be delivered for this batch.", __FUNCTION__);
        for (int streamId : streams) {
            auto it = batch->mBatchBufs.find(streamId);
            if (it == batch->mBatchBufs.end()) {
                ALOGE("%s: cannot find stream %d in batched buffers!", __FUNCTION__, streamId);
                return;
            }
            InflightBatch::BufferBatch& bb = it->second;
            bb.mDelivered = true;
        }
        return;
    }

    std::vector<CaptureResult> results;
    results.resize(batchSize);
    for (size_t i = 0; i < batchSize; i++) {
        results[i].frameNumber = batch->mFirstFrame + i;
        results[i].fmqResultSize = 0;
        results[i].partialResult = 0; // 0 for buffer only results
        results[i].inputBuffer.streamId = -1;
        results[i].inputBuffer.bufferId = 0;
        results[i].inputBuffer.buffer = aidl::android::hardware::common::NativeHandle();
        std::vector<StreamBuffer> outBufs;
        outBufs.reserve(streams.size());
        for (int streamId : streams) {
            auto it = batch->mBatchBufs.find(streamId);
            if (it == batch->mBatchBufs.end()) {
                ALOGE("%s: cannot find stream %d in batched buffers!", __FUNCTION__, streamId);
                return;
            }
            InflightBatch::BufferBatch& bb = it->second;
            if (bb.mDelivered) {
                continue;
            }
            if (i < bb.mBuffers.size()) {
                pushStreamBuffer(std::move(bb.mBuffers[i]), outBufs);
            }
        }
        results[i].outputBuffers.resize(outBufs.size());
        for (size_t j = 0; j < outBufs.size(); j++) {
            moveStreamBuffer(std::move(outBufs[j]), results[i].outputBuffers[j]);
        }
    }
    invokeProcessCaptureResultCallback(results, /* tryWriteFmq */false);
    freeReleaseFences(results);
    for (int streamId : streams) {
        auto it = batch->mBatchBufs.find(streamId);
        if (it == batch->mBatchBufs.end()) {
            ALOGE("%s: cannot find stream %d in batched buffers!", __FUNCTION__, streamId);
            return;
        }
        InflightBatch::BufferBatch& bb = it->second;
        bb.mDelivered = true;
        bb.mBuffers.clear();
    }
}

void CameraDeviceSession::ResultBatcher::sendBatchMetadataLocked(
    std::shared_ptr<InflightBatch> batch, uint32_t lastPartialResultIdx) {
    if (lastPartialResultIdx <= batch->mPartialResultProgress) {
        // Result has been delivered. Return
        ALOGW("%s: partial result %u has been delivered", __FUNCTION__, lastPartialResultIdx);
        return;
    }

    std::vector<CaptureResult> results;
    std::vector<uint32_t> toBeRemovedIdxes;
    for (auto& pair : batch->mResultMds) {
        uint32_t partialIdx = pair.first;
        if (partialIdx > lastPartialResultIdx) {
            continue;
        }
        toBeRemovedIdxes.push_back(partialIdx);
        InflightBatch::MetadataBatch& mb = pair.second;
        for (const auto& p : mb.mMds) {
            CaptureResult result;
            result.frameNumber = p.first;
            result.result = std::move(p.second);
            result.fmqResultSize = 0;
            result.inputBuffer.streamId = -1;
            result.inputBuffer.bufferId = 0;
            result.inputBuffer.buffer = aidl::android::hardware::common::NativeHandle();
            result.partialResult = partialIdx;
            results.push_back(std::move(result));
        }
        mb.mMds.clear();
    }
    invokeProcessCaptureResultCallback(results, /* tryWriteFmq */true);
    batch->mPartialResultProgress = lastPartialResultIdx;
    for (uint32_t partialIdx : toBeRemovedIdxes) {
        batch->mResultMds.erase(partialIdx);
    }
}

void CameraDeviceSession::ResultBatcher::notifySingleMsg(NotifyMsg& msg) {
    auto ret = mCallback->notify({msg});
    if (!ret.isOk()) {
        ALOGE("%s: notify transaction failed: %s",
                __FUNCTION__, ret.getDescription().c_str());
    }
}

void CameraDeviceSession::ResultBatcher::notify(NotifyMsg& msg) {
    uint32_t frameNumber;
    if (msg.getTag() == NotifyMsg::Tag::shutter) {
        frameNumber = msg.get<NotifyMsg::Tag::shutter>().frameNumber;
    } else {
        frameNumber = msg.get<NotifyMsg::Tag::error>().frameNumber;
    }

    auto pair = getBatch(frameNumber);
    int batchIdx = pair.first;
    if (batchIdx == NOT_BATCHED) {
        notifySingleMsg(msg);
        return;
    }

    // When error happened, stop batching for all batches earlier
    if (msg.getTag() == NotifyMsg::Tag::error) {
        Mutex::Autolock _l(mLock);
        for (int i = 0; i <= batchIdx; i++) {
            // Send batched data up
            std::shared_ptr<InflightBatch> batch = mInflightBatches[0];
            if (batch != nullptr)
            {
                Mutex::Autolock _l(batch->mLock);
                sendBatchShutterCbsLocked(batch);
                sendBatchBuffersLocked(batch);
                sendBatchMetadataLocked(batch, mNumPartialResults);
                if (!batch->allDelivered()) {
                    ALOGE("%s: error: some batch data not sent back to framework!",
                            __FUNCTION__);
                }
                batch->mRemoved = true;
            }
            mInflightBatches.pop_front();
        }
        // Send the error up
        notifySingleMsg(msg);
        return;
    }
    // Queue shutter callbacks for future delivery
    std::shared_ptr<InflightBatch> batch = pair.second;
    {
        Mutex::Autolock _l(batch->mLock);
        // Check if the batch is removed (mostly by notify error) before lock was acquired
        if (batch->mRemoved) {
            // Fall back to non-batch path
            notifySingleMsg(msg);
            return;
        }

        batch->mShutterMsgs.push_back(msg);
        if (frameNumber == batch->mLastFrame) {
            sendBatchShutterCbsLocked(batch);
        }
    } // end of batch lock scope

    // see if the batch is complete
    if (frameNumber == batch->mLastFrame) {
        checkAndRemoveFirstBatch();
    }
}

void CameraDeviceSession::ResultBatcher::processCaptureResult(CaptureResult& result) {
    auto pair = getBatch(result.frameNumber);
    int batchIdx = pair.first;
    if (batchIdx == NOT_BATCHED) {
        processOneCaptureResult(result);
        return;
    }
    std::shared_ptr<InflightBatch> batch = pair.second;
    {
        Mutex::Autolock _l(batch->mLock);
        // Check if the batch is removed (mostly by notify error) before lock was acquired
        if (batch->mRemoved) {
            // Fall back to non-batch path
            processOneCaptureResult(result);
            return;
        }

        // queue metadata
        if (result.result.metadata.size() != 0) {
            // Save a copy of metadata
            batch->mResultMds[result.partialResult].mMds.push_back(
                    std::make_pair(result.frameNumber, result.result));
        }

        // queue buffer
        std::vector<int> filledStreams;
        std::vector<StreamBuffer> nonBatchedBuffers;
        for (auto& buffer : result.outputBuffers) {
            auto it = batch->mBatchBufs.find(buffer.streamId);
            if (it != batch->mBatchBufs.end()) {
                InflightBatch::BufferBatch& bb = it->second;
                pushStreamBuffer(std::move(buffer), bb.mBuffers);
                filledStreams.push_back(buffer.streamId);
            } else {
                pushStreamBuffer(std::move(buffer), nonBatchedBuffers);
            }
        }

        // send non-batched buffers up
        if (nonBatchedBuffers.size() > 0 || result.inputBuffer.streamId != -1) {
            CaptureResult nonBatchedResult;
            nonBatchedResult.frameNumber = result.frameNumber;
            nonBatchedResult.fmqResultSize = 0;
            nonBatchedResult.outputBuffers.resize(nonBatchedBuffers.size());
            for (size_t i = 0; i < nonBatchedBuffers.size(); i++) {
                moveStreamBuffer(
                        std::move(nonBatchedBuffers[i]), nonBatchedResult.outputBuffers[i]);
            }
            moveStreamBuffer(std::move(result.inputBuffer), nonBatchedResult.inputBuffer);
            nonBatchedResult.partialResult = 0; // 0 for buffer only results
            processOneCaptureResult(nonBatchedResult);
        }

        if (result.frameNumber == batch->mLastFrame) {
            // Send data up
            if (result.partialResult > 0) {
                sendBatchMetadataLocked(batch, result.partialResult);
            }
            // send buffer up
            if (filledStreams.size() > 0) {
                sendBatchBuffersLocked(batch, filledStreams);
            }
        }
    } // end of batch lock scope

    // see if the batch is complete
    if (result.frameNumber == batch->mLastFrame) {
        checkAndRemoveFirstBatch();
    }
}

void CameraDeviceSession::ResultBatcher::processOneCaptureResult(CaptureResult& result) {
    std::vector<CaptureResult> results;
    results.resize(1);
    results[0] = std::move(result);
    invokeProcessCaptureResultCallback(results, /* tryWriteFmq */true);
}

void CameraDeviceSession::ResultBatcher::invokeProcessCaptureResultCallback(
        std::vector<CaptureResult> &results, bool tryWriteFmq) {
    if (mProcessCaptureResultLock.tryLock() != OK) {
        ALOGV("%s: previous call is not finished! waiting 1s...", __FUNCTION__);
        if (mProcessCaptureResultLock.timedLock(1000000000 /* 1s */) != OK) {
            ALOGE("%s: cannot acquire lock in 1s, cannot proceed",
                    __FUNCTION__);
            return;
        }
    }
    if (tryWriteFmq && mResultMetadataQueue->availableToWrite() > 0) {
        for (CaptureResult &result : results) {
            if (result.result.metadata.size() > 0) {
                if (mResultMetadataQueue->write(reinterpret_cast<int8_t*>(result.result.metadata.data()),
                        result.result.metadata.size())) {
                    result.fmqResultSize = result.result.metadata.size();
                    result.result.metadata.resize(0);
                } else {
                    ALOGW("%s: couldn't utilize fmq, fall back to hwbinder", __FUNCTION__);
                    result.fmqResultSize = 0;
                }
            }

            for (auto& onePhysMetadata : result.physicalCameraMetadata) {
                if (mResultMetadataQueue->write(reinterpret_cast<int8_t*>(onePhysMetadata.metadata.metadata.data()),
                        onePhysMetadata.metadata.metadata.size())) {
                    onePhysMetadata.fmqMetadataSize = onePhysMetadata.metadata.metadata.size();
                    onePhysMetadata.metadata.metadata.resize(0);
                } else {
                    ALOGW("%s: couldn't utilize fmq, fall back to hwbinder", __FUNCTION__);
                    onePhysMetadata.fmqMetadataSize = 0;
                }
            }
        }
    }
    mCallback->processCaptureResult(results);
    mProcessCaptureResultLock.unlock();
}

bool CameraDeviceSession::ResultBatcher::InflightBatch::allDelivered() const {
    if (!mShutterDelivered) return false;

    if (mPartialResultProgress < mNumPartialResults) {
        return false;
    }

    for (const auto& pair : mBatchBufs) {
        if (!pair.second.mDelivered) {
            return false;
        }
    }
    return true;
}

}  // namespace implementation
}  // namespace device
}  // namespace camera
}  // namespace hardware
}  // namespace android
