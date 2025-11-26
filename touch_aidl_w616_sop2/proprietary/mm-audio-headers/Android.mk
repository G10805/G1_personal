LOCAL_PATH := $(call my-dir)

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DYNAMIC_LOG)), true)

include $(CLEAR_VARS)

LOCAL_MODULE := libaudiologutils_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/audio_log_utils
LOCAL_VENDOR_MODULE := true
include $(BUILD_HEADER_LIBRARY)

endif

include $(CLEAR_VARS)

LOCAL_MODULE := libcapiv2_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/capiv2_api
LOCAL_VENDOR_MODULE := true

include $(BUILD_HEADER_LIBRARY)

# 3-mic SSR header library
include $(CLEAR_VARS)
LOCAL_MODULE := libsurround_3mic_proc_headers
LOCAL_EXPORT_C_INCLUDE_DIRS   := $(LOCAL_PATH)/surround_rec_interface/
LOCAL_VENDOR_MODULE := true
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libar-gsl_fe_headers
LOCAL_EXPORT_C_INCLUDE_DIRS   := $(LOCAL_PATH)/gsl_fe/
LOCAL_VENDOR_MODULE := true
include $(BUILD_HEADER_LIBRARY)
