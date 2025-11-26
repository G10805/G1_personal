ifneq ($(AUDIO_USE_STUB_HAL),true)
ifeq ($(call is-board-platform-in-list,msm8909 msm8996 msm8937 msm8953 msm8998 apq8098_latv sdm660 sdm845 sdm670 sdm710 qcs605 sdmshrike msmnile gen4 $(MSMSTEPPE) $(TRINKET) qcs405 kona lahaina holi lito atoll bengal),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

libaudcal-def := -g -O3
libaudcal-def += -D_ANDROID_
libaudcal-def += -D_ENABLE_QC_MSG_LOG_

libaudcal-inc     := $(TARGET_OUT_HEADERS)/diag/include
libaudcal-inc     += $(TARGET_OUT_HEADERS)/common/inc
libaudcal-def += -mllvm -arm-assume-misaligned-load-store=true
# ---------------------------------------------------------------------------------
#             Include files
# ---------------------------------------------------------------------------------

libaudcal-inc     += $(LOCAL_PATH)/acdb/inc
libaudcal-inc     += $(LOCAL_PATH)/acdb/src
libaudcal-inc     += $(LOCAL_PATH)/acdb_hlos/inc
libaudcal-inc     += $(LOCAL_PATH)/acdb_hlos/src
libaudcal-inc     += $(LOCAL_PATH)/acph/inc
libaudcal-inc     += $(LOCAL_PATH)/acph/src
libaudcal-inc     += $(LOCAL_PATH)/acph_online/inc
libaudcal-inc     += $(LOCAL_PATH)/acph_online/src
libaudcal-inc     += $(LOCAL_PATH)/actp/inc
libaudcal-inc     += $(LOCAL_PATH)/actp/src
libaudcal-inc     += $(LOCAL_PATH)/audtp/inc
libaudcal-inc     += $(LOCAL_PATH)/audtp/src

# ---------------------------------------------------------------------------------
#             Make the Shared library (libaudcal)
# ---------------------------------------------------------------------------------


LOCAL_MODULE            := libaudcal
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(libaudcal-def)
LOCAL_C_INCLUDES        := $(libaudcal-inc)
LOCAL_C_INCLUDES	+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES	+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/audio

LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libutils liblog
LOCAL_SHARED_LIBRARIES  += libdiag

LOCAL_SRC_FILES         := acdb/src/acdb.c
LOCAL_SRC_FILES         += acdb/src/acdb_command.c
LOCAL_SRC_FILES         += acdb/src/acdb_data_mgr.c
LOCAL_SRC_FILES         += acdb/src/acdb_delta_file_mgr.c
LOCAL_SRC_FILES         += acdb/src/acdb_delta_parser.c
LOCAL_SRC_FILES         += acdb/src/acdb_file_mgr.c
LOCAL_SRC_FILES         += acdb/src/acdb_init.c
LOCAL_SRC_FILES         += acdb/src/acdb_linked_list.c
LOCAL_SRC_FILES         += acdb/src/acdb_parser.c
LOCAL_SRC_FILES         += acdb/src/acdb_translation.c
LOCAL_SRC_FILES         += acdb/src/acdb_utility.c
LOCAL_SRC_FILES         += acdb_hlos/src/acdb_init_utility.c
LOCAL_SRC_FILES         += acdb/src/acdb_instance_override.c
LOCAL_SRC_FILES         += acdb/src/acdb_new_linked_list.c
LOCAL_SRC_FILES         += acph/src/acph.c
LOCAL_SRC_FILES         += acph_online/src/acph_online.c
LOCAL_SRC_FILES         += actp/src/actp.c
LOCAL_SRC_FILES         += audtp/src/audtp.c

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_GCOV)),true)
LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_CPPFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_STATIC_LIBRARIES += libprofile_rt
endif

LOCAL_HEADER_LIBRARIES := vendor_common_inc

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(call is-board-platform-in-list,kona lahaina holi),true)
LOCAL_SANITIZE := integer_overflow
endif
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libaudcal_headers
LOCAL_EXPORT_C_INCLUDE_DIRS   := $(LOCAL_PATH)/acdb/inc/
LOCAL_EXPORT_C_INCLUDE_DIRS   += $(LOCAL_PATH)/acdb_hlos/inc/
LOCAL_EXPORT_C_INCLUDE_DIRS   += $(LOCAL_PATH)/acph/inc/
LOCAL_VENDOR_MODULE := true
include $(BUILD_HEADER_LIBRARY)

# ---------------------------------------------------------------------------------
#             Make the apps-test (mm-audio-send-cal)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE            := mm-audio-send-cal
LOCAL_MODULE_TAGS       := optional
LOCAL_C_INCLUDES        := $(sendcal-inc)
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libacdbloader
LOCAL_PROPRIETARY_MODULE := true

LOCAL_HEADER_LIBRARIES := libacdbloader_headers

LOCAL_SRC_FILES         := test/sendcal.c

ifeq ($(call is-board-platform-in-list,kona lahaina holi),true)
LOCAL_SANITIZE := integer_overflow
endif
include $(BUILD_EXECUTABLE)

# ---------------------------------------------------------------------------------
#             Populate ACDB data files
# ---------------------------------------------------------------------------------
include $(LOCAL_PATH)/acdbdata/Android.mk

endif #BUILD_TINY_ANDROID
endif
endif # AUDIO_USE_STUB_HAL
# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------
