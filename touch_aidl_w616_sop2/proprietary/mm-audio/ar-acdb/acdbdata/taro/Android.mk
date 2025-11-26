LOCAL_PATH:= $(call my-dir)

# ---------------------------------------------------------------------------------
#             Populate ACDB data files to file system for Tavil Codec
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := MTP_acdb_cal.acdb
LOCAL_MODULE_FILENAME   := acdb_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/waipio_mtp/
LOCAL_SRC_FILES         := waipio_mtp/acdb_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := MTP_workspaceFileXml.qwsp
LOCAL_MODULE_FILENAME   := workspaceFileXml.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/waipio_mtp/
LOCAL_SRC_FILES         := waipio_mtp/workspaceFileXml.qwsp
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := CDP_acdb_cal.acdb
LOCAL_MODULE_FILENAME   := acdb_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/waipio_cdp/
LOCAL_SRC_FILES         := waipio_cdp/acdb_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := CDP_workspaceFileXml.qwsp
LOCAL_MODULE_FILENAME   := workspaceFileXml.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/waipio_cdp/
LOCAL_SRC_FILES         := waipio_cdp/workspaceFileXml.qwsp
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_acdb_cal.acdb
LOCAL_MODULE_FILENAME   := acdb_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/waipio_qrd/
LOCAL_SRC_FILES         := waipio_qrd/acdb_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_workspaceFileXml.qwsp
LOCAL_MODULE_FILENAME   := workspaceFileXml.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/waipio_qrd/
LOCAL_SRC_FILES         := waipio_qrd/workspaceFileXml.qwsp
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_UPD_acdb_cal.acdb
LOCAL_MODULE_FILENAME   := acdb_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP_UPD/
LOCAL_SRC_FILES         := IDP_UPD/acdb_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_UPD_workspaceFileXml.qwsp
LOCAL_MODULE_FILENAME   := workspaceFileXml.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP_UPD/
LOCAL_SRC_FILES         := IDP_UPD/workspaceFileXml.qwsp
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_diwali_acdb_cal.acdb
LOCAL_MODULE_FILENAME   := acdb_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/diwali_idp/
LOCAL_SRC_FILES         := diwali_idp/acdb_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_diwali_workspaceFileXml.qwsp
LOCAL_MODULE_FILENAME   := workspaceFileXml.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/diwali_idp/
LOCAL_SRC_FILES         := diwali_idp/workspaceFileXml.qwsp
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_diwali_acdb_cal.acdb
LOCAL_MODULE_FILENAME   := acdb_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/diwali_qrd/
LOCAL_SRC_FILES         := diwali_qrd/acdb_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_diwali_workspaceFileXml.qwsp
LOCAL_MODULE_FILENAME   := workspaceFileXml.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/diwali_qrd/
LOCAL_SRC_FILES         := diwali_qrd/workspaceFileXml.qwsp
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_diwali_sku1_acdb_cal.acdb
LOCAL_MODULE_FILENAME   := acdb_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/diwali_idp_sku1/
LOCAL_SRC_FILES         := diwali_idp_sku1/acdb_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_diwali_sku1_workspaceFileXml.qwsp
LOCAL_MODULE_FILENAME   := workspaceFileXml.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/diwali_idp_sku1/
LOCAL_SRC_FILES         := diwali_idp_sku1/workspaceFileXml.qwsp
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_diwali_sku1_acdb_cal.acdb
LOCAL_MODULE_FILENAME   := acdb_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/diwali_qrd_sku1/
LOCAL_SRC_FILES         := diwali_qrd_sku1/acdb_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_diwali_sku1_workspaceFileXml.qwsp
LOCAL_MODULE_FILENAME   := workspaceFileXml.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/diwali_qrd_sku1/
LOCAL_SRC_FILES         := diwali_qrd_sku1/workspaceFileXml.qwsp

# ---------------------------------------------------------------------------------
#             Populate NN NS models for Fluence VX
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := fai__2.3.0_0.1__3.0.0_0.0__eai_1.36_enpu2_comp.pmd
LOCAL_MODULE_FILENAME   := fai__2.3.0_0.1__3.0.0_0.0__eai_1.36_enpu2_comp.pmd
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/nn_ns_models/
LOCAL_SRC_FILES         := nn_ns_models/fai__2.3.0_0.1__3.0.0_0.0__eai_1.36_enpu2_comp.pmd
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := fai__2.3.0_0.1__3.0.0_0.0__eai_1.10.pmd
LOCAL_MODULE_FILENAME   := fai__2.3.0_0.1__3.0.0_0.0__eai_1.10.pmd
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/nn_ns_models/
LOCAL_SRC_FILES         := nn_ns_models/fai__2.3.0_0.1__3.0.0_0.0__eai_1.10.pmd
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := fai__2.0.0_0.1__3.0.0_0.0__eai_1.36_enpu2.pmd
LOCAL_MODULE_FILENAME   := fai__2.0.0_0.1__3.0.0_0.0__eai_1.36_enpu2.pmd
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/nn_ns_models/
LOCAL_SRC_FILES         := nn_ns_models/fai__2.0.0_0.1__3.0.0_0.0__eai_1.36_enpu2.pmd
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := fai__2.7.2_0.0__3.0.0_0.0__eai_1.36_enpu2.pmd
LOCAL_MODULE_FILENAME   := fai__2.7.2_0.0__3.0.0_0.0__eai_1.36_enpu2.pmd
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/nn_ns_models/
LOCAL_SRC_FILES         := nn_ns_models/fai__2.7.2_0.0__3.0.0_0.0__eai_1.36_enpu2.pmd
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := fai__2.7.20_0.0__3.0.0_0.0__eai_1.36_enpu2.pmd
LOCAL_MODULE_FILENAME   := fai__2.7.20_0.0__3.0.0_0.0__eai_1.36_enpu2.pmd
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/nn_ns_models/
LOCAL_SRC_FILES         := nn_ns_models/fai__2.7.20_0.0__3.0.0_0.0__eai_1.36_enpu2.pmd
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := fai__3.0.0_0.0__eai_1.36_enpu2.pmd
LOCAL_MODULE_FILENAME   := fai__3.0.0_0.0__eai_1.36_enpu2.pmd
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/nn_vad_models/
LOCAL_SRC_FILES         := nn_vad_models/fai__3.0.0_0.0__eai_1.36_enpu2.pmd
include $(BUILD_PREBUILT)
