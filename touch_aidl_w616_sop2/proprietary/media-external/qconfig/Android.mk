LOCAL_PATH := $(call my-dir)

# Local control to disable Qconfig
TARGET_DISABLE_QCONFIG_COMPILE := false

ifeq ($(TARGET_DISABLE_GLOBAL_VPP), true)
    TARGET_DISABLE_QCONFIG_COMPILE := true
endif # TARGET_DISABLE_GLOBAL_VPP

ifneq ($(TARGET_DISABLE_QCONFIG_COMPILE), true)
#--------------------------------------------------------------------------------------------------
# qconfigservice
#--------------------------------------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_MODULE                := qconfigservice

LOCAL_MODULE_TAGS           := optional
LOCAL_MODULE_RELATIVE_PATH  := hw
LOCAL_PROPRIETARY_MODULE    := true
LOCAL_MODULE_OWNER          := qti

LOCAL_INIT_RC := service/qconfig.rc \

LOCAL_VINTF_FRAGMENTS := \
    service/vendor.qti.hardware.qconfig-service.xml

LOCAL_SRC_FILES  := service/QConfigService.cpp \
                    service/QConfig.cpp \

LOCAL_C_INCLUDES := $(LOCAL_PATH)/service/inc \

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog \
    vendor.qti.hardware.qconfig-V1-ndk \
    libjsoncpp \
    libbase \
    libbinder_ndk \

LOCAL_CFLAGS           := -Wall -Werror \

include $(BUILD_EXECUTABLE)

#--------------------------------------------------------------------------------------------------
# libqconfigclient
#--------------------------------------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_MODULE         := libqconfigclient

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER   := qti

LOCAL_SRC_FILES      := \
    client/QConfigClient.cpp \

LOCAL_C_INCLUDES        := $(LOCAL_PATH)/client/inc \
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/client/inc \

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog \
    vendor.qti.hardware.qconfig-V1-ndk \
    libjsoncpp \
    libbase \
    libbinder_ndk \

LOCAL_CFLAGS := -Wall -Werror \

include $(BUILD_SHARED_LIBRARY)

# --------------------------------------------------------------------------------------------------
#  qconfig_test
# --------------------------------------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_MODULE             := qconfig_test

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER       := qti

LOCAL_SRC_FILES  := \
    service/QConfig.cpp \
    client/QConfigClient.cpp \
    test/unittest_main.cpp

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog \
    vendor.qti.hardware.qconfig-V1-ndk \
    libjsoncpp \
    libbase \
    libbinder_ndk \

LOCAL_C_INCLUDES := $(LOCAL_PATH)/service/inc \
                    $(LOCAL_PATH)/client/inc

LOCAL_CFLAGS := -Wall -Werror \

include $(BUILD_NATIVE_TEST)

#--------------------------------------------------------------------------------------------------
# qconfigfunctest
#--------------------------------------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_MODULE             := qconfigfunctest

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS        := optional

LOCAL_SRC_FILES      := \
    client/QConfigClient.cpp \
    test/functest_main.cpp \

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog \
    vendor.qti.hardware.qconfig-V1-ndk \
    libjsoncpp \
    libbase \
    libbinder_ndk \

LOCAL_C_INCLUDES := $(LOCAL_PATH)/client/inc \

LOCAL_CFLAGS := -Wall -Werror \

include $(BUILD_EXECUTABLE)

endif # TARGET_DISABLE_QCONFIG_COMPILE
