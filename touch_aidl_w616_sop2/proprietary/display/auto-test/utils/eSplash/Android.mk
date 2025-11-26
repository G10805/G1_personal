LOCAL_PATH:= $(call my-dir)
# ---------------------------------------------------------------------------------
# 				Common definitons
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

# Common Includes

LOCAL_C_INCLUDES:= \
        $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
        $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/display \
        $(TOP)/external/libdrm \
        $(TOP)/system/core/include

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SRC_FILES:= esplash.cpp

LOCAL_SHARED_LIBRARIES := \
        libdrm liblog libselinux

LOCAL_CFLAGS              := -Wno-missing-field-initializers -Wall -Werror -fno-operator-names \
                             -Wno-unused-parameter

ifeq ($(BOARD_SUPPORTS_RAMDISK_EARLY_INIT), true)
LOCAL_MODULE_PATH         := $(TARGET_VENDOR_RAMDISK_OUT)/vendor_early_services/vendor/bin
LOCAL_LDFLAGS             := -Wl,-rpath,'/vendor_early_services/system/lib64/' -Wl,--dynamic-linker,/vendor_early_services/system/bin/bootstrap/linker64
endif

ifneq (,$(filter Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_HEADER_LIBRARIES    += qti_kernel_headers qti_display_kernel_headers device_kernel_headers
LOCAL_CFLAGS              += -D__MIN_ANDROID_VER_T__
endif

ifneq (,$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_CFLAGS              += -DSILENT_BOOT_MIN_ANDROID_VER_U
endif

# Enable KPIs to boot marker
LOCAL_CFLAGS += -DUSE_BOOT_MARKER

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := esplash
LOCAL_MODULE_OWNER               := qti
LOCAL_PROPRIETARY_MODULE         := true

include $(BUILD_EXECUTABLE)
