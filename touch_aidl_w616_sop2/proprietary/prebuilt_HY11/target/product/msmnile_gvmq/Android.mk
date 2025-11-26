PREBUILT_PATH := $(call my-dir)
LOCAL_PATH := $(PREBUILT_PATH)
-include $(LOCAL_PATH)/interface_modules.mk

include $(CLEAR_VARS)
LOCAL_MODULE := libdrmMinimalfs
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/obj_arm/STATIC_LIBRARIES/libdrmMinimalfs_intermediates/libdrmMinimalfs.a
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libqsappsver
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/obj_arm/STATIC_LIBRARIES/libqsappsver_intermediates/libqsappsver.a
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libQSEEComAPIStatic
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/obj_arm/STATIC_LIBRARIES/libQSEEComAPIStatic_intermediates/libQSEEComAPIStatic.a
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := librpmbStatic
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/obj_arm/STATIC_LIBRARIES/librpmbStatic_intermediates/librpmbStatic.a
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libslimcommon
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/obj_arm/STATIC_LIBRARIES/libslimcommon_intermediates/libslimcommon.a
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libssdStatic
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/obj_arm/STATIC_LIBRARIES/libssdStatic_intermediates/libssdStatic.a
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/c2d2.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/c2dExt.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno/CL
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/CL/cl_egl.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno/CL
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/CL/cl_ext.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno/CL
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/CL/cl_ext_qcom.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno/CL
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/CL/cl_gl_ext.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno/CL
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/CL/cl_gl.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno/CL
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/CL/cl.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno/CL
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/CL/cl.hpp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno/CL
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/CL/cl_platform.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno/CL
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/CL/cl_version.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno/CL
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/CL/opencl.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno/EGL
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/EGL/eglext.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno/EGL
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/EGL/eglextQCOM.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno/EGL
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/EGL/egl.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno/EGL
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/EGL/eglplatform.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno/GLES2
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/GLES2/gl2ext.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno/GLES2
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/GLES2/gl2.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno/GLES2
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/GLES2/gl2platform.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno/GLES3
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/GLES3/gl3.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno/GLES3
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/GLES3/gl3platform.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),msmnile_gvmq)
ifeq ($(filter qssi qssi_au qssi_32 qssi_32go,$(TARGET_BOARD_PLATFORM)),)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := adreno/KHR
LOCAL_COPY_HEADERS := ../../.././target/product/msmnile_gvmq/obj/include/adreno/KHR/khrplatform.h
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
endif
endif

include $(CLEAR_VARS)
LOCAL_MODULE := libdrmMinimalfs
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/obj/STATIC_LIBRARIES/libdrmMinimalfs_intermediates/libdrmMinimalfs.a
LOCAL_MULTILIB := 64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libqsappsver
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/obj/STATIC_LIBRARIES/libqsappsver_intermediates/libqsappsver.a
LOCAL_MULTILIB := 64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libQSEEComAPIStatic
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/obj/STATIC_LIBRARIES/libQSEEComAPIStatic_intermediates/libQSEEComAPIStatic.a
LOCAL_MULTILIB := 64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := librpmbStatic
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/obj/STATIC_LIBRARIES/librpmbStatic_intermediates/librpmbStatic.a
LOCAL_MULTILIB := 64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libslimcommon
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/obj/STATIC_LIBRARIES/libslimcommon_intermediates/libslimcommon.a
LOCAL_MULTILIB := 64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libssdStatic
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/obj/STATIC_LIBRARIES/libssdStatic_intermediates/libssdStatic.a
LOCAL_MULTILIB := 64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := CACertService
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := APPS
LOCAL_CERTIFICATE := platform
LOCAL_MODULE_SUFFIX := .apk
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/app/CACertService/CACertService.apk
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/app
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := TrustZoneAccessService
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := APPS
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_MODULE_SUFFIX := .apk
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/app/TrustZoneAccessService/TrustZoneAccessService.apk
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/app
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := amfsservice
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libuhab liblog libutils libcutils libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/amfsservice
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := audioflacapp
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libFlacSwDec liblog libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/audioflacapp
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := edgnss-daemon
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libutils liblog libssl libgps.utils libcdfw_remote_api libpal_net_if libsys_info_cache libavservices_minijail libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/edgnss-daemon
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := engine-service
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libgps.utils libengineplugin liblog liblocdiagiface libutils libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/engine-service
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := fastcv.sh
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES :=
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/fastcv.sh
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := fastrpc_tests_apps
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libadsprpc liblog libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/fastrpc_tests_apps
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := fastrpc_tests_apps32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_32_BIT_ONLY := true
LOCAL_SHARED_LIBRARIES := libadsprpc liblog libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/fastrpc_tests_apps32
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := hdcp1prov
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libdiag liblog libhdcp1prov libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/hdcp1prov
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := hdcp2p2prov
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libdiag liblog libhdcp2p2prov libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/hdcp2p2prov
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

#include $(CLEAR_VARS)
#LOCAL_MODULE := android.hardware.automotive.vehicle@V1-qti-service
#LOCAL_MODULE_OWNER := qcom
#LOCAL_MODULE_TAGS := optional
#LOCAL_MODULE_CLASS := EXECUTABLES
#LOCAL_SHARED_LIBRARIES := libcutils libbase liblog libutils FakeVehicleHalValueGenerators libjsoncpp VehicleHalJsonConfigLoaderEnableTestProperties FakeUserHal DefaultVehicleHal VehicleHardware VehicleHalUtils libbinder_ndk libc++ libc libm libdl
#LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/hw/android.hardware.automotive.vehicle@V1-qti-service
#LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin/hw
#LOCAL_PROPRIETARY_MODULE := true
#include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.drm-service.playready-qti
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libion libcpion liblog libhidlbase libbase64 libbinder libutils libstagefright_foundation android.hardware.drm-V1-ndk android.hardware.common-V2-ndk libbase libcrypto libbinder_ndk libhwbinder libcutils libjsoncpp libpraidl libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/hw/android.hardware.drm-service.playready-qti
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin/hw
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.gatekeeper-service-qti
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libqtigatekeeper libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/hw/android.hardware.gatekeeper-service-qti
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin/hw
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.keymaster@4.0-service-qti
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := liblog libcutils libbase libutils libhardware libhidlbase android.hardware.keymaster@4.0 libqtikeymaster4 libcrypto libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/hw/android.hardware.keymaster@4.0-service-qti
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin/hw
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.security.keymint-service-qti
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := liblog libcutils libbase libutils libhardware libbinder_ndk libhidlbase android.hardware.keymaster@4.0 libqtikeymint libcrypto android.hardware.security.keymint-V3-ndk android.hardware.security.rkp-V3-ndk android.hardware.security.secureclock-V1-ndk android.hardware.security.sharedsecret-V1-ndk libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/hw/android.hardware.security.keymint-service-qti
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin/hw
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.display.color@1.0-service
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := liblog libcutils libbase libutils libbinder libhardware_legacy libhardware libhidlbase vendor.display.color@1.0 vendor.display.color@1.1 vendor.display.color@1.2 vendor.display.color@1.3 vendor.display.postproc@1.0 libsdm-disp-vndapis libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/hw/vendor.display.color@1.0-service
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin/hw
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.display.color-service
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := vendor.qti.hardware.display.postproc-V1-ndk liblog libcutils libbase libutils libbinder libbinder_ndk libsdm-disp-vndapis libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/hw/vendor.qti.hardware.display.color-service
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin/hw
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.qseecom@1.0-service
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := liblog libbase libutils libbinder_ndk libdmabufheap libQSEEComAPI vendor.qti.hardware.qseecom-V1-ndk vendor.qti.hardware.qseecom@1.0-impl libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/hw/vendor.qti.hardware.qseecom@1.0-service
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin/hw
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.qteeconnector@1.0-service
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libutils libbase libcutils liblog libQSEEComAPI libxml2 libbinder_ndk vendor.qti.hardware.qteeconnector-V1-ndk vendor.qti.hardware.qteeconnector@1.0-impl libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/hw/vendor.qti.hardware.qteeconnector@1.0-service
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin/hw
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

