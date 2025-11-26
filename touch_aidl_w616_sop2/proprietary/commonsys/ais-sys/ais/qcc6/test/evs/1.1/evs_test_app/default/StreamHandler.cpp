/*
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

#include "StreamHandler.h"

#include <stdio.h>
#include <string.h>

#include <android-base/logging.h>
#include <cutils/native_handle.h>
#include <ui/GraphicBufferAllocator.h>

using ::android::hardware::automotive::evs::V1_0::EvsResult;


buffer_handle_t memHandle = nullptr;
StreamHandler::StreamHandler(android::sp <IEvsCamera> pCamera,
                             uint32_t numBuffers,
                             bool useOwnBuffers,
                             android_pixel_format_t format,
                             int32_t width,
                             int32_t height)
    : mCamera(pCamera),
      mUseOwnBuffers(useOwnBuffers) {
    if (!useOwnBuffers) {
        // We rely on the camera having at least two buffers available since we'll hold one and
        // expect the camera to be able to capture a new image in the background.
        pCamera->setMaxFramesInFlight(numBuffers);
    } else {
        mOwnBuffers.resize(numBuffers);

        // Acquire the graphics buffer allocator
        android::GraphicBufferAllocator &alloc(android::GraphicBufferAllocator::get());
        const auto usage = GRALLOC_USAGE_HW_TEXTURE |
                           GRALLOC_USAGE_SW_READ_OFTEN |
                           GRALLOC_USAGE_SW_WRITE_OFTEN;
        //const auto format = HAL_PIXEL_FORMAT_RGBA_8888;
        for (auto i = 0; i < numBuffers; ++i) {
            unsigned pixelsPerLine;
            android::status_t result = alloc.allocate(width,
                                                      height,
                                                      format,
                                                      1,
                                                      usage,
                                                      &memHandle,
                                                      &pixelsPerLine,
                                                      0,
                                                      "EvsApp");
            if (result != android::NO_ERROR) {
                LOG(ERROR) << __FUNCTION__ << " failed to allocate memory.";
            } else {
                BufferDesc_1_1 buf;
                AHardwareBuffer_Desc* pDesc =
                    reinterpret_cast<AHardwareBuffer_Desc *>(&buf.buffer.description);
                pDesc->width = width;
                pDesc->height = height;
                pDesc->layers = 1;
                pDesc->format = format;
                pDesc->usage = GRALLOC_USAGE_HW_TEXTURE |
                               GRALLOC_USAGE_SW_READ_OFTEN |
                               GRALLOC_USAGE_SW_WRITE_OFTEN;
                pDesc->stride = pixelsPerLine;
                buf.buffer.nativeHandle = memHandle;
                buf.bufferId = i;   // Unique number to identify this buffer
                mOwnBuffers[i] = buf;
            }
        }

        int delta = 0;
        EvsResult result = EvsResult::OK;
        pCamera->importExternalBuffers(mOwnBuffers,
                                       [&](auto _result, auto _delta) {
                                           result = _result;
                                           delta = _delta;
                                       });

        LOG(INFO) << delta << " buffers are imported by EVS.";
    }
#ifdef TEST_VENDOR_PARAM
    LOG(ERROR) << "Calling vendor param";
    // Send predefined max9296 data to max9296 bridge chip
    hidl_vec<unsigned int> vendor_data{0x00, 0x00, 61, 0x90, 0x02, 0xf3, 0x50, 0x00,
                                     0x40, 0xdf, 0xa8, 0x07, 0x35, 0x12, 0x66, 0xcd,
                                     0x94, 0x07, 0x08, 0x01, 0x0f, 0x04, 0xda, 0x89,
                                     0x34, 0x41, 0x01, 0xa0, 0x00, 0x07, 0x20, 0xc0,
                                     0x00, 0x08, 0x80, 0x9f, 0x9f, 0x00, 0x00, 0x00,
                                     0x00, 0x22, 0x22, 0x60, 0x08, 0x20, 0x44, 0x00,
                                     0x28, 0x50, 0x06, 0x41, 0x88, 0x44, 0x04, 0xff,
                                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned int qcarcam_paramid = QCARCAM_PARAM_VENDOR;

    qcarcam_param_value_t param;
    for (int i = 0; i < vendor_data.size(); i++) {
        param.vendor_param.data[i] = vendor_data[i];
    }
    vendor_ext_property_t *p_prop = (vendor_ext_property_t*)&(param.vendor_param.data[0]);
    p_prop->type = VENDOR_EXT_PROP_TEST_I2C_BULK;
    p_prop->value.uint_val = vendor_data[0];

    uint8_t *ptr = (uint8_t *)&(param.vendor_param.data[0]);
    hidl_vec<uint8_t> p_param =  hidl_vec<uint8_t>(ptr, ptr + vendor_data.size()*(sizeof(unsigned int)));
    Return <EvsResult> result = pCamera->setExtendedInfo_1_1(qcarcam_paramid /*opaqueIdentifier*/, p_param);
    if (result != EvsResult::OK) {
        LOG(ERROR) << "Could not send vendor param";
    } else {
        LOG(ERROR) << "setExtendedInfo_1_1 was successful";

        pCamera->getExtendedInfo_1_1(qcarcam_paramid /*opaqueIdentifier*/,
                [vendor_data, &param](const auto& result, const auto& byte_data) {
                    if (result == EvsResult::OK) {
                        uint32_t *ptr = (uint32_t *)&byte_data[0];
                        for (int i = 0; i < (byte_data.size()/4); i++)
                            param.vendor_param.data[i] = (unsigned int)ptr[i];

                        LOG(ERROR) << "getExtendedInfo_1_1 was successful; received below vendor data ";
                        for (int i=3;i<(byte_data.size()/4);i++) {
                            LOG(ERROR) <<"read data 0x"<< std::hex<< (unsigned int)(vendor_data[0]+i-3)
                                    <<" <- 0x"<< std::hex<< (unsigned int)(param.vendor_param.data[i]);
                        }

                    } else if (result ==  EvsResult::UNDERLYING_SERVICE_ERROR) {
                        LOG(ERROR) << "qcarcam_g_param API failed";

                    } else
                        LOG(ERROR) << "Invalid opaqueIdentifier";
                });
    }
