ifneq ($(AUDIO_USE_STUB_HAL), true)
ifeq ($(TARGET_USES_QCOM_MM_AUDIO), true)
include $(call all-subdir-makefiles)
endif
endif
