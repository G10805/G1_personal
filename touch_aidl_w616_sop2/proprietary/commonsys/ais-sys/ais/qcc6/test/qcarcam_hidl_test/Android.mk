#======================================================================
# qcarcam_hidl_test - system app
#======================================================================
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_LDFLAGS :=

LOCAL_SRC_FILES:= src/qcarcam_test.cpp

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

LOCAL_CFLAGS :=-Werror \
	-DC2D_DISABLED \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers

LOCAL_HEADER_LIBRARIES := libmmosal_headers

LOCAL_SHARED_LIBRARIES:= libais_hidl_client_qcc6 libais_test_util_qcc6 \
	libmmosal liblog \
	libutils

LOCAL_MODULE:= qcarcam_hidl_test_qcc6

LOCAL_PRELINK_MODULE:= false

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

ifneq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
include $(BUILD_EXECUTABLE)
endif

#======================================================================
# qcarcam_aidl_test - system app
#======================================================================
include $(CLEAR_VARS)

LOCAL_LDFLAGS :=

LOCAL_SRC_FILES:= src/qcarcam_test.cpp

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

LOCAL_CFLAGS :=-Werror \
        -DC2D_DISABLED \
        -Wno-unused-parameter \
        -Wno-missing-field-initializers

LOCAL_HEADER_LIBRARIES := libmmosal_headers

LOCAL_SHARED_LIBRARIES:= libais_aidl_client_qcc6 libais_test_util_qcc6 \
        libmmosal liblog \
        libutils

ifeq ($(AIS_BUILD_WITH_CPMS_SUPPORT),true)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../hal/power/inc/
LOCAL_SHARED_LIBRARIES += libais_sys_power
LOCAL_CFLAGS += -DCAMERA_CPMS_SUPPORT
endif

LOCAL_MODULE:= qcarcam_aidl_test_qcc6

LOCAL_PRELINK_MODULE:= false

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

ifneq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
include $(BUILD_EXECUTABLE)
endif
