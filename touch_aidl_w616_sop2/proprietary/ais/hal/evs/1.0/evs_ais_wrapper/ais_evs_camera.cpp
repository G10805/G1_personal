/*
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
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
 *
 *
 * Copyright (C) 2017 The Android Open Source Project
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
#include "ais_evs_camera.h"
#include "ais_evs_enumerator.h"
#include "buffer_copy.h"

#include <ui/GraphicBufferAllocator.h>
#include <ui/GraphicBufferMapper.h>
#include <gralloc_priv.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "c2d2.h"
#ifdef FPS_PRINT
#include <sys/time.h>
#endif

#ifndef CPU_CONVERSION
#define TARGET_SCISSOR 0
#define TARGET_MASK_ID 0
#define TARGET_COLOR_KEY 0
#define NO_OF_OBJECTS 1
#define KGSL_USER_MEM_TYPE_ION 3
#endif

namespace android {
namespace hardware {
namespace automotive {
namespace evs {
namespace V1_0 {
namespace implementation {


// Arbitrary limit on number of graphics buffers allowed to be allocated
// Safeguards against unreasonable resource consumption and provides a testable limit
static const unsigned MAX_BUFFERS_IN_FLIGHT = 100;


EvsAISCamera::EvsAISCamera(const char *deviceName,
        uint32_t frameWidth,
        uint32_t frameHeight,
        uint32_t frameFormat) :
        mFramesAllowed(0),
        mFramesInUse(0) {
    ALOGE("EvsAISCamera instantiated");

    mDescription.cameraId = deviceName;
    mAisFrameWidth = frameWidth;
    mAisFrameHeight = frameHeight;
    mAisFrameFormat = frameFormat;

    // Initialize qcarcam camera
    int camera_id, i;
    sscanf(deviceName, "%d", &camera_id);
    qcarcam_context = qcarcam_open((qcarcam_input_desc_t)camera_id);
    if (qcarcam_context == 0)
    {
        ALOGE("qcarcam_open() failed");
    }
    else
    {
        ALOGE("qcarcam_open() success for stream %d qcarcam_context = %p", camera_id, qcarcam_context);
        mMultiCamQLock.lock();
        // Find an empty slot and insert handle
        for (i=0; i<MAX_AIS_CAMERAS; i++) {
            if (NOT_IN_USE == sQcarcamCameraArray[i].qcarcam_context)
                break;
        }
        if (i < MAX_AIS_CAMERAS) {
            sQcarcamCameraArray[i].qcarcam_context = qcarcam_context;
            pCurrentRecord = &sQcarcamCameraArray[i];
            mMultiCamQLock.unlock();
        } else {
            mMultiCamQLock.unlock();
            ALOGE("Max number of streams reached!!");
        }
        mMultiCamQLock.unlock();
    }
    mRunMode = STOPPED;

    // NOTE:  Our current spec says only support NV21 -- can we stick to that with software
    // conversion?  Will this work with the hardware texture units?
    // TODO:  Settle on the one official format that works on all platforms
    // TODO:  Get NV21 working?  It is scrambled somewhere along the way right now.
//    mFormat = HAL_PIXEL_FORMAT_YCRCB_420_SP;    // 420SP == NV21
    //mFormat = HAL_PIXEL_FORMAT_YCBCR_422_I;
//TODO:: Remove this RGBA conversion once the EVS display HAL start supporting
//UYVY rendering
#ifdef ENABLE_RGBA_CONVERSION
    mFormat = HAL_PIXEL_FORMAT_RGBA_8888;
#else
    mFormat = mAisFrameFormat;
#endif

    // How we expect to use the gralloc buffers we'll exchange with our client
    mUsage  = GRALLOC_USAGE_HW_TEXTURE     |
              GRALLOC_USAGE_SW_READ_OFTEN |
              GRALLOC_USAGE_SW_WRITE_OFTEN;
#ifdef FPS_PRINT
    mFrameCnt = 0;
    mPrevMsec = 0;
#endif
    p_buffers_output.buffers = NULL;
}


EvsAISCamera::~EvsAISCamera() {
    ALOGI("EvsAISCamera being destroyed");
    shutdown();
    if (p_buffers_output.buffers != NULL) {
    for (int i = 0; i < (int)p_buffers_output.n_buffers; ++i) {
        if (qcarcam_mmap_buffer && qcarcam_mmap_buffer[i].ptr)
            munmap(qcarcam_mmap_buffer[i].ptr, qcarcam_mmap_buffer[i].size);
        if (gfx_bufs[i])
            gfx_bufs[i] = NULL;
    }
    if (gfx_bufs)
        free(gfx_bufs);
    if (p_buffers_output.buffers)
        free(p_buffers_output.buffers);
    if (qcarcam_mmap_buffer)
        free(qcarcam_mmap_buffer);
    }
}


//
// This gets called if another caller "steals" ownership of the camera
//
void EvsAISCamera::shutdown()
{
    ALOGI("EvsAISCamera shutdown");
    if (!qcarcam_context) {
        ALOGE("Ignoring this call when camera has been lost.");
        return;
    }

    // Make sure our output stream is cleaned up
    // (It really should be already)
    stopVideoStream();

    qcarcam_ret_t ret;
    // Close AIS camera stream
    ret = qcarcam_close(this->qcarcam_context);
    if (ret != QCARCAM_RET_OK)
    {
        ALOGE("qcarcam_close() failed");
    }
    if (p_buffers_output.buffers != NULL) {
#ifndef CPU_CONVERSION
        std::map<std::uint64_t, void *>::iterator it;

        for (it = mTgtGpuMapList.begin(); it != mTgtGpuMapList.end(); ++it) {
            c2dUnMapAddr(it->second);
        }
        for (it = mSrcGpuMapList.begin(); it != mSrcGpuMapList.end(); ++it) {
            c2dUnMapAddr(it->second);
        }

        std::map<uint64_t, uint32_t>::iterator itr;

        /* Destroy C2D Source Surface */
        for (int i = 0; i < (int)p_buffers_output.n_buffers; ++i) {
            c2dDestroySurface(qcarcam_mmap_buffer[i].srcC2dSurfaceId);
        }
        /* Destroy C2D Target Surface */
        for (itr = mTgtSurfaceIdList.begin(); itr != mTgtSurfaceIdList.end(); ++itr) {
            c2dDestroySurface(itr->second);
        }
        mTgtGpuMapList.clear();
        mSrcGpuMapList.clear();
        mTgtSurfaceIdList.clear();
