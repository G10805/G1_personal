/* ===========================================================================
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <cutils/properties.h>
#include <log/log.h>

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

#include <memory>
#include <vector>
#include <utils/Mutex.h>
#include <time.h>

#if !defined(AIDL_ANDROID_API)
#include "../v4l2_camera_hal/camera.h"
#include "../../API/inc/qcarcam.h"
#define QCC6_LIB "/vendor/lib/hw/camera.v4l2.qcc6.so"
#define QCC4_LIB "/vendor/lib/hw/camera.v4l2.qcc4.so"
#else
#include "../ais_v4l2_camera_hal/camera.h"
#include "hardware/camera_common.h"
#include "hardware/hardware.h"
#define QCC6_LIB "/vendor/lib64/hw/camera.v4l2.qcc6.so"
#define QCC4_LIB "/vendor/lib64/hw/camera.v4l2.qcc4.so"
#endif

#ifdef QCC_SOC_OVERRIDE

static bool isMakena()
{
    int     socFd = 0;
    char    buf[32]  = { 0 };
    int     chipsetVersion = -1;
    char    fileName[] = "/sys/devices/soc0/soc_id";

    socFd = open(fileName, O_RDONLY);

    if (socFd > 0)
    {
        int ret = read(socFd, buf, sizeof(buf) - 1);

        if (-1 == ret)
            return false;
        else
            chipsetVersion = atoi(buf);

        close(socFd);

        if (chipsetVersion == 460) //CHIP_ID_SA8295P = 460
            return true;
    }

    return false;
}

#endif

namespace default_camera_hal {

extern "C" {

    Camera *camdev_to_camera(const camera3_device_t *dev) { return NULL; }
    int initialize(const camera3_device_t *dev,
     const camera3_callback_ops_t *callback_ops) { return 0; }
    int configure_streams(const camera3_device_t *dev,
     camera3_stream_configuration_t *stream_list) { return 0; }
    const camera_metadata_t *construct_default_request_settings(
     const camera3_device_t *dev, int type) { return NULL; }
    int process_capture_request(const camera3_device_t *dev,
     camera3_capture_request_t *request) { return 0; }
    void dump(const camera3_device_t *dev, int fd) {}
    int flush(const camera3_device_t *dev) { return 0; }

} // extern "C"

    const camera3_device_ops_t Camera::sOps = {
        .initialize = default_camera_hal::initialize,
        .configure_streams = default_camera_hal::configure_streams,
        .register_stream_buffers = nullptr,
        .construct_default_request_settings
            = default_camera_hal::construct_default_request_settings,
        .process_capture_request = default_camera_hal::process_capture_request,
        .get_metadata_vendor_tag_ops = nullptr,
        .dump = default_camera_hal::dump,
        .flush = default_camera_hal::flush,
        .reserved = {0},
    };
}

namespace v4l2_camera_hal {
    int get_number_of_cameras() { return 0; }
    int get_camera_info(int id, struct camera_info* info) { return 0; }
    int set_callbacks(const camera_module_callbacks_t* callbacks) { return 0; }
    void get_vendor_tag_ops(vendor_tag_ops_t* ops) {}
    int open_legacy(const hw_module_t* module,
                    const char* id,
                    uint32_t halVersion,
                    hw_device_t** device) { return 0; }
    int set_torch_mode(const char* camera_id, bool enabled) { return 0; }
    int open_dev(const hw_module_t* module,
                 const char* name,
                 hw_device_t** device) { return 0; }
#if defined(AIDL_ANDROID_API)
    int is_stream_combination_supported(int cameraId,
                 const camera_stream_combination_t* pStreamCombo) {return 0; }
    int get_concurrent_streaming_camera_ids(uint32_t* pConcCamArrayLength,
                 concurrent_camera_combination_t** ppConcCamArray) { return 0; }
    int is_concurrent_stream_combination_supported(uint32_t numCombinations,
                 const cameraid_stream_combination_t* pStreamComb) { return 0; }
#endif
}

static hw_module_methods_t v4l2_module_methods = {
    .open = v4l2_camera_hal::open_dev
};

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
    .get_number_of_cameras = v4l2_camera_hal::get_number_of_cameras,
    .get_camera_info = v4l2_camera_hal::get_camera_info,
    .set_callbacks = v4l2_camera_hal::set_callbacks,
    .get_vendor_tag_ops = v4l2_camera_hal::get_vendor_tag_ops,
    .open_legacy = v4l2_camera_hal::open_legacy,
    .set_torch_mode = v4l2_camera_hal::set_torch_mode,
#if defined(AIDL_ANDROID_API) // Android-U or better
    .is_stream_combination_supported = v4l2_camera_hal::is_stream_combination_supported,
    .get_concurrent_streaming_camera_ids = v4l2_camera_hal::get_concurrent_streaming_camera_ids,
    .is_concurrent_stream_combination_supported = v4l2_camera_hal::is_concurrent_stream_combination_supported,
#endif // AIDL_ANDROID_API
    .init = nullptr,
    .reserved = {nullptr, nullptr}
};

void *handle = NULL;
char *error = NULL;

__attribute__((constructor)) void init(void) {

    bool isQCC6 = false;
    int val = 0;

    ALOGE("AIS: In v4l2 camera hal wrapper init");

    char value[92];
    int QCC_FLAG_ENABLED = 0;
    property_get("ro.boot.camera.qcc.version", value, "");
    ALOGE("value of property for qcc: %s", value);
    val = atoi(value);
    QCC_FLAG_ENABLED = (strlen(value) > 0) ? 1:0;

    if (QCC_FLAG_ENABLED) {
        isQCC6 = (val == 6) ? true : false;
    }
    else {
#ifdef QCC_SOC_OVERRIDE
    isQCC6 = isMakena();
#endif
    }
    if (isQCC6)
        handle = dlopen (QCC6_LIB, RTLD_GLOBAL | RTLD_NOW);
    else
        handle = dlopen (QCC4_LIB, RTLD_GLOBAL | RTLD_NOW);

    if (!handle) {
        ALOGE("Error while opening %s lib : %s", isQCC6 ? QCC6_LIB : QCC4_LIB, dlerror());
        goto exit;
    } else {
        ALOGE("%s loaded succuessfully", isQCC6 ? QCC6_LIB : QCC4_LIB);

        camera3_device_ops_t* sOps = const_cast<camera3_device_ops_t*>(default_camera_hal::Camera::getsOps());

        sOps->initialize = (int (*)(const camera3_device *, const camera3_callback_ops *))dlsym(handle, "initialize");
        if ((error = dlerror()) != NULL)  {
            ALOGE("dlsym(initialize) failed, error : %s", error);
            goto exit;
        }

        sOps->configure_streams = (int (*)(const camera3_device *, camera3_stream_configuration *))dlsym(handle, "configure_streams");
        if ((error = dlerror()) != NULL)  {
            ALOGE("dlsym(configure_streams) failed, error : %s", error);
            goto exit;
        }

        sOps->construct_default_request_settings = (const camera_metadata *(*)(const camera3_device *, int))dlsym(handle, "construct_default_request_settings");
        if ((error = dlerror()) != NULL)  {
            ALOGE("dlsym(construct_default_request_settings) failed, error : %s", error);
            goto exit;
        }

        sOps->process_capture_request = (int (*)(const camera3_device *, camera3_capture_request *))dlsym(handle, "process_capture_request");
        if ((error = dlerror()) != NULL)  {
            ALOGE("dlsym(process_capture_request) failed, error : %s", error);
            goto exit;
        }

        sOps->dump = (void (*)(const struct camera3_device *, int))dlsym(handle, "dump");
        if ((error = dlerror()) != NULL)  {
            ALOGE("dlsym(dump) failed, error : %s", error);
            goto exit;
        }

        sOps->flush = (int (*)(const struct camera3_device *))dlsym(handle, "flush");
        if ((error = dlerror()) != NULL)  {
            ALOGE("dlsym(flush) failed, error : %s", error);
            goto exit;
        }

        HAL_MODULE_INFO_SYM.common.methods->open = (int (*)(const struct hw_module_t *, const char *, struct hw_device_t **))dlsym(handle, "open_dev");
        if ((error = dlerror()) != NULL)  {
            ALOGE("dlsym(open_dev) failed, error : %s", error);
            goto exit;
        }

        HAL_MODULE_INFO_SYM.get_number_of_cameras = (int (*)())dlsym(handle, "get_number_of_cameras");
        if ((error = dlerror()) != NULL)  {
            ALOGE("dlsym(get_number_of_cameras) failed, error : %s", error);
            goto exit;
        }

        HAL_MODULE_INFO_SYM.get_camera_info = (int (*)(int, struct camera_info *))dlsym(handle, "get_camera_info");
        if ((error = dlerror()) != NULL)  {
            ALOGE("dlsym(get_camera_info) failed, error : %s", error);
            goto exit;
        }

        HAL_MODULE_INFO_SYM.set_callbacks = (int (*)(const camera_module_callbacks_t *))dlsym(handle, "set_callbacks");
        if ((error = dlerror()) != NULL)  {
            ALOGE("dlsym(set_callbacks) failed, error : %s", error);
            goto exit;
        }

        HAL_MODULE_INFO_SYM.get_vendor_tag_ops = (void (*)(vendor_tag_ops_t *))dlsym(handle, "get_vendor_tag_ops");
        if ((error = dlerror()) != NULL)  {
            ALOGE("dlsym(get_vendor_tag_ops) failed, error : %s", error);
            goto exit;
        }

        HAL_MODULE_INFO_SYM.open_legacy = (int (*)(const struct hw_module_t *, const char *, uint32_t, struct hw_device_t **))dlsym(handle, "open_legacy");
        if ((error = dlerror()) != NULL)  {
            ALOGE("dlsym(open_legacy) failed, error : %s", error);
            goto exit;
        }

        HAL_MODULE_INFO_SYM.set_torch_mode = (int (*)(const char *, bool))dlsym(handle, "set_torch_mode");
        if ((error = dlerror()) != NULL)  {
            ALOGE("dlsym(set_torch_mode) failed, error : %s", error);
            goto exit;
        }
#if defined(AIDL_ANDROID_API)
        HAL_MODULE_INFO_SYM.is_stream_combination_supported = (int (*)(int, const camera_stream_combination_t*))dlsym(handle, "is_stream_combination_supported");
        if ((error = dlerror()) != NULL)  {
            ALOGE("dlsym(is_stream_combination_supported) failed, error : %s", error);
            goto exit;
        }

        HAL_MODULE_INFO_SYM.get_concurrent_streaming_camera_ids = (int (*)(uint32_t *, concurrent_camera_combination_t**))dlsym(handle, "get_concurrent_streaming_camera_ids");
        if ((error = dlerror()) != NULL)  {
            ALOGE("dlsym(get_concurrent_streaming_camera_ids) failed, error : %s", error);
            goto exit;
        }

        HAL_MODULE_INFO_SYM.is_concurrent_stream_combination_supported = (int (*)(uint32_t, const cameraid_stream_combination_t*))dlsym(handle, "is_concurrent_stream_combination_supported");
        if ((error = dlerror()) != NULL)  {
            ALOGE("dlsym(is_concurrent_stream_combination_supported) failed, error : %s", error);
            goto exit;
        }
#endif
    }
    ALOGE("%s lib initialization succuessful", isQCC6 ? QCC6_LIB : QCC4_LIB);

    return;

    exit:
        ALOGE("Error while %s lib initialization: %s", isQCC6 ? QCC6_LIB : QCC4_LIB, dlerror());
}

__attribute__((destructor))  void fini(void) {
    ALOGE("AIS: In v4l2 camera hal wrapper exit");
    if (handle != NULL)
        dlclose(handle);
}