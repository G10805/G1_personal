LOCAL_DIR_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PATH := $(LOCAL_DIR_PATH)
LOCAL_MODULE := VtsBtHfAudio
LOCAL_PROPRIETARY_MODULE := true

LOCAL_SRC_FILES := VtsBtHfAudio.cpp

LOCAL_SHARED_LIBRARIES := \
    libbinder_ndk \
    libc \
    liblog \
    libcutils \
    libutils \
    libhwbinder \
    libhidlbase \
    libhidltransport \
    vendor.qti.hardware.bluetooth.handsfree_audio-V1-ndk \
    libqti-bluetooth-handsfree-audio-rpc

include $(BUILD_EXECUTABLE)