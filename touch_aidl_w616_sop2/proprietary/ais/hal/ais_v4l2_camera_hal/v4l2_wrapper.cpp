/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a contribution.
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

#define MODULE_NAME "V4L2Wrapper"
#define LOG_NDEBUG 0
#include "v4l2_wrapper.h"

#include <algorithm>
#include <array>
#include <limits>
#include <mutex>
#include <vector>
#include <string.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <utility>
#include <libyuv.h>
#include <android-base/unique_fd.h>
#include <stdio.h>
#include <errno.h>
#include <cmath>
#include <unistd.h>

#include <poll.h>
#include "c2d2.h"
#include "arc/cached_frame.h"
#include "arc/exif_utils.h"
#include "arc/jpeg_compressor.h"
#if !defined(ANDROID_Q_AOSP)
#include <QtiGralloc.h>
#else
#include <gralloc_priv.h>
#endif
extern "C" {
#include "jpeglib.h"
}

#include "function_thread.h"
#include "v4l2_camera.h"

#define JPEG_QP 100
#define YUV444_PIXELSIZE 3
#define YUV420_PIXELSIZE 1.5f
#define PREVIEW_BUF_IDX 0
#define SNAPSHOT_BUF_IDX 1
#define BYTESPERPIXEL 4
#define CLAMP(x) ((x)<0 ? 0 : ((x)>255 ? 255: (x)))
#define MIN(a, b) ( ( (a) < (b) ) ? (a) : (b) )
#define MAX(a, b) ( ( (a) > (b) ) ? (a) : (b) )

#define WIDTH_ALIGN 64

// used to allocate snapshot internal buffer
#define MAX_OUTPUT_WIDTH 4096
#define MAX_OUTPUT_HEIGHT 2160

#define STREAM_STATUS_UNINITIALIZED 0
#define STREAM_STATUS_STREAM_OFF    1
#define STREAM_STATUS_STREAM_ON     2

#define ALIGN( num, to ) (((num) + (to-1)) & (~(to-1)))
#define ALIGN8K 8192
#define ALIGN4K 4096
#define ALIGN2K 2048
#define ALIGN512 512
#define ALIGN256 256
#define ALIGN128 128
#define ALIGN64 64
#define ALIGN32 32
#define ALIGN16 16

#define MAX_POLL_TIME 33

namespace v4l2_camera_hal {

using arc::AllocatedFrameBuffer;
using arc::SupportedFormat;
using arc::SupportedFormats;
using default_camera_hal::CaptureRequest;

#if !defined(ANDROID_Q_AOSP)
#include <android/hardware/graphics/mapper/4.0/IMapper.h>
using Error_v4       = ::android::hardware::graphics::mapper::V4_0::Error;
using aidl::android::hardware::graphics::common::PlaneLayout;
using IMapper_v4     = ::android::hardware::graphics::mapper::V4_0::IMapper;
#endif

#ifdef SOFT_SCALE
struct YuvFrame {
    int width;
    int height;
    uint8_t *data;
    uint8_t *y;
    uint8_t *u;
    uint8_t *v;
};
static struct YuvFrame i420_input_frame;
static struct YuvFrame i420_output_frame;
const int32_t kSupportedResolution[][2] = {
    { 640,  480}, // VGA
    { 320,  240}, // QVGA
};
#else
const int32_t kSupportedResolution[][2] = {
    {3840, 2160}, // 4KUHD (for USB camera)
    {2560, 1440}, // QHD
    {1920, 1440}, // QHD
    {1920, 1080}, // HD1080
    {1280,  720}, // HD
    { 640,  480}, // VGA
    { 320,  240}, // QVGA
};
C2D_DRIVER_SETUP_INFO set_driver_op = {
    .max_surface_template_needed = 10 };
#endif

const int32_t kStandardSizes[][2] = {
    {4096, 2160}, // 4KDCI (for USB camera)
    {3840, 2160}, // 4KUHD (for USB camera)
    {3280, 2464}, // 8MP
    {2560, 1440}, // QHD
    {1920, 1080}, // HD1080
    {1640, 1232}, // 2MP
    {1280,  720}, // HD
    {1024,  768}, // XGA
    { 640,  480}, // VGA
    { 320,  240}, // QVGA
    { 176,  144}  // QCIF
};

#define TARGET_SCISSOR  0
#define TARGET_MASK_ID  0
#define TARGET_COLOR_KEY  0
#define NO_OF_OBJECTS    1

#define KGSL_USER_MEM_TYPE_PMEM   0
#define KGSL_USER_MEM_TYPE_ASHMEM 1
#define KGSL_USER_MEM_TYPE_ADDR   2
#define KGSL_USER_MEM_TYPE_ION    3

/**
  * 3A constants
  */
const int64_t USEC = 1000LL;
const int64_t MSEC = USEC * 1000LL;
// Default exposure and gain targets for different scenarios
const nsecs_t V4L2Wrapper::kNormalExposureTime       = 10 * MSEC;
const nsecs_t V4L2Wrapper::kFacePriorityExposureTime = 30 * MSEC;
const int     V4L2Wrapper::kNormalSensitivity        = 100;
const int     V4L2Wrapper::kFacePrioritySensitivity  = 400;
// CTS requires 8 frames timeout in waitForAeStable
const float   V4L2Wrapper::kExposureTrackRate        = 0.2;
const int     V4L2Wrapper::kPrecaptureMinFrames      = 10;
const int     V4L2Wrapper::kStableAeMaxFrames        = 100;
const float   V4L2Wrapper::kExposureWanderMin        = -2;
const float   V4L2Wrapper::kExposureWanderMax        = 1;
const nsecs_t V4L2Wrapper::kminExposureTime          = V4L2Wrapper::kNormalExposureTime / 100;

v4l2_buffer device_buffer;
int q_status = 0;
static uint32_t mClientCnt =0;
static bool mQcarcamInit = false;
static std::map<qcarcam_hndl_t, V4L2Wrapper*> gClientMap;
static std::mutex gClientMapMutex;

static char const* printHalFormat(uint32_t halFormat) {
    switch (halFormat) {
        case HAL_PIXEL_FORMAT_CbYCrY_422_I:
            return "HAL_PIXEL_FORMAT_CbYCrY_422_I";
        case HAL_PIXEL_FORMAT_YCBCR_422_I:
            return "HAL_PIXEL_FORMAT_YCBCR_422_I";
        case HAL_PIXEL_FORMAT_YCbCr_420_SP:
            return "HAL_PIXEL_FORMAT_YCbCr_420_SP";
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            return "HAL_PIXEL_FORMAT_YCrCb_420_SP";
        case HAL_PIXEL_FORMAT_NV12_ENCODEABLE:
            return "HAL_PIXEL_FORMAT_NV12_ENCODEABLE";
        case HAL_PIXEL_FORMAT_YCbCr_420_SP_TILED:
            return "HAL_PIXEL_FORMAT_YCbCr_420_SP_TILED";
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
            return "HAL_PIXEL_FORMAT_YCbCr_420_888";
        case HAL_PIXEL_FORMAT_RGB_565:
            return "HAL_PIXEL_FORMAT_RGB_565";
        case HAL_PIXEL_FORMAT_RGB_888:
            return "HAL_PIXEL_FORMAT_RGB_888";
        case HAL_PIXEL_FORMAT_RGBX_8888:
            return "HAL_PIXEL_FORMAT_RGBX_8888";
        case HAL_PIXEL_FORMAT_RGBA_8888:
            return "HAL_PIXEL_FORMAT_RGBA_8888";
        case HAL_PIXEL_FORMAT_BGRA_8888:
            return "HAL_PIXEL_FORMAT_BGRA_8888";
        case HAL_PIXEL_FORMAT_RGBA_5551:
            return "HAL_PIXEL_FORMAT_RGBA_5551";
        case HAL_PIXEL_FORMAT_RGBA_4444:
            return "HAL_PIXEL_FORMAT_RGBA_4444";
        case HAL_PIXEL_FORMAT_BLOB:
            return "HAL_PIXEL_FORMAT_BLOB";
        default:
            HAL_LOGE("Unsupported HAL format: 0x%x", halFormat);
    }
    return "Unsupported HAL format";
}

#ifdef SOFT_SCALE
/* @Brief: Function to software scale the resolution using libyuv.
  * @params: Source buffer pointer.
  *          Destination buffer pointer
  *          width and height.
  * @return: NULL
  */
int V4L2Wrapper::Nv21Scale(unsigned char* uvInputPixels, unsigned char* yInputPointer, void * pOutBuf, unsigned char* uvOutputPixels, int OUTWIDTH, int OUTHEIGHT)
{
    int y_size = SENSOR_RAWHEIGHT * SENSOR_RAWWIDTH;
    int out_size = OUTWIDTH * OUTHEIGHT;

    i420_input_frame.width = SENSOR_RAWWIDTH;
    i420_input_frame.height = SENSOR_RAWHEIGHT;
    i420_input_frame.y = yInputPointer;
    i420_input_frame.u = uvInputPixels;
    i420_input_frame.v = i420_input_frame.u + y_size / 4;

    i420_output_frame.width = OUTWIDTH;
    i420_output_frame.height = OUTHEIGHT;
    i420_output_frame.data = (uint8_t *)pOutBuf;
    i420_output_frame.y = i420_output_frame.data;
    i420_output_frame.u = uvOutputPixels;
    i420_output_frame.v = i420_output_frame.u + out_size / 4;

    int result = libyuv::I420Scale(i420_input_frame.y, i420_input_frame.width,
            i420_input_frame.u, i420_input_frame.width / 2,
            i420_input_frame.v, i420_input_frame.width / 2,
            i420_input_frame.width, i420_input_frame.height,
            i420_output_frame.y, i420_output_frame.width,
            i420_output_frame.u, i420_output_frame.width / 2,
            i420_output_frame.v, i420_output_frame.width / 2,
            i420_output_frame.width, i420_output_frame.height,
            libyuv::FilterMode::kFilterNone);
    return result;
}
#endif

/* @Brief: Function to Encode YUV raw to jpg frame
  * @Params: pOutBuf- Contains the buffer pointer to be filled
  *          and the memory is allocate by calling function.
  *          pInputPixel- Contains the raw buffer pointer
  *          imagewidth- Width of the desired encoded file
  *          imageHeight- Height of the desired encoded file
  *          pJpegLength- Size of the encoded jpeg frame
  * @return: NULL
  */
void V4L2Wrapper::JpegEncode(void * pOutBuf, unsigned char* pInputPixels, int imageWidth, int imageHeight, unsigned long *pJpegLen)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1]; /* pointer to JSAMPLE row[s] */
    int row_stride; /* physical row width in image buffer */
    uint8_t* outbuffer = (uint8_t*)pOutBuf;
    unsigned long jpeg_output_len = *pJpegLen;

    cinfo.err = jpeg_std_error(&jerr);
    /* Now we can initialize the JPEG compression object. */
    jpeg_create_compress(&cinfo);
    jpeg_mem_dest(&cinfo, &outbuffer, &jpeg_output_len);
    cinfo.image_width = imageWidth; /* image width and height, in pixels */
    cinfo.image_height = imageHeight;
    cinfo.input_components = YUV444_PIXELSIZE; /* # of color components per pixel */
    cinfo.in_color_space = JCS_RGB; // JCS_YCbCr; /* colorspace of input image */
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, JPEG_QP, TRUE /* limit to baseline-JPEG values */);

    jpeg_start_compress(&cinfo, TRUE);

    row_stride = imageWidth * cinfo.input_components; /* JSAMPLEs per row in imageBuffer ie 3 component Y,U,V */

    while (cinfo.next_scanline < cinfo.image_height) {
        /* jpeg_write_scanlines expects an array of pointers to scanlines.
          * Here the array is only one element long, but you could pass
          * more than one scanline at a time if that's more convenient.
          * */
        if (pInputPixels != NULL) {
            row_pointer[0] = & pInputPixels[cinfo.next_scanline * row_stride];
            (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }
        else {
            HAL_LOGE("Fail to raw buffer\n");
            goto destroy;
        }
    }

    jpeg_finish_compress(&cinfo);
    if (jpeg_output_len > *pJpegLen) {
        HAL_LOGE("ERROR: Jpeg data:%ld is more than output buffer:%ld, so failed\n",
                jpeg_output_len, *pJpegLen);
        // when jpeg data is more than output buffer size, then libjpeg will malloc buffer,
        // application need free this buffer.
        free(outbuffer);
        *pJpegLen = 0;
    } else  {
        *pJpegLen = jpeg_output_len;
    }
destroy:
    /* release JPEG compression object */
    jpeg_destroy_compress(&cinfo);
}

V4L2Wrapper* V4L2Wrapper::NewV4L2Wrapper(int id, const V4L2CameraHAL::QCarCamInfo& qcarcam_info) {
    return new V4L2Wrapper(id, qcarcam_info);
}

V4L2Wrapper::V4L2Wrapper(int id_, const V4L2CameraHAL::QCarCamInfo qcarcam_info_)
    : connection_count_(0), first_frame(true),
    streamstatus(STREAM_STATUS_UNINITIALIZED),
    cam_id(id_), mAisFrameWidth(qcarcam_info_.width),
    mAisFrameHeight(qcarcam_info_.height), mAisFrameFormat(qcarcam_info_.color_fmt),

    mBufferDequeueThread(new FunctionThread(
            std::bind(&V4L2Wrapper::DequeueRequest, this))) {
    HAL_LOG_ENTER();
    memset(input_buffer_infos, 0, sizeof(buffer_info_t) * MAX_INPUT_BUFFER_NUM);
    memset(output_buffer_infos, 0, sizeof(buffer_info_t) * MAX_OUTPUT_BUFFER_NUM);
    memset(&capture_buffer_info, 0, sizeof(buffer_info_t));

    snapshot_output_buffer = new GraphicBuffer(MAX_OUTPUT_WIDTH, MAX_OUTPUT_HEIGHT,
            HAL_PIXEL_FORMAT_RGB_888,
            GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN);
    if (snapshot_output_buffer == nullptr) {
        HAL_LOGE("ERROR: new GraphicBuffer failed, out of memory");
    }

    mHalFrameFormat = getQcarcamToHalFormat(mAisFrameFormat);
    mInputWidth = 0;
    mInputHeight = 0;
    mInputHalFormat = 0;
    mapper = IMapper_v4::getService();

    mOutBufFormat = HAL_PIXEL_FORMAT_RGBA_8888;
    mpGfxBufs = nullptr;
    mpQcarcamMmapBufs = nullptr;
    memset(&mQcarcamOutBufs, 0x0, sizeof(mQcarcamOutBufs));
    memset(&mQcarcamFrameInfo, 0x0, sizeof(mQcarcamFrameInfo));

#ifndef SOFT_SCALE
    C2D_STATUS c2d_status = c2dDriverInit(&set_driver_op);
    if(c2d_status != C2D_STATUS_OK)
    {
        HAL_LOGE("\n c2dDriverInit failed \n");
    }
    else
        HAL_LOGV("c2D Driver init success");
#endif

    mFrameNum = 0;

#ifdef FPS_PRINT
    mFrameCnt = 0;
    mPrevMsec = 0;
#endif

    if (!mBufferDequeueThread->isRunning()) {
        std::string threadName = "DQBUF:";
        std::string videoName =  "video";
        android::status_t res = mBufferDequeueThread->run(threadName.c_str());
        if (res != android::OK) {
            HAL_LOGE("Failed to start DEQUEUE thread: %d", res);
        }
    }

    HAL_LOG_EXIT();
}

V4L2Wrapper::~V4L2Wrapper() {
#ifndef SOFT_SCALE
    ReleaseInputSurfaces();
    ReleaseOutputSurfaces();

    c2dDriverDeInit();
#endif
    snapshot_output_buffer.clear();
}

int V4L2Wrapper::Connect() {
    HAL_LOG_ENTER();

    std::lock_guard<std::mutex> lock(connection_lock_);

    // if (mInputWidth == 0) {
    //   int32_t ret = GetInputInformation();
    //   if (ret < 0) {
    //     // Do not return error, otherwise the camera provider would not connect to HAL at startup.
    //     // Failure at this step is not critical and GetInputInformation() can be called on Init().
    //     // This will ensure that format and dimesnsions fields are initialized before they are used.
    //     HAL_LOGI("Get input information failed");
    //   }
    // }

    // // Check if this connection has the extended control query capability.
    // v4l2_query_ext_ctrl query;
    // query.id = V4L2_CTRL_FLAG_NEXT_CTRL | V4L2_CTRL_FLAG_NEXT_COMPOUND;

    // //TODO: Enable the below statement and remove hardcoding after get camera implments and get it from v4l2 driver.
    // //extended_query_supported_ = (IoctlLocked(VIDIOC_QUERY_EXT_CTRL, &query) == 0);
    // extended_query_supported_ = 0;

    // // TODO(b/29185945): confirm this is a supported device.
    // // This is checked by the HAL, but the device at device_path_ may
    // // not be the same one that was there when the HAL was loaded.
    // // (Alternatively, better hotplugging support may make this unecessary
    // // by disabling cameras that get disconnected and checking newly connected
    // // cameras, so Connect() is never called on an unsupported camera)

    // supported_formats_ = GetSupportedFormats();
    // qualified_formats_ = StreamFormat::GetQualifiedFormats(supported_formats_);

    HAL_LOG_EXIT();
    return 0;
}

void V4L2Wrapper::Disconnect() {
    HAL_LOG_ENTER();

    std::lock_guard<std::mutex> lock(connection_lock_);
    HAL_LOG_EXIT();
}

