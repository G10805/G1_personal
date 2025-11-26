/*
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
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

// Modified from hardware/libhardware/modules/camera/CameraHAL.cpp

#define MODULE_NAME "V4L2CameraHAL"
//#define LOG_NDEBUG 0
#include "v4l2_camera_hal.h"

#include <dirent.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <unordered_set>

#include <android-base/parseint.h>

#include "common.h"
#include "v4l2_camera.h"
#include "config_port_metadata.h"

namespace v4l2_camera_hal {

// Default global camera hal.
static V4L2CameraHAL gCameraHAL;

// Private functions
int V4L2CameraHAL::v4l2QcarcamInit(void)
{
    int ret = 0;
    qcarcam_init_t qcarcam_init = {};
    qcarcam_init.version = QCARCAM_VERSION;
    qcarcam_init.debug_tag = (char *)"ais_v4l2_hal_client";

    std::unique_lock<std::mutex> lock(mInitStatusLock);
    if (!mQcarcamInitialized) {
        // Initialize qcarcam
        ret = qcarcam_initialize(&qcarcam_init);
        if (ret)
            mQcarcamInitialized = false;
        else
            mQcarcamInitialized = true;
        ais_log_init(NULL, (char *)qcarcam_init.debug_tag);
    }
    // return success if its already initialized
    lock.unlock();

    return ret;
}

void V4L2CameraHAL::v4l2QcarcamDeInit(void)
{
    std::unique_lock<std::mutex> lock(mInitStatusLock);
    if (mQcarcamInitialized) {
        qcarcam_uninitialize();
        mQcarcamInitialized = false;
    }
    lock.unlock();
}

int V4L2CameraHAL::v4l2QcarcamQueryInputs(void)
{
    config_port_metadata config;
    QCarCamInfo qcarcamInfo = {};
    if(!config.cameraConfigFound)
      {
        if (!config.initialize(CAMERA_CONFIG_JSON_FILE)) {
            HAL_LOGE("Missing or improper configuration for the V4L2 HAL application.  Exiting.");
            config.cameraConfigFound = false;
        }
          else
            config.cameraConfigFound = true;
      }
    unsigned captureCount = 0;
    /*query qcarcam*/
    char camera_name[20];
    unsigned int queryNumInputs = 0;
    qcarcam_input_t *m_pAisInputList = NULL;
    int i, ret = 0;

    ret = qcarcam_query_inputs(NULL, 0, &queryNumInputs);
    if (QCARCAM_RET_OK != ret || queryNumInputs == 0)
    {
        HAL_LOGE("Failed qcarcam_query_inputs number of inputs= %d  ret= %d", queryNumInputs, ret);
    }
    else
    {
        m_pAisInputList = (qcarcam_input_t *)calloc(queryNumInputs, sizeof(*m_pAisInputList));
        if (m_pAisInputList)
        {
            ret = qcarcam_query_inputs(m_pAisInputList, queryNumInputs, &mNumAisInputs);
            if (QCARCAM_RET_OK != ret || mNumAisInputs != queryNumInputs)
            {
                HAL_LOGE("Failed qcarcam_query_inputs with ret = %d", ret);
            }
            else
            {
                HAL_LOGE("--- QCarCam Queried Inputs:----%d",mNumAisInputs);
                for (i = 0; i < (int)mNumAisInputs; i++)
                {
                    HAL_LOGD("%d, input_id = %d, width = %d, height = %d, colorFmt =%d", i, m_pAisInputList[i].desc,
                    m_pAisInputList[i].res[0].width, m_pAisInputList[i].res[0].height, m_pAisInputList[i].color_fmt[0]);
                    config.device_idx = i;
                    config.port_idx = m_pAisInputList[i].desc;
                    qcarcamInfo.width  = m_pAisInputList[i].res[0].width;
                    qcarcamInfo.height = m_pAisInputList[i].res[0].height;
                    qcarcamInfo.color_fmt = m_pAisInputList[i].color_fmt[0];
                    //Currently we support only standard cameras
                    if (m_pAisInputList[i].desc > QCARCAM_INPUT_TYPE_TUNER)
                        continue;
                    std::unique_ptr<V4L2Camera> cam(V4L2Camera::NewV4L2Camera(config, qcarcamInfo));
                    if (cam) {
                        mCameras.push_back(std::move(cam));
                    } else {
                        HAL_LOGE("Failed to initialize camera at %s.", m_pAisInputList[i].desc);
                    }
                    HAL_LOGE("CameraID %d PortID %d create over", config.device_idx,config.port_idx);
                    captureCount++;
                }
            }
        }
    }
    HAL_LOGE("Found %d qualified video capture devices found",captureCount);
    if (m_pAisInputList)
        free(m_pAisInputList);
    return ret;
}

