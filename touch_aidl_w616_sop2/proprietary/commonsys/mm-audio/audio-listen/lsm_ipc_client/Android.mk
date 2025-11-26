LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := liblsmclient
LOCAL_MODULE_OWNER := qti
LOCAL_SYSTEM_EXT_MODULE := true
LOCAL_MULTILIB := both

LOCAL_SRC_FILES := \
    src/lsm_client_wrapper.cpp\

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/../framework/jni/inc \

LOCAL_SHARED_LIBRARIES := \
    libhidlbase \
    libutils \
    liblog \
    libcutils \
    libhardware \
    libbase \
    android.hidl.allocator@1.0 \
    android.hidl.memory@1.0 \
    libhidlmemory \
    vendor.qti.hardware.ListenSoundModel@1.0 \

include $(BUILD_SHARED_LIBRARY)

