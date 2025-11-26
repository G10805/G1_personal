#======================================================================
#makefile for libais_hidl_client_qcc6.so
#======================================================================
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS := $(ais_compile_cflags)

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/src \
    $(LOCAL_PATH)/../../../CameraOSServices/CameraOSServices/inc \
    $(LOCAL_PATH)/../../../Common/inc \
    $(LOCAL_PATH)/../../../API/inc

LOCAL_SRC_FILES += src/ais_hidl_client.cpp \
    src/qcarcam_api.cpp \
    ../../../CameraOSServices/CameraOSServicesMMOSAL/src/CameraOSServices.c \
    ../../../Common/src/ais_log.c

LOCAL_HEADER_LIBRARIES := libmmosal_headers

LOCAL_SHARED_LIBRARIES += libutils \
    libmmosal \
    libcutils \
    libais_log_qcc6 \
    liblog \
    vendor.qti.automotive.qcarcam@2.0 \
    libhidltransport \
    libhidlbase \
    libhwbinder \
	libbinder

LOCAL_MODULE := libais_hidl_client_qcc6

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)

