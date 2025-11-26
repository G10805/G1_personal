ifeq ($(call is-board-platform-in-list, sdm710 sdm845 msmnile gen4 kona lahaina sm6150),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := capiv2-headers
LOCAL_EXPORT_C_INCLUDE_DIRS := \
    $(LOCAL_PATH)/

LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_HEADER_LIBRARY)

endif # BUILD_TINY_ANDROID
endif # is-board-platform-in-list
