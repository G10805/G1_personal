ifeq ($(call is-board-platform-in-list,lahaina),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)

# ---------------------------------------------------------------------------------
#             Populate ACDB data files to file system for IDP Bolero/Traverso Codec
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := MTP_Bluetooth_cal.acdb
LOCAL_MODULE_FILENAME   := Bluetooth_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/MTP/
LOCAL_SRC_FILES         := MTP/Bluetooth_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := MTP_General_cal.acdb
LOCAL_MODULE_FILENAME   := General_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/MTP/
LOCAL_SRC_FILES         := MTP/General_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := MTP_Global_cal.acdb
LOCAL_MODULE_FILENAME   := Global_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/MTP/
LOCAL_SRC_FILES         := MTP/Global_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := MTP_Handset_cal.acdb
LOCAL_MODULE_FILENAME   := Handset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/MTP/
LOCAL_SRC_FILES         := MTP/Handset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := MTP_Hdmi_cal.acdb
LOCAL_MODULE_FILENAME   := Hdmi_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/MTP/
LOCAL_SRC_FILES         := MTP/Hdmi_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := MTP_Headset_cal.acdb
LOCAL_MODULE_FILENAME   := Headset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/MTP/
LOCAL_SRC_FILES         := MTP/Headset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := MTP_Speaker_cal.acdb
LOCAL_MODULE_FILENAME   := Speaker_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/MTP/
LOCAL_SRC_FILES         := MTP/Speaker_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := MTP_workspaceFile.qwsp
LOCAL_MODULE_FILENAME   := workspaceFile.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/MTP/
LOCAL_SRC_FILES         := MTP/workspaceFile.qwsp
include $(BUILD_PREBUILT)

# ---------------------------------------------------------------------------------
#             Populate ACDB data files to file system for IDP Bolero/Traverso Codec
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := CDP_Bluetooth_cal.acdb
LOCAL_MODULE_FILENAME   := Bluetooth_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/CDP/
LOCAL_SRC_FILES         := CDP/Bluetooth_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := CDP_General_cal.acdb
LOCAL_MODULE_FILENAME   := General_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/CDP/
LOCAL_SRC_FILES         := CDP/General_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := CDP_Global_cal.acdb
LOCAL_MODULE_FILENAME   := Global_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/CDP/
LOCAL_SRC_FILES         := CDP/Global_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := CDP_Handset_cal.acdb
LOCAL_MODULE_FILENAME   := Handset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/CDP/
LOCAL_SRC_FILES         := CDP/Handset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := CDP_Hdmi_cal.acdb
LOCAL_MODULE_FILENAME   := Hdmi_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/CDP/
LOCAL_SRC_FILES         := CDP/Hdmi_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := CDP_Headset_cal.acdb
LOCAL_MODULE_FILENAME   := Headset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/CDP/
LOCAL_SRC_FILES         := CDP/Headset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := CDP_Speaker_cal.acdb
LOCAL_MODULE_FILENAME   := Speaker_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/CDP/
LOCAL_SRC_FILES         := CDP/Speaker_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := CDP_workspaceFile.qwsp
LOCAL_MODULE_FILENAME   := workspaceFile.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/CDP/
LOCAL_SRC_FILES         := CDP/workspaceFile.qwsp
include $(BUILD_PREBUILT)

