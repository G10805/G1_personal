ifneq ($(AUDIO_USE_STUB_HAL), true)
ifeq ($(call is-board-platform-in-list, msm8998 apq8098_latv sdm660 sdm845 sdm710 msm8953 msm8937 qcs605 msm8909 sdmshrike msmnile gen4 $(MSMSTEPPE) $(TRINKET) kona lahaina holi lito bengal atoll),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#----------------------------------------------------------------------------
#                 Common definitons
#----------------------------------------------------------------------------

osal-def += -D_ANDROID_

#----------------------------------------------------------------------------
#             Make the Shared library (libgcs-osal)
#----------------------------------------------------------------------------

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_CFLAGS := $(osal-def)
ifeq ($(call is-platform-sdk-version-at-least,28),true)   #Android P and above
LOCAL_HEADER_LIBRARIES := libutils_headers
endif

LOCAL_SRC_FILES := src/osal_lock.c \
                   src/osal_mem.c \
                   src/osal_cond.c \
                   src/osal_thread.c \
                   src/osal_dev.c

LOCAL_SHARED_LIBRARIES := liblog
ifneq ($(filter P% p%,$(TARGET_PLATFORM_VERSION)),)   # ANDROID_P
LOCAL_CFLAGS += -DVENDOR_COMPLIANCE
LOCAL_SHARED_LIBRARIES += libcutils
endif

LOCAL_MODULE := libgcs-osal
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_GCOV)),true)
LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_CPPFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_STATIC_LIBRARIES += libprofile_rt
endif

ifeq ($(call is-board-platform-in-list,kona lahaina holi),true)
LOCAL_SANITIZE := integer_overflow
endif
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libgcs-osal_headers
LOCAL_EXPORT_C_INCLUDE_DIRS   := $(LOCAL_PATH)/inc/
LOCAL_VENDOR_MODULE := true
include $(BUILD_HEADER_LIBRARY)

endif # BUILD_TINY_ANDROID
endif # is-board-platform-in-list
endif
