LOCAL_PATH:= $(call my-dir)
# ---------------------------------------------------------------------------------
# 				Common definitons
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

# Common Includes

LOCAL_C_INCLUDES:= \
        $(TOP)/system/memory/libion/include \
        $(TOP)/system/memory/libion/kernel-headers \
        $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
        $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/display \
        $(TOP)/external/libdrm \

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SRC_FILES:= drm-test.cpp

LOCAL_SHARED_LIBRARIES := \
        libutils libdrm libion

LOCAL_CFLAGS              := -Wno-missing-field-initializers -Wall -Werror -fno-operator-names \
                             -Wno-unused-parameter

ifneq (,$(filter Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_HEADER_LIBRARIES    += qti_kernel_headers qti_display_kernel_headers device_kernel_headers
LOCAL_CFLAGS              += -D__MIN_ANDROID_VER_T__
endif

ifeq ($(TARGET_BUILD_VARIANT),userdebug)
LOCAL_CFLAGS += -DDEBUG_BUILD
endif

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := drm-test

LOCAL_MODULE_OWNER               := qti
LOCAL_PROPRIETARY_MODULE         := true

include $(BUILD_EXECUTABLE)
