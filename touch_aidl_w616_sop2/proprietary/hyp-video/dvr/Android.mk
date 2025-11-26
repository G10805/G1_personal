ifeq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),4.14 4.19 4.9 5.4))
ifeq ($(TARGET_BOARD_PLATFORM), msmnile_gvmq)
ifeq ($(TARGET_ARCH),$(filter $(TARGET_ARCH),arm arm64))
  include $(call all-subdir-makefiles)
endif
endif
endif
