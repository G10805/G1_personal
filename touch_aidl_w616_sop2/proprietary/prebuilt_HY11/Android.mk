LOCAL_PATH := $(call my-dir)
PREBUILT_DIR_PATH := $(LOCAL_PATH)

ifeq ($(call is-board-platform,msmnile),true)
ifeq ($(strip $(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX)),_au)
-include $(PREBUILT_DIR_PATH)/target/product/qssi_au/Android.mk
-include $(PREBUILT_DIR_PATH)/target/product/msmnile_au/Android.mk
endif
endif

ifeq ($(call is-board-platform,msmnile),true)
ifeq ($(strip $(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX)),_au_s_u)
-include $(PREBUILT_DIR_PATH)/target/product/qssi_au/Android.mk
-include $(PREBUILT_DIR_PATH)/target/product/msmnile_au_s_u/Android.mk
endif
endif

ifeq ($(call is-board-platform,msmnile),true)
ifeq ($(strip $(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX)),_gvmq)
-include $(PREBUILT_DIR_PATH)/target/product/qssi_au/Android.mk
-include $(PREBUILT_DIR_PATH)/target/product/msmnile_gvmq/Android.mk
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM),qssi)
ifeq ($(strip $(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX)),_au)
-include $(PREBUILT_DIR_PATH)/target/product/qssi_au/Android.mk
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM),qssi)
ifeq ($(strip $(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX)),_au_64)
-include $(PREBUILT_DIR_PATH)/target/product/qssi_au_64/Android.mk
endif
endif

ifeq ($(call is-board-platform,sm6150),true)
ifeq ($(strip $(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX)),_au)
-include $(PREBUILT_DIR_PATH)/target/product/qssi_au/Android.mk
-include $(PREBUILT_DIR_PATH)/target/product/sm6150_au/Android.mk
endif
endif

-include $(sort $(wildcard $(PREBUILT_DIR_PATH)/*/*/Android.mk))
