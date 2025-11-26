#
# ais_v4l2_proxy
#
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin

LOCAL_LDFLAGS :=

LOCAL_SRC_FILES:= src/ais_v4l2_xml_util.cpp src/ais_v4l2_util.cpp src/ais_v4l2_proxy.cpp src/ais_dmabuffer_la.cpp \
                  src/ais_v4l2_proxy_stream.cpp src/ais_v4l2_proxy_input.cpp

LOCAL_C_INCLUDES:= \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/src \
    $(LOCAL_PATH)/../../API/inc \
    $(LOCAL_PATH)/../../Common/inc \
    $(LOCAL_PATH)/../../../../gles/adreno200/include/private/C2D \
    $(TARGET_OUT_HEADERS)/mm-osal/include \
    $(LOCAL_PATH)/../../CameraOSServices/CameraOSServices/inc \
    $(LOCAL_PATH)/../../../../mm-osal/inc \
    $(TARGET_OUT_HEADERS)/common/inc \
    $(LOCAL_PATH)/../../test/test_util/inc \
    external/libxml2/include \
    external/icu/icu4c/source/common \
    $(LOCAL_PATH)/../../../chi-cdk/vendor/node/chiutils \

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/ \
                    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/display \
                    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/ais

LOCAL_HEADER_LIBRARIES := display_intf_headers
ifeq ($(TARGET_KERNEL_VERSION),$(filter 5.15 6.1,$(TARGET_KERNEL_VERSION)))
LOCAL_HEADER_LIBRARIES += qti_ais_kernel_uapi
endif

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_CFLAGS :=-Werror -D_ANDROID_  -DC2D_DISABLED \
    -Wno-unused-variable -Wno-unused-parameter -Wno-missing-field-initializers

ifeq ($(ENABLE_HYP), true)
ifeq ($(PLATFORM_VERSION),$(filter T Tiramisu 13 U UpsideDownCake 14, $(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DQCC_SOC_OVERRIDE
endif
endif

ifeq ($(call is-platform-sdk-version-at-least,28),true)
#Android R uses libmmsoal, overwrite headers and libs
ifeq ( ,$(filter Q 10, $(PLATFORM_VERSION)))
LOCAL_HEADER_LIBRARIES += libmmosal_headers
else
LOCAL_HEADER_LIBRARIES += libmmosal_proprietary_headers
endif
endif

LOCAL_SHARED_LIBRARIES:= libais_client libais_log_proprietary \
    libais_test_util_proprietary liblog libxml2 libEGL libui libutils libcutils
    #libC2D2 #disabled by -DC2D_DISABLED so no need to link

ifeq ( ,$(filter Q 10, $(PLATFORM_VERSION)))
LOCAL_SHARED_LIBRARIES += libhidlbase android.hardware.graphics.mapper@4.0 \
    libgralloctypes
endif

LOCAL_INIT_RC := ais_v4l2_proxy.rc

LOCAL_MODULE:= ais_v4l2_proxy
LOCAL_MODULE_TAGS := optional

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_EXECUTABLE)

#===================
#config files
#===================
include $(CLEAR_VARS)
LOCAL_MODULE:= ais_v4l2loopback_config.xml
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES := config/ais_v4l2loopback_config.xml
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin
LOCAL_MODULE_OWNER := qti
include $(BUILD_PREBUILT)
