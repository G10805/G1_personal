LOCAL_PATH := $(call my-dir)/../..

include $(CLEAR_VARS)

# Shared library
LOCAL_MODULE       := aidl.device-impl
LOCAL_MODULE_CLASS := SHARED_LIBRARIES

LOCAL_SRC_FILES :=            \
    camera_device.cpp         \
    camera_device_session.cpp \
    convert.cpp               \

LOCAL_PROPRIETARY_MODULE    := true
LOCAL_CPPFLAGS              := $(AIDL_SERVICE_CPP_FLAGS)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include \

LOCAL_STATIC_LIBRARIES      :=  \
    aidl.common@1.0-helper      \
    libaidlcommonsupport

LOCAL_SHARED_LIBRARIES      :=              \
    android.hardware.camera.common-V1-ndk   \
    android.hardware.camera.device-V1-ndk   \
    android.hardware.camera.metadata-V1-ndk \
    android.hardware.camera.provider-V1-ndk \
    android.hardware.common.fmq-V1-ndk      \
    libbinder_ndk                           \
    $(AIDL_SERVICE_SHARED_LIBRARIES)

LOCAL_C_INCLUDES :=                                 \
    $(AIDL_SERVICE_INCLUDES)                        \
    $(LOCAL_PATH)/include/                          \

LOCAL_EXPORT_SHARED_LIBRARY_HEADERS := libfmq

include $(BUILD_SHARED_LIBRARY)
