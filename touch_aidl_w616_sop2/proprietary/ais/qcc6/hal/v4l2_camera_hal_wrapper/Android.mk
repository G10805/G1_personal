#
# Copyright 2016 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

ifeq ($(ENABLE_HYP), true)
ifeq ($(PLATFORM_VERSION),$(filter T Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_PATH := $(call my-dir)

v4l2_shared_libs := \
  libbase \
  libcamera_metadata \
  libcutils \
  libexif \
  libhardware \
  liblog \
  libsync \
  libutils \
  libyuv \
  libjpeg \
  libC2D2 \
  libui

v4l2_cflags := -fno-short-enums -Wall -Wno-error -Wextra -DHAVE_JPEG

v4l2_c_includes := $(call include-path-for, camera) \
  external/libyuv/files/include \
  $(TARGET_OUT_HEADERS)/adreno \
  $(TOP)/frameworks/native/libs/binder/include \
  $(TOP)/vendor/qcom/proprietary/ais/qcc6/Common/inc \
  $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/display \
  $(LOCAL_PATH)/../../API/inc

v4l2_src_files := v4l2_camera_hal_wrapper.cpp

ifeq ($(AIS_BUILD_WITH_CPMS_SUPPORT),true)
v4l2_shared_libs += android.frameworks.automotive.powerpolicy-V1-ndk
v4l2_c_includes += $(TOP)/vendor/qcom/proprietary/ais/qcc6/hal/power/inc
endif

# V4L2 Camera HAL.
# ==============================================================================
include $(CLEAR_VARS)
LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE := camera.v4l2

LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_CFLAGS += $(v4l2_cflags) -DFPS_PRINT
LOCAL_SHARED_LIBRARIES := $(v4l2_shared_libs)

#gralloc_priv.h location moved in Android Q
ifeq ($(PLATFORM_VERSION), 10)
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/opensource/commonsys-intf/display/gralloc/ \
  $(TOP)/frameworks/native/include \
  $(TARGET_OUT_HEADERS)/common/inc
LOCAL_CFLAGS += -DANDROID_Q_AOSP -DTESTUTIL_VENDOR_LIB
LOCAL_SHARED_LIBRARIES += libbinder
else
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/display

LOCAL_SHARED_LIBRARIES += libbinder \
    libhidlbase libgralloctypes android.hardware.graphics.mapper@4.0 libgralloc.qti \
    vendor.qti.hardware.display.mapper@4.0
endif
LOCAL_HEADER_LIBRARIES += display_intf_headers
LOCAL_STATIC_LIBRARIES := \
  android.hardware.camera.common@1.0-helper \

ifeq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14, $(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DAIDL_ANDROID_API
v4l2_c_includes += $(LOCAL_PATH)/../include
endif

LOCAL_C_INCLUDES += $(v4l2_c_includes) \

#LOCAL_CFLAGS += -DENABLE_LOGV
LOCAL_CFLAGS += -DPRODUCT_MODEL="$(PRODUCT_MODEL)"
LOCAL_CFLAGS += -DQCC_SOC_OVERRIDE

LOCAL_SRC_FILES := $(v4l2_src_files)
include $(BUILD_SHARED_LIBRARY)
endif #Android 13/14
endif #currently, we compile this only if ENABLE_HYP is TRUE
