/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries. 
 * All rights reserved.
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

#include "EvsAISCamera.h"
#include "EvsEnumerator.h"
#include "bufferCopy.h"
#include "vendor_ext_properties.h"

#include <android/hardware_buffer.h>
#include <android-base/logging.h>
#include <ui/GraphicBufferAllocator.h>
#include <ui/GraphicBufferMapper.h>
#include <utils/SystemClock.h>
#include <sys/mman.h> //for PROT_READ | PROT_WRITE, MAP_SHARED
#ifdef FPS_PRINT
#include <sys/time.h>
#endif
#include <chrono>
#include <android/hardware/graphics/mapper/4.0/IMapper.h>
using Error_v4       = ::android::hardware::graphics::mapper::V4_0::Error;
using IMapper_v4     = ::android::hardware::graphics::mapper::V4_0::IMapper;

#define MIN_AIS_BUF_CNT 5
#define MAX_EVENT_Q_SIZE 5
#define QCARCAM_DEFAULT_GET_FRAME_TIMEOUT 500000000
#define NOT_IN_USE 0
#define ALIGN(num, to) (((num) + (to-1)) & (~(to-1))) /* To align to 8,16,32 or 64 bit values. */
#define WIDTH_ALIGN 64

namespace android {
namespace hardware {
namespace automotive {
namespace evs {
namespace V1_1 {
namespace implementation {

static char const* printHalFormat(uint32_t halFormat);
// Arbitrary limit on number of graphics buffers allowed to be allocated
// Safeguards against unreasonable resource consumption and provides a testable limit
static const unsigned MAX_BUFFERS_IN_FLIGHT = 100;

EvsAISCamera::EvsAISCamera(const char *deviceName,
                           unique_ptr<ConfigManager::CameraInfo> &camInfo,
                           EvsEnumerator *service) :
        mFramesAllowed(0),
        mFramesInUse(0),
        mCameraInfo(camInfo) {
    LOG(DEBUG) << "EvsAISCamera instantiated";

    mDescription.v1.cameraId = deviceName;
    if (camInfo != nullptr) {
        mDescription.metadata.setToExternal((uint8_t *)camInfo->characteristics,
                                            get_camera_metadata_size(camInfo->characteristics));
    }

    // Default output buffer format.
    mOutBufFormat = HAL_PIXEL_FORMAT_RGBA_8888;

    // How we expect to use the gralloc buffers we'll exchange with our client
    mUsage  = GRALLOC_USAGE_HW_TEXTURE     |
              GRALLOC_USAGE_SW_READ_OFTEN |
              GRALLOC_USAGE_SW_WRITE_OFTEN;
    mpGfxBufs = nullptr;
    mpQcarcamMmapBufs = nullptr;
    memset(&mQcarcamOutBufs, 0x0, sizeof(mQcarcamOutBufs));
    memset(&mQcarcamFrameInfo, 0x0, sizeof(mQcarcamFrameInfo));
    // direct rendering with externally allocated buffer
    mDirectRendering = false;
    mEvs = service;
#ifdef FPS_PRINT
    mFrameCnt = 0;
    mPrevMsec = 0;
#endif
}


EvsAISCamera::~EvsAISCamera() {
    LOG(DEBUG) << "EvsAISCamera being destroyed";
    mEvs = nullptr;
    shutdown();
    for (int i = 0; i < (int)mQcarcamOutBufs.n_buffers; ++i) {
        if (mpQcarcamMmapBufs && mpQcarcamMmapBufs[i].ptr)
            munmap(mpQcarcamMmapBufs[i].ptr, mpQcarcamMmapBufs[i].size);
        if (mpGfxBufs && mpGfxBufs[i])
            mpGfxBufs[i] = nullptr; //No need to delete sp<GraphicBuffer>
    }
    if (mpGfxBufs)
        free(mpGfxBufs);
    if (mQcarcamOutBufs.buffers)
        free(mQcarcamOutBufs.buffers);
    if (mpQcarcamMmapBufs)
        free(mpQcarcamMmapBufs);
}


//
// This gets called if another caller "steals" ownership of the camera
//
void EvsAISCamera::shutdown()
{
    LOG(DEBUG) << "EvsAISCamera shutdown";

    if (!mQcarcamContext) {
        LOG(WARNING) << "Ignoring this call when camera has been lost.";
        return;
    }

    // Make sure our output stream is cleaned up
    // (It really should be already)
    stopVideoStream();

    // Note:  Since stopVideoStream is blocking, no other threads can now be running

    // Close qcarcam
    qcarcam_ret_t ret;
    LOG(DEBUG) << "EvsAISCamera being destroyed";
    // Close AIS camera stream
    ret = qcarcam_close(this->mQcarcamContext);
    if (ret != QCARCAM_RET_OK)
        LOG(ERROR) << "qcarcam_close() failed";
    if (mC2dInitStatus && (mHalFrameFormat != mOutBufFormat)) {
        std::map<std::uint64_t, void *>::iterator it;

        for (it = mTgtGpuMapList.begin(); it != mTgtGpuMapList.end(); ++it) {
            c2dUnMapAddr(it->second);
        }
        for (it = mSrcGpuMapList.begin(); it != mSrcGpuMapList.end(); ++it) {
            c2dUnMapAddr(it->second);
        }

        std::map<uint64_t, uint32_t>::iterator itr;

        /* Destroy C2D Source Surface */
        for (int i = 0; i < (int)mQcarcamOutBufs.n_buffers; ++i) {
            c2dDestroySurface(mpQcarcamMmapBufs[i].srcC2dSurfaceId);
        }
        /* Destroy C2D Target Surface */
        for (itr = mTgtSurfaceIdList.begin(); itr != mTgtSurfaceIdList.end(); ++itr) {
            c2dDestroySurface(itr->second);
        }
        mTgtGpuMapList.clear();
        mSrcGpuMapList.clear();
        mTgtSurfaceIdList.clear();
    }
    // Clear external FD list
    if (mDirectRendering)
        mFdToMemHndlMapList.clear();
    // Remove mQcarcamContext from Client Record
    mMultiCamQLock.lock();
    LOG(DEBUG) << "Removing mQcarcamContext = "<< this->mQcarcamContext <<" from Q";
    gQcarCamClientData *pRecord = findEventQByHndl(this->mQcarcamContext);
    if (pRecord != NULL)
        pRecord->gQcarcamContext = NOT_IN_USE;
    mMultiCamQLock.unlock();
    this->mQcarcamContext = NOT_IN_USE;
    pCurrentRecord = nullptr;

    // Drop all the graphics buffers we've been using
    if (mBuffers.size() > 0) {
        GraphicBufferAllocator& alloc(GraphicBufferAllocator::get());
        for (auto&& rec : mBuffers) {
            if (rec.inUse) {
                LOG(WARNING) << "Releasing buffer despite remote ownership";
            }
            alloc.free(rec.handle);
            rec.handle = nullptr;
        }
        mBuffers.clear();
    }
}


// Methods from ::android::hardware::automotive::evs::V1_0::IEvsCamera follow.
Return<void> EvsAISCamera::getCameraInfo(getCameraInfo_cb _hidl_cb) {
    LOG(DEBUG) << __FUNCTION__;

    // Send back our self description
    _hidl_cb(mDescription.v1);
    return Void();
}

/**
 * get_handle_fd
 *
 * @brief retrieves file descriptor for handle
 *
 * @param hndl is of type buffer_handle_t
 *
 * @return int
*/
static int get_handle_fd(buffer_handle_t hndl)
{
    sp<IMapper_v4> mapper = IMapper_v4::getService();
    int fd = -1;
    mapper->get(const_cast<native_handle_t*>(hndl), qtigralloc::MetadataType_FD,
        [&](const auto &_error, const auto &_bytestream) {
                if (_error !=  Error_v4::NONE)
                    LOG(ERROR) << "Failed to get FD";
                else {
                    android::gralloc4::decodeInt32(qtigralloc::MetadataType_FD,
                            _bytestream, &fd);
                }
    });
    return fd;
}

Return<EvsResult> EvsAISCamera::setMaxFramesInFlight(uint32_t bufferCount) {
    LOG(DEBUG) << __FUNCTION__;
    std::lock_guard<std::mutex> lock(mAccessLock);

    if (bufferCount > MAX_BUFFERS_IN_FLIGHT) {
        LOG(WARNING) << "Rejecting buffer request in excess of internal limit";
        return EvsResult::BUFFER_NOT_AVAILABLE;;
    }

    // If we've been displaced by another owner of the camera, then we can't do anything else
    if (!mQcarcamContext) {
        LOG(WARNING) << "Ignoring setMaxFramesInFlight call when camera has been lost.";
        return EvsResult::OWNERSHIP_LOST;
    }

    // We cannot function without at least one video buffer to send data
    if (bufferCount < 1) {
        LOG(ERROR) << "Ignoring setMaxFramesInFlight with less than one buffer requested";
        return EvsResult::INVALID_ARG;
    }

    // Update our internal state
    if (!setAvailableFrames_Locked(bufferCount)) {
        return EvsResult::BUFFER_NOT_AVAILABLE;
    }

    if (0 == mQcarcamOutBufs.n_buffers) {
        //allocate qcarcam internal buffers
        if (!allocQcarcamInternalBuffers(MIN_AIS_BUF_CNT)) {
            return EvsResult::BUFFER_NOT_AVAILABLE;
        }

        LOG(DEBUG) << " calling qcarcam_s_buffers";
        mQcarcamOutBufs.flags = 0;
        qcarcam_ret_t ret = QCARCAM_RET_OK;
        ret = qcarcam_s_buffers(mQcarcamContext, &mQcarcamOutBufs);
        if (ret != QCARCAM_RET_OK) {
            LOG(ERROR) << "qcarcam_s_buffers failed!";
            return EvsResult::BUFFER_NOT_AVAILABLE;
        }
    }

    return EvsResult::OK;
}


Return<EvsResult> EvsAISCamera::startVideoStream(const sp<IEvsCameraStream_1_0>& stream)  {
    LOG(DEBUG) << __FUNCTION__;

    // If we've been displaced by another owner of the camera, then we can't do anything else
    if (!mQcarcamContext) {
        LOG(WARNING) << "Ignoring startVideoStream call when camera has been lost.";
        return EvsResult::OWNERSHIP_LOST;
    }

    if (mStream.get() != nullptr) {
        LOG(ERROR) << "Ignoring startVideoStream call when a stream is already running.";
        return EvsResult::STREAM_ALREADY_RUNNING;
    }

    //Initialize mFrameInUse to zero at start of video stream
    mFramesInUse = 0;

    // If the client never indicated otherwise, configure ourselves for 3 streaming buffer
    if (mFramesAllowed < 1) {
        Return <EvsResult> result = setMaxFramesInFlight(3);
        if (result != EvsResult::OK) {
            LOG(WARNING) << "Allocating streaming buffers";
            return result;
        }
    }

    std::lock_guard<std::mutex> lock(mAccessLock);
    int ret = 0;
    qcarcam_param_value_t param;
    param.ptr_value = (void *)&EvsAISCamera::evs_ais_event_cb;
    ret = qcarcam_s_param(mQcarcamContext, QCARCAM_PARAM_EVENT_CB, &param);

    // Default qcarcam events

    // To enable self-recovery, call qcarcam_s_param with QCARCAM_PARAM_RECOVERY
    param.uint_value = (QCARCAM_EVENT_FRAME_READY | QCARCAM_EVENT_ERROR |
                        QCARCAM_EVENT_RECOVERY | QCARCAM_EVENT_RECOVERY_SUCCESS | QCARCAM_EVENT_FRAME_DROP | QCARCAM_EVENT_VENDOR);
    ret = qcarcam_s_param(mQcarcamContext, QCARCAM_PARAM_EVENT_MASK, &param);

    ret = qcarcam_start(mQcarcamContext);
    if (ret != QCARCAM_RET_OK) {
        LOG(ERROR) << "qcarcam_start() failed";
        return EvsResult::UNDERLYING_SERVICE_ERROR;
    } else
        LOG(DEBUG) << "qcarcam_start() success for stream "<<mDescription.v1.cameraId;

    // Record the user's callback for use when we have a frame ready
    mStream = stream;
    mStream_1_1 = IEvsCameraStream_1_1::castFrom(mStream).withDefault(nullptr);

    if (mStream_1_1 == nullptr) {
        LOG(ERROR) << "Invalid mStream_1_1 for streamId " <<mDescription.v1.cameraId
                   << " continuing with mStream1_0";
    }

    // Set the state of our background thread
    int prevRunMode = mRunMode.fetch_or(RUN);
    if (prevRunMode & RUN) {
        // The background thread is already running, so we can't start a new stream
        LOG(ERROR) << "Already in RUN state, so we can't start a new streaming thread";
        return EvsResult::OK;
    }

    // Fire up a thread to receive and dispatch the qcarcam frames
    mCaptureThread = std::thread([this](){ collectEvents(); });

    if (mHalFrameFormat == mOutBufFormat) {
        mFillBufferFromVideo = fillOutBufWithoutConversion;
    } else {
        switch (mHalFrameFormat) {
            case HAL_PIXEL_FORMAT_CbYCrY_422_I:
                mFillBufferFromVideo = fillRGBAFromUYVY;    break;
                break;
            case HAL_PIXEL_FORMAT_YCBCR_422_I:
                mFillBufferFromVideo = fillRGBAFromYUYV;    break;
                break;
            default:
                LOG(ERROR) << "Unhandled output format 0x" << std::hex <<mAisFrameFormat;
        }
    }

    return EvsResult::OK;
}


Return<void> EvsAISCamera::doneWithFrame(const BufferDesc_1_0& buffer)  {
    doneWithFrame_impl(buffer.bufferId, buffer.memHandle);

    return Void();
}


Return<void> EvsAISCamera::stopVideoStream()  {
    LOG(DEBUG) << __FUNCTION__ <<" for stream "<<mDescription.v1.cameraId;

    qcarcam_ret_t ret = QCARCAM_RET_OK;

    if (mStream == nullptr) {
        LOG(ERROR) << "Ignoring stopVideoStream call when a stream is already stopped.";
        return Void();
    }

    if (!mQcarcamContext) {
        LOG(WARNING) << "Ignoring stopVideoStream call when camera has been lost.";
        return Void();
    }

    // Stop AIS stream
    ret = qcarcam_stop(mQcarcamContext);
    if (ret != QCARCAM_RET_OK)
        LOG(ERROR) << "qcarcam_stop() failed";

    // Indicate the background thread to stop
    int prevRunMode = mRunMode.fetch_or(STOPPING);
    if (prevRunMode == STOPPED) {
        // The background thread wasn't running, so set the flag back to STOPPED
        mRunMode = STOPPED;
    } else if (prevRunMode & STOPPING) {
        LOG(ERROR) << "stopStream called while stream is already stopping.  Reentrancy is not supported!";
        return Void();
    } else {
        if (!pCurrentRecord) {
            LOG(ERROR) << "pCurrentRecord is null";
            return Void();
        }
        pCurrentRecord->gCV.notify_one();
        // Block until the background thread is stopped
        if (mCaptureThread.joinable()) {
            mCaptureThread.join();
        }
        LOG(DEBUG) << "Capture thread stopped.";
    }

    if (mStream_1_1 != nullptr) {
        // V1.1 client is waiting on STREAM_STOPPED event.
        std::unique_lock <std::mutex> lock(mAccessLock);

        EvsEventDesc event;
        event.aType = EvsEventType::STREAM_STOPPED;
        auto result = mStream_1_1->notify(event);
        if (!result.isOk()) {
            LOG(ERROR) << "Error delivering end of stream event";
        }

        // Drop our reference to the client's stream receiver
        mStream_1_1 = nullptr;
        mStream     = nullptr;
    } else if (mStream != nullptr) {
        std::unique_lock <std::mutex> lock(mAccessLock);

        // Send one last NULL frame to signal the actual end of stream
        BufferDesc_1_0 nullBuff = {};
        auto result = mStream->deliverFrame(nullBuff);
        if (!result.isOk()) {
            LOG(ERROR) << "Error delivering end of stream marker";
        }

        // Drop our reference to the client's stream receiver
        mStream = nullptr;
    }

    return Void();
}


Return<int32_t> EvsAISCamera::getExtendedInfo(uint32_t /*opaqueIdentifier*/)  {
    LOG(DEBUG) << __FUNCTION__;
    // Return zero by default as required by the spec
    return 0;
}


Return<EvsResult> EvsAISCamera::setExtendedInfo(uint32_t /*opaqueIdentifier*/,
                                                int32_t  /*opaqueValue*/)  {
    LOG(DEBUG) << __FUNCTION__;
    std::lock_guard<std::mutex> lock(mAccessLock);

    // We don't store any device specific information in this implementation
    return EvsResult::INVALID_ARG;
}


// Methods from ::android::hardware::automotive::evs::V1_1::IEvsCamera follow.
Return<void> EvsAISCamera::getCameraInfo_1_1(getCameraInfo_1_1_cb _hidl_cb) {
    LOG(DEBUG) << __FUNCTION__;

    // Send back our self description
    _hidl_cb(mDescription);
    return Void();
}


Return<void> EvsAISCamera::getPhysicalCameraInfo(const hidl_string& id,
                                                 getPhysicalCameraInfo_cb _hidl_cb) {
    LOG(DEBUG) << __FUNCTION__;

    // This method works exactly same as getCameraInfo_1_1() in EVS HW module.
    (void)id;
    _hidl_cb(mDescription);
    return Void();
}


Return<EvsResult> EvsAISCamera::doneWithFrame_1_1(const hidl_vec<BufferDesc_1_1>& buffers)  {

    for (auto&& buffer : buffers) {
        doneWithFrame_impl(buffer.bufferId, buffer.buffer.nativeHandle);
    }

    return EvsResult::OK;
}


Return<EvsResult> EvsAISCamera::pauseVideoStream() {

    if(!qcarcam_pause(mQcarcamContext))
        return EvsResult::OK;

    return EvsResult::UNDERLYING_SERVICE_ERROR;
}


Return<EvsResult> EvsAISCamera::resumeVideoStream() {

    if(!qcarcam_resume(mQcarcamContext))
        return EvsResult::OK;

    return EvsResult::UNDERLYING_SERVICE_ERROR;
}


Return<EvsResult> EvsAISCamera::setMaster() {
    /* Because EVS HW module reference implementation expects a single client at
     * a time, this returns a success code always.
     */
    return EvsResult::OK;
}


Return<EvsResult> EvsAISCamera::forceMaster(const sp<IEvsDisplay_1_0>&) {
    /* Because EVS HW module reference implementation expects a single client at
     * a time, this returns a success code always.
     */
    return EvsResult::OK;
}


Return<EvsResult> EvsAISCamera::unsetMaster() {
    /* Because EVS HW module reference implementation expects a single client at
     * a time, there is no chance that this is called by a non-master client and
     * therefore returns a success code always.
     */
    return EvsResult::OK;
}


Return<void> EvsAISCamera::getParameterList(getParameterList_cb _hidl_cb) {
    hidl_vec<CameraParam> hidlCtrls;
    if (mCameraInfo != nullptr) {
        hidlCtrls.resize(mCameraInfo->controls.size());
        unsigned idx = 0;
        for (auto& [cid, range]: mCameraInfo->controls) {
            hidlCtrls[idx++] = cid;
        }
    }

    _hidl_cb(hidlCtrls);
    return Void();
}


Return<void> EvsAISCamera::getIntParameterRange(CameraParam id,
                                                getIntParameterRange_cb _hidl_cb) {
    if (mCameraInfo != nullptr) {
        auto range = mCameraInfo->controls[id];
        _hidl_cb(get<0>(range), get<1>(range), get<2>(range));
    } else {
        _hidl_cb(0, 0, 0);
    }

    return Void();
}


Return<void> EvsAISCamera::setIntParameter(CameraParam id, int32_t value,
                                           setIntParameter_cb _hidl_cb) {
    int ret = 0;
    qcarcam_param_value_t param = {};
    qcarcam_param_t qcarcamParamId;
    hidl_vec<int32_t> values;
    values.resize(1);
    values[0] = value;

    if (!convertToqcarcamParamID(id, qcarcamParamId, values, param)) {
        _hidl_cb(EvsResult::INVALID_ARG, values);
    } else {
        EvsResult result = EvsResult::OK;
        ret = qcarcam_s_param(mQcarcamContext, qcarcamParamId, &param);
        if (ret != QCARCAM_RET_OK) {
            result = EvsResult::UNDERLYING_SERVICE_ERROR;
        }

        _hidl_cb(result, values);
    }

    return Void();
}


Return<void> EvsAISCamera::getIntParameter(CameraParam id,
                                           getIntParameter_cb _hidl_cb) {
    int ret = 0;
    qcarcam_param_value_t param;
    qcarcam_param_t qcarcamParamId;
    hidl_vec<int32_t> values;
    values.resize(1);

    if (!convertToqcarcamParamID(id, qcarcamParamId, values, param)) {
        _hidl_cb(EvsResult::INVALID_ARG, values);
    } else {
        EvsResult result = EvsResult::OK;
        ret = qcarcam_g_param(mQcarcamContext, qcarcamParamId, &param);
        if (ret != QCARCAM_RET_OK) {
            result = EvsResult::INVALID_ARG;
        } else {
            convertFormqcarcamParamID(qcarcamParamId, values, param);
        }

        // Report a result
        _hidl_cb(result, values);
    }

    return Void();
}


Return<EvsResult> EvsAISCamera::setExtendedInfo_1_1(uint32_t opaqueIdentifier,
                                                    const hidl_vec<uint8_t>& opaqueValue) {
    if (QCARCAM_MAX_VENDOR_PAYLOAD_SIZE*(sizeof(unsigned int)) < opaqueValue.size()) {
        LOG(ERROR) << "Max supported values upto "<<QCARCAM_MAX_VENDOR_PAYLOAD_SIZE;
        return EvsResult::UNDERLYING_SERVICE_ERROR;
    }

    mExtInfo.insert_or_assign(opaqueIdentifier, opaqueValue);

    if (mQcarcamContext) {
        // Send extended info to qcarcam as vendor param
        qcarcam_ret_t ret = QCARCAM_RET_OK;
        qcarcam_param_value_t param = {};
        uint8_t *param_data;
        param_data = (uint8_t *)&param.vendor_param.data[0];
        for (int i=0; i<opaqueValue.size(); i++) {
            param_data[i] = opaqueValue[i];
        }
        ret = qcarcam_s_param(mQcarcamContext, (qcarcam_param_t)opaqueIdentifier, &param);
        if (QCARCAM_RET_OK != ret) {
            LOG(ERROR) <<"qcarcam_s_param failed for ctxt" <<mQcarcamContext;
            return EvsResult::UNDERLYING_SERVICE_ERROR;
        } else {
            LOG(DEBUG) <<"qcarcam_s_param success for ctxt" <<mQcarcamContext;
        }
    } else {
        LOG(WARNING) << "Ignoring setExtendedInfo call when camera has been lost.";
        return EvsResult::OWNERSHIP_LOST;
    }

    return EvsResult::OK;
}


Return<void> EvsAISCamera::getExtendedInfo_1_1(uint32_t opaqueIdentifier,
                                               getExtendedInfo_1_1_cb _hidl_cb) {
    const auto it = mExtInfo.find(opaqueIdentifier);
    hidl_vec<uint8_t> value;
    auto status = EvsResult::OK;
    if (it == mExtInfo.end()) {
        status = EvsResult::INVALID_ARG;
        _hidl_cb(status, value);
        return Void();
    } else {
        value = mExtInfo[opaqueIdentifier];
    }

    if (mQcarcamContext) {
        // Get extended info from qcarcam
        qcarcam_ret_t ret = QCARCAM_RET_OK;
        qcarcam_param_value_t param;
        uint8_t *param_data;
        param_data = (uint8_t *)&param.vendor_param.data[0];
        for (int i=0; i<value.size(); i++) {
            param_data[i] = value[i];
        }
        ret = qcarcam_g_param(mQcarcamContext, (qcarcam_param_t)opaqueIdentifier, &param);
        if (QCARCAM_RET_OK != ret) {
            LOG(ERROR) <<"qcarcam_g_param failed for ctxt" <<mQcarcamContext;
            status = EvsResult::UNDERLYING_SERVICE_ERROR;
        }
        else {
            LOG(DEBUG) <<"qcarcam_g_param success for ctxt" <<mQcarcamContext;
            for (int i=0; i<value.size(); i++) {
                value[i] = param_data[i];
            }
        }
    }

    _hidl_cb(status, value);
    return Void();
}


Return<void>
EvsAISCamera::importExternalBuffers(const hidl_vec<BufferDesc_1_1>& buffers,
                                    importExternalBuffers_cb _hidl_cb) {
    LOG(DEBUG) << __FUNCTION__;

    if (mDescription.v1.cameraId.size() > 1) {
        LOG(WARNING) << "Logical camera device does not support";
        _hidl_cb(EvsResult::UNDERLYING_SERVICE_ERROR, 0);
        return {};
    }

    // If we've been displaced by another owner of the camera, then we can't do anything else
    if (!mQcarcamContext) {
        LOG(WARNING) << "Ignoring a request add external buffers "
                     << "when camera has been lost.";
        _hidl_cb(EvsResult::UNDERLYING_SERVICE_ERROR, mFramesAllowed);
        return {};
    }
    if (buffers.size() == 0) {
        LOG(WARNING) << "Invalid buffer size";
        _hidl_cb(EvsResult::UNDERLYING_SERVICE_ERROR, 0);
        return {};
    }

    auto numBuffersToAdd = buffers.size();
    if (numBuffersToAdd < 1) {
        LOG(DEBUG) << "No buffers to add.";
        _hidl_cb(EvsResult::OK, mFramesAllowed);
        return {};
    }
    const auto before = mFramesAllowed;
    vector<int> extFds(numBuffersToAdd, -1);

    {
        std::scoped_lock<std::mutex> lock(mAccessLock);

        if (numBuffersToAdd > (MAX_BUFFERS_IN_FLIGHT - mFramesAllowed)) {
            numBuffersToAdd -= (MAX_BUFFERS_IN_FLIGHT - mFramesAllowed);
            LOG(WARNING) << "Exceed the limit on number of buffers.  "
                         << numBuffersToAdd << " buffers will be added only.";
        }

        GraphicBufferMapper& mapper = GraphicBufferMapper::get();
        for (auto i = 0; i < numBuffersToAdd; ++i) {
            // TODO: reject if external buffer is configured differently.
            auto& b = buffers[i];
            const AHardwareBuffer_Desc* pDesc =
                reinterpret_cast<const AHardwareBuffer_Desc *>(&b.buffer.description);

            mOutBufFormat = pDesc->format;
            mStride = pDesc->stride;
            mOutHeight = pDesc->height;
            mOutWidth = pDesc->width;
            // Import a buffer to add
            buffer_handle_t memHandle = nullptr;
            status_t result = mapper.importBuffer(b.buffer.nativeHandle,
                                                  pDesc->width,
                                                  pDesc->height,
                                                  1,
                                                  pDesc->format,
                                                  pDesc->usage,
                                                  pDesc->stride,
                                                  &memHandle);
            if (result != android::NO_ERROR || !memHandle) {
                LOG(WARNING) << "Failed to import a buffer " << b.bufferId;
                continue;
            }
            LOG(DEBUG) <<"QCDEBUG: memHandle=" << memHandle;
            LOG(DEBUG) <<"QCDEBUG: EVS nativeHandle=" <<b.buffer.nativeHandle << " ID="<<i <<" Format=" <<pDesc->format <<" W"<<pDesc->width <<" H" << pDesc->height;
            auto stored = false;
            for (auto&& rec : mBuffers) {
                if (rec.handle == nullptr) {
                    // Use this existing entry
                    rec.handle = memHandle;
                    rec.inUse = false;
                    stored = true;
                    break;
                }
            }

            if (!stored) {
                // Add a BufferRecord wrapping this handle to our set of available buffers
                mBuffers.emplace_back(memHandle);
            }
            if (mC2dInitStatus && (mHalFrameFormat != mOutBufFormat)) {
                void *targetPixels = NULL;
                C2D_YUV_SURFACE_DEF targetSurfaceDef;
                memset(&targetSurfaceDef, 0, sizeof(targetSurfaceDef));
                int fd = get_handle_fd(memHandle);
                int ret = -1;
                int size = pDesc->height * EvsAISCamera::getQcarcamStride(ALIGN(pDesc->width, WIDTH_ALIGN), mOutBufFormat);
                void *mapped_buf = mmap(NULL, size , PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                uint32_t targetSurfaceId = 0;
                C2D_STATUS c2d_status;

                c2d_status = c2dMapAddr(fd, mapped_buf, size, 0, KGSL_USER_MEM_TYPE_ION, &targetPixels);
                if (c2d_status != C2D_STATUS_OK || targetPixels == NULL) {
                    LOG(ERROR) <<"c2dMapAddr failed status "<< c2d_status;
                    break;
                }
                targetSurfaceDef.width = pDesc->width;
                targetSurfaceDef.height = pDesc->height;
                targetSurfaceDef.stride0 = EvsAISCamera::getQcarcamStride(ALIGN(pDesc->width, WIDTH_ALIGN), mOutBufFormat);
                targetSurfaceDef.stride1 = EvsAISCamera::getQcarcamStride(ALIGN(pDesc->width, WIDTH_ALIGN), mOutBufFormat);
                targetSurfaceDef.plane0 = mapped_buf;
                targetSurfaceDef.plane1 = (void *)((unsigned char *)targetSurfaceDef.plane0 + (targetSurfaceDef.width * targetSurfaceDef.height));
                targetSurfaceDef.format = getHalToC2dFormat(mOutBufFormat);
                targetSurfaceDef.phys0 = targetPixels;
                targetSurfaceDef.phys1 = 0;
                LOG(DEBUG) <<"Inserting target surface id "<<targetSurfaceId<<"at memHandle="<<(uint64_t)memHandle;
                ret = c2dCreateSurface(&targetSurfaceId, C2D_TARGET, (C2D_SURFACE_TYPE) (C2D_SURFACE_RGB_HOST | C2D_SURFACE_WITH_PHYS), &targetSurfaceDef);
                if (ret != C2D_STATUS_OK) {
                    c2dUnMapAddr(targetPixels);
                    LOG(ERROR) <<"Target c2d_create API falied ret="<< ret;
                    break;
                }
                mTgtSurfaceIdList.insert(std::pair<std::uint64_t, uint32_t>((uint64_t)memHandle, targetSurfaceId));
                mTgtGpuMapList.insert(std::pair<std::uint64_t, void *>((uint64_t)memHandle, targetPixels));
                munmap(mapped_buf, size);
            }
            else {
                // Store FDs for qcarcam_s_buffer
                extFds[i] = get_handle_fd(memHandle);
                // Map index with memHandle
                mFdToMemHndlMapList.insert(std::pair<std::uint32_t, std::uint64_t>(i, (uint64_t)memHandle));
            }

            ++mFramesAllowed;
        }
    }

    if ((mHalFrameFormat != mOutBufFormat) || (mOutHeight != mAisFrameHeight)
            || (mOutWidth != mAisFrameWidth)) {
        //allocate qcarcam internal buffers
        if (!allocQcarcamInternalBuffers(MIN_AIS_BUF_CNT)) {
            _hidl_cb(EvsResult::UNDERLYING_SERVICE_ERROR, mFramesAllowed);
            return {};
        }
    } else {
        if (!setExternalBuffersAsInteranl(extFds)) {
            _hidl_cb(EvsResult::UNDERLYING_SERVICE_ERROR, mFramesAllowed);
            return {};
        }
        mDirectRendering = true;
    }

    LOG(DEBUG) << " calling qcarcam_s_buffers";
    mQcarcamOutBufs.flags = 0;
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    ret = qcarcam_s_buffers(mQcarcamContext, &mQcarcamOutBufs);
    if (ret != QCARCAM_RET_OK) {
        LOG(ERROR) << "qcarcam_s_buffers failed!";
        _hidl_cb(EvsResult::UNDERLYING_SERVICE_ERROR, mFramesAllowed);
        return {};
    }

    _hidl_cb(EvsResult::OK, mFramesAllowed - before);
    return {};
}


EvsResult EvsAISCamera::doneWithFrame_impl(const uint32_t bufferId,
                                           const buffer_handle_t memHandle) {
    std::lock_guard <std::mutex> lock(mAccessLock);

    // If we've been displaced by another owner of the camera, then we can't do anything else
    if (!mQcarcamContext) {
        LOG(WARNING) << "Ignoring doneWithFrame call when camera has been lost.";
    } else {
        if (memHandle == nullptr) {
            LOG(ERROR) << "Ignoring doneWithFrame called with null handle";
        } else if (bufferId >= mBuffers.size()) {
            LOG(ERROR) << "Ignoring doneWithFrame called with invalid bufferId " << bufferId
                       << " (max is " << mBuffers.size() - 1 << ")";
        } else if (!mBuffers[bufferId].inUse) {
            LOG(ERROR) << "Ignoring doneWithFrame called on frame " << bufferId
                       << " which is already free";
        } else {
            if (mDirectRendering) {
                // Indicate available buffer to ais
                /* Get the idx corresponds to memHandle */
                std::map<std::uint32_t, std::uint64_t>::iterator it;
                it = mFdToMemHndlMapList.find((uint64_t)memHandle);
                if (it != mFdToMemHndlMapList.end()) {
                    uint32_t idx = it->first;
                    qcarcam_ret_t ret = qcarcam_release_frame(mQcarcamContext, idx);
                    if (QCARCAM_RET_OK != ret)
                        LOG(ERROR) << "qcarcam_release_frame()"<< idx << " failed";
                } else
                    LOG(ERROR) <<"Error while traversing, did not find memHandle="<<memHandle;
            }
            // Mark the frame as available
            mBuffers[bufferId].inUse = false;
            if (mFramesInUse > 0) {
                mFramesInUse--;
            } else {
                LOG(ERROR) << "Attempt to decrement, while frames not in use";
           }

            // If this frame's index is high in the array, try to move it down
            // to improve locality after mFramesAllowed has been reduced.
            if (bufferId >= mFramesAllowed) {
                // Find an empty slot lower in the array (which should always exist in this case)
                for (auto&& rec : mBuffers) {
                    if (rec.handle == nullptr) {
                        rec.handle = mBuffers[bufferId].handle;
                        mBuffers[bufferId].handle = nullptr;
                        break;
                    }
                }
            }
        }
    }

    return EvsResult::OK;
}


bool EvsAISCamera::setAvailableFrames_Locked(unsigned bufferCount) {
    if (bufferCount < 1) {
        LOG(ERROR) << "Ignoring request to set buffer count to zero";
        return false;
    }
    if (bufferCount > MAX_BUFFERS_IN_FLIGHT) {
        LOG(ERROR) << "Rejecting buffer request in excess of internal limit";
        return false;
    }

    // Is an increase required?
    if (mFramesAllowed < bufferCount) {
        // An increase is required
        unsigned needed = bufferCount - mFramesAllowed;
        LOG(INFO) << "Allocating " << needed << " buffers for camera frames";

        unsigned added = increaseAvailableFrames_Locked(needed);
        if (added != needed) {
            // If we didn't add all the frames we needed, then roll back to the previous state
            LOG(ERROR) << "Rolling back to previous frame queue size";
            decreaseAvailableFrames_Locked(added);
            return false;
        }
    } else if (mFramesAllowed > bufferCount) {
        // A decrease is required
        unsigned framesToRelease = mFramesAllowed - bufferCount;
        LOG(INFO) << "Returning " << framesToRelease << " camera frame buffers";

        unsigned released = decreaseAvailableFrames_Locked(framesToRelease);
        if (released != framesToRelease) {
            // This shouldn't happen with a properly behaving client because the client
            // should only make this call after returning sufficient outstanding buffers
            // to allow a clean resize.
            LOG(ERROR) << "Buffer queue shrink failed -- too many buffers currently in use?";
        }
    }

    return true;
}


unsigned EvsAISCamera::increaseAvailableFrames_Locked(unsigned numToAdd) {
    // Acquire the graphics buffer allocator
    GraphicBufferAllocator &alloc(GraphicBufferAllocator::get());

    unsigned added = 0;


    while (added < numToAdd) {
        unsigned pixelsPerLine;
        buffer_handle_t memHandle = nullptr;
        mOutHeight = mAisFrameHeight;
        mOutWidth = mAisFrameWidth;
        status_t result = alloc.allocate(ALIGN(mAisFrameWidth, WIDTH_ALIGN),
                                         mAisFrameHeight,
                                         mOutBufFormat, 1,
                                         mUsage,
                                         &memHandle, &pixelsPerLine, 0, "EvsAISCamera");
        if (result != NO_ERROR) {
            LOG(ERROR) << "Error " << result << " allocating "
                       << ALIGN(mAisFrameWidth, WIDTH_ALIGN) << " x " << mAisFrameHeight
                       << " graphics buffer";
            break;
        } else {
            LOG(DEBUG) << "Allocated Render RGBA buffer "<<memHandle<<endl
                <<"Width="<<ALIGN(mAisFrameWidth, WIDTH_ALIGN)<<" Height="<<mAisFrameHeight<<endl
                <<"Stride="<<pixelsPerLine<<" Format="<<printHalFormat(mOutBufFormat);
        }
        if (!memHandle) {
            LOG(ERROR) << "We didn't get a buffer handle back from the allocator";
            break;
        }
        if (mStride) {
            if (mStride != pixelsPerLine) {
                LOG(ERROR) << "We did not expect to get buffers with different strides! "<<mStride;
            }
        } else {
            // Gralloc defines stride in terms of pixels per line
            mStride = pixelsPerLine;
        }

        // Find a place to store the new buffer
        bool stored = false;
        for (auto&& rec : mBuffers) {
            if (rec.handle == nullptr) {
                // Use this existing entry
                rec.handle = memHandle;
                rec.inUse = false;
                stored = true;
                break;
            }
        }
        if (!stored) {
            // Add a BufferRecord wrapping this handle to our set of available buffers
            mBuffers.emplace_back(memHandle);
        }
        if (mC2dInitStatus && (mHalFrameFormat != mOutBufFormat)) {
            void *targetPixels = NULL;
            C2D_YUV_SURFACE_DEF targetSurfaceDef;
            memset(&targetSurfaceDef, 0, sizeof(targetSurfaceDef));
            int fd = get_handle_fd(memHandle);
            int ret = -1;
            int size = mAisFrameHeight * EvsAISCamera::getQcarcamStride(ALIGN(mStride, WIDTH_ALIGN), mOutBufFormat);
            void *mapped_buf = mmap(NULL, size , PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            uint32_t targetSurfaceId = 0;
            C2D_STATUS c2d_status;

            c2d_status = c2dMapAddr(fd, mapped_buf, size, 0, KGSL_USER_MEM_TYPE_ION, &targetPixels);
            if (c2d_status != C2D_STATUS_OK || targetPixels == NULL) {
                LOG(ERROR) <<"c2dMapAddr failed status "<< c2d_status;
                break;
            }
            targetSurfaceDef.width = mAisFrameWidth;
            targetSurfaceDef.height = mAisFrameHeight;
            targetSurfaceDef.stride0 = EvsAISCamera::getQcarcamStride(ALIGN(mStride, WIDTH_ALIGN), mOutBufFormat);
            targetSurfaceDef.stride1 = EvsAISCamera::getQcarcamStride(ALIGN(mStride, WIDTH_ALIGN), mOutBufFormat);
            targetSurfaceDef.plane0 = mapped_buf;
            targetSurfaceDef.plane1 = (void *)((unsigned char *)targetSurfaceDef.plane0 + (targetSurfaceDef.width * targetSurfaceDef.height));
            targetSurfaceDef.format = getHalToC2dFormat(mOutBufFormat);
            targetSurfaceDef.phys0 = targetPixels;
            targetSurfaceDef.phys1 = 0;
            LOG(DEBUG) <<"Inserting target surface id "<<targetSurfaceId<<"at memHandle="<< (uint64_t)memHandle;
            ret = c2dCreateSurface(&targetSurfaceId, C2D_TARGET, (C2D_SURFACE_TYPE) (C2D_SURFACE_RGB_HOST | C2D_SURFACE_WITH_PHYS), &targetSurfaceDef);
            if (ret != C2D_STATUS_OK) {
                c2dUnMapAddr(targetPixels);
                LOG(ERROR) <<"Target c2d_create API falied ret="<< ret;
                break;
            }
            mTgtSurfaceIdList.insert(std::pair<std::uint64_t, uint32_t>((uint64_t)memHandle, targetSurfaceId));
            mTgtGpuMapList.insert(std::pair<std::uint64_t, void *>((uint64_t)memHandle, targetPixels));
            munmap(mapped_buf, size);
        }

        mFramesAllowed++;
        added++;
    }

    return added;
}


unsigned EvsAISCamera::decreaseAvailableFrames_Locked(unsigned numToRemove) {
    // Acquire the graphics buffer allocator
    GraphicBufferAllocator &alloc(GraphicBufferAllocator::get());

    unsigned removed = 0;

    for (auto&& rec : mBuffers) {
        // Is this record not in use, but holding a buffer that we can free?
        if ((rec.inUse == false) && (rec.handle != nullptr)) {
            // Release buffer and update the record so we can recognize it as "empty"
            if (mC2dInitStatus && (mHalFrameFormat != mOutBufFormat)) {
                std::map<std::uint64_t, uint32_t>::iterator it;
                it = mTgtSurfaceIdList.find((uint64_t)rec.handle);
                if (it != mTgtSurfaceIdList.end()) {
                    c2dDestroySurface(it->second);
                    mTgtSurfaceIdList.erase((uint64_t)rec.handle);
                } else {
                    LOG(ERROR) <<"Error while traversing did not find handle="<<(uint64_t)rec.handle;
                }
                std::map<std::uint64_t, void *>::iterator itr;
                itr = mTgtGpuMapList.find((uint64_t)rec.handle);
                if (itr != mTgtGpuMapList.end()) {
                    c2dUnMapAddr(itr->second);
                    mTgtGpuMapList.erase((uint64_t)rec.handle);
                } else {
                    ALOGE("Error while traversing\n");
                }
            }
            alloc.free(rec.handle);
            rec.handle = nullptr;
            mFramesAllowed--;
            removed++;

            if (removed == numToRemove) {
                break;
            }
        }
    }

    return removed;
}


// This is the async callback from the video camera that tells us a frame is ready
void EvsAISCamera::forwardFrame(qcarcam_frame_info_t *qcarcamFrameInfo) {
    bool readyForFrame = false;
    size_t idx = 0;
    void *pData = NULL;

    if (!mpQcarcamMmapBufs[qcarcamFrameInfo->idx].ptr) {
        LOG(ERROR) <<"buffer is not mapped";
        return;
    }

    pData = (void *)mpQcarcamMmapBufs[qcarcamFrameInfo->idx].ptr;

    // Lock scope for updating shared state
    {
        std::lock_guard<std::mutex> lock(mAccessLock);

        // Are we allowed to issue another buffer?
        if (mFramesInUse >= mFramesAllowed) {
            // Can't do anything right now -- skip this frame
            LOG(WARNING) << "Skipped a frame because too many are in flight"
                <<" mFramesInUse="<<mFramesInUse<<" mFramesAllowed="<<mFramesAllowed;
        } else {
            if (!mDirectRendering) {
                // Identify an available buffer to fill
                for (idx = 0; idx < mBuffers.size(); idx++) {
                    if (!mBuffers[idx].inUse) {
                        if (mBuffers[idx].handle != nullptr) {
                            // Found an available record, so stop looking
                            break;
                        }
                    }
                }
            } else {
                // Identify filled buffer
                /* Get the memHandle corresponds to idx */
                std::map<std::uint32_t, std::uint64_t>::iterator it;
                it = mFdToMemHndlMapList.find((uint32_t)qcarcamFrameInfo->idx);
                if (it != mFdToMemHndlMapList.end()) {
                    // Identify buffer from list
                    for (idx = 0; idx < mBuffers.size(); idx++) {
                        if (it->second == (uint64_t)mBuffers[idx].handle)
                            break;
                    }
                } else
                    LOG(ERROR) <<"Error while traversing, did not find buf idx="<<qcarcamFrameInfo->idx;
            }
            if (idx >= mBuffers.size()) {
                // This shouldn't happen since we already checked mFramesInUse vs mFramesAllowed
                LOG(ERROR) << "Failed to find an available buffer slot";
            } else {
                // We're going to make the frame busy
                mBuffers[idx].inUse = true;
                mFramesInUse++;
                readyForFrame = true;
            }
        }
    }

    if (!readyForFrame) {
        // We need to return the vide buffer so it can capture a new frame
        LOG(DEBUG) << "Frame is not ready";
    } else {
        // retrive qcarcam mapped buffer
        if (!mpQcarcamMmapBufs[qcarcamFrameInfo->idx].ptr) {
            LOG(ERROR) << "buffer is not mapped";
            return;
        }

        // Assemble the buffer description we'll transmit below
        BufferDesc_1_1 bufDesc_1_1 = {};
        AHardwareBuffer_Desc* pDesc =
            reinterpret_cast<AHardwareBuffer_Desc *>(&bufDesc_1_1.buffer.description);
        pDesc->width  = mOutWidth;
        pDesc->height = mOutHeight;
        pDesc->layers = 1;
        pDesc->format = mOutBufFormat;
        pDesc->usage  = mUsage;
        pDesc->stride = mStride;
        bufDesc_1_1.buffer.nativeHandle = mBuffers[idx].handle;
        bufDesc_1_1.bufferId = idx;
        bufDesc_1_1.deviceId = mDescription.v1.cameraId;
        // timestamp in microseconds.
        bufDesc_1_1.timestamp = qcarcamFrameInfo->timestamp;

        // Fill Frame Meta Data of bufDesc_1_1 by reading from Sensor lib
#ifdef READ_FRAME_METADATA
        // Get extended info from qcarcam
        qcarcam_ret_t ret = QCARCAM_RET_OK;
        qcarcam_param_value_t param;

        param.vendor_param.data[0] = VENDOR_EXT_PROP_FRAME_META_DATA;

        ret = qcarcam_g_param(mQcarcamContext, QCARCAM_PARAM_VENDOR, &param);
        if (QCARCAM_RET_OK != ret) {
            LOG(ERROR) <<"qcarcam_g_param failed for ctxt" <<mQcarcamContext;
            //Continue
        }
        else {
            LOG(DEBUG) <<"qcarcam_g_param success for ctxt" <<mQcarcamContext;

            //Meta Data Size will be filled from SensorLib
            unsigned int metaDataSize = param.vendor_param.data[0];
            LOG(DEBUG) <<"Meta Data Size received from sensor lib = " <<metaDataSize;

            if(metaDataSize > ((QCARCAM_MAX_VENDOR_PAYLOAD_SIZE - 1) * sizeof(param.vendor_param.data[0]))){
                LOG(WARNING) <<"Meta Data Limit Exceeded";
                metaDataSize = (QCARCAM_MAX_VENDOR_PAYLOAD_SIZE - 1) * sizeof(param.vendor_param.data[0]);
            }

            bufDesc_1_1.metadata.resize(metaDataSize);
            uint8_t* param_data = (uint8_t *)&param.vendor_param.data[1];
            for (unsigned int i=0; i<metaDataSize; i++) {
                bufDesc_1_1.metadata[i] = param_data[i];
            }

        }
#endif

        // Lock our output buffer for writing
        // TODO(b/145459970): Sometimes, physical camera device maps a buffer
        // into the address that is about to be unmapped by another device; this
        // causes SEGV_MAPPER.
        void *targetPixels = nullptr;
        GraphicBufferMapper &mapper = GraphicBufferMapper::get();
        status_t result =
            mapper.lock(bufDesc_1_1.buffer.nativeHandle,
                        GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN,
                        android::Rect(ALIGN(pDesc->width, WIDTH_ALIGN), pDesc->height),
                        (void **)&targetPixels);

        // If we failed to lock the pixel buffer, we're about to crash, but log it first
        if (!targetPixels) {
            // TODO(b/145457727): When EvsHidlTest::CameraToDisplayRoundTrip
            // test case was repeatedly executed, EVS occasionally fails to map
            // a buffer.
            LOG(ERROR) << "Camera failed to gain access to image buffer for writing - "
                       << " status: " << statusToString(result)
                       << " , error: " << strerror(errno);
            mapper.unlock(bufDesc_1_1.buffer.nativeHandle);
            return;
        }

        // Transfer the video image into the output buffer, making any needed
        // format conversion along the way

        if (!mDirectRendering) {
            if ((!mC2dInitStatus) || (mHalFrameFormat == mOutBufFormat)) {
                //just do memcpy or cpu color conversion
                mFillBufferFromVideo(bufDesc_1_1, (uint8_t *)targetPixels, pData, mStride);
            } else {
                C2D_OBJECT c2dObject;
                uint32_t targetSurfaceId = 0;
                int ret = C2D_STATUS_OK;

                memset(&c2dObject, 0, sizeof(C2D_OBJECT));
                c2dObject.surface_id = mpQcarcamMmapBufs[qcarcamFrameInfo->idx].srcC2dSurfaceId;

                /* Get the C2D target Surface ID corresponds to memHandle */
                std::map<std::uint64_t, uint32_t>::iterator it;
                it = mTgtSurfaceIdList.find((uint64_t)mBuffers[idx].handle);

                if (it != mTgtSurfaceIdList.end()) {
                    targetSurfaceId = it->second;
                } else {
                    LOG(ERROR) <<"Error while traversing, did not find memHandle="<<(uint64_t)mBuffers[idx].handle;
                }
                ret = c2dDraw(targetSurfaceId, C2D_TARGET_ROTATE_0 , TARGET_SCISSOR, TARGET_MASK_ID, TARGET_COLOR_KEY, &c2dObject, NO_OF_OBJECTS);
                if (ret != C2D_STATUS_OK) {
                    LOG(ERROR) <<"C2DDraw failed with ret="<< ret;
                    return;
                }
                ret = c2dFinish(targetSurfaceId);
                if (ret != C2D_STATUS_OK) {
                    LOG(ERROR) <<"C2DFinish failed with ret="<< ret;
                    return;
                }
            }
        }
        // Unlock the output buffer
        mapper.unlock(bufDesc_1_1.buffer.nativeHandle);

        // Issue the (asynchronous) callback to the client -- can't be holding
        // the lock
        bool flag = false;
        if (mStream_1_1 != nullptr) {
            hidl_vec<BufferDesc_1_1> frames;
            frames.resize(1);
            frames[0] = bufDesc_1_1;
            auto result = mStream_1_1->deliverFrame_1_1(frames);
            flag = result.isOk();
        } else {
            BufferDesc_1_0 bufDesc_1_0 = {
                pDesc->width,
                pDesc->height,
                pDesc->stride,
                bufDesc_1_1.pixelSize,
                static_cast<uint32_t>(pDesc->format),
                static_cast<uint32_t>(pDesc->usage),
                bufDesc_1_1.bufferId,
                bufDesc_1_1.buffer.nativeHandle
            };

            auto result = mStream->deliverFrame(bufDesc_1_0);
            flag = result.isOk();
        }

        if (flag) {
            LOG(DEBUG) << "Delivered " << bufDesc_1_1.buffer.nativeHandle.getNativeHandle()
                       << " as id " << bufDesc_1_1.bufferId;
        } else {
            // This can happen if the client dies and is likely unrecoverable.
            // To avoid consuming resources generating failing calls, we stop sending
            // frames.  Note, however, that the stream remains in the "STREAMING" state
            // until cleaned up on the main thread.
            LOG(ERROR) << "Frame delivery call failed in the transport layer.";

            // Since we didn't actually deliver it, mark the frame as available
            std::lock_guard<std::mutex> lock(mAccessLock);
            mBuffers[idx].inUse = false;

            if (mFramesInUse > 0) {
                mFramesInUse--;
            } else {
                LOG(ERROR) << "Attempt to decrement, while frames not in use";
           }
        }
#ifdef FPS_PRINT
        uint64_t cur_msec = 0, diff_msec = 0;
        struct timeval current_time;
        gettimeofday(&current_time, NULL);
        if (0 == mFrameCnt)
            mPrevMsec = (current_time.tv_sec*1000) + (current_time.tv_usec/1000);
        mFrameCnt++;
        // Will calculate number of frames for 5 seconds
        uint32_t numFrames = (uint32_t)mAisFrameFPS * 5;
        if (numFrames == mFrameCnt) {
            gettimeofday(&current_time, NULL);
            cur_msec = (current_time.tv_sec*1000) + (current_time.tv_usec/1000);
            diff_msec = cur_msec - mPrevMsec;
            if (diff_msec > (uint64_t)5000) {
                double total_frame_rendered = diff_msec * (mAisFrameFPS/1000);
                double extra_frames = total_frame_rendered / (double)numFrames;
                double fps = mAisFrameFPS / extra_frames;
                LOG(ERROR) <<fps<<" FPS for stream id "<<mDescription.v1.cameraId;
            }
            else
                LOG(ERROR) <<mAisFrameFPS<<" FPS for Stream Id "<<mDescription.v1.cameraId;
            // Reset frame count
            mFrameCnt = 0;
            // Save current time
            mPrevMsec = cur_msec;
        }
#endif //FPS_PRINT
    }
}


bool EvsAISCamera::convertToqcarcamParamID(CameraParam id,
        qcarcam_param_t& qcarcamParamId,
        hidl_vec<int32_t>& values,
        qcarcam_param_value_t& param) {
    switch (id) {
        case CameraParam::BRIGHTNESS:
            qcarcamParamId = QCARCAM_PARAM_BRIGHTNESS;
            if (values.size())
                param.uint_value = values[0];
            break;
        case CameraParam::CONTRAST:
            qcarcamParamId = QCARCAM_PARAM_CONTRAST;
            if (values.size())
                param.uint_value = values[0];
            break;
        case CameraParam::AUTO_EXPOSURE:
            qcarcamParamId = QCARCAM_PARAM_EXPOSURE;
            param.exposure_config.exposure_mode_type = QCARCAM_EXPOSURE_AUTO;
            break;
        case CameraParam::ABSOLUTE_EXPOSURE:
            qcarcamParamId = QCARCAM_PARAM_EXPOSURE;
            param.exposure_config.exposure_mode_type = QCARCAM_EXPOSURE_MANUAL;
            if (values.size())
                param.exposure_config.exposure_time = values[0];
            break;
        case CameraParam::AUTO_WHITE_BALANCE:
        case CameraParam::WHITE_BALANCE_TEMPERATURE:
        case CameraParam::SHARPNESS:
        case CameraParam::AUTO_FOCUS:
        case CameraParam::ABSOLUTE_FOCUS:
        case CameraParam::ABSOLUTE_ZOOM:
        default:
            LOG(ERROR) <<"Camera parameter is unknown";
            return false;
    }
    return true;
}

void EvsAISCamera::convertFormqcarcamParamID(qcarcam_param_t& qcarcamParamId,
        hidl_vec<int32_t>& values,
        qcarcam_param_value_t& param) {
    switch (qcarcamParamId) {
        case QCARCAM_PARAM_BRIGHTNESS:
            if (values.size())
                values[0] = param.uint_value;
            break;
        case QCARCAM_PARAM_CONTRAST:
            if (values.size())
                values[0] = param.uint_value;
            break;
        case QCARCAM_PARAM_EXPOSURE:
            if (values.size())
                if (QCARCAM_EXPOSURE_MANUAL == param.exposure_config.exposure_mode_type)
                    values[0] = param.exposure_config.exposure_time;
            break;
        default:
            LOG(DEBUG) <<"Unsupported ignoring";
    }
}

sp<EvsAISCamera> EvsAISCamera::Create(const char *deviceName,
                                      void *m_pAisInputList,
                                      int numAisInputs,
                                      bool c2dInitStatus,
                                      EvsEnumerator *service) {
    unique_ptr<ConfigManager::CameraInfo> nullCamInfo = nullptr;
    return Create(deviceName, m_pAisInputList, numAisInputs, c2dInitStatus, nullCamInfo, service);
}


sp<EvsAISCamera> EvsAISCamera::Create(const char *deviceName,
                                      void *AisInputList,
                                      int numAisInputs,
                                      bool c2dInitStatus,
                                      unique_ptr<ConfigManager::CameraInfo> &camInfo,
                                      EvsEnumerator *service,
                                      const Stream *requestedStreamCfg) {
    LOG(INFO) << "Create Device ID " << deviceName;
    std::string qcarcamId = service->deviceIdToQcarcamId(deviceName);
    int camera_id, stream_id = -1, i;
    qcarcam_input_t *m_pAisInputList = (qcarcam_input_t*)AisInputList;
    sscanf(qcarcamId.c_str(), "%d", &camera_id);
    sp<EvsAISCamera> evsCamera = new EvsAISCamera(deviceName, camInfo, service);
    if (evsCamera == nullptr) {
        return nullptr;
    }
    evsCamera->mC2dInitStatus = c2dInitStatus;

    for (int i = 0; i < numAisInputs; i++) {
        if (m_pAisInputList[i].desc == camera_id) {
            //set evsCamera paramaters from qcarcam paramters
            evsCamera->mAisFrameWidth = m_pAisInputList[i].res[0].width;
            evsCamera->mAisFrameHeight = m_pAisInputList[i].res[0].height;
            evsCamera->mAisFrameFormat = m_pAisInputList[i].color_fmt[0];
            evsCamera->mHalFrameFormat = EvsAISCamera::getQcarcamToHalFormat(evsCamera->mAisFrameFormat);
            evsCamera->mAisFrameFPS = m_pAisInputList[i].res[0].fps;
        }
        if (i == numAisInputs) {
            LOG(ERROR) << "Given camera id "<<camera_id<<" does not match with enumarated cameras exiting";
            return nullptr;
        }
    }

    // Validate a given stream configuration.
    if (camInfo != nullptr && requestedStreamCfg != nullptr) {
        for (auto& [id, cfg] : camInfo->streamConfigurations) {
            // RawConfiguration has id, width, height, format, direction, and
            // fps.
            if (cfg[3] == static_cast<uint32_t>(requestedStreamCfg->format)||
                    HAL_PIXEL_FORMAT_RGBA_8888 == static_cast<int32_t>(requestedStreamCfg->format)) {
                if (cfg[1] == requestedStreamCfg->width &&
                        cfg[2] == requestedStreamCfg->height) {
                    // Find exact match, update width, height and format.
                    stream_id = cfg[0];
                    evsCamera->mAisFrameWidth = requestedStreamCfg->width;
                    evsCamera->mAisFrameHeight = requestedStreamCfg->height;
                    evsCamera->mOutBufFormat = static_cast<int32_t>(requestedStreamCfg->format);
                    LOG(ERROR) << "Stream config is set from input stream config";
                    break;
                }
            }
        }
        if (-1 == stream_id) {
            LOG(ERROR) << "requestedStreamCfg does not match with existing config, taking qcarcam query input values";
            LOG(ERROR) << "requestedStreamCfg->width = " << requestedStreamCfg->width <<endl
                <<"requestedStreamCfg->height = " << requestedStreamCfg->height <<endl
                <<"requestedStreamCfg->format = " << printHalFormat(static_cast<uint32_t>(requestedStreamCfg->format));
        }
    } else if (camInfo != nullptr) {
        //Obtain valid streamId from CameraInfo of ConfigManager
        for (auto& [id, cfg] : camInfo->streamConfigurations) {
            if (cfg[0] == camera_id) {
                // Find exact match, update width, height and format.
                stream_id = cfg[0];
                evsCamera->mAisFrameWidth = camInfo->streamConfigurations[stream_id][1];
                evsCamera->mAisFrameHeight = camInfo->streamConfigurations[stream_id][2];
                evsCamera->mOutBufFormat = cfg[3];
                LOG(ERROR) << "Stream config is set from xml stream config";
                break;
            }
        }
        if (-1 == stream_id) {
            LOG(ERROR) << "Given stram config does not match with existing config";
            return nullptr;
        }
    } else
        LOG(ERROR) << "Stream config is not given taking qcarcam query input values";

    // Initialize qcarcam camera
    evsCamera->mQcarcamContext = qcarcam_open((qcarcam_input_desc_t)camera_id);
    if (evsCamera->mQcarcamContext == 0) {
        LOG(ERROR) << "qcarcam_open() failed";
        return nullptr;
    }
    else {
        LOG(DEBUG) << "qcarcam_open() success for stream "<<camera_id
            <<" mQcarcamContext = "<<evsCamera->mQcarcamContext;
        mMultiCamQLock.lock();
        // Find an empty slot and insert handle
        for (i=0; i<MAX_AIS_CAMERAS; i++) {
            if (NOT_IN_USE == sQcarcamCameraArray[i].gQcarcamContext)
                break;
        }
        if (i < MAX_AIS_CAMERAS) {
            sQcarcamCameraArray[i].gQcarcamContext = evsCamera->mQcarcamContext;
            evsCamera->pCurrentRecord = &sQcarcamCameraArray[i];
            mMultiCamQLock.unlock();
        } else {
            mMultiCamQLock.unlock();
            LOG(ERROR) << "Max number of streams reached!!";
            if (evsCamera->mQcarcamContext)
                (void)qcarcam_close(evsCamera->mQcarcamContext);
            return nullptr;
        }
        mMultiCamQLock.unlock();
    }
    evsCamera->mRunMode = STOPPED;

    //Handling for bayer camera
    if ((QCARCAM_FMT_MIPIRAW_8 == evsCamera->mAisFrameFormat) ||
            (QCARCAM_FMT_MIPIRAW_10 == evsCamera->mAisFrameFormat) ||
            (QCARCAM_FMT_MIPIRAW_12 == evsCamera->mAisFrameFormat) ||
            (QCARCAM_FMT_MIPIRAW_14 == evsCamera->mAisFrameFormat) ||
            (QCARCAM_FMT_MIPIRAW_16 == evsCamera->mAisFrameFormat) ||
            (QCARCAM_FMT_MIPIRAW_20 == evsCamera->mAisFrameFormat)) {
        qcarcam_param_value_t param;
        param.uint_value = QCARCAM_OPMODE_SHDR; //Input operation mode
        evsCamera->mAisFrameFormat = QCARCAM_FMT_NV12; // PProc from Raw12 to NV12 format.
        evsCamera->mHalFrameFormat = EvsAISCamera::getQcarcamToHalFormat(evsCamera->mAisFrameFormat);
        evsCamera->mAisFrameFPS = m_pAisInputList[i].res[0].fps;
        if (qcarcam_s_param(evsCamera->mQcarcamContext, QCARCAM_PARAM_OPMODE, &param))
        {
            LOG(ERROR) << "qcarcam_s_param(QCARCAM_PARAM_OPMODE) failed";
            if (evsCamera->mQcarcamContext)
                (void)qcarcam_close(evsCamera->mQcarcamContext);
            return nullptr;
        }
        // Mark hal format as RGBA8888 as we are getting bayer data as RGBA
        evsCamera->mHalFrameFormat = HAL_PIXEL_FORMAT_RGBA_8888;
    }
    //TODO::Add import buffer and color format handling,
    //use QCARCAM_OPMODE_TRANSFORMER to get RGB buffers from qcarcam

    // Please note that the buffer usage flag does not come from a given stream
    // configuration.
    evsCamera->mUsage  = GRALLOC_USAGE_HW_TEXTURE     |
        GRALLOC_USAGE_SW_READ_OFTEN |
        GRALLOC_USAGE_SW_WRITE_OFTEN;

    LOG(ERROR) << "evsCamera->mAisFrameWidth = " << evsCamera->mAisFrameWidth <<endl
        <<"evsCamera->mAisFrameHeight = " << evsCamera->mAisFrameHeight <<endl
        <<"evsCamera->mAisFrameFormat = 0x" << std::hex <<evsCamera->mAisFrameFormat <<endl
        <<"evsCamera->mHalFrameFormat = " << printHalFormat(evsCamera->mHalFrameFormat) <<endl
        <<"evsCamera->mOutBufFormat = " << printHalFormat(evsCamera->mOutBufFormat);

    if (evsCamera->mHalFrameFormat != evsCamera->mOutBufFormat) {
        LOG(ERROR) << "Frame format does not match with Output buffer format"<<endl
            <<"color conversion needed from "<<printHalFormat(evsCamera->mHalFrameFormat)<<" to "
            <<printHalFormat(evsCamera->mOutBufFormat);
    }

    return evsCamera;
}

/**
 * Qcarcam event callback function
 * @param hndl
 * @param event_id
 * @param p_payload
 */
void EvsAISCamera::evs_ais_event_cb(qcarcam_hndl_t hndl, qcarcam_event_t event_id, qcarcam_event_payload_t *p_payload)
{
    int q_size = 0;
    switch (event_id)
    {
        case QCARCAM_EVENT_FRAME_READY:
            //ALOGI("received QCARCAM_EVENT_FRAME_READY");
            break;
        default:
            LOG(ERROR) << "received unsupported event "<< event_id;
            break;
    }

    //TODO::Protect with semaphore
    // This stream should be already in Queue
    mMultiCamQLock.lock();
    gQcarCamClientData *pRecord = findEventQByHndl(hndl);
    if (!pRecord) {
            LOG(ERROR) << "ERROR::Requested qcarcam_hndl "<<hndl<<" not found";
            mMultiCamQLock.unlock();
            return;
    }
    else {
        //ALOGI("Found qcarcam_hndl in the list");
    }
    mMultiCamQLock.unlock();

    // Push the event into queue
    if (pRecord)
    {
        std::unique_lock<std::mutex> lk(pRecord->gEventQLock);
        q_size = pRecord->sQcarcamList.size();
        if (q_size < MAX_EVENT_Q_SIZE)
        {
            // allocate and copy payload
            qcarcam_event_payload_t *payload =
                (qcarcam_event_payload_t  *)malloc(sizeof(qcarcam_event_payload_t));
            if (!payload)
            {
                LOG(ERROR) << "payload is null";
                lk.unlock();
                return;
            }
            memcpy(payload, p_payload, sizeof(qcarcam_event_payload_t));
            pRecord->sQcarcamList.emplace_back(event_id, payload);
            //ALOGI("Pushed event id %d to Event Queue", (int)event_id);
        }
        else {
            LOG(ERROR) << "Max events are Queued in event Q";
        }
        lk.unlock();
        pRecord->gCV.notify_one();
    }
}

gQcarCamClientData* EvsAISCamera::findEventQByHndl(const qcarcam_hndl_t qcarcam_hndl) {
    // Find the named camera
    int i = 0;
    for (i=0; i<MAX_AIS_CAMERAS; i++) {
        if (sQcarcamCameraArray[i].gQcarcamContext == qcarcam_hndl) {
            // Found a match!
            return &sQcarcamCameraArray[i];
        }
    }
    LOG(ERROR) << "ERROR gQcarcamContext = "<< qcarcam_hndl <<" did not match!!";

    // We didn't find a match
    return nullptr;
}

// This runs on a background thread to receive and dispatch qcarcam frames
void EvsAISCamera::collectEvents() {
    int q_size = 0;
    LOG(DEBUG) << "collectEvents thread running";

    if (!this->pCurrentRecord) {
        LOG(ERROR) << "ERROR::Requested qcarcam_hndl not found in the list";
        LOG(ERROR) << "VideoCapture thread ending";
        mRunMode = STOPPED;
        return;
    }
    // Run until our atomic signal is cleared
    while (mRunMode == RUN) {
        // Wait for a buffer to be ready
        std::unique_lock<std::mutex> lk(this->pCurrentRecord->gEventQLock);

        // fetch events from event Q
        q_size = this->pCurrentRecord->sQcarcamList.size();
        if (q_size) {
            //ALOGI("Found %d events in the Event Queue", q_size);
            QEventDesc eventData = this->pCurrentRecord->sQcarcamList.front();
            this->pCurrentRecord->sQcarcamList.pop_front();
            lk.unlock();
            handleEvents(eventData);
        } else {
            /**
            * Wait for a buffer to be ready
            * 1. In normal scenario, ais_server will feed the pCurrentRecord->sQcarcamList in ~30ms.
            * 2. If qcarcam_stop() has already been called, ais_server will not feed the QcarcamList.
            *  We will wait for the stop signal here or exit after 200 ms (timeout).
            */
            int delay_ms = 200;
            if (std::cv_status::timeout == this->pCurrentRecord->gCV.wait_for(lk, std::chrono::milliseconds(delay_ms))) {
               LOG(ERROR) <<"ERROR::pCurrentRecord->gCV.wait_for() timeout in "<<delay_ms<<"ms";
            }

            if (mRunMode == STOPPED) {
                LOG(INFO) <<"Exiting eventDispatcherThread as user has called stop";
                lk.unlock();
                break;
            }
            // fetch events from event Q
            q_size = this->pCurrentRecord->sQcarcamList.size();
            if (q_size) {
                //ALOGI("Found %d events in the Event Queue", q_size);
                QEventDesc eventData = this->pCurrentRecord->sQcarcamList.front();
                this->pCurrentRecord->sQcarcamList.pop_front();
                lk.unlock();
                handleEvents(eventData);
            } else {
                //ALOGI("Event Q is empty");
                lk.unlock();
            }
        }
    }

    // Mark ourselves stopped
    LOG(ERROR) << "CollectEvents thread ending";
    mRunMode = STOPPED;
}

void EvsAISCamera::handleEvents(QEventDesc &events) {
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    EvsEventDesc event;
    auto result = Void();

    switch(events.event_id) {
        case QCARCAM_EVENT_FRAME_READY:
            ret = qcarcam_get_frame(mQcarcamContext, &mQcarcamFrameInfo, QCARCAM_DEFAULT_GET_FRAME_TIMEOUT, 0);
            if (QCARCAM_RET_OK != ret) {
                LOG(ERROR) << "qcarcam_get_frame failed "<<mQcarcamContext<<" ret "<< ret;
            } else {
                //ALOGI("Fetched new frame from AIS");
#ifdef ENABLE_FRAME_DUMP
                dumpqcarcamFrame(&mQcarcamFrameInfo);
#endif
                forwardFrame(&mQcarcamFrameInfo);
                if (!mDirectRendering) {
                    // We can release the buffer here because we did memcpy to app
                    // RGBA8888 buffer
                    ret = qcarcam_release_frame(mQcarcamContext, mQcarcamFrameInfo.idx);
                    if (QCARCAM_RET_OK != ret)
                        LOG(ERROR) << "qcarcam_release_frame()"<< mQcarcamFrameInfo.idx << " failed";
                }
            }
            break;
        case QCARCAM_EVENT_ERROR:
            LOG(ERROR) << "QCARCAM_EVENT_ERROR received error type = "<<events.p_payload->uint_payload;
            if (events.p_payload->uint_payload == QCARCAM_CONN_ERROR) {
                LOG(ERROR) << "Connection to the server got lost or time out";
                event.aType = EvsEventType::STREAM_ERROR;
                event.payload.push_back(events.event_id);

                if (!(mStream->notify(event)).isOk()) {
                    LOG(ERROR) << "Error delivering a event error";
                }
            } else {
            // Restart error stream
            ret = qcarcam_stop(mQcarcamContext);
            if (QCARCAM_RET_OK != ret)
                LOG(ERROR) << "qcarcam_stop() failed for ctxt "<<mQcarcamContext<<" ret "<< ret;
            ret = qcarcam_start(mQcarcamContext);
            if (QCARCAM_RET_OK != ret)
                LOG(ERROR) << "qcarcam_start() failed for ctxt "<<mQcarcamContext<<" ret "<< ret;
            }
            break;
        case QCARCAM_EVENT_RECOVERY:
            LOG(ERROR) << "Received a QCARCAM_EVENT_RECOVERY event";

            event.aType = EvsEventType::STREAM_ERROR;
            event.payload[0] = events.event_id;
            result = mStream_1_1->notify(event);
            if (!result.isOk()) {
                LOG(ERROR) << "Error delivering a recovery event";
            }
            break;
        case QCARCAM_EVENT_RECOVERY_SUCCESS:
            LOG(ERROR) << "Received a QCARCAM_EVENT_RECOVERY_SUCCESS event";

            event.aType = EvsEventType::STREAM_ERROR;
            event.payload[0] = events.event_id;
            result = mStream_1_1->notify(event);
            if (!result.isOk()) {
                LOG(ERROR) << "Error delivering a recovery success event";
            }
            break;
        case QCARCAM_EVENT_FRAME_DROP:
            LOG(ERROR) << "Received a Frame drop event";

            event.aType = EvsEventType::FRAME_DROPPED;
            result = mStream_1_1->notify(event);
            if (!result.isOk()) {
                LOG(ERROR) << "Error delivering a frame drop event";
            }
            break;
        case QCARCAM_EVENT_VENDOR:
            LOG(ERROR) << "Received a QCARCAM_EVENT_VENDOR event";

            event.aType = EvsEventType::STREAM_ERROR;
            event.payload[0] = events.p_payload->uint_payload;
            result = mStream_1_1->notify(event);
            if (!result.isOk()) {
                LOG(ERROR) << "Error delivering a recovery success event";
            }
            break;
        default:
            LOG(ERROR) << "Unsupported event "<<events.event_id;
    }
    // free payload data
    if (events.p_payload) {
        free(events.p_payload);
        events.p_payload = nullptr;
    }
}

void EvsAISCamera::dumpqcarcamFrame(qcarcam_frame_info_t *pQcarcamFrameInfo)
{
    FILE *fp;
    size_t numBytesWritten = 0;
    size_t numByteToWrite = 0;
    unsigned char *pBuf = 0;
    static int frame_cnt = 0;
    char filename[128];

    if (frame_cnt > 5)
        return;

    snprintf(filename, sizeof(filename), "%s_%d","/data/vendor/camera/evs_frame",frame_cnt);

    if (NULL == pQcarcamFrameInfo)
    {
        LOG(ERROR) << "Enpty frame info";
        return;
    }

    if (!mpQcarcamMmapBufs[pQcarcamFrameInfo->idx].ptr)
    {
        LOG(ERROR) << "buffer is not mapped";
        return;
    }

    fp = fopen(filename, "w+");

    LOG(ERROR) << "dumping qcarcam frame "<< filename <<" numbytes "
        << mpQcarcamMmapBufs[pQcarcamFrameInfo->idx].size;

    if (0 != fp)
    {
        pBuf = (unsigned char *)mpQcarcamMmapBufs[pQcarcamFrameInfo->idx].ptr;
        numByteToWrite = mpQcarcamMmapBufs[pQcarcamFrameInfo->idx].size;

        numBytesWritten = fwrite(pBuf, 1, numByteToWrite, fp);

        if (numBytesWritten != numByteToWrite)
        {
            LOG(ERROR) << "error no data written to file";
        }
        fclose(fp);
    }
    else
    {
        LOG(ERROR) << "failed to open file";
        return;
    }
    frame_cnt++;
}

uint32_t EvsAISCamera::getQcarcamStride(uint32_t width, uint32_t mFormat)
{
    uint32_t stride_val = 1;
    switch(mFormat)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
            stride_val = width * 4;
            break;
        case HAL_PIXEL_FORMAT_CbYCrY_422_I:
        case HAL_PIXEL_FORMAT_YCBCR_422_I:
            stride_val = width * 2;
            break;
        case HAL_PIXEL_FORMAT_RGB_888:
            stride_val = width * 3;
            break;
        case HAL_PIXEL_FORMAT_RAW8:
            stride_val = width;
            break;
        case HAL_PIXEL_FORMAT_RAW10:
            stride_val = width * 5 / 4;
            break;
        case HAL_PIXEL_FORMAT_RAW12:
            stride_val = width * 3 / 2;
            break;
        case HAL_PIXEL_FORMAT_YCbCr_420_SP:
            stride_val = width;
            break;
        default:
            stride_val = width * 2;
    }
    return stride_val;
}

qcarcam_color_fmt_t EvsAISCamera::getHalToQcarcamFormat(uint32_t halFormat) {
    switch(halFormat) {
        case HAL_PIXEL_FORMAT_CbYCrY_422_I:
            return QCARCAM_FMT_UYVY_8;
        case HAL_PIXEL_FORMAT_YCBCR_422_I:
            return QCARCAM_FMT_YUYV_8;
        case HAL_PIXEL_FORMAT_RGB_888:
            return QCARCAM_FMT_RGB_888;
        case HAL_PIXEL_FORMAT_RAW8:
            return QCARCAM_FMT_MIPIRAW_8;
        case HAL_PIXEL_FORMAT_RAW10:
            return QCARCAM_FMT_MIPIRAW_10;
        case HAL_PIXEL_FORMAT_RAW12:
            return QCARCAM_FMT_MIPIRAW_12;
        default: LOG(ERROR) << "Unsupported HAL format 0x"<<std::hex<<halFormat;
                 return QCARCAM_FMT_UYVY_8;
    }
}

uint32_t EvsAISCamera::getQcarcamToHalFormat(qcarcam_color_fmt_t qcarcamFormat)
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
        case QCARCAM_FMT_NV12:
            return HAL_PIXEL_FORMAT_YCbCr_420_SP;
        default: LOG(ERROR) << "Unsupported HAL format 0x"<<std::hex<<qcarcamFormat;
            return -1;
    }
}

C2D_FORMAT_MODE EvsAISCamera::getC2dColorSpaceFormat(uint32_t colorSpace) {
    switch(colorSpace) {
        case QCARCAM_COLOR_SPACE_BT601:
            return C2D_FORMAT_BT601_LIMITEDRANGE;
        case QCARCAM_COLOR_SPACE_BT601_FULL:
            return C2D_FORMAT_BT601_FULLRANGE;
        case QCARCAM_COLOR_SPACE_BT709:
            return C2D_FORMAT_BT709_LIMITEDRANGE;
        case QCARCAM_COLOR_SPACE_BT709_FULL:
            return C2D_FORMAT_BT709_FULLRANGE;
        case QCARCAM_COLOR_SPACE_UNCORRECTED:
        case QCARCAM_COLOR_SPACE_SRGB:
        case QCARCAM_COLOR_SPACE_LRGB:
            return C2D_FORMAT_BT601_LIMITEDRANGE;
        default: LOG(ERROR) <<"Unsupported HAL format, default set to C2D_COLOR_FORMAT_422_UYVY";
                 return C2D_FORMAT_BT601_LIMITEDRANGE;
    }
}

/* @Breif: Function to convert hal to c2d format.
 * @Params: hal format
 * @return: c2d format
 */
uint32_t EvsAISCamera::getHalToC2dFormat(uint32_t halFormat) {
    switch(halFormat) {
        case HAL_PIXEL_FORMAT_CbYCrY_422_I:
            return C2D_COLOR_FORMAT_422_UYVY;
        case HAL_PIXEL_FORMAT_YCBCR_422_I:
            return C2D_COLOR_FORMAT_422_YUYV;
        case HAL_PIXEL_FORMAT_RGB_888:
            return C2D_COLOR_FORMAT_888_RGB | C2D_FORMAT_SWAP_RB;
        case HAL_PIXEL_FORMAT_RGBA_8888:
            return C2D_COLOR_FORMAT_8888_ARGB | C2D_FORMAT_SWAP_RB;
        case HAL_PIXEL_FORMAT_YCbCr_420_SP: // 0x109, NV12
            return C2D_COLOR_FORMAT_420_NV12;
        default: LOG(ERROR) <<"Unsupported HAL format, default set to C2D_COLOR_FORMAT_422_UYVY";
                 return C2D_COLOR_FORMAT_422_UYVY;
    }
}

bool EvsAISCamera::allocQcarcamInternalBuffers(uint32_t count) {

    mQcarcamOutBufs.n_buffers = count;
    mQcarcamOutBufs.color_fmt = mAisFrameFormat;

    mQcarcamOutBufs.buffers = (qcarcam_buffer_t *)calloc(mQcarcamOutBufs.n_buffers, sizeof(*mQcarcamOutBufs.buffers));
    if (mQcarcamOutBufs.buffers == 0) {
        LOG(ERROR) << "alloc qcarcam_buffer failed";
        return false;
    }
    mpGfxBufs = (sp<GraphicBuffer>*)calloc(mQcarcamOutBufs.n_buffers, sizeof(sp<GraphicBuffer>));
    if (0 == mpGfxBufs) {
        LOG(ERROR) << "alloc mpGfxBufs failed";
        free(mQcarcamOutBufs.buffers);
        return false;
    }
    mpQcarcamMmapBufs = (qcarcam_mapped_buffer_t*)calloc(mQcarcamOutBufs.n_buffers,
            sizeof(qcarcam_mapped_buffer_t));
    if (0 == mpQcarcamMmapBufs) {
        LOG(ERROR) << "alloc mpQcarcamMmapBufs failed";
        free(mQcarcamOutBufs.buffers);
        free(mpGfxBufs);
        return false;
    }

    const int graphicsUsage  = android::GraphicBuffer::USAGE_HW_TEXTURE |
        android::GraphicBuffer::USAGE_SW_WRITE_OFTEN |
        android::GraphicBuffer::USAGE_SW_READ_OFTEN;
    const int nativeBufferFormat = mHalFrameFormat;

    qcarcam_param_value_t param;
    if (0 != qcarcam_g_param(mQcarcamContext, QCARCAM_PARAM_INPUT_COLOR_SPACE, &param))
    {
        LOG(ERROR) << "Failed to get INPUT_COLOR_SPACE format";
        // Assign default color space format
        param.color_space = QCARCAM_COLOR_SPACE_BT601;
    }

    C2D_FORMAT_MODE c2d_color_space = getC2dColorSpaceFormat(param.color_space);
    LOG(ERROR) << " C2D Color space format = " << (uint32_t)c2d_color_space;

    for (int i = 0; i < (int)mQcarcamOutBufs.n_buffers; ++i) {
        mQcarcamOutBufs.buffers[i].n_planes = 1;
        mQcarcamOutBufs.buffers[i].planes[0].width = ALIGN(mAisFrameWidth, WIDTH_ALIGN);
        mQcarcamOutBufs.buffers[i].planes[0].height = mAisFrameHeight;
        mQcarcamOutBufs.buffers[i].planes[0].stride =
            EvsAISCamera::getQcarcamStride(ALIGN(mAisFrameWidth, WIDTH_ALIGN), mHalFrameFormat);
        mQcarcamOutBufs.buffers[i].planes[0].size = mQcarcamOutBufs.buffers[i].planes[0].height *
            mQcarcamOutBufs.buffers[i].planes[0].stride;

        LOG(DEBUG) <<"w="<< mQcarcamOutBufs.buffers[i].planes[0].width
                <<" h="<< mQcarcamOutBufs.buffers[i].planes[0].height<<endl
                <<"stride="<< mQcarcamOutBufs.buffers[i].planes[0].stride
                <<" size="<< mQcarcamOutBufs.buffers[i].planes[0].size;

        mpGfxBufs[i] = NULL;
        mpGfxBufs[i] = new GraphicBuffer(ALIGN(mQcarcamOutBufs.buffers[i].planes[0].width, WIDTH_ALIGN),
                mQcarcamOutBufs.buffers[i].planes[0].height,
                nativeBufferFormat,
                graphicsUsage);

        int fd = get_handle_fd(mpGfxBufs[i]->getNativeBuffer()->handle);
        mQcarcamOutBufs.buffers[i].planes[0].p_buf = (void*)(uintptr_t)(fd);

        mpQcarcamMmapBufs[i].size = mQcarcamOutBufs.buffers[i].planes[0].size;
        mpQcarcamMmapBufs[i].ptr = mmap(NULL, mpQcarcamMmapBufs[i].size,
                        PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (!mpQcarcamMmapBufs[i].ptr)
            LOG(ERROR) << "mmap failed for "<<i;

        if (mC2dInitStatus && (mHalFrameFormat != mOutBufFormat)) {
            C2D_STATUS c2d_status;
            C2D_YUV_SURFACE_DEF srcSurfaceDef;
            void *c2dsrcGpuAddr = NULL;

            c2d_status = c2dMapAddr(fd, mpQcarcamMmapBufs[i].ptr,
                    mpQcarcamMmapBufs[i].size, 0,
                    KGSL_USER_MEM_TYPE_ION, &c2dsrcGpuAddr);
            if (c2d_status != C2D_STATUS_OK) {
                LOG(ERROR) << "c2dMapAddr failed for id "<<i<<" status "<<c2d_status;
                free(mQcarcamOutBufs.buffers);
                free(mpGfxBufs);
                return false;
            }

            memset(&srcSurfaceDef, 0, sizeof(srcSurfaceDef));
            srcSurfaceDef.width = mQcarcamOutBufs.buffers[i].planes[0].width;
            srcSurfaceDef.height = mQcarcamOutBufs.buffers[i].planes[0].height;
            srcSurfaceDef.stride0 = mQcarcamOutBufs.buffers[i].planes[0].stride;
            srcSurfaceDef.stride1 = mQcarcamOutBufs.buffers[i].planes[0].stride;
            srcSurfaceDef.format = getHalToC2dFormat(mHalFrameFormat) | (uint32_t)c2d_color_space;
            srcSurfaceDef.plane0 = mpQcarcamMmapBufs[i].ptr; //Creating surface with one buffer later we update it in update_surface.
            srcSurfaceDef.plane1 = (void *) ((unsigned char *)srcSurfaceDef.plane0 + (srcSurfaceDef.width * srcSurfaceDef.height));
            srcSurfaceDef.phys0 = c2dsrcGpuAddr;
            srcSurfaceDef.phys1 = (void *) ((unsigned char *)c2dsrcGpuAddr + (srcSurfaceDef.width * srcSurfaceDef.height));

            c2d_status = c2dCreateSurface(&mpQcarcamMmapBufs[i].srcC2dSurfaceId,
                    C2D_SOURCE, (C2D_SURFACE_TYPE) ((((srcSurfaceDef.format & C2D_COLOR_FORMAT_888_RGB)  == C2D_COLOR_FORMAT_888_RGB) ? C2D_SURFACE_RGB_HOST : C2D_SURFACE_YUV_HOST) | C2D_SURFACE_WITH_PHYS), &srcSurfaceDef);
            if (c2d_status != C2D_STATUS_OK) {
                c2dUnMapAddr(c2dsrcGpuAddr);
                LOG(ERROR) << "Source c2d_create API falied ret= "<<c2d_status;
                free(mQcarcamOutBufs.buffers);
                free(mpGfxBufs);
                return false;
            }
            mSrcGpuMapList.insert(std::pair<std::uint64_t, void *>((uint64_t)mpQcarcamMmapBufs[i].ptr, c2dsrcGpuAddr));
        }
    }
    return true;
}


bool EvsAISCamera::setExternalBuffersAsInteranl(vector<int>& extFds) {
    mQcarcamOutBufs.n_buffers = extFds.size();
    mQcarcamOutBufs.color_fmt = mAisFrameFormat;

    mQcarcamOutBufs.buffers = (qcarcam_buffer_t *)calloc(mQcarcamOutBufs.n_buffers, sizeof(*mQcarcamOutBufs.buffers));
    if (mQcarcamOutBufs.buffers == 0) {
        LOG(ERROR) << "alloc qcarcam_buffer failed";
        return false;
    }
    mpQcarcamMmapBufs = (qcarcam_mapped_buffer_t*)calloc(mQcarcamOutBufs.n_buffers,
            sizeof(qcarcam_mapped_buffer_t));
    if (0 == mpQcarcamMmapBufs) {
        LOG(ERROR) << "alloc mpQcarcamMmapBufs failed";
        free(mQcarcamOutBufs.buffers);
        return false;
    }

    for (int i = 0; i < (int)mQcarcamOutBufs.n_buffers; ++i) {
        mQcarcamOutBufs.buffers[i].n_planes = 1;
        mQcarcamOutBufs.buffers[i].planes[0].width = ALIGN(mAisFrameWidth, WIDTH_ALIGN);
        mQcarcamOutBufs.buffers[i].planes[0].height = mAisFrameHeight;
        mQcarcamOutBufs.buffers[i].planes[0].stride =
            EvsAISCamera::getQcarcamStride(ALIGN(mAisFrameWidth, WIDTH_ALIGN), mHalFrameFormat);
        mQcarcamOutBufs.buffers[i].planes[0].size = mQcarcamOutBufs.buffers[i].planes[0].height *
            mQcarcamOutBufs.buffers[i].planes[0].stride;

        LOG(DEBUG) <<"w="<< mQcarcamOutBufs.buffers[i].planes[0].width
            <<" h="<< mQcarcamOutBufs.buffers[i].planes[0].height<<endl
            <<"stride="<< mQcarcamOutBufs.buffers[i].planes[0].stride
            <<" size="<< mQcarcamOutBufs.buffers[i].planes[0].size;

        // Assign external FD to qcarcam buffer
        mQcarcamOutBufs.buffers[i].planes[0].p_buf = (void*)(uintptr_t)(extFds[i]);

        // Map external buffer for future use
        mpQcarcamMmapBufs[i].size = mQcarcamOutBufs.buffers[i].planes[0].size;
        mpQcarcamMmapBufs[i].ptr = mmap(NULL, mpQcarcamMmapBufs[i].size,
                PROT_READ | PROT_WRITE, MAP_SHARED, extFds[i], 0);
        if (!mpQcarcamMmapBufs[i].ptr)
            LOG(ERROR) << "mmap failed for "<<i;
    }
    return true;
}

/* @Breif: Function to print hal format.
 * @Params: hal format
 * @return: void
 */
static char const* printHalFormat(uint32_t halFormat) {
    switch(halFormat) {
        case HAL_PIXEL_FORMAT_CbYCrY_422_I:
            return "HAL_PIXEL_FORMAT_CbYCrY_422_I";
        case HAL_PIXEL_FORMAT_YCBCR_422_I:
            return "HAL_PIXEL_FORMAT_YCBCR_422_I";
        case HAL_PIXEL_FORMAT_RGB_888:
            return "HAL_PIXEL_FORMAT_RGB_888";
        case HAL_PIXEL_FORMAT_RGBA_8888:
            return "HAL_PIXEL_FORMAT_RGBA_8888";
        case HAL_PIXEL_FORMAT_RAW8:
            return "HAL_PIXEL_FORMAT_RAW8";
        case HAL_PIXEL_FORMAT_RAW10:
            return "HAL_PIXEL_FORMAT_RAW10";
        case HAL_PIXEL_FORMAT_RAW12:
            return "HAL_PIXEL_FORMAT_RAW12";
        case HAL_PIXEL_FORMAT_YCbCr_420_SP:   // 0x109, NV12
            return "HAL_PIXEL_FORMAT_YCbCr_420_SP";
        default: LOG(ERROR) << "Unsupported HAL format=0x"<<std::hex<<halFormat;
    }
    return "Unsupported HAL format";
}
} // namespace implementation
} // namespace V1_1
} // namespace evs
} // namespace automotive
} // namespace hardware
} // namespace android
