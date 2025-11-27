# SPDX-License-Identifier: MediaTekProprietary
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libmnl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES_arm := libs/android/libmnl.so
LOCAL_SRC_FILES_arm64 := libs64/android/libmnl.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MULTILIB := first
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libhotstill
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_SRC_FILES_arm := libs/android/libhotstill.a
LOCAL_SRC_FILES_arm64 := libs64/android/libhotstill.a
LOCAL_MODULE_SUFFIX := .a
LOCAL_MULTILIB := first
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsupl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_SRC_FILES_arm := libs/android/libsupl.a
LOCAL_SRC_FILES_arm64 := libs64/android/libsupl.a
LOCAL_MODULE_SUFFIX := .a
LOCAL_MULTILIB := first
include $(BUILD_PREBUILT)
