#Disabling ImsDataChannelService App for Low Memory targets
ifneq ($(TARGET_HAS_LOW_RAM), true)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := vendor.qti.imsdcservice.xml
LOCAL_SYSTEM_EXT_MODULE := true
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_SYSTEM_EXT_ETC)/permissions
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_PROGUARD_ENABLED := disabled

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_JAVA_LIBRARIES := vendor.qti.ims.datachannelservice-V1-java
LOCAL_STATIC_JAVA_LIBRARIES := vendor.qti.imsdatachannel

LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PACKAGE_NAME := ImsDataChannelService
LOCAL_SYSTEM_EXT_MODULE := true
LOCAL_CERTIFICATE := platform
LOCAL_MODULE_OWNER := qti


include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
endif
