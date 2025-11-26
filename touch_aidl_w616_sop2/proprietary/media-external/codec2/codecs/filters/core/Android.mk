#--------------------------------------------------------------------------------------------------
# Core Filter Module
#--------------------------------------------------------------------------------------------------
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE         := libqc2filter
LOCAL_VENDOR_MODULE  := true
LOCAL_MODULE_TAGS    := optional


LOCAL_SRC_FILES :=    QC2Filter.cpp \
                      QC2FilterCapsHelper.cpp \
                      QC2FilterCommon.cpp

LOCAL_C_INCLUDES :=	$(LOCAL_PATH)/include

LOCAL_HEADER_LIBRARIES   := libcodec2_headers \
                            libhardware_headers \
                            libqcodec2_platform_headers \
                            display_intf_headers \
                            libqcodec2_base_headers \
                            libqcodec2_utils_headers \
                            libqcodec2_basecodec_headers

LOCAL_SHARED_LIBRARIES  :=  libutils \
                            libcutils \
                            liblog \
                            libcodec2_vndk \
                            libqcodec2_base \
                            libqcodec2_utils \
                            libqcodec2_basecodec

LOCAL_CFLAGS            := -Wall -Werror -std=c++17 -Wno-error=deprecated-declarations \
                           -fexceptions $(CODEC_FLAGS)

LOCAL_CLANG             := true
include $(BUILD_SHARED_LIBRARY)
