ifeq ($(strip $(AUDIO_FEATURE_ENABLED_AUDIO_CONTROL_HAL)),true)
AUDIO_ROOT := $(call my-dir)
include $(call all-subdir-makefiles)
endif
