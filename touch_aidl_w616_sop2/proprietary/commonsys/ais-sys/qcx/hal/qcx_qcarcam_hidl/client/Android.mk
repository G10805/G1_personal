#======================================================================
#makefile for libqcx_hidl_client.so
#======================================================================
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS := $(qcx_compile_cflags)

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/src \
    $(LOCAL_PATH)/../../../CameraOSServices/CameraOSServices/inc \
    $(LOCAL_PATH)/../../../Common/inc \
    $(LOCAL_PATH)/../../../API/inc

LOCAL_SRC_FILES += src/qcx_hidl_client.cpp \
    src/qcarcam_api.cpp \
    ../../../CameraOSServices/CameraOSServicesMMOSAL/src/CameraOSServices.c \
    ../../../Common/src/qcxlog.c

LOCAL_HEADER_LIBRARIES := libmmosal_headers

LOCAL_SHARED_LIBRARIES += libutils \
    libmmosal \
    libcutils \
    libais_log_qcx \
    liblog \
    vendor.qti.automotive.qcarcam@2.0 \
    libhidltransport \
    libhidlbase \
    libhwbinder \
    libbinder

LOCAL_MODULE := libqcx_hidl_client

ifeq ($(QCX_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)

