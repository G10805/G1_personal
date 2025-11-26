#======================================================================
#makefile for libqcx_aidl_client_qcx.so
#======================================================================
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS := $(qcx_compile_cflags)

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/src \
    $(LOCAL_PATH)/../../../CameraOSServices/CameraOSServices/inc \
    $(LOCAL_PATH)/../../../Common/inc \
    $(LOCAL_PATH)/../../../API/inc \
    $(TOP)/hardware/interfaces/common/support/include/aidlcommonsupport \

LOCAL_SRC_FILES += src/qcx_aidl_client.cpp \
    src/qcarcam_api.cpp \
    ../../../CameraOSServices/CameraOSServicesMMOSAL/src/CameraOSServices.c \
    ../../../Common/src/qcxlog.c \

LOCAL_HEADER_LIBRARIES := libmmosal_headers

LOCAL_STATIC_LIBRARIES := libaidlcommonsupport

LOCAL_CFLAGS := -D_ANDROID_ \
    -DC2D_DISABLED \
    -Wno-error -Wno-unused-parameter

LOCAL_SHARED_LIBRARIES += libutils \
    libmmosal \
    libcutils \
    libais_log_qcx \
    liblog \
    vendor.qti.automotive.qcarcam2-V3-ndk \
    libhwbinder \
    libbinder \
    libbinder_ndk \

LOCAL_MODULE := libqcx_aidl_client

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)

