# @file Android.mk
#
ifneq ($(TARGET_IS_HEADLESS), true)
include $(call all-subdir-makefiles)
endif
