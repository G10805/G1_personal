LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.bluetooth-impl-qti
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true

LOCAL_SRC_FILES := \
    BluetoothHci.cpp

LOCAL_CFLAGS += -Werror=unused-variable

LOCAL_C_INCLUDES += vendor/qcom/proprietary/bluetooth/hidl_transport/bt/1.0/default

LOCAL_SHARED_LIBRARIES := \
    libbase \
    libbinder \
    libbinder_ndk \
    libcutils \
    liblog \
    libutils \
    libdiag \
    libqmi_cci \
    libbtnv \
    libsoc_helper \
    android.hardware.bluetooth-V1-ndk \
    android.hardware.bluetooth@1.0 \
    android.hardware.bluetooth@1.0-impl-qti

ifeq ($(TARGET_BOARD_AUTO),false)
LOCAL_SHARED_LIBRARIES += libqmiservices
endif

LOCAL_HEADER_LIBRARIES := libril-qc-qmi-services-headers libdiag_headers vendor_common_inc

include $(BUILD_SHARED_LIBRARY)

ifeq ($(TARGET_USE_QTI_BT_AIDL), true)
include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.bluetooth-service-qti
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true

LOCAL_INIT_RC := android.hardware.bluetooth-service-qti.rc

LOCAL_VINTF_FRAGMENTS := android.hardware.bluetooth-service-qti.xml
ifeq ($(TARGET_USE_QTI_BT_HF_AUDIO_RPC), true)
LOCAL_VINTF_FRAGMENTS += vendor.qti.hardware.bluetooth.handsfree_audio.xml
endif

LOCAL_SRC_FILES := \
  service.cpp

LOCAL_SHARED_LIBRARIES := \
    libbinder \
    libbinder_ndk \
    liblog \
    libcutils \
    libutils \
    libhidlbase \
    android.hardware.bluetooth-V1-ndk \
    android.hardware.bluetooth-impl-qti

ifeq ($(TARGET_USE_QTI_BT_HAL_RPC), true)
LOCAL_CFLAGS += -DENABLE_BT_HAL_RPC
ifeq ($(TARGET_USE_QTI_BT_RPC_NEW), true)
LOCAL_SHARED_LIBRARIES += libqti-bluetooth-rpc
else
LOCAL_SHARED_LIBRARIES += libqti_bt_hal_rpc_impl
endif
ifeq ($(TARGET_USE_QTI_BT_HF_AUDIO_RPC), true)
LOCAL_CFLAGS += -DENABLE_BT_HF_AUDIO_RPC
LOCAL_SHARED_LIBRARIES += libqti-bluetooth-handsfree-audio-rpc
LOCAL_SHARED_LIBRARIES += vendor.qti.hardware.bluetooth.handsfree_audio-V1-ndk
endif
endif

LOCAL_C_INCLUDES += \
    vendor/qcom/proprietary/bluetooth/hidl_transport/bt/1.0/default \
    vendor/qcom/proprietary/bluetooth/hidl_transport/bt/aidl/api

include $(BUILD_EXECUTABLE)
endif