ifeq ($(wildcard vendor/qcom/proprietary/media),)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.media.c2@1.0-service
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libcodec2_vndk android.hardware.media.c2@1.0 libavservices_minijail libbinder libhidlbase libhidltransport libhwbinder libvndksupport libcodec2_hidl@1.0 libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/hw/vendor.qti.media.c2@1.0-service
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin/hw
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.psiclient@1.0-service
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libcutils libutils liblog libbinder_ndk libbase libhardware libhidlbase libpsi libpasrutils vendor.qti.memory.pasrmanager-V1-ndk vendor.qti.memory.pasrmanager@1.0 vendor.qti.memory.pasrmanager@1.1 android.hidl.allocator@1.0 android.hidl.memory@1.0 libhidlmemory libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/hw/vendor.qti.psiclient@1.0-service
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin/hw
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := ks
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libc libcutils libutils liblog libc++ libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/ks
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := lowi-server
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libcutils libgps.utils libloc_base_util libloc_mq_client liblog libnl libwpa_client liblowi_common libqmi_cci libqmi_common_so libdiag libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/lowi-server
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := lowi_test
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libcutils liblog liblowi_client libxml2 libgps.utils libloc_base_util libloc_mq_client liblowi_common libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/lowi_test
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := mlid
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libcutils liblowi_client libminksocket_vendor libloc_base_util liblowi_common libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/mlid
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := msm_irqbalance
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libcutils libutils liblog libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/msm_irqbalance
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := npu_launcher
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libutils liblog libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/npu_launcher
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := pm-proxy
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libperipheral_client libcutils liblog libmdmdetect libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/pm-proxy
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := pm-service
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libcutils libutils liblog libbinder libqmi_cci libqmi_common_so libqmi_encdec libqmi_csi libmdmdetect libperipheral_client libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/pm-service
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := qcom-system-daemon
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libcutils libutils liblog libdiag libsubsystem_control libavservices_minijail libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/qcom-system-daemon
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := qdcmss
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libcutils libutils libqservice libsdmutils libbinder liblog libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/qdcmss
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := qdss_qmi_helper
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libqmi_cci libqmi_common_so libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/qdss_qmi_helper
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := qguard
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libbase libcutils liblog libjsoncpp libprocessgroup libutils libbinder libbinder_ndk libui libhidlbase libdisplayconfig.qti android.hardware.graphics.allocator@4.0 android.hardware.graphics.mapper@4.0 android.hardware.graphics.common@1.2 libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/qguard
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := qlm-service
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libcutils libxml2 liblog libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/qlm-service
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := qwesd
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libqcbor libminksocket_vendor libminkdescriptor libcrypto libQSEEComAPI libcurl libqmi_csi liblog libutils libavservices_minijail libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/qwesd
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := StoreKeybox
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := liboemcrypto libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/StoreKeybox
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := sysprofiler_app
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libsysprofiler liblog libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/sysprofiler_app
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := testapp_diag_senddata
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libdiag liblog libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/testapp_diag_senddata
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := tftp_server
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libqsocket libqrtr liblog libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/tftp_server
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := xtra-daemon
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libutils libcutils libcrypto libssl libsqlite libgps.utils libloc_core libizat_core liblbs_core liblog libandroid_net libavservices_minijail libsys_info_cache libpal_net_if libbinder libbinder_ndk libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/xtra-daemon
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := xtwifi-client
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libcutils liblog libcrypto libz libsqlite libqmi_cci libqmi_common_so libgps.utils libloc_base_util libloc_mq_client liblowi_common libloc_core libizat_core liblbs_core libavservices_minijail libgdtap liblowi_client libc++ libc libm libdl
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/bin/xtwifi-client
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/bin
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

ifeq ($(wildcard vendor/qcom/proprietary/media),)
endif

ifeq ($(wildcard vendor/qcom/proprietary/media),)
endif

ifeq ($(wildcard vendor/qcom/proprietary/media),)
endif

ifeq ($(wildcard vendor/qcom/proprietary/media),)
endif

ifeq ($(wildcard vendor/qcom/proprietary/media),)
endif

ifeq ($(wildcard vendor/qcom/proprietary/media),)
endif

ifeq ($(wildcard vendor/qcom/proprietary/media),)

include $(CLEAR_VARS)
LOCAL_MODULE := c2_manifest_vendor.xml
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/etc/vintf/manifest/c2_manifest_vendor.xml
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/etc/vintf/manifest
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := manifest_android.hardware.drm-service.playready-qti.xml
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/etc/vintf/manifest/manifest_android.hardware.drm-service.playready-qti.xml
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/etc/vintf/manifest
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.qseecom@1.0-service.xml
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/etc/vintf/manifest/vendor.qti.hardware.qseecom@1.0-service.xml
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/etc/vintf/manifest
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.qteeconnector@1.0-service.xml
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/etc/vintf/manifest/vendor.qti.hardware.qteeconnector@1.0-service.xml
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/etc/vintf/manifest
include $(BUILD_PREBUILT)

