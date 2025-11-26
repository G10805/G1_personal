#Disabling ImsRcsService App for Low Memory targets
ifneq ($(TARGET_HAS_LOW_RAM), true)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := vendor.qti.ims.rcsservice.xml
LOCAL_SYSTEM_EXT_MODULE := true
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_SYSTEM_EXT_ETC)/permissions
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_PROGUARD_ENABLED := disabled

LOCAL_SRC_FILES := $(call all-subdir-java-files)


LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PACKAGE_NAME := ImsRcsService
LOCAL_SYSTEM_EXT_MODULE := true
LOCAL_CERTIFICATE := platform
LOCAL_MODULE_OWNER := qti

LOCAL_STATIC_JAVA_LIBRARIES := vendor.qti.ims.configservice-V1.0-java
LOCAL_STATIC_JAVA_LIBRARIES += vendor.qti.ims.callcapability-V1.0-java
LOCAL_STATIC_JAVA_LIBRARIES += vendor.qti.ims.configservice-V1.1-java
LOCAL_STATIC_JAVA_LIBRARIES += android.hidl.manager-V1.2-java

LOCAL_JAVA_LIBRARIES := vendor.qti.ims.factory-V2.0-java
LOCAL_JAVA_LIBRARIES += vendor.qti.ims.connection-V1.0-java
LOCAL_JAVA_LIBRARIES += vendor.qti.ims.rcsuce-V1.0-java
LOCAL_JAVA_LIBRARIES += vendor.qti.ims.rcssip-V1.0-java
LOCAL_JAVA_LIBRARIES += vendor.qti.ims.factory-V2.1-java
LOCAL_JAVA_LIBRARIES += vendor.qti.ims.rcsuce-V1.1-java
LOCAL_JAVA_LIBRARIES += vendor.qti.ims.rcssip-V1.1-java
LOCAL_JAVA_LIBRARIES += vendor.qti.ims.factory-V2.2-java
LOCAL_JAVA_LIBRARIES += vendor.qti.ims.rcsuce-V1.2-java
LOCAL_JAVA_LIBRARIES += vendor.qti.ims.rcssip-V1.2-java

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
endif