# ---------------------------------------------------------------------------------
#             Populate ACDB data files to file system for IOT rb3gen2
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := IOT_Bluetooth_cal.acdb
LOCAL_MODULE_FILENAME   := Bluetooth_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IOT/
LOCAL_SRC_FILES         := IOT/Bluetooth_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IOT_General_cal.acdb
LOCAL_MODULE_FILENAME   := General_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IOT/
LOCAL_SRC_FILES         := IOT/General_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IOT_Global_cal.acdb
LOCAL_MODULE_FILENAME   := Global_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IOT/
LOCAL_SRC_FILES         := IOT/Global_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IOT_Handset_cal.acdb
LOCAL_MODULE_FILENAME   := Handset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IOT/
LOCAL_SRC_FILES         := IOT/Handset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IOT_Hdmi_cal.acdb
LOCAL_MODULE_FILENAME   := Hdmi_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IOT/
LOCAL_SRC_FILES         := IOT/Hdmi_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IOT_Headset_cal.acdb
LOCAL_MODULE_FILENAME   := Headset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IOT/
LOCAL_SRC_FILES         := IOT/Headset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IOT_Speaker_cal.acdb
LOCAL_MODULE_FILENAME   := Speaker_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IOT/
LOCAL_SRC_FILES         := IOT/Speaker_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IOT_workspaceFile.qwsp
LOCAL_MODULE_FILENAME   := workspaceFile.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IOT/
LOCAL_SRC_FILES         := IOT/workspaceFile.qwsp
include $(BUILD_PREBUILT)

# ---------------------------------------------------------------------------------
#             Populate ACDB data files to file system for yupik IDP/IDPS
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_Yupik_Bluetooth_cal.acdb
LOCAL_MODULE_FILENAME   := Bluetooth_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/lahaina-yupikidp-snd-card/
LOCAL_SRC_FILES         := IDP/lahaina-yupikidp-snd-card/Bluetooth_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_Yupik_General_cal.acdb
LOCAL_MODULE_FILENAME   := General_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/lahaina-yupikidp-snd-card/
LOCAL_SRC_FILES         := IDP/lahaina-yupikidp-snd-card/General_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_Yupik_Global_cal.acdb
LOCAL_MODULE_FILENAME   := Global_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/lahaina-yupikidp-snd-card/
LOCAL_SRC_FILES         := IDP/lahaina-yupikidp-snd-card/Global_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_Yupik_Handset_cal.acdb
LOCAL_MODULE_FILENAME   := Handset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/lahaina-yupikidp-snd-card/
LOCAL_SRC_FILES         := IDP/lahaina-yupikidp-snd-card/Handset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_Yupik_Hdmi_cal.acdb
LOCAL_MODULE_FILENAME   := Hdmi_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/lahaina-yupikidp-snd-card/
LOCAL_SRC_FILES         := IDP/lahaina-yupikidp-snd-card/Hdmi_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_Yupik_Headset_cal.acdb
LOCAL_MODULE_FILENAME   := Headset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/lahaina-yupikidp-snd-card/
LOCAL_SRC_FILES         := IDP/lahaina-yupikidp-snd-card/Headset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_Yupik_Speaker_cal.acdb
LOCAL_MODULE_FILENAME   := Speaker_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/lahaina-yupikidp-snd-card/
LOCAL_SRC_FILES         := IDP/lahaina-yupikidp-snd-card/Speaker_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_Yupik_workspaceFile.qwsp
LOCAL_MODULE_FILENAME   := workspaceFile.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/lahaina-yupikidp-snd-card/
LOCAL_SRC_FILES         := IDP/lahaina-yupikidp-snd-card/workspaceFile.qwsp
include $(BUILD_PREBUILT)