#endif /* TEST_VENDOR_PARAM */
}


void StreamHandler::shutdown()
{
    // Make sure we're not still streaming
    blockingStopStream();

    // At this point, the receiver thread is no longer running, so we can safely drop
    // our remote object references so they can be freed
    mCamera = nullptr;

    if (mUseOwnBuffers) {
        android::GraphicBufferAllocator &alloc(android::GraphicBufferAllocator::get());
        for (auto& b : mOwnBuffers) {
            alloc.free(b.buffer.nativeHandle);
        }

        mOwnBuffers.resize(0);
    }
}


bool StreamHandler::startStream() {
    std::unique_lock<std::mutex> lock(mLock);

    if (!mRunning) {
        // Tell the camera to start streaming
        Return <EvsResult> result = mCamera->startVideoStream(this);
        if (result != EvsResult::OK) {
            return false;
        }

        // Mark ourselves as running
        mRunning = true;
    }

    return true;
}


void StreamHandler::asyncStopStream() {
    // Tell the camera to stop streaming.
    // This will result in a null frame being delivered when the stream actually stops.
    mCamera->stopVideoStream();
}


void StreamHandler::blockingStopStream() {
    // Tell the stream to stop
    asyncStopStream();

    // Wait until the stream has actually stopped
    std::unique_lock<std::mutex> lock(mLock);
    if (mRunning) {
        mSignal.wait(lock, [this]() { return !mRunning; });
    }
}


bool StreamHandler::isRunning() {
    std::unique_lock<std::mutex> lock(mLock);
    return mRunning;
}


bool StreamHandler::newFrameAvailable() {
    std::unique_lock<std::mutex> lock(mLock);
    return (mReadyBuffer >= 0);
}


const BufferDesc_1_1& StreamHandler::getNewFrame() {
    std::unique_lock<std::mutex> lock(mLock);

    if (mHeldBuffer >= 0) {
        LOG(ERROR) << "Ignored call for new frame while still holding the old one.";
    } else {
        if (mReadyBuffer < 0) {
            LOG(ERROR) << "Returning invalid buffer because we don't have any.  "
                       << "Call newFrameAvailable first?";
            mReadyBuffer = 0;   // This is a lie!
        }

        // Move the ready buffer into the held position, and clear the ready position
        mHeldBuffer = mReadyBuffer;
        mReadyBuffer = -1;
    }

    return mBuffers[mHeldBuffer];
}


