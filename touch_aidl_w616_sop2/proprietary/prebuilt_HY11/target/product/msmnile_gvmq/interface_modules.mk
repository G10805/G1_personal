PREBUILT_PATH := $(call my-dir)
LOCAL_PATH := $(PREBUILT_PATH)

include $(CLEAR_VARS)
LOCAL_MODULE := com.qualcomm.qti.dpm.api@1.0.vendor
LOCAL_INSTALLED_MODULE_STEM := com.qualcomm.qti.dpm.api@1.0.so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/com.qualcomm.qti.dpm.api@1.0.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcdsprpc
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libhidlbase libhardware libutils vendor.qti.hardware.dsp@1.0 liblog libc libcutils libdmabufheap libvmmem libc++ libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libcdsprpc.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libmdsprpc
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libhidlbase libhardware libutils vendor.qti.hardware.dsp@1.0 liblog libc libcutils libdmabufheap libvmmem libc++ libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libmdsprpc.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsubsystem_control
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libqmi_cci libqmi_common_so libmdmdetect libperipheral_client libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/libsubsystem_control.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.cacert@1.0
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/vendor.qti.hardware.cacert@1.0.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.dpmservice@1.0.vendor
LOCAL_INSTALLED_MODULE_STEM := vendor.qti.hardware.dpmservice@1.0.so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/vendor.qti.hardware.dpmservice@1.0.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.dpmservice@1.1.vendor
LOCAL_INSTALLED_MODULE_STEM := vendor.qti.hardware.dpmservice@1.1.so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := vendor.qti.hardware.dpmservice@1.0.vendor libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/vendor.qti.hardware.dpmservice@1.1.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.mwqemadapter@1.0.vendor
LOCAL_INSTALLED_MODULE_STEM := vendor.qti.hardware.mwqemadapter@1.0.so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/vendor.qti.hardware.mwqemadapter@1.0.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.slmadapter@1.0.vendor
LOCAL_INSTALLED_MODULE_STEM := vendor.qti.hardware.slmadapter@1.0.so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib64/vendor.qti.hardware.slmadapter@1.0.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := com.qualcomm.qti.dpm.api@1.0.vendor
LOCAL_INSTALLED_MODULE_STEM := com.qualcomm.qti.dpm.api@1.0.so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/com.qualcomm.qti.dpm.api@1.0.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcdsprpc
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libhidlbase libhardware libutils vendor.qti.hardware.dsp@1.0 liblog libc libcutils libdmabufheap libvmmem libc++ libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libcdsprpc.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libmdsprpc
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libhidlbase libhardware libutils vendor.qti.hardware.dsp@1.0 liblog libc libcutils libdmabufheap libvmmem libc++ libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libmdsprpc.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsubsystem_control
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog libqmi_cci libqmi_common_so libmdmdetect libperipheral_client libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/libsubsystem_control.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.cacert@1.0
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/vendor.qti.hardware.cacert@1.0.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.dpmservice@1.0.vendor
LOCAL_INSTALLED_MODULE_STEM := vendor.qti.hardware.dpmservice@1.0.so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/vendor.qti.hardware.dpmservice@1.0.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.dpmservice@1.1.vendor
LOCAL_INSTALLED_MODULE_STEM := vendor.qti.hardware.dpmservice@1.1.so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := vendor.qti.hardware.dpmservice@1.0.vendor libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/vendor.qti.hardware.dpmservice@1.1.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.mwqemadapter@1.0.vendor
LOCAL_INSTALLED_MODULE_STEM := vendor.qti.hardware.mwqemadapter@1.0.so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/vendor.qti.hardware.mwqemadapter@1.0.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.slmadapter@1.0.vendor
LOCAL_INSTALLED_MODULE_STEM := vendor.qti.hardware.slmadapter@1.0.so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../../.././target/product/msmnile_gvmq/vendor/lib/vendor.qti.hardware.slmadapter@1.0.so
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)
