ifeq ($(call is-board-platform-in-list, gen4),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)

# ---------------------------------------------------------------------------------
#        Populate ACDB data files to file system for ADP_AR Auto Codec for
#        AudioReach
# ---------------------------------------------------------------------------------

ifeq ( ,$(filter 13 T,$(PLATFORM_VERSION)))
include $(CLEAR_VARS)
LOCAL_MODULE            := acdb_cal.acdb
LOCAL_MODULE_FILENAME   := acdb_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/ADP_AR/
LOCAL_SRC_FILES         := ADP_AR/acdb_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := workspaceFileXml.qwsp
LOCAL_MODULE_FILENAME   := workspaceFileXml.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/ADP_AR/
LOCAL_SRC_FILES         := ADP_AR/workspaceFile.qwsp
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := acdb_cal.acdbdelta
LOCAL_MODULE_FILENAME   := acdb_cal.acdbdelta
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/delta/
LOCAL_SRC_FILES         := ADP_AR/acdb_cal.acdbdelta
include $(BUILD_PREBUILT)
else
include $(CLEAR_VARS)
LOCAL_MODULE            := acdb_cal.acdb
LOCAL_MODULE_FILENAME   := acdb_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/ADP_AR/
LOCAL_SRC_FILES         := ADP_AR/tiramisu/acdb_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := workspaceFileXml.qwsp
LOCAL_MODULE_FILENAME   := workspaceFileXml.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/ADP_AR/
LOCAL_SRC_FILES         := ADP_AR/tiramisu/workspaceFileXml.qwsp
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := acdb_cal.acdbdelta
LOCAL_MODULE_FILENAME   := acdb_cal.acdbdelta
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/delta/
LOCAL_SRC_FILES         := ADP_AR/tiramisu/acdb_cal.acdbdelta
include $(BUILD_PREBUILT)

endif #BUILD_TINY_ANDROID
endif
endif
# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------