#include $(CLEAR_VARS)
#LOCAL_MODULE := vhal-qti-service.xml
#LOCAL_MODULE_OWNER := qcom
#LOCAL_MODULE_TAGS := optional
#LOCAL_MODULE_CLASS := ETC
#LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/etc/vintf/manifest/vhal-qti-service.xml
#LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/etc/vintf/manifest
#include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := eglSubDriverAndroid
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libEGL_adreno libcutils libhardware libgsl libnativewindow vendor.qti.hardware.display.mapper@2.0 vendor.qti.hardware.display.mapper@3.0 android.hardware.graphics.mapper@3.0 android.hardware.graphics.mapper@2.0 vendor.qti.hardware.display.mapper@4.0 android.hardware.graphics.mapper@4.0 libgralloc.qti libgralloctypes vendor.qti.hardware.display.mapperextensions@1.1 libhidlbase libutils libdl libz liblog libc++ libc libm
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/egl/eglSubDriverAndroid.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64/egl
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libEGL_adreno
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libadreno_utils libgsl libcutils libdl libz liblog libc++ libc libm
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/egl/libEGL_adreno.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64/egl
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libGLESv1_CM_adreno
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgsl libcutils libdl libz liblog libc++ libc libm
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/egl/libGLESv1_CM_adreno.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64/egl
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libGLESv2_adreno
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libadreno_utils libgsl libllvm-glnext libcutils libdl libz liblog libc++ libc libm
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/egl/libGLESv2_adreno.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64/egl
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libq3dtools_adreno
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libgsl libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/egl/libq3dtools_adreno.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64/egl
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libq3dtools_esx
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libz libdl libc++ libc libm
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/egl/libq3dtools_esx.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64/egl
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libqtigatekeeper
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := android.hardware.gatekeeper-V1-ndk libhardware libbinder_ndk libbase libutils liblog libcutils libgatekeeper libkeymasterdeviceutils libqcbor libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/hw/libqtigatekeeper.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64/hw
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.qseecom@1.0-impl
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libbase libutils libcutils liblog libdmabufheap vendor.qti.hardware.qseecom-V1-ndk libbinder_ndk libQSEEComAPI libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/hw/vendor.qti.hardware.qseecom@1.0-impl.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64/hw
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.qteeconnector@1.0-impl
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libQSEEComAPI libxml2 vendor.qti.hardware.qteeconnector-V1-ndk libbinder_ndk libion libGPreqcancel_svc libdmabufheap libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/hw/vendor.qti.hardware.qteeconnector@1.0-impl.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64/hw
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vulkan.adreno
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libadreno_utils libgsl libz liblog libllvm-glnext libcutils libhardware libnativewindow vendor.qti.hardware.display.mapper@2.0 android.hardware.graphics.mapper@2.0 vendor.qti.hardware.display.mapper@3.0 android.hardware.graphics.mapper@3.0 vendor.qti.hardware.display.mapperextensions@1.1 libhidlbase libutils vendor.qti.hardware.display.mapper@4.0 android.hardware.graphics.mapper@4.0 libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/hw/vulkan.adreno.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64/hw
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libadreno_app_profiles
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils libdl libz liblog libc++ libc libm
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libadreno_app_profiles.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libadreno_utils
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgsl libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libadreno_utils.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libadsp_default_listener
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libadsprpc libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libadsp_default_listener.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libAlacSwDec
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libAlacSwDec.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libApeSwDec
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libApeSwDec.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libar-gsl_fe
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblx-osal liblx-ar_util libuhab libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libar-gsl_fe.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libaudiohalplugin
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils libcdcdriver libaudcal liblog libuhab libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libaudiohalplugin.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libbase64
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libbase64.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libbufferq
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libuhab liblog libmmosal_vendor libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libbufferq.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libC2D2
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgsl libcutils libutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libC2D2.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libc2d30_bltlib
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgsl libcutils libutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libc2d30_bltlib.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcacertclient
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libutils libcutils libhidlbase vendor.qti.hardware.cacert@1.0 libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libcacertclient.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcantranslator
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libbinder_ndk libbase libcutils libutils liblog libcanwrapper libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libcantranslator.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcanwrapper
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libcanwrapper.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcapiv2svacnn
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libcapiv2svacnn.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcapiv2vop
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libcapiv2vop.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libCB
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libgsl libutils libnativewindow libvndksupport libgralloc.qti libgralloctypes vendor.qti.hardware.display.mapper@4.0 android.hardware.graphics.mapper@4.0 vendor.qti.hardware.display.mapperextensions@1.0 vendor.qti.hardware.display.mapperextensions@1.1 libhidlbase vendor.qti.hardware.display.mapper@3.0 android.hardware.graphics.mapper@3.0 libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libCB.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcdcdriver
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libcdcdriver.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcdfw_remote_api
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgps.utils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libcdfw_remote_api.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcdfw
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgps.utils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libcdfw.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcdsp_default_listener
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcdsprpc libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libcdsp_default_listener.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcpion
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libdmabufheap libvmmem libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libcpion.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libdisp-aba
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libdisp-aba.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libdisplayqos
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libdisplayqos.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libdpps
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils libdisp-aba libqdutils libutils liblog libbinder libdrm libtinyxml2 libhidlbase android.frameworks.sensorservice@1.0 libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libdpps.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libdrmfs
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libdiag liblog libQSEEComAPI libxml2 libseclog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libdrmfs.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libdrmtime
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libdiag liblog libQSEEComAPI libtime_genoff libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libdrmtime.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libengineplugin
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgps.utils libcutils libdiag libz liblog liblocdiagiface libutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libengineplugin.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libfastcvdsp_stub
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcdsprpc libdmabufheap libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libfastcvdsp_stub.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libfastcvopt
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libdmabufheap libOpenCL libfastcvdsp_stub libcdsprpc libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libfastcvopt.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libfenceq
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libmmosal_vendor libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libfenceq.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libFlacSwDec
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libFlacSwDec.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libgdtap
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libgps.utils liblog libloc_core libizat_core liblbs_core libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libgdtap.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libGPMTEEC_vendor
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc libcutils libutils liblog libGPTEE_vendor libminkdescriptor libminksocket_vendor libdmabufheap libc++ libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libGPMTEEC_vendor.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libGPQTEEC_vendor
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc libcutils libutils liblog libhidlbase libGPTEE_vendor libQTEEConnector_vendor vendor.qti.hardware.qteeconnector-V1-ndk libc++ libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libGPQTEEC_vendor.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libGPreqcancel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libdiag liblog libQSEEComAPI libGPreqcancel_svc libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libGPreqcancel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libGPreqcancel_svc
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libGPreqcancel_svc.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libGPTEE_vendor
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc libcutils libutils liblog libc++ libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libGPTEE_vendor.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libgpudataproducer
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libz libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libgpudataproducer.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libgsl
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libz libsync libion libdmabufheap libuhab libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libgsl.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libhdcp1prov
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libdiag libutils libcutils liblog libminkdescriptor libtrustedapploader libQSEEComAPI libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libhdcp1prov.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libhdcp2p2prov
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libdiag libutils libcutils liblog libminkdescriptor libtrustedapploader libQSEEComAPI libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libhdcp2p2prov.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libhdcpsrm
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libQSEEComAPI libminkdescriptor libtrustedapploader libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libhdcpsrm.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libhdr_tm
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libsdm-color libtinyxml2_1 libcutils libutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libhdr_tm.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libhta_hexagon_runtime
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcdsprpc libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libhta_hexagon_runtime.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libI420colorconvert
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libmm-color-convertor libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libI420colorconvert.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libizat_core
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libloc_core libgps.utils libloc_api_v02 libloc_util libloc_base_util libsqlite liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libizat_core.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libjnihelper
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libqmiservices libqmi_cci libcacertclient libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libjnihelper.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libkeymasterdeviceutils
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libion libcutils liblog libcrypto libdmabufheap libuhab libQSEEComAPI libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libkeymasterdeviceutils.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libkeymasterprovision
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcrypto libkeymasterdeviceutils libkeymasterutils libqcbor libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libkeymasterprovision.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := liblistensoundmodel2
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/liblistensoundmodel2.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libllvm-glnext
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgsl liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libllvm-glnext.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libllvm-qcom
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libllvm-qcom.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libllvm-qgl
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgsl liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libllvm-qgl.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libloc_eng_hub
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgps.utils liblog libizat_core libgnsspps libdiag liblocdiagiface libutils libloc_util libcdfw libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libloc_eng_hub.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libloc_qwes_iface
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libgps.utils libqcbor libminksocket_vendor libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libloc_qwes_iface.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libloc_sll_impl
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libloc_core libgps.utils liblog libssl libcrypto libz libdiag libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libloc_sll_impl.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libloc_util
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libgps.utils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libloc_util.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := liblowi_client
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libgps.utils libloc_base_util libloc_mq_client liblowi_common libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/liblowi_client.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := liblowi_common
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libloc_base_util libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/liblowi_common.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := liblowi_wifihal
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblowi_client libcutils libutils liblog libgps.utils libloc_base_util libloc_mq_client liblowi_common libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/liblowi_wifihal.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libmapdata_api
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libizat_client_api libloc_core libgps.utils libutils libcutils liblog libprotobuf-cpp-lite-21.7 libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libmapdata_api.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libmm-color-convertor
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libmm-color-convertor.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libmm-hdcpmgr
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libmm-hdcpmgr.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libmulawdec
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libmulawdec.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libnative-api
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libminksocket_vendor liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libnative-api.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libnpu
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcdsprpc libdmabufheap libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libnpu.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := liboemcrypto
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils liblog libQSEEComAPI libcpion libminkdescriptor libcutils libtrustedapploader vendor.qti.hardware.display.config-V7-ndk libpower libcrypto libbinder_ndk libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/liboemcrypto.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libOpenCL
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils libvndksupport libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libOpenCL.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libops
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libQSEEComAPI liblog libdrm vendor.qti.hardware.display.config-V7-ndk libbinder_ndk libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libops.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libpal_net_if
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libgps.utils liblog libandroid_net libcrypto libssl libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libpal_net_if.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libpasrutils
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libpasrutils.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libperipheral_client
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils libutils liblog libbinder libmdmdetect libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libperipheral_client.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libpraidl
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libion libcpion liblog libhidlbase libbase64 libbinder libutils libstagefright_foundation android.hardware.drm-V1-ndk android.hardware.common-V2-ndk libbase libcrypto libbinder_ndk libhwbinder libcutils libjsoncpp libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libpraidl.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