// Helper function. Should be used instead of ioctl throughout this class.
template <typename T>
int V4L2Wrapper::IoctlLocked(int request, T data) {
    //TODO: V4l2 extended control command implementation.
    //if (VIDIOC_ENUM_FRAMESIZES == request || VIDIOC_QUERY_EXT_CTRL == request || VIDIOC_G_EXT_CTRLS  == request || VIDIOC_G_CTRL == request || VIDIOC_S_EXT_CTRLS == request|| VIDIOC_S_CTRL == request)
    //    return 1;
    // Potentially called so many times logging entry is a bad idea.
    if (-1069263324 == request) {
        HAL_LOGE("failed: request == -1069263324");
        return -1;
    }
    std::lock_guard<std::mutex> lock(device_lock_);

    if (!connected()) {
        HAL_LOGE("Device %s not connected.", device_path_.c_str());
        return -ENODEV;
    }

    return TEMP_FAILURE_RETRY(ioctl(device_fd_.get(), request, data));
}

/*
  * Breif: V4L2Wrapper::V4l2Read function is no longer in use but can
  * be use for debugging in future. So commenting it out.
  */
#ifdef V4L2_READ
int V4L2Wrapper::V4l2Read(unsigned char *ptr, int size) {
    std::lock_guard<std::mutex> lock(device_lock_);

    if (!connected()) {
        HAL_LOGE("Device %s not connected.", device_path_.c_str());
        return -ENODEV;
    }
    return read(device_fd_.get(), ptr, size);
}
#endif

void V4L2Wrapper::v4l2_ais_event_cb(const qcarcam_hndl_t hndl, const qcarcam_event_t event_id, const qcarcam_event_payload_t *p_payload)
{
    qcarcam_event_msg_t event_msg;
    event_msg.event_id = event_id;
    memcpy(&event_msg.payload, p_payload, sizeof(qcarcam_event_payload_t));

    std::unique_lock<std::mutex> lk(gClientMapMutex);
    qcarcam_test_input_t &input_ctxt = (gClientMap[hndl])->input_ctxt;

    std::unique_lock<std::mutex> lock(input_ctxt.queue_mutex);
    input_ctxt.eventqueue.push(event_msg);
    lock.unlock();
    input_ctxt.eventHandlerSignal.notify_one();
    switch (event_id)
    {
        case QCARCAM_EVENT_FRAME_READY:
            HAL_LOGD("Frame Ready is available");
            break;
        case QCARCAM_EVENT_ERROR:
            HAL_LOGD("Frame Event Error");
            break;
        default:
            HAL_LOGD("Unsupported Event");
            break;
    }
}

int V4L2Wrapper::StreamOn() {

    HAL_LOG_ENTER();

    std::lock_guard<std::mutex> guard(mRequestQueueLock);
    std::unique_lock<std::mutex> lock(streamStatusLock);
    if (streamstatus == STREAM_STATUS_STREAM_ON) {
        HAL_LOGE("Already streaming , skip this request");
        lock.unlock();
        return 0;
    }

    int ret = 0;
    qcarcam_param_value_t param;
    param.ptr_value = (void *)v4l2_ais_event_cb;
    ret = qcarcam_s_param(qcarcam_context, QCARCAM_PARAM_EVENT_CB, &param);
    if (ret != QCARCAM_RET_OK) {
        HAL_LOGE("qcarcam_s_param failed ret = %d", ret);
        goto EXIT_FLAG;
    }
    //Default qcarcam events
    // To enable self-recovery, call qcarcam_s_param with QCARCAM_PARAM_RECOVERY
    param.uint_value = (QCARCAM_EVENT_FRAME_READY | QCARCAM_EVENT_ERROR |
                        QCARCAM_EVENT_RECOVERY | QCARCAM_EVENT_RECOVERY_SUCCESS | QCARCAM_EVENT_FRAME_DROP);
    ret = qcarcam_s_param(qcarcam_context, QCARCAM_PARAM_EVENT_MASK, &param);
    if (ret != QCARCAM_RET_OK) {
        HAL_LOGE("qcarcam_s_param failed ret = %d", ret);
        goto EXIT_FLAG;
    }
    else
    {
        HAL_LOGV("qcarcam_s_param success");
    }

    ret = qcarcam_start(qcarcam_context);
    if (ret != QCARCAM_RET_OK) {
        HAL_LOGE("qcarcam_start() failed");
        goto EXIT_FLAG;
    } else
        HAL_LOGE("qcarcam_start() success for cam_id %d ",cam_id);

    streamstatus = STREAM_STATUS_STREAM_ON;
    lock.unlock();
    streamStatusSignal.notify_all();

    // wakeup dequeue request thread
    mRequestQueueAvailable.notify_one();
    HAL_LOG_EXIT();
    return 0;

EXIT_FLAG:
    qcarcam_close(qcarcam_context);
    std::unique_lock<std::mutex> lk(gClientMapMutex);
    gClientMap.erase(qcarcam_context);
    qcarcam_context = NULL;
    mClientCntStatus();
    lock.unlock();
    return -1;
}

int V4L2Wrapper::StreamOff() {
    HAL_LOG_ENTER();
    qcarcam_ret_t ret = QCARCAM_RET_OK;

    std::unique_lock<std::mutex> lock(streamStatusLock);
    if (streamstatus == STREAM_STATUS_STREAM_OFF) {
        HAL_LOGE("Not streaming, skiping this request");
        lock.unlock();
        return 0;
    }

    ret = qcarcam_stop(qcarcam_context);
    if (QCARCAM_RET_OK != ret) {
        lock.unlock();
        HAL_LOGE("qcarcam_stop() failed , returns %d ", ret);
        return 0;
    }
    else
    HAL_LOGV("qcarcam_stop() success , returns %d ", ret);

    flush();
    streamstatus = STREAM_STATUS_STREAM_OFF;
    lock.unlock();
    streamStatusSignal.notify_all();

    ret = qcarcam_close(qcarcam_context);
    if (QCARCAM_RET_OK != ret) {
        HAL_LOGE("qcarcam_close() failed , returns %d ", ret);
        return 0;
    } else {
        std::unique_lock<std::mutex> lk(gClientMapMutex);
        gClientMap.erase(qcarcam_context);
        mClientCntStatus();
        qcarcam_context = NULL;
        HAL_LOGD("qcarcam_close() success , returns %d ", ret);
    }

    //Added to resolve OpenCamera freeze issue when trying to reopen the app
    std::map<std::uint64_t, void *>::iterator it;
    for (it = mSrcGpuMapList.begin(); it != mSrcGpuMapList.end(); ++it) {
        c2dUnMapAddr(it->second);
    }
    mSrcGpuMapList.clear();

    for (int i = 0; i < (int)mQcarcamOutBufs.n_buffers; ++i) {
        /* Destroy C2D Source Surface */
        c2dDestroySurface(mpQcarcamMmapBufs[i].srcC2dSurfaceId);
        if (mpQcarcamMmapBufs && mpQcarcamMmapBufs[i].ptr)
        {
            munmap(mpQcarcamMmapBufs[i].ptr, mpQcarcamMmapBufs[i].size);
            // close(mQcarcamOutBufs.buffers[i].planes[0].p_buf);
            // mpQcarcamMmapBufs[i].fd = 0;
            int fd = get_handle_fd(mpGfxBufs[i]->getNativeBuffer()->handle);
            close(fd);
            fd = 0;
        }
        if (mpGfxBufs && mpGfxBufs[i])
            mpGfxBufs[i] = nullptr; //No need to delete sp<GraphicBuffer>
        HAL_LOGV("destroy & munmap success for buffer no. %d of %d ", i,(int)mQcarcamOutBufs.n_buffers);
    }
    if (mpGfxBufs)
        free(mpGfxBufs);
    if (mQcarcamOutBufs.buffers)
        free(mQcarcamOutBufs.buffers);
    if (mpQcarcamMmapBufs)
        free(mpQcarcamMmapBufs);
    memset(&mQcarcamOutBufs, 0x0, sizeof(mQcarcamOutBufs));

    HAL_LOG_EXIT();
    return 0;
}

int V4L2Wrapper::GetInputInformation() {
    struct v4l2_format k_format;
    int ret = 0;

    mInputWidth = mAisFrameWidth;
    mInputHeight = mAisFrameHeight;

    // Setting No crop value

    mInputCropLeft = MIN(0, mInputWidth - 1);
    mInputCropTop = MIN(0, mInputHeight - 1);
    mInputCropWidth = MIN(mInputWidth, mInputWidth - mInputCropLeft);
    mInputCropHeight = MIN(mInputHeight, mInputHeight - mInputCropTop);

    mInputHalFormat = mHalFrameFormat;
    HAL_LOGE("input width:%d, input height:%d, hal format %s, mInputCropWidth:%d,"
            "mInputCropHeight:%d, mInputCropLeft:%d, mInputCropTop:%d",
            mInputWidth, mInputHeight, printHalFormat(mInputHalFormat), mInputCropWidth,
            mInputCropHeight, mInputCropLeft, mInputCropTop);
    return 0;
}

int V4L2Wrapper::GetSensorWidth() {
    return mInputWidth;
}

int V4L2Wrapper::GetSensorHeight() {
    return mInputHeight;
}

uint32_t V4L2Wrapper::ConvertToHalFormat(uint32_t kernelFormat) {
    switch(kernelFormat) {
        case V4L2_PIX_FMT_UYVY:
            return HAL_PIXEL_FORMAT_CbYCrY_422_I;
        case V4L2_PIX_FMT_YUYV:
            return HAL_PIXEL_FORMAT_YCbCr_422_I;
        case V4L2_PIX_FMT_YVU420:
            return HAL_PIXEL_FORMAT_YV12;
        case V4L2_PIX_FMT_YUV420:
            return HAL_PIXEL_FORMAT_YCbCr_420_888;
        case V4L2_PIX_FMT_NV21:
            return HAL_PIXEL_FORMAT_YCrCb_420_SP;
        case V4L2_PIX_FMT_NV12:
            return HAL_PIXEL_FORMAT_YCbCr_420_SP;
        default:
            HAL_LOGE("ERROR: do not support this kernel format:0x%x", kernelFormat);
            return V4L2_PIX_FMT_UYVY;
    }

    return V4L2_PIX_FMT_UYVY;
}

int V4L2Wrapper::QueryControl(uint32_t control_id,
        v4l2_query_ext_ctrl* result) {
#ifndef KERNEL_VERSION4_14
    //VIDIOC_QUERYCTRL is not supported anymore
    return -ENODEV;
#else
    int res;

    memset(result, 0, sizeof(*result));

    if (extended_query_supported_) {
        result->id = control_id;
        res = IoctlLocked(VIDIOC_QUERY_EXT_CTRL, result);
        // Assuming the operation was supported (not ENOTTY), no more to do.
        if (errno != ENOTTY) {
            if (res) {
                HAL_LOGE("QUERY_EXT_CTRL fails: %s", strerror(errno));
                return -ENODEV;
            }
            return 0;
        }
    }

    // Extended control querying not supported, fall back to basic control query.
    v4l2_queryctrl query;
    query.id = control_id;
    if (IoctlLocked(VIDIOC_QUERYCTRL, &query)) {
        HAL_LOGE("QUERYCTRL fails: %s", strerror(errno));
        return -ENODEV;
    }

    // Convert the basic result to the extended result.
    result->id = query.id;
    result->type = query.type;
    memcpy(result->name, query.name, sizeof(query.name));
    result->minimum = query.minimum;
    if (query.type == V4L2_CTRL_TYPE_BITMASK) {
        // According to the V4L2 documentation, when type is BITMASK,
        // max and default should be interpreted as __u32. Practically,
        // this means the conversion from 32 bit to 64 will pad with 0s not 1s.
        result->maximum = static_cast<uint32_t>(query.maximum);
        result->default_value = static_cast<uint32_t>(query.default_value);
    } else {
        result->maximum = query.maximum;
        result->default_value = query.default_value;
    }
    result->step = static_cast<uint32_t>(query.step);
    result->flags = query.flags;
    result->elems = 1;
    switch (result->type) {
        case V4L2_CTRL_TYPE_INTEGER64:
            result->elem_size = sizeof(int64_t);
            break;
        case V4L2_CTRL_TYPE_STRING:
            result->elem_size = result->maximum + 1;
            break;
        default:
            result->elem_size = sizeof(int32_t);
            break;
    }

    return 0;
#endif
}

int V4L2Wrapper::GetControl(uint32_t control_id, int32_t* value) {
    // For extended controls (any control class other than "user"),
    // G_EXT_CTRL must be used instead of G_CTRL.
    if (V4L2_CTRL_ID2CLASS(control_id) != V4L2_CTRL_CLASS_USER) {
        v4l2_ext_control control;
        v4l2_ext_controls controls;
        memset(&control, 0, sizeof(control));
        memset(&controls, 0, sizeof(controls));

        control.id = control_id;
        controls.ctrl_class = V4L2_CTRL_ID2CLASS(control_id);
        controls.count = 1;
        controls.controls = &control;

        if (IoctlLocked(VIDIOC_G_EXT_CTRLS, &controls) < 0) {
            HAL_LOGE("G_EXT_CTRLS fails: %s", strerror(errno));
            return -ENODEV;
        }
        *value = control.value;
    } else {
        v4l2_control control{control_id, 0};
        if (IoctlLocked(VIDIOC_G_CTRL, &control) < 0) {
            HAL_LOGE("G_CTRL fails: %s", strerror(errno));
            return -ENODEV;
        }
        *value = control.value;
    }
    return 0;
}

int V4L2Wrapper::SetControl(uint32_t control_id,
        int32_t desired,
        int32_t* result) {
    int32_t result_value = 0;

    // TODO(b/29334616): When async, this may need to check if the stream
    // is on, and if so, lock it off while setting format. Need to look
    // into if V4L2 supports adjusting controls while the stream is on.

    // For extended controls (any control class other than "user"),
    // S_EXT_CTRL must be used instead of S_CTRL.
    if (V4L2_CTRL_ID2CLASS(control_id) != V4L2_CTRL_CLASS_USER) {
        v4l2_ext_control control;
        v4l2_ext_controls controls;
        memset(&control, 0, sizeof(control));
        memset(&controls, 0, sizeof(controls));

        control.id = control_id;
        control.value = desired;
        controls.ctrl_class = V4L2_CTRL_ID2CLASS(control_id);
        controls.count = 1;
        controls.controls = &control;

        if (IoctlLocked(VIDIOC_S_EXT_CTRLS, &controls) < 0) {
            HAL_LOGE("S_EXT_CTRLS fails: %s", strerror(errno));
            return -ENODEV;
        }
        result_value = control.value;
    } else {
        v4l2_control control{control_id, desired};
        if (IoctlLocked(VIDIOC_S_CTRL, &control) < 0) {
            HAL_LOGE("S_CTRL fails: %s", strerror(errno));
            return -ENODEV;
        }
        result_value = control.value;
    }

    // If the caller wants to know the result, pass it back.
    if (result != nullptr) {
        *result = result_value;
    }
    return 0;
}

const SupportedFormats V4L2Wrapper::GetSupportedFormats() {
    SupportedFormats formats;
    std::set<uint32_t> pixel_formats;
    int res = GetFormats(&pixel_formats);
    if (res) {
        HAL_LOGE("Failed to get device formats.");
        return formats;
    }

    arc::SupportedFormat supported_format;
    std::set<std::array<int32_t, 2>> frame_sizes;

    for (auto pixel_format : pixel_formats) {
        supported_format.fourcc = pixel_format;

        frame_sizes.clear();
        res = GetFormatFrameSizes(pixel_format, &frame_sizes);
        if (res) {
            HAL_LOGE("Failed to get frame sizes for format: 0x%x", pixel_format);
            continue;
        }
        for (auto frame_size : frame_sizes) {
            supported_format.width = frame_size[0];
            supported_format.height = frame_size[1];
            formats.push_back(supported_format);
        }
    }
    return formats;
}

int V4L2Wrapper::GetFormats(std::set<uint32_t>* v4l2_formats) {
    HAL_LOG_ENTER();

    v4l2_fmtdesc format_query;
    memset(&format_query, 0, sizeof(format_query));
    format_query.pixelformat = V4L2_PIX_FMT_UYVY;
    v4l2_formats->insert(format_query.pixelformat);
    ++format_query.index;
#if 0
    // TODO(b/30000211): multiplanar support.
    format_query.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while (IoctlLocked(VIDIOC_ENUM_FMT, &format_query) >= 0) {
        v4l2_formats->insert(format_query.pixelformat);
        ++format_query.index;
    }

    if (errno != EINVAL) {
        HAL_LOGE("ENUM_FMT fails at index %d: %s",
                format_query.index, strerror(errno));
        return -ENODEV;
    }
#endif
    return 0;
}

int V4L2Wrapper::GetQualifiedFormats(std::vector<uint32_t>* v4l2_formats) {
    HAL_LOG_ENTER();
    if (!connected()) {
        HAL_LOGE(
                "Device is not connected, qualified formats may not have been set.");
        return -EINVAL;
    }
    v4l2_formats->clear();
    std::set<uint32_t> unique_fourccs;
    for (auto& format : qualified_formats_) {
        unique_fourccs.insert(format.fourcc);
    }
    v4l2_formats->assign(unique_fourccs.begin(), unique_fourccs.end());
    return 0;
}

