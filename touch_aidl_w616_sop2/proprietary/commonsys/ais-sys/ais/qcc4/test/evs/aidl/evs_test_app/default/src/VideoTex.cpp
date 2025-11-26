/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a contribution.
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
#include "VideoTex.h"

#include "Utils.h"
#include "glError.h"

#include <aidl/android/hardware/automotive/evs/IEvsCamera.h>
#include <aidl/android/hardware/automotive/evs/IEvsEnumerator.h>
#include <aidlcommonsupport/NativeHandle.h>
#include <android-base/logging.h>
#include <android-base/scopeguard.h>
#include <ui/GraphicBuffer.h>

#include <alloca.h>
#include <fcntl.h>
#include <malloc.h>
#include <png.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#ifdef ENABLE_BYPASS_MODE
#include <sys/mman.h> //for PROT_READ | PROT_WRITE, MAP_SHARED
#endif

namespace {

using aidl::android::hardware::automotive::evs::BufferDesc;
using aidl::android::hardware::automotive::evs::IEvsCamera;
using aidl::android::hardware::automotive::evs::IEvsEnumerator;
using aidl::android::hardware::automotive::evs::Stream;
using android::GraphicBuffer;

}  // namespace

VideoTex::VideoTex(std::shared_ptr<IEvsEnumerator> pEnum, std::shared_ptr<IEvsCamera> pCamera,
                   std::shared_ptr<StreamHandler> pStreamHandler, EGLDisplay glDisplay) :
      TexWrapper(),
      mEnumerator(pEnum),
      mCamera(pCamera),
      mStreamHandler(pStreamHandler),
      mDisplay(glDisplay) {

    mStatusCb = ndk::SharedRefBase::make<EvsEnumeratorStatusCallback>();
    if (auto status = pEnum->registerStatusCallback(mStatusCb); !status.isOk()) {
        LOG(ERROR) << "Failed to register status callback ";
    }
#ifdef FPS_PRINT
    startCalculateFps();
#endif
#ifdef ENABLE_BYPASS_MODE
    frame_dump = false;
#endif
}

VideoTex::~VideoTex() {
#ifdef FPS_PRINT
    terminateCalculateFps();
#endif
    // Tell the stream to stop flowing
    mStreamHandler->asyncStopStream();

    // Close the camera
    mEnumerator->closeCamera(mCamera);

    // Drop our device texture image
    if (mKHRimage != EGL_NO_IMAGE_KHR) {
        eglDestroyImageKHR(mDisplay, mKHRimage);
        mKHRimage = EGL_NO_IMAGE_KHR;
    }
}

// Return true if the texture contents are changed
bool VideoTex::refresh() {
    if (!mStreamHandler->newFrameAvailable()) {
        // No new image has been delivered, so there's nothing to do here
        return false;
    }

    // If we already have an image backing us, then it's time to return it
    if (getNativeHandle(mImageBuffer) != nullptr) {
        // Drop our device texture image
        if (mKHRimage != EGL_NO_IMAGE_KHR) {
            eglDestroyImageKHR(mDisplay, mKHRimage);
            mKHRimage = EGL_NO_IMAGE_KHR;
        }

        // Return it since we're done with it
        mStreamHandler->doneWithFrame(mImageBuffer);
    }

    // Get the new image we want to use as our contents
    mImageBuffer = dupBufferDesc(mStreamHandler->getNewFrame());

    // create a GraphicBuffer from the existing handle
    native_handle_t* nativeHandle = getNativeHandle(mImageBuffer);
    const auto handleGuard =
            android::base::make_scope_guard([nativeHandle] { free(nativeHandle); });
    if (nativeHandle == nullptr) {
        // New frame contains an invalid native handle.
        return false;
    }

    const AHardwareBuffer_Desc* pDesc =
            reinterpret_cast<const AHardwareBuffer_Desc*>(&mImageBuffer.buffer.description);
    android::sp<GraphicBuffer> pGfxBuffer =  // AHardwareBuffer_to_GraphicBuffer?
            new GraphicBuffer(nativeHandle, GraphicBuffer::CLONE_HANDLE, pDesc->width,
                              pDesc->height, pDesc->format,
                              1,  // pDesc->layers,
                              GRALLOC_USAGE_HW_TEXTURE, pDesc->stride);
    if (!pGfxBuffer) {
        LOG(ERROR) << "Failed to allocate GraphicBuffer to wrap image handle";
        // Returning "true" in this error condition because we already released the
        // previous image (if any) and so the texture may change in unpredictable ways now!
        return true;
    }
#ifdef ENABLE_BYPASS_MODE
    // Dump first frame for each camera
    if (!frame_dump) {
        if (pGfxBuffer->getNativeBuffer()->handle) {
            //int fd = get_buffer_fd(pGfxBuffer->getNativeBuffer());
            static int count = 0;
            int fd = pGfxBuffer->getNativeBuffer()->handle->data[0];
            if(fd < 0)
                LOG(ERROR) << "Not able to get FD from this handle";
            else
            {
                if(pGfxBuffer->getNativeBuffer()->handle->data[1]<0)
                    LOG(ERROR) << "Not able to get FD 2 from this handle";
                int size = pDesc->height * pDesc->stride * 2; //Currently supports UYVY format - 2 bytes per pixel
                LOG(ERROR) << "dumping video frame height="<< pDesc->height <<" stride=" <<  pDesc->stride;
                void *maped_buf = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                FILE *fp;
                size_t numBytesWritten = 0;
                size_t numByteToWrite = 0;
                unsigned char *pBuf = 0;
                char filename[128];

                snprintf(filename, sizeof(filename), "/data/vendor/camera/evs_app_frame_%d",count);
                if (!maped_buf) {
                    LOG(ERROR) << "buffer is not mapped";
                } else {
                    fp = fopen(filename, "w+");
                    LOG(ERROR) << "dumping video frame "<< filename <<" numbytes " <<  size;
                    if (0 != fp) {
                        pBuf = (unsigned char *)maped_buf;
                        numByteToWrite = size;
                        numBytesWritten = fwrite(pBuf, 1, numByteToWrite, fp);
                        if (numBytesWritten != numByteToWrite)
                            LOG(ERROR) << "error no data written to file";
                        fclose(fp);
                    } else {
                        LOG(ERROR) << "failed to open file";
                    }
                    munmap(maped_buf, size);
                    if(count>30)
                        frame_dump = true;
                    else
                        count++;
                }
            }
        }
    }
#endif

    // Get a GL compatible reference to the graphics buffer we've been given
    EGLint eglImageAttributes[] = {EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_NONE};
    EGLClientBuffer clientBuf = static_cast<EGLClientBuffer>(pGfxBuffer->getNativeBuffer());
    mKHRimage = eglCreateImageKHR(mDisplay, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, clientBuf,
                                  eglImageAttributes);
    if (mKHRimage == EGL_NO_IMAGE_KHR) {
        const char* msg = getEGLError();
        LOG(ERROR) << "Error creating EGLImage: " << msg;
    } else {
        // Update the texture handle we already created to refer to this gralloc buffer
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glId());
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, static_cast<GLeglImageOES>(mKHRimage));

        // Initialize the sampling properties (it seems the sample may not work if this isn't done)
        // The user of this texture may very well want to set their own filtering, but we're going
        // to pay the (minor) price of setting this up for them to avoid the dreaded "black image"
        // if they forget.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

