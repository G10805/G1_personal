#--------------------------------------------------------------------------------------------------
# Customization/debugging hooks
#--------------------------------------------------------------------------------------------------
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE         := libqcodec2_hooks
LOCAL_VENDOR_MODULE  := true
LOCAL_MODULE_TAGS    := optional


LOCAL_SRC_FILES :=      QC2ComponentObserver.cpp \
                        QC2PerfController.cpp

LOCAL_HEADER_LIBRARIES :=  \
    libcodec2_headers \
    libqcodec2_base_headers \
    libqcodec2_hooks_headers \
    libhardware_headers \
    display_intf_headers

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog \
    libcodec2_vndk \
    libqcodec2_base \

LOCAL_CFLAGS            := -Wall -Werror -std=c++17 -Wno-error=deprecated-declarations \
                           -fexceptions $(CODEC_FLAGS)

LOCAL_CLANG             := true

include $(BUILD_SHARED_LIBRARY)