ifeq ($(wildcard vendor/qcom/proprietary/media),)

include $(CLEAR_VARS)
LOCAL_MODULE := libqcodec2_basecodec
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libcodec2_vndk libqcodec2_base libqcodec2_utils libqcodec2_platform libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libqcodec2_basecodec.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)
endif

ifeq ($(wildcard vendor/qcom/proprietary/media),)

include $(CLEAR_VARS)
LOCAL_MODULE := libqcodec2_base
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libcodec2_vndk libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libqcodec2_base.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)
endif

ifeq ($(wildcard vendor/qcom/proprietary/media),)

include $(CLEAR_VARS)
LOCAL_MODULE := libqcodec2_core
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libcodec2_vndk libqcodec2_base libqcodec2_hooks libqcodec2_utils libqcodec2_platform libqcodec2_basecodec libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libqcodec2_core.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)
endif

ifeq ($(wildcard vendor/qcom/proprietary/media),)

include $(CLEAR_VARS)
LOCAL_MODULE := libqcodec2_platform
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libcodec2_vndk libqcodec2_base libjsoncpp libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libqcodec2_platform.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)
endif

ifeq ($(wildcard vendor/qcom/proprietary/media),)

include $(CLEAR_VARS)
LOCAL_MODULE := libqcodec2_utils
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libcodec2_vndk libqcodec2_base libqcodec2_platform libhidlbase libgrallocutils libgralloctypes libgralloc.qti libbinder_ndk libdisplayconfig.qti android.hardware.graphics.mapper@3.0 android.hardware.graphics.mapper@4.0 libnativewindow libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libqcodec2_utils.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)
endif

ifeq ($(wildcard vendor/qcom/proprietary/media),)

