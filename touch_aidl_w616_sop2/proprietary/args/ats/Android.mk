LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#----------------------------------------------------------------------------
#                 Common definitons
#----------------------------------------------------------------------------

ats-def += -D_ANDROID_

#----------------------------------------------------------------------------
#             Make the Shared library (libar-acdb)
#----------------------------------------------------------------------------

#LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc

LOCAL_CFLAGS := $(ats-def)

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/rtc/inc \
    $(LOCAL_PATH)/online/inc/ \
    $(LOCAL_PATH)/mcs/inc/ \
    $(LOCAL_PATH)/mcs/api/ \
    $(LOCAL_PATH)/fts/platform/inc/ \
    $(LOCAL_PATH)/tcpip/platform/inc/ \
    $(LOCAL_PATH)/diag/Platform/actp/inc/ \
    $(LOCAL_PATH)/diag/Platform/audtp/inc/ \
    $(LOCAL_PATH)/adie/inc \
    $(LOCAL_PATH)/adie/api \
    $(LOCAL_PATH)/api

LOCAL_SRC_FILES := \
    tcpip/platform/src/ats_server.cpp \
    src/ats.c \
    src/ats_command.c \
    rtc/src/ats_rtc.c \
    online/src/ats_online.c \
    mcs/src/ats_mcs.c \
    mcs/platform/lx/mcs.c \
    fts/platform/src/ats_fts.c \
    diag/Platform/actp/src/actp.c \
    diag/Platform/audtp/src/audtp.c \
    adie/src/ats_adie_rtc.c \
    adie/platform/lx/adie_rtc.c

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include

LOCAL_MODULE := libats
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

LOCAL_HEADER_LIBRARIES := \
    libcutils_headers \
    libutils_headers \
    libdiag_headers \
    vendor_common_inc

LOCAL_SHARED_LIBRARIES := \
    liblx-osal\
    libutils\
    libcutils \
    libdiag \
    libar-gsl\
    libar-acdb

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/api
LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)/inc
LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)/adie/api
LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)/mcs/api

include $(BUILD_SHARED_LIBRARY)