#endif
    }
    mMultiCamQLock.lock();
    ALOGE("Removing qcarcam_context = %p from Q",this->qcarcam_context);
    gQcarCamClientData *pRecord = findEventQByHndl(this->qcarcam_context);
    if (pRecord != NULL)
        pRecord->qcarcam_context = NOT_IN_USE;
    mMultiCamQLock.unlock();
    this->qcarcam_context = NOT_IN_USE;
    pCurrentRecord = nullptr;

    // Drop all the graphics buffers we've been using
    if (mBuffers.size() > 0) {
        GraphicBufferAllocator& alloc(GraphicBufferAllocator::get());
        for (auto&& rec : mBuffers) {
            if (rec.inUse) {
                ALOGE("Error - releasing buffer despite remote ownership");
            }
            alloc.free(rec.handle);
            rec.handle = nullptr;
        }
        mBuffers.clear();
    }

}

int EvsAISCamera::getStrideMultiplayer(uint32_t mFormat)
{
    int stride_val = 1;
    switch(mFormat)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
            stride_val = 4;
            break;
        case HAL_PIXEL_FORMAT_CbYCrY_422_I:
            stride_val = 2;
            break;
        case HAL_PIXEL_FORMAT_RGB_888:
            stride_val = 3;
            break;
        default:
            stride_val = 1;
    }
    ALOGE("DEBUG LOG: mFormat = %x stride_val=%x",mFormat, stride_val);
    return stride_val;
}

// Methods from ::android::hardware::automotive::evs::V1_0::IEvsCamera follow.
Return<void> EvsAISCamera::getCameraInfo(getCameraInfo_cb _hidl_cb) {
    ALOGE("getCameraInfo");

    // Send back our self description
    _hidl_cb(mDescription);
    return Void();
}


