ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM),qssi sdm660 msm8937 msm8953))
ifneq ($(TARGET_BOARD_SUFFIX), _32go)
include $(call all-subdir-makefiles)
endif
endif
