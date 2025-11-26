LOCAL_PATH:= $(call my-dir)
# ---------------------------------------------------------------------------------
# 				Common definitons
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

# Common Includes

LOCAL_C_INCLUDES:= \
        $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
        $(TOP)/external/libdrm \
        $(TOP)/system/core/include \
        $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/display

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SRC_FILES:= test-app-recv.c

LOCAL_SHARED_LIBRARIES := \
        libdrm liblog

LOCAL_CFLAGS              := -Wno-missing-field-initializers -Wall -Werror -fno-operator-names \
                             -Wno-unused-parameter

ifneq (,$(filter Tiramisu 13 U 14 UpsideDownCake V 15 VanillaIceCream, $(PLATFORM_VERSION)))
LOCAL_HEADER_LIBRARIES    += qti_kernel_headers qti_display_kernel_headers device_kernel_headers
LOCAL_CFLAGS              += -D__MIN_ANDROID_VER_T__
endif

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := test-app
LOCAL_MODULE_OWNER               := qti
LOCAL_PROPRIETARY_MODULE         := true

include $(BUILD_EXECUTABLE)