void V4L2CameraHAL::enumerateCameras(int lpm) {

    int ret = 0;
    ret = v4l2QcarcamInit();
    if (ret == QCARCAM_RET_OK)
    {
        HAL_LOGD( "qcarcam_initialize successfully");
        ret = v4l2QcarcamQueryInputs();
        if (ret != QCARCAM_RET_OK)
            HAL_LOGE("v4l2QcarcamQueryInputs failed ret = %d", ret);
    }
    else
        HAL_LOGE("qcarcam_initialize failed %d ", ret);
    // Deinit for LPM
    if (lpm)
        v4l2QcarcamDeInit();
}

V4L2CameraHAL::V4L2CameraHAL() : mCameras(), mCallbacks(NULL) {

  HAL_LOG_ENTER();
  mQcarcamInitialized = false;
  enumerateCameras(1); //lpm==1
}

V4L2CameraHAL::~V4L2CameraHAL() {
  HAL_LOG_ENTER();
  v4l2QcarcamDeInit();
}

int V4L2CameraHAL::getNumberOfCameras() {


  HAL_LOGV("getNumberOfCameras returns %zu", mCameras.size());
  return mCameras.size();
}

int V4L2CameraHAL::getCameraInfo(int id, camera_info_t* info) {
    HAL_LOG_ENTER();
    if (id < 0 || id >= mCameras.size()) {
        HAL_LOGE("id = %d, mCameras.size() = %zu", id, mCameras.size());
        return -EINVAL;
    }
    // TODO(b/29185945): Hotplugging: return -EINVAL if unplugged.
    return mCameras[id]->getInfo(info);
}

int V4L2CameraHAL::setCallbacks(const camera_module_callbacks_t* callbacks) {
    HAL_LOG_ENTER();
    mCallbacks = callbacks;
    return 0;
}

void V4L2CameraHAL::getVendorTagOps(vendor_tag_ops_t* ops) {
    HAL_LOG_ENTER();
    // No vendor ops for this HAL. From <hardware/camera_common.h>:
    // "leave ops unchanged if no vendor tags are defined."
}

int V4L2CameraHAL::openLegacy(const hw_module_t* module,
        const char* id,
        uint32_t halVersion,
        hw_device_t** device) {
    HAL_LOG_ENTER();
    // Not supported.
    return -ENOSYS;
}

int V4L2CameraHAL::setTorchMode(const char* camera_id, bool enabled) {
    HAL_LOG_ENTER();
    // TODO(b/29158098): HAL is required to respond appropriately if
    // the desired camera actually does support flash.
    return -ENOSYS;
}

int V4L2CameraHAL::openDevice(const hw_module_t* module,
        const char* name,
        hw_device_t** device) {
    HAL_LOG_ENTER();
/*
  if (module != &HAL_MODULE_INFO_SYM.common) {
    HAL_LOGE(
        "Invalid module %p expected %p", module, &HAL_MODULE_INFO_SYM.common);
    return -EINVAL;
  }
*/
    int id;
    if (!android::base::ParseInt(name, &id, 0, getNumberOfCameras() - 1)) {
        HAL_LOGV("opendevice returns error");
        return -EINVAL;
    }
    // TODO(b/29185945): Hotplugging: return -EINVAL if unplugged.
    return mCameras[id]->openDevice(module, device);
}

