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
  $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/display \
  $(LOCAL_PATH)/../../API/inc

v4l2_src_files := \
  arc/cached_frame.cpp \
  arc/exif_utils.cpp \
  arc/frame_buffer.cpp \
  arc/image_processor.cpp \
  arc/jpeg_compressor.cpp \
  camera.cpp \
  capture_request.cpp \
  format_metadata_factory.cpp \
  metadata/boottime_state_delegate.cpp \
  metadata/enum_converter.cpp \
  metadata/metadata.cpp \
  metadata/metadata_reader.cpp \
  request_tracker.cpp \
  static_properties.cpp \
  stream_format.cpp \
  v4l2_camera.cpp \
  v4l2_camera_hal.cpp \
  v4l2_metadata_factory.cpp \
  v4l2_wrapper.cpp \
  config_port_metadata.cpp \

v4l2_test_files := \
  format_metadata_factory_test.cpp \
  metadata/control_test.cpp \
  metadata/default_option_delegate_test.cpp \
  metadata/enum_converter_test.cpp \
  metadata/ignored_control_delegate_test.cpp \
  metadata/map_converter_test.cpp \
  metadata/menu_control_options_test.cpp \
  metadata/metadata_reader_test.cpp \
  metadata/metadata_test.cpp \
  metadata/no_effect_control_delegate_test.cpp \
  metadata/partial_metadata_factory_test.cpp \
  metadata/property_test.cpp \
  metadata/ranged_converter_test.cpp \
  metadata/slider_control_options_test.cpp \
  metadata/state_test.cpp \
  metadata/tagged_control_delegate_test.cpp \
  metadata/tagged_control_options_test.cpp \
  metadata/v4l2_control_delegate_test.cpp \
  request_tracker_test.cpp \
  static_properties_test.cpp \

# V4L2 Camera HAL.
# ==============================================================================
include $(CLEAR_VARS)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := camera.v4l2.qcc6
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_CFLAGS += $(v4l2_cflags) -DFPS_PRINT
ifeq ($(PLATFORM_VERSION),$(filter T Tiramisu 13 U UpsideDownCake 14, $(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DANDROID_T_ABOVE
endif
ifeq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14, $(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DAIDL_ANDROID_API
v4l2_c_includes += $(LOCAL_PATH)/../include
endif
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
    vendor.qti.hardware.display.mapper@4.0 libjsoncpp
endif
LOCAL_HEADER_LIBRARIES += display_intf_headers
LOCAL_STATIC_LIBRARIES := \
  android.hardware.camera.common@1.0-helper \

LOCAL_C_INCLUDES += $(v4l2_c_includes) \

#LOCAL_CFLAGS += -DENABLE_LOGV
LOCAL_CFLAGS += -DPRODUCT_MODEL="$(PRODUCT_MODEL)"
LOCAL_CFLAGS += -DPRODUCT_MANUFACTURER="$(PRODUCT_MANUFACTURER)"
LOCAL_SRC_FILES := $(v4l2_src_files)
include $(BUILD_SHARED_LIBRARY)

#V4L2 Automotive camera config file
include $(CLEAR_VARS)
LOCAL_MODULE := config_v4l2_camera_hal_qcc6.json
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin
LOCAL_SRC_FILES := config_port_metadata.json
include $(BUILD_PREBUILT)

## Unit tests for V4L2 Camera HAL.
## ==============================================================================
#include $(CLEAR_VARS)
#LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE := camera.v4l2_test_qcc6
#LOCAL_CFLAGS += $(v4l2_cflags)
#LOCAL_SHARED_LIBRARIES := $(v4l2_shared_libs) \
#
#LOCAL_STATIC_LIBRARIES := \
#  libgmock \
#
##gralloc_priv.h location moved in Android Q
#ifeq ($(PLATFORM_VERSION), 10)
#LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/opensource/commonsys-intf/display/gralloc/ \
#  $(TOP)/frameworks/native/include \
#  $(TARGET_OUT_HEADERS)/common/inc
#LOCAL_CFLAGS += -DANDROID_Q_AOSP -DTESTUTIL_VENDOR_LIB
#LOCAL_SHARED_LIBRARIES += libbinder
#endif
#
#LOCAL_C_INCLUDES += $(v4l2_c_includes) \
#
#LOCAL_SRC_FILES := \
#  $(v4l2_src_files) \
#  $(v4l2_test_files) \
#
#ifeq ($(PLATFORM_VERSION), 11)
#include $(BUILD_NATIVE_TEST)
#endif
