#======================================================================
#makefile for libais_power
#======================================================================
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_LDFLAGS :=

LOCAL_CFLAGS:= $(ais_compile_cflags)

LOCAL_C_INCLUDES:= \
    $(LOCAL_PATH)/src \
        $(LOCAL_PATH)/../../API/inc \
        $(LOCAL_PATH)/../../API/vendor \
        $(LOCAL_PATH)/../../Common/inc \
        $(LOCAL_PATH)/inc


LOCAL_SRC_FILES:= src/PowerPolicyService.cpp

LOCAL_HEADER_LIBRARIES := libmmosal_headers

LOCAL_SHARED_LIBRARIES:= \
    libbase \
    libbinder_ndk \
    libcutils \
    libutils \
    libui \
    libhidlbase \
    liblog \
    android.frameworks.automotive.powerpolicy-V1-ndk \
    libpowerpolicyclient

LOCAL_MODULE:= libais_power

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libais_power_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/src/ \
                               $(LOCAL_PATH)/inc \
include $(BUILD_HEADER_LIBRARY)
