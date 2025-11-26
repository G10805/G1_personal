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
#include "bufferCopy.h"
#include "EvsEnumerator.h"

#include <aidl/android/hardware/graphics/common/HardwareBufferDescription.h>
#include <aidlcommonsupport/NativeHandle.h>
#include <android-base/logging.h>
#include <ui/GraphicBufferAllocator.h>
#include <ui/GraphicBufferMapper.h>
#include <utils/SystemClock.h>
#include <sys/mman.h> //for PROT_READ | PROT_WRITE, MAP_SHARED
#include <cutils/properties.h>

#include <android/hardware/graphics/mapper/4.0/IMapper.h>
using Error_v4       = ::android::hardware::graphics::mapper::V4_0::Error;
using IMapper_v4     = ::android::hardware::graphics::mapper::V4_0::IMapper;

#define MIN_AIS_BUF_CNT 5
#define MAX_EVENT_Q_SIZE 5
#define QCARCAM_DEFAULT_GET_FRAME_TIMEOUT 500000000
#define NOT_IN_USE 0
#define ALIGN(num, to) (((num) + (to-1)) & (~(to-1))) /* To align to 8,16,32 or 64 bit values. */
#define WIDTH_ALIGN 64
//#define MOCK_CAMERA_HOTPLUG

namespace {

/* check if we need these
using ::android::base::Error;
using ::android::base::Result;*/
using ::aidl::android::hardware::graphics::common::HardwareBufferDescription;
using ::aidl::android::hardware::graphics::common::BufferUsage;
using ::ndk::ScopedAStatus;

}  // namespace

namespace aidl::android::hardware::automotive::evs::implementation {

static char const* printHalFormat(uint32_t halFormat);
// Arbitrary limit on number of graphics buffers allowed to be allocated
// Safeguards against unreasonable resource consumption and provides a testable limit
static const unsigned MAX_BUFFERS_IN_FLIGHT = 100;

EvsAISCamera::EvsAISCamera(const char *deviceName,
                           unique_ptr<ConfigManager::CameraInfo> &camInfo,
                           EvsEnumerator *service) :
        mFramesAllowed(0),
        mFramesInUse(0),
        mCameraInfo(camInfo),
        mEvs (service) {
    LOG(DEBUG) << "EvsAISCamera instantiated";

    mDescription.id = deviceName;
    if (camInfo != nullptr) {
        uint8_t* ptr = reinterpret_cast<uint8_t*>(camInfo->characteristics);
        const size_t len = get_camera_metadata_size(camInfo->characteristics);
        mDescription.metadata.insert(mDescription.metadata.end(), ptr, ptr + len);
    }

#ifndef ENABLE_BYPASS_MODE
    // Default output buffer format.
    mOutBufFormat = HAL_PIXEL_FORMAT_RGBA_8888;
#endif

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
}


EvsAISCamera::~EvsAISCamera() {
    LOG(DEBUG) << "EvsAISCamera being destroyed";
    shutdown();
    for (int i = 0; i < (int)mQcarcamOutBufs.nBuffers; ++i) {
        if (mpQcarcamMmapBufs && mpQcarcamMmapBufs[i].ptr)
            munmap(mpQcarcamMmapBufs[i].ptr, mpQcarcamMmapBufs[i].size);
        if (mpGfxBufs && mpGfxBufs[i])
            mpGfxBufs[i] = nullptr; //No need to delete sp<GraphicBuffer>
    }
    if (mpGfxBufs)
        free(mpGfxBufs);
    if (mQcarcamOutBufs.pBuffers)
        free(mQcarcamOutBufs.pBuffers);
    if (mpQcarcamMmapBufs)
        free(mpQcarcamMmapBufs);
}


//
// This gets called if another caller "steals" ownership of the camera
//
void EvsAISCamera::shutdown()
{
    LOG(DEBUG) << "EvsAISCamera shutdown";

    if (!mQcarcamHndl) {
        LOG(WARNING) << "Ignoring this call when camera has been lost.";
        return;
    }

    // Make sure our output stream is cleaned up
    // (It really should be already)
    stopVideoStream();

    // Note:  Since stopVideoStream is blocking, no other threads can now be running

    // Close qcarcam
    QCarCamRet_e ret;
    LOG(DEBUG) << "EvsAISCamera being destroyed";
    // Close AIS camera stream
    ret = QCarCamClose(this->mQcarcamHndl);
    if (ret != QCARCAM_RET_OK)
        LOG(ERROR) << "QCarCamClose() failed";
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
        for (int i = 0; i < (int)mQcarcamOutBufs.nBuffers; ++i) {
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
    // Remove mQcarcamHndl from Client Record
    mMultiCamQLock.lock();
    LOG(DEBUG) << "Removing mQcarcamHndl = "<< this->mQcarcamHndl <<" from Q";
    gQcarCamClientData *pRecord = findEventQByHndl(this->mQcarcamHndl);
    if (pRecord != NULL)
        pRecord->gQcarcamHndl = NOT_IN_USE;
    mMultiCamQLock.unlock();
    this->mQcarcamHndl = NOT_IN_USE;
    pCurrentRecord = nullptr;

    // Drop all the graphics buffers we've been using
    if (mBuffers.size() > 0) {
        ::android::GraphicBufferAllocator& alloc(::android::GraphicBufferAllocator::get());
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
    ::android::sp<IMapper_v4> mapper = IMapper_v4::getService();
    int fd = -1;
    mapper->get(const_cast<native_handle_t*>(hndl), qtigralloc::MetadataType_FD,
        [&](const auto &_error, const auto &_bytestream) {
                if (_error !=  Error_v4::NONE)
                    LOG(ERROR) << "Failed to get FD";
                else {
                    ::android::gralloc4::decodeInt32(qtigralloc::MetadataType_FD,
                            _bytestream, &fd);
                }
    });
    return fd;
}

ScopedAStatus EvsAISCamera::setMaxFramesInFlight(int32_t bufferCount) {
    LOG(DEBUG) << __FUNCTION__;
    std::lock_guard<std::mutex> lock(mAccessLock);

    if (bufferCount > MAX_BUFFERS_IN_FLIGHT) {
        LOG(WARNING) << "Rejecting buffer request in excess of internal limit";
        return ScopedAStatus::fromServiceSpecificError(
                    static_cast<int>(EvsResult::BUFFER_NOT_AVAILABLE));
    }

    // If we've been displaced by another owner of the camera, then we can't do anything else
    if (!mQcarcamHndl) {
        LOG(WARNING) << "Ignoring setMaxFramesInFlight call when camera has been lost.";
        return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::OWNERSHIP_LOST));
    }

    // We cannot function without at least one video buffer to send data
    if (bufferCount < 1) {
        LOG(ERROR) << "Ignoring setMaxFramesInFlight with less than one buffer requested";
        return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::INVALID_ARG));
    }

    // Update our internal state
    if (!setAvailableFrames_Locked(bufferCount)) {
        return ScopedAStatus::fromServiceSpecificError(
                static_cast<int>(EvsResult::BUFFER_NOT_AVAILABLE));
    }

    if (0 == mQcarcamOutBufs.nBuffers) {
        //allocate qcarcam internal buffers
        if (!allocQcarcamInternalBuffers(MIN_AIS_BUF_CNT)) {
            return ScopedAStatus::fromServiceSpecificError(
                    static_cast<int>(EvsResult::BUFFER_NOT_AVAILABLE));
        }

        LOG(DEBUG) << " calling QCarCamSetBuffers";
        mQcarcamOutBufs.id = 0;
        mQcarcamOutBufs.flags = 0;
        LOG(ERROR) << "Number of buffers QCarCamSetBuffers" << mQcarcamOutBufs.nBuffers;
        QCarCamRet_e ret = QCARCAM_RET_OK;
        ret = QCarCamSetBuffers(mQcarcamHndl, &mQcarcamOutBufs);
        if (ret != QCARCAM_RET_OK) {
            LOG(ERROR) << "QCarCamSetBuffers failed!";

            return ScopedAStatus::fromServiceSpecificError(
                    static_cast<int>(EvsResult::BUFFER_NOT_AVAILABLE));
        }
    }

    return ScopedAStatus::ok();
}


