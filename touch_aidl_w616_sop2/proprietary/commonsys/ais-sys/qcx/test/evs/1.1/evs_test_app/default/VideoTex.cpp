/*
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a contribution.
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
#include <vector>
#include <stdio.h>
#include <fcntl.h>
#include <alloca.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <malloc.h>
#include <png.h>
#ifdef FPS_PRINT
#include <sys/time.h>
#endif
#ifdef ENABLE_FRAME_DUMP
#include <QtiGralloc.h>
#include <sys/mman.h> //for PROT_READ | PROT_WRITE, MAP_SHARED
#endif

#include "VideoTex.h"
#include "glError.h"

#include <ui/GraphicBuffer.h>
#include <android/hardware/camera/device/3.2/ICameraDevice.h>
#include <android-base/logging.h>

#ifdef ENABLE_FRAME_DUMP
#include <android/hardware/graphics/mapper/4.0/IMapper.h>
using Error_v4       = ::android::hardware::graphics::mapper::V4_0::Error;
using IMapper_v4     = ::android::hardware::graphics::mapper::V4_0::IMapper;
#endif

// Eventually we shouldn't need this dependency, but for now the
// graphics allocator interface isn't fully supported on all platforms
// and this is our work around.
using ::android::GraphicBuffer;
const int numCamBuffers = 10; // 10 buffers because of color conversion

VideoTex::VideoTex(sp<IEvsEnumerator> pEnum,
                   sp<IEvsCamera> pCamera,
                   sp<StreamHandler> pStreamHandler,
                   EGLDisplay glDisplay)
    : TexWrapper()
    , mEnumerator(pEnum)
    , mCamera(pCamera)
    , mStreamHandler(pStreamHandler)
    , mDisplay(glDisplay) {
    // Nothing but initialization here...
#ifdef FPS_PRINT
    frame_count = 0;
    prev_msec = 0;
#endif
#ifdef ENABLE_FRAME_DUMP
    frame_dump = false;
#endif
}

VideoTex::~VideoTex() {
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

#ifdef ENABLE_FRAME_DUMP
/**
 * get_buffer_fd
 *
 * @brief retrieves file descriptor for ANativeWindowBuffer
 *
 * @param window_buf : pointer to ANativeWindowBuffer
 *
 * @return int
*/
static int get_buffer_fd(ANativeWindowBuffer *window_buf)
{
    sp<IMapper_v4> mapper = IMapper_v4::getService();
    int fd = -1;
    mapper->get(const_cast<native_handle_t*>(window_buf->handle), qtigralloc::MetadataType_FD,
        [&](const auto &_error, const auto &_bytestream) {
                if (_error !=  Error_v4::NONE)
                    LOG(ERROR) << "Failed to get FD ";
                else {
                    android::gralloc4::decodeInt32(qtigralloc::MetadataType_FD,
                            _bytestream, &fd);
                }
    });
    return fd;
}
#endif

