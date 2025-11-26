ifeq ($(call is-board-platform-in-list, lahaina),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE            := event.eai
LOCAL_MODULE_FILENAME   := event.eai
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/models/acd/
LOCAL_SRC_FILES         := event.eai
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := music.eai
LOCAL_MODULE_FILENAME   := music.eai
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/models/acd/
LOCAL_SRC_FILES         := music.eai
include $(BUILD_PREBUILT)


include $(CLEAR_VARS)
LOCAL_MODULE            := speech.eai
LOCAL_MODULE_FILENAME   := speech.eai
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/models/acd/
LOCAL_SRC_FILES         := speech.eai
include $(BUILD_PREBUILT)

endif #BUILD_TINY_ANDROID
endif