#if defined(AIDL_ANDROID_API)
bool V4L2CameraHAL::IsStreamCombinationSupported(
    uint32_t frameworkId,
    const camera_stream_combination_t& rStreamCombo)
{
    bool result = true;
    //We don't intend to support the Logical Camera, resource cost etc. in Limited HAL
    HAL_LOGV("The resource manager is disabled, validation is not possible");
    return result;
}

bool V4L2CameraHAL::IsConcurrentStreamCombinationSupported(
    uint32_t numCombinations,
    const cameraid_stream_combination_t*  pConcurrentCombinations)
{
  bool result = true;
  //We don't intend to support the Logical Camera, resource cost etc. in Limited HAL
  HAL_LOGV("The resource manager is disabled, validation is not possible");
  return result;
}

int V4L2CameraHAL::getConcurrentStreamingCameraIds(
    uint32_t*                          pConcCamArrayLength,
    concurrent_camera_combination_t**  ppConcCamArray)
{
  int result                  = 0;
  const uint32_t MaxCameras       = 25;   // Variable is used to store front and back cameras list
  const uint32_t MaxStreams       = 2;    // Number of streams (Preview and YUV streams)
  const uint32_t MaxConcCameras   = 2;    // Currently only one front and back camera is supported as concurrent
  const uint32_t MinWidth         = 1280; // Minimum resolution needs to be supported is 1280x720
  const uint32_t MinHeight        = 720;
  uint32_t       numCameras       = 0;
  uint32_t   frontCamIndex    = 0;
  uint32_t   backCamIndex     = 0;
  uint32_t   concCamArraySize = 0;

  uint32_t   frontCam[MaxCameras];
  uint32_t   backCam[MaxCameras];

  camera_info_t                   camInfo;
  camera_stream_combination_t     streamComb;
  camera_stream_combination_t     frontCamstreamCombArr;
  camera_stream_combination_t     backCamstreamCombArr;
  cameraid_stream_combination_t*  pCamIdStreamComb       = NULL;
  camera_stream_t*                pCam1Streams           = NULL;
  camera_stream_t*                pCam2Streams           = NULL;

  /**
   * This API provides list of tuples of camera ids that support concurrent
   * stream concurrently. Each camera device in a tuple must be able to
   * stream frames with minimum 1280x720 resolution preview and YUV streams.
   */

  if ((NULL == pConcCamArrayLength) || (NULL == ppConcCamArray))
  {
      HAL_LOGE("Invalid arguments pConcCamArray %p, pConcCamArrayLength %p ",
                     ppConcCamArray, pConcCamArrayLength);
      result = EINVAL;
  }
  else if (MaxConcCameras > MaxConcurrentCameraSupported)
  {
      HAL_LOGE("array size in structure is insufficient to hold all the entries, %d %d ",
                     MaxConcCameras, MaxConcurrentCameraSupported);
      result = EINVAL;
  }
  else
  {
      *pConcCamArrayLength = 0;
      numCameras           = getNumberOfCameras();
      // Update this initial value to avoid reallocations
      concCamArraySize     = numCameras << 1;

      *ppConcCamArray         = new concurrent_camera_combination[concCamArraySize];
      pCam1Streams            = new camera_stream_t[MaxStreams];
      pCam2Streams            = new camera_stream_t[MaxStreams];
      // Currently only one front and back cameras are supported as concurrent Cameras
      pCamIdStreamComb        = new cameraid_stream_combination_t[MaxConcCameras];

      if ((NULL == pCam1Streams) || (NULL == pCam2Streams) ||
          (NULL == *ppConcCamArray) || (NULL == pCamIdStreamComb))
      {
          HAL_LOGE("Memory allocation failed, pCam1Streams %p, pCam2Streams %p, pConcCamArray %p ",
                         " pCamIdStreamComb %p",
                         pCam1Streams, pCam2Streams, *ppConcCamArray, pCamIdStreamComb);
          result     = ENOMEM;
          numCameras = 0;
      }
  }

  if (0 == result)
  {
      memset(&pCam1Streams[0], 0, sizeof(camera_stream_t));

      // Initialize streams with basic resolution
      pCam1Streams[0].width        = MinWidth;
      pCam1Streams[0].height       = MinHeight;
      pCam1Streams[0].stream_type  = 0; // output stream
      pCam1Streams[0].format       = HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED;
      pCam1Streams[0].usage        = 0x900;

      pCam1Streams[1]              = pCam1Streams[0];
      pCam1Streams[1].format       = HAL_PIXEL_FORMAT_YCBCR_420_888;
      pCam1Streams[1].usage        = 0x00010000;

      streamComb.streams      = pCam1Streams;
      streamComb.num_streams  = MaxStreams;

      // Initialize camera2 entries with camera1 stream combination
      pCam2Streams[0] = pCam1Streams[0];
      pCam2Streams[1] = pCam1Streams[1];
  }

  // Every camera that can open, can be supported individually
  for (uint32_t cameraId = 0; cameraId < numCameras; ++cameraId)
  {
      getCameraInfo(cameraId, &camInfo);
      if (false == IsStreamCombinationSupported(cameraId, streamComb))
      {
          HAL_LOGE("Camera Id %d is not supported", cameraId);
          continue;
      }
      if (CAMERA_FACING_FRONT == camInfo.facing)
      {
          frontCam[frontCamIndex++] = cameraId;
      }
      else if (CAMERA_FACING_BACK == camInfo.facing)
      {
          backCam[backCamIndex++] = cameraId;
      }
  }

  // If either of front or back cameras are not present, then update the list with single camera set.
  // This is just to satisfy API requirements so that regular single camera operations will not impact.
  if (((0 == backCamIndex) || (0 == frontCamIndex)) && (0 != numCameras))
  {
      auto addCameraIdCombo = [&] (concurrent_camera_combination_t& rCameraIdCombo, uint32_t cameraId)
      {
          // Single camera entries will be added if we have only one set of cameras
          rCameraIdCombo.numCameras            = 1;
          rCameraIdCombo.concurrentCamArray[0] = cameraId;
          HAL_LOGV( "Camera Id %d is supported", cameraId);
      };

      for (uint32_t frontIndex = 0; frontIndex < frontCamIndex; ++frontIndex)
      {
          concurrent_camera_combination_t& rCameraIdCombo = (*ppConcCamArray)[(*pConcCamArrayLength)++];
          addCameraIdCombo(rCameraIdCombo, frontCam[frontIndex]);
      }

      for (uint32_t backIndex = 0; backIndex < backCamIndex; ++backIndex)
      {
          concurrent_camera_combination_t& rCameraIdCombo = (*ppConcCamArray)[(*pConcCamArrayLength)++];
          addCameraIdCombo(rCameraIdCombo, backCam[backIndex]);
      }
  }

  for (uint32_t frontIndex = 0; frontIndex < frontCamIndex; ++frontIndex)
  {
      frontCamstreamCombArr.num_streams  = MaxStreams;
      frontCamstreamCombArr.streams      = pCam1Streams;

      pCamIdStreamComb[0].fwkCameraId  = frontCam[frontIndex];
      pCamIdStreamComb[0].streamConfig = &frontCamstreamCombArr;

      for (uint32_t backIndex = 0; backIndex < backCamIndex; ++backIndex)
      {
          backCamstreamCombArr.num_streams  = MaxStreams;
          backCamstreamCombArr.streams      = pCam2Streams;

          pCamIdStreamComb[1].fwkCameraId  = backCam[backIndex];
          pCamIdStreamComb[1].streamConfig = &backCamstreamCombArr;

          if (false == IsConcurrentStreamCombinationSupported(MaxConcCameras, pCamIdStreamComb))
          {
              HAL_LOGV("Camera Id %d, %d is not supported", frontCam[frontIndex], backCam[backIndex]);
              continue;
          }
          concurrent_camera_combination_t& rCameraIdCombo = (*ppConcCamArray)[(*pConcCamArrayLength)++];

          rCameraIdCombo.numCameras            = MaxConcCameras;
          rCameraIdCombo.concurrentCamArray[0] = frontCam[frontIndex];
          rCameraIdCombo.concurrentCamArray[1] = backCam[backIndex];

          HAL_LOGV("Camera Id %d, %d is supported", frontCam[frontIndex], backCam[backIndex]);
      }
  }

  if (NULL != pCam2Streams)
  {
      delete [] pCam2Streams;
  }

  if (NULL != pCam1Streams)
  {
      delete [] pCam1Streams;
  }

  if (NULL != pCamIdStreamComb)
  {
      delete [] pCamIdStreamComb;
  }

  return result;
}
#endif

}  // namespace v4l2_camera_hal

