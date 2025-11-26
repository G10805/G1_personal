LOCAL_PATH := $(call my-dir)
# Build libams_headers
include $(CLEAR_VARS)
LOCAL_MODULE                := libams_headers
LOCAL_VENDOR_MODULE         := true
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/../api
include $(BUILD_HEADER_LIBRARY)

#LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_USE_VNDK := true

#----------------------------------------------------------------------------
#                 Common definitons
#----------------------------------------------------------------------------

ams-def += -D_ANDROID_

#----------------------------------------------------------------------------
#             Make the Shared library (libams)
#----------------------------------------------------------------------------

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../api
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar/ar_osal
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../ipc/HwBinder/ams_client_proxy/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../ams_core/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../ams_osal/inc
LOCAL_CFLAGS     := $(ams-def)
LOCAL_CFLAGS += -Wno-tautological-compare
LOCAL_CFLAGS += -Wno-macro-redefined
LOCAL_CFLAGS += -D_GNU_SOURCE
LOCAL_CFLAGS += -Wall -DAMS_SSR_SUPPORT_EN

LOCAL_SRC_FILES  := src/ams_impl.c\
                    src/ams_platform.c\
                    src/ams_util.c\
                    src/la_driver/platform_drv.c

LOCAL_MODULE               := libams
LOCAL_MODULE_OWNER         := qti
LOCAL_MODULE_TAGS          := optional

LOCAL_HEADER_LIBRARIES := libspf-headers \
                          libutils_headers \
                          libams_headers \
                          libamsosal_headers

LOCAL_SHARED_LIBRARIES := \
      liblog \
      libutils \
      libcutils \
      libamsclient \
      liblx-osal \
      libamsosal

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DYNAMIC_LOG)), true)
      LOCAL_CFLAGS += -DDYNAMIC_LOG_ENABLED
      LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-audio/audio-log-utils
      LOCAL_C_INCLUDES += $(TOP)/external/expat/lib/expat.h
      LOCAL_SHARED_LIBRARIES += libaudio_log_utils \
                                libexpat
endif


LOCAL_VENDOR_MODULE := true

include $(BUILD_SHARED_LIBRARY)

