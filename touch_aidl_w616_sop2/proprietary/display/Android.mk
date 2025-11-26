ifneq ($(TARGET_DISABLE_DISPLAY),true)
DISPLAY_DIR := $(call my-dir)

include $(DISPLAY_DIR)/auto-test/test-apps/test/Android.mk
include $(DISPLAY_DIR)/auto-test/test-apps/drm-test/Android.mk
include $(DISPLAY_DIR)/auto-test/test-apps/test-app/Android.mk
ifeq ($(TARGET_BOARD_AUTO),true)
include $(DISPLAY_DIR)/auto-test/utils/eSplash/Android.mk
endif

endif #TARGET_DISABLE_DISPLAY