include $(CLEAR_VARS)
LOCAL_MODULE := libqcodec2_v4l2codec
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libqcodec2_base libqcodec2_platform libqcodec2_utils libqcodec2_basecodec libcodec2_vndk libhyp_video_intercept libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libqcodec2_v4l2codec.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := libqcrildatactl
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libqmi_cci libqmiservices libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libqcrildatactl.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libqdpr
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libqmiservices libqmi_cci libqmi_common_so libqmi_client_helper libcutils libutils liblog libdiag libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libqdpr.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libQProfilerInterface
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libz liblog libEGL libGLESv2 libm libdl libc
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libQProfilerInterface.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libQSEEComAPI
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libion libcutils libutils liblog libdmabufheap libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libQSEEComAPI.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libqseed3
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libqseed3.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libQTEEConnector_vendor
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libhidlbase liblog libcutils libutils libbinder_ndk vendor.qti.hardware.qteeconnector-V1-ndk libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libQTEEConnector_vendor.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libqtikeymaster4
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcrypto libhidlbase libutils libcutils libkeymasterdeviceutils libkeymasterutils libbase libqcbor android.hardware.keymaster@4.0 android.hardware.keymaster@4.1 libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libqtikeymaster4.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libqtikeymint
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcrypto libhidlbase libutils libcutils libkeymasterdeviceutils libbase libqcbor libkeymaster_messages android.hardware.keymaster@4.0 android.hardware.keymaster@4.1 android.hardware.security.keymint-V3-ndk android.hardware.security.rkp-V3-ndk android.hardware.security.secureclock-V1-ndk android.hardware.security.sharedsecret-V1-ndk libbinder_ndk libkeymasterutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libqtikeymint.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libreverse_rpc_tests_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libadsprpc liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libreverse_rpc_tests_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := librpmb
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libQSEEComAPI liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/librpmb.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsdm-colormgr-algo
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libdisplaydebug libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libsdm-colormgr-algo.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsdm-color
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcutils libbinder libutils libdisplaydebug libtinyxml2_1 libsdmutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libsdm-color.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsdm-diag
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libdiag libsdmutils libbinder libdisplaydebug libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libsdm-diag.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsdm-disp-vndapis
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libdisplaydebug libqservice libbinder libcutils libutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libsdm-disp-vndapis.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsdmextension
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libdisplaydebug libsdmutils libdisplayqos libsdm-color libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libsdmextension.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libseclog
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libseclog.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsll_izat_if
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libgps.utils libandroid_net liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libsll_izat_if.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libssc_default_listener
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libsdsprpc libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libssc_default_listener.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libssd
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libdiag liblog libQSEEComAPI libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libssd.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsys_info_cache
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libgps.utils libloc_core liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libsys_info_cache.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsysmon_cdsp_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libsysmon_cdsp_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsysprofiler
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libQProfilerInterface libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libsysprofiler.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libtinyxml2_1
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libtinyxml2_1.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libtrustedapploader
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libminkdescriptor libminksocket_vendor libqcbor libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libtrustedapploader.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libtzdrmgenprov
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libQSEEComAPI libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libtzdrmgenprov.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libxtadapter
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgps.utils liblog libloc_core libizat_core libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libxtadapter.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libprdrmengine
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libion libcpion liblog libhidlbase libbase64 libbinder libutils libstagefright_foundation libcutils libQSEEComAPI vendor.display.config@1.0 android.hardware.drm-V1-ndk libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/mediadrm/libprdrmengine.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64/mediadrm
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libasphere
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/soundfx/libasphere.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64/soundfx
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libqcbassboost
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/soundfx/libqcbassboost.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64/soundfx
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libqcreverb
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/soundfx/libqcreverb.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64/soundfx
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libqcvirt
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/soundfx/libqcvirt.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64/soundfx
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libqtiautobundle
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libjson libqti-utils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/soundfx/libqtiautobundle.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64/soundfx
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libshoebox
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/soundfx/libshoebox.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64/soundfx
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := VehicleHardware
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := VehicleHalUtils FakeVehicleHalValueGenerators libjsoncpp VehicleHalJsonConfigLoaderEnableTestProperties FakeUserHal libbinder_ndk libcutils libbase liblog libutils libcanwrapper libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/VehicleHardware.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := eglSubDriverAndroid
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libEGL_adreno libcutils libhardware libgsl libnativewindow vendor.qti.hardware.display.mapper@2.0 vendor.qti.hardware.display.mapper@3.0 android.hardware.graphics.mapper@3.0 android.hardware.graphics.mapper@2.0 vendor.qti.hardware.display.mapper@4.0 android.hardware.graphics.mapper@4.0 libgralloc.qti libgralloctypes vendor.qti.hardware.display.mapperextensions@1.1 libhidlbase libutils libdl libz liblog libc++ libc libm
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/egl/eglSubDriverAndroid.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/egl
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libEGL_adreno
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libadreno_utils libgsl libcutils libdl libz liblog libc++ libc libm
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/egl/libEGL_adreno.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/egl
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libGLESv1_CM_adreno
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgsl libcutils libdl libz liblog libc++ libc libm
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/egl/libGLESv1_CM_adreno.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/egl
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libGLESv2_adreno
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libadreno_utils libgsl libllvm-glnext libcutils libdl libz liblog libc++ libc libm
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/egl/libGLESv2_adreno.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/egl
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libq3dtools_adreno
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libgsl libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/egl/libq3dtools_adreno.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/egl
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libq3dtools_esx
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libz libdl libc++ libc libm
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/egl/libq3dtools_esx.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/egl
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libqtigatekeeper
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := android.hardware.gatekeeper-V1-ndk libhardware libbinder_ndk libbase libutils liblog libcutils libgatekeeper libkeymasterdeviceutils libqcbor libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/hw/libqtigatekeeper.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/hw
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.qseecom@1.0-impl
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libbase libutils libcutils liblog libdmabufheap vendor.qti.hardware.qseecom-V1-ndk libbinder_ndk libQSEEComAPI libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/hw/vendor.qti.hardware.qseecom@1.0-impl.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/hw
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.qteeconnector@1.0-impl
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libQSEEComAPI libxml2 vendor.qti.hardware.qteeconnector-V1-ndk libbinder_ndk libion libGPreqcancel_svc libdmabufheap libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/hw/vendor.qti.hardware.qteeconnector@1.0-impl.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/hw
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vulkan.adreno
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libadreno_utils libgsl libz liblog libllvm-glnext libcutils libhardware libnativewindow vendor.qti.hardware.display.mapper@2.0 android.hardware.graphics.mapper@2.0 vendor.qti.hardware.display.mapper@3.0 android.hardware.graphics.mapper@3.0 vendor.qti.hardware.display.mapperextensions@1.1 libhidlbase libutils vendor.qti.hardware.display.mapper@4.0 android.hardware.graphics.mapper@4.0 libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/hw/vulkan.adreno.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/hw
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libadm
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcutils libtinyalsa libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libadm.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libadreno_app_profiles
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils libdl libz liblog libc++ libc libm
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libadreno_app_profiles.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libadreno_utils
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgsl libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libadreno_utils.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libadsp_default_listener
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libadsprpc libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libadsp_default_listener.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libAlacSwDec
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libAlacSwDec.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libApeSwDec
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libApeSwDec.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libar-gsl_fe
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblx-osal liblx-ar_util libuhab libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libar-gsl_fe.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libaudiohalplugin
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils libcdcdriver libaudcal liblog libuhab libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libaudiohalplugin.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libbase64
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libbase64.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libbufferq
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libuhab liblog libmmosal_vendor libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libbufferq.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libC2D2
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgsl libcutils libutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libC2D2.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libc2d30_bltlib
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgsl libcutils libutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libc2d30_bltlib.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcacertclient
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libutils libcutils libhidlbase vendor.qti.hardware.cacert@1.0 libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libcacertclient.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcantranslator
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libbinder_ndk libbase libcutils libutils liblog libcanwrapper libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libcantranslator.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcanwrapper
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libcanwrapper.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcapiv2svacnn
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libcapiv2svacnn.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcapiv2vop
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libcapiv2vop.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libCB
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libgsl libutils libnativewindow libvndksupport libgralloc.qti libgralloctypes vendor.qti.hardware.display.mapper@4.0 android.hardware.graphics.mapper@4.0 vendor.qti.hardware.display.mapperextensions@1.0 vendor.qti.hardware.display.mapperextensions@1.1 libhidlbase vendor.qti.hardware.display.mapper@3.0 android.hardware.graphics.mapper@3.0 libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libCB.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcdcdriver
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libcdcdriver.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcdfw_remote_api
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgps.utils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libcdfw_remote_api.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcdfw
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgps.utils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libcdfw.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcdsp_default_listener
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcdsprpc libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libcdsp_default_listener.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcpion
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libdmabufheap libvmmem libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libcpion.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libdisp-aba
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libdisp-aba.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libdisplayqos
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libdisplayqos.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libdpps
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils libdisp-aba libqdutils libutils liblog libbinder libdrm libtinyxml2 libhidlbase android.frameworks.sensorservice@1.0 libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libdpps.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libdrc
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libdrc.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libdrmfs
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libdiag liblog libQSEEComAPI libxml2 libseclog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libdrmfs.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libdrmtime
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libdiag liblog libQSEEComAPI libtime_genoff libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libdrmtime.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libdsd2pcm
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libdsd2pcm.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libengineplugin
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgps.utils libcutils libdiag libz liblog liblocdiagiface libutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libengineplugin.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libfastcrc
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libfastcrc.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libfastcvdsp_stub
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcdsprpc libdmabufheap libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libfastcvdsp_stub.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libfastcvopt
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libdmabufheap libOpenCL libfastcvdsp_stub libcdsprpc libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libfastcvopt.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libfenceq
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libmmosal_vendor libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libfenceq.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libFlacSwDec
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libFlacSwDec.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libgdtap
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libgps.utils liblog libloc_core libizat_core liblbs_core libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libgdtap.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libGPMTEEC_vendor
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc libcutils libutils liblog libGPTEE_vendor libminkdescriptor libminksocket_vendor libdmabufheap libc++ libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libGPMTEEC_vendor.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libGPQTEEC_vendor
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc libcutils libutils liblog libhidlbase libGPTEE_vendor libQTEEConnector_vendor vendor.qti.hardware.qteeconnector-V1-ndk libc++ libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libGPQTEEC_vendor.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libGPreqcancel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libdiag liblog libQSEEComAPI libGPreqcancel_svc libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libGPreqcancel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libGPreqcancel_svc
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libGPreqcancel_svc.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libGPTEE_vendor
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc libcutils libutils liblog libc++ libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libGPTEE_vendor.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libgpudataproducer
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libz libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libgpudataproducer.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libgsl
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libz libsync libion libdmabufheap libuhab libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libgsl.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libhdcp1prov
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libdiag libutils libcutils liblog libminkdescriptor libtrustedapploader libQSEEComAPI libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libhdcp1prov.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libhdcp2p2prov
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libdiag libutils libcutils liblog libminkdescriptor libtrustedapploader libQSEEComAPI libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libhdcp2p2prov.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libhdcpsrm
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libQSEEComAPI libminkdescriptor libtrustedapploader libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libhdcpsrm.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libhdr_tm
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libsdm-color libtinyxml2_1 libcutils libutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libhdr_tm.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libhta_hexagon_runtime
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcdsprpc libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libhta_hexagon_runtime.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libI420colorconvert
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libmm-color-convertor libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libI420colorconvert.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libizat_core
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libloc_core libgps.utils libloc_api_v02 libloc_util libloc_base_util libsqlite liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libizat_core.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libjnihelper
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libqmiservices libqmi_cci libcacertclient libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libjnihelper.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libkeymasterdeviceutils
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libion libcutils liblog libcrypto libdmabufheap libuhab libQSEEComAPI libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libkeymasterdeviceutils.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libkeymasterprovision
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcrypto libkeymasterdeviceutils libkeymasterutils libqcbor libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libkeymasterprovision.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := liblistensoundmodel2
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/liblistensoundmodel2.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libllvm-glnext
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgsl liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libllvm-glnext.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libllvm-qcom
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libllvm-qcom.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libllvm-qgl
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgsl liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libllvm-qgl.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libloc_eng_hub
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgps.utils liblog libizat_core libgnsspps libdiag liblocdiagiface libutils libloc_util libcdfw libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libloc_eng_hub.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libloc_qwes_iface
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libgps.utils libqcbor libminksocket_vendor libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libloc_qwes_iface.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libloc_sll_impl
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libloc_core libgps.utils liblog libssl libcrypto libz libdiag libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libloc_sll_impl.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libloc_util
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libgps.utils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libloc_util.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := liblowi_client
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libgps.utils libloc_base_util libloc_mq_client liblowi_common libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/liblowi_client.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := liblowi_common
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libloc_base_util libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/liblowi_common.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := liblowi_wifihal
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblowi_client libcutils libutils liblog libgps.utils libloc_base_util libloc_mq_client liblowi_common libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/liblowi_wifihal.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libmapdata_api
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libizat_client_api libloc_core libgps.utils libutils libcutils liblog libprotobuf-cpp-lite-21.7 libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libmapdata_api.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libmm-color-convertor
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libmm-color-convertor.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libmm-hdcpmgr
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libmm-hdcpmgr.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libmulawdec
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libmulawdec.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libnative-api
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libminksocket_vendor liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libnative-api.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libnpu
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcdsprpc libdmabufheap libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libnpu.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := liboemcrypto
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils liblog libQSEEComAPI libcpion libminkdescriptor libcutils libtrustedapploader vendor.qti.hardware.display.config-V7-ndk libpower libcrypto libbinder_ndk libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/liboemcrypto.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libOpenCL
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils libvndksupport libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libOpenCL.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libops
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libQSEEComAPI liblog libdrm vendor.qti.hardware.display.config-V7-ndk libbinder_ndk libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libops.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libpal_net_if
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libgps.utils liblog libandroid_net libcrypto libssl libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libpal_net_if.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libperipheral_client
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils libutils liblog libbinder libmdmdetect libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libperipheral_client.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libpraidl
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libion libcpion liblog libhidlbase libbase64 libbinder libutils libstagefright_foundation android.hardware.drm-V1-ndk android.hardware.common-V2-ndk libbase libcrypto libbinder_ndk libhwbinder libcutils libjsoncpp libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libpraidl.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