# ---------------------------------------------------------------------------------
#             Populate ACDB data files to file system for shima IDP
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_Shima_Bluetooth_cal.acdb
LOCAL_MODULE_FILENAME   := Bluetooth_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/
LOCAL_SRC_FILES         := IDP/Bluetooth_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_Shima_General_cal.acdb
LOCAL_MODULE_FILENAME   := General_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/
LOCAL_SRC_FILES         := IDP/General_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_Shima_Global_cal.acdb
LOCAL_MODULE_FILENAME   := Global_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/
LOCAL_SRC_FILES         := IDP/Global_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_Shima_Handset_cal.acdb
LOCAL_MODULE_FILENAME   := Handset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/
LOCAL_SRC_FILES         := IDP/Handset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_Shima_Hdmi_cal.acdb
LOCAL_MODULE_FILENAME   := Hdmi_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/
LOCAL_SRC_FILES         := IDP/Hdmi_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_Shima_Headset_cal.acdb
LOCAL_MODULE_FILENAME   := Headset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/
LOCAL_SRC_FILES         := IDP/Headset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_Shima_Speaker_cal.acdb
LOCAL_MODULE_FILENAME   := Speaker_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/
LOCAL_SRC_FILES         := IDP/Speaker_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDP_Shima_workspaceFile.qwsp
LOCAL_MODULE_FILENAME   := workspaceFile.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/
LOCAL_SRC_FILES         := IDP/workspaceFile.qwsp
include $(BUILD_PREBUILT)

# ---------------------------------------------------------------------------------
#             Populate ACDB data files to file system for shima IDPS
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := IDPS_Shima_Bluetooth_cal.acdb
LOCAL_MODULE_FILENAME   := Bluetooth_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/lahaina-shimaidps-snd-card/
LOCAL_SRC_FILES         := IDP/lahaina-shimaidps-snd-card/Bluetooth_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDPS_Shima_General_cal.acdb
LOCAL_MODULE_FILENAME   := General_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/lahaina-shimaidps-snd-card/
LOCAL_SRC_FILES         := IDP/lahaina-shimaidps-snd-card/General_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDPS_Shima_Global_cal.acdb
LOCAL_MODULE_FILENAME   := Global_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/lahaina-shimaidps-snd-card/
LOCAL_SRC_FILES         := IDP/lahaina-shimaidps-snd-card/Global_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDPS_Shima_Handset_cal.acdb
LOCAL_MODULE_FILENAME   := Handset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/lahaina-shimaidps-snd-card/
LOCAL_SRC_FILES         := IDP/lahaina-shimaidps-snd-card/Handset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDPS_Shima_Hdmi_cal.acdb
LOCAL_MODULE_FILENAME   := Hdmi_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/lahaina-shimaidps-snd-card/
LOCAL_SRC_FILES         := IDP/lahaina-shimaidps-snd-card/Hdmi_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDPS_Shima_Headset_cal.acdb
LOCAL_MODULE_FILENAME   := Headset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/lahaina-shimaidps-snd-card/
LOCAL_SRC_FILES         := IDP/lahaina-shimaidps-snd-card/Headset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDPS_Shima_Speaker_cal.acdb
LOCAL_MODULE_FILENAME   := Speaker_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/lahaina-shimaidps-snd-card/
LOCAL_SRC_FILES         := IDP/lahaina-shimaidps-snd-card/Speaker_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := IDPS_Shima_workspaceFile.qwsp
LOCAL_MODULE_FILENAME   := workspaceFile.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/IDP/lahaina-shimaidps-snd-card/
LOCAL_SRC_FILES         := IDP/lahaina-shimaidps-snd-card/workspaceFile.qwsp
include $(BUILD_PREBUILT)

# ---------------------------------------------------------------------------------
#             Populate ACDB data files to file system for HDK Bolero/Traverso Codec
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := HDK_Bluetooth_cal.acdb
LOCAL_MODULE_FILENAME   := Bluetooth_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/HDK/
LOCAL_SRC_FILES         := HDK/Bluetooth_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := HDK_General_cal.acdb
LOCAL_MODULE_FILENAME   := General_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/HDK/
LOCAL_SRC_FILES         := HDK/General_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := HDK_Global_cal.acdb
LOCAL_MODULE_FILENAME   := Global_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/HDK/
LOCAL_SRC_FILES         := HDK/Global_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := HDK_Handset_cal.acdb
LOCAL_MODULE_FILENAME   := Handset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/HDK/
LOCAL_SRC_FILES         := HDK/Handset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := HDK_Hdmi_cal.acdb
LOCAL_MODULE_FILENAME   := Hdmi_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/HDK/
LOCAL_SRC_FILES         := HDK/Hdmi_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := HDK_Headset_cal.acdb
LOCAL_MODULE_FILENAME   := Headset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/HDK/
LOCAL_SRC_FILES         := HDK/Headset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := HDK_Speaker_cal.acdb
LOCAL_MODULE_FILENAME   := Speaker_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/HDK/
LOCAL_SRC_FILES         := HDK/Speaker_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := HDK_workspaceFile.qwsp
LOCAL_MODULE_FILENAME   := workspaceFile.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/HDK/
LOCAL_SRC_FILES         := HDK/workspaceFile.qwsp
include $(BUILD_PREBUILT)

