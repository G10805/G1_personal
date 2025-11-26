ifneq ($(TARGET_HAS_LOW_RAM), true)
ifneq ($(TARGET_BOARD_AUTO),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE        := IWlanService
LOCAL_JAVA_LIBRARIES := android.hardware.radio-V1.0-java \
              android.hardware.radio-V1.1-java \
              android.hardware.radio-V1.4-java \
              android.hardware.radio-V1.6-java
LOCAL_STATIC_JAVA_LIBRARIES := vendor.qti.hardware.data.iwlan-V1.0-java \
              vendor.qti.hardware.data.iwlan-V1.1-java \
LOCAL_MODULE_OWNER  := qti
LOCAL_MODULE_TAGS   := optional
LOCAL_MODULE_CLASS  := APPS
LOCAL_CERTIFICATE   := platform
LOCAL_MODULE_SUFFIX := .apk
LOCAL_SRC_FILES     := IWlanService.apk
LOCAL_MODULE_PATH   := $(TARGET_OUT_VENDOR)/app/
include $(BUILD_PREBUILT)

endif
endif