ifeq ($(wildcard vendor/qcom/proprietary/media),)

include $(CLEAR_VARS)
LOCAL_MODULE := libqcodec2_basecodec
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libcodec2_vndk libqcodec2_base libqcodec2_utils libqcodec2_platform libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libqcodec2_basecodec.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)
endif

ifeq ($(wildcard vendor/qcom/proprietary/media),)

include $(CLEAR_VARS)
LOCAL_MODULE := libqcodec2_base
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libcodec2_vndk libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libqcodec2_base.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)
endif

ifeq ($(wildcard vendor/qcom/proprietary/media),)

include $(CLEAR_VARS)
LOCAL_MODULE := libqcodec2_core
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libcodec2_vndk libqcodec2_base libqcodec2_hooks libqcodec2_utils libqcodec2_platform libqcodec2_basecodec libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libqcodec2_core.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)
endif

ifeq ($(wildcard vendor/qcom/proprietary/media),)

include $(CLEAR_VARS)
LOCAL_MODULE := libqcodec2_platform
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libcodec2_vndk libqcodec2_base libjsoncpp libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libqcodec2_platform.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)
endif

ifeq ($(wildcard vendor/qcom/proprietary/media),)

include $(CLEAR_VARS)
LOCAL_MODULE := libqcodec2_utils
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libcodec2_vndk libqcodec2_base libqcodec2_platform libhidlbase libgrallocutils libgralloctypes libgralloc.qti libbinder_ndk libdisplayconfig.qti android.hardware.graphics.mapper@3.0 android.hardware.graphics.mapper@4.0 libnativewindow libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libqcodec2_utils.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)
endif

