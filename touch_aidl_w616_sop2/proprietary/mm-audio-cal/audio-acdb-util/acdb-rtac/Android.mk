ifneq ($(AUDIO_USE_STUB_HAL), true)
ifeq ($(call is-board-platform-in-list,msm8909 msm8996 msm8937 msm8953 msm8998 apq8098_latv sdm660 sdm845 sdm670 sdm710 qcs605 sdmshrike msmnile gen4  $(MSMSTEPPE) $(TRINKET) kona lahaina holi lito atoll bengal),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

libacdbrtac-def := -g -O3
libacdbrtac-def += -D_ANDROID_
libacdbrtac-def += -D_ENABLE_QC_MSG_LOG_
libacdbrtac-def += -mllvm -arm-assume-misaligned-load-store=true
# ---------------------------------------------------------------------------------
#             Make the Shared library (libacdbrtac)
# ---------------------------------------------------------------------------------

libacdbrtac-inc     := $(LOCAL_PATH)/inc
libacdbrtac-inc     += $(LOCAL_PATH)/src

LOCAL_MODULE            := libacdbrtac
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(libacdbrtac-def)
LOCAL_C_INCLUDES        := $(libacdbrtac-inc)
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES        += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES        += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/audio
LOCAL_C_INCLUDES 	+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/techpack/audio/include

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DLKM)),true)
  LOCAL_HEADER_LIBRARIES := audio_kernel_headers
  ifneq ($(BOARD_OPENSOURCE_DIR), )
    LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/$(BOARD_OPENSOURCE_DIR)/audio-kernel/include
  else
    LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/opensource/audio-kernel/include
  endif # BOARD_OPENSOURCE_DIR
endif

LOCAL_HEADER_LIBRARIES += libaudcal_headers

ifeq ($(ENABLE_AUDIO_LEGACY_TECHPACK),true)
LOCAL_HEADER_LIBRARIES += qti_legacy_audio_kernel_uapi
endif

ifeq ($(TARGET_BOARD_AUTO),true)
LOCAL_HEADER_LIBRARIES += qti_audio_kernel_uapi
endif

LOCAL_HEADER_LIBRARIES += vendor_common_inc
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libcutils libutils liblog libaudcal

LOCAL_EXPORT_C_INCLUDE_DIRS   := $(LOCAL_PATH)/inc/

LOCAL_SRC_FILES         := src/acdb-rtac.c

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_GCOV)),true)
LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_CPPFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_STATIC_LIBRARIES += libprofile_rt
endif

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(call is-board-platform-in-list,kona lahaina holi),true)
LOCAL_SANITIZE := integer_overflow
endif
include $(BUILD_SHARED_LIBRARY)

endif
endif # is-board-platform-in-list
endif # AUDIO_USE_STUB_HAL
# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------