int V4L2Wrapper::GetFormatFrameSizes(uint32_t v4l2_format,
        std::set<std::array<int32_t, 2>>* sizes) {
    sizes->insert({{{static_cast<int32_t>(mAisFrameWidth),
            static_cast<int32_t>(mAisFrameHeight)}}});

    for (const auto size : kSupportedResolution) {
        sizes->insert({{{size[0], size[1]}}});
    }
#if 0
    v4l2_frmsizeenum size_query;
    memset(&size_query, 0, sizeof(size_query));
    size_query.pixel_format = v4l2_format;
    if (IoctlLocked(VIDIOC_ENUM_FRAMESIZES, &size_query) < 0) {
        HAL_LOGE("ENUM_FRAMESIZES failed: %s", strerror(errno));
        return -ENODEV;
    }
    if (size_query.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
        // Discrete: enumerate all sizes using VIDIOC_ENUM_FRAMESIZES.
        // Assuming that a driver with discrete frame sizes has a reasonable number
        // of them.
        do {
            sizes->insert({{{static_cast<int32_t>(size_query.discrete.width),
                    static_cast<int32_t>(size_query.discrete.height)}}});
            ++size_query.index;
        } while (IoctlLocked(VIDIOC_ENUM_FRAMESIZES, &size_query) >= 0);
        for (const auto size : kSupportedResolution) {
            sizes->insert({{{size[0], size[1]}}});
            ++size_query.index;
        }
        if (errno != EINVAL) {
            HAL_LOGE("ENUM_FRAMESIZES fails at index %d: %s",
                    size_query.index,
                    strerror(errno));
            return -ENODEV;
        }
    } else {
        // Continuous/Step-wise: based on the stepwise struct returned by the query.
        // Fully listing all possible sizes, with large enough range/small enough
        // step size, may produce far too many potential sizes. Instead, find the
        // closest to a set of standard sizes.
        for (const auto size : kStandardSizes) {
            // Find the closest size, rounding up.
            uint32_t desired_width = size[0];
            uint32_t desired_height = size[1];
            if (desired_width < size_query.stepwise.min_width ||
                    desired_height < size_query.stepwise.min_height) {
                HAL_LOGV("Standard size %u x %u is too small for format %d",
                        desired_width,
                        desired_height,
                        v4l2_format);
                continue;
            } else if (desired_width > size_query.stepwise.max_width ||
                    desired_height > size_query.stepwise.max_height) {
                HAL_LOGV("Standard size %u x %u is too big for format %d",
                        desired_width,
                        desired_height,
                        v4l2_format);
                continue;
            }

            // Round up.
            uint32_t width_steps = (desired_width - size_query.stepwise.min_width +
                    size_query.stepwise.step_width - 1) /
                size_query.stepwise.step_width;
            uint32_t height_steps = (desired_height - size_query.stepwise.min_height +
                    size_query.stepwise.step_height - 1) /
                size_query.stepwise.step_height;
            sizes->insert(
                    {{{static_cast<int32_t>(size_query.stepwise.min_width +
                            width_steps * size_query.stepwise.step_width),
                    static_cast<int32_t>(size_query.stepwise.min_height +
                            height_steps *
                            size_query.stepwise.step_height)}}});
        }
    }
#endif
    return 0;
}

// Converts a v4l2_fract with units of seconds to an int64_t with units of ns.
inline int64_t FractToNs(const v4l2_fract& fract) {
    return (1000000000LL * fract.numerator) / fract.denominator;
}

int V4L2Wrapper::GetFormatFrameDurationRange(
        uint32_t v4l2_format,
        const std::array<int32_t, 2>& size,
        std::array<int64_t, 2>* duration_range) {
    // Potentially called so many times logging entry is a bad idea.

    v4l2_frmivalenum duration_query;
    memset(&duration_query, 0, sizeof(duration_query));
    duration_query.pixel_format = v4l2_format;
    duration_query.width = size[0];
    duration_query.height = size[1];
    if (IoctlLocked(VIDIOC_ENUM_FRAMEINTERVALS, &duration_query) < 0) {
        HAL_LOGE("ENUM_FRAMEINTERVALS failed: %s", strerror(errno));
        return -ENODEV;
    }

    int64_t min = std::numeric_limits<int64_t>::max();
    int64_t max = std::numeric_limits<int64_t>::min();
    if (duration_query.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
        // Discrete: enumerate all durations using VIDIOC_ENUM_FRAMEINTERVALS.
        do {
            min = std::min(min, FractToNs(duration_query.discrete));
            max = std::max(max, FractToNs(duration_query.discrete));
            ++duration_query.index;
        } while (IoctlLocked(VIDIOC_ENUM_FRAMEINTERVALS, &duration_query) >= 0);
        if (errno != EINVAL) {
            HAL_LOGE("ENUM_FRAMEINTERVALS fails at index %d: %s",
                    duration_query.index,
                    strerror(errno));
            return -ENODEV;
        }
    } else {
        // Continuous/Step-wise: simply convert the given min and max.
        min = FractToNs(duration_query.stepwise.min);
        max = FractToNs(duration_query.stepwise.max);
    }
    (*duration_range)[0] = min;
    (*duration_range)[1] = max;
    return 0;
}

// Private functions
int V4L2Wrapper::v4l2QcarcamInit(void)
{
    int ret = 0;
    qcarcam_init_t qcarcam_init = {};
    qcarcam_init.version = QCARCAM_VERSION;
    qcarcam_init.debug_tag = (char *)"ais_v4l2_hal_client";

    std::unique_lock<std::mutex> lock(qcarcamInitLock);
    if (!mQcarcamInit) {
        // Initialize qcarcam
        ret = qcarcam_initialize(&qcarcam_init);
        if (ret)
            mQcarcamInit = false;
        else
            mQcarcamInit = true;
        ais_log_init(NULL, (char *)qcarcam_init.debug_tag);
    }
    // return success if its already initialized
    lock.unlock();

    return ret;
}

void V4L2Wrapper::v4l2QcarcamDeInit(void)
{
    std::unique_lock<std::mutex> lock(qcarcamInitLock);
    if (mQcarcamInit) {
        qcarcam_uninitialize();
        mQcarcamInit = false;
    }
    lock.unlock();
}

void V4L2Wrapper::mClientCntStatus(void)
{
    std::unique_lock<std::mutex> lock(mClientCntLock);
    HAL_LOGE("mClient count before removing 1 client is %d",mClientCnt);
    mClientCnt = (mClientCnt > 0)?(mClientCnt-1):0;
    // Uninitialize qcarcam only if its last client
    if (0 == mClientCnt) {
        v4l2QcarcamDeInit();
        HAL_LOGE("qcarcam_uninitialize() called");
    }
    HAL_LOGE("mClient count after removing 1 client is %d",mClientCnt);
    lock.unlock();
}