// Return true if the texture contents are changed
bool VideoTex::refresh() {
    if (!mStreamHandler->newFrameAvailable()) {
        // No new image has been delivered, so there's nothing to do here
        return false;
    }

    // If we already have an image backing us, then it's time to return it
    if (mImageBuffer.buffer.nativeHandle.getNativeHandle() != nullptr) {
        // Drop our device texture image
        if (mKHRimage != EGL_NO_IMAGE_KHR) {
            eglDestroyImageKHR(mDisplay, mKHRimage);
            mKHRimage = EGL_NO_IMAGE_KHR;
        }

        // Return it since we're done with it
        mStreamHandler->doneWithFrame(mImageBuffer);
    }

    // Get the new image we want to use as our contents
    mImageBuffer = mStreamHandler->getNewFrame();


    // create a GraphicBuffer from the existing handle
    const AHardwareBuffer_Desc* pDesc =
        reinterpret_cast<const AHardwareBuffer_Desc *>(&mImageBuffer.buffer.description);
    sp<GraphicBuffer> pGfxBuffer = new GraphicBuffer(mImageBuffer.buffer.nativeHandle,
                                                     GraphicBuffer::CLONE_HANDLE,
                                                     pDesc->width,
                                                     pDesc->height,
                                                     pDesc->format,
                                                     1,//pDesc->layers,
                                                     GRALLOC_USAGE_HW_TEXTURE,
                                                     pDesc->stride);

    if (pGfxBuffer.get() == nullptr) {
        LOG(ERROR) << "Failed to allocate GraphicBuffer to wrap image handle";
        // Returning "true" in this error condition because we already released the
        // previous image (if any) and so the texture may change in unpredictable ways now!
        return true;
    }

#ifdef FPS_PRINT
    uint64_t cur_msec = 0, diff_msec = 0;
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    if (0 == frame_count)
        prev_msec = (current_time.tv_sec*1000) + (current_time.tv_usec/1000);
    frame_count++;
    // If we get 30 FPS we will get 150 frames for every 5 seconds
    if (150 == frame_count) {
        gettimeofday(&current_time, NULL);
        cur_msec = (current_time.tv_sec*1000) + (current_time.tv_usec/1000);
        diff_msec = cur_msec - prev_msec;
        // Get the camera id for FPS print
        std::string cameraId;
        mCamera->getCameraInfo([&cameraId](::android::hardware::automotive::evs::V1_0::CameraDesc desc) {
                cameraId = desc.cameraId;
                }
                );
        if (diff_msec > (uint64_t)5000) {
            double total_frame_rendered = diff_msec * 0.03; // 0.03 frames per 1msec for 30FPS
            double extra_frames = total_frame_rendered / 150.0;
            double fps = 30.0 / extra_frames;
            LOG(ERROR) <<" FPS is "<<fps<<" for stream id "<<cameraId;
        }
        else
            LOG(ERROR) <<"FPS is 30 for Stream Id "<<cameraId;
        // Reset frame count
        frame_count = 0;
        // Save current time
        prev_msec = cur_msec;
    }
#endif //FPS_PRINT
#ifdef ENABLE_FRAME_DUMP
    // Dump first frame for each camera
    if (!frame_dump) {
        if (pGfxBuffer->getNativeBuffer()->handle) {
            int fd = get_buffer_fd(pGfxBuffer->getNativeBuffer());
            int size = pDesc->height * pDesc->stride;
            void *maped_buf = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            FILE *fp;
            size_t numBytesWritten = 0;
            size_t numByteToWrite = 0;
            unsigned char *pBuf = 0;
            char filename[128];
            // Get the camera id for FPS print
            std::string cameraId;
            mCamera->getCameraInfo([&cameraId](::android::hardware::automotive::evs::V1_0::CameraDesc desc) {
                    cameraId = desc.cameraId;
                    }
                    );
            snprintf(filename, sizeof(filename), "/vendor/etc/camera/evs_app_frame_%s", cameraId.c_str());
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
                frame_dump = true;
            }
        }
    }
#endif //ENABLE_FRAME_DUMP

    // Get a GL compatible reference to the graphics buffer we've been given
    EGLint eglImageAttributes[] = {EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_NONE};
    EGLClientBuffer clientBuf = static_cast<EGLClientBuffer>(pGfxBuffer->getNativeBuffer());
    mKHRimage = eglCreateImageKHR(mDisplay, EGL_NO_CONTEXT,
                                  EGL_NATIVE_BUFFER_ANDROID, clientBuf,
                                  eglImageAttributes);

    if (mKHRimage == EGL_NO_IMAGE_KHR) {
        const char *msg = getEGLError();
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
        // TODO:  Can we do this once for the texture ID rather than ever refresh?
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    return true;
}


VideoTex* createVideoTexture(sp<IEvsEnumerator> pEnum,
                             const char* evsCameraId,
                             std::unique_ptr<Stream> streamCfg,
                             EGLDisplay glDisplay,
                             bool useExternalMemory,
                             android_pixel_format_t format) {
    // Set up the camera to feed this texture
    sp<IEvsCamera> pCamera = nullptr;
    sp<StreamHandler> pStreamHandler = nullptr;

    if (streamCfg != nullptr) {
        pCamera = pEnum->openCamera_1_1(evsCameraId, *streamCfg);
        if (pCamera != nullptr)
        // Initialize the stream that will help us update this texture's contents
        pStreamHandler = new StreamHandler(pCamera,
                                           numCamBuffers,     // number of buffers
                                           useExternalMemory,
                                           format,
                                           streamCfg->width,
                                           streamCfg->height);
    } else {
        pCamera =
            IEvsCamera::castFrom(pEnum->openCamera(evsCameraId))
            .withDefault(nullptr);
         if (pCamera != nullptr)
        // Initialize the stream with the default resolution
        pStreamHandler = new StreamHandler(pCamera,
                                           numCamBuffers,     // number of buffers
                                           useExternalMemory,
                                           format);
    }

    if (pCamera == nullptr) {
        LOG(ERROR) << "Failed to allocate new EVS Camera interface for " << evsCameraId;
        return nullptr;
    }

    if (pStreamHandler == nullptr) {
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