void StreamHandler::doneWithFrame(const BufferDesc_1_1& bufDesc_1_1) {
    std::unique_lock<std::mutex> lock(mLock);

    // We better be getting back the buffer we original delivered!
    if ((mHeldBuffer < 0) || (bufDesc_1_1.bufferId != mBuffers[mHeldBuffer].bufferId)) {
        LOG(ERROR) << "StreamHandler::doneWithFrame got an unexpected bufDesc_1_1!";
    }

    // Send the buffer back to the underlying camera
    hidl_vec<BufferDesc_1_1> frames;
    frames.resize(1);
    frames[0] = mBuffers[mHeldBuffer];
    mCamera->doneWithFrame_1_1(frames);

    // Clear the held position
    mHeldBuffer = -1;
}


Return<void> StreamHandler::deliverFrame(const BufferDesc_1_0& bufDesc_1_0) {
    LOG(INFO) << "Ignores a frame delivered from v1.0 EVS service.";
    mCamera->doneWithFrame(bufDesc_1_0);

    return Void();
}


Return<void> StreamHandler::deliverFrame_1_1(const hidl_vec<BufferDesc_1_1>& buffers) {
    LOG(DEBUG) << "Received frames from the camera";

    // Take the lock to protect our frame slots and running state variable
    std::unique_lock <std::mutex> lock(mLock);
    BufferDesc_1_1 bufDesc = buffers[0];
    if (bufDesc.buffer.nativeHandle.getNativeHandle() == nullptr) {
        // Signal that the last frame has been received and the stream is stopped
        LOG(WARNING) << "Invalid null frame (id: " << std::hex << bufDesc.bufferId
                     << ") is ignored";
    } else {
        // Do we already have a "ready" frame?
        if (mReadyBuffer >= 0) {
            // Send the previously saved buffer back to the camera unused
            hidl_vec<BufferDesc_1_1> frames;
            frames.resize(1);
            frames[0] = mBuffers[mReadyBuffer];
            mCamera->doneWithFrame_1_1(frames);

            // We'll reuse the same ready buffer index
        } else if (mHeldBuffer >= 0) {
            // The client is holding a buffer, so use the other slot for "on deck"
            mReadyBuffer = 1 - mHeldBuffer;
        } else {
            // This is our first buffer, so just pick a slot
            mReadyBuffer = 0;
        }

        // Save this frame until our client is interested in it
        mBuffers[mReadyBuffer] = bufDesc;
    }

    // Notify anybody who cares that things have changed
    lock.unlock();
    mSignal.notify_all();

    return Void();
}


Return<void> StreamHandler::notify(const EvsEventDesc& event) {
    switch(event.aType) {
        case EvsEventType::STREAM_STOPPED:
        {
            {
                std::lock_guard<std::mutex> lock(mLock);

                // Signal that the last frame has been received and the stream is stopped
                mRunning = false;
            }
            LOG(INFO) << "Received a STREAM_STOPPED event";
            break;
        }

        case EvsEventType::PARAMETER_CHANGED:
            LOG(INFO) << "Camera parameter " << std::hex << event.payload[0]
                      << " is set to " << event.payload[1];
            break;

        case EvsEventType::STREAM_ERROR:
            LOG(ERROR) << "Received a STREAM_ERROR event with payload " << event.payload[0];
            break;

        case EvsEventType::FRAME_DROPPED:
            LOG(ERROR) << "Received a FRAME_DROPPED event";
            break;

        // Below events are ignored in reference implementation.
        case EvsEventType::STREAM_STARTED:
        [[fallthrough]];
        case EvsEventType::TIMEOUT:
            LOG(INFO) << "Event " << std::hex << static_cast<unsigned>(event.aType)
                      << "is received but ignored.";
            break;
        default:
            LOG(ERROR) << "Unknown event id: " << static_cast<unsigned>(event.aType);
            break;
    }

    return Void();
}