int V4L2Wrapper::openCamera()
{
    HAL_LOG_ENTER();
    HAL_LOGE("Open camera id =%d", cam_id);
    int ret = 0;
    std::unique_lock<std::mutex> lock(mClientCntLock);
    HAL_LOGE("mClient count before qcarcam_open is %d",mClientCnt);
    if (0 == mClientCnt) {
        ret = v4l2QcarcamInit();
        if (ret == QCARCAM_RET_OK) {
            HAL_LOGE("qcarcam initialized.");
        }
        else {
            HAL_LOGE ("qcarcam_initialize failed ret = %d",ret);
            lock.unlock();
            return -1;
        }
    }

    qcarcam_context = qcarcam_open((qcarcam_input_desc_t)cam_id);
    if (qcarcam_context == 0) {
        HAL_LOGE("qcarcam_open() failed");
        qcarcam_context = NULL;
        if (0 == mClientCnt)
            v4l2QcarcamDeInit();
        lock.unlock();
        return -1;
    }
    else {
        mClientCnt++;
        HAL_LOGE("mClient count after qcarcam_open is %d",mClientCnt);
        HAL_LOGE("qcarcam_open() successful for camera id %d", cam_id);
        std::unique_lock<std::mutex> lk(gClientMapMutex);
        gClientMap[qcarcam_context] = this;
    }

    lock.unlock();
    HAL_LOG_EXIT();
    return 0;

}
int V4L2Wrapper::allocInputBuffers()
{
    HAL_LOG_ENTER();
    if (0 == mQcarcamOutBufs.n_buffers) {
        //allocate qcarcam internal buffers
        if (!allocQcarcamInternalBuffers(MAX_BUFFER_NUM)) {
            return -1;
        }

        HAL_LOGE(" calling qcarcam_s_buffers");
        mQcarcamOutBufs.flags = 0;
        qcarcam_ret_t ret = QCARCAM_RET_OK;
        ret = qcarcam_s_buffers(qcarcam_context, &mQcarcamOutBufs);
        if (ret != QCARCAM_RET_OK) {
            HAL_LOGE( "qcarcam_s_buffers failed!");
            qcarcam_close(qcarcam_context);
            std::unique_lock<std::mutex> lk(gClientMapMutex);
            gClientMap.erase(qcarcam_context);
            qcarcam_context = NULL;
            mClientCntStatus();
            return -1;
        }
    }
    HAL_LOG_EXIT();
    return 0;
}
bool V4L2Wrapper::allocQcarcamInternalBuffers(uint32_t count)
{
    HAL_LOG_ENTER();
    mQcarcamOutBufs.n_buffers = count;
    mQcarcamOutBufs.color_fmt = mAisFrameFormat;

    mQcarcamOutBufs.buffers = (qcarcam_buffer_t *)calloc(mQcarcamOutBufs.n_buffers, sizeof(*mQcarcamOutBufs.buffers));
    if (mQcarcamOutBufs.buffers == 0) {
        HAL_LOGE( "alloc qcarcam_buffer failed");
        return false;
    }
    mpGfxBufs = (sp<GraphicBuffer>*)calloc(mQcarcamOutBufs.n_buffers, sizeof(sp<GraphicBuffer>));
    if (0 == mpGfxBufs) {
        HAL_LOGE( "alloc mpGfxBufs failed");
        free(mQcarcamOutBufs.buffers);
        return false;
    }
    mpQcarcamMmapBufs = (qcarcam_mapped_buffer_t*)calloc(mQcarcamOutBufs.n_buffers,
            sizeof(qcarcam_mapped_buffer_t));
    if (0 == mpQcarcamMmapBufs) {
        HAL_LOGE( "alloc mpQcarcamMmapBufs failed");
        free(mQcarcamOutBufs.buffers);
        free(mpGfxBufs);
        return false;
    }

    const int graphicsUsage  = android::GraphicBuffer::USAGE_HW_TEXTURE |
        android::GraphicBuffer::USAGE_SW_WRITE_OFTEN |
        android::GraphicBuffer::USAGE_SW_READ_OFTEN;
    const int nativeBufferFormat = mHalFrameFormat;

    for (int i = 0; i < (int)mQcarcamOutBufs.n_buffers; ++i) {
        mQcarcamOutBufs.buffers[i].n_planes = 1;
        mQcarcamOutBufs.buffers[i].planes[0].width = ALIGN(mAisFrameWidth, WIDTH_ALIGN);
        mQcarcamOutBufs.buffers[i].planes[0].height = mAisFrameHeight;
        mQcarcamOutBufs.buffers[i].planes[0].stride =
            CalcStride0(mHalFrameFormat,ALIGN(mAisFrameWidth, WIDTH_ALIGN));
        //getQcarcamStride(ALIGN(mAisFrameWidth, WIDTH_ALIGN), mHalFrameFormat);
        mQcarcamOutBufs.buffers[i].planes[0].size = mQcarcamOutBufs.buffers[i].planes[0].height *
            mQcarcamOutBufs.buffers[i].planes[0].stride;

        /*        std::cout <<"w="<< mQcarcamOutBufs.buffers[i].planes[0].width
                  <<" h="<< mQcarcamOutBufs.buffers[i].planes[0].height
                  <<"stride="<< mQcarcamOutBufs.buffers[i].planes[0].stride
                  <<" size="<< mQcarcamOutBufs.buffers[i].planes[0].size
                  <<" hal_format="<< mHalFrameFormat<<std::endl;*/

        mpGfxBufs[i] = NULL;
        mpGfxBufs[i] = new GraphicBuffer(ALIGN(mQcarcamOutBufs.buffers[i].planes[0].width, WIDTH_ALIGN),
                mQcarcamOutBufs.buffers[i].planes[0].height,
                nativeBufferFormat,
                graphicsUsage);
        if (mpGfxBufs[i] == NULL) {
            HAL_LOGE("create gfx buffer failed");
            free(mpGfxBufs);
            return 0;
        }

        int fd = get_handle_fd(mpGfxBufs[i]->getNativeBuffer()->handle);
        HAL_LOGE("fd value %d",fd);
        mQcarcamOutBufs.buffers[i].planes[0].p_buf = (void*)(uintptr_t)(fd);

        mpQcarcamMmapBufs[i].size = mQcarcamOutBufs.buffers[i].planes[0].size;
        mpQcarcamMmapBufs[i].ptr = mmap(NULL, mpQcarcamMmapBufs[i].size,
                PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (!mpQcarcamMmapBufs[i].ptr)
            HAL_LOGE( "mmap failed for %d",i);

        if ((mHalFrameFormat != mOutBufFormat)) {
            HAL_LOG_ENTER();
            C2D_STATUS c2d_status;
            C2D_YUV_SURFACE_DEF srcSurfaceDef;
            void *c2dsrcGpuAddr = NULL;

            c2d_status = c2dMapAddr(fd, mpQcarcamMmapBufs[i].ptr,
                    mpQcarcamMmapBufs[i].size, 0,
                    KGSL_USER_MEM_TYPE_ION, &c2dsrcGpuAddr);
            if (c2d_status != C2D_STATUS_OK) {
                HAL_LOGE("c2dMapAddr failed for id %d status %d ",i, c2d_status);
                free(mQcarcamOutBufs.buffers);
                free(mpGfxBufs);
                return false;
            }

            memset(&srcSurfaceDef, 0, sizeof(srcSurfaceDef));
            srcSurfaceDef.width = mQcarcamOutBufs.buffers[i].planes[0].width;
            srcSurfaceDef.height = mQcarcamOutBufs.buffers[i].planes[0].height;
            srcSurfaceDef.stride0 = mQcarcamOutBufs.buffers[i].planes[0].stride;
            srcSurfaceDef.stride1 = mQcarcamOutBufs.buffers[i].planes[0].stride;
            srcSurfaceDef.format = GetC2dFormat(mHalFrameFormat, true); //getHalToC2dFormat(mHalFrameFormat);
            srcSurfaceDef.plane0 = mpQcarcamMmapBufs[i].ptr; //Creating surface with one buffer later we update it in update_surface.
            srcSurfaceDef.plane1 = (void *) ((unsigned char *)srcSurfaceDef.plane0 + (srcSurfaceDef.width * srcSurfaceDef.height));
            srcSurfaceDef.phys0 = c2dsrcGpuAddr;
            srcSurfaceDef.phys1 = (void *) ((unsigned char *)c2dsrcGpuAddr + (srcSurfaceDef.width * srcSurfaceDef.height));

            HAL_LOGV("format:0x%x, w:%d, h:%d, plan0:%p, plane1:%p, plane2:%p, phys0:%p, phys1:%p, phys2:%p, s0:%d, s1:%d, s2:%d",
                    srcSurfaceDef.format, srcSurfaceDef.width, srcSurfaceDef.height,
                    srcSurfaceDef.plane0, srcSurfaceDef.plane1, srcSurfaceDef.plane2,
                    srcSurfaceDef.phys0, srcSurfaceDef.phys1, srcSurfaceDef.phys2,
                    srcSurfaceDef.stride0, srcSurfaceDef.stride1, srcSurfaceDef.stride2);
            c2d_status = c2dCreateSurface(&mpQcarcamMmapBufs[i].srcC2dSurfaceId,
                    C2D_SOURCE, (C2D_SURFACE_TYPE) (((srcSurfaceDef.format != C2D_COLOR_FORMAT_888_RGB) ? C2D_SURFACE_YUV_HOST: C2D_SURFACE_RGB_HOST) | C2D_SURFACE_WITH_PHYS), &srcSurfaceDef);
            if (c2d_status != C2D_STATUS_OK) {
                c2dUnMapAddr(c2dsrcGpuAddr);
                HAL_LOGE("Source c2d_create API falied ret= %d",c2d_status);
                free(mQcarcamOutBufs.buffers);
                free(mpGfxBufs);
                return false;
            }
            mSrcGpuMapList.insert(std::pair<std::uint64_t, void *>((uint64_t)mpQcarcamMmapBufs[i].ptr, c2dsrcGpuAddr));
            HAL_LOG_EXIT();
        }
    }
    return true;
}

int V4L2Wrapper::Init() {
    HAL_LOG_ENTER();

    int res = 0;

    if (mInputWidth == 0) {
        res = GetInputInformation();
        if (res < 0) {
            HAL_LOGE("Get input information failed, so can not stream on");
            return res;
        }
    }

    mControlMode  = ANDROID_CONTROL_MODE_AUTO;
    mFacePriority = false;
    mAeMode       = ANDROID_CONTROL_AE_MODE_ON;
    mAfMode       = ANDROID_CONTROL_AF_MODE_AUTO;
    mAwbMode      = ANDROID_CONTROL_AWB_MODE_AUTO;
    mAeState      = ANDROID_CONTROL_AE_STATE_INACTIVE;
    mAfState      = ANDROID_CONTROL_AF_STATE_INACTIVE;
    mAwbState     = ANDROID_CONTROL_AWB_STATE_INACTIVE;
    mAeCounter    = 0;
    mAeTargetExposureTime = kNormalExposureTime;
    mAeCurrentExposureTime = kNormalExposureTime;
    mAeCurrentSensitivity  = kNormalSensitivity;
    mAeMinExposureTime     = kminExposureTime;
    mExposureSeed = 1;
    mTimesBetweentAndroidQnx = 0;

    mFrameNum = 0;
    return 0;
}

int V4L2Wrapper::RequestBuffers(uint32_t num_requested) {
    v4l2_requestbuffers req_buffers;

    memset(&req_buffers, 0, sizeof(req_buffers));
    req_buffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req_buffers.memory = V4L2_MEMORY_MMAP;//V4L2_MEMORY_USERPTR;
    req_buffers.count = num_requested;

    int res = IoctlLocked(VIDIOC_REQBUFS, &req_buffers);
    // Calling REQBUFS releases all queued buffers back to the user.
    if (res < 0) {
        HAL_LOGE("REQBUFS failed: %s", strerror(errno));
        return -ENODEV;
    }

    // V4L2 will set req_buffers.count to a number of buffers it can handle.
    if (num_requested > 0 && req_buffers.count < 1) {
    HAL_LOGE("REQBUFS claims it can't handle any buffers.");
        return -ENODEV;
    }

    CreateInputSurfaces(num_requested);

    return 0;
}

int V4L2Wrapper::ReleaseBuffers() {
    v4l2_requestbuffers req_buffers;

    memset(&req_buffers, 0, sizeof(req_buffers));
    req_buffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req_buffers.memory = V4L2_MEMORY_MMAP;//V4L2_MEMORY_USERPTR;
    req_buffers.count = 0;

    int res = IoctlLocked(VIDIOC_REQBUFS, &req_buffers);
    // Calling REQBUFS releases all queued pBuffers back to the user.
    if (res < 0) {
        HAL_LOGE("REQBUFS failed: %s", strerror(errno));
        return -ENODEV;
    }

    ReleaseInputSurfaces();

    return 0;
}

int32_t V4L2Wrapper::CreateInputSurfaces(int32_t num) {
    if (num <= 0 || num > MAX_INPUT_BUFFER_NUM) {
        HAL_LOGE("skip num:%d, max:%d", num, MAX_INPUT_BUFFER_NUM);
        return -1;
    }

    for (int32_t i = 0; i < num; i++) {
        if (CreateInputSurface(i) < 0) {
            HAL_LOGE("ERROR: create input surface failed: %d", i);
            ReleaseInputSurfaces();
            return -1;
        }
    }

    return 0;
}

int32_t V4L2Wrapper::CreateInputSurface(int32_t index) {
    HAL_LOG_ENTER();

    if (index < 0 || index >= MAX_INPUT_BUFFER_NUM) {
        HAL_LOGE("ERROR: bad parameter:%d", index);
        return -1;
    }

    v4l2_buffer device_buffer;
    memset(&device_buffer, 0, sizeof(device_buffer));
    device_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    device_buffer.memory = V4L2_MEMORY_MMAP;
    device_buffer.index = index;

    // Use QUERYBUF to ensure our buffer/device is in good shape,
    // and fill out remaining fields.
    if (IoctlLocked(VIDIOC_QUERYBUF, &device_buffer) < 0) {
        HAL_LOGE("QUERYBUF fails: %s", strerror(errno));
        return -ENODEV;
    }

    if (input_buffer_infos[index].isUsed) {
        HAL_LOGE("ERROR: index:%d, isUsed:%d", index,
                input_buffer_infos[index].isUsed);
        return -1;
    }

    struct v4l2_exportbuffer expbuf = { 0 };
    expbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    expbuf.index = index;
    expbuf.plane = 1;
    expbuf.flags = O_CLOEXEC | O_RDWR;

    if (IoctlLocked(VIDIOC_EXPBUF, &expbuf) < 0)
    {
        HAL_LOGE("EXPBUF failed err= %s index %x", strerror(-errno), index);
        return 1;
    }

    input_buffer_infos[index].width = mInputWidth;
    input_buffer_infos[index].height = mInputHeight;
    input_buffer_infos[index].cropWidth = mInputCropWidth;
    input_buffer_infos[index].cropHeight = mInputCropHeight;
    input_buffer_infos[index].cropLeft = mInputCropLeft;
    input_buffer_infos[index].cropTop = mInputCropTop;
    input_buffer_infos[index].halFormat = mInputHalFormat;
    input_buffer_infos[index].fd = expbuf.fd; //dup(device_fd_.get());
    input_buffer_infos[index].length = device_buffer.length;
    input_buffer_infos[index].offset = device_buffer.m.offset;
    input_buffer_infos[index].id = index;

    HAL_LOGV("DEBUG: fd:%d Buffer size:%d offset:%d, index:%d, w:%d, h:%d, device_fd_.get():%d",
            input_buffer_infos[index].fd,
            device_buffer.length, device_buffer.m.offset,
            device_buffer.index,
            input_buffer_infos[index].width,
            input_buffer_infos[index].height, device_fd_.get());

    int32_t ret = CreateC2dSurface(&input_buffer_infos[index], true);
    if (ret < 0) {
        HAL_LOGE("ERROR create c2d surface failed");
        return -1;
    }

    if (IoctlLocked(VIDIOC_QBUF, &device_buffer) < 0) {
        HAL_LOGE("VIDIOC_QBUF fails: %s", strerror(errno));
        return -ENODEV;
    }

    return ret;
}

void V4L2Wrapper::ReleaseInputSurfaces() {
    HAL_LOG_ENTER();
    for (int i = 0; i < MAX_INPUT_BUFFER_NUM; i++) {
        if (input_buffer_infos[i].isUsed) {
            ReleaseC2dSurface(&input_buffer_infos[i]);
        }
    }
}

int V4L2Wrapper::EnqueueRequest(
        std::shared_ptr<default_camera_hal::CaptureRequest> request) {
    HAL_LOG_ENTER();
    int32_t ret = 0;

    if (streamstatus != STREAM_STATUS_STREAM_ON) {
        HAL_LOGE("ERROR: not stream on, can not enqueue buffer");
        return -1;
    }

    ret = CreateOutputSurfaces(request);
    if (ret < 0) {
        HAL_LOGE("ERROR: create output stream surfaces failed");
        return -1;
    }

    PushRequest(request);

    HAL_LOGV("new request:%d", request->frame_number);
    return 0;
}

#if !defined(ANDROID_Q_AOSP)
/**
  * get_native_hnd_buffer_fd
  *
  * @brief retrieves file descriptor for buffer_handle_t hndl
  *
  * @param hndl : buffer_handle_t
  *
  * @return int
  */
int V4L2Wrapper::get_native_hnd_buffer_fd(buffer_handle_t hndl)
{
    int fd = -1;
    auto err = qtigralloc::get(const_cast<native_handle_t*>(hndl), QTI_FD, &fd);
    if (err != Error_v4::NONE)
    {
        HAL_LOGE("ERROR: qtigralloc::get() failed to retrieve fd!");
    }

    return fd;
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
int V4L2Wrapper::get_handle_fd(buffer_handle_t hndl)
{
    sp<IMapper_v4> mapper = IMapper_v4::getService();
    int fd = -1;
    mapper->get(const_cast<native_handle_t*>(hndl), qtigralloc::MetadataType_FD,
            [&](const auto &_error, const auto &_bytestream) {
            if (_error !=  Error_v4::NONE)
            HAL_LOGE("Failed to get FD");
            else {
            android::gralloc4::decodeInt32(qtigralloc::MetadataType_FD,
                    _bytestream, &fd);
            }
            });
    return fd;
}

/**
  * get_native_hnd_buffer_offset
  *
  * @brief retrieves offset of buffer from buffer_handle_t hndl
  *
  * @param hndl : buffer_handle_t
  *
  * @return int
  */
int V4L2Wrapper::get_native_hnd_buffer_offset(buffer_handle_t hndl)
{
    std::vector<PlaneLayout> planeLayouts;
    mapper->get(const_cast<native_handle_t*>(hndl), android::gralloc4::MetadataType_PlaneLayouts,
            [&](const auto &_error, const auto &_bytestream) {
            if (_error !=  Error_v4::NONE)
            HAL_LOGE("ERROR: Failed to get offset %d", _error);
            else {
            android::gralloc4::decodePlaneLayouts(_bytestream, &planeLayouts);
            }
            });
    return  planeLayouts[0].offsetInBytes;
}

/**
  * get_native_hnd_buffer_size
  *
  * @brief retrieves size of buffer from buffer_handle_t hndl
  *
  * @param hndl : buffer_handle_t
  *
  * @return uint64_t
  */
uint64_t V4L2Wrapper::get_native_hnd_buffer_size(buffer_handle_t hndl)
{
    uint64_t size;
    mapper->get(const_cast<native_handle_t*>(hndl), android::gralloc4::MetadataType_AllocationSize,
            [&](const auto &_error, const auto &_bytestream) {
            if (_error !=  Error_v4::NONE)
            HAL_LOGE("ERROR: Failed to get size %d", _error);
            else {
            android::gralloc4::decodeAllocationSize(_bytestream, &size);
            }
            });
    return size;
}

/**
  * get_native_hnd_buffer_id
  *
  * @brief retrieves bufId of buffer from buffer_handle_t hndl
  *
  * @param hndl : buffer_handle_t
  *
  * @return uint64_t
  */
uint64_t V4L2Wrapper::get_native_hnd_buffer_id(buffer_handle_t hndl)
{

    uint64_t buf_id;
    mapper->get(const_cast<native_handle_t*>(hndl), android::gralloc4::MetadataType_BufferId,
            [&](const auto &_error, const auto &_bytestream) {
            if (_error !=  Error_v4::NONE)
            HAL_LOGE("ERROR: Failed to get bufid %d", _error);
            else
            android::gralloc4::decodeBufferId(_bytestream, &buf_id);
            });

    return buf_id;
}

#endif

int32_t V4L2Wrapper::CreateOutputSurfaces(
        std::shared_ptr<default_camera_hal::CaptureRequest> request) {
    int32_t ret = 0;

    for(auto it = request->output_buffers.begin();
            it != request->output_buffers.end(); it++){
#if defined(ANDROID_Q_AOSP)
      const private_handle_t *hnd_output = PRIV_HANDLE_CONST(*(it->buffer));
      buffer_info_t * buff_info = GetOutputBufferInfo(hnd_output->id);
      if (buff_info != nullptr) {
        if (hnd_output->id != buff_info->id || hnd_output->size != buff_info->length ||
            hnd_output->offset != buff_info->offset ||
            it->stream->format != buff_info->halFormat) {
          HAL_LOGE("ERROR: request buffer parameter is not match with created c2d surface");
          HAL_LOGE("ERROR: surface fd:%d, offset:%d, size:%d, format:%s, id:%" PRIu64 ", "
                   "output fd:%d, offset:%d, size:%d, format:%s, id:%" PRIu64 "",
                  buff_info->fd, buff_info->offset,
                  buff_info->length, printHalFormat(buff_info->halFormat), buff_info->id,
                  hnd_output->fd, hnd_output->offset,
                  hnd_output->size, printHalFormat(it->stream->format), hnd_output->id);
          DeleteOutputBufferInfo(buff_info);
          ReleaseC2dSurface(buff_info);
        } else {
          HAL_LOGV("has been created c2d surface");
          continue;
        }
      }
#else
        int buf_id = get_native_hnd_buffer_id(*(it->buffer));
        int buf_fd = get_native_hnd_buffer_fd(*(it->buffer));
        int buf_size = get_native_hnd_buffer_size(*(it->buffer));
        buffer_info_t * buff_info = GetOutputBufferInfo(buf_id);
        if (buff_info != nullptr) {
            if (buf_id != buff_info->id || buf_size != buff_info->length ||
                    it->stream->format != buff_info->halFormat) {
                HAL_LOGE("ERROR: request buffer parameter is not match with created c2d surface");
                HAL_LOGE("ERROR: surface fd:%d, offset:%d, size:%d, format:%s, id:%" PRIu64 ", "
                        "output fd:%d, size:%d, format:%s, id:%" PRIu64 "",
                        buff_info->fd, buff_info->offset,
                        buff_info->length, printHalFormat(buff_info->halFormat), buff_info->id,
                        buf_fd, buf_size, printHalFormat(it->stream->format), buf_id);
                DeleteOutputBufferInfo(buff_info);
                ReleaseC2dSurface(buff_info);
            } else {
                HAL_LOGV("has been created c2d surface");
                continue;
            }
        }

#endif
        int32_t index = 0;
        for (index = 0; index < MAX_OUTPUT_BUFFER_NUM; index++) {
            if (!output_buffer_infos[index].isUsed) {
                break;
            }
        }

        if (index >= MAX_OUTPUT_BUFFER_NUM) {
            HAL_LOGE("WARNING: can not find idle buffer, so release older frame surface");
            for (index = 0; index < MAX_OUTPUT_BUFFER_NUM; index++) {
                uint32_t distance = request->frame_number - output_buffer_infos[index].frame_number;
                if (distance > (MAX_BUFFER_NUM * 2)) {
                    break;
                }
            }
            if (index >= MAX_OUTPUT_BUFFER_NUM) {
                HAL_LOGE("ERROR: can not find suitable free index, then use index 0");
                index = 0;
            }
            DeleteOutputBufferInfo(&output_buffer_infos[index]);
            ReleaseC2dSurface(&output_buffer_infos[index]);
        }

        ret = CreateOutputSurface(&(*it), &output_buffer_infos[index]);
        if (ret == 0) {
            output_buffer_infos[index].frame_number = request->frame_number;
            AddOutputBufferInfo(&output_buffer_infos[index]);

            HAL_LOGV("new create surface gpuAddr:%p buffer id:%" PRIu64 ", index:%d",
                    output_buffer_infos[index].gpuAddr, output_buffer_infos[index].id, index);
        } else {
            HAL_LOGE("ERROR: create output surface failed");
            break;
        }
    }

    return ret;
}

buffer_info_t * V4L2Wrapper::GetOutputBufferInfo(uint64_t id) {
    std::lock_guard<std::mutex> guard(output_buffer_map_lock_);

    auto iter = outputBufferMap.find(id);
    if (iter == outputBufferMap.end()) {
        return nullptr;
    }

    return iter->second;
}

int32_t V4L2Wrapper::AddOutputBufferInfo(buffer_info_t * buff_info) {
    if (buff_info == nullptr) {
        HAL_LOGE("ERROR: bad paramer");
        return -1;
    }

    std::lock_guard<std::mutex> guard(output_buffer_map_lock_);
    auto iter = outputBufferMap.find(buff_info->id);
    if (iter != outputBufferMap.end()) {
        HAL_LOGE("ERROR: buffer id:%" PRIu64 " already exist", buff_info->id);
        return -1;
    }

    outputBufferMap.insert(
            std::pair<uint64_t, buffer_info_t *>(buff_info->id, buff_info));
    return 0;
}

int32_t V4L2Wrapper::DeleteOutputBufferInfo(buffer_info_t * buff_info) {
    if (buff_info == nullptr) {
        HAL_LOGE("ERROR: bad paramer");
        return -1;
    }

    std::lock_guard<std::mutex> guard(output_buffer_map_lock_);
    auto iter = outputBufferMap.find(buff_info->id);
    if (iter == outputBufferMap.end()) {
        HAL_LOGE("ERROR: buffer id:%" PRIu64 " not exist", buff_info->id);
        return -1;
    }

    outputBufferMap.erase(iter);
    return 0;
}

int32_t V4L2Wrapper::CreateOutputSurface(camera3_stream_buffer_t* output_stream_buffer,
        buffer_info_t * buff_info) {
    if (output_stream_buffer == NULL || buff_info == NULL) {
        HAL_LOGE("ERROR: bad parameter");
        return -1;
    }

#if defined(ANDROID_Q_AOSP)
    if (output_stream_buffer->stream->format == HAL_PIXEL_FORMAT_BLOB) {
        // snapshot: C2D output format adjust as UYVY 422, used do JPEG encoder
    const private_handle_t *hnd =
          PRIV_HANDLE_CONST(snapshot_output_buffer->getNativeBuffer()->handle);
    const private_handle_t *hnd_output = PRIV_HANDLE_CONST(*(output_stream_buffer->buffer));
    buff_info->fd = dup(hnd->fd);
    buff_info->length = hnd->size;
    buff_info->offset = hnd->offset;
    buff_info->halFormat = HAL_PIXEL_FORMAT_RGB_888; // HAL_PIXEL_FORMAT_CbYCrY_422_I;  // UYVY 422
    buff_info->id = hnd_output->id;
  } else {
    const private_handle_t *hnd = PRIV_HANDLE_CONST(*(output_stream_buffer->buffer));
    buff_info->fd = dup(hnd->fd);
    buff_info->length = hnd->size;
    buff_info->offset = hnd->offset;
    buff_info->halFormat = output_stream_buffer->stream->format;
    buff_info->id = hnd->id;
  }
#else
  if (output_stream_buffer->stream->format == HAL_PIXEL_FORMAT_BLOB) {
    // snapshot: C2D output format adjust as UYVY 422, used do JPEG encoder

        int buf_id = get_native_hnd_buffer_id(snapshot_output_buffer->getNativeBuffer()->handle);
        int buf_fd = get_native_hnd_buffer_fd(snapshot_output_buffer->getNativeBuffer()->handle);
        int buf_size = get_native_hnd_buffer_size(snapshot_output_buffer->getNativeBuffer()->handle);
        int buf_offset = get_native_hnd_buffer_offset(snapshot_output_buffer->getNativeBuffer()->handle);

        int output_buf_id = get_native_hnd_buffer_id(*(output_stream_buffer->buffer));
        buff_info->fd = dup(buf_fd);
        buff_info->length = buf_size;
        buff_info->offset = buf_offset;
        buff_info->halFormat = HAL_PIXEL_FORMAT_RGB_888; // HAL_PIXEL_FORMAT_CbYCrY_422_I;  // UYVY 422
        buff_info->id = output_buf_id;
    } else {
        int buf_id = get_native_hnd_buffer_id(*(output_stream_buffer->buffer));
        int buf_fd = get_native_hnd_buffer_fd(*(output_stream_buffer->buffer));
        int buf_size = get_native_hnd_buffer_size(*(output_stream_buffer->buffer));
        int buf_offset = get_native_hnd_buffer_offset(*(output_stream_buffer->buffer));

        buff_info->fd = dup(buf_fd);
        buff_info->length = buf_size;
        buff_info->offset = buf_offset;
        buff_info->halFormat = output_stream_buffer->stream->format;
        buff_info->id = buf_id;
    }
#endif

    buff_info->width = output_stream_buffer->stream->width;
    buff_info->height = output_stream_buffer->stream->height;

    HAL_LOGV("DEBUG: Buffer size:%d offset:%d, fd:%d, w:%d, h:%d, format:%s, id:%" PRIu64 "",
            buff_info->length, buff_info->offset,
            buff_info->fd, buff_info->width, buff_info->height,
            printHalFormat(buff_info->halFormat), buff_info->id);

    int32_t ret = CreateC2dSurface(buff_info, false);
    return ret;
}

void V4L2Wrapper::ReleaseOutputSurfaces() {
    HAL_LOG_ENTER();
    for (int i = 0; i < MAX_OUTPUT_BUFFER_NUM; i++) {
        if (output_buffer_infos[i].isUsed) {
            DeleteOutputBufferInfo(&output_buffer_infos[i]);
            ReleaseC2dSurface(&output_buffer_infos[i]);
        }
    }

    std::lock_guard<std::mutex> guard(output_buffer_map_lock_);
    outputBufferMap.clear();
    HAL_LOG_EXIT();
}

/*
  * @Breif: Function to convert NV21 to RGBA format
  * @Input:  Configured width of the stream
  *          Configured height of the stream
  *          Pointer to Y components
  *          Pointer to UV components
  *          Y stride
  *          UV stride
  *          RGBA outbut buffer pointer
  *          RGBA stride
  * @return: null
  */
void V4L2Wrapper::nv21_rgb24_std(
        uint32_t width, uint32_t height,
        const uint8_t *Y, const uint8_t *UV, uint32_t Y_stride, uint32_t UV_stride,
        uint8_t *RGB, uint32_t RGB_stride)
{
    uint32_t x, y;
    uint8_t a = 255;

    for(y=0; y<(height-1); y+=2)
    {
        const uint8_t *y_ptr1=Y+y*Y_stride,
              *y_ptr2=Y+(y+1)*Y_stride,
              *uv_ptr=UV+(y/2)*UV_stride;
        uint8_t *rgb_ptr1=RGB+y*RGB_stride,
                *rgb_ptr2=RGB+(y+1)*RGB_stride;

        for(x=0; x<(width-1); x+=2)
        {
            int8_t u_tmp, v_tmp;
            u_tmp = uv_ptr[1]-128;
            v_tmp = uv_ptr[0]-128;

            //compute Cb Cr color offsets, common to four pixels
            int16_t b_cb_offset, r_cr_offset, g_cbcr_offset;
            b_cb_offset = (113*u_tmp)>>6;
            r_cr_offset = (90*v_tmp)>>6;
            g_cbcr_offset = (44*u_tmp + 91*v_tmp)>>7;

            int16_t y_tmp;
            y_tmp = y_ptr1[0];
            rgb_ptr1[0] = CLAMP(y_tmp + r_cr_offset);
            rgb_ptr1[1] = CLAMP(y_tmp - g_cbcr_offset);
            rgb_ptr1[2] = CLAMP(y_tmp + b_cb_offset);
            rgb_ptr1[3] = a;

            y_tmp = y_ptr1[1];
            rgb_ptr1[4] = CLAMP(y_tmp + r_cr_offset);
            rgb_ptr1[5] = CLAMP(y_tmp - g_cbcr_offset);
            rgb_ptr1[6] = CLAMP(y_tmp + b_cb_offset);
            rgb_ptr1[7] = a;

            y_tmp = y_ptr1[0];
            rgb_ptr2[0] = CLAMP(y_tmp + r_cr_offset);
            rgb_ptr2[1] = CLAMP(y_tmp - g_cbcr_offset);
            rgb_ptr2[2] = CLAMP(y_tmp + b_cb_offset);
            rgb_ptr2[3] = a;

            y_tmp = y_ptr1[1];
            rgb_ptr2[4] = CLAMP(y_tmp + r_cr_offset);
            rgb_ptr2[5] = CLAMP(y_tmp - g_cbcr_offset);
            rgb_ptr2[6] = CLAMP(y_tmp + b_cb_offset);


            rgb_ptr2[7] = a;

            rgb_ptr1 += 8;
            rgb_ptr2 += 8;
            y_ptr1 += 2;
            y_ptr2 += 2;
            uv_ptr += 2;
        }
    }
}

uint32_t V4L2Wrapper::getQcarcamToHalFormat(qcarcam_color_fmt_t qcarcamFormat)
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
        default:
            return HAL_PIXEL_FORMAT_CbYCrY_422_I;
            break;
    }
}

