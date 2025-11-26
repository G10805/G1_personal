################################################################################
# @file cne-commonsys/automs_vlan/Android.mk
# @brief Makefile for building vlan interface creation scripts
################################################################################

ifeq ($(TARGET_BOARD_AUTO),true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := create_vlan_vm_1.sh
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := create_vlan_vm_1.sh
LOCAL_INIT_RC := automs_vlan.rc
LOCAL_MODULE_TAGS := optional
LOCAL_SYSTEM_EXT_MODULE := true
LOCAL_MODULE_PATH := $(TARGET_OUT_SYSTEM_EXT)/bin
include $(BUILD_PREBUILT)


include $(CLEAR_VARS)
LOCAL_MODULE := create_vlan_vm_2.sh
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := create_vlan_vm_2.sh
LOCAL_INIT_RC := automs_vlan.rc
LOCAL_SYSTEM_EXT_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_SYSTEM_EXT)/bin
include $(BUILD_PREBUILT)


include $(CLEAR_VARS)
LOCAL_MODULE := network_config_default
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := network_config_default.sh
LOCAL_INIT_RC := automs_vlan.rc
LOCAL_SYSTEM_EXT_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_SYSTEM_EXT)/bin
include $(BUILD_PREBUILT)


endif
