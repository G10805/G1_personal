LOCAL_PATH := $(call my-dir)

AIDL_SERVICE_PATH := $(LOCAL_PATH)
AIDL_DEVICE_PATH  := $(LOCAL_PATH)/device

AIDL_SERVICE_CPP_FLAGS :=
AIDL_SERVICE_SHARED_LIBRARIES :=            \
    android.hardware.graphics.mapper@2.0    \
    android.hardware.graphics.mapper@3.0    \
    android.hardware.graphics.mapper@4.0    \
    libcamera_metadata                      \
    libcutils                               \
    libexif                                 \
    libfmq                                  \
    libgralloctypes                         \
    libhardware                             \
    libhidlbase                             \
    liblog                                  \
    libutils                                \

AIDL_SERVICE_INCLUDES :=                    \
    $(AIDL_SERVICE_PATH)/../include/hardware  \
    $(AIDL_SERVICE_PATH)/common/1.0/include \

# NDK builds needs extra includes and cpp flags
ifneq ($(AIDL_EXT_VBUILD),)
AIDL_SERVICE_INCLUDES += \
    $(AIDL_C_INCLUDES)

AIDL_SERVICE_CPP_FLAGS += $(AIDL_CPPFLAGS) -std=c++17 -stdlib=libc++

# This macro is added only for NDK builds to supress warnings
AIDL_SERVICE_CPP_FLAGS += -Wno-inconsistent-missing-override
endif

ifneq ( ,$(filter V VanillaIceCream 15, $(PLATFORM_VERSION)))
AIDL_SERVICE_CPP_FLAGS += -D__ANDROID_V__
endif

AIDL_SERVICE_CPP_FLAGS += -DAIDL_ANDROID_API=$(AIDL_ANDROID_SDK_VERSION)

AIDL_PROVIDER_SHARED_LIBRARIES_AIDL :=               \
    android.hardware.camera.common@1.0               \
    android.hardware.camera.common-V1-ndk            \
    android.hardware.camera.device-V1-ndk            \
    android.hardware.camera.metadata-V1-ndk          \
    android.hardware.camera.provider-V1-ndk          \
    android.hardware.common.fmq-V1-ndk               \
    android.hidl.allocator@1.0                       \
    android.hidl.memory@1.0                          \
    aidl.device-impl                                 \
    libtinyxml2                                      \
    $(AIDL_SERVICE_SHARED_LIBRARIES)                 \

include $(AIDL_SERVICE_PATH)/common/1.0/build/android/Android.mk
include $(AIDL_SERVICE_PATH)/device/aidl/build/android/Android.mk
include $(AIDL_SERVICE_PATH)/provider/aidl/build/android/Android.mk

