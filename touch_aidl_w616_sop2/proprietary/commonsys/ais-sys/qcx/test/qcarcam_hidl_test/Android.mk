ifneq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
#======================================================================
# qcarcam_hidl_test - system app
#======================================================================
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_LDFLAGS :=

LOCAL_SRC_FILES:= src/qcarcam_test.cpp src/qcarcam_test_menu.cpp

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../../API/inc \
	$(LOCAL_PATH)/../../API/vendor \
	$(LOCAL_PATH)/../../Common/inc \
	$(LOCAL_PATH)/../../CameraOSServices/CameraOSServices/inc \
	$(LOCAL_PATH)/../test_util/inc \
	$(TARGET_OUT_HEADERS)/mm-osal/include \
	$(TARGET_OUT_HEADERS)/common/inc

ifneq ($(TARGET_USES_GRALLOC1), true)
    LOCAL_C_INCLUDES  += $(TOP)/hardware/qcom/display/libgralloc
else
    LOCAL_C_INCLUDES  += $(TOP)/hardware/qcom/display/libgralloc1
endif #TARGET_USES_GRALLOC1

LOCAL_CFLAGS := -Wno-error \
	-DC2D_DISABLED \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers

LOCAL_HEADER_LIBRARIES := libmmosal_headers

LOCAL_SHARED_LIBRARIES:= libqcx_hidl_client libqcx_test_util \
	libmmosal liblog \
	libutils

LOCAL_MODULE:= qcarcam_hidl_test_qcx

LOCAL_PRELINK_MODULE:= false

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

ifneq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
include $(BUILD_EXECUTABLE)
endif

else
#======================================================================
# qcarcam_aidl_test - system app
#======================================================================
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_LDFLAGS :=

LOCAL_SRC_FILES:= src/qcarcam_test.cpp src/qcarcam_test_menu.cpp

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../../API/inc \
	$(LOCAL_PATH)/../../API/vendor \
	$(LOCAL_PATH)/../../Common/inc \
	$(LOCAL_PATH)/../../CameraOSServices/CameraOSServices/inc \
	$(LOCAL_PATH)/../test_util/inc \
	$(TARGET_OUT_HEADERS)/mm-osal/include \
	$(TARGET_OUT_HEADERS)/common/inc

ifneq ($(TARGET_USES_GRALLOC1), true)
    LOCAL_C_INCLUDES  += $(TOP)/hardware/qcom/display/libgralloc
else
    LOCAL_C_INCLUDES  += $(TOP)/hardware/qcom/display/libgralloc1
endif #TARGET_USES_GRALLOC1

LOCAL_CFLAGS := -Wno-error \
	-DC2D_DISABLED \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers

LOCAL_HEADER_LIBRARIES := libmmosal_headers

LOCAL_SHARED_LIBRARIES:= libqcx_aidl_client libqcx_test_util \
	libmmosal liblog \
	libutils

LOCAL_MODULE:= qcarcam_aidl_test_qcx

LOCAL_PRELINK_MODULE:= false

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

ifneq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
include $(BUILD_EXECUTABLE)
endif
endif