uint32_t V4L2Wrapper::GetC2dFormat(uint32_t halFormat, bool isSource) {
    switch (halFormat) {
        case HAL_PIXEL_FORMAT_CbYCrY_422_I:
            return C2D_COLOR_FORMAT_422_UYVY;
        case HAL_PIXEL_FORMAT_YCBCR_422_I:
            return C2D_COLOR_FORMAT_422_YUYV;
        case HAL_PIXEL_FORMAT_YCbCr_420_SP:
            return C2D_COLOR_FORMAT_420_NV12;
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            return C2D_COLOR_FORMAT_420_NV21;
        case HAL_PIXEL_FORMAT_NV12_ENCODEABLE:
            return C2D_COLOR_FORMAT_420_NV12;
        case HAL_PIXEL_FORMAT_YCbCr_420_SP_TILED:
            return C2D_COLOR_FORMAT_420_NV12 | C2D_FORMAT_MACROTILED;
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
            return C2D_COLOR_FORMAT_420_NV21;
            //return C2D_COLOR_FORMAT_420_Y_U_V;
        case HAL_PIXEL_FORMAT_RGB_565:
            return C2D_COLOR_FORMAT_565_RGB;
        case HAL_PIXEL_FORMAT_RGB_888:
            return C2D_COLOR_FORMAT_888_RGB | C2D_FORMAT_SWAP_RB;
        case HAL_PIXEL_FORMAT_RGBX_8888:
            return C2D_COLOR_FORMAT_8888_ARGB | C2D_FORMAT_SWAP_RB | C2D_FORMAT_DISABLE_ALPHA;
        case HAL_PIXEL_FORMAT_RGBA_8888:
            if (isSource) {
                return C2D_COLOR_FORMAT_8888_ARGB | C2D_FORMAT_SWAP_RB | C2D_FORMAT_PREMULTIPLIED;
            } else {
                return C2D_COLOR_FORMAT_8888_ARGB | C2D_FORMAT_SWAP_RB;
            }
        case HAL_PIXEL_FORMAT_BGRA_8888:
            return C2D_COLOR_FORMAT_8888_ARGB;
        case HAL_PIXEL_FORMAT_RGBA_5551:
            return C2D_COLOR_FORMAT_5551_RGBA;
        case HAL_PIXEL_FORMAT_RGBA_4444:
            return C2D_COLOR_FORMAT_4444_RGBA;
        default:
            HAL_LOGE("Unsupported HAL format: 0x%x, set as default C2D format", halFormat);
            return C2D_COLOR_FORMAT_422_UYVY;
    }

    return C2D_COLOR_FORMAT_422_UYVY;
}

bool V4L2Wrapper::IsYuvformat(uint32_t halFormat) {
    switch (halFormat) {
        case HAL_PIXEL_FORMAT_CbYCrY_422_I:
        case HAL_PIXEL_FORMAT_YCBCR_422_I:
        case HAL_PIXEL_FORMAT_YCbCr_420_SP:
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
        case HAL_PIXEL_FORMAT_NV12_ENCODEABLE:
        case HAL_PIXEL_FORMAT_YCbCr_420_SP_TILED:
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
            return true;
        case HAL_PIXEL_FORMAT_RGB_565:
        case HAL_PIXEL_FORMAT_RGB_888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
        case HAL_PIXEL_FORMAT_RGBA_5551:
        case HAL_PIXEL_FORMAT_RGBA_4444:
        default:
            return false;
    }

    return false;
}

uint32_t V4L2Wrapper::CalcStride0(uint32_t halFormat, size_t width) {
    switch (halFormat) {
        case HAL_PIXEL_FORMAT_CbYCrY_422_I:
        case HAL_PIXEL_FORMAT_YCBCR_422_I:
            return ALIGN(width*2, ALIGN64);
        case HAL_PIXEL_FORMAT_YCbCr_420_SP:
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            return ALIGN(width, ALIGN16);
        case HAL_PIXEL_FORMAT_NV12_ENCODEABLE:
            return ALIGN(width, ALIGN128);
        case HAL_PIXEL_FORMAT_YCbCr_420_SP_TILED:
            return ALIGN(width, ALIGN128);
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
            return ALIGN(width, ALIGN16);
        case HAL_PIXEL_FORMAT_RGB_565:
            return ALIGN(width, ALIGN32) * 2;
        case HAL_PIXEL_FORMAT_RGB_888:
            return ALIGN(width, ALIGN32) * 3;
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
            return ALIGN(width, ALIGN32) * 4;
        case HAL_PIXEL_FORMAT_RGBA_5551:
        case HAL_PIXEL_FORMAT_RGBA_4444:
            return ALIGN(width, ALIGN32) * 2;
        default:
            return width;
    }

    return width;
}

uint32_t V4L2Wrapper::CalcStride1(uint32_t halFormat, size_t width) {
    switch (halFormat) {
        case HAL_PIXEL_FORMAT_CbYCrY_422_I:
        case HAL_PIXEL_FORMAT_YCBCR_422_I:
            return 0;
        case HAL_PIXEL_FORMAT_YCbCr_420_SP:
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            return ALIGN(width, ALIGN16);
        case HAL_PIXEL_FORMAT_NV12_ENCODEABLE:
            return ALIGN(width, ALIGN128);
        case HAL_PIXEL_FORMAT_YCbCr_420_SP_TILED:
            return ALIGN(width, ALIGN128);
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
            return ALIGN(width, ALIGN16);
        case HAL_PIXEL_FORMAT_RGB_565:
        case HAL_PIXEL_FORMAT_RGB_888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
        case HAL_PIXEL_FORMAT_RGBA_5551:
        case HAL_PIXEL_FORMAT_RGBA_4444:
            return 0;
        default:
            return 0;
    }

    return 0;
}

size_t V4L2Wrapper::CalcSize(uint32_t halFormat, size_t width, size_t height) {
    int32_t size = 0;
    int32_t alignedw = 0;
    int32_t alignedh = 0;

    switch (halFormat) {
        case HAL_PIXEL_FORMAT_CbYCrY_422_I:
        case HAL_PIXEL_FORMAT_YCBCR_422_I:
            size = ALIGN(ALIGN(width * 2, ALIGN64) * height, ALIGN4K);
            break;
        case HAL_PIXEL_FORMAT_YCbCr_420_SP:
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            alignedw = ALIGN(width, ALIGN16);
            size = ALIGN((alignedw * height) + (ALIGN((width+1)/2, ALIGN32) * ((height+1)/2) * 2), ALIGN4K);
            break;
        case HAL_PIXEL_FORMAT_NV12_ENCODEABLE:
        case HAL_PIXEL_FORMAT_YCbCr_420_SP_TILED:
            // TODO:
            alignedw = ALIGN(width, ALIGN128);
            size = ALIGN((alignedw * height) + (ALIGN((width+1)/2, ALIGN32) * ((height+1)/2) * 2), ALIGN4K);
            break;
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
            alignedw = ALIGN(width, ALIGN16);
            alignedh = ALIGN(height, ALIGN64);
            size = ALIGN((alignedw * alignedh) + (ALIGN((width+1)/2, ALIGN16) * ((alignedh+1)/2) * 2), ALIGN4K);
            break;
        case HAL_PIXEL_FORMAT_RGB_565:
            size = ALIGN(width, ALIGN32) * 2;
            break;
        case HAL_PIXEL_FORMAT_RGB_888:
            size = ALIGN(width, ALIGN32) * 3;
            break;
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
            size = ALIGN(width, ALIGN32) * 4;
            break;
        case HAL_PIXEL_FORMAT_RGBA_5551:
        case HAL_PIXEL_FORMAT_RGBA_4444:
            size = ALIGN(width, ALIGN32) * 2;
            break;
        default:
            HAL_LOGE("Unsupported format:0x%x", halFormat);
            return 0;
    }

    return size;
}

size_t V4L2Wrapper::CalcPlane0Size(uint32_t halFormat, size_t width, size_t height) {
    int32_t size = 0;
    int32_t alignedw = 0;
    int32_t alignedh = 0;

    switch (halFormat) {
        case HAL_PIXEL_FORMAT_CbYCrY_422_I:
        case HAL_PIXEL_FORMAT_YCBCR_422_I:
            size = ALIGN(ALIGN(width * 2, ALIGN64) * height, ALIGN4K);
            break;
        case HAL_PIXEL_FORMAT_YCbCr_420_SP:
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            alignedw = ALIGN(width, ALIGN16);
            size = alignedw * height;
            break;
        case HAL_PIXEL_FORMAT_NV12_ENCODEABLE:
        case HAL_PIXEL_FORMAT_YCbCr_420_SP_TILED:
            // TODO:
            alignedw = ALIGN(width, ALIGN128);
            size = alignedw * height;
            break;
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
            alignedw = ALIGN(width, ALIGN16);
            alignedh = ALIGN(height, ALIGN64);
            size = alignedw * alignedh;
            break;
        case HAL_PIXEL_FORMAT_RGB_565:
            size = ALIGN(width, ALIGN32) * 2;
            break;
        case HAL_PIXEL_FORMAT_RGB_888:
            size = ALIGN(width, ALIGN32) * 3;
            break;
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
            size = ALIGN(width, ALIGN32) * 4;
            break;
        case HAL_PIXEL_FORMAT_RGBA_5551:
        case HAL_PIXEL_FORMAT_RGBA_4444:
            size = ALIGN(width, ALIGN32) * 2;
            break;
        default:
            HAL_LOGE("Unsupported format:0x%x", halFormat);
            return 0;
    }

    return size;
}

size_t V4L2Wrapper::CalcPlane1Size(uint32_t halFormat, size_t width, size_t height) {
    int32_t size = 0;
    int32_t alignedw = 0;
    int32_t alignedh = 0;

    switch (halFormat) {
        case HAL_PIXEL_FORMAT_CbYCrY_422_I:
        case HAL_PIXEL_FORMAT_YCBCR_422_I:
            size = 0;
            break;
        case HAL_PIXEL_FORMAT_YCbCr_420_SP:
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            alignedw = ALIGN(width, ALIGN16);
            size = (ALIGN((width+1)/2, ALIGN32) * ((height+1)/2)) * 2;
            break;
        case HAL_PIXEL_FORMAT_NV12_ENCODEABLE:
        case HAL_PIXEL_FORMAT_YCbCr_420_SP_TILED:
            // TODO:
            alignedw = ALIGN(width, ALIGN128);
            size = (ALIGN((width+1)/2, ALIGN32) * ((height+1)/2)) * 2;
            break;
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
            alignedw = ALIGN(width, ALIGN16);
            alignedh = ALIGN(height, ALIGN64);
            size = (ALIGN((width+1)/2, ALIGN32) * ((alignedh+1)/2)) * 2;
            break;
        case HAL_PIXEL_FORMAT_RGB_565:
        case HAL_PIXEL_FORMAT_RGB_888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
        case HAL_PIXEL_FORMAT_RGBA_5551:
        case HAL_PIXEL_FORMAT_RGBA_4444:
        default:
            size = 0;
    }

    return size;
}

size_t V4L2Wrapper::CalcPlane2Size(uint32_t halFormat, size_t width, size_t height) {
    int32_t size = 0;
    int32_t alignedw = 0;

    switch (halFormat) {
        case HAL_PIXEL_FORMAT_CbYCrY_422_I:
        case HAL_PIXEL_FORMAT_YCBCR_422_I:
        case HAL_PIXEL_FORMAT_YCbCr_420_SP:
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
        case HAL_PIXEL_FORMAT_NV12_ENCODEABLE:
        case HAL_PIXEL_FORMAT_YCbCr_420_SP_TILED:
            size = 0;
            break;
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
            size = 0;
            // alignedw = ALIGN(width, ALIGN16);
            // size = ALIGN((width+1)/2, ALIGN16) * ((height+1)/2);
            break;
        case HAL_PIXEL_FORMAT_RGB_565:
        case HAL_PIXEL_FORMAT_RGB_888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
        case HAL_PIXEL_FORMAT_RGBA_5551:
        case HAL_PIXEL_FORMAT_RGBA_4444:
        default:
            size = 0;
    }

    return size;
}

