LOCAL_PATH := $(call my-dir)/../..

# This is needed for techpack builds
include $(CLEAR_VARS)
LOCAL_MODULE       := vendor.qti.camera.provider.xml
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)/vintf/manifest
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

# Shared library name
LOCAL_MODULE       := aidl.provider-impl
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES    :=               \
    camera_provider.cpp

LOCAL_PROPRIETARY_MODULE    := true
LOCAL_CPPFLAGS              := $(AIDL_SERVICE_CPP_FLAGS)
LOCAL_EXPORT_C_INCLUDE_DIRS :=     \
    $(LOCAL_PATH)                  \

LOCAL_STATIC_LIBRARIES      := \
     aidl.common@1.0-helper
LOCAL_SHARED_LIBRARIES      :=              \
    libbase                                 \
    libbinder_ndk                           \
    libcutils                               \
    libfmq                                  \
    liblog                                  \
    libutils                                \
    $(AIDL_PROVIDER_SHARED_LIBRARIES_AIDL)

LOCAL_C_INCLUDES :=                                     \
    $(AIDL_SERVICE_INCLUDES)                            \
    $(AIDL_DEVICE_PATH)/aidl/include                    \
    $(AIDL_SERVICE_PATH)/provider/aidl

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

# Binary name
LOCAL_MODULE       := android.hardware.camera.provider@V1-qti-service
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_INIT_RC      := vendor.qti.camera.provider-service.rc

# If 32 bit service is needed, update LOCAL_MULTILIB to 32.
# Make sure only version of service is compiled for each build variant.
LOCAL_MULTILIB     := 64
LOCAL_SRC_FILES    := \
    service.cpp

LOCAL_MODULE_RELATIVE_PATH  := hw
LOCAL_PROPRIETARY_MODULE    := true
LOCAL_CPPFLAGS              := $(AIDL_SERVICE_CPP_FLAGS)
LOCAL_STATIC_LIBRARIES      := \
    aidl.common@1.0-helper

LOCAL_SHARED_LIBRARIES      :=              \
    aidl.provider-impl                      \
    libbase                                 \
    libbinder                               \
    libbinder_ndk                           \
    libhardware                             \
    libfmq                                  \
    $(AIDL_PROVIDER_SHARED_LIBRARIES_AIDL)

LOCAL_VINTF_FRAGMENTS := vendor.qti.camera.provider.xml

LOCAL_C_INCLUDES :=                                     \
    $(AIDL_SERVICE_INCLUDES)                            \
    $(AIDL_DEVICE_PATH)/aidl/include                    \
    $(AIDL_SERVICE_PATH)/provider/aidl                  \

include $(BUILD_EXECUTABLE)

# This is needed for techpack builds
include $(CLEAR_VARS)
LOCAL_MODULE       := vendor.qti.camera.provider-service.rc
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)/init
include $(BUILD_PREBUILT)
