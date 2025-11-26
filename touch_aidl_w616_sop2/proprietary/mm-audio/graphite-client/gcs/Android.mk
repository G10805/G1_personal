ifneq ($(AUDIO_USE_STUB_HAL), true)
ifeq ($(call is-board-platform-in-list, msm8998 apq8098_latv sdm660 sdm845 sdm710 msm8953 msm8937 qcs605 msm8909 sdmshrike msmnile gen4 $(MSMSTEPPE) $(TRINKET) kona lahaina holi lito bengal atoll),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#----------------------------------------------------------------------------
#                 Common definitons
#----------------------------------------------------------------------------

gcs-def += -D_ANDROID_

#----------------------------------------------------------------------------
#             Make the Shared library (libgcs)
#----------------------------------------------------------------------------

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-audio/graphite-client/osal
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-audio/graphite-client/cal-wrapper
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-audio/graphite-client/ipc
LOCAL_CFLAGS     := $(gcs-def)

ifeq ($(call is-platform-sdk-version-at-least,28),true)   #Android P and above
LOCAL_HEADER_LIBRARIES := libutils_headers
endif

LOCAL_HEADER_LIBRARIES += libgcs-osal_headers

LOCAL_SHARED_LIBRARIES := libgcs-osal \
                          libgcs-calwrapper \
                          libgcs-ipc \
                          liblog

LOCAL_SRC_FILES        := src/gcs.c

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_GCOV)),true)
LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_CPPFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_STATIC_LIBRARIES += libprofile_rt
endif

LOCAL_MODULE               := libgcs
LOCAL_MODULE_OWNER         := qti
LOCAL_MODULE_TAGS          := optional
LOCAL_PROPRIETARY_MODULE   := true

ifeq ($(call is-board-platform-in-list,kona lahaina holi),true)
LOCAL_SANITIZE := integer_overflow
endif
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libgcs_headers
LOCAL_EXPORT_C_INCLUDE_DIRS   := $(LOCAL_PATH)/inc/
LOCAL_VENDOR_MODULE := true
include $(BUILD_HEADER_LIBRARY)

endif # BUILD_TINY_ANDROID
endif # is-board-platform-in-list
endif
