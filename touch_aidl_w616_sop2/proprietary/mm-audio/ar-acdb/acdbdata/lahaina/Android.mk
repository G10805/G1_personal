LOCAL_PATH:= $(call my-dir)

# ---------------------------------------------------------------------------------
#             Populate ACDB data files to file system for Tavil Codec
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := MTP_acdb_cal.acdb
LOCAL_MODULE_FILENAME   := acdb_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/lahaina_mtp/
LOCAL_SRC_FILES         := lahaina_mtp/acdb_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := MTP_workspaceFileXml.qwsp
LOCAL_MODULE_FILENAME   := workspaceFileXml.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/lahaina_mtp/
LOCAL_SRC_FILES         := lahaina_mtp/workspaceFileXml.qwsp
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := CDP_acdb_cal.acdb
LOCAL_MODULE_FILENAME   := acdb_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/lahaina_cdp/
LOCAL_SRC_FILES         := lahaina_cdp/acdb_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := CDP_workspaceFileXml.qwsp
LOCAL_MODULE_FILENAME   := workspaceFileXml.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/lahaina_cdp/
LOCAL_SRC_FILES         := lahaina_cdp/workspaceFileXml.qwsp
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_acdb_cal.acdb
LOCAL_MODULE_FILENAME   := acdb_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/lahaina_qrd/
LOCAL_SRC_FILES         := lahaina_qrd/acdb_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_workspaceFileXml.qwsp
LOCAL_MODULE_FILENAME   := workspaceFileXml.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/lahaina_qrd/
LOCAL_SRC_FILES         := lahaina_qrd/workspaceFileXml.qwsp
include $(BUILD_PREBUILT)

# ---------------------------------------------------------------------------------
#             Populate NN NS models for Fluence VX
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := fai__2.3.0_0.1__3.0.0_0.0__eai_1.10.pmd
LOCAL_MODULE_FILENAME   := fai__2.3.0_0.1__3.0.0_0.0__eai_1.10.pmd
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/nn_ns_models/
LOCAL_SRC_FILES         := nn_ns_models/fai__2.3.0_0.1__3.0.0_0.0__eai_1.10.pmd
include $(BUILD_PREBUILT)