ScopedAStatus EvsAISCamera::startVideoStream(const std::shared_ptr<IEvsCameraStream>& client) {
    LOG(DEBUG) << __FUNCTION__;

    // If we've been displaced by another owner of the camera, then we can't do anything else
    if (!mQcarcamHndl) {
        LOG(WARNING) << "Ignoring startVideoStream call when camera has been lost.";
        return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::OWNERSHIP_LOST));
    }

    if (mStream.get() != nullptr) {
        LOG(ERROR) << "Ignoring startVideoStream call when a stream is already running.";
        return ScopedAStatus::fromServiceSpecificError(
                static_cast<int>(EvsResult::STREAM_ALREADY_RUNNING));
    }
    // If the client never indicated otherwise, configure ourselves for 3 streaming buffer
    if (mFramesAllowed < 1) {
        auto result = setMaxFramesInFlight(3);
        if (!result.isOk()) {
            LOG(WARNING) << "Allocating streaming buffers";
            return result;
        }
    }

    std::lock_guard<std::mutex> lock(mAccessLock);
    QCarCamRet_e ret = QCARCAM_RET_OK;
    ret = QCarCamRegisterEventCallback(mQcarcamHndl, &EvsAISCamera::evs_ais_event_cb, NULL);
    if (ret != QCARCAM_RET_OK) {
        LOG(ERROR) << "QCarCamRegisterEventCallback() failed";
		return ScopedAStatus::fromServiceSpecificError(
                static_cast<int>(EvsResult::UNDERLYING_SERVICE_ERROR));
    }

    //Default qcarcam events
    uint32_t param = QCARCAM_EVENT_FRAME_READY |
                QCARCAM_EVENT_INPUT_SIGNAL |
                QCARCAM_EVENT_ERROR | QCARCAM_EVENT_VENDOR |
                QCARCAM_EVENT_RECOVERY;
    ret = QCarCamSetParam(mQcarcamHndl, QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK, &param, sizeof(param));
    if (ret != QCARCAM_RET_OK) {
        LOG(ERROR) << "QCarCamSetParam() failed with ret "<<ret;
        return ScopedAStatus::fromServiceSpecificError(
                static_cast<int>(EvsResult::UNDERLYING_SERVICE_ERROR));
    }

    ret = QCarCamStart(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK) {
        LOG(ERROR) << "QCarCamStart() failed";
        return ScopedAStatus::fromServiceSpecificError(
                static_cast<int>(EvsResult::UNDERLYING_SERVICE_ERROR));
    } else
        LOG(DEBUG) << "QCarCamStart() success for stream "<<mDescription.id;

    // Record the user's callback for use when we have a frame ready
    mStream = client;

    // Set the state of our background thread
    int prevRunMode = mRunMode.fetch_or(RUN);
    if (prevRunMode & RUN) {
        // The background thread is already running, so we can't start a new stream
        LOG(ERROR) << "Already in RUN state, so we can't start a new streaming thread";
        return ScopedAStatus::ok();
    }

    // Fire up a thread to receive and dispatch the qcarcam frames
    mCaptureThread = std::thread([this](){ collectEvents(); });
#ifdef MOCK_CAMERA_HOTPLUG
    mHotPlugThread = std::thread([this](){ mockCameraHotPlug(); });
#endif

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

#ifdef FPS_PRINT
    startCalculateFps();
#endif
    return ScopedAStatus::ok();
}

ScopedAStatus EvsAISCamera::doneWithFrame(const BufferDesc& bufferDesc)  {
    doneWithFrame_impl(bufferDesc.bufferId, ::android::makeFromAidl(bufferDesc.buffer.handle));

    return ScopedAStatus::ok();
}

ScopedAStatus EvsAISCamera::stopVideoStream()  {
    LOG(DEBUG) << __FUNCTION__ <<" for stream "<<mDescription.id;

    QCarCamRet_e ret = QCARCAM_RET_OK;

    if (mStream == nullptr) {
        LOG(ERROR) << "Ignoring stopVideoStream call when a stream is already stopped.";
        return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::INVALID_ARG));
    }

    if (!mQcarcamHndl) {
        LOG(WARNING) << "Ignoring stopVideoStream call when camera has been lost.";
        return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::OWNERSHIP_LOST));
    }

#ifdef FPS_PRINT
    terminateCalculateFps();
#endif

    // Stop AIS stream
    ret = QCarCamStop(mQcarcamHndl);
    if (ret != QCARCAM_RET_OK)
        LOG(ERROR) << "QCarCamStop() failed";

    // Indicate the background thread to stop
    int prevRunMode = mRunMode.fetch_or(STOPPING);
    if (prevRunMode == STOPPED) {
        // The background thread wasn't running, so set the flag back to STOPPED
        mRunMode = STOPPED;
    } else if (prevRunMode & STOPPING) {
        LOG(ERROR) << "stopStream called while stream is already stopping.  Reentrancy is not supported!";
        return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::INVALID_ARG));
    } else {
        if (!pCurrentRecord) {
            LOG(ERROR) << "pCurrentRecord is null";
		    return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::INVALID_ARG));
        }
        pCurrentRecord->gCV.notify_one();
        // Block until the background thread is stopped
        if (mCaptureThread.joinable()) {
            mCaptureThread.join();
        }
        LOG(DEBUG) << "Capture thread stopped.";
    }

    if (mStream != nullptr) {
        //  mStream client is waiting on STREAM_STOPPED event.
        std::unique_lock <std::mutex> lock(mAccessLock);

        EvsEventDesc event;
        event.aType = EvsEventType::STREAM_STOPPED;
        auto result = mStream->notify(event);
        if (!result.isOk()) {
            LOG(ERROR) << "Error delivering end of stream event";
        }

        // Drop our reference to the client's stream receiver
        mStream     = nullptr;
    }

    return ScopedAStatus::ok();
}

// Methods from ::aidl::android::hardware::automotive::evs::IEvsCamera follow.
ScopedAStatus EvsAISCamera::getCameraInfo(CameraDesc* _aidl_return) {
    LOG(DEBUG) << __FUNCTION__;

    // Send back our self description
    *_aidl_return = mDescription;
    return ScopedAStatus::ok();
}

ScopedAStatus EvsAISCamera::getPhysicalCameraInfo([[maybe_unused]] const std::string& id,
                                                  CameraDesc* _aidl_return) {
    LOG(DEBUG) << __FUNCTION__;

    // This method works exactly same as getCameraInfo_1_1() in EVS HW module.
    *_aidl_return = mDescription;
    return ScopedAStatus::ok();
}

ScopedAStatus EvsAISCamera::doneWithFrame(const std::vector<BufferDesc>& buffers) {
    LOG(DEBUG) << __FUNCTION__;

    for (const auto& buffer : buffers) {
        doneWithFrame_impl(buffer.bufferId, ::android::makeFromAidl(buffer.buffer.handle));
    }

    return ScopedAStatus::ok();
}

ScopedAStatus EvsAISCamera::pauseVideoStream() {

    if(!QCarCamPause(mQcarcamHndl))
        return ScopedAStatus::ok();

    return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::UNDERLYING_SERVICE_ERROR));
}


