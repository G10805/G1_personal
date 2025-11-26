LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libamsclient
LOCAL_MODULE_OWNER := qti
LOCAL_VENDOR_MODULE := true

LOCAL_C_INCLUDES = $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-audio-auto/ams/ams_core/inc/
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-audio-auto/ams/ams_osal/inc

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar/ar_osal
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar/gpr

LOCAL_SRC_FILES := \
    src/ams_client_proxy.cpp

LOCAL_SHARED_LIBRARIES := \
    libhidlbase \
    libutils \
    liblog \
    libcutils \
    libhardware \
    libbase \
    vendor.qti.hardware.AMSIPC@1.0

include $(BUILD_SHARED_LIBRARY)