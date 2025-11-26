LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

# Add enroll config file to make code auto enroll in case device
# is not enrolled while making call for signing.
LOCAL_MODULE := c2pa_enroll_config.json
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := c2pa_enroll_config.json
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_ETC)/ssg
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