ScopedAStatus EvsAISCamera::resumeVideoStream() {

    if(!QCarCamResume(mQcarcamHndl))
        return ScopedAStatus::ok();

    return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::UNDERLYING_SERVICE_ERROR));
}

ScopedAStatus EvsAISCamera::setPrimaryClient() {
    /* Because EVS HW module reference implementation expects a single client at
     * a time, this returns a success code always.
     */
    return ScopedAStatus::ok();
}

ScopedAStatus EvsAISCamera::forcePrimaryClient(const std::shared_ptr<IEvsDisplay>&) {
    /* Because EVS HW module reference implementation expects a single client at
     * a time, this returns a success code always.
     */
    return ScopedAStatus::ok();
}

ScopedAStatus EvsAISCamera::unsetPrimaryClient() {
    /* Because EVS HW module reference implementation expects a single client at
     * a time, there is no chance that this is called by the secondary client and
     * therefore returns a success code always.
     */
    return ScopedAStatus::ok();
}


ScopedAStatus EvsAISCamera::getParameterList(std::vector<CameraParam>* _aidl_return) {
    if (mCameraInfo) {
        _aidl_return->resize(mCameraInfo->controls.size());
        auto idx = 0;
        for (auto& [name, range] : mCameraInfo->controls) {
            (*_aidl_return)[idx++] = name;
        }
    }

    return ScopedAStatus::ok();
}


ScopedAStatus EvsAISCamera::getIntParameterRange(CameraParam id, ParameterRange* _aidl_return) {
    if (!mCameraInfo) {
        return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::NOT_SUPPORTED));
    }

    auto it = mCameraInfo->controls.find(id);
    if (it == mCameraInfo->controls.end()) {
        return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::NOT_SUPPORTED));
    }

    _aidl_return->min = std::get<0>(it->second);
    _aidl_return->max = std::get<1>(it->second);
    _aidl_return->step = std::get<2>(it->second);

    return ScopedAStatus::ok();
}


ScopedAStatus EvsAISCamera::setIntParameter(CameraParam id, int32_t value,
                                           std::vector<int32_t>* effectiveValue) {
    int ret = 0;

    (*effectiveValue).resize(1);
    (*effectiveValue)[0] = value;

    if (!convertToqcarcamParamID(id, (*effectiveValue)[0])) {
        return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::INVALID_ARG));
    } else {
        ScopedAStatus result = ScopedAStatus::ok();
    }

    return ScopedAStatus::ok();
}


ScopedAStatus EvsAISCamera::getIntParameter(CameraParam id,
                                           std::vector<int32_t>* value) {
    int ret = 0;
    (*value).resize(1);

    if (!convertFormqcarcamParamID(id, (*value))) {
        return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::INVALID_ARG));
    } else {
        return ScopedAStatus::ok();
    }    
}


ScopedAStatus EvsAISCamera::setExtendedInfo(int32_t opaqueIdentifier,
                                            const std::vector<uint8_t>& opaqueValue) {
    if (QCARCAM_MAX_VENDOR_PAYLOAD_SIZE*(sizeof(unsigned int)) < opaqueValue.size()) {
        LOG(ERROR) << "Max supported values upto "<<QCARCAM_MAX_VENDOR_PAYLOAD_SIZE;
        return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::UNDERLYING_SERVICE_ERROR));
    }

    mExtInfo.insert_or_assign(opaqueIdentifier, opaqueValue);

    if (mQcarcamHndl) {
        // Send extended info to qcarcam as vendor param
        QCarCamRet_e ret = QCARCAM_RET_OK;
        QCarCamVendorParam_t param = {};

        for (int i=0; i<opaqueValue.size(); i++) {
            param.data[i] = opaqueValue[i];
        }
        ret = QCarCamSetParam(mQcarcamHndl, (QCarCamParamType_e)opaqueIdentifier, &param, sizeof(param));
        if (QCARCAM_RET_OK != ret) {
            LOG(ERROR) <<"QCarCamSetParam() failed with err = " <<ret;
            if (QCARCAM_RET_UNSUPPORTED == ret) {
                mExtInfo.erase(opaqueIdentifier);
                return ScopedAStatus::fromServiceSpecificError(
                        static_cast<int>(EvsResult::INVALID_ARG));
            }
            else {
                return ScopedAStatus::fromServiceSpecificError(
                        static_cast<int>(EvsResult::UNDERLYING_SERVICE_ERROR));
            }
        }
        else
            LOG(DEBUG) <<"QCarCamSetParam() success for ctxt" <<mQcarcamHndl;
    }

    return ScopedAStatus::ok();
}


ScopedAStatus EvsAISCamera::getExtendedInfo(int32_t opaqueIdentifier,
                                            std::vector<uint8_t>* opaqueValue) {
    const auto it = mExtInfo.find(opaqueIdentifier);
    if (it == mExtInfo.end()) {
        return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::INVALID_ARG));
    } else {
        *opaqueValue = mExtInfo[opaqueIdentifier];
    }

    if (mQcarcamHndl) {
        // Get extended info from qcarcam
        QCarCamRet_e ret = QCARCAM_RET_OK;
        QCarCamVendorParam_t param = {};
        ret = QCarCamGetParam(mQcarcamHndl, (QCarCamParamType_e)opaqueIdentifier, &param, sizeof(param));
        if (QCARCAM_RET_OK != ret) {
            LOG(ERROR) <<"QCarCamGetParam() failed with ret = " <<ret;
            return ScopedAStatus::fromServiceSpecificError(
                    static_cast<int>(EvsResult::INVALID_ARG));
        }
        else {
            LOG(DEBUG) <<"QCarCamGetParam() success for ctxt" <<mQcarcamHndl;
            for (int i=0; i<(*opaqueValue).size(); i++) {
                (*opaqueValue)[i] = param.data[i];
            }
        }
    }

    return ScopedAStatus::ok();
}


