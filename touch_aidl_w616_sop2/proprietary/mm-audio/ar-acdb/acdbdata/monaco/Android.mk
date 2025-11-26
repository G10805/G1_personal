ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)

# ---------------------------------------------------------------------------------
# #             Populate ACDB data files to file system for Tavil Codec
# # ---------------------------------------------------------------------------------
 ifeq ($(TARGET_SUPPORTS_WEAR_AON),true)
  include $(CLEAR_VARS)
  LOCAL_MODULE            := IDP_acdb_cal_monaco_slate.acdb
  LOCAL_MODULE_FILENAME   := acdb_cal.acdb
  LOCAL_MODULE_TAGS       := optional
  LOCAL_MODULE_CLASS      := ETC
  LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/monaco_idp_slate/
  LOCAL_SRC_FILES         := monaco_idp_slate/acdb_cal.acdb
  include $(BUILD_PREBUILT)

  include $(CLEAR_VARS)
  LOCAL_MODULE            := IDP_workspaceFileXml_monaco_slate.qwsp
  LOCAL_MODULE_FILENAME   := workspaceFileXml.qwsp
  LOCAL_MODULE_TAGS       := optional
  LOCAL_MODULE_CLASS      := ETC
  LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/monaco_idp_slate/
  LOCAL_SRC_FILES         := monaco_idp_slate/workspaceFileXml.qwsp
  include $(BUILD_PREBUILT)

  include $(CLEAR_VARS)
  LOCAL_MODULE            := IDP_acdb_cal_monaco_slate_amic.acdb
  LOCAL_MODULE_FILENAME   := acdb_cal.acdb
  LOCAL_MODULE_TAGS       := optional
  LOCAL_MODULE_CLASS      := ETC
  LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/monaco_idp_slate_amic/
  LOCAL_SRC_FILES         := monaco_idp_slate_amic/acdb_cal.acdb
  include $(BUILD_PREBUILT)

  include $(CLEAR_VARS)
  LOCAL_MODULE            := IDP_workspaceFileXml_monaco_slate_amic.qwsp
  LOCAL_MODULE_FILENAME   := workspaceFileXml.qwsp
  LOCAL_MODULE_TAGS       := optional
  LOCAL_MODULE_CLASS      := ETC
  LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/monaco_idp_slate_amic/
  LOCAL_SRC_FILES         := monaco_idp_slate_amic/workspaceFileXml.qwsp
  include $(BUILD_PREBUILT)

  include $(CLEAR_VARS)
  LOCAL_MODULE            := IDP_acdb_cal_monaco_slate_wsa.acdb
  LOCAL_MODULE_FILENAME   := acdb_cal.acdb
  LOCAL_MODULE_TAGS       := optional
  LOCAL_MODULE_CLASS      := ETC
  LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/monaco_idp_slate_wsa/
  LOCAL_SRC_FILES         := monaco_idp_slate_wsa/acdb_cal.acdb
  include $(BUILD_PREBUILT)

  include $(CLEAR_VARS)
  LOCAL_MODULE            := IDP_workspaceFileXml_monaco_slate_wsa.qwsp
  LOCAL_MODULE_FILENAME   := workspaceFileXml.qwsp
  LOCAL_MODULE_TAGS       := optional
  LOCAL_MODULE_CLASS      := ETC
  LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/monaco_idp_slate_wsa/
  LOCAL_SRC_FILES         := monaco_idp_slate_wsa/workspaceFileXml.qwsp
  include $(BUILD_PREBUILT)
 else
  include $(CLEAR_VARS)
  LOCAL_MODULE            := IDP_acdb_cal_monaco.acdb
  LOCAL_MODULE_FILENAME   := acdb_cal.acdb
  LOCAL_MODULE_TAGS       := optional
  LOCAL_MODULE_CLASS      := ETC
  LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/monaco_idp/
  LOCAL_SRC_FILES         := monaco_idp/acdb_cal.acdb
  include $(BUILD_PREBUILT)

  include $(CLEAR_VARS)
  LOCAL_MODULE            := IDP_workspaceFileXml_monaco.qwsp
  LOCAL_MODULE_FILENAME   := workspaceFileXml.qwsp
  LOCAL_MODULE_TAGS       := optional
  LOCAL_MODULE_CLASS      := ETC
  LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/monaco_idp/
  LOCAL_SRC_FILES         := monaco_idp/workspaceFileXml.qwsp
  include $(BUILD_PREBUILT)

  include $(CLEAR_VARS)
  LOCAL_MODULE            := IDP_acdb_cal_monaco_amic.acdb
  LOCAL_MODULE_FILENAME   := acdb_cal.acdb
  LOCAL_MODULE_TAGS       := optional
  LOCAL_MODULE_CLASS      := ETC
  LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/monaco_idp_amic/
  LOCAL_SRC_FILES         := monaco_idp_amic/acdb_cal.acdb
  include $(BUILD_PREBUILT)

  include $(CLEAR_VARS)
  LOCAL_MODULE            := IDP_workspaceFileXml_monaco_amic.qwsp
  LOCAL_MODULE_FILENAME   := workspaceFileXml.qwsp
  LOCAL_MODULE_TAGS       := optional
  LOCAL_MODULE_CLASS      := ETC
  LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/monaco_idp_amic/
  LOCAL_SRC_FILES         := monaco_idp_amic/workspaceFileXml.qwsp
  include $(BUILD_PREBUILT)

  include $(CLEAR_VARS)
  LOCAL_MODULE            := IDP_acdb_cal_monaco_wsa.acdb
  LOCAL_MODULE_FILENAME   := acdb_cal.acdb
  LOCAL_MODULE_TAGS       := optional
  LOCAL_MODULE_CLASS      := ETC
  LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/monaco_idp_wsa/
  LOCAL_SRC_FILES         := monaco_idp_wsa/acdb_cal.acdb
  include $(BUILD_PREBUILT)

  include $(CLEAR_VARS)
  LOCAL_MODULE            := IDP_workspaceFileXml_monaco_wsa.qwsp
  LOCAL_MODULE_FILENAME   := workspaceFileXml.qwsp
  LOCAL_MODULE_TAGS       := optional
  LOCAL_MODULE_CLASS      := ETC
  LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/monaco_idp_wsa/
  LOCAL_SRC_FILES         := monaco_idp_wsa/workspaceFileXml.qwsp
  include $(BUILD_PREBUILT)
 endif
endif #BUILD_TINY_ANDROID
# # ---------------------------------------------------------------------------------
# #                     END
# # ---------------------------------------------------------------------------------
#
