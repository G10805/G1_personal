ifeq ($(strip $(AUDIO_FEATURE_ENABLED_AUDIO_CONTROL_HAL_AIDL)),true)
# Save current directory in variable to use with Qti AudioControl
CURRENT_DIR := $(call my-dir)

LOCAL_PATH := $(TOP)/hardware/interfaces/automotive/audiocontrol/aidl/default

include $(CLEAR_VARS)

# default audiocontrol base lib to link to vendor impl
LOCAL_MODULE := libaudiocontrolbase
LOCAL_VENDOR_MODULE := true

LOCAL_SHARED_LIBRARIES := \
    android.hardware.audio.common@7.0-enums \
    libbase \
    libbinder_ndk \
    libcutils \
    liblog \

ifneq ( ,$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_SHARED_LIBRARIES += \
    android.hardware.automotive.audiocontrol-V3-ndk
else
LOCAL_SHARED_LIBRARIES += \
    android.hardware.automotive.audiocontrol-V2-ndk
endif

LOCAL_SRC_FILES := AudioControl.cpp

include $(BUILD_SHARED_LIBRARY)


# Qti AIDL AudioControl
LOCAL_PATH := $(CURRENT_DIR)
include $(CLEAR_VARS)

LOCAL_MODULE := vendor.qti.hardware.automotive.audiocontrol-service
LOCAL_INIT_RC := vendor.qti.hardware.automotive.audiocontrol-service.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_C_INCLUDES += \
    $(TOP)/hardware/interfaces/automotive/audiocontrol/aidl/default

ifneq ( ,$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_C_INCLUDES += \
    $(TOP)/android/hardware/audio/7.0
else
lOCAL_C_INCLUDES += \
    $(TOP)/android/hardware/audio/6.0
endif

LOCAL_SRC_FILES := \
    QtiAudioControl.cpp \
    QtiAudioControlHalService.cpp \

LOCAL_CFLAGS := \
    -DLOG_TAG=\"QtiAudCntrlDrv\" \
    -O0 \
    -g

ifneq ( ,$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DANDROID_U_HAL7
endif

ifneq ( ,$(filter V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DANDROID_AUDIO_CONTROL_HAL_AIDL
endif

ifneq ( ,$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_SHARED_LIBRARIES := \
    android.hardware.audio@7.0 \
    android.hardware.audio.common@7.0
else
LOCAL_SHARED_LIBRARIES := \
    android.hardware.audio@6.0 \
    android.hardware.audio.common@6.0
endif

LOCAL_SHARED_LIBRARIES += \
    android.hardware.audio.common@7.0-enums \
    libaudiocontrolbase \
    libbase \
    libbinder \
    libcutils \
    libutils \
    liblog \
    libhidlbase \
    libhidltransport \
    libhwbinder \
    libbinder_ndk

ifneq ( ,$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_SHARED_LIBRARIES += \
    android.hardware.automotive.audiocontrol-V3-ndk
else
LOCAL_SHARED_LIBRARIES += \
    android.hardware.automotive.audiocontrol-V2-ndk
endif

ifneq ( ,$(filter V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_SHARED_LIBRARIES += \
    android.hardware.audio.core-V2-ndk \
    qti-audio-types-aidl-V1-ndk
endif

include $(BUILD_EXECUTABLE)
endif