ScopedAStatus EvsAISCamera::importExternalBuffers(const std::vector<BufferDesc>& buffers,
                                                  int32_t* _aidl_return) {
    LOG(DEBUG) << __FUNCTION__;

    if (mDescription.id.size() > 1) {
        LOG(WARNING) << "Logical camera device does not support";
        *_aidl_return = 0;
        return ScopedAStatus::fromServiceSpecificError(
                static_cast<int>(EvsResult::UNDERLYING_SERVICE_ERROR));
    }

    // If we've been displaced by another owner of the camera, then we can't do anything else
    if (!mQcarcamHndl) {
        LOG(WARNING) << "Ignoring a request add external buffers "
                     << "when camera has been lost.";
        *_aidl_return = 0;
        return ScopedAStatus::fromServiceSpecificError(static_cast<int>(EvsResult::UNDERLYING_SERVICE_ERROR));
    }
    if (buffers.size() == 0) {
        LOG(WARNING) << "Invalid buffer size";
        *_aidl_return = 0;
        return ScopedAStatus::fromServiceSpecificError(
                static_cast<int>(EvsResult::UNDERLYING_SERVICE_ERROR));
    }

    auto numBuffersToAdd = buffers.size();
    if (numBuffersToAdd < 1) {
        LOG(DEBUG) << "No buffers to add.";
        *_aidl_return = 0;
        return ScopedAStatus::ok();
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

        ::android::GraphicBufferMapper& mapper = ::android::GraphicBufferMapper::get();
        for (auto i = 0; i < numBuffersToAdd; ++i) {
            // TODO: reject if external buffer is configured differently.
            auto& b = buffers[i];
            const HardwareBufferDescription& description = b.buffer.description;

            mOutBufFormat = static_cast<uint32_t>(description.format);
            mStride = description.stride;
            mOutHeight = description.height;
            mOutWidth = description.width;
            // Import a buffer to add
            buffer_handle_t memHandle = nullptr;

            const auto result =
                    mapper.importBuffer(::android::dupFromAidl(b.buffer.handle), description.width,
                                        description.height, 1,
                                        static_cast<::android::PixelFormat>(description.format),
                                        static_cast<uint64_t>(description.usage),
                                        description.stride, &memHandle);

            if (result != ::android::NO_ERROR || memHandle == nullptr) {
                LOG(WARNING) << "Failed to import a buffer " << b.bufferId;
                continue;
            }
            LOG(DEBUG) <<"QCDEBUG: memHandle=" << memHandle;
            LOG(DEBUG) <<"QCDEBUG: EVS ID= "<<i <<" Format=" <<static_cast<::android::PixelFormat>(description.format) <<" W"<<description.width <<" H" << description.height;
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
                mBuffers.push_back(std::move(BufferRecord(memHandle)));
            }
            if (mC2dInitStatus && (mHalFrameFormat != mOutBufFormat)) {
                void *targetPixels = NULL;
                C2D_YUV_SURFACE_DEF targetSurfaceDef;
                memset(&targetSurfaceDef, 0, sizeof(targetSurfaceDef));
                int fd = get_handle_fd(memHandle);
                int ret = -1;
                int size = description.height * EvsAISCamera::getQcarcamStride(ALIGN(description.width, WIDTH_ALIGN), mOutBufFormat);
                void *mapped_buf = mmap(NULL, size , PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                uint32_t targetSurfaceId = 0;
                C2D_STATUS c2d_status;

                c2d_status = c2dMapAddr(fd, mapped_buf, size, 0, KGSL_USER_MEM_TYPE_ION, &targetPixels);
                if (c2d_status != C2D_STATUS_OK || targetPixels == NULL) {
                    LOG(ERROR) <<"c2dMapAddr failed status "<< c2d_status;
                    break;
                }
                targetSurfaceDef.width = description.width;
                targetSurfaceDef.height = description.height;
                targetSurfaceDef.stride0 = EvsAISCamera::getQcarcamStride(ALIGN(description.width, WIDTH_ALIGN), mOutBufFormat);
                targetSurfaceDef.stride1 = EvsAISCamera::getQcarcamStride(ALIGN(description.width, WIDTH_ALIGN), mOutBufFormat);
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
            *_aidl_return = mFramesAllowed;
		    return ScopedAStatus::fromServiceSpecificError(
                    static_cast<int>(EvsResult::UNDERLYING_SERVICE_ERROR));
        }
    } else {
        if (!setExternalBuffersAsInteranl(extFds)) {
            *_aidl_return = mFramesAllowed;
		    return ScopedAStatus::fromServiceSpecificError(
                    static_cast<int>(EvsResult::UNDERLYING_SERVICE_ERROR));
        }
        mDirectRendering = true;
    }

    LOG(DEBUG) << " calling QCarCamSetBuffers";
    mQcarcamOutBufs.flags = 0;
    QCarCamRet_e ret = QCARCAM_RET_OK;
    ret = QCarCamSetBuffers(mQcarcamHndl, &mQcarcamOutBufs);
    if (ret != QCARCAM_RET_OK) {
        LOG(ERROR) << "QCarCamSetBuffers failed!";
        *_aidl_return = mFramesAllowed;
		return ScopedAStatus::fromServiceSpecificError(
                static_cast<int>(EvsResult::UNDERLYING_SERVICE_ERROR));
    }

    *_aidl_return = mFramesAllowed - before;
    return ScopedAStatus::ok();
}


EvsResult EvsAISCamera::doneWithFrame_impl(uint32_t bufferId,
                                           buffer_handle_t memHandle) {
    std::lock_guard <std::mutex> lock(mAccessLock);

    // If we've been displaced by another owner of the camera, then we can't do anything else
    if (!mQcarcamHndl) {
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
                    QCarCamRet_e ret = QCarCamReleaseFrame(mQcarcamHndl, 0, idx);
                    if (QCARCAM_RET_OK != ret)
                        LOG(ERROR) << "QCarCamReleaseFrame() failed for id "<< idx << " ret = "<<ret;
                } else
                    LOG(ERROR) <<"Error while traversing, did not find memHandle="<<memHandle;
            }
            // Mark the frame as available
            mBuffers[bufferId].inUse = false;
            mFramesInUse--;

            // If this frame's index is high in the array, try to move it down
            // to improve locality after mFramesAllowed has been reduced.
    
            if (bufferId >= mFramesAllowed) {
                // Find an empty slot lower in the array (which should always exist in this case)
                bool found = false;
                for (auto&& rec : mBuffers) {
                    if (!rec.handle) {
                        rec.handle = mBuffers[bufferId].handle;
                        mBuffers[bufferId].handle = nullptr;
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    LOG(WARNING) << "No empty slot!";
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
    ::android::GraphicBufferAllocator& alloc(::android::GraphicBufferAllocator::get());

    unsigned added = 0;


    while (added < numToAdd) {
        unsigned pixelsPerLine;
        buffer_handle_t memHandle = nullptr;
        mOutHeight = mAisFrameHeight;
        mOutWidth = mAisFrameWidth;
        ::android::status_t result = alloc.allocate(ALIGN(mAisFrameWidth, WIDTH_ALIGN),
                                         mAisFrameHeight,
                                         mOutBufFormat, 1,
                                         mUsage,
                                         &memHandle, &pixelsPerLine, 0, "EvsAISCamera");
        if (result != ::android::NO_ERROR) {
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
        if (mStride > 0) {
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
            mBuffers.push_back(std::move(BufferRecord(memHandle)));
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
    ::android::GraphicBufferAllocator& alloc(::android::GraphicBufferAllocator::get());

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

#ifdef FPS_PRINT
void EvsAISCamera::calculateFps() {
    // Get the camera id for FPS print
    CameraDesc desc;
    auto status = getCameraInfo(&desc);
    if (!status.isOk()) {
        LOG(ERROR) << "Failed to read a camera descriptor";
        return;
    }
    auto cameraId = desc.id;

    while (get_frame_cnt)
    {
        for (int i = 0; i < FPS_TIME_COUNTER; i++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(FPS_TIME_INTERVAL_IN_MS));
            if(!get_frame_cnt)
                break;
        }
        LOG(INFO) << " FPS is " << frame_cnt << " for stream id "<< cameraId;
        frame_cnt = 0;
    }
}

bool EvsAISCamera::startCalculateFps() {
    LOG(INFO) << "Starting FPS Calculation";
    // Create the thread and report success if it gets started
    get_frame_cnt = true;
    frame_cnt = 0;
    mCalculateFps = std::thread([this]() { calculateFps(); });
    return mCalculateFps.joinable();
}

void EvsAISCamera::terminateCalculateFps() {
    LOG(INFO) << "Terminate FPS Calculation";
    get_frame_cnt = false;
    if (mCalculateFps.get_id() == std::this_thread::get_id()) {
        // We should not join by ourselves
        mCalculateFps.detach();
    } else if (mCalculateFps.joinable()) {
        // Join a rendering thread
        mCalculateFps.join();
    }
}
#endif

// This is the async callback from the video camera that tells us a frame is ready
void EvsAISCamera::forwardFrame(QCarCamFrameInfo_t *qcarcamFrameInfo) {
    LOG(DEBUG) << __FUNCTION__;
    bool readyForFrame = false;
    size_t idx = 0;
    void *pData = NULL;

    if (!mpQcarcamMmapBufs[qcarcamFrameInfo->id].ptr) {
        LOG(ERROR) <<"buffer is not mapped";
        return;
    }

    pData = (void *)mpQcarcamMmapBufs[qcarcamFrameInfo->id].ptr;

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
                it = mFdToMemHndlMapList.find((uint32_t)qcarcamFrameInfo->id);
                if (it != mFdToMemHndlMapList.end()) {
                    // Identify buffer from list
                    for (idx = 0; idx < mBuffers.size(); idx++) {
                        if (it->second == (uint64_t)mBuffers[idx].handle)
                            break;
                    }
                } else
                    LOG(ERROR) <<"Error while traversing, did not find buf idx="<<qcarcamFrameInfo->id;
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
        using AidlPixelFormat = ::aidl::android::hardware::graphics::common::PixelFormat;
        // retrive qcarcam mapped buffer
        if (!mpQcarcamMmapBufs[qcarcamFrameInfo->id].ptr) {
            LOG(ERROR) << "buffer is not mapped";
            return;
        }

        // Assemble the buffer description we'll transmit below
        buffer_handle_t memHandle = mBuffers[idx].handle;
        BufferDesc bufferDesc = {
                .buffer =
                        {
                                .description =
                                        {
                                                .width = static_cast<int32_t>(mOutWidth),
                                                .height = static_cast<int32_t>(mOutHeight),
                                                .layers = 1,
                                                .format = static_cast<AidlPixelFormat>(mOutBufFormat),
                                                .usage = static_cast<BufferUsage>(mUsage),
                                                .stride = static_cast<int32_t>(mStride),
                                        },
                                .handle = ::android::dupToAidl(memHandle),
                        },
                .bufferId = static_cast<int32_t>(idx),
                .deviceId = mDescription.id,
                .timestamp = static_cast<int64_t>(qcarcamFrameInfo->timestamp),
        };

        // Lock our output buffer for writing
        // TODO(b/145459970): Sometimes, physical camera device maps a buffer
        // into the address that is about to be unmapped by another device; this
        // causes SEGV_MAPPER.
        void *targetPixels = nullptr;
        ::android::GraphicBufferMapper& mapper = ::android::GraphicBufferMapper::get();
        auto result =
                mapper.lock(memHandle, GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN,
                            ::android::Rect(bufferDesc.buffer.description.width,
                                            bufferDesc.buffer.description.height),
                            (void**)&targetPixels);

        // If we failed to lock the pixel buffer, we're about to crash, but log it first
        if (!targetPixels) {
            // TODO(b/145457727): When EvsHidlTest::CameraToDisplayRoundTrip
            // test case was repeatedly executed, EVS occasionally fails to map
            // a buffer.
            LOG(ERROR) << "Camera failed to gain access to image buffer for writing - "
                       << " status: " << ::android::statusToString(result);
            mapper.unlock(memHandle);
            return;
        }

        // Transfer the video image into the output buffer, making any needed
        // format conversion along the way

        if (!mDirectRendering) {
            if ((!mC2dInitStatus) || (mHalFrameFormat == mOutBufFormat)) {
                //just do memcpy or cpu color conversion
                mFillBufferFromVideo(bufferDesc, (uint8_t *)targetPixels, pData, EvsAISCamera::getQcarcamStride(ALIGN(bufferDesc.buffer.description.width, WIDTH_ALIGN), mOutBufFormat));
            } else {
                C2D_OBJECT c2dObject;
                uint32_t targetSurfaceId = 0;
                int ret = C2D_STATUS_OK;

                memset(&c2dObject, 0, sizeof(C2D_OBJECT));
                c2dObject.surface_id = mpQcarcamMmapBufs[qcarcamFrameInfo->id].srcC2dSurfaceId;

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
        mapper.unlock(memHandle);

        // Issue the (asynchronous) callback to the client -- can't be holding
        // the lock
        bool flag = false;
        if (mStream) {
            std::vector<BufferDesc> frames;
            frames.push_back(std::move(bufferDesc));
            flag = mStream->deliverFrame(frames).isOk();
        }

        if (flag) {
            LOG(DEBUG) << "Delivered " << memHandle
                       << " as id " << bufferDesc.bufferId;
        } else {
            // This can happen if the client dies and is likely unrecoverable.
            // To avoid consuming resources generating failing calls, we stop sending
            // frames.  Note, however, that the stream remains in the "STREAMING" state
            // until cleaned up on the main thread.
            LOG(ERROR) << "Frame delivery call failed in the transport layer.";

            // Since we didn't actually deliver it, mark the frame as available
            std::lock_guard<std::mutex> lock(mAccessLock);
            mBuffers[idx].inUse = false;

            mFramesInUse--;
        }
#ifdef FPS_PRINT
        frame_cnt++;
#endif //FPS_PRINT
    }
}


bool EvsAISCamera::convertToqcarcamParamID(CameraParam id, int32_t value) {
	QCarCamRet_e ret = QCARCAM_RET_OK;
	QCarCamParamType_e qcarcamParamId;
	void *param = NULL;
	uint32_t param_size = 0;
	float brightness_value = 0.0f;
	float contrast_value = 0.0f;
	QCarCamExposureConfig_t paramExposure = {};
    switch (id) {
        case CameraParam::BRIGHTNESS:
            qcarcamParamId = QCARCAM_SENSOR_PARAM_BRIGHTNESS;
            brightness_value = (float)value;
            param = (void *)&brightness_value;
            param_size = sizeof(brightness_value);
            break;
        case CameraParam::CONTRAST:
            qcarcamParamId = QCARCAM_SENSOR_PARAM_CONTRAST;
            contrast_value = (float)value;
            param = (void *)&contrast_value;
            param_size = sizeof(contrast_value);
            break;
        case CameraParam::AUTO_EXPOSURE:
        case CameraParam::ABSOLUTE_EXPOSURE:
            qcarcamParamId = QCARCAM_SENSOR_PARAM_EXPOSURE;
            paramExposure.mode = (id == CameraParam::AUTO_EXPOSURE)?QCARCAM_EXPOSURE_AUTO:QCARCAM_EXPOSURE_MANUAL;
            paramExposure.gain[0] = (float)value;
            param = (void *)&paramExposure;
            param_size = sizeof(paramExposure);
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
    ret = QCarCamSetParam(mQcarcamHndl, qcarcamParamId, param, param_size);
    if (!ret) {
        LOG(ERROR) << "QCarCamSetParam() failed with ret = "<<ret;
        return false;
    }
    return true;
}

bool EvsAISCamera::convertFormqcarcamParamID(CameraParam id,
        vector<int32_t>& values) {
    
    QCarCamRet_e ret = QCARCAM_RET_OK;
    QCarCamParamType_e qcarcamParamId;
    void *param = NULL;
    uint32_t param_size = 0;
    float brightness_value = 0.0f;
    float contrast_value = 0.0f;
    QCarCamExposureConfig_t paramExposure = {};
    switch (id) {
        case CameraParam::BRIGHTNESS:
            qcarcamParamId = QCARCAM_SENSOR_PARAM_BRIGHTNESS;
            param = (void *)&brightness_value;
            param_size = sizeof(brightness_value);
            break;
        case CameraParam::CONTRAST:
            qcarcamParamId = QCARCAM_SENSOR_PARAM_CONTRAST;
            param = (void *)&contrast_value;
            param_size = sizeof(contrast_value);
            break;
        case CameraParam::AUTO_EXPOSURE:
        case CameraParam::ABSOLUTE_EXPOSURE:
            qcarcamParamId = QCARCAM_SENSOR_PARAM_EXPOSURE;
            paramExposure.mode = (id == CameraParam::AUTO_EXPOSURE)?QCARCAM_EXPOSURE_AUTO:QCARCAM_EXPOSURE_MANUAL;
            param = (void *)&paramExposure;
            param_size = sizeof(paramExposure);
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
    ret = QCarCamGetParam(mQcarcamHndl, qcarcamParamId, param, param_size);
    if (!ret) {
        LOG(ERROR) << "QCarCamSetParam() failed with ret = "<<ret;
        return false;
    }
    switch (id) {
        case CameraParam::BRIGHTNESS:
            values[0] = (int32_t)brightness_value;
            break;
        case CameraParam::CONTRAST:
            values[0] = (int32_t)contrast_value;
            break;
        case CameraParam::AUTO_EXPOSURE:
        case CameraParam::ABSOLUTE_EXPOSURE:
            values[0] = paramExposure.gain[0];
            break;
        default:
            LOG(ERROR) <<"Camera parameter is unknown";
            return false;
    }
    return true;
}

std::shared_ptr<EvsAISCamera> EvsAISCamera::Create(const char *deviceName,
                                      void *pvStreamConfigs,
                                      int numAisInputs,
                                      bool c2dInitStatus,
                                      EvsEnumerator *service) {
    std::unique_ptr<ConfigManager::CameraInfo> nullCamInfo = nullptr;

    return Create(deviceName, pvStreamConfigs, numAisInputs, c2dInitStatus, nullCamInfo, service);
}


std::shared_ptr<EvsAISCamera> EvsAISCamera::Create(const char *deviceName,
                                      void *pvStreamConfigs,
                                      int numAisInputs,
                                      bool c2dInitStatus,
                                      std::unique_ptr<ConfigManager::CameraInfo> &camInfo,
                                      EvsEnumerator *service,
                                      const Stream *requestedStreamCfg) {
    QCarCamRet_e ret = QCARCAM_RET_OK;
    LOG(INFO) << "Create " << deviceName;
    int camera_id, stream_id = -1, i;
    StreamConfiguration *pStreamConfigs = (StreamConfiguration *)pvStreamConfigs;
    sscanf(deviceName, "%d", &camera_id);
    std::shared_ptr<EvsAISCamera> evsCamera = ndk::SharedRefBase::make<EvsAISCamera>(deviceName, camInfo, service);
    if (!evsCamera) {
        return nullptr;
    }
    evsCamera->mC2dInitStatus = c2dInitStatus;

    for (int i = 0; i < numAisInputs; i++) {
        if (pStreamConfigs[i].inputId == camera_id) {
            //set evsCamera paramaters from qcarcam paramters
            evsCamera->mAisFrameWidth = pStreamConfigs[i].width;
            evsCamera->mAisFrameHeight = pStreamConfigs[i].height;
            evsCamera->mAisFrameFormat = pStreamConfigs[i].colorFmt;
            evsCamera->mHalFrameFormat = EvsAISCamera::getQcarcamToHalFormat(evsCamera->mAisFrameFormat);
            evsCamera->mAisFrameFPS = pStreamConfigs[i].fps;
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
            if (cfg.format == (requestedStreamCfg->format) ||
                    HAL_PIXEL_FORMAT_RGBA_8888 == static_cast<int32_t>(requestedStreamCfg->format)) {
                if (cfg.width == requestedStreamCfg->width &&
                        cfg.height == requestedStreamCfg->height) {
                    // Find exact match, update width, height and format.
                    stream_id = id;
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
            if (id == camera_id) {
                // Find exact match, update width, height and format.
                stream_id = id;
                evsCamera->mAisFrameWidth = camInfo->streamConfigurations[stream_id].width;
                evsCamera->mAisFrameHeight = camInfo->streamConfigurations[stream_id].height;
                evsCamera->mOutBufFormat = static_cast<uint32_t>(cfg.format);
                LOG(ERROR) << "Stream config is set from xml stream config camera_id = " << camera_id << "stream_id = " << id;
                break;
            }
        }
        if (-1 == stream_id) {
            LOG(ERROR) << "Given stram config does not match with existing config";
            return nullptr;
        }
    } else
        LOG(ERROR) << "Stream config is not given taking qcarcam query input values";

#ifdef ENABLE_BYPASS_MODE
    evsCamera->mOutBufFormat = evsCamera->mHalFrameFormat;
#endif

    // Initialize qcarcam camera
    QCarCamOpen_t openParams = {};
    openParams.opMode = QCARCAM_OPMODE_RAW_DUMP;
    openParams.numInputs = 1;
    openParams.inputs[0].inputId = (uint32_t)camera_id;
    openParams.flags = QCARCAM_OPEN_FLAGS_RECOVERY;
    ret = QCarCamOpen(&openParams, &evsCamera->mQcarcamHndl);
    if (ret != QCARCAM_RET_OK || evsCamera->mQcarcamHndl == QCARCAM_HNDL_INVALID) {
        LOG(ERROR) << "QCarCamOpen() failed";
        return nullptr;
    } else {
        LOG(DEBUG) << "QCarCamOpen() success for stream "<<camera_id
            <<" mQcarcamHndl = "<<evsCamera->mQcarcamHndl;
        mMultiCamQLock.lock();
        // Find an empty slot and insert handle
        for (i=0; i<MAX_AIS_CAMERAS; i++) {
            if (NOT_IN_USE == sQcarcamCameraArray[i].gQcarcamHndl)
                break;
        }
        if (i < MAX_AIS_CAMERAS) {
            sQcarcamCameraArray[i].gQcarcamHndl = evsCamera->mQcarcamHndl;
            evsCamera->pCurrentRecord = &sQcarcamCameraArray[i];
            mMultiCamQLock.unlock();
        } else {
            mMultiCamQLock.unlock();
            LOG(ERROR) << "Max number of streams reached!!";
            if (evsCamera->mQcarcamHndl)
                (void)QCarCamClose(evsCamera->mQcarcamHndl);
            return nullptr;
        }
        mMultiCamQLock.unlock();
    }
    evsCamera->mRunMode = STOPPED;

    // Please note that the buffer usage flag does not come from a given stream
    // configuration.
    evsCamera->mUsage  = GRALLOC_USAGE_HW_TEXTURE |
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
QCarCamRet_e EvsAISCamera::evs_ais_event_cb(const QCarCamHndl_t hndl,
        const uint32_t event_id,
        const QCarCamEventPayload_t *p_payload,
        void  *pPrivateData)
{
    int q_size = 0;
    (void)pPrivateData;

    //TODO::Protect with semaphore
    // This stream should be already in Queue
    mMultiCamQLock.lock();
    gQcarCamClientData *pRecord = findEventQByHndl(hndl);
    if (!pRecord) {
            LOG(ERROR) << "ERROR::Requested QCarCamHndl "<<hndl<<" not found";
            mMultiCamQLock.unlock();
            return QCARCAM_RET_FAILED;
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
            QCarCamEventPayload_t *payload =
                (QCarCamEventPayload_t *)malloc(sizeof(QCarCamEventPayload_t));
            if(payload != nullptr){
               memcpy(payload, p_payload, sizeof(QCarCamEventPayload_t));
               pRecord->sQcarcamList.emplace_back(event_id, payload);
               //ALOGI("Pushed event id %d to Event Queue", (int)event_id);
               }
        }
        else {
            LOG(ERROR) << "Max events are Queued in event Q";
        }
        lk.unlock();
        pRecord->gCV.notify_one();
    }
    return QCARCAM_RET_OK;
}

gQcarCamClientData* EvsAISCamera::findEventQByHndl(const QCarCamHndl_t qcarcam_hndl) {
    // Find the named camera
    int i = 0;
    for (i=0; i<MAX_AIS_CAMERAS; i++) {
        if (sQcarcamCameraArray[i].gQcarcamHndl == qcarcam_hndl) {
            // Found a match!
            return &sQcarcamCameraArray[i];
        }
    }
    LOG(ERROR) << "ERROR gQcarcamHndl = "<< qcarcam_hndl <<" did not match!!";

    // We didn't find a match
    return nullptr;
}

#ifdef MOCK_CAMERA_HOTPLUG
void EvsAISCamera::mockCameraHotPlug() {
    char value[PROPERTY_VALUE_MAX];
    value[0] = '1';
    char preVal;

    while(mRunMode == RUN) {
        preVal = value[0];
        property_get("evs.qcom.camera.status", value, "1");
        if (preVal != value[0]) {
            QCarCamEventPayload_t *payload = (QCarCamEventPayload_t*)malloc(sizeof(QCarCamEventPayload_t));
            QEventDesc event(QCARCAM_EVENT_INPUT_SIGNAL, payload);

            switch(value[0]) {
                case '0':
                event.p_payload->u32Data = QCARCAM_INPUT_SIGNAL_LOST;
                break;

                case '1':
                event.p_payload->u32Data = QCARCAM_INPUT_SIGNAL_VALID;
                break;

                default: break;
            }
            handleEvents(event);
        }
        usleep(100000);
    }
}
#endif

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
            // Wait for a buffer to be ready
            this->pCurrentRecord->gCV.wait(lk);
            if (mRunMode == STOPPED) {
                LOG(ERROR) <<"ERROR::Exiting eventDispatcherThread as user has called stop";
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
    EvsEventDesc event;
    QCarCamRet_e ret = QCARCAM_RET_OK;

    switch(events.event_id) {
        case QCARCAM_EVENT_FRAME_READY:
            ret = QCarCamGetFrame(mQcarcamHndl, &mQcarcamFrameInfo, QCARCAM_DEFAULT_GET_FRAME_TIMEOUT, 0);
            if (QCARCAM_RET_OK != ret) {
                LOG(ERROR) << "QCarCamGetFrame() failed for hndl "<<mQcarcamHndl<<" ret "<< ret;
            } else {
                //ALOGI("Fetched new frame from AIS");
#ifdef ENABLE_FRAME_DUMP
                dumpqcarcamFrame(&mQcarcamFrameInfo);
#endif
                forwardFrame(&mQcarcamFrameInfo);
                if (!mDirectRendering) {
                    // We can release the buffer here because we did memcpy to app
                    // RGBA8888 buffer
                    ret = QCarCamReleaseFrame(mQcarcamHndl, 0, mQcarcamFrameInfo.bufferIndex);
                    if (QCARCAM_RET_OK != ret)
                        LOG(ERROR) << "QCarCamReleaseFrame() failed for id "<<mQcarcamFrameInfo.bufferIndex<<" ret="<<ret;
                }
            }
            break;
        case QCARCAM_EVENT_ERROR:
            LOG(WARNING) << "QCARCAM_EVENT_ERROR received error u32Data = "<<events.p_payload->u32Data;
            switch (events.p_payload->errInfo.errorId)
            {
                case QCARCAM_ERROR_FATAL:
                    LOG(ERROR) << "QCARCAM_ERROR_FATAL received, need to restart stream";
                    // Restart error stream
                    ret = QCarCamStop(mQcarcamHndl);
                    if (QCARCAM_RET_OK != ret)
                        LOG(ERROR) << "QCarCamStop() failed for ctxt "<<mQcarcamHndl<<" ret "<< ret;
                    ret = QCarCamStart(mQcarcamHndl);
                    if (QCARCAM_RET_OK != ret)
                        LOG(ERROR) << "QCarCamStart() failed for ctxt "<<mQcarcamHndl<<" ret "<< ret;
                    break;
                case QCARCAM_ERROR_WARNING:
                    LOG(WARNING) << "QCARCAM_ERROR_WARNING errorCode = "<<events.p_payload->errInfo.errorCode;
                    break;
            }
            break;
        case QCARCAM_EVENT_RECOVERY:
            LOG(ERROR) << "Received a QCARCAM_EVENT_RECOVERY event";

            event.aType = EvsEventType::STREAM_ERROR;
            event.payload[0] = events.event_id;
            if (!(mStream->notify(event)).isOk()) {
                LOG(ERROR) << "Error delivering a recovery event";
            }
            break;
#ifdef QCC4x
        case QCARCAM_EVENT_FRAME_DROP:
            LOG(ERROR) << "Received a Frame drop event";

            event.aType = EvsEventType::FRAME_DROPPED;
            if (!(mStream->notify(event)).isOk()) {
                LOG(ERROR) << "Error delivering a frame drop event";
            }
            break;
#endif
        case QCARCAM_EVENT_INPUT_SIGNAL:
            LOG(ERROR) << "Received a QCARCAM_EVENT_INPUT_SIGNAL event";
            if (mEvs) {
                switch ((QCarCamInputSignal_e)(events.p_payload->u32Data)) {
                    case QCARCAM_INPUT_SIGNAL_VALID:
                        LOG(DEBUG) << "QCARCAM_INPUT_SIGNAL_VALID Called";
                        mEvs->notifyDeviceStatusChange(mDescription.id, DeviceStatusType::CAMERA_AVAILABLE);
                        break;
                    case QCARCAM_INPUT_SIGNAL_LOST:
                        LOG(DEBUG) << "QCARCAM_INPUT_SIGNAL_LOST Called";
                        mEvs->notifyDeviceStatusChange(mDescription.id, DeviceStatusType::CAMERA_NOT_AVAILABLE);
                        break;
                    case QCARCAM_INPUT_SIGNAL_MAX:
                        LOG(DEBUG) << "QCARCAM_INPUT_SIGNAL_MAX Called";
                        break;
                    default:
			LOG(ERROR) << "Unknown QCARCAM_EVENT_INPUT_SIGNAL received, ignore the event...";
                }
            } else
                LOG(ERROR) << "Invalid Enumerator service, ignore the event...";
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

void EvsAISCamera::dumpqcarcamFrame(QCarCamFrameInfo_t *pQcarcamFrameInfo)
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

    if (!mpQcarcamMmapBufs[pQcarcamFrameInfo->id].ptr)
    {
        LOG(ERROR) << "buffer is not mapped";
        return;
    }

    fp = fopen(filename, "w+");

    LOG(ERROR) << "dumping qcarcam frame "<< filename <<" numbytes "
        << mpQcarcamMmapBufs[pQcarcamFrameInfo->id].size;

    if (0 != fp)
    {
        pBuf = (unsigned char *)mpQcarcamMmapBufs[pQcarcamFrameInfo->id].ptr;
        numByteToWrite = mpQcarcamMmapBufs[pQcarcamFrameInfo->id].size;

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

QCarCamColorFmt_e EvsAISCamera::getHalToQcarcamFormat(uint32_t halFormat) {
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

uint32_t EvsAISCamera::getQcarcamToHalFormat(QCarCamColorFmt_e qcarcamFormat)
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
            return C2D_COLOR_FORMAT_888_RGB;
        case HAL_PIXEL_FORMAT_RGBA_8888:
            return C2D_COLOR_FORMAT_8888_ARGB | C2D_FORMAT_SWAP_RB;
        case HAL_PIXEL_FORMAT_YCbCr_420_SP: // 0x109, NV12
            return C2D_COLOR_FORMAT_420_NV12;
        default: LOG(ERROR) <<"Unsupported HAL format, default set to C2D_COLOR_FORMAT_422_UYVY";
                 return C2D_COLOR_FORMAT_422_UYVY;
    }
}

bool EvsAISCamera::allocQcarcamInternalBuffers(uint32_t count) {

    mQcarcamOutBufs.nBuffers = count;
    mQcarcamOutBufs.colorFmt = mAisFrameFormat;

    mQcarcamOutBufs.pBuffers = (QCarCamBuffer_t *)calloc(mQcarcamOutBufs.nBuffers, sizeof(*mQcarcamOutBufs.pBuffers));
    if (mQcarcamOutBufs.pBuffers == NULL) {
        LOG(ERROR) << "alloc qcarcam buffer failed";
        return false;
    }
    mpGfxBufs = (sp<::android::GraphicBuffer>*)calloc(mQcarcamOutBufs.nBuffers, sizeof(sp<::android::GraphicBuffer>));
    if (0 == mpGfxBufs) {
        LOG(ERROR) << "alloc mpGfxBufs failed";
        free(mQcarcamOutBufs.pBuffers);
        return false;
    }
    mpQcarcamMmapBufs = (qcarcam_mapped_buffer_t*)calloc(mQcarcamOutBufs.nBuffers,
            sizeof(qcarcam_mapped_buffer_t));
    if (0 == mpQcarcamMmapBufs) {
        LOG(ERROR) << "alloc mpQcarcamMmapBufs failed";
        free(mQcarcamOutBufs.pBuffers);
        free(mpGfxBufs);
        return false;
    }

    const int graphicsUsage  = ::android::GraphicBuffer::USAGE_HW_TEXTURE |
        ::android::GraphicBuffer::USAGE_SW_WRITE_OFTEN |
        ::android::GraphicBuffer::USAGE_SW_READ_OFTEN;
    const int nativeBufferFormat = mHalFrameFormat;
    C2D_FORMAT_MODE c2d_color_space = C2D_FORMAT_BT601_LIMITEDRANGE;
#ifdef QCC4x
    qcarcam_param_value_t param;
    if (0 != qcarcam_g_param(mQcarcamContext, QCARCAM_PARAM_INPUT_COLOR_SPACE, &param))
    {
        LOG(ERROR) << "Failed to get INPUT_COLOR_SPACE format";
        // Assign default color space format
        param.color_space = QCARCAM_COLOR_SPACE_BT601;
    }

    c2d_color_space = getC2dColorSpaceFormat(param.color_space);
#endif
    LOG(ERROR) << " C2D Color space format = " << (uint32_t)c2d_color_space;
    for (int i = 0; i < (int)mQcarcamOutBufs.nBuffers; ++i) {
        mQcarcamOutBufs.pBuffers[i].numPlanes = 1;
        mQcarcamOutBufs.pBuffers[i].planes[0].width = ALIGN(mAisFrameWidth, WIDTH_ALIGN);
        mQcarcamOutBufs.pBuffers[i].planes[0].height = mAisFrameHeight;
        mQcarcamOutBufs.pBuffers[i].planes[0].stride =
            EvsAISCamera::getQcarcamStride(ALIGN(mAisFrameWidth, WIDTH_ALIGN), mHalFrameFormat);
        mQcarcamOutBufs.pBuffers[i].planes[0].size = mQcarcamOutBufs.pBuffers[i].planes[0].height *
            mQcarcamOutBufs.pBuffers[i].planes[0].stride;

        LOG(DEBUG) <<"w="<< mQcarcamOutBufs.pBuffers[i].planes[0].width
                <<" h="<< mQcarcamOutBufs.pBuffers[i].planes[0].height<<endl
                <<"stride="<< mQcarcamOutBufs.pBuffers[i].planes[0].stride
                <<" size="<< mQcarcamOutBufs.pBuffers[i].planes[0].size;

        mpGfxBufs[i] = NULL;
        mpGfxBufs[i] = new ::android::GraphicBuffer(ALIGN(mQcarcamOutBufs.pBuffers[i].planes[0].width, WIDTH_ALIGN),
                mQcarcamOutBufs.pBuffers[i].planes[0].height,
                nativeBufferFormat,
                graphicsUsage);

        int fd = get_handle_fd(mpGfxBufs[i]->getNativeBuffer()->handle);
        mQcarcamOutBufs.pBuffers[i].planes[0].memHndl = (uint64_t)(uintptr_t)(fd);

        mpQcarcamMmapBufs[i].size = mQcarcamOutBufs.pBuffers[i].planes[0].size;
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
                free(mQcarcamOutBufs.pBuffers);
                free(mpGfxBufs);
                return false;
            }

            memset(&srcSurfaceDef, 0, sizeof(srcSurfaceDef));
            srcSurfaceDef.width = mQcarcamOutBufs.pBuffers[i].planes[0].width;
            srcSurfaceDef.height = mQcarcamOutBufs.pBuffers[i].planes[0].height;
            srcSurfaceDef.stride0 = mQcarcamOutBufs.pBuffers[i].planes[0].stride;
            srcSurfaceDef.stride1 = mQcarcamOutBufs.pBuffers[i].planes[0].stride;
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
                free(mQcarcamOutBufs.pBuffers);
                free(mpGfxBufs);
                return false;
            }
            mSrcGpuMapList.insert(std::pair<std::uint64_t, void *>((uint64_t)mpQcarcamMmapBufs[i].ptr, c2dsrcGpuAddr));
        }
    }
    return true;
}


bool EvsAISCamera::setExternalBuffersAsInteranl(vector<int>& extFds) {
    mQcarcamOutBufs.nBuffers = extFds.size();
    mQcarcamOutBufs.colorFmt = mAisFrameFormat;

    mQcarcamOutBufs.pBuffers = (QCarCamBuffer_t *)calloc(mQcarcamOutBufs.nBuffers, sizeof(*mQcarcamOutBufs.pBuffers));
    if (mQcarcamOutBufs.pBuffers == 0) {
        LOG(ERROR) << "alloc qcarcam buffer failed";
        return false;
    }
    mpQcarcamMmapBufs = (qcarcam_mapped_buffer_t*)calloc(mQcarcamOutBufs.nBuffers,
            sizeof(qcarcam_mapped_buffer_t));
    if (0 == mpQcarcamMmapBufs) {
        LOG(ERROR) << "alloc mpQcarcamMmapBufs failed";
        free(mQcarcamOutBufs.pBuffers);
        return false;
    }

    for (int i = 0; i < (int)mQcarcamOutBufs.nBuffers; ++i) {
        mQcarcamOutBufs.pBuffers[i].numPlanes = 1;
        mQcarcamOutBufs.pBuffers[i].planes[0].width = ALIGN(mAisFrameWidth, WIDTH_ALIGN);
        mQcarcamOutBufs.pBuffers[i].planes[0].height = mAisFrameHeight;
        mQcarcamOutBufs.pBuffers[i].planes[0].stride =
            EvsAISCamera::getQcarcamStride(ALIGN(mAisFrameWidth, WIDTH_ALIGN), mHalFrameFormat);
        mQcarcamOutBufs.pBuffers[i].planes[0].size = mQcarcamOutBufs.pBuffers[i].planes[0].height *
            mQcarcamOutBufs.pBuffers[i].planes[0].stride;

        LOG(DEBUG) <<"w="<< mQcarcamOutBufs.pBuffers[i].planes[0].width
            <<" h="<< mQcarcamOutBufs.pBuffers[i].planes[0].height<<endl
            <<"stride="<< mQcarcamOutBufs.pBuffers[i].planes[0].stride
            <<" size="<< mQcarcamOutBufs.pBuffers[i].planes[0].size;

        // Assign external FD to qcarcam buffer
        mQcarcamOutBufs.pBuffers[i].planes[0].memHndl = (uint64_t)(uintptr_t)(extFds[i]);

        // Map external buffer for future use
        mpQcarcamMmapBufs[i].size = mQcarcamOutBufs.pBuffers[i].planes[0].size;
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
}  // namespace aidl::android::hardware::automotive::evs::implementation
