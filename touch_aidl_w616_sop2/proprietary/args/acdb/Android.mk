LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#----------------------------------------------------------------------------
#                 Common definitons
#----------------------------------------------------------------------------

acdb-def += -D_ANDROID_

#----------------------------------------------------------------------------
#             Make the Shared library (libar-acdb)
#----------------------------------------------------------------------------

#LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc

LOCAL_CFLAGS := $(acdb-def)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/api\
    $(LOCAL_PATH)/inc

LOCAL_SRC_FILES := \
    src/acdb.c \
    src/acdb_command.c \
    src/acdb_delta_file_mgr.c \
    src/acdb_delta_parser.c \
    src/acdb_file_mgr.c \
    src/acdb_init.c \
    src/acdb_init_utility.c \
    src/acdb_parser.c \
    src/acdb_utility.c\
    src/acdb_data_proc.c\
    src/acdb_heap.c

LOCAL_MODULE := libar-acdb
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

LOCAL_HEADER_LIBRARIES := \
    libcutils_headers \
    libutils_headers \
    vendor_common_inc

LOCAL_SHARED_LIBRARIES := \
    liblx-osal

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/api
LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)/inc

include $(BUILD_SHARED_LIBRARY)
