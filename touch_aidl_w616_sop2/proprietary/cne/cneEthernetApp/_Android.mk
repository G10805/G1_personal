TOP_LOCAL_PATH:= $(call my-dir)
include $(call all-subdir-makefiles)
LOCAL_PATH:= $(TOP_LOCAL_PATH)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(call all-java-files-under)
LOCAL_PACKAGE_NAME := cneEthernetService
LOCAL_PROGUARD_ENABLED := disabled
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := platform
LOCAL_SDK_VERSION := system_current
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PACKAGE)


include $(CLEAR_VARS)
LOCAL_MODULE := cne_auto_permission.xml
LOCAL_SRC_FILES := cne_auto_permission.xml
LOCAL_MODULE_CLASS := ETC
include $(BUILD_PREBUILT)




