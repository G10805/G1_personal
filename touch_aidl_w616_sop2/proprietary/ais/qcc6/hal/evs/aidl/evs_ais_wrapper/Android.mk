LOCAL_PATH:= $(call my-dir)

##################################
include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin

LOCAL_SRC_FILES := \
    src/bufferCopy.cpp \
    src/ConfigManager.cpp \
    src/ConfigManagerUtil.cpp \
    src/EvsAISCamera.cpp \
    src/EvsEnumerator.cpp \
    src/EvsGlDisplay.cpp \
    src/GlWrapper.cpp \
    src/service.cpp

LOCAL_SHARED_LIBRARIES := \
    android.hardware.graphics.bufferqueue@1.0 \
    android.hardware.graphics.bufferqueue@2.0 \
    android.hidl.token@1.0-utils \
    libEGL \
    libGLESv2 \
    libbase \
    libbinder_ndk \
    libbufferqueueconverter \
    libcamera_metadata \
    libhardware_legacy \
    libhidlbase \
    liblog \
    android.frameworks.automotive.display-V1-ndk \
    android.hardware.automotive.evs-V1-ndk \
    android.hardware.common-V2-ndk \
    android.hardware.graphics.common-V3-ndk \
    libnativewindow \
    libtinyxml2 \
    libui \
    libutils \
    libyuv \
    android.frameworks.automotive.display@1.0 \
    libC2D2 \
    libcutils \
    android.hardware.graphics.mapper@4.0 \
    libgralloctypes

LOCAL_STATIC_LIBRARIES := \
    libaidlcommonsupport

ifeq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_STATIC_LIBRARIES += libgui_aidl_static
endif


LOCAL_INIT_RC := android.hardware.automotive.evs-ais-qcc6.rc

#qcarcam library
LOCAL_SHARED_LIBRARIES += libais_client_qcc6 libais_log_proprietary_qcc6

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \
                    $(TOP)/frameworks/native/include \
                    $(TOP)/vendor/qcom/proprietary/ais/qcc6/API/inc \
                    $(TOP)/vendor/qcom/proprietary/ais/qcc6/Common/inc \
                    $(TOP)/vendor/qcom/proprietary/ais/qcc6/API/vendor

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/adreno \
                    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
                    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/display

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_HEADER_LIBRARIES += display_intf_headers
LOCAL_MODULE := android.hardware.automotive.evs-ais-qcc6

LOCAL_MODULE_TAGS := optional
LOCAL_STRIP_MODULE := keep_symbols

LOCAL_CFLAGS := -DLOG_TAG=\"EvsHal\"
LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES -DFPS_PRINT
LOCAL_CFLAGS += -Wall -Werror -Wunreachable-code -Wthread-safety
LOCAL_CFLAGS += -DQCC_SOC_OVERRIDE

ifneq ( ,$(filter 11 R ,$(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DANDROID_R_AOSP
endif

ifeq ($(AIS_BUILD_WITH_CPMS_SUPPORT),true)
LOCAL_SHARED_LIBRARIES += libais_power android.frameworks.automotive.powerpolicy-V1-ndk
LOCAL_CFLAGS += -DHAL_CAMERA_CPMS_SUPPORT
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/ais/qcc6/hal/power/inc
endif

LOCAL_VINTF_FRAGMENTS := manifest_android.hardware.automotive.evs-default.xml

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := evs_configuration_ais_aidl_qcc6.dtd
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/etc/automotive/evs
LOCAL_SRC_FILES := resources/evs_configuration.dtd
include $(BUILD_PREBUILT)

# Enable this for override xml
#include $(CLEAR_VARS)
#LOCAL_MODULE := evs_configuration_default.xml
#LOCAL_MODULE_CLASS := ETC
#LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/etc/automotive/evs
#LOCAL_SRC_FILES := resources/evs_sample_configuration.xml
#include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := evs_ais_aidl_resources
LOCAL_REQUIRED_MODULES := \
    evs_configuration_ais.dtd \
    evs_sample_configuration_ais.xml
include $(BUILD_PHONY_PACKAGE)
