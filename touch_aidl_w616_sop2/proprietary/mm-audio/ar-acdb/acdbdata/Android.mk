ifneq ($(BUILD_TINY_ANDROID),true)
LOCAL_PATH := $(call my-dir)
# --------------------------------------------------------------------------------
#             Populate ACDB data files
# --------------------------------------------------------------------------------

ifeq ($(call is-board-platform-in-list, msmnile gen4),true)
ifeq ($(TARGET_BOARD_AUTO),true)
ifeq ($(call is-board-platform-in-list, gen4),true)
include $(LOCAL_PATH)/gen4_au/Android.mk
else
include $(LOCAL_PATH)/msmnile_au/Android.mk
endif
else
include $(LOCAL_PATH)/msmnile/Android.mk
endif
endif

ifeq ($(call is-board-platform-in-list, kona),true)
include $(LOCAL_PATH)/kona/Android.mk
endif
ifeq ($(call is-board-platform-in-list, lahaina),true)
include $(LOCAL_PATH)/lahaina/Android.mk
endif

endif # BUILD_TINY_ANDROID
