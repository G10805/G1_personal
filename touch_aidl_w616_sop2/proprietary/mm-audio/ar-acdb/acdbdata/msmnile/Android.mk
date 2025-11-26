LOCAL_PATH:= $(call my-dir)

# ---------------------------------------------------------------------------------
#             Populate ACDB data files to file system for Tavil Codec
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := acdb_cal.acdb
LOCAL_MODULE_FILENAME   := acdb_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/MTP/
LOCAL_SRC_FILES         := MTP/acdb_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := workspaceFileXml.qwsp
LOCAL_MODULE_FILENAME   := workspaceFileXml.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/MTP/
LOCAL_SRC_FILES         := MTP/workspaceFileXml.qwsp
include $(BUILD_PREBUILT)