# ---------------------------------------------------------------------------------
#             Populate ACDB data files to file system for HDK HHG Bolero/Traverso Codec
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := HHG_Bluetooth_cal.acdb
LOCAL_MODULE_FILENAME   := Bluetooth_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/HDK/lahaina-hhg-snd-card/
LOCAL_SRC_FILES         := HDK/lahaina-hhg-snd-card/Bluetooth_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := HHG_General_cal.acdb
LOCAL_MODULE_FILENAME   := General_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/HDK/lahaina-hhg-snd-card/
LOCAL_SRC_FILES         := HDK/lahaina-hhg-snd-card/General_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := HHG_Global_cal.acdb
LOCAL_MODULE_FILENAME   := Global_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/HDK/lahaina-hhg-snd-card/
LOCAL_SRC_FILES         := HDK/lahaina-hhg-snd-card/Global_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := HHG_Handset_cal.acdb
LOCAL_MODULE_FILENAME   := Handset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/HDK/lahaina-hhg-snd-card/
LOCAL_SRC_FILES         := HDK/lahaina-hhg-snd-card/Handset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := HHG_Hdmi_cal.acdb
LOCAL_MODULE_FILENAME   := Hdmi_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/HDK/lahaina-hhg-snd-card/
LOCAL_SRC_FILES         := HDK/lahaina-hhg-snd-card/Hdmi_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := HHG_Headset_cal.acdb
LOCAL_MODULE_FILENAME   := Headset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/HDK/lahaina-hhg-snd-card/
LOCAL_SRC_FILES         := HDK/lahaina-hhg-snd-card/Headset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := HHG_Speaker_cal.acdb
LOCAL_MODULE_FILENAME   := Speaker_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/HDK/lahaina-hhg-snd-card/
LOCAL_SRC_FILES         := HDK/lahaina-hhg-snd-card/Speaker_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := HHG_workspaceFile.qwsp
LOCAL_MODULE_FILENAME   := workspaceFile.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/HDK/lahaina-hhg-snd-card/
LOCAL_SRC_FILES         := HDK/lahaina-hhg-snd-card/workspaceFile.qwsp
include $(BUILD_PREBUILT)


# ---------------------------------------------------------------------------------
#             Populate ACDB data files to file system for QRD Bolero/Traverso Codec
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Bluetooth_cal.acdb
LOCAL_MODULE_FILENAME   := Bluetooth_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/
LOCAL_SRC_FILES         := QRD/Bluetooth_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_General_cal.acdb
LOCAL_MODULE_FILENAME   := General_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/
LOCAL_SRC_FILES         := QRD/General_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Global_cal.acdb
LOCAL_MODULE_FILENAME   := Global_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/
LOCAL_SRC_FILES         := QRD/Global_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Handset_cal.acdb
LOCAL_MODULE_FILENAME   := Handset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/
LOCAL_SRC_FILES         := QRD/Handset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Hdmi_cal.acdb
LOCAL_MODULE_FILENAME   := Hdmi_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/
LOCAL_SRC_FILES         := QRD/Hdmi_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Headset_cal.acdb
LOCAL_MODULE_FILENAME   := Headset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/
LOCAL_SRC_FILES         := QRD/Headset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Speaker_cal.acdb
LOCAL_MODULE_FILENAME   := Speaker_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/
LOCAL_SRC_FILES         := QRD/Speaker_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_workspaceFile.qwsp
LOCAL_MODULE_FILENAME   := workspaceFile.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/
LOCAL_SRC_FILES         := QRD/workspaceFile.qwsp
include $(BUILD_PREBUILT)

# ---------------------------------------------------------------------------------
#             Populate ACDB data files to file system for Yupik QRD
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Yupik_Bluetooth_cal.acdb
LOCAL_MODULE_FILENAME   := Bluetooth_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/lahaina-yupikqrd-snd-card/
LOCAL_SRC_FILES         := QRD/lahaina-yupikqrd-snd-card/Bluetooth_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Yupik_General_cal.acdb
LOCAL_MODULE_FILENAME   := General_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/lahaina-yupikqrd-snd-card/
LOCAL_SRC_FILES         := QRD/lahaina-yupikqrd-snd-card/General_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Yupik_Global_cal.acdb
LOCAL_MODULE_FILENAME   := Global_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/lahaina-yupikqrd-snd-card/
LOCAL_SRC_FILES         := QRD/lahaina-yupikqrd-snd-card/Global_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Yupik_Handset_cal.acdb
LOCAL_MODULE_FILENAME   := Handset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/lahaina-yupikqrd-snd-card/
LOCAL_SRC_FILES         := QRD/lahaina-yupikqrd-snd-card/Handset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Yupik_Hdmi_cal.acdb
LOCAL_MODULE_FILENAME   := Hdmi_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/lahaina-yupikqrd-snd-card/
LOCAL_SRC_FILES         := QRD/lahaina-yupikqrd-snd-card/Hdmi_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Yupik_Headset_cal.acdb
LOCAL_MODULE_FILENAME   := Headset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/lahaina-yupikqrd-snd-card/
LOCAL_SRC_FILES         := QRD/lahaina-yupikqrd-snd-card/Headset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Yupik_Speaker_cal.acdb
LOCAL_MODULE_FILENAME   := Speaker_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/lahaina-yupikqrd-snd-card/
LOCAL_SRC_FILES         := QRD/lahaina-yupikqrd-snd-card/Speaker_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Yupik_workspaceFile.qwsp
LOCAL_MODULE_FILENAME   := workspaceFile.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/lahaina-yupikqrd-snd-card/
LOCAL_SRC_FILES         := QRD/lahaina-yupikqrd-snd-card/workspaceFile.qwsp
include $(BUILD_PREBUILT)

# ---------------------------------------------------------------------------------
#             Populate ACDB data files to file system for Shima QRD
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Shima_Bluetooth_cal.acdb
LOCAL_MODULE_FILENAME   := Bluetooth_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/lahaina-shimaqrd-snd-card/
LOCAL_SRC_FILES         := QRD/lahaina-shimaqrd-snd-card/Bluetooth_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Shima_General_cal.acdb
LOCAL_MODULE_FILENAME   := General_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/lahaina-shimaqrd-snd-card/
LOCAL_SRC_FILES         := QRD/lahaina-shimaqrd-snd-card/General_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Shima_Global_cal.acdb
LOCAL_MODULE_FILENAME   := Global_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/lahaina-shimaqrd-snd-card/
LOCAL_SRC_FILES         := QRD/lahaina-shimaqrd-snd-card/Global_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Shima_Handset_cal.acdb
LOCAL_MODULE_FILENAME   := Handset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/lahaina-shimaqrd-snd-card/
LOCAL_SRC_FILES         := QRD/lahaina-shimaqrd-snd-card/Handset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Shima_Hdmi_cal.acdb
LOCAL_MODULE_FILENAME   := Hdmi_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/lahaina-shimaqrd-snd-card/
LOCAL_SRC_FILES         := QRD/lahaina-shimaqrd-snd-card/Hdmi_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Shima_Headset_cal.acdb
LOCAL_MODULE_FILENAME   := Headset_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/lahaina-shimaqrd-snd-card/
LOCAL_SRC_FILES         := QRD/lahaina-shimaqrd-snd-card/Headset_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Shima_Speaker_cal.acdb
LOCAL_MODULE_FILENAME   := Speaker_cal.acdb
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/lahaina-shimaqrd-snd-card/
LOCAL_SRC_FILES         := QRD/lahaina-shimaqrd-snd-card/Speaker_cal.acdb
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := QRD_Shima_workspaceFile.qwsp
LOCAL_MODULE_FILENAME   := workspaceFile.qwsp
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/QRD/lahaina-shimaqrd-snd-card/
LOCAL_SRC_FILES         := QRD/lahaina-shimaqrd-snd-card/workspaceFile.qwsp
include $(BUILD_PREBUILT)

# ---------------------------------------------------------------------------------
#             Populate NN VAD models for Fluence VX
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := fai_3.0.0_0.0_eai_1.00.pmd
LOCAL_MODULE_FILENAME   := fai_3.0.0_0.0_eai_1.00.pmd
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/nn_vad_models/
LOCAL_SRC_FILES         := nn_vad_models/fai_3.0.0_0.0_eai_1.00.pmd
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := fai__3.0.0_0.0__eai_1.10.pmd
LOCAL_MODULE_FILENAME   := fai__3.0.0_0.0__eai_1.10.pmd
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/nn_vad_models/
LOCAL_SRC_FILES         := nn_vad_models/fai__3.0.0_0.0__eai_1.10.pmd
include $(BUILD_PREBUILT)

# ---------------------------------------------------------------------------------
#             Populate NN NS models for Fluence VX
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := fai__2.0.0_0.1__3.0.0_0.0__eai_1.10.pmd
LOCAL_MODULE_FILENAME   := fai__2.0.0_0.1__3.0.0_0.0__eai_1.10.pmd
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/nn_ns_models/
LOCAL_SRC_FILES         := nn_ns_models/fai__2.0.0_0.1__3.0.0_0.0__eai_1.10.pmd
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
LOCAL_MODULE            := fai__2.0.0_0.1__3.0.0_0.0__eai_1.00.pmd
LOCAL_MODULE_FILENAME   := fai__2.0.0_0.1__3.0.0_0.0__eai_1.00.pmd
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/nn_ns_models/
LOCAL_SRC_FILES         := nn_ns_models/fai__2.0.0_0.1__3.0.0_0.0__eai_1.00.pmd
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := fai__2.3.0_0.1__3.0.0_0.0__eai_1.00.pmd
LOCAL_MODULE_FILENAME   := fai__2.3.0_0.1__3.0.0_0.0__eai_1.00.pmd
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/nn_ns_models/
LOCAL_SRC_FILES         := nn_ns_models/fai__2.3.0_0.1__3.0.0_0.0__eai_1.00.pmd
include $(BUILD_PREBUILT)

# ---------------------------------------------------------------------------------
#             Populate NN NS models for Fluence VXIII
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := fai__4.8.8_0.0__3.0.0_0.0__3.1.2_0.0__3.2.0_0.1__eai_1.10.pmd
LOCAL_MODULE_FILENAME   := fai__4.8.8_0.0__3.0.0_0.0__3.1.2_0.0__3.2.0_0.1__eai_1.10.pmd
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/acdbdata/nn_ns_models/
LOCAL_SRC_FILES         := nn_ns_models/fai__4.8.8_0.0__3.0.0_0.0__3.1.2_0.0__3.2.0_0.1__eai_1.10.pmd
include $(BUILD_PREBUILT)

endif #BUILD_TINY_ANDROID
endif
# ---------------------------------------------------------------------------------
#                 END
# ---------------------------------------------------------------------------------
