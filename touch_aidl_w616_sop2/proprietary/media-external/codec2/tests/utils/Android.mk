#--------------------------------------------------------------------------------------------------
# qcodec2 test utils
#--------------------------------------------------------------------------------------------------
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE := libqcodec2_test_utils
LOCAL_PROPRIETARY_MODULE    := true
LOCAL_MODULE_OWNER          := qti

LOCAL_SRC_FILES := \
    ./QC2TestInputStream.cpp \

LOCAL_HEADER_LIBRARIES :=  \
    libhardware_headers \
    libcodec2_headers \
    libqcodec2_base_headers \
    libqcodec2_utils_headers \
    libqcodec2_utils_internal_headers \
    libqcodec2_platform_headers \
    libqcodec2_basecodec_headers \
    libqcodec2_v4l2codec_headers \

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog \
    libmediandk \
    libcodec2_vndk \
    libqcodec2_base \
    libqcodec2_utils \
    libqcodec2_platform \
    libqcodec2_basecodec \

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \

LOCAL_CFLAGS := -Wall -Werror -Wno-error=deprecated-declarations -Wno-enum-compare \
                            $(CODEC_FLAGS) $(EXT_FLAGS)

include $(BUILD_STATIC_LIBRARY)
