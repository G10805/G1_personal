ifeq ($(strip $(AUDIO_FEATURE_QSSI_COMPLIANCE)), true)
ifneq ($(AUDIO_USE_STUB_HAL), true)
include $(call all-subdir-makefiles)
endif
else
ifeq ($(strip $(TARGET_BOARD_PLATFORM)), msmnile)
ifeq ($(strip $(TARGET_BOARD_SUFFIX)), _gvmq)
include $(call all-subdir-makefiles)
endif
endif
endif
