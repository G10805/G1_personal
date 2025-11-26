LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(TOP)/vendor/qcom/proprietary/mm-audio-auto/ams/ams_core/inc/
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-audio-auto/ams/ams_osal/inc
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-audio-auto/ams/ipc/HwBinder/interfaces/ams_ipc/1.0/default/inc
LOCAL_MODULE := vendor.qti.hardware.AMSIPC@1.0-impl
LOCAL_MODULE_OWNER := qti
LOCAL_VENDOR_MODULE := true
LOCAL_CFLAGS += -v -Wall
LOCAL_SRC_FILES := \
    AMS.cpp

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar/ar_osal
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar/gpr


LOCAL_SHARED_LIBRARIES := \
    libhidlbase \
    libutils \
    liblog \
    libcutils \
    libhardware \
    libbase \
    libamscore \
    vendor.qti.hardware.AMSIPC@1.0

include $(BUILD_SHARED_LIBRARY)
