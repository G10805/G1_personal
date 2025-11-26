LOCAL_PATH := $(call my-dir)
LOCAL_C_INCLUDES     := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

ifeq ($(filter $(TARGET_BOARD_PLATFORM), kona lahaina lito holi msmnile sm6150),$(TARGET_BOARD_PLATFORM))
  include $(call all-subdir-makefiles)

#==================================================================================================
# Temporarily build modules with Android.mk till kernel headers are available in soong
# TODO(PC): wipe this out and re-enable Android.bp once 'kernel-headers' header_library is available
#==================================================================================================
#--------------------------------------------------------------------------------------------------
# mock filter plugin
#--------------------------------------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE         := libqcodec2_mockfilter
LOCAL_VENDOR_MODULE  := true
LOCAL_MODULE_TAGS    := optional

LOCAL_SRC_FILES      := QC2MockFilter.cpp

LOCAL_C_INCLUDES     :=  $(LOCAL_PATH)/include

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_HEADER_LIBRARIES :=  \
    libcodec2_headers \
    libqcodec2_base_headers \
    libqcodec2_utils_headers \
    display_intf_headers \
    libhardware_headers \
    libqcodec2_v4l2codec_headers \
    libqcodec2_basecodec_headers

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog \
    libqcodec2_base \
    libqcodec2_platform \
    libqcodec2_utils \
    libqcodec2_basecodec \
    libcodec2_vndk \
    libqcodec2_v4l2codec

LOCAL_CFLAGS := -DHAVE_MOCK_CODEC -Wall -Werror -Wno-error=deprecated-declarations \
                 -Wno-enum-compare \

LOCAL_CLANG             := true
include $(BUILD_SHARED_LIBRARY)

endif # TARGET_BOARD_PLATFORM
