LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#LOCAL_CFLAGS += -Wall -Werror -Wno-missing-field-initializers -Wunused
LOCAL_CFLAGS += -Wall -Wno-missing-field-initializers
LOCAL_C_INCLUDES += vendor/qcom/proprietary/mm-hab/uhab
LOCAL_SRC_FILES :=  habmenu.c habtest.c habtestmem.c khab_test.c

LOCAL_SHARED_LIBRARIES := libuhab
LOCAL_MODULE := uhabtest

# This is not the ideal way to decide ION/DMA HEAP. It has below assumption
# existing supported kernel versions in Android GVM are: 4.14, 5.4, 5.15, 6.1
# 4.14 and 5.4 will pick ION while 5.15, 6.1 and newer kernel will pick dma heap
ifeq ($(TARGET_KERNEL_VERSION), $(filter $(TARGET_KERNEL_VERSION), 4.14 5.4))
	LOCAL_CFLAGS += -DTEST_ION_EXPORT
	LOCAL_C_INCLUDES += system/core/libion/include/ion \
		    system/core/libion/kernel-headers
	LOCAL_SHARED_LIBRARIES += libion
# for additional ION related APIs
ifneq ($(TARGET_COMPILE_WITH_MSM_KERNEL), false)
	LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
	LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif
else
	LOCAL_CFLAGS += -DTEST_DMAHEAP_EXPORT
endif

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)