void * V4L2Wrapper::GetMappedGPUAddr(int bufFD, void *bufPtr, size_t bufLen) {
    C2D_STATUS status;
    void *gpuaddr = NULL;

    status = c2dMapAddr(bufFD, bufPtr, bufLen, 0, KGSL_USER_MEM_TYPE_ION, &gpuaddr);
    if (status != C2D_STATUS_OK) {
        HAL_LOGE("c2dMapAddr failed: status %d fd %d ptr %p len %zu flags %d",
                status, bufFD, bufPtr, bufLen, KGSL_USER_MEM_TYPE_ION);
        return NULL;
    }

    HAL_LOGV("fd: %d, bufPtr:%p, bufLen:%zu, gpuaddr:%p", bufFD, bufPtr, bufLen, gpuaddr);
    return gpuaddr;
}

int32_t V4L2Wrapper::CreateC2dSurface(buffer_info_t * buff_info, bool isSource) {
    if (buff_info == NULL) {
        HAL_LOGE("ERROR: invalid parameter");
        return -1;
    }

    if (IsYuvformat(buff_info->halFormat)) {
        return CreateYuvC2dSurface(buff_info, isSource);
    } else {
        return CreateRgbC2dSurface(buff_info, isSource);
    }
}

int32_t V4L2Wrapper::CreateYuvC2dSurface(buffer_info_t * buff_info, bool isSource) {
    if (buff_info == NULL) {
        HAL_LOGE("ERROR: Invalid parameter");
        return -1;
    }

    if (buff_info->fd < 0 || buff_info->offset < 0 || buff_info->length < 0) {
        HAL_LOGE("ERROR: bad parameter halformat:%s, w:%d,h:%d,fd:%d,offset:%d,len:%d, source:%d",
                printHalFormat(buff_info->halFormat), buff_info->width, buff_info->height,
                buff_info->fd, buff_info->offset, buff_info->length, isSource);
        return -1;
    }

    HAL_LOGV("halformat:%s, w:%d,h:%d,fd:%d,offset:%d,len:%d, source:%d",
            printHalFormat(buff_info->halFormat), buff_info->width, buff_info->height,
            buff_info->fd, buff_info->offset, buff_info->length, isSource);

    buff_info->virtPtr = NULL;
    buff_info->gpuAddr = NULL;

    void * bufPtr = mmap(NULL, buff_info->length, PROT_WRITE | PROT_READ, MAP_SHARED,
            buff_info->fd, 0);

    HAL_LOGV("mmap vptr = %p", bufPtr);
    if (bufPtr == NULL || (bufPtr == MAP_FAILED)) {
        HAL_LOGE("ERROR: mmap failed: %s", strerror(errno));
        return -1;
    }
    void *c2dGpuAddr = NULL;
    C2D_STATUS c2d_status = C2D_STATUS_NOT_SUPPORTED;

    c2d_status = c2dMapAddr(buff_info->fd, bufPtr, buff_info->length, 0,
            KGSL_USER_MEM_TYPE_ION, &c2dGpuAddr);
    if (c2d_status != C2D_STATUS_OK) {
        HAL_LOGE("ERROR: c2dMapAddr failed:%d", c2d_status);
        munmap(bufPtr, buff_info->length);
        return -1;

    }

    HAL_LOGV("virtual addr: %p, GPU addr: %p", bufPtr, c2dGpuAddr);

    C2D_YUV_SURFACE_DEF yuvSurfaceDef;

    memset((void *)&yuvSurfaceDef, 0, sizeof(C2D_YUV_SURFACE_DEF));

    yuvSurfaceDef.format = GetC2dFormat(buff_info->halFormat, isSource);
    yuvSurfaceDef.width = buff_info->width;
    yuvSurfaceDef.height = buff_info->height;
    yuvSurfaceDef.plane0 = bufPtr;
    yuvSurfaceDef.phys0 = c2dGpuAddr;
    yuvSurfaceDef.stride0 = CalcStride0(buff_info->halFormat, buff_info->width);

    if (CalcPlane1Size(buff_info->halFormat, buff_info->width, buff_info->height) > 0) {
        int32_t plane0Size = CalcPlane0Size(buff_info->halFormat, buff_info->width, buff_info->height);
        yuvSurfaceDef.plane1 = (void *) ((unsigned char *)bufPtr + plane0Size);

        if (c2dGpuAddr != NULL) {
            yuvSurfaceDef.phys1 = (void *) ((unsigned char *)c2dGpuAddr + plane0Size);
        }
        yuvSurfaceDef.stride1 = CalcStride1(buff_info->halFormat, buff_info->width);
    }

    if (CalcPlane2Size(buff_info->halFormat, buff_info->width, buff_info->height) > 0) {
        int32_t plane0Size = CalcPlane0Size(buff_info->halFormat, buff_info->width, buff_info->height);
        int32_t plane1Size = CalcPlane1Size(buff_info->halFormat, buff_info->width, buff_info->height);

        yuvSurfaceDef.plane2 = (void *) ((unsigned char *)bufPtr + plane0Size + plane1Size);

        if (c2dGpuAddr != NULL) {
            yuvSurfaceDef.phys2 = (void *) ((unsigned char *)c2dGpuAddr + plane0Size + plane1Size);
        }

        yuvSurfaceDef.stride2 = 0;
    }

    HAL_LOGV("format:0x%x, w:%d, h:%d, plan0:%p, plane1:%p, plane2:%p, phys0:%p, phys1:%p, phys2:%p, s0:%d, s1:%d, s2:%d",
            yuvSurfaceDef.format, yuvSurfaceDef.width, yuvSurfaceDef.height,
            yuvSurfaceDef.plane0, yuvSurfaceDef.plane1, yuvSurfaceDef.plane2,
            yuvSurfaceDef.phys0, yuvSurfaceDef.phys1, yuvSurfaceDef.phys2,
            yuvSurfaceDef.stride0, yuvSurfaceDef.stride1, yuvSurfaceDef.stride2);

    uint32_t surfaceId = 0;
    int surface_type = C2D_SURFACE_YUV_HOST;

    if (c2dGpuAddr != NULL) {
        surface_type = surface_type | C2D_SURFACE_WITH_PHYS;
    }

    c2d_status = c2dCreateSurface(&surfaceId,  isSource ? C2D_SOURCE : C2D_TARGET,
            (C2D_SURFACE_TYPE)surface_type, &yuvSurfaceDef);
    if (c2d_status != C2D_STATUS_OK) {
        HAL_LOGE("ERROR:create c2d surface failed: %d", c2d_status);
        c2dUnMapAddr(c2dGpuAddr);
        munmap(bufPtr, buff_info->length);
        return -1;
    }

    HAL_LOGV("surfaceId: 0x%x", surfaceId);

    buff_info->virtPtr = bufPtr;
    buff_info->gpuAddr = c2dGpuAddr;
    buff_info->surfaceId = surfaceId;
    buff_info->isUsed = true;
    return 0;
}

int32_t V4L2Wrapper::CreateRgbC2dSurface(buffer_info_t * buff_info, bool isSource) {
    if (buff_info == NULL) {
        HAL_LOGE("ERROR: Invalid parameter");
        return -1;
    }

    if (buff_info->fd < 0 || buff_info->offset < 0 || buff_info->length < 0) {
        HAL_LOGE("ERROR: bad parameter halformat:%s, w:%d,h:%d,fd:%d,offset:%d,len:%d, source:%d",
                printHalFormat(buff_info->halFormat), buff_info->width, buff_info->height,
                buff_info->fd, buff_info->offset, buff_info->length, isSource);
        return -1;
    }

    HAL_LOGV("halformat:%s, w:%d,h:%d,fd:%d,offset:%d,len:%d, source:%d",
            printHalFormat(buff_info->halFormat), buff_info->width, buff_info->height,
            buff_info->fd, buff_info->offset, buff_info->length, isSource);

    buff_info->virtPtr = NULL;
    buff_info->gpuAddr = NULL;

    void * bufPtr = mmap(NULL, buff_info->length, PROT_WRITE | PROT_READ, MAP_SHARED,
            buff_info->fd, buff_info->offset);
    if ((bufPtr == NULL) || (bufPtr == MAP_FAILED)) {
        HAL_LOGE("ERROR: mmap failed: %s", strerror(errno));
        return -1;
    }

    C2D_STATUS c2d_status;
    void *c2dGpuAddr = NULL;
    c2d_status = c2dMapAddr(buff_info->fd, bufPtr, buff_info->length, buff_info->offset,
            KGSL_USER_MEM_TYPE_ION, &c2dGpuAddr);
    if (c2d_status != C2D_STATUS_OK) {
        HAL_LOGE("ERROR: c2dMapAddr failed:%d", c2d_status);
        munmap(bufPtr, buff_info->length);
        return -1;
    }

    HAL_LOGV("virtual addr: %p, GPU addr: %p, offset:%d, size:%d, w:%d, h:%d, format:%s", bufPtr, c2dGpuAddr,
            buff_info->offset, buff_info->length,
            buff_info->width, buff_info->height, printHalFormat(buff_info->halFormat));

    C2D_RGB_SURFACE_DEF rgbSurfaceDef;

    memset((void *)&rgbSurfaceDef, 0, sizeof(C2D_RGB_SURFACE_DEF));

    rgbSurfaceDef.format = GetC2dFormat(buff_info->halFormat, isSource);
    rgbSurfaceDef.width = buff_info->width;
    rgbSurfaceDef.height = buff_info->height;
    rgbSurfaceDef.buffer = bufPtr;
    rgbSurfaceDef.phys = c2dGpuAddr;
    rgbSurfaceDef.stride = CalcStride0(buff_info->halFormat, buff_info->width);

    HAL_LOGV("format:0x%x, w:%d, h:%d, buffer:%p, phys:%p, stride:%d",
            rgbSurfaceDef.format, rgbSurfaceDef.width, rgbSurfaceDef.height,
            rgbSurfaceDef.buffer, rgbSurfaceDef.phys, rgbSurfaceDef.stride);

    uint32_t surfaceId = 0;
    int surface_type = C2D_SURFACE_RGB_HOST;

    if (c2dGpuAddr != NULL) {
        surface_type = surface_type | C2D_SURFACE_WITH_PHYS;
    }

    c2d_status = c2dCreateSurface(&surfaceId, isSource ? C2D_SOURCE : C2D_TARGET,
            (C2D_SURFACE_TYPE)surface_type, &rgbSurfaceDef);
    if (c2d_status != C2D_STATUS_OK) {
        HAL_LOGE("ERROR:create c2d surface failed: %d", c2d_status);
        c2dUnMapAddr(c2dGpuAddr);
        munmap(bufPtr, buff_info->length);
        return -1;
    }

    HAL_LOGV("surfaceId: 0x%x", surfaceId);

    buff_info->virtPtr = bufPtr;
    buff_info->gpuAddr = c2dGpuAddr;
    buff_info->surfaceId = surfaceId;
    buff_info->isUsed = true;
    return 0;
}

int32_t V4L2Wrapper::ReleaseC2dSurface(buffer_info_t * buff_info) {
    if (buff_info == NULL) {
        HAL_LOGE("ERROR: Invalid parameter");
        return -1;
    }
    std::lock_guard<std::mutex> lock(buff_info->lock);

    if (buff_info->fd < 0 || buff_info->offset < 0 || buff_info->length < 0) {
        HAL_LOGE("ERROR: bad parameter halformat:%s, w:%d,h:%d,fd:%d,offset:%d,len:%d",
                printHalFormat(buff_info->halFormat), buff_info->width, buff_info->height,
                buff_info->fd, buff_info->offset, buff_info->length);
        return -1;
    }

    HAL_LOGV("parameter halformat:%s, w:%d,h:%d,fd:%d,offset:%d,len:%d",
            printHalFormat(buff_info->halFormat), buff_info->width, buff_info->height,
            buff_info->fd, buff_info->offset, buff_info->length);

    if (buff_info->virtPtr != NULL) {
        munmap(buff_info->virtPtr, buff_info->length);;
    }

    if (buff_info->gpuAddr != NULL) {
        c2dUnMapAddr(buff_info->gpuAddr);
    }

    c2dDestroySurface(buff_info->surfaceId);

    buff_info->gpuAddr = NULL;
    buff_info->virtPtr = NULL;
    buff_info->surfaceId = 0;
    buff_info->isUsed = false;
    buff_info->id = 0;

    if (buff_info->fd >= 0) {
        close(buff_info->fd);
    }

    memset(buff_info, 0, sizeof(buffer_info_t));
    return 0;
}

int32_t V4L2Wrapper::DumpFrameBuffer(char *buffer, int32_t size, bool raw)
{
    FILE *fp;
    size_t numBytesWritten = 0;
    char filename[128] = "";

    if (!buffer || size <= 0)
    {
        HAL_LOGE("ERROR: bad input parameter, buffer:%p, size:%d, file name:%p",
                buffer, size, filename);
        return -1;
    }

    if (mFrameNum % 100 != 0) {
        return -1;
    }

    snprintf(filename, sizeof(filename), "/data/vendor/camera/frame_%s_%d.raw",
            raw ? "sensor" : "c2d" ,mFrameNum);
    fp = fopen(filename, "w+");
    HAL_LOGE("dumping qcarcam frame %s numbytes %d", filename, size);

    if (0 != fp)
    {
        numBytesWritten = fwrite(buffer, 1, size, fp);

        if (numBytesWritten != size)
        {
            HAL_LOGE("ERROR: write buffer to file failed, write:%zu, total:%d",
                    numBytesWritten, size);
        }
        fclose(fp);
    }
    else
    {
        HAL_LOGE("failed to open file:%s", strerror(errno));
        return -1;
    }

    return 0;
}


int32_t V4L2Wrapper::DoC2dScale(buffer_info_t * dst_buff_info, bool useTestPattern) {
    HAL_LOG_ENTER();
    if (dst_buff_info == NULL) {
        HAL_LOGE("ERROR: bad parameter");
        return -1;
    }
    std::lock_guard<std::mutex> lock_dst(dst_buff_info->lock);

    if (!dst_buff_info->isUsed) {
        HAL_LOGE("ERROR: check buffer still valid: dst:%d", dst_buff_info->isUsed);
        return -1;
    }

#ifdef PRINT_BUFFER_INFO
    HAL_LOGE("Source buffer info");
    printBufferInfo(src_buff_info);
    HAL_LOGE("Destination buffer info");
    printBufferInfo(dst_buff_info);
#endif

    C2D_STATUS c2d_status = C2D_STATUS_OK;
    C2D_OBJECT c2dObject;
    memset(&c2dObject, 0, sizeof(c2dObject));

    c2dObject.source_rect.height = mAisFrameHeight << 16;
    c2dObject.source_rect.width  = mAisFrameWidth << 16;

    c2dObject.source_rect.x      = 0 << 16;
    c2dObject.source_rect.y      = 0 << 16;
    c2dObject.target_rect.x      = 0 << 16;
    c2dObject.target_rect.y      = 0 << 16;
    c2dObject.config_mask = C2D_TARGET_RECT_BIT;
    c2dObject.config_mask |= C2D_NO_PIXEL_ALPHA_BIT;
    c2dObject.config_mask |= C2D_SOURCE_RECT_BIT;
    c2dObject.config_mask |= C2D_NO_BILINEAR_BIT;
    c2dObject.config_mask |= C2D_NO_ANTIALIASING_BIT;
    c2dObject.target_rect.height = dst_buff_info->height << 16;
    c2dObject.target_rect.width  = dst_buff_info->width << 16;
    if (true == useTestPattern) {
        c2dObject.bg_color = 0x0c0c0c;
    }
    else {
    c2dObject.surface_id = mpQcarcamMmapBufs[mQcarcamFrameInfo.idx].srcC2dSurfaceId;

        c2d_status = c2dSurfaceUpdated(c2dObject.surface_id, &c2dObject.source_rect);
        if (c2d_status != C2D_STATUS_OK) {
            HAL_LOGE("ERROR:c2dSurfaceUpdated src failed: %d", c2d_status);
            return -1;
        }
    }

    c2d_status = c2dSurfaceUpdated(dst_buff_info->surfaceId, &c2dObject.target_rect);
    if (c2d_status != C2D_STATUS_OK) {
        HAL_LOGE("ERROR:c2dSurfaceUpdated dst failed: %d", c2d_status);
        return -1;
    }

    c2d_status = c2dDraw(dst_buff_info->surfaceId, C2D_TARGET_ROTATE_0, TARGET_SCISSOR,
            TARGET_MASK_ID, TARGET_COLOR_KEY, &c2dObject, NO_OF_OBJECTS);
    if (c2d_status != C2D_STATUS_OK) {
        HAL_LOGE("ERROR:c2d draw failed: %d", c2d_status);
        return -1;
    }

    c2d_status = c2dFinish(dst_buff_info->surfaceId);
    if (c2d_status != C2D_STATUS_OK) {
        HAL_LOGE("ERROR:c2d finish failed: %d", c2d_status);
        return -1;
    }

    HAL_LOG_EXIT();
    return 0;
}


bool V4L2Wrapper::isTestPatternSet(std::shared_ptr<default_camera_hal::CaptureRequest> request) {

    int testPatternMode, entry;
    bool useTestPattern = false;
    testPatternMode = ANDROID_SENSOR_TEST_PATTERN_MODE_OFF; // Default to OFF

    // Read the test pattern mode from metadata
    int res = v4l2_camera_hal::SingleTagValue(
        request->settings, ANDROID_SENSOR_TEST_PATTERN_MODE, &entry);
    if (res == 0) {
        testPatternMode = entry; // Read the test pattern mode
        HAL_LOGD("Test Pattern Mode: %d", testPatternMode);
    } else {
        HAL_LOGD("Test Pattern Mode not found in metadata, defaulting to OFF");
    }

    switch (testPatternMode) {
        case ANDROID_SENSOR_TEST_PATTERN_MODE_SOLID_COLOR:
        case ANDROID_SENSOR_TEST_PATTERN_MODE_BLACK:
            HAL_LOGI("Selecting Solid Color Test Pattern");
            useTestPattern = true;
            break;
        default:
            HAL_LOGI("No test pattern Selected, normal capture");
            break;
    }
    return useTestPattern;
}

