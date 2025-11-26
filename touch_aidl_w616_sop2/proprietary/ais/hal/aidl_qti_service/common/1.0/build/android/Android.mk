LOCAL_PATH := $(call my-dir)/../..

include $(CLEAR_VARS)

# Static library name
LOCAL_MODULE       := aidl.common@1.0-helper
LOCAL_MODULE_CLASS := STATIC_LIBRARIES

LOCAL_SRC_FILES :=              \
    CameraMetadata.cpp          \
    CameraModule.cpp            \
    CameraParameters.cpp        \
    Exif.cpp                    \
    HandleImporter.cpp          \
    VendorTagDescriptor.cpp     \

LOCAL_PROPRIETARY_MODULE    := true
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/inc
LOCAL_SHARED_LIBRARIES      :=      \
    libc++                          \
    $(AIDL_SERVICE_SHARED_LIBRARIES)

LOCAL_LDLIBS   :=
LOCAL_LDFLAGS  :=
LOCAL_CPPFLAGS := $(AIDL_SERVICE_CPP_FLAGS)

LOCAL_C_INCLUDES :=             \
    $(AIDL_SERVICE_INCLUDES)

include $(BUILD_STATIC_LIBRARY)