#ifdef FPS_PRINT
    frame_cnt++;
#endif
    return true;
}

#ifdef FPS_PRINT
void VideoTex::calculateFps() {
    // Get the camera id for FPS print
    CameraDesc desc;
    auto status = mCamera->getCameraInfo(&desc);
    if (!status.isOk()) {
        LOG(ERROR) << "Failed to read a camera descriptor";
        return;
    }
    auto cameraId = desc.id;

    while (get_frame_cnt)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(FPS_TIME_INTERVAL_IN_MS));
        LOG(INFO) << " FPS is " << frame_cnt << " for stream id "<< cameraId;
        frame_cnt = 0;
    }
}

bool VideoTex::startCalculateFps() {
    LOG(INFO) << "Starting FPS Calculation";
    // Create the thread and report success if it gets started
    get_frame_cnt = true;
    frame_cnt = 0;
    mCalculateFps = std::thread([this]() { calculateFps(); });
    return mCalculateFps.joinable();
}

void VideoTex::terminateCalculateFps() {
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

VideoTex* createVideoTexture(const std::shared_ptr<IEvsEnumerator>& pEnum, const char* evsCameraId,
                             std::unique_ptr<Stream> streamCfg, EGLDisplay glDisplay,
                             bool useExternalMemory, android_pixel_format_t format) {
    // Set up the camera to feed this texture
    std::shared_ptr<IEvsCamera> pCamera;
    std::shared_ptr<StreamHandler> pStreamHandler;
    if (!streamCfg) {
        LOG(WARNING) << "Given stream configuration is NULL.";
        LOG(INFO) << "Setting default streamconfig parameters";
        std::unique_ptr<Stream> targetCfg(new Stream());
        streamCfg = std::move(targetCfg);
        //currently below parameters are not utilized by HAL
        streamCfg->width = 1920;
        streamCfg->height = 1024;
    }

    if (auto status = pEnum->openCamera(evsCameraId, *streamCfg, &pCamera); !status.isOk()) {
        LOG(ERROR) << "Failed to open a camera " << evsCameraId;
        return nullptr;
    }

    // Initialize the stream that will help us update this texture's contents
    pStreamHandler =
            ndk::SharedRefBase::make<StreamHandler>(pCamera, /* numBuffers= */ 10, useExternalMemory,
                                                    format, streamCfg->width, streamCfg->height);
    if (!pStreamHandler) {
        LOG(ERROR) << "Failed to allocate FrameHandler";
        return nullptr;
    }

    // Start the video stream
    if (!pStreamHandler->startStream()) {
        printf("Couldn't start the camera stream (%s)\n", evsCameraId);
        LOG(ERROR) << "Start stream failed for " << evsCameraId;
        return nullptr;
    }

    return new VideoTex(pEnum, pCamera, pStreamHandler, glDisplay);
}
