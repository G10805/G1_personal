#
#   VISTEON TECHNICAL & SERVICES CENTRE CONFIDENTIAL PROPRIETARY
#

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, java) $(call all-Iaidl-files-under, aidl)

LOCAL_PACKAGE_NAME := VstRvcAppTest

LOCAL_PRIVATE_PLATFORM_APIS := true

include $(BUILD_PACKAGE)