Return<EvsResult> EvsAISCamera::setMaxFramesInFlight(uint32_t bufferCount) {
    ALOGE("setMaxFramesInFlight bufferCount = %u",bufferCount);
    std::lock_guard<std::mutex> lock(mAccessLock);

    if (bufferCount > MAX_BUFFERS_IN_FLIGHT) {
        ALOGE("Rejecting buffer request in excess of internal limit");
        return EvsResult::BUFFER_NOT_AVAILABLE;;
    }

    // If we've been displaced by another owner of the camera, then we can't do anything else
    if (!qcarcam_context) {
        ALOGE("Ignoring setMaxFramesInFlight call when camera has been lost.");
        return EvsResult::OWNERSHIP_LOST;
    }

    // We cannot function without at least one video buffer to send data
    if (bufferCount < 1) {
        ALOGE("Ignoring setMaxFramesInFlight with less than one buffer requested");
        return EvsResult::INVALID_ARG;
    }
    int ret = 0, i;

    p_buffers_output.n_buffers = MIN_AIS_BUF_CNT;
    // Update our internal state
    if (!setAvailableFrames_Locked(bufferCount)) {
        return EvsResult::BUFFER_NOT_AVAILABLE;
    }
    if (p_buffers_output.buffers != NULL) { // Do buf alloc and create c2d once only
        ALOGE("Already allocated buffers available\n");
        return EvsResult::OK;
    }
    p_buffers_output.color_fmt = getHalToQcarcamFormat(mAisFrameFormat);
    p_buffers_output.buffers = (qcarcam_buffer_t *)calloc(p_buffers_output.n_buffers, sizeof(*p_buffers_output.buffers));
    if (p_buffers_output.buffers == 0)
    {
        ALOGE("alloc qcarcam_buffer failed");
        return EvsResult::BUFFER_NOT_AVAILABLE;
    }
    gfx_bufs = (sp<GraphicBuffer>*)calloc(p_buffers_output.n_buffers, sizeof(sp<GraphicBuffer>));
    if (gfx_bufs == 0)
    {
        ALOGE("alloc gfx_bufs failed");
        return EvsResult::BUFFER_NOT_AVAILABLE;
    }
    qcarcam_mmap_buffer = (test_util_buffer_t*)calloc(p_buffers_output.n_buffers, sizeof(test_util_buffer_t));
    if (qcarcam_mmap_buffer == 0)
    {
        ALOGE("alloc qcarcam_mmap_buffer failed");
        free(gfx_bufs);
        return EvsResult::BUFFER_NOT_AVAILABLE;
    }
    const int graphicsUsage  = android::GraphicBuffer::USAGE_HW_TEXTURE | android::GraphicBuffer::USAGE_SW_WRITE_OFTEN;
    const int nativeBufferFormat = mAisFrameFormat;
    for (i = 0; i < (int)p_buffers_output.n_buffers; ++i)
    {
        p_buffers_output.buffers[i].n_planes = 1;
        p_buffers_output.buffers[i].planes[0].width = mAisFrameWidth;
        p_buffers_output.buffers[i].planes[0].height = mAisFrameHeight;

        p_buffers_output.buffers[i].planes[0].stride = ALIGN(mAisFrameWidth*getStrideMultiplayer(mAisFrameFormat),64);
        p_buffers_output.buffers[i].planes[0].size = p_buffers_output.buffers[i].planes[0].height * p_buffers_output.buffers[i].planes[0].stride;

        ALOGE("%dx%d %d %d", p_buffers_output.buffers[i].planes[0].width, p_buffers_output.buffers[i].planes[0].height, p_buffers_output.buffers[i].planes[0].stride, p_buffers_output.buffers[i].planes[0].size);

        gfx_bufs[i] = NULL;
        gfx_bufs[i] = new GraphicBuffer(ALIGN(p_buffers_output.buffers[i].planes[0].width,64),
                p_buffers_output.buffers[i].planes[0].height,
                nativeBufferFormat,
                graphicsUsage);

        struct private_handle_t * private_hndl = (struct private_handle_t *)(gfx_bufs[i]->getNativeBuffer()->handle);
        ALOGE("%d %p\n",i, private_hndl);
        p_buffers_output.buffers[i].planes[0].p_buf = (void*)(uintptr_t)(private_hndl->fd);

        qcarcam_mmap_buffer[i].size = p_buffers_output.buffers[i].planes[0].size;

        qcarcam_mmap_buffer[i].ptr = mmap(NULL, qcarcam_mmap_buffer[i].size,
                        PROT_READ | PROT_WRITE, MAP_SHARED,
                        private_hndl->fd, 0);
        if (qcarcam_mmap_buffer[i].ptr)
            ALOGE("mmap success\n");

#ifndef CPU_CONVERSION
        C2D_STATUS c2d_status;
        C2D_YUV_SURFACE_DEF srcSurfaceDef;
        void *c2dsrcGpuAddr = NULL;

        c2d_status = c2dMapAddr(private_hndl->fd, qcarcam_mmap_buffer[i].ptr, qcarcam_mmap_buffer[i].size, 0, KGSL_USER_MEM_TYPE_ION, &c2dsrcGpuAddr);
        if (c2d_status != C2D_STATUS_OK)
        {
            ALOGE("c2dMapAddr failed for id %d status %d", i, c2d_status);
            return EvsResult::BUFFER_NOT_AVAILABLE;
        }
        memset(&srcSurfaceDef, 0, sizeof(srcSurfaceDef));

        srcSurfaceDef.width = mAisFrameWidth;
        srcSurfaceDef.height = mAisFrameHeight;
        srcSurfaceDef.stride0 = ALIGN(mAisFrameWidth * getStrideMultiplayer(mAisFrameFormat),64);
        srcSurfaceDef.stride1 = ALIGN(mAisFrameWidth * getStrideMultiplayer(mAisFrameFormat),64);
        srcSurfaceDef.format = getHalToC2dFormat(mAisFrameFormat);
        srcSurfaceDef.plane0 = qcarcam_mmap_buffer[i].ptr; //Creating surface with one buffer later we update it in update_surface.
        srcSurfaceDef.plane1 = (void *) ((unsigned char *)srcSurfaceDef.plane0 + (mAisFrameWidth * mAisFrameHeight));
        srcSurfaceDef.phys0 = c2dsrcGpuAddr;
        srcSurfaceDef.phys1 = 0;
        ret = c2dCreateSurface(&qcarcam_mmap_buffer[i].srcC2dSurfaceId, C2D_SOURCE, (C2D_SURFACE_TYPE) (((srcSurfaceDef.format != C2D_COLOR_FORMAT_888_RGB) ? C2D_SURFACE_YUV_HOST: C2D_SURFACE_RGB_HOST) | C2D_SURFACE_WITH_PHYS), &srcSurfaceDef);
        if (ret != C2D_STATUS_OK) {
            c2dUnMapAddr(c2dsrcGpuAddr);
            ALOGE("Source c2d_create API falied ret=%d\n", ret);
            return EvsResult::BUFFER_NOT_AVAILABLE;
        }
        mSrcGpuMapList.insert(std::pair<std::uint64_t, void *>((uint64_t)qcarcam_mmap_buffer[i].ptr, c2dsrcGpuAddr));
    }
#endif

    ALOGV("QCDEBUG: calling qcarcam_s_buffers");
    p_buffers_output.flags = 0;
    ret = qcarcam_s_buffers(qcarcam_context, &p_buffers_output);
    if (ret != QCARCAM_RET_OK)
    {
        ALOGE("qcarcam_s_buffers failed!");
        return EvsResult::BUFFER_NOT_AVAILABLE;
    }
    else
    {
        ALOGE("qcarcam_s_buffers success");
    }
    return EvsResult::OK;
}


