LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := fastanimation_config.xml
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT)/fastlogo_file
LOCAL_SRC_FILES := $(TARGET_DEVICE)/config_file/fastanimation_config.xml
include $(BUILD_PREBUILT)