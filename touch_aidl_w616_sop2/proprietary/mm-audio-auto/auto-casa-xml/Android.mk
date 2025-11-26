ifeq ($(call is-board-platform-in-list,sdmshrike msmnile gen4 $(MSMSTEPPE)),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)

# ---------------------------------------------------------------------------------
#                 Populate XMLs to file system for CASA
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := hw_ep_info.xml
LOCAL_MODULE_FILENAME   := hw_ep_info.xml
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)
LOCAL_SRC_FILES         := hw_ep_info.xml
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := resourcemanager_8155.xml
LOCAL_MODULE_FILENAME   := resourcemanager_8155.xml
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)
LOCAL_SRC_FILES         := resourcemanager_8155.xml
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := resourcemanager_6155.xml
LOCAL_MODULE_FILENAME   := resourcemanager_6155.xml
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)
LOCAL_SRC_FILES         := resourcemanager_6155.xml
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := resourcemanager_sa8155_adp_star.xml
LOCAL_MODULE_FILENAME   := resourcemanager_sa8155_adp_star.xml
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)
LOCAL_SRC_FILES         := resourcemanager_sa8155_adp_star.xml
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := resourcemanager_sa8255_adp_star.xml
LOCAL_MODULE_FILENAME   := resourcemanager_sa8255_adp_star.xml
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)
LOCAL_SRC_FILES         := resourcemanager_sa8255_adp_star.xml
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := resourcemanager_gvmauto8295_adp_star.xml
LOCAL_MODULE_FILENAME   := resourcemanager_gvmauto8295_adp_star.xml
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)
LOCAL_SRC_FILES         := resourcemanager_gvmauto8295_adp_star.xml
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := resourcemanager_gvmauto8255_adp_star.xml
LOCAL_MODULE_FILENAME   := resourcemanager_gvmauto8255_adp_star.xml
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)
LOCAL_SRC_FILES         := resourcemanager_gvmauto8255_adp_star.xml
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := resourcemanager_gvmauto7255_adp_star.xml
LOCAL_MODULE_FILENAME   := resourcemanager_gvmauto7255_adp_star.xml
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)
LOCAL_SRC_FILES         := resourcemanager_gvmauto7255_adp_star.xml
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := resourcemanager_gvmauto_8155.xml
LOCAL_MODULE_FILENAME   := resourcemanager_gvmauto_8155.xml
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)
LOCAL_SRC_FILES         := resourcemanager_gvmauto_8155.xml
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := resourcemanager_gvmauto_6155.xml
LOCAL_MODULE_FILENAME   := resourcemanager_gvmauto_6155.xml
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)
LOCAL_SRC_FILES         := resourcemanager_gvmauto_6155.xml
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := usecaseKvManager.xml
LOCAL_MODULE_FILENAME   := usecaseKvManager.xml
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)
LOCAL_SRC_FILES         := usecaseKvManager.xml
include $(BUILD_PREBUILT)

# HGY only configs
ifeq ($(TARGET_USES_GY), true)
include $(CLEAR_VARS)
LOCAL_MODULE            := usecaseKvManager-VIOSND.xml
LOCAL_MODULE_FILENAME   := usecaseKvManager-VIOSND.xml
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)
LOCAL_SRC_FILES         := usecaseKvManager-VIOSND.xml
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := resourcemanager_VIOSND.xml
LOCAL_MODULE_FILENAME   := resourcemanager_VIOSND.xml
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)
LOCAL_SRC_FILES         := resourcemanager_VIOSND.xml
include $(BUILD_PREBUILT)
endif

endif #BUILD_TINY_ANDROID
endif