bool V4L2Wrapper::DequeueRequest() {

    qcarcam_ret_t ret = QCARCAM_RET_OK;
    qcarcam_event_msg_t event_msg;
    int res;
    std::shared_ptr<CaptureRequest> request;

    HAL_LOG_ENTER();
    std::unique_lock<std::mutex> lock(streamStatusLock);
    if (streamstatus != STREAM_STATUS_STREAM_ON) {
        HAL_LOGE("Stream is not started");
        streamStatusSignal.wait(lock);
    }
    lock.unlock();

    std::unique_lock<std::mutex> queue_lock(input_ctxt.queue_mutex);
    input_ctxt.eventHandlerSignal.wait(queue_lock);
    if (!input_ctxt.eventqueue.empty())
    {
        event_msg = input_ctxt.eventqueue.front();
        HAL_LOGD("event msg id received is %d",event_msg.event_id);
        input_ctxt.eventqueue.pop();
    }
    else
    {
        HAL_LOGE("event queue is empty");
        queue_lock.unlock();
        return true;
    }

    queue_lock.unlock();

    bool result = false;
    switch (event_msg.event_id)
    {
        case QCARCAM_EVENT_FRAME_READY:
            HAL_LOGD("DequeueRequest::Frame Ready is available");
            break;
        case QCARCAM_EVENT_ERROR:
            HAL_LOGE("DequeueRequest::Frame Event Error");
            result = true;
            break;
        default:
            HAL_LOGE("DequeueRequest::Unsupported Event");
            result = true;
            break;
    }

    if(true != result)
    {
        request = PopRequest();
        HAL_LOGV("calling VIDIOC_DQBUF frame num:%d", request->frame_number);

  ret = qcarcam_get_frame(qcarcam_context, &mQcarcamFrameInfo, QCARCAM_DEFAULT_GET_FRAME_TIMEOUT, 0);
        if (QCARCAM_RET_OK != ret) {
            HAL_LOGE("qcarcam_get_frame failed %p ret %d", qcarcam_context, ret);
            if (mCamera != NULL) {
                request->output_buffers[0].status = CAMERA3_BUFFER_STATUS_ERROR;
                mCamera->enqueueCompleteRequest(request);
            }
            return true;
        }

        void *pData = NULL;

        if (!mpQcarcamMmapBufs[mQcarcamFrameInfo.idx].ptr) {
            HAL_LOGE("buffer is not mapped");
            goto EXIT_FLAG;
        }

        pData = (void *)mpQcarcamMmapBufs[mQcarcamFrameInfo.idx].ptr;

#ifdef USE_HYP
      //use QNX timestamp
      // if the first time to get buffer , Record the difference in time between Android and QNX systems
      //Update the tag ANDROID_SENSOR_TIMESTAMP with the IFE Timestamp
      {
          struct timespec ts;
          uint64_t timestamp_qnx = 0;
          uint64_t timestamp_al = 0;

          int res = clock_gettime(CLOCK_MONOTONIC, &ts);
          if (res) {
              HAL_LOGE("Failed to get BOOTTIME for state delegate: %d (%s)",
                       errno,
                       strerror(errno));
          }
          timestamp_al = ts.tv_sec * 1000000000ULL + ts.tv_nsec;

          mTimesBetweentAndroidQnx = mQcarcamFrameInfo.timestamp_system - timestamp_al;
          mQcarcamFrameInfo.timestamp_system = mQcarcamFrameInfo.timestamp_system - mTimesBetweentAndroidQnx;

          if (mQcarcamFrameInfo.timestamp_system <= timestamp_al) {
              UpdateMetadata(&(request->settings), ANDROID_SENSOR_TIMESTAMP, (int64_t)mQcarcamFrameInfo.timestamp_system);
              HAL_LOGD("X, timestamp: %llu", mQcarcamFrameInfo.timestamp_system);
          } else {
              UpdateMetadata(&(request->settings), ANDROID_SENSOR_TIMESTAMP, (int64_t)timestamp_al);
              HAL_LOGD("X, timestamp: %llu", timestamp_al);
          }
     }

#else
     {
          UpdateMetadata(&(request->settings), ANDROID_SENSOR_TIMESTAMP, (int64_t)mQcarcamFrameInfo.timestamp_system);
     }
#endif

        // Now, check if the test pattern is set
        bool useTestPattern = isTestPatternSet(request);

        buffer_info_t * dst_buff_info = NULL;

        //DumpFrameBuffer((char *)pData, mpQcarcamMmapBufs[mQcarcamFrameInfo.idx].size, true);

        for(auto it = request->output_buffers.begin();
                it != request->output_buffers.end(); it++){

            int buf_id = get_native_hnd_buffer_id(*(it->buffer));
            dst_buff_info = GetOutputBufferInfo(buf_id);
            if (dst_buff_info == nullptr) {
                HAL_LOGE("ERROR: can't find output c2d surface, format:0x%x, w:%d,h:%d, id:%" PRIu64 "",
                        it->stream->format, it->stream->width, it->stream->height, buf_id);
                if (mCamera != NULL) {
                    request->output_buffers[0].status = CAMERA3_BUFFER_STATUS_ERROR;
                    mCamera->enqueueCompleteRequest(request);
                }
                goto EXIT_FLAG;
            }

            HAL_LOGV(" output c2d surface, format:0x%x, w:%d,h:%d, id:%d",
                    it->stream->format, it->stream->width, it->stream->height, buf_id);
            res = DoC2dScale(dst_buff_info, useTestPattern);

            if (res != 0) {
                HAL_LOGE("ERROR: c2d process failed: output format:0x%x, w:%d, h:%d",
                        it->stream->format, it->stream->width, it->stream->height);
                if (mCamera != NULL) {
                    request->output_buffers[0].status = CAMERA3_BUFFER_STATUS_ERROR;
                    mCamera->enqueueCompleteRequest(request);
                }
                goto EXIT_FLAG;
            }

            //DumpFrameBuffer((char *)dst_buff_info->virtPtr, dst_buff_info->length, false);

            if (it->stream->format == HAL_PIXEL_FORMAT_BLOB) {
                ProcessSnapshotData(&(*it), request->settings, dst_buff_info);
                DeleteOutputBufferInfo(dst_buff_info);
                ReleaseC2dSurface(dst_buff_info);
            }
        }
        ret = qcarcam_release_frame(qcarcam_context, mQcarcamFrameInfo.idx);
        if (QCARCAM_RET_OK != ret)
            HAL_LOGE("qcarcam_release_frame() failed %d", mQcarcamFrameInfo.idx);
        process3A(request->settings);

        if (mCamera != NULL) {
            request->output_buffers[0].status = CAMERA3_BUFFER_STATUS_OK;
            mCamera->enqueueCompleteRequest(request);
        }

        HAL_LOGV("X, complete request:%d", request->frame_number);
    }
    return true;

EXIT_FLAG:
    ret = qcarcam_release_frame(qcarcam_context, mQcarcamFrameInfo.idx);
    if (QCARCAM_RET_OK != ret)
        HAL_LOGE("qcarcam_release_frame() failed %d", mQcarcamFrameInfo.idx);
    return true;
}

int V4L2Wrapper::ProcessSnapshotData(const camera3_stream_buffer_t* stream_buffer,
        android::hardware::camera::common::V1_0::helper::CameraMetadata& settings,
        buffer_info_t * dst_buff_info){
    /* Below code to handle the Snapshot and video recording requests  */
    unsigned long jpeglength;
    void *bufptr = NULL;
    void *exifbufptr = NULL;
    int exifbufptrNum = 0;
    arc::ExifUtils *exif_info = new arc::ExifUtils();
    camera3_jpeg_blob_t jpegHeader;

    if (stream_buffer == NULL || dst_buff_info == NULL) {
        HAL_LOGE("ERROR: bad parameter, buffer: %p, dst buff info:%p",
                stream_buffer, dst_buff_info);
        delete(exif_info);
        return -1;
    }

    //Use ExifUtils to add some EXIF information to jpeg.
    exif_info->Initialize((unsigned char*)dst_buff_info->virtPtr,
            stream_buffer->stream->width,
            stream_buffer->stream->height, 6);
    exif_info->SetDefaultTag();
    SetExifTags(settings,exif_info);
    exif_info->GenerateApp1();
    exifbufptrNum = exif_info->GetApp1Length();

#if defined(ANDROID_Q_AOSP)
  struct private_handle_t *snapshot;
  snapshot = (struct private_handle_t *)(*(stream_buffer->buffer));
  /* Maping to Android native handler */
  exifbufptr = mmap(NULL, snapshot->size, PROT_WRITE | PROT_READ, MAP_SHARED, snapshot->fd, 0);
  if(exifbufptr == NULL) {
    HAL_LOGE("ERROR: mmap snapshot buffer failed, fd:%d, size:%d", snapshot->fd, snapshot->size);
    delete(exif_info);
    return -EAGAIN;
  }

  bufptr = (unsigned char*) exifbufptr + exifbufptrNum + 6;
  jpeglength = snapshot->size - exifbufptrNum - 6 - sizeof(jpegHeader);

  /* Jpeg encoder */
  JpegEncode(bufptr, (unsigned char *)dst_buff_info->virtPtr, //output,
                      stream_buffer->stream->width, stream_buffer->stream->height,
                      &jpeglength);
  if ((jpeglength == 0) || (jpeglength > (snapshot->size - sizeof(jpegHeader)))) {
    munmap(exifbufptr, snapshot->size);
    HAL_LOGE("ERROR: JpegEncode failed, len:%ld, snapshot size:%d, header size:%zu",
                jpeglength, snapshot->size, sizeof(jpegHeader));
    delete(exif_info);
    return -EAGAIN;
  }

#else
    int snapshot_id = get_native_hnd_buffer_id(*(stream_buffer->buffer));
    int snapshot_fd = get_native_hnd_buffer_fd(*(stream_buffer->buffer));
    int snapshot_size = get_native_hnd_buffer_size(*(stream_buffer->buffer));

    /* Maping to Android native handler */
    exifbufptr = mmap(NULL, snapshot_size, PROT_WRITE | PROT_READ, MAP_SHARED, snapshot_fd, 0);
    if(exifbufptr == NULL) {
        HAL_LOGE("ERROR: mmap snapshot buffer failed, fd:%d, size:%d", snapshot_fd, snapshot_size);
        delete(exif_info);
        return -EAGAIN;
    }

    bufptr = (unsigned char*) exifbufptr + exifbufptrNum + 6;
    jpeglength = snapshot_size - exifbufptrNum - 6 - sizeof(jpegHeader);

    /* Jpeg encoder */
    JpegEncode(bufptr, (unsigned char *)dst_buff_info->virtPtr, //output,
            stream_buffer->stream->width, stream_buffer->stream->height,
            &jpeglength);
    if ((jpeglength == 0) || (jpeglength > (snapshot_size - sizeof(jpegHeader)))) {
        munmap(exifbufptr, snapshot_size);
        HAL_LOGE("ERROR: JpegEncode failed, len:%ld, snapshot size:%d, header size:%zu",
                jpeglength, snapshot_size, sizeof(jpegHeader));
        delete(exif_info);
        return -EAGAIN;
    }
#endif

    jpegHeader.jpeg_blob_id = CAMERA3_JPEG_BLOB_ID;
    jpegHeader.jpeg_size = jpeglength + exifbufptrNum + 6;

    memcpy(((char*)exifbufptr + 6),exif_info->GetApp1Buffer() , exifbufptrNum );

    // Modify some necessary header information
    // 0xff 0xd8 represents the beginning of a JPEG
    // 0xff 0xe1 represents the beginning of the EXIF message
    unsigned char *p = (unsigned char *)exifbufptr;
    p[0] = 0xff;
    p[1] = 0xd8;
    p[2] = 0xff;
    p[3] = 0xe1;
    exifbufptrNum += 4;
    p[4] = exifbufptrNum>>8;
    p[5] = exifbufptrNum&0xff;

    memcpy((reinterpret_cast<char*>(exifbufptr) + jpeglength + exifbufptrNum + 6), &jpegHeader, sizeof(jpegHeader));
#if defined(ANDROID_Q_AOSP)
  munmap(exifbufptr, snapshot->size);
#else
    munmap(exifbufptr, snapshot_size);
#endif
    delete(exif_info);

    HAL_LOGV("JPEG size: %lu, exif size: %d----x", jpeglength, exifbufptrNum);
    return 0;
}

int32_t V4L2Wrapper::process3A(android::hardware::camera::common::V1_0::helper::CameraMetadata &settings) {
    int32_t res;
    camera_metadata_entry entry;

    entry = settings.find(ANDROID_CONTROL_MODE);
    if (entry.count == 0) {
        HAL_LOGE("%s: No control mode entry!", __FUNCTION__);
        return -1;
    }

    uint8_t controlMode = entry.data.u8[0];
    if (controlMode == ANDROID_CONTROL_MODE_OFF) {
        mAeMode   = ANDROID_CONTROL_AE_MODE_OFF;
        mAfMode   = ANDROID_CONTROL_AF_MODE_OFF;
        mAwbMode  = ANDROID_CONTROL_AWB_MODE_OFF;
        mAeState  = ANDROID_CONTROL_AE_STATE_INACTIVE;
        mAfState  = ANDROID_CONTROL_AF_STATE_INACTIVE;
        mAwbState = ANDROID_CONTROL_AWB_STATE_INACTIVE;

        update3A(settings);

        return 0;
    }  else if (controlMode == ANDROID_CONTROL_MODE_USE_SCENE_MODE) {
        entry = settings.find(ANDROID_CONTROL_SCENE_MODE);
        if (entry.count == 0) {
            HAL_LOGE("%s: No scene mode entry!", __FUNCTION__);
            return -1;
        }
        uint8_t sceneMode = entry.data.u8[0];

        switch (sceneMode) {
            case ANDROID_CONTROL_SCENE_MODE_FACE_PRIORITY:
                mFacePriority = true;
                break;
            default:
                HAL_LOGE("%s: doesn't support scene mode %d",
                        __FUNCTION__, sceneMode);
                return -1;
        }
    } else {
        mFacePriority = false;
    }

    // controlMode == AUTO or sceneMode = FACE_PRIORITY
    // Process individual 3A controls

    res = doFakeAE(settings);
    if (res != 0) {
        HAL_LOGE("doFakeAE failed: %d", res);
        return res;
    }

    res = doFakeAF(settings);
    if (res != 0) {
        HAL_LOGE("doFakeAF failed: %d", res);
        return res;
    }

    res = doFakeAWB(settings);
    if (res != 0) {
        HAL_LOGE("doFakeAWB failed: %d", res);
        return res;
    }

    update3A(settings);

    return 0;
}

