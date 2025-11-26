#--------------------------------------------------------------------------------------------------
# qcodec2 filter unit tests
#--------------------------------------------------------------------------------------------------
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE := qcodec2_filter_test
LOCAL_PROPRIETARY_MODULE    := true
LOCAL_MODULE_OWNER          := qti

LOCAL_SRC_FILES := \
    ./test_main.cpp \

LOCAL_HEADER_LIBRARIES :=  \
    libcodec2_headers \
    libqcodec2_base_headers \
    libhardware_headers \
    libqcodec2_platform_headers \
    libqcodec2_utils_headers \
    libqcodec2_utils_internal_headers \
    libqcodec2_core_headers \
    libqcodec2_basecodec_headers \
    libqcodec2_v4l2codec_headers \
    libqcodec2_pipelinecodec_headers \
    display_intf_headers \
    libqcodec2_mockfilter_headers \
    libqcodec2_test_utils_headers \

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog \
    libcodec2_vndk \
    libqcodec2_base \
    libqcodec2_utils \
    libqcodec2_basecodec \
    libqcodec2_platform \
    libqcodec2_core \
    libqcodec2_mockfilter \
    libhidlbase \
    libhidltransport \
    libhwbinder \
    libmediandk \

LOCAL_STATIC_LIBRARIES := libqcodec2_test_utils

LOCAL_C_INCLUDES := \
    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_CFLAGS := -Wall -Werror -Wno-error=deprecated-declarations -Wno-enum-compare \
                            $(CODEC_FLAGS) $(EXT_FLAGS)

include $(BUILD_NATIVE_TEST)
