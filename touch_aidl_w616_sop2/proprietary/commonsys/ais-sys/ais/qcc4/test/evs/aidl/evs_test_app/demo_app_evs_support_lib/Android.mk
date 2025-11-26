LOCAL_PATH:= $(call my-dir)

##################################
include $(CLEAR_VARS)

LOCAL_SRC_FILES +=evs_app_support_lib.cpp

LOCAL_SHARED_LIBRARIES := \
    libui \
    liblog \
    android.hardware.automotive.evs@1.0 \
    android.hardware.automotive.vehicle@2.0 \
    libevssupport

LOCAL_INIT_RC := evs_qcom_app_support_lib.rc

LOCAL_MODULE := evs_qcom_app_support_lib
ifneq ( ,$(filter 12 S ,$(PLATFORM_VERSION)))
LOCAL_C_INCLUDES += $(TOP)/packages/services/Car/cpp/evs/support_library
else
LOCAL_C_INCLUDES += $(TOP)/packages/services/Car/evs/support_library
endif

LOCAL_MODULE_TAGS := optional
LOCAL_STRIP_MODULE := keep_symbols

LOCAL_CFLAGS := -DLOG_TAG=\"EvsAppSupportLib\"
LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES
LOCAL_CFLAGS += -Wall -Werror -Wunused -Wunreachable-code

include $(BUILD_EXECUTABLE)
