LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_SRC_FILES := src/qti_gef_api.c

LOCAL_C_INCLUDES  += $(LOCAL_PATH)/api \
                     $(LOCAL_PATH)/inc \
                     vendor/qcom/opensource/pal \
                     system/core/include/utils

LOCAL_CFLAGS += -Werror -Wall

LOCAL_CFLAGS += -DGEF_PLATFORM_NAME=$(TARGET_BOARD_PLATFORM)

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_GCOV)),true)
LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_CPPFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_STATIC_LIBRARIES += libprofile_rt
endif

LOCAL_SHARED_LIBRARIES := \
        liblog \
        libcutils \
        libdl \
        libar-pal

ifeq ($(call is-platform-sdk-version-at-least,28),true)   #Android P and above
LOCAL_HEADER_LIBRARIES := libaudio_system_headers \
                          libhardware_headers
endif

LOCAL_MODULE := libqtigefar
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
