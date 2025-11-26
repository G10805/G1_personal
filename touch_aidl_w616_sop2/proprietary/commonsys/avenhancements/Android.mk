ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS), true)
include $(call all-makefiles-under,$(call my-dir))
endif
