/*
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

#ifndef V4L2_CAMERA_HAL_V4L2_WRAPPER_H_
#define V4L2_CAMERA_HAL_V4L2_WRAPPER_H_

#include <array>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>
#include <map>
#include <queue>

#include "common.h"

#include <android-base/unique_fd.h>
#include <ui/GraphicBuffer.h>
#include <utils/RefBase.h>
#include <utils/Thread.h>

#include "arc/common_types.h"
#include "arc/frame_buffer.h"
#include "capture_request.h"
#include "stream_format.h"

#if !defined(ANDROID_Q_AOSP)
#include <android/hardware/graphics/mapper/4.0/IMapper.h>
using Error_v4       = ::android::hardware::graphics::mapper::V4_0::Error;
using aidl::android::hardware::graphics::common::PlaneLayout;
using IMapper_v4     = ::android::hardware::graphics::mapper::V4_0::IMapper;
#endif

namespace v4l2_camera_hal {

using android::sp;
using android::GraphicBuffer;

class V4L2Camera;
// max kernel buffer num and max one stream capture request num
#define MAX_INPUT_BUFFER_NUM 10
#define MAX_OUTPUT_BUFFER_NUM 32  // > 2 * MAX_BUFFER_NUM

#define MAX_BUFFER_NUM 5

typedef int64_t nsecs_t;

typedef struct {
  uint32_t halFormat;
  uint32_t width;
  uint32_t height;
  uint32_t cropWidth;
  uint32_t cropHeight;
  uint32_t cropLeft;
  uint32_t cropTop;
  int32_t fd;
  int32_t offset;
  int32_t length;
  void * virtPtr;
  void * gpuAddr;
  uint32_t surfaceId;
  bool isUsed;
  uint32_t frame_number;
  uint64_t id;
  std::mutex lock;
} buffer_info_t;

class V4L2Wrapper {
 public:
  // Use this method to create V4L2Wrapper objects. Functionally equivalent
  // to "new V4L2Wrapper", except that it may return nullptr in case of failure.
  static V4L2Wrapper* NewV4L2Wrapper(const std::string device_path);
  virtual ~V4L2Wrapper();

  void SetCameraHal(V4L2Camera * pCamera) { mCamera = pCamera;}

  // Helper class to ensure all opened connections are closed.
  class Connection {
   public:
    Connection(std::shared_ptr<V4L2Wrapper> device)
        : device_(std::move(device)), connect_result_(device_->Connect()) {}
    ~Connection() {
      if (connect_result_ == 0) {
        device_->Disconnect();
      }
    }
    // Check whether the connection succeeded or not.
    inline int status() const { return connect_result_; }

   private:
    std::shared_ptr<V4L2Wrapper> device_;
    const int connect_result_;
  };

  // Turn the stream on or off.
  virtual int StreamOn();
  virtual int StreamOff();
  // Manage controls.
  virtual int QueryControl(uint32_t control_id, v4l2_query_ext_ctrl* result);
  virtual int GetControl(uint32_t control_id, int32_t* value);
  virtual int SetControl(uint32_t control_id,
                         int32_t desired,
                         int32_t* result = nullptr);
  // Manage format.
  virtual int GetFormats(std::set<uint32_t>* v4l2_formats);
  virtual int GetQualifiedFormats(std::vector<uint32_t>* v4l2_formats);
  virtual int GetFormatFrameSizes(uint32_t v4l2_format,
                                  std::set<std::array<int32_t, 2>>* sizes);

  // Durations are returned in ns.
  virtual int GetFormatFrameDurationRange(
      uint32_t v4l2_format,
      const std::array<int32_t, 2>& size,
      std::array<int64_t, 2>* duration_range);
  int Init();
  // Manage buffers.
  virtual int EnqueueRequest(
      std::shared_ptr<default_camera_hal::CaptureRequest> request);
  virtual bool DequeueRequest();
  virtual int GetInFlightBufferCount();

  //NV21 to RGBA conversion for video recording.
  void nv21_rgb24_std(uint32_t width, uint32_t height,
          const uint8_t *Y, const uint8_t *UV, uint32_t Y_stride, uint32_t UV_stride,
          uint8_t *RGB, uint32_t RGB_stride);
#ifdef SOFT_SCALE
  int Nv21Scale(unsigned char* uvInputPixels, unsigned char* yInputPointer, void * pOutBuf, unsigned char* uvOutputPixels, int OUTWIDTH, int OUTHEIGHT);
#endif

  void flush();
  int GetSensorWidth();
  int GetSensorHeight();
 private:
  // Constructor is private to allow failing on bad input.
  // Use NewV4L2Wrapper instead.
  V4L2Wrapper(const std::string device_path);

  // Connect or disconnect to the device. Access by creating/destroying
  // a V4L2Wrapper::Connection object.
  int Connect();
  void Disconnect();
  // Perform an ioctl call in a thread-safe fashion.
  template <typename T>
  int IoctlLocked(int request, T data);
  int V4l2Read(unsigned char *ptr, int size);
  // Request/release userspace buffer mode via VIDIOC_REQBUFS.
  int RequestBuffers(uint32_t num_buffers);

  inline bool connected() { return device_fd_.get() >= 0; }
  void JpegEncode(void * pOutBuf, unsigned char* pInputPixels,
                  int imageWidth, int imageHeight, unsigned long *pJpegLen);

  int ProcessSnapshotData(const camera3_stream_buffer_t* stream_buffer,
                                      android::hardware::camera::common::V1_0::helper::CameraMetadata& settings,
                                      buffer_info_t * dst_buff_info);

  int32_t process3A(android::hardware::camera::common::V1_0::helper::CameraMetadata &settings);
  int32_t doFakeAF(android::hardware::camera::common::V1_0::helper::CameraMetadata &settings);
  int32_t doFakeAE(android::hardware::camera::common::V1_0::helper::CameraMetadata &settings);
  int32_t doFakeAWB(android::hardware::camera::common::V1_0::helper::CameraMetadata &settings);
  void update3A(android::hardware::camera::common::V1_0::helper::CameraMetadata &settings);
  void update3ARegion(uint32_t tag, android::hardware::camera::common::V1_0::helper::CameraMetadata &settings);

  uint32_t GetC2dFormat(uint32_t halFormat, bool isSource);
  bool IsYuvformat(uint32_t halFormat);
  uint32_t CalcStride0(uint32_t halFormat, size_t width);
  uint32_t CalcStride1(uint32_t halFormat, size_t width);
  size_t CalcSize(uint32_t halFormat, size_t width, size_t height);
  size_t CalcPlane0Size(uint32_t halFormat, size_t width, size_t height);
  size_t CalcPlane1Size(uint32_t halFormat, size_t width, size_t height);
  size_t CalcPlane2Size(uint32_t halFormat, size_t width, size_t height);
  void * GetMappedGPUAddr(int bufFD, void *bufPtr, size_t bufLen);
  int32_t CreateC2dSurface(buffer_info_t * buff_info, bool isSource);
  int32_t CreateYuvC2dSurface(buffer_info_t * buff_info, bool isSource);
  int32_t CreateRgbC2dSurface(buffer_info_t * buff_info, bool isSource);
  int32_t ReleaseC2dSurface(buffer_info_t * buff_info);
  int32_t DoC2dScale(buffer_info_t * src_buff_info, buffer_info_t * dst_buff_info);

  int32_t CreateInputSurfaces(int32_t num);
  int32_t CreateInputSurface(int32_t index);
  void ReleaseInputSurfaces();
  int ReleaseBuffers();

  int32_t CreateOutputSurfaces(
                std::shared_ptr<default_camera_hal::CaptureRequest> request);
  int32_t CreateOutputSurface(camera3_stream_buffer_t* output_stream_buffer,
                                            buffer_info_t * buff_info);
  void ReleaseOutputSurfaces();
  buffer_info_t * GetOutputBufferInfo(uint64_t id);
  int32_t AddOutputBufferInfo(buffer_info_t * buff_info);
  int32_t DeleteOutputBufferInfo(buffer_info_t * buff_info);

  int32_t DumpFrameBuffer(char *buffer, int32_t size, bool raw);

  void PushRequest(std::shared_ptr<default_camera_hal::CaptureRequest> request);
  std::shared_ptr<default_camera_hal::CaptureRequest> PopRequest();
  int GetInputInformation();
  uint32_t ConvertToHalFormat(uint32_t kernelFormat);
  void printBufferInfo(buffer_info_t * buff_info);
  int get_native_hnd_buffer_fd(buffer_handle_t hndl);
  int get_native_hnd_buffer_offset(buffer_handle_t hndl);
  uint64_t get_native_hnd_buffer_size(buffer_handle_t hndl);
  uint64_t get_native_hnd_buffer_id(buffer_handle_t hndl);

  bool first_frame;
  // Format management.
  const arc::SupportedFormats GetSupportedFormats();

  // The camera device path. For example, /dev/video0.
  const std::string device_path_;
  // The opened device fd.
  android::base::unique_fd device_fd_;
  // The underlying gralloc module.
  // std::unique_ptr<V4L2Gralloc> gralloc_;
  // Whether or not the device supports the extended control query.
  bool extended_query_supported_;
  // The format this device is set up for.
  std::unique_ptr<StreamFormat> format_;
  // Lock protecting use of the buffer tracker.
  std::mutex buffer_queue_lock_;
  // Lock protecting use of the camera switching tracker.
  std::mutex flush_v4l2_lock_;
  // Lock protecting use of the device.
  std::mutex device_lock_;
  // Lock protecting connecting/disconnecting the device.
  std::mutex connection_lock_;
  // Reference count connections.
  int connection_count_;
  // Supported formats.
  arc::SupportedFormats supported_formats_;
  // Qualified formats.
  arc::SupportedFormats qualified_formats_;

  // and the camera after switching will not be able to print normally.
  // Setting this variable ensures that StreamOff will only be executed once when switching cameras.
  volatile int streamstatus;
  std::mutex streamStatusLock;
  std::condition_variable streamStatusSignal;

  /** Fake 3A constants */
  static const nsecs_t kNormalExposureTime;
  static const nsecs_t kFacePriorityExposureTime;
  static const int     kNormalSensitivity;
  static const int     kFacePrioritySensitivity;
  // Rate of converging AE to new target value, as fraction of difference between
  // current and target value.
  static const float   kExposureTrackRate;
  // Minimum duration for precapture state. May be longer if slow to converge
  // to target exposure
  static const int     kPrecaptureMinFrames;
  // How often to restart AE 'scanning'
  static const int     kStableAeMaxFrames;
  // Maximum stop below 'normal' exposure time that we'll wander to while
  // pretending to converge AE. In powers of 2. (-2 == 1/4 as bright)
  static const float   kExposureWanderMin;
  // Maximum stop above 'normal' exposure time that we'll wander to while
  // pretending to converge AE. In powers of 2. (2 == 4x as bright)
  static const float   kExposureWanderMax;

  /** Fake 3A state */
  uint8_t mControlMode;
  bool    mFacePriority;

  uint8_t mAfState;
  uint8_t mAfMode;

  uint8_t mAeState;
  uint8_t mAeMode;
  int mAeCounter;
  nsecs_t mAeCurrentExposureTime;
  nsecs_t mAeTargetExposureTime;
  int     mAeCurrentSensitivity;
  uint32_t mExposureSeed;

  uint8_t mAwbMode;
  uint8_t mAwbState;

  std::mutex output_buffer_map_lock_;
  std::map<uint64_t, buffer_info_t *> outputBufferMap;  // private_handle_t.id ==> buffer_info_t

  buffer_info_t input_buffer_infos[MAX_INPUT_BUFFER_NUM];  // for kernel input
  buffer_info_t output_buffer_infos[MAX_OUTPUT_BUFFER_NUM];  // for preview and video output
  buffer_info_t capture_buffer_info;  // for snapshot

  std::mutex mRequestQueueLock;
  std::queue<std::shared_ptr<default_camera_hal::CaptureRequest>>
      mRequestQueue;
  std::condition_variable mRequestQueueAvailable;
  sp<GraphicBuffer> snapshot_output_buffer;

  bool mFlush;
  uint32_t mFrameNum;
  sp<IMapper_v4> mapper;

  uint32_t mType;

  V4L2Camera *mCamera;
  android::sp<android::Thread> mBufferDequeueThread;

  int32_t mInputWidth;
  int32_t mInputHeight;
  int32_t mInputCropWidth;
  int32_t mInputCropHeight;
  int32_t mInputCropLeft;
  int32_t mInputCropTop;
  uint32_t mInputHalFormat;

  friend class Connection;
  friend class V4L2WrapperMock;

  DISALLOW_COPY_AND_ASSIGN(V4L2Wrapper);
};

}  // namespace v4l2_camera_hal

#endif  // V4L2_CAMERA_HAL_V4L2_WRAPPER_H_
