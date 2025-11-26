#======================================================================
#makefile for libais_hidl_server
#======================================================================
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE:= vendor.qti.automotive.qcarcam@2.0-service-qcc6
LOCAL_INIT_RC := vendor.qti.automotive.qcarcam@2.0-service-qcc6.rc
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -DLOG_TAG=\"AIS_HIDL_SERVER\"

ifeq ($(ENABLE_HYP), true)
ifeq ($(PLATFORM_VERSION),$(filter T Tiramisu 13 U UpsideDownCake 14, $(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DQCC_SOC_OVERRIDE
endif
endif

LOCAL_SRC_FILES:= src/service.cpp \
    src/ais_hidl_stream.cpp \
    src/ais_hidl_camera.cpp

LOCAL_C_INCLUDES := $(TOP)/vendor/qcom/proprietary/ais/qcc6/API/inc \
                    $(TOP)/vendor/qcom/proprietary/ais/qcc6/Common/inc

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
    vendor.qti.automotive.qcarcam@2.0

LOCAL_VINTF_FRAGMENTS := manifest_vendor.qti.automotive.qcarcam@2.0.xml

include $(BUILD_EXECUTABLE)
