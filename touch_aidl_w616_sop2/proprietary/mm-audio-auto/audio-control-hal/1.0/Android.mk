LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.automotive.audiocontrol@1.0-service
LOCAL_INIT_RC := vendor.qti.hardware.automotive.audiocontrol@1.0-service.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_SRC_FILES := \
    QtiAudioControl.cpp \
    QtiAudioControlHalService.cpp

LOCAL_CFLAGS := \
    -DLOG_TAG=\"QtiAudCntrlDrv\" \
    -O0 \
    -g

LOCAL_SHARED_LIBRARIES := \
    android.hardware.automotive.audiocontrol@1.0 \
    libhidlbase \
    libhidltransport \
    liblog \
    libutils \

include $(BUILD_EXECUTABLE)