Return<EvsResult> EvsAISCamera::startVideoStream(const ::android::sp<IEvsCameraStream>& stream)  {
    ALOGE("startVideoStream");

    if (!qcarcam_context) {
        ALOGW("Ignoring startVideoStream call when camera has been lost.");
        return EvsResult::OWNERSHIP_LOST;
    }

    if (mStream.get() != nullptr) {
        ALOGE("Ignoring startVideoStream call when a stream is already running.");
        return EvsResult::STREAM_ALREADY_RUNNING;
    }
    // If the client never indicated otherwise, configure ourselves for a 3 streaming buffer
    if (mFramesAllowed < 1) {
        Return <EvsResult> result = setMaxFramesInFlight(3);
        if (result != EvsResult::OK) {
            ALOGE("Fail to allocate streaming buffers");
            return result;
        }
    }

    int ret = 0;
    mStream = stream;
    qcarcam_param_value_t param;
    param.ptr_value = (void *)&EvsAISCamera::evs_ais_event_cb;
    ret = qcarcam_s_param(this->qcarcam_context, QCARCAM_PARAM_EVENT_CB, &param);

    param.uint_value = QCARCAM_EVENT_FRAME_READY | QCARCAM_EVENT_INPUT_SIGNAL | QCARCAM_EVENT_ERROR;
    ret = qcarcam_s_param(this->qcarcam_context, QCARCAM_PARAM_EVENT_MASK, &param);

    ret = qcarcam_start(this->qcarcam_context);
    if (ret != QCARCAM_RET_OK)
    {
        ALOGE("qcarcam_start() failed");
        return EvsResult::UNDERLYING_SERVICE_ERROR;
    }
    else
    {
        ALOGE("qcarcam_start() success");
    }

    // Set the state of our background thread
    int prevRunMode = mRunMode.fetch_or(RUN);
    if (prevRunMode & RUN) {
        // The background thread is already running, so we can't start a new stream
        ALOGE("Already in RUN state, so we can't start a new streaming thread");
        return EvsResult::OK;
    }

    // Fire up a thread to receive and dispatch the qcarcam frames
    mCaptureThread = std::thread([this](){ collectFrames(); });

    return EvsResult::OK;
}


Return<void> EvsAISCamera::doneWithFrame(const BufferDesc& buffer)  {
    //ALOGE("doneWithFrame");
    std::lock_guard <std::mutex> lock(mAccessLock);

    if (!qcarcam_context) {
        ALOGE("Ignoring doneWithFrame call when camera has been lost.");
    } else {

        if (buffer.memHandle == nullptr) {
            ALOGE("ignoring doneWithFrame called with null handle");
        } else if (buffer.bufferId >= mBuffers.size()) {
            ALOGE("ignoring doneWithFrame called with invalid bufferId %d (max is %zu)",
                    buffer.bufferId, mBuffers.size()-1);
        } else if (!mBuffers[buffer.bufferId].inUse) {
            ALOGE("ignoring doneWithFrame called on frame %d which is already free",
                    buffer.bufferId);
        } else {
            // Mark the frame as available
            mBuffers[buffer.bufferId].inUse = false;
            mFramesInUse--;

            // If this frame's index is high in the array, try to move it down
            // to improve locality after mFramesAllowed has been reduced.
            if (buffer.bufferId >= mFramesAllowed) {
                // Find an empty slot lower in the array (which should always exist in this case)
                for (auto&& rec : mBuffers) {
                    if (rec.handle == nullptr) {
                        rec.handle = mBuffers[buffer.bufferId].handle;
                        mBuffers[buffer.bufferId].handle = nullptr;
                        break;
                    }
                }
            }
        }
    }
    return Void();
}


Return<void> EvsAISCamera::stopVideoStream()  {
    ALOGE("stopVideoStream");

    qcarcam_ret_t ret = QCARCAM_RET_OK;

    if (mStream == nullptr) {
        ALOGE("Ignoring stopVideoStream call when a stream is already stopped.");
        return Void();
    }

    if (!qcarcam_context) {
        ALOGE("Ignoring stopVideoStream call when camera has been lost.");
        return Void();
    }

    // Stop AIS stream
    ret = qcarcam_stop(this->qcarcam_context);
    if (ret != QCARCAM_RET_OK)
    {
        ALOGE("qcarcam_stop() failed");
    }

    // Tell the background thread to stop
    int prevRunMode = mRunMode.fetch_or(STOPPING);
    if (prevRunMode == STOPPED) {
        // The background thread wasn't running, so set the flag back to STOPPED
        mRunMode = STOPPED;
    } else if (prevRunMode & STOPPING) {
        ALOGE("stopStream called while stream is already stopping.  Reentrancy is not supported!");
        return Void();
    } else {
        if (!pCurrentRecord) {
            ALOGE("pCurrentRecord is null");
            return Void();
        }
        pCurrentRecord->gCV.notify_one();
        // Block until the background thread is stopped
        if (mCaptureThread.joinable()) {
            mCaptureThread.join();
        }
        ALOGI("Capture thread stopped.");
    }

    if (mStream != nullptr) {
        std::unique_lock <std::mutex> lock(mAccessLock);

        // Send one last NULL frame to signal the actual end of stream
        BufferDesc nullBuff = {};
        auto result = mStream->deliverFrame(nullBuff);
        if (!result.isOk()) {
            ALOGE("Error delivering end of stream marker");
        }

        // Drop our reference to the client's stream receiver
        mStream = nullptr;
    }
    return Void();
}


Return<int32_t> EvsAISCamera::getExtendedInfo(uint32_t /*opaqueIdentifier*/)  {
    ALOGE("getExtendedInfo");
    // Return zero by default as required by the spec
    return 0;
}


Return<EvsResult> EvsAISCamera::setExtendedInfo(uint32_t /*opaqueIdentifier*/,
                                                int32_t /*opaqueValue*/)  {
    ALOGE("setExtendedInfo");
    // Return zero by default as required by the spec
    return EvsResult::OK;
}


bool EvsAISCamera::setAvailableFrames_Locked(unsigned bufferCount) {
    if (bufferCount < 1) {
        ALOGE("Ignoring request to set buffer count to zero");
        return false;
    }
    if (bufferCount > MAX_BUFFERS_IN_FLIGHT) {
        ALOGE("Rejecting buffer request in excess of internal limit");
        return false;
    }

    // Is an increase required?
    if (mFramesAllowed < bufferCount) {
        // An increase is required
        unsigned needed = bufferCount - mFramesAllowed;
        ALOGE("Allocating %d buffers for camera frames", needed);

        unsigned added = increaseAvailableFrames_Locked(needed);
        if (added != needed) {
            // If we didn't add all the frames we needed, then roll back to the previous state
            ALOGE("Rolling back to previous frame queue size");
            decreaseAvailableFrames_Locked(added);
            return false;
        }
    } else if (mFramesAllowed > bufferCount) {
        // A decrease is required
        unsigned framesToRelease = mFramesAllowed - bufferCount;
        ALOGE("Returning %d camera frame buffers", framesToRelease);

        unsigned released = decreaseAvailableFrames_Locked(framesToRelease);
        if (released != framesToRelease) {
            // This shouldn't happen with a properly behaving client because the client
            // should only make this call after returning sufficient outstanding buffers
            // to allow a clean resize.
            ALOGE("Buffer queue shrink failed -- too many buffers currently in use?");
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

        status_t result = alloc.allocate(ALIGN(mAisFrameWidth,64),
                                         mAisFrameHeight,
                                         mFormat, 1,
                                         mUsage,
                                         &memHandle, &pixelsPerLine, 0, "EvsAISCamera");
        if (result != NO_ERROR) {
            ALOGE("Error %d allocating %d x %d graphics buffer",
                  result,
                  mAisFrameWidth,
                  mAisFrameHeight);
            break;
        }
        if (!memHandle) {
            ALOGE("We didn't get a buffer handle back from the allocator");
            break;
        }
        if (mStride) {
            if (mStride != pixelsPerLine) {
                ALOGE("We did not expect to get buffers with different strides!");
            }
        } else {
            // Gralloc defines stride in terms of pixels per line
            mStride = pixelsPerLine;
            ALOGE("mstride = %d\n",mStride);
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
#ifndef CPU_CONVERSION
            void *targetPixels = NULL;
            C2D_YUV_SURFACE_DEF targetSurfaceDef;

            memset(&targetSurfaceDef, 0, sizeof(targetSurfaceDef));
            struct private_handle_t * private_hndl = (struct private_handle_t *)(void *)memHandle;

            ALOGV("private_hndl %p\n", private_hndl);
            if (private_hndl) {
                int fd = private_hndl->fd;
                int ret = -1;
                int size = ALIGN(mAisFrameWidth, 64)*mAisFrameHeight*getStrideMultiplayer(mFormat);;
                void *mapped_buf = mmap(NULL, size , PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                uint32_t targetSurfaceId = 0;
                C2D_STATUS c2d_status;
                C2D_YUV_SURFACE_DEF targetSurfaceDef;

                c2d_status = c2dMapAddr(private_hndl->fd, mapped_buf, size, 0, KGSL_USER_MEM_TYPE_ION, &targetPixels);
                if (c2d_status != C2D_STATUS_OK || targetPixels == NULL)
                {
                    ALOGE("c2dMapAddr failed status %d", c2d_status);
                    break;
                }
                targetSurfaceDef.width = ALIGN(mAisFrameWidth, 64);
                targetSurfaceDef.height = mAisFrameHeight;
                targetSurfaceDef.stride0 = targetSurfaceDef.width * getStrideMultiplayer(mFormat);
                targetSurfaceDef.stride1 = targetSurfaceDef.width * getStrideMultiplayer(mFormat);
                targetSurfaceDef.plane0 = mapped_buf;
                targetSurfaceDef.plane1 = (void *)((unsigned char *)targetSurfaceDef.plane0 + (targetSurfaceDef.width * targetSurfaceDef.height));
                if (getHalToC2dFormat(mAisFrameFormat) == C2D_COLOR_FORMAT_888_RGB)
                    targetSurfaceDef.format = C2D_COLOR_FORMAT_8888_ARGB;
                else
                    targetSurfaceDef.format = C2D_COLOR_FORMAT_8888_ARGB | C2D_FORMAT_SWAP_RB;
                targetSurfaceDef.phys0 = targetPixels;
                targetSurfaceDef.phys1 = 0;
                ALOGE("QCDEBUG: memHandle=%lu\n", (uint64_t)memHandle);
                ret = c2dCreateSurface(&targetSurfaceId, C2D_TARGET, (C2D_SURFACE_TYPE) (C2D_SURFACE_RGB_HOST | C2D_SURFACE_WITH_PHYS), &targetSurfaceDef);
                if (ret != C2D_STATUS_OK) {
                    c2dUnMapAddr(targetPixels);
                    ALOGE("Target c2d_create API falied ret=%d\n", ret);
                    break;
                }
                mTgtSurfaceIdList.insert(std::pair<std::uint64_t, uint32_t>((uint64_t)memHandle, targetSurfaceId));
                mTgtGpuMapList.insert(std::pair<std::uint64_t, void *>((uint64_t)memHandle, targetPixels));
                munmap(mapped_buf, size);
            }
#endif
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
#ifndef CPU_CONVERSION
            std::map<std::uint64_t, uint32_t>::iterator it;
            it = mTgtSurfaceIdList.find((uint64_t)rec.handle);
            if (it != mTgtSurfaceIdList.end()) {
                c2dDestroySurface(it->second);
                mTgtSurfaceIdList.erase((uint64_t)rec.handle);
            } else {
                ALOGE("Error while traversing\n");
            }
            std::map<std::uint64_t, void *>::iterator itr;
            itr = mTgtGpuMapList.find((uint64_t)rec.handle);
            if (itr != mTgtGpuMapList.end()) {
                c2dUnMapAddr(itr->second);
                mTgtGpuMapList.erase((uint64_t)rec.handle);
            } else {
                ALOGE("Error while traversing\n");
            }
#endif
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
            ALOGE("received unsupported event %d", event_id);
            break;
    }

    //TODO::Protect with semaphore
    // This stream should be already in Queue
    mMultiCamQLock.lock();
    gQcarCamClientData *pRecord = findEventQByHndl(hndl);
    if (!pRecord) {
            ALOGE("ERROR::Requested qcarcam_hndl %p not found", hndl);
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
        if (q_size < 3)
        {
            pRecord->sQcarcamList.emplace_back(event_id, p_payload);
            //ALOGI("Pushed event id %d to Event Queue", (int)event_id);
        }
        else {
            ALOGE("Max events are Queued in event Q");
        }
        lk.unlock();
        pRecord->gCV.notify_one();
    }
}

gQcarCamClientData* EvsAISCamera::findEventQByHndl(const qcarcam_hndl_t qcarcam_hndl) {
    // Find the named camera
    int i = 0;
    for (i=0; i<MAX_AIS_CAMERAS; i++) {
        if (sQcarcamCameraArray[i].qcarcam_context == qcarcam_hndl) {
            // Found a match!
            return &sQcarcamCameraArray[i];
        }
    }
    ALOGE("ERROR qcarcam_context = %p did not match!!\n",qcarcam_hndl);

    // We didn't find a match
    return nullptr;
}

// This runs on a background thread to receive and dispatch qcarcam frames
void EvsAISCamera::collectFrames() {
    int q_size = 0;
    qcarcam_ret_t ret;
    ALOGE("collectFrames thread running");

    if (!this->pCurrentRecord) {
        ALOGE("ERROR::Requested qcarcam_hndl not found in the list");
        ALOGI("VideoCapture thread ending");
        mRunMode = STOPPED;
        return;
    }
    // Run until our atomic signal is cleared
    while (mRunMode == RUN) {
        // Wait for a buffer to be ready
        std::unique_lock<std::mutex> lk(this->pCurrentRecord->gEventQLock);
        q_size = this->pCurrentRecord->sQcarcamList.size();
        if (q_size) {
            //ALOGI("Found %d events in the Event Queue", q_size);
            this->pCurrentRecord->sQcarcamList.pop_front();
            lk.unlock();

            ret = qcarcam_get_frame(this->qcarcam_context, &frame_info, 500000000, 0);

            if (QCARCAM_RET_OK != ret)
            {
                ALOGE("qcarcam_get_frame failed 0x%p ret %d\n", this->qcarcam_context, ret);
            }
            else
            {
                //ALOGI("Fetched new frame from AIS");
                //dumpqcarcamFrame(&frame_info);
                sendFramesToApp(&frame_info);
                ret = qcarcam_release_frame(this->qcarcam_context, frame_info.idx);
                if (QCARCAM_RET_OK != ret)
                {
                    ALOGE("qcarcam_release_frame() %d failed", frame_info.idx);
                }
            }
        }
        else
        {
            // Wait for a buffer to be ready
            this->pCurrentRecord->gCV.wait(lk);
            if (mRunMode == STOPPED) {
                ALOGE("ERROR::Exiting eventDispatcherThread as user has called stop");
                lk.unlock();
                break;
            }
            q_size = this->pCurrentRecord->sQcarcamList.size();
            if (q_size) {
                //ALOGI("Found %d events in the Event Queue", q_size);
                this->pCurrentRecord->sQcarcamList.pop_front();
                lk.unlock();

                ret = qcarcam_get_frame(this->qcarcam_context, &frame_info, 500000000, 0);
                if (QCARCAM_RET_OK != ret)
                {
                    ALOGE("qcarcam_get_frame failed 0x%p ret %d\n", this->qcarcam_context, ret);
                }
                else
                {
                    //ALOGI("Fetched new frame from AIS");
                    //dumpqcarcamFrame(&frame_info);
                    sendFramesToApp(&frame_info);
                    ret = qcarcam_release_frame(this->qcarcam_context, frame_info.idx);
                    if (QCARCAM_RET_OK != ret)
                    {
                        ALOGE("qcarcam_release_frame() %d failed", frame_info.idx);
                    }
                }
            }
            else
            {
                //ALOGI("Event Q is empty");
                lk.unlock();
            }
        }
    }

    // Mark ourselves stopped
    ALOGI("VideoCapture thread ending");
    mRunMode = STOPPED;
}

void EvsAISCamera::dumpqcarcamFrame(qcarcam_frame_info_t *frame_info)
{
    FILE *fp;
    size_t numBytesWritten = 0;
    size_t numByteToWrite = 0;
    unsigned char *pBuf = 0;
    static int frame_cnt = 0;
    char filename[128];

    if (frame_cnt > 5)
        return;

    snprintf(filename, sizeof(filename), "%s_%d","/sdcard/Pictures/evs_frame",frame_cnt);

    if (NULL == frame_info)
    {
        ALOGE("Enpty frame info");
        return;
    }

    if (!qcarcam_mmap_buffer[frame_info->idx].ptr)
    {
        ALOGE("buffer is not mapped");
        return;
    }

    fp = fopen(filename, "w+");

    ALOGE("dumping qcarcam frame %s numbytes %d", filename,
        qcarcam_mmap_buffer[frame_info->idx].size);

    if (0 != fp)
    {
        pBuf = (unsigned char *)qcarcam_mmap_buffer[frame_info->idx].ptr;
        numByteToWrite = qcarcam_mmap_buffer[frame_info->idx].size;

        numBytesWritten = fwrite(pBuf, 1, numByteToWrite, fp);

        if (numBytesWritten != numByteToWrite)
        {
            ALOGE("error no data written to file");
        }
        fclose(fp);
    }
    else
    {
        ALOGE("failed to open file");
        return;
    }
    frame_cnt++;
}

void EvsAISCamera::sendFramesToApp(qcarcam_frame_info_t *frame_info)
{
    bool readyForFrame = false;
    size_t idx = 0;
    int ret = -1;

#ifdef CPU_CONVERSION
    void *pData = NULL;

    if (!qcarcam_mmap_buffer[frame_info->idx].ptr)
    {
        ALOGE("buffer is not mapped");
        return;
    }

    pData = (void *)qcarcam_mmap_buffer[frame_info->idx].ptr;
#endif
    // Lock scope for updating shared state
    {
        std::lock_guard<std::mutex> lock(mAccessLock);

        // Are we allowed to issue another buffer?
        if (mFramesInUse >= mFramesAllowed) {
            // Can't do anything right now -- skip this frame
            ALOGE("Skipped a frame because too many are in flight mFramesInUse = %d, mFramesAllowed = %d\n", mFramesInUse, mFramesAllowed);
        } else {
            // Identify an available buffer to fill
            for (idx = 0; idx < mBuffers.size(); idx++) {
                if (!mBuffers[idx].inUse) {
                    if (mBuffers[idx].handle != nullptr) {
                        // Found an available record, so stop looking
                        break;
                    }
                }
            }
            if (idx >= mBuffers.size()) {
                // This shouldn't happen since we already checked mFramesInUse vs mFramesAllowed
                ALOGE("Failed to find an available buffer slot\n");
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
        //mVideo.markFrameConsumed();
        ALOGI("Frame is not ready");
    } else {
        // Assemble the buffer description we'll transmit below
        BufferDesc buff = {};
        buff.width      = p_buffers_output.buffers[frame_info->idx].planes[0].width;
        buff.height     = p_buffers_output.buffers[frame_info->idx].planes[0].height;
        buff.stride     = mStride;
        buff.format     = mFormat;
        buff.usage      = mUsage;
        buff.bufferId   = idx;
        buff.memHandle  = mBuffers[idx].handle;

        // Lock our output buffer for writing
        void *targetPixels = nullptr;

        // Lock our output buffer for writing
        GraphicBufferMapper &mapper = GraphicBufferMapper::get();
        mapper.lock(buff.memHandle,
                GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_RARELY,
                android::Rect(ALIGN(buff.width, 64), buff.height),
                (void **) &targetPixels);

        // If we failed to lock the pixel buffer, we're about to crash, but log it first
        if (!targetPixels) {
            ALOGE("Camera failed to gain access to image buffer for writing");
            return;
        }
        else {
            // Transfer the video image into the output buffer, making any needed
            // format conversion along the way
#ifdef CPU_CONVERSION
            fillRGBAFromUYVY(buff, (uint8_t*)targetPixels, pData);
#else
            C2D_OBJECT c2dObject;
            uint32_t targetSurfaceId = 0;

            memset(&c2dObject, 0, sizeof(C2D_OBJECT));
            c2dObject.surface_id = qcarcam_mmap_buffer[frame_info->idx].srcC2dSurfaceId;

            /* Get the C2D target Surface ID corresponds to memHandle */
            std::map<std::uint64_t, uint32_t>::iterator it;
            it = mTgtSurfaceIdList.find((uint64_t)&(*buff.memHandle));

            if (it != mTgtSurfaceIdList.end()) {
                targetSurfaceId = it->second;
            } else {
                ALOGE("Error while traversing\n");
            }
            ret = c2dDraw(targetSurfaceId, C2D_TARGET_ROTATE_0 , TARGET_SCISSOR, TARGET_MASK_ID, TARGET_COLOR_KEY, &c2dObject, NO_OF_OBJECTS);
            if (ret != C2D_STATUS_OK) {
                ALOGE("C2DDraw failed with ret=%d\n", ret);
                return;
            }
            ret = c2dFinish(targetSurfaceId);
            if (ret != C2D_STATUS_OK) {
                ALOGE("C2DFinish failed with ret=%d\n", ret);
                return;
            }
#endif
            // Unlock the output buffer
            mapper.unlock(buff.memHandle);
        }
        //ALOGI("Target buffer is ready");

        //ALOGI("buff.stride = %u buff.mUsage = %u, buff.idx = %d",buff.stride,  buff.usage, buff.bufferId);
        //ALOGI("Sending %p as id %d", buff.memHandle.getNativeHandle(), buff.bufferId);

        // Issue the (asynchronous) callback to the client -- can't be holding the lock
        auto result = mStream->deliverFrame(buff);
        if (!result.isOk()) {
            // This can happen if the client dies and is likely unrecoverable.
            // To avoid consuming resources generating failing calls, we stop sending
            // frames.  Note, however, that the stream remains in the "STREAMING" state
            // until cleaned up on the main thread.
            ALOGE("Frame delivery call failed in the transport layer.");

            // Since we didn't actually deliver it, mark the frame as available
            std::lock_guard<std::mutex> lock(mAccessLock);
            mBuffers[idx].inUse = false;
            mFramesInUse--;
        }
#ifdef FPS_PRINT
        uint64_t cur_msec = 0, diff_msec = 0;
        struct timeval current_time;
        gettimeofday(&current_time, NULL);
        if (0 == mFrameCnt)
            mPrevMsec = (current_time.tv_sec*1000) + (current_time.tv_usec/1000);
        mFrameCnt++;
        // If we get 30 FPS we will get 150 frames for every 5 seconds
        if (150 == mFrameCnt) {
            gettimeofday(&current_time, NULL);
            cur_msec = (current_time.tv_sec*1000) + (current_time.tv_usec/1000);
            diff_msec = cur_msec - mPrevMsec;
            if (diff_msec > (uint64_t)5000) {
                double total_frame_rendered = diff_msec * 0.03; // 0.03 frames per 1msec for 30FPS
                double extra_frames = total_frame_rendered / 150.0;
                double fps = 30.0 / extra_frames;
                LOG(ERROR) <<fps<<" FPS for stream id "<<mDescription.v1.cameraId;
            }
            else
                LOG(ERROR) <<"30 FPS for Stream Id "<<mDescription.v1.cameraId;
            // Reset frame count
            mFrameCnt = 0;
            // Save current time
            mPrevMsec = cur_msec;
        }
#endif //FPS_PRINT
    }
}

void EvsAISCamera::fillTargetBuffer(uint8_t* tgt, void* imgData, unsigned size) {
    uint32_t* src = (uint32_t*)imgData;
    uint32_t* dst = (uint32_t*)tgt;

    memcpy(dst, src, size);
}

#ifdef ENABLE_RGBA_CONVERSION
// Limit the given value to the provided range.  :)
float EvsAISCamera::clamp(float v, float min, float max) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}


uint32_t EvsAISCamera::yuvToRgbx(const unsigned char Y, const unsigned char Uin, const unsigned char Vin) {
    // Don't use this if you want to see the best performance.  :)
    // Better to do this in a pixel shader if we really have to, but on actual
    // embedded hardware we expect to be able to texture directly from the YUV data
    float U = Uin - 128.0f;
    float V = Vin - 128.0f;

    float Rf = Y + 1.140f*V;
    float Gf = Y - 0.395f*U - 0.581f*V;
    float Bf = Y + 2.032f*U;
    unsigned char R = (unsigned char)clamp(Rf, 0.0f, 255.0f);
    unsigned char G = (unsigned char)clamp(Gf, 0.0f, 255.0f);
    unsigned char B = (unsigned char)clamp(Bf, 0.0f, 255.0f);

    return ((R & 0xFF))       |
           ((G & 0xFF) << 8)  |
           ((B & 0xFF) << 16) |
           0xFF000000;  // Fill the alpha channel with ones
}

void EvsAISCamera::fillRGBAFromUYVY(const BufferDesc& tgtBuff, uint8_t* tgt, void* imgData) {
    unsigned width = tgtBuff.width;
    unsigned height = tgtBuff.height;
    uint32_t* src = (uint32_t*)imgData;
    uint32_t* dst = (uint32_t*)tgt;

    for (unsigned r=0; r<height; r++) {
        for (unsigned c=0; c<width/2; c++) {
            // Note:  we're walking two pixels at a time here (even/odd)
            uint32_t srcPixel = *src++;

            uint8_t U  = (srcPixel)       & 0xFF;
            uint8_t Y1 = (srcPixel >> 8)  & 0xFF;
            uint8_t V  = (srcPixel >> 16) & 0xFF;
            uint8_t Y2 = (srcPixel >> 24) & 0xFF;

            // On the RGB output, we're writing one pixel at a time
            *(dst+0) = yuvToRgbx(Y1, U, V);
            *(dst+1) = yuvToRgbx(Y2, U, V);
            dst += 2;
        }
    }
}
#endif /* ENABLE_RGBA_CONVERSION */
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
        default: ALOGE("Unsupported HAL format, default set to QCARCAM_FMT_UYVY_8");
                 return QCARCAM_FMT_UYVY_8;
    }
}

#ifndef CPU_CONVERSION
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
        default: ALOGE("Unsupported HAL format, default set to C2D_COLOR_FORMAT_422_UYVY");
                 return C2D_COLOR_FORMAT_422_UYVY;
    }
}
#endif


} // namespace implementation
} // namespace V1_0
} // namespace evs
} // namespace automotive
} // namespace hardware
} // namespace android