int32_t V4L2Wrapper::doFakeAF(android::hardware::camera::common::V1_0::helper::CameraMetadata &settings) {
    camera_metadata_entry e;

    e = settings.find(ANDROID_CONTROL_AF_MODE);
    if (e.count == 0) {  // && hasCapability(BACKWARD_COMPATIBLE)
        HAL_LOGE("%s: No AF mode entry!", __FUNCTION__);
        return -1;
    }
    uint8_t afMode = (e.count > 0) ? e.data.u8[0] : (uint8_t)ANDROID_CONTROL_AF_MODE_OFF;

    e = settings.find(ANDROID_CONTROL_AF_TRIGGER);
    typedef camera_metadata_enum_android_control_af_trigger af_trigger_t;
    af_trigger_t afTrigger;
    if (e.count != 0) {
        afTrigger = static_cast<af_trigger_t>(e.data.u8[0]);

        HAL_LOGV("%s: AF trigger set to 0x%x", __FUNCTION__, afTrigger);
        HAL_LOGV("%s: AF mode is 0x%x", __FUNCTION__, afMode);
    } else {
        afTrigger = ANDROID_CONTROL_AF_TRIGGER_IDLE;
    }

    switch (afMode) {
        case ANDROID_CONTROL_AF_MODE_OFF:
            mAfState = ANDROID_CONTROL_AF_STATE_INACTIVE;
            return 0;
        case ANDROID_CONTROL_AF_MODE_AUTO:
        case ANDROID_CONTROL_AF_MODE_MACRO:
        case ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO:
        case ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE:
#if 0
            if (!mFacingBack) {
                // Always report INACTIVE for front Emulated Camera
                mAfState = ANDROID_CONTROL_AF_STATE_INACTIVE;
                return 0;
            }
#endif
            break;
        default:
            HAL_LOGE("%s: doesn't support AF mode %d",
                    __FUNCTION__, afMode);
            return -1;
    }

    bool afModeChanged = mAfMode != afMode;
    mAfMode = afMode;

    /**
      * Simulate AF triggers. Transition at most 1 state per frame.
      * - Focusing always succeeds (goes into locked, or PASSIVE_SCAN).
      */

    bool afTriggerStart = false;
    bool afTriggerCancel = false;
    switch (afTrigger) {
        case ANDROID_CONTROL_AF_TRIGGER_IDLE:
            break;
        case ANDROID_CONTROL_AF_TRIGGER_START:
            afTriggerStart = true;
            break;
        case ANDROID_CONTROL_AF_TRIGGER_CANCEL:
            afTriggerCancel = true;
            // Cancel trigger always transitions into INACTIVE
            mAfState = ANDROID_CONTROL_AF_STATE_INACTIVE;

            HAL_LOGV("%s: AF State transition to STATE_INACTIVE", __FUNCTION__);

            // Stay in 'inactive' until at least next frame
            return 0;
        default:
            HAL_LOGE("%s: Unknown af trigger value %d", __FUNCTION__, afTrigger);
            return -1;
    }

    // If we get down here, we're either in an autofocus mode
    //  or in a continuous focus mode (and no other modes)

    int oldAfState = mAfState;
    switch (mAfState) {
        case ANDROID_CONTROL_AF_STATE_INACTIVE:
            if (afTriggerStart) {
                switch (afMode) {
                    case ANDROID_CONTROL_AF_MODE_AUTO:
                        // fall-through
                    case ANDROID_CONTROL_AF_MODE_MACRO:
                        mAfState = ANDROID_CONTROL_AF_STATE_ACTIVE_SCAN;
                        break;
                    case ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO:
                        // fall-through
                    case ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE:
                        mAfState = ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
                        break;
                }
            } else {
                // At least one frame stays in INACTIVE
                if (!afModeChanged) {
                    switch (afMode) {
                        case ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO:
                            // fall-through
                        case ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE:
                            mAfState = ANDROID_CONTROL_AF_STATE_PASSIVE_SCAN;
                            break;
                    }
                }
            }
            break;
        case ANDROID_CONTROL_AF_STATE_PASSIVE_SCAN:
            /**
              * When the AF trigger is activated, the algorithm should finish
              * its PASSIVE_SCAN if active, and then transition into AF_FOCUSED
              * or AF_NOT_FOCUSED as appropriate
              */
            if (afTriggerStart) {
                // Randomly transition to focused or not focused
                // if (rand() % 3) {
                mAfState = ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED;
                // } else {
                //    mAfState = ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
                // }
            } else if (!afTriggerCancel) {
                /**
                  * When the AF trigger is not involved, the AF algorithm should
                  * start in INACTIVE state, and then transition into PASSIVE_SCAN
                  * and PASSIVE_FOCUSED states
                  */

                // Randomly transition to passive focus
                // if (rand() % 3 == 0) {
                mAfState = ANDROID_CONTROL_AF_STATE_PASSIVE_FOCUSED;
                // }
            }

            break;
        case ANDROID_CONTROL_AF_STATE_PASSIVE_FOCUSED:
            if (afTriggerStart) {
                // Randomly transition to focused or not focused
                // if (rand() % 3) {
                mAfState = ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED;
                // } else {
                //    mAfState = ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
                // }
            }
            // TODO: initiate passive scan (PASSIVE_SCAN)
            break;
        case ANDROID_CONTROL_AF_STATE_ACTIVE_SCAN:
            // Simulate AF sweep completing instantaneously

            // Randomly transition to focused or not focused
            // if (rand() % 3) {
            mAfState = ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED;
            // } else {
            //    mAfState = ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
            // }
            break;
        case ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED:
            if (afTriggerStart) {
                switch (afMode) {
                    case ANDROID_CONTROL_AF_MODE_AUTO:
                        // fall-through
                    case ANDROID_CONTROL_AF_MODE_MACRO:
                        mAfState = ANDROID_CONTROL_AF_STATE_ACTIVE_SCAN;
                        break;
                    case ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO:
                        // fall-through
                    case ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE:
                        // continuous autofocus => trigger start has no effect
                        break;
                }
            }
            break;
        case ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED:
            if (afTriggerStart) {
                switch (afMode) {
                    case ANDROID_CONTROL_AF_MODE_AUTO:
                        // fall-through
                    case ANDROID_CONTROL_AF_MODE_MACRO:
                        mAfState = ANDROID_CONTROL_AF_STATE_ACTIVE_SCAN;
                        break;
                    case ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO:
                        // fall-through
                    case ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE:
                        // continuous autofocus => trigger start has no effect
                        break;
                }
            }
            break;
        default:
            HAL_LOGE("%s: Bad af state %d", __FUNCTION__, mAfState);
    }

    {
        char afStateString[100] = {0, };
        camera_metadata_enum_snprint(ANDROID_CONTROL_AF_STATE,
                oldAfState,
                afStateString,
                sizeof(afStateString));

        char afNewStateString[100] = {0, };
        camera_metadata_enum_snprint(ANDROID_CONTROL_AF_STATE,
                mAfState,
                afNewStateString,
                sizeof(afNewStateString));
        HAL_LOGV("%s: AF state transitioned from %s to %s",
                __FUNCTION__, afStateString, afNewStateString);
    }

    return 0;
}

int32_t V4L2Wrapper::doFakeAE(android::hardware::camera::common::V1_0::helper::CameraMetadata &settings) {
    camera_metadata_entry e;

    e = settings.find(ANDROID_CONTROL_AE_MODE);
    if (e.count == 0) {  //  && hasCapability(BACKWARD_COMPATIBLE)
        HAL_LOGE("%s: No AE mode entry!", __FUNCTION__);
        return -1;
    }
    uint8_t aeMode = (e.count > 0) ? e.data.u8[0] : (uint8_t)ANDROID_CONTROL_AE_MODE_ON;
    mAeMode = aeMode;

    switch (aeMode) {
        case ANDROID_CONTROL_AE_MODE_OFF:
            // AE is OFF
            mAeState = ANDROID_CONTROL_AE_STATE_INACTIVE;
            return 0;
        case ANDROID_CONTROL_AE_MODE_ON:
            // OK for AUTO modes
            break;
        default:
            // Mostly silently ignore unsupported modes
            HAL_LOGE("%s: doesn't support AE mode %d, assuming ON",
                    __FUNCTION__, aeMode);
            break;
    }

    e = settings.find(ANDROID_CONTROL_AE_LOCK);
    bool aeLocked = (e.count > 0) ? (e.data.u8[0] == ANDROID_CONTROL_AE_LOCK_ON) : false;

    e = settings.find(ANDROID_CONTROL_AE_PRECAPTURE_TRIGGER);
    bool precaptureTrigger = false;
    if (e.count != 0) {
        precaptureTrigger =
            (e.data.u8[0] == ANDROID_CONTROL_AE_PRECAPTURE_TRIGGER_START);
    }

    if (precaptureTrigger) {
        HAL_LOGV("%s: Pre capture trigger = %d", __FUNCTION__, precaptureTrigger);
    } else if (e.count > 0) {
        HAL_LOGV("%s: Pre capture trigger was present? %zu",
                __FUNCTION__, e.count);
    }

    if (precaptureTrigger || mAeState == ANDROID_CONTROL_AE_STATE_PRECAPTURE) {
        // Run precapture sequence
        if (mAeState != ANDROID_CONTROL_AE_STATE_PRECAPTURE) {
            mAeCounter = 0;
        }

        if (mFacePriority) {
            mAeTargetExposureTime = kFacePriorityExposureTime;
        } else {
            mAeTargetExposureTime = kNormalExposureTime;
        }

        if (mAeCounter > kPrecaptureMinFrames &&
                (mAeTargetExposureTime - mAeCurrentExposureTime) <
                mAeTargetExposureTime / 10) {
            // Done with precapture
            mAeCounter = 0;
            mAeState = aeLocked ? ANDROID_CONTROL_AE_STATE_LOCKED :
                ANDROID_CONTROL_AE_STATE_CONVERGED;
        } else {
            // Converge some more
            mAeCurrentExposureTime +=
                (mAeTargetExposureTime - mAeCurrentExposureTime) *
                kExposureTrackRate;
            mAeCounter++;
            mAeState = ANDROID_CONTROL_AE_STATE_PRECAPTURE;
        }

    } else if (!aeLocked) {
        // Run standard occasional AE scan
        switch (mAeState) {
            case ANDROID_CONTROL_AE_STATE_INACTIVE:
                mAeState = ANDROID_CONTROL_AE_STATE_SEARCHING;
                break;
            case ANDROID_CONTROL_AE_STATE_CONVERGED:
                mAeCounter++;
                if (mAeCounter > kStableAeMaxFrames) {
                    mAeTargetExposureTime =
                        mFacePriority ? kFacePriorityExposureTime :
                        kNormalExposureTime;
                    float exposureStep = (static_cast<double>(rand_r(&mExposureSeed)) / RAND_MAX) *
                        (kExposureWanderMax - kExposureWanderMin) +
                        kExposureWanderMin;
                    mAeTargetExposureTime *= std::pow(2, exposureStep);
                    mAeState = ANDROID_CONTROL_AE_STATE_SEARCHING;
                }
                break;
            case ANDROID_CONTROL_AE_STATE_SEARCHING:
                mAeCurrentExposureTime +=
                    (mAeTargetExposureTime - mAeCurrentExposureTime) *
                    kExposureTrackRate;
                if (abs(mAeTargetExposureTime - mAeCurrentExposureTime) <
                        mAeTargetExposureTime / 10) {
                    // Close enough
                    mAeState = ANDROID_CONTROL_AE_STATE_CONVERGED;
                    mAeCounter = 0;
                }
                break;
            case ANDROID_CONTROL_AE_STATE_LOCKED:
                mAeState = ANDROID_CONTROL_AE_STATE_CONVERGED;
                mAeCounter = 0;
                break;
            default:
                HAL_LOGE("%s: Emulator in unexpected AE state %d",
                        __FUNCTION__, mAeState);
                return -1;
        }
    } else {
        // AE is locked
        mAeState = ANDROID_CONTROL_AE_STATE_LOCKED;
    }

    return 0;
}

int32_t V4L2Wrapper::doFakeAWB(android::hardware::camera::common::V1_0::helper::CameraMetadata &settings) {
    camera_metadata_entry e;

    e = settings.find(ANDROID_CONTROL_AWB_MODE);
    if (e.count == 0) {  // && hasCapability(BACKWARD_COMPATIBLE
        HAL_LOGE("%s: No AWB mode entry!", __FUNCTION__);
        return -1;
    }
    uint8_t awbMode = (e.count > 0) ? e.data.u8[0] : (uint8_t)ANDROID_CONTROL_AWB_MODE_AUTO;

    // TODO: Add white balance simulation
    e = settings.find(ANDROID_CONTROL_AWB_LOCK);
    bool awbLocked = (e.count > 0) ? (e.data.u8[0] == ANDROID_CONTROL_AWB_LOCK_ON) : false;

    switch (awbMode) {
        case ANDROID_CONTROL_AWB_MODE_OFF:
            mAwbState = ANDROID_CONTROL_AWB_STATE_INACTIVE;
            break;
        case ANDROID_CONTROL_AWB_MODE_AUTO:
        case ANDROID_CONTROL_AWB_MODE_INCANDESCENT:
        case ANDROID_CONTROL_AWB_MODE_FLUORESCENT:
        case ANDROID_CONTROL_AWB_MODE_DAYLIGHT:
        case ANDROID_CONTROL_AWB_MODE_SHADE:
            // Always magically right, or locked
            mAwbState = awbLocked ? ANDROID_CONTROL_AWB_STATE_LOCKED :
                ANDROID_CONTROL_AWB_STATE_CONVERGED;
            break;
        default:
            HAL_LOGE("%s: doesn't support AWB mode %d",
                    __FUNCTION__, awbMode);
            return -1;
    }

    return 0;
}

void V4L2Wrapper::update3A(android::hardware::camera::common::V1_0::helper::CameraMetadata &settings) {
    if (mAeMode != ANDROID_CONTROL_AE_MODE_OFF) {
        settings.update(ANDROID_SENSOR_EXPOSURE_TIME,
                &mAeCurrentExposureTime, 1);
        settings.update(ANDROID_SENSOR_SENSITIVITY,
                &mAeCurrentSensitivity, 1);
        settings.update(ANDROID_SENSOR_ROLLING_SHUTTER_SKEW,
                &mAeMinExposureTime, 1);
    }

    settings.update(ANDROID_CONTROL_AE_STATE,
            &mAeState, 1);
    settings.update(ANDROID_CONTROL_AF_STATE,
            &mAfState, 1);
    settings.update(ANDROID_CONTROL_AWB_STATE,
            &mAwbState, 1);

    uint8_t lensState;
    switch (mAfState) {
        case ANDROID_CONTROL_AF_STATE_PASSIVE_SCAN:
        case ANDROID_CONTROL_AF_STATE_ACTIVE_SCAN:
            lensState = ANDROID_LENS_STATE_MOVING;
            break;
        case ANDROID_CONTROL_AF_STATE_INACTIVE:
        case ANDROID_CONTROL_AF_STATE_PASSIVE_FOCUSED:
        case ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED:
        case ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED:
        case ANDROID_CONTROL_AF_STATE_PASSIVE_UNFOCUSED:
        default:
            lensState = ANDROID_LENS_STATE_STATIONARY;
            break;
    }

    settings.update(ANDROID_LENS_STATE, &lensState, 1);
    update3ARegion(ANDROID_CONTROL_AE_REGIONS, settings);
    update3ARegion(ANDROID_CONTROL_AF_REGIONS, settings);
    update3ARegion(ANDROID_CONTROL_AWB_REGIONS, settings);
}

void V4L2Wrapper::update3ARegion(uint32_t tag, android::hardware::camera::common::V1_0::helper::CameraMetadata &settings) {
    if (tag != ANDROID_CONTROL_AE_REGIONS &&
            tag != ANDROID_CONTROL_AF_REGIONS &&
            tag != ANDROID_CONTROL_AWB_REGIONS) {
        return;
    }

    camera_metadata_entry_t entry;
    entry = settings.find(ANDROID_SCALER_CROP_REGION);
    if (entry.count > 0) {
        int32_t cropRegion[4];
        cropRegion[0] =  entry.data.i32[0];
        cropRegion[1] =  entry.data.i32[1];
        cropRegion[2] =  entry.data.i32[2] + cropRegion[0];
        cropRegion[3] =  entry.data.i32[3] + cropRegion[1];
        entry = settings.find(tag);
        if (entry.count > 0) {
            int32_t* ARegion = entry.data.i32;
            // calculate the intersection of AE/AF/AWB and CROP regions
            if (ARegion[0] < cropRegion[2] && cropRegion[0] < ARegion[2] &&
                    ARegion[1] < cropRegion[3] && cropRegion[1] < ARegion[3]) {
                int32_t interSect[5];
                interSect[0] = std::max(ARegion[0], cropRegion[0]);
                interSect[1] = std::max(ARegion[1], cropRegion[1]);
                interSect[2] = std::min(ARegion[2], cropRegion[2]);
                interSect[3] = std::min(ARegion[3], cropRegion[3]);
                interSect[4] = ARegion[4];
                settings.update(tag, &interSect[0], 5);
            }
        }
    }
}

void V4L2Wrapper::PushRequest(
        std::shared_ptr<default_camera_hal::CaptureRequest> request) {
    HAL_LOG_ENTER();
    std::lock_guard<std::mutex> guard(mRequestQueueLock);
    mRequestQueue.push(request);
    mRequestQueueAvailable.notify_one();
    HAL_LOGV("----X size: %zu, frame num:%d",
            mRequestQueue.size(), request->frame_number);
}

std::shared_ptr<default_camera_hal::CaptureRequest>
V4L2Wrapper::PopRequest() {
    std::unique_lock<std::mutex> lock(mRequestQueueLock);
    while (mRequestQueue.empty() || streamstatus != STREAM_STATUS_STREAM_ON) {
        mRequestQueueAvailable.wait(lock);
    }

    std::shared_ptr<default_camera_hal::CaptureRequest> request =
        mRequestQueue.front();
    mRequestQueue.pop();
    HAL_LOGV("----X left size: %zu, frame num:%d",
            mRequestQueue.size(), request->frame_number);
    return request;
}

int V4L2Wrapper::GetInFlightBufferCount() {
    std::lock_guard<std::mutex> guard(mRequestQueueLock);
    return mRequestQueue.size();
}

void V4L2Wrapper::flush() {
    ReleaseOutputSurfaces();
    std::lock_guard<std::mutex> guard(mRequestQueueLock);
    while(!mRequestQueue.empty()) {
        mRequestQueue.pop();
    }
}

void V4L2Wrapper::printBufferInfo(buffer_info_t *buff_info) {
    HAL_LOGE("hal format = %s", printHalFormat(buff_info->halFormat));
    HAL_LOGE("Width = %u", buff_info->width);
    HAL_LOGE("Height = %u", buff_info->height);
    HAL_LOGE("Length = %u", buff_info->length);
    HAL_LOGE("corpWidth = %u", buff_info->cropWidth);
    HAL_LOGE("corpHeight = %u", buff_info->cropHeight);
    HAL_LOGE("corpLeft = %u", buff_info->cropLeft);
    HAL_LOGE("corpTop = %u", buff_info->cropTop);
    HAL_LOGE("fd = %d", buff_info->fd);
    HAL_LOGE("offset = %d", buff_info->offset);
    HAL_LOGE("virtPtr = %p", buff_info->virtPtr);
    HAL_LOGE("gpuAddr = %p", buff_info->gpuAddr);
    HAL_LOGE("surfaceId = %d", buff_info->surfaceId);
    HAL_LOGE("isUsed = %d", (int)buff_info->isUsed);
    HAL_LOGE("frame_number = %u", buff_info->frame_number);
    HAL_LOGE("id = %lu", buff_info->id);
}

}  // namespace v4l2_camera_hal