extern "C" {
/*
 * The framework calls the following wrappers, which in turn
 * call the corresponding methods of the global HAL object.
 */

    int get_number_of_cameras() {
      return v4l2_camera_hal::gCameraHAL.getNumberOfCameras();
    }

    int get_camera_info(int id, struct camera_info* info) {
      return v4l2_camera_hal::gCameraHAL.getCameraInfo(id, info);
    }

    int set_callbacks(const camera_module_callbacks_t* callbacks) {
      return v4l2_camera_hal::gCameraHAL.setCallbacks(callbacks);
    }

    void get_vendor_tag_ops(vendor_tag_ops_t* ops) {
      return v4l2_camera_hal::gCameraHAL.getVendorTagOps(ops);
    }

    int open_legacy(const hw_module_t* module,
                           const char* id,
                           uint32_t halVersion,
                           hw_device_t** device) {
      return v4l2_camera_hal::gCameraHAL.openLegacy(module, id, halVersion, device);
    }

    int set_torch_mode(const char* camera_id, bool enabled) {
      return v4l2_camera_hal::gCameraHAL.setTorchMode(camera_id, enabled);
    }

    int open_dev(const hw_module_t* module,
                        const char* name,
                        hw_device_t** device) {
      return v4l2_camera_hal::gCameraHAL.openDevice(module, name, device);
    }

#if defined(AIDL_ANDROID_API)
int is_stream_combination_supported(
    int                                cameraId,
    const camera_stream_combination_t* pStreamCombo)
{
    int result     = 0;
    bool       saneInputs = true;

    bool       invalidStreamUseCase = false;

    if (NULL != pStreamCombo)
    {
        for (uint32_t idx = 0; idx < pStreamCombo->num_streams; idx++)
        {
            HAL_LOGE("  stream[%d] = %p - info:", idx, pStreamCombo->streams[idx]);
            HAL_LOGE("            format       : %d", pStreamCombo->streams[idx].format);
            HAL_LOGE("            width        : %d", pStreamCombo->streams[idx].width);
            HAL_LOGE("            height       : %d", pStreamCombo->streams[idx].height);
            HAL_LOGE("            stream_type  : %08x", pStreamCombo->streams[idx].stream_type);
            HAL_LOGE("            usage        : %08x", pStreamCombo->streams[idx].usage);
            HAL_LOGE("            rotation     : %08x", pStreamCombo->streams[idx].rotation);
            HAL_LOGE("            data_space   : %08x", pStreamCombo->streams[idx].data_space);
            HAL_LOGE("            physical_camera_id         : %s", pStreamCombo->streams[idx].physical_camera_id);
            HAL_LOGE("            group_id                   : %d",pStreamCombo->streams[idx].group_id);
            HAL_LOGE("            stream_use_case             : %d",pStreamCombo->streams[idx].stream_use_case);

            // Check if the stream has a valid rotation
            int  rotation = pStreamCombo->streams[idx].rotation;
            int  format   = pStreamCombo->streams[idx].format;
            uint32_t width    = pStreamCombo->streams[idx].width;
            uint32_t height   = pStreamCombo->streams[idx].height;
#if defined(AIDL_ANDROID_API) //Android-T or better// Android-T or better
            uint64_t streamUseCase = pStreamCombo->streams[idx].stream_use_case;
#endif // Android-T or better

            if ((rotation != CAMERA3_STREAM_ROTATION_0)   && (rotation != CAMERA3_STREAM_ROTATION_90) &&
                (rotation != CAMERA3_STREAM_ROTATION_180) && (rotation != CAMERA3_STREAM_ROTATION_270))
            {
                saneInputs = false;
                HAL_LOGE("Stream has an invalid rotation: %d", rotation);
            }

            // Check if stream has invalid format
            if (format <= 0)
            {
                saneInputs = false;
                HAL_LOGE("Stream has an invalid format: %d", format);
            }

            // Check if stream has non zero width/height
            if ((width < 1) || (height < 1))
            {
                saneInputs = false;
                HAL_LOGE("Invalid width: %u, height: %u", rotation, format, width, height);
            }

#if defined(AIDL_ANDROID_API) //Android-T or better// Android-T or better
            if (((ANDROID_SCALER_AVAILABLE_STREAM_USE_CASES_VIDEO_CALL < streamUseCase) &&
                (ANDROID_SCALER_AVAILABLE_STREAM_USE_CASES_VENDOR_START > streamUseCase)))
            {
                invalidStreamUseCase = true;
                HAL_LOGE("Invalid Stream Use Case %u", streamUseCase);
            }
#endif // Android-T or better
        }

        // Ensure sanity
        if ((false == saneInputs) ||
            (true != v4l2_camera_hal::gCameraHAL.IsStreamCombinationSupported(cameraId, *pStreamCombo)))
        {
            // HAL interface requires -ENODEV (EFailed) if a fatal error occurs
            result = -EINVAL;
            HAL_LOGE("Stream Combination %p, Fwk CamId %d",
                           pStreamCombo,
                           cameraId);
        }
        else if (true == invalidStreamUseCase)
        {
            // VTS test expects -ENOSYS
            result = -ENOSYS;
        }
        else
        {
            HAL_LOGE( "Stream Combination %p, Fwk CamId %d",
                          pStreamCombo,
                          cameraId);
        }
    }
    else
    {
        HAL_LOGE("Invalid argument(s) for process_capture_request()");
        // HAL interface requires -EINVAL (EInvalidArg) for invalid arguments
        result = -EINVAL;
    }

    return (result);
}

int get_concurrent_streaming_camera_ids(
    uint32_t*                         pConcCamArrayLength,
    concurrent_camera_combination_t** ppConcCamArray) {
    return v4l2_camera_hal::gCameraHAL.getConcurrentStreamingCameraIds(pConcCamArrayLength, ppConcCamArray);
}

int is_concurrent_stream_combination_supported(
    uint32_t                             numCombinations,
    const cameraid_stream_combination_t* pStreamComb)
{
    int result = 0;
    if (false == v4l2_camera_hal::gCameraHAL.IsConcurrentStreamCombinationSupported(numCombinations, pStreamComb))
    {
        HAL_LOGE("Streamcombination is not supported");
        result = ENODEV;
    }
    else
    {
        HAL_LOGV("Streamcombination is supported");
    }

    return result;
}
#endif
}  // extern "C"

