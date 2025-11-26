#======================================================================
#makefile for libais_aidl_server
#======================================================================
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE:= vendor.qti.automotive.qcarcam2@V1-service-qcc6
LOCAL_INIT_RC := vendor.qti.automotive.qcarcam2-service-qcc6.rc
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
#LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_CFLAGS += -DLOG_TAG=\"AIS_AIDL_SERVER\"
#LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include \

ifeq ($(ENABLE_HYP), true)
ifeq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DQCC_SOC_OVERRIDE
endif
endif

LOCAL_SRC_FILES:= src/service.cpp \
    src/ais_aidl_stream.cpp \
    src/ais_aidl_camera.cpp

LOCAL_C_INCLUDES := $(TOP)/vendor/qcom/proprietary/ais/qcc6/API/inc \
                    $(TOP)/vendor/qcom/proprietary/ais/qcc6/Common/inc \
                    $(TOP)/hardware/interfaces/common/support/include/aidlcommonsupport

LOCAL_STATIC_LIBRARIES := libaidlcommonsupport

LOCAL_SHARED_LIBRARIES := libais_client_qcc6 \
    liblog \
    libais_log_proprietary_qcc6 \
    libcutils \
    libutils \
    libbinder \
    libhardware \
    libhidlbase \
    libhidltransport \
    libhwbinder \
    libbase \
    libbinder_ndk \
    vendor.qti.automotive.qcarcam2-V1-ndk

ifeq ($(AIS_BUILD_WITH_CPMS_SUPPORT),true)
LOCAL_SHARED_LIBRARIES += libais_power android.frameworks.automotive.powerpolicy-V1-ndk
LOCAL_CFLAGS += -DHAL_CAMERA_CPMS_SUPPORT
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/ais/qcc6/hal/power/inc
endif

LOCAL_VINTF_FRAGMENTS := manifest_vendor.qti.automotive.qcarcam2.xml

include $(BUILD_EXECUTABLE)