ifeq ($(wildcard vendor/qcom/proprietary/media),)

include $(CLEAR_VARS)
LOCAL_MODULE := libqcodec2_v4l2codec
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libqcodec2_base libqcodec2_platform libqcodec2_utils libqcodec2_basecodec libcodec2_vndk libhyp_video_intercept libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libqcodec2_v4l2codec.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := libqcrildatactl
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libqmi_cci libqmiservices libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libqcrildatactl.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libqdpr
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libqmiservices libqmi_cci libqmi_common_so libqmi_client_helper libcutils libutils liblog libdiag libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libqdpr.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libQSEEComAPI
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libion libcutils libutils liblog libdmabufheap libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libQSEEComAPI.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libqseed3
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libqseed3.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libQTEEConnector_vendor
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libhidlbase liblog libcutils libutils libbinder_ndk vendor.qti.hardware.qteeconnector-V1-ndk libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libQTEEConnector_vendor.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libqtikeymaster4
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcrypto libhidlbase libutils libcutils libkeymasterdeviceutils libkeymasterutils libbase libqcbor android.hardware.keymaster@4.0 android.hardware.keymaster@4.1 libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libqtikeymaster4.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libqtikeymint
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcrypto libhidlbase libutils libcutils libkeymasterdeviceutils libbase libqcbor libkeymaster_messages android.hardware.keymaster@4.0 android.hardware.keymaster@4.1 android.hardware.security.keymint-V3-ndk android.hardware.security.rkp-V3-ndk android.hardware.security.secureclock-V1-ndk android.hardware.security.sharedsecret-V1-ndk libbinder_ndk libkeymasterutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libqtikeymint.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libreverse_rpc_tests_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libadsprpc liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libreverse_rpc_tests_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := librpmb
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libQSEEComAPI liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/librpmb.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsdm-colormgr-algo
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libdisplaydebug libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libsdm-colormgr-algo.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsdm-color
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcutils libbinder libutils libdisplaydebug libtinyxml2_1 libsdmutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libsdm-color.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsdm-diag
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libdiag libsdmutils libbinder libdisplaydebug libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libsdm-diag.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsdm-disp-vndapis
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libdisplaydebug libqservice libbinder libcutils libutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libsdm-disp-vndapis.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsdmextension
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libdisplaydebug libsdmutils libdisplayqos libsdm-color libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libsdmextension.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libseclog
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libseclog.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsll_izat_if
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libgps.utils libandroid_net liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libsll_izat_if.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libssc_default_listener
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libsdsprpc libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libssc_default_listener.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libssd
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libdiag liblog libQSEEComAPI libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libssd.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libstreamparser
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libvideoutils liblog libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libstreamparser.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsurround_3mic_proc
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libsurround_3mic_proc.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsys_info_cache
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils libgps.utils libloc_core liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libsys_info_cache.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsysmon_cdsp_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libsysmon_cdsp_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libtinyxml2_1
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libtinyxml2_1.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libtrustedapploader
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libminkdescriptor libminksocket_vendor libqcbor libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libtrustedapploader.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libtzdrmgenprov
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libQSEEComAPI libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libtzdrmgenprov.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libvideoutils
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libvideoutils.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libxtadapter
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libgps.utils liblog libloc_core libizat_core libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libxtadapter.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libprdrmengine
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libion libcpion liblog libhidlbase libbase64 libbinder libutils libstagefright_foundation libcutils libQSEEComAPI vendor.display.config@1.0 android.hardware.drm-V1-ndk libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/mediadrm/libprdrmengine.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/mediadrm
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := capi_v2_asrc
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/capi_v2_asrc.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := capi_v2_avc
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/capi_v2_avc.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := capi_v2_bmt
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/capi_v2_bmt.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := capi_v2_ecmx
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/capi_v2_ecmx.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := capi_v2_fnb
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/capi_v2_fnb.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := capi_v2_loud
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/capi_v2_loud.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := capi_v2_peq
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/capi_v2_peq.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := capi_v2_sumx
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/capi_v2_sumx.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := capi_v2_synth
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/capi_v2_synth.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := icc_module.so.1
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=
LOCAL_MODULE_SUFFIX :=
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/icc_module.so.1
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libapps_mem_heap
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc libgcc
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/libapps_mem_heap.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libdspCV_v66_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc libapps_mem_heap libgcc
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/libdspCV_v66_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libdspCV_v68_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc libgcc
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/libdspCV_v68_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libdspCV_v73_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc libgcc
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/libdspCV_v73_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libfastcvadsp_v66
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc libdspCV_v66_skel libgcc
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/libfastcvadsp_v66.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libfastcvadsp_v68
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc libdspCV_v68_skel libworker_pool libgcc
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/libfastcvadsp_v68.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libfastcvadsp_v73
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc libdspCV_v73_skel libworker_pool libgcc
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/libfastcvadsp_v73.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libfastcvdsp_v66_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc libfastcvadsp_v66 libdspCV_v66_skel libgcc
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/libfastcvdsp_v66_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libfastcvdsp_v68_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc libfastcvadsp_v68 libdspCV_v68_skel libgcc
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/libfastcvdsp_v68_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libfastcvdsp_v73_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc libfastcvadsp_v73 libdspCV_v73_skel libgcc
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/libfastcvdsp_v73_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libworker_pool_v68
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc libgcc
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/libworker_pool_v68.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libworker_pool_v73
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libc libgcc
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/libworker_pool_v73.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := sec_module.so.1
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=
LOCAL_MODULE_SUFFIX :=
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/sec_module.so.1
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libbenchmark_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/libbenchmark_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libfastrpc_tests1_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/libfastrpc_tests1_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libfastrpc_tests_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/libfastrpc_tests_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libhap_power_test_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/libhap_power_test_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := librpcperf1_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/librpcperf1_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := librpcperf_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/librpcperf_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libtest_dl1_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/libtest_dl1_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libtest_dl_perf1_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/libtest_dl_perf1_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libtest_dl_perf_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/libtest_dl_perf_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libtest_dl_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/libtest_dl_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libunsigned_pd_test_skel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/libunsigned_pd_test_skel.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := test_elf_dep1
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/test_elf_dep1.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := test_elf_dep2
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/test_elf_dep2.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := test_elf_nop
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/test_elf_nop.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := test_elf_size_128k
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/test_elf_size_128k.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := test_elf_size_16k
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/test_elf_size_16k.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := test_elf_size_1MB
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/test_elf_size_1MB.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := test_elf_size_256k
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/test_elf_size_256k.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := test_elf_size_32k
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/test_elf_size_32k.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := test_elf_size_512k
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/test_elf_size_512k.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := test_elf_size_64k
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/test_elf_size_64k.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := test_elf_size_768k
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CHECK_ELF_FILES := false
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/rfsa/adsp/tests/test_elf_size_768k.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp/tests
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libasphere
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/soundfx/libasphere.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/soundfx
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libqcbassboost
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/soundfx/libqcbassboost.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/soundfx
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libqcreverb
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/soundfx/libqcreverb.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/soundfx
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libqcvirt
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/soundfx/libqcvirt.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/soundfx
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libqtiautobundle
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libjson libqti-utils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/soundfx/libqtiautobundle.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/soundfx
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libshoebox
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/soundfx/libshoebox.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/soundfx
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)
LOCAL_PATH := $(PREBUILT_PATH)