hw_module_methods_t v4l2_module_methods = {
    .open = open_dev};

camera_module_t HAL_MODULE_INFO_SYM __attribute__((visibility("default"))) = {
        .common =
        {
                .tag = HARDWARE_MODULE_TAG,
#if defined(AIDL_ANDROID_API)
                .module_api_version = CAMERA_MODULE_API_VERSION_2_5,
#else
                .module_api_version = CAMERA_MODULE_API_VERSION_2_4,
#endif
                .hal_api_version = HARDWARE_HAL_API_VERSION,
                .id = CAMERA_HARDWARE_MODULE_ID,
                .name = "V4L2 Camera HAL v3",
                .author = "The Android Open Source Project",
                .methods = &v4l2_module_methods,
                .dso = nullptr,
                .reserved = {0},
        },
    .get_number_of_cameras = get_number_of_cameras,
    .get_camera_info = get_camera_info,
    .set_callbacks = set_callbacks,
    .get_vendor_tag_ops = get_vendor_tag_ops,
    .open_legacy = open_legacy,
    .set_torch_mode = set_torch_mode,
#if defined(AIDL_ANDROID_API) // Android-T or better
    .is_stream_combination_supported = is_stream_combination_supported,
    .get_concurrent_streaming_camera_ids = get_concurrent_streaming_camera_ids,
    .is_concurrent_stream_combination_supported = is_concurrent_stream_combination_supported,
#endif // AIDL_ANDROID_API
    .init = nullptr,
    .reserved = {nullptr, nullptr}};
