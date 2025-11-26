#======================================================================
#makefile for libais_aidl_client.so
#======================================================================
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS := $(ais_compile_cflags)

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/src \
    $(LOCAL_PATH)/../../../CameraOSServices/CameraOSServices/inc \
    $(LOCAL_PATH)/../../../Common/inc \
    $(LOCAL_PATH)/../../../API/inc \
    $(TOP)/hardware/interfaces/common/support/include/aidlcommonsupport

LOCAL_SRC_FILES += src/ais_aidl_client.cpp \
    src/qcarcam_api.cpp \
    ../../../CameraOSServices/CameraOSServicesMMOSAL/src/CameraOSServices.c \
    ../../../Common/src/ais_log.c

LOCAL_HEADER_LIBRARIES := libmmosal_headers

LOCAL_STATIC_LIBRARIES := libaidlcommonsupport

LOCAL_SHARED_LIBRARIES += libutils \
    libmmosal \
    libcutils \
    libais_log \
    liblog \
    libbase \
    libbinder \
    libbinder_ndk \
    vendor.qti.automotive.qcarcam-V2-ndk

LOCAL_MODULE := libais_aidl_client

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)