include $(CLEAR_VARS)
LOCAL_MODULE := com.quicinc.cne.server-V2.1-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/com.quicinc.cne.server-V2.1-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := com.quicinc.cne.server-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/com.quicinc.cne.server-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.rcsuce-V1.1-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcsuce-V1.1-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.rcsuce-V1.1-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcsuce-V1.1-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := com.quicinc.cne.server-V2.2-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/com.quicinc.cne.server-V2.2-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.data.cne.internal.constants-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.data.cne.internal.constants-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.data.factory-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.data.factory-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V1.0-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.factory-V2.1-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.factory-V2.1-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.factory-V2.1-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.factory-V2.1-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.data.factory-V2.2-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.2-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := com.quicinc.cne.api-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/com.quicinc.cne.api-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := com.quicinc.cne.api-V1.1-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/com.quicinc.cne.api-V1.1-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.data.cne.internal.server-V1.3-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.data.cne.internal.server-V1.3-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := com.quicinc.cne.server-V2.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/com.quicinc.cne.server-V2.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.factory-V2.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.factory-V2.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.factory-V2.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.factory-V2.0-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := com.quicinc.cne.constants-V2.1-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/com.quicinc.cne.constants-V2.1-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.data.factory-V2.3-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.3-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.dpmservice-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.dpmservice-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.dpmservice-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.dpmservice-V1.0-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.rcsuce-V1.2-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcsuce-V1.2-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.rcsuce-V1.2-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcsuce-V1.2-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.rcssip-V1.2-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcssip-V1.2-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.rcssip-V1.2-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcssip-V1.2-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.data.factory-V2.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.data.factory-V2.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.0-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.data.factory-V2.1-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.1-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.data.factory-V2.1-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.1-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := com.quicinc.cne.constants-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/com.quicinc.cne.constants-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.data.cne.internal.server-V1.2-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.data.cne.internal.server-V1.2-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.data.factory-V2.6-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.6-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.slmadapter-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.slmadapter-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.slmadapter-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.slmadapter-V1.0-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.data.cne.internal.server-V1.1-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.data.cne.internal.server-V1.1-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.data.cne.internal.server-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.data.cne.internal.server-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.rcssip-V1.1-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcssip-V1.1-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.rcssip-V1.1-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcssip-V1.1-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.data.factory-V2.5-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.5-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.data.factory-V2.4-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.4-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.data.factory-V2.4-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.4-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.connection-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.connection-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.connection-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.connection-V1.0-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.rcsuce-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcsuce-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.rcsuce-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcsuce-V1.0-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.factory-V1.1-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.factory-V1.1-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.factory-V1.1-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.factory-V1.1-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.factory-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.factory-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.factory-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.factory-V1.0-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := com.quicinc.cne.constants-V2.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/com.quicinc.cne.constants-V2.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.rcssip-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcssip-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.rcssip-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcssip-V1.0-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.mwqemadapter-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.mwqemadapter-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.mwqemadapter-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.mwqemadapter-V1.0-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.factory-V2.2-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.factory-V2.2-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.ims.factory-V2.2-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.factory-V2.2-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.dpmservice-V1.1-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.dpmservice-V1.1-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.dpmservice-V1.1-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.dpmservice-V1.1-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.data.factory-V2.7-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.7-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.data.cne.internal.api-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.data.cne.internal.api-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.data.qmi-V1.0-java
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.data.qmi-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

$(call symlink-file,,/data/vendor/fastcv/libfastcvdsp_skel.so,$(PRODUCT_OUT)/vendor/lib/rfsa/adsp/libfastcvdsp_skel.so)
ALL_DEFAULT_INSTALLED_MODULES += $(PRODUCT_OUT)/vendor/lib/rfsa/adsp/libfastcvdsp_skel.so

$(call symlink-file,,/data/vendor/fastcv/libdspCV_skel.so,$(PRODUCT_OUT)/vendor/lib/rfsa/adsp/libdspCV_skel.so)
ALL_DEFAULT_INSTALLED_MODULES += $(PRODUCT_OUT)/vendor/lib/rfsa/adsp/libdspCV_skel.so

$(call symlink-file,,/data/vendor/fastcv/libfastcvadsp.so,$(PRODUCT_OUT)/vendor/lib/rfsa/adsp/libfastcvadsp.so)
ALL_DEFAULT_INSTALLED_MODULES += $(PRODUCT_OUT)/vendor/lib/rfsa/adsp/libfastcvadsp.so

$(call symlink-file,,/data/vendor/fastcv/libworker_pool.so,$(PRODUCT_OUT)/vendor/lib/rfsa/adsp/libworker_pool.so)
ALL_DEFAULT_INSTALLED_MODULES += $(PRODUCT_OUT)/vendor/lib/rfsa/adsp/libworker_pool.so

$(call symlink-file,,egl/libEGL_adreno.so,$(PRODUCT_OUT)/vendor/lib/libEGL_adreno.so)
ALL_DEFAULT_INSTALLED_MODULES += $(PRODUCT_OUT)/vendor/lib/libEGL_adreno.so

$(call symlink-file,,egl/libGLESv2_adreno.so,$(PRODUCT_OUT)/vendor/lib/libGLESv2_adreno.so)
ALL_DEFAULT_INSTALLED_MODULES += $(PRODUCT_OUT)/vendor/lib/libGLESv2_adreno.so

$(call symlink-file,,egl/libq3dtools_adreno.so,$(PRODUCT_OUT)/vendor/lib/libq3dtools_adreno.so)
ALL_DEFAULT_INSTALLED_MODULES += $(PRODUCT_OUT)/vendor/lib/libq3dtools_adreno.so

$(call symlink-file,,egl/libEGL_adreno.so,$(PRODUCT_OUT)/vendor/lib64/libEGL_adreno.so)
ALL_DEFAULT_INSTALLED_MODULES += $(PRODUCT_OUT)/vendor/lib64/libEGL_adreno.so

$(call symlink-file,,egl/libGLESv2_adreno.so,$(PRODUCT_OUT)/vendor/lib64/libGLESv2_adreno.so)
ALL_DEFAULT_INSTALLED_MODULES += $(PRODUCT_OUT)/vendor/lib64/libGLESv2_adreno.so

$(call symlink-file,,egl/libq3dtools_adreno.so,$(PRODUCT_OUT)/vendor/lib64/libq3dtools_adreno.so)
ALL_DEFAULT_INSTALLED_MODULES += $(PRODUCT_OUT)/vendor/lib64/libq3dtools_adreno.so
