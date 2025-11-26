LOCAL_PATH          := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := ams_test

LOCAL_SRC_FILES:= \
    ams_test.c \
    ams_test__share_mic_spkr.c


LOCAL_C_INCLUDES := $(LOCAL_PATH)/../api
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../ams_lib/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../ams_core/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../ams_osal/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar/ar_osal
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar/gpr

LOCAL_SHARED_LIBRARIES := liblog \
                        libcutils \
                        libdl \
                        libbase \
                        libutils \
                        libams

LOCAL_CFLAGS := -DAMS_CORE_TEST

LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)