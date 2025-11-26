
LOCAL_PATH          := $(call my-dir)

#temporary solution for VTS test vts_treble_vintf_vendor_test failure
#Adding Android U and target check to avoid AMS service inclusion for elite
#on LA3.6.0
ifneq ( ,$(filter U UpsideDownCake 14, $(PLATFORM_VERSION)))
ifeq (,$(filter $(PRODUCT_NAME), msmnile_au msmnile_au_s_u sm6150_au msmnile_gvmq))
include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.AMSIPC@1.0-service
LOCAL_VENDOR_MODULE        := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER         := qti
LOCAL_SRC_FILES := service.cpp

LOCAL_INIT_RC := vendor.qti.hardware.AMSIPC@1.0-service.rc
LOCAL_VINTF_FRAGMENTS := vendor.qti.hardware.AMSIPC@1.0-service.xml
LOCAL_C_INCLUDES = $(TOP)/vendor/qcom/proprietary/mm-audio-auto/ams/ams_core/inc/
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-audio-auto/ams/ams_osal/inc
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-audio-auto/ams/ipc/HwBinder/interfaces/ams_ipc/1.0/default/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar/ar_osal
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar/gpr

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX), msmnile_au)
LOCAL_CFLAGS += -DPLATFORM_MSMNILE_AU
endif

LOCAL_SHARED_LIBRARIES := liblog \
                        libcutils \
                        libdl \
                        libbase \
                        libutils \
                        libhardware \
                        libhidlbase \
                        libamscore \
                        vendor.qti.hardware.AMSIPC@1.0-impl \
                        vendor.qti.hardware.AMSIPC@1.0

LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)

ifneq (,$(filter $(PRODUCT_NAME), msmnile_gvmq msmnile_au))
include $(CLEAR_VARS)
LOCAL_MODULE       := init.qti.AMSIPC.sh
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_EXECUTABLES)
include $(BUILD_PREBUILT)
endif
endif
#other than Android U
else
$(warning "#### SHALINI Other than Android U, include AMS####")
include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.AMSIPC@1.0-service
LOCAL_VENDOR_MODULE        := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER         := qti
LOCAL_SRC_FILES := service.cpp

LOCAL_INIT_RC := vendor.qti.hardware.AMSIPC@1.0-service.rc
LOCAL_VINTF_FRAGMENTS := vendor.qti.hardware.AMSIPC@1.0-service.xml
LOCAL_C_INCLUDES = $(TOP)/vendor/qcom/proprietary/mm-audio-auto/ams/ams_core/inc/
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-audio-auto/ams/ams_osal/inc
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-audio-auto/ams/ipc/HwBinder/interfaces/ams_ipc/1.0/default/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar/ar_osal
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar/gpr

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX), msmnile_au)
LOCAL_CFLAGS += -DPLATFORM_MSMNILE_AU
endif

LOCAL_SHARED_LIBRARIES := liblog \
                        libcutils \
                        libdl \
                        libbase \
                        libutils \
                        libhardware \
                        libhidlbase \
                        libamscore \
                        vendor.qti.hardware.AMSIPC@1.0-impl \
                        vendor.qti.hardware.AMSIPC@1.0

LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)

ifneq (,$(filter $(PRODUCT_NAME), msmnile_gvmq msmnile_au))
include $(CLEAR_VARS)
LOCAL_MODULE       := init.qti.AMSIPC.sh
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_EXECUTABLES)
include $(BUILD_PREBUILT)
endif
endif
