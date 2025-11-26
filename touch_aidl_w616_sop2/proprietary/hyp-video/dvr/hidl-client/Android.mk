LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#               Common definitons
# ---------------------------------------------------------------------------------

hyp-dvr-hidl-client-def = -g -O3
hyp-dvr-hidl-client-def += -Werror
hyp-dvr-hidl-client-def += -D_ANDROID_

# Common Includes
hyp-dvr-hidl-client-inc := $(LOCAL_PATH)/inc
hyp-dvr-hidl-client-inc += $(LOCAL_PATH)/../common/inc
hyp-dvr-hidl-client-inc += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
hyp-dvr-hidl-client-inc += $(TOP)/vendor/qcom/proprietary/commonsys-intf/mm-osal/inc
hyp-dvr-hidl-client-inc += $(TARGET_OUT_HEADERS)/mm-hab
hyp-dvr-hidl-client-inc += $(TOP)/vendor/qcom/opensource/commonsys-intf/display/gralloc

hyp-dvr-hidl-client-add-dep := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

# ---------------------------------------------------------------------------------
#           Make the executable (hyp_dvr_hidl_client)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE                    := hyp_dvr_hidl_client
LOCAL_MODULE_TAGS               := optional
LOCAL_CFLAGS                    := $(hyp-dvr-hidl-client-def)
ifeq ($(PLATFORM_VERSION), 11)
LOCAL_CFLAGS                    += -DANDROID_R_AOSP
endif
LOCAL_C_INCLUDES                += $(hyp-dvr-hidl-client-inc)
LOCAL_ADDITIONAL_DEPENDENCIES   := $(hyp-dvr-hidl-client-add-dep)

LOCAL_SHARED_LIBRARIES  += liblog libcutils libmmosal libgui vendor.qti.dvr@1.0 libhidlbase libbinder libutils libui

LOCAL_SRC_FILES         := hyp_dvr_hidl_client.cpp

include $(BUILD_EXECUTABLE)


# ---------------------------------------------------------------------------------
#                END
# ---------------------------------------------------------------------------------

