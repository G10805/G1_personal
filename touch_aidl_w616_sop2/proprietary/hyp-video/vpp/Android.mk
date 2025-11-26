LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#               Common definitions
# ---------------------------------------------------------------------------------

libhyp-vpp-fe-def = -g -O3
libhyp-vpp-fe-def += -Werror
libhyp-vpp-fe-def += -D_ANDROID_

# Common Includes
libhyp-vpp-fe-inc          := $(LOCAL_PATH)/common/inc

libhyp-vpp-fe-inc          += $(TOP)/system/core/libutils/include
libhyp-vpp-fe-inc          += $(TOP)/system/core/include

libhyp-vpp-fe-inc          += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
libhyp-vpp-fe-inc          += $(TARGET_OUT_HEADERS)/mm-hab

libhyp-vpp-fe-inc          += $(TOP)/vendor/qcom/proprietary/mm-osal/inc
libhyp-vpp-fe-inc          += $(TOP)/vendor/qcom/proprietary/commonsys-intf/mm-osal/inc


# Common Dependencies
libhyp-vpp-fe-add-dep := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr


# ---------------------------------------------------------------------------------
#           Make the Shared library (libhyp-vpp-fe)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE                    := libhyp_vpp_fe
LOCAL_MODULE_TAGS               := optional
LOCAL_CFLAGS                    := $(libhyp-vpp-fe-def)
LOCAL_C_INCLUDES                += $(libhyp-vpp-fe-inc)
LOCAL_ADDITIONAL_DEPENDENCIES   := $(libhyp-vpp-fe-add-dep)

LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  += liblog libcutils
ifneq ($(PLATFORM_VERSION), $(filter $(PLATFORM_VERSION),10 Q 11 R 12 S 13 T))
LOCAL_SHARED_LIBRARIES  += libmmosal_vendor
else
LOCAL_SHARED_LIBRARIES  += libmmosal
endif

LOCAL_HEADER_LIBRARIES := \
        libutils_headers \
        qti_video_kernel_uapi \
        libvppheaders \
        libmm-hab_headers

LOCAL_SRC_FILES         := common/src/hyp_vpp_buf_manager.cpp
LOCAL_SRC_FILES         += common/src/hyp_vpp_queue_util.cpp
LOCAL_SRC_FILES         += common/src/hyp_vpp_fe.cpp
LOCAL_SRC_FILES         += common/src/hyp_vpp_fe_translation.cpp
LOCAL_SRC_FILES         += fe/src/vpp_fe.cpp

LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#           Make the test binary (hyp_vpp_test)
# ---------------------------------------------------------------------------------
#
include $(CLEAR_VARS)

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE             := hyp_vpp_test
LOCAL_C_INCLUDES         += $(hyp-vpp-test-inc)
LOCAL_C_INCLUDES         += $(TOP)/external/googletest/googlemock/include

LOCAL_SRC_FILES          := test/test_vpp_main.cpp

LOCAL_HEADER_LIBRARIES := \
        libutils_headers \
        qti_video_kernel_uapi \
        libvppheaders

LOCAL_SHARED_LIBRARIES  := libhyp_vpp_fe
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),4.14 4.19 4.9 5.4))
LOCAL_SHARED_LIBRARIES  += libdmabufheap
endif

include $(BUILD_NATIVE_TEST)

# ---------------------------------------------------------------------------------
#           Make the test binary (vpp_ais_test)
# ---------------------------------------------------------------------------------
#
include $(CLEAR_VARS)

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE             := vpp_ais_test

LOCAL_SRC_FILES          := test/aistest.cpp

LOCAL_HEADER_LIBRARIES := \
        libutils_headers \
        qti_video_kernel_uapi \
        libvppheaders

LOCAL_SHARED_LIBRARIES  := libhyp_vpp_fe
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),4.14 4.19 4.9 5.4))
LOCAL_SHARED_LIBRARIES  += libdmabufheap
endif

include $(BUILD_NATIVE_TEST)

# ---------------------------------------------------------------------------------
#                END
# ---------------------------------------------------------------------------------
