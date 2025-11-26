LOCAL_PATH:= $(call my-dir)

##################################
include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin

LOCAL_SRC_FILES := \
    GlWrapper.cpp \
    bufferCopy.cpp \
    service.cpp \
    EvsEnumerator.cpp \
    EvsAISCamera.cpp \
    EvsGlDisplay.cpp \
    ConfigManager.cpp \
    ConfigManagerUtil.cpp

ifeq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14, $(PLATFORM_VERSION)))
LOCAL_STATIC_LIBRARIES += libgui_aidl_static
endif

LOCAL_SHARED_LIBRARIES := \
    android.hardware.automotive.evs@1.0 \
    android.hardware.automotive.evs@1.1 \
    android.hardware.camera.device@3.2 \
    libui \
    libEGL \
    libGLESv2 \
    libbase \
    libbinder \
    libcutils \
    libhardware \
    libhidlbase \
    libgralloctypes \
    liblog \
    libutils \
    libhardware_legacy \
    libcamera_metadata \
    libtinyxml2 \
    libbufferqueueconverter \
    android.hidl.token@1.0-utils \
    android.frameworks.automotive.display@1.0 \
    android.hardware.graphics.bufferqueue@1.0 \
    android.hardware.graphics.bufferqueue@2.0 \
    libC2D2 \
    android.hardware.graphics.mapper@4.0

LOCAL_INIT_RC := android.hardware.automotive.evs@1.1-ais-qcc6.rc

#qcarcam library
LOCAL_SHARED_LIBRARIES += libais_client_qcc6 libais_log_proprietary_qcc6

LOCAL_C_INCLUDES := $(TOP)/frameworks/native/include \
                    $(TOP)/vendor/qcom/proprietary/ais/qcc6/API/inc \
                    $(TOP)/vendor/qcom/proprietary/ais/qcc6/API/vendor \
                    $(TOP)/vendor/qcom/proprietary/ais/qcc6/Common/inc

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/adreno \
                    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
                    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/display

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_HEADER_LIBRARIES += display_intf_headers
LOCAL_MODULE := android.hardware.automotive.evs@1.1-ais-qcc6

LOCAL_MODULE_TAGS := optional
LOCAL_STRIP_MODULE := keep_symbols

LOCAL_CFLAGS := -DLOG_TAG=\"EvsHal\"
LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES -DFPS_PRINT
LOCAL_CFLAGS += -Wall -Werror -Wunreachable-code

ifeq ($(ENABLE_HYP), true)
ifeq ($(PLATFORM_VERSION),$(filter T Tiramisu 13 U UpsideDownCake 14, $(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DQCC_SOC_OVERRIDE
endif
endif

ifneq ( ,$(filter 11 R ,$(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DANDROID_R_AOSP
endif

LOCAL_VINTF_FRAGMENTS := manifest_android.hardware.automotive.evs@1.1.xml

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := evs_configuration_ais_qcc6.dtd
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/etc/automotive/evs
LOCAL_SRC_FILES := resources/evs_configuration.dtd
include $(BUILD_PREBUILT)

# Enable this for override xml
#include $(CLEAR_VARS)
#LOCAL_MODULE := evs_sample_configuration_ais_qcc6.xml
#LOCAL_MODULE_CLASS := ETC
#LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/etc/automotive/evs
#LOCAL_SRC_FILES := resources/evs_sample_configuration.xml
#include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := evs_ais_hal_resources_qcc6
LOCAL_REQUIRED_MODULES := \
    evs_configuration_ais.dtd \
    evs_sample_configuration_ais.xml
include $(BUILD_PHONY_PACKAGE)
