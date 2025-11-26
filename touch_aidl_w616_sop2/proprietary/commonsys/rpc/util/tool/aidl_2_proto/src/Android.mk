LOCAL_DIR_PATH:= $(call my-dir)

BUILD_AIDL_2_PROTO_LIB := true
BUILD_AIDL_2_PROTO_BIN := true
ifeq ($(BUILD_AIDL_2_PROTO_BIN), true)
ENABLE_LOG := true
endif

A2P_DIR := $(TOP)/vendor/qcom/proprietary/commonsys/rpc/util/tool/aidl_2_proto/src

COMMON_INCLUDES := \
    $(A2P_DIR) \
    $(A2P_DIR)/aidl/include \
    $(A2P_DIR)/common/include \
    $(A2P_DIR)/proto/include

#
#  liba2p.so
#
ifeq ($(BUILD_AIDL_2_PROTO_BIN), true)

include $(CLEAR_VARS)
LOCAL_PATH := $(LOCAL_DIR_PATH)
LOCAL_MODULE := liba2p

LOCAL_C_INCLUDES := \
    $(COMMON_INCLUDES)

LOCAL_CPPFLAGS := -Werror -Wall
LOCAL_CPPFLAGS += -fexceptions
ifeq ($(ENABLE_LOG), true)
LOCAL_CPPFLAGS += -DENABLE_LOG
endif

LOCAL_SRC_FILES := \
    aidl/src/aidl.cpp \
    aidl/src/aidl_util.cpp \
    common/src/arg_search.cpp \
    common/src/log.cpp \
    common/src/util.cpp \
    proto/src/proto.cpp \
    proto/src/proto_file.cpp \
    proto/src/proto_util.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libutils

LOCAL_STATIC_LIBRARIES := \
    libc++fs

LOCAL_EXPORT_C_INCLUDE_DIRS := \
    $(COMMON_INCLUDES)

include $(BUILD_SHARED_LIBRARY)
endif

#
#  a2p (aidl2proto bin)
#
ifeq ($(BUILD_AIDL_2_PROTO_BIN), true)

include $(CLEAR_VARS)
LOCAL_PATH := $(LOCAL_DIR_PATH)
LOCAL_MODULE := a2p

LOCAL_C_INCLUDES := \
    $(COMMON_INCLUDES)

LOCAL_SRC_FILES := \
    aidl_2_proto.cpp \
    aidl_2_proto_main.cpp

LOCAL_SHARED_LIBRARIES := \
    liba2p \
    liblog \
    libcutils \
    libutils

include $(BUILD_EXECUTABLE)
endif