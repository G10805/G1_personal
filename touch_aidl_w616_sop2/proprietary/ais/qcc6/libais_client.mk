#======================================================================
#makefile for libais_client_qcc6.so
#======================================================================
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

MY_AIS_ROOT := $(LOCAL_PATH)

LOCAL_CFLAGS := $(ais_compile_cflags) \
	-Wno-unused-parameter

LOCAL_C_INCLUDES += \
	$(MY_AIS_ROOT)/API/inc \
	$(MY_AIS_ROOT)/CameraMulticlient/common/inc \
	$(MY_AIS_ROOT)/CameraMulticlient/client/inc \
	$(MY_AIS_ROOT)/CameraOSServices/CameraOSServices/inc \
	$(MY_AIS_ROOT)/CameraOSServices/CameraOSServicesMMOSAL/inc \
	$(MY_AIS_ROOT)/CameraQueue/CameraQueue/inc \
	$(MY_AIS_ROOT)/CameraEventLog/inc \
	$(MY_AIS_ROOT)/Common/inc \
	$(MY_AIS_ROOT)/Engine/inc \
	$(TARGET_OUT_HEADERS)/mm-osal/include

ifneq ($(ENABLE_HYP), true)
	LOCAL_SRC_DIR :=\
		$(MY_AIS_ROOT)/CameraMulticlient/common/src/linux \
		$(MY_AIS_ROOT)/CameraMulticlient/common/src \
		$(MY_AIS_ROOT)/CameraMulticlient/client/src \
		$(MY_AIS_ROOT)/CameraOSServices/CameraOSServicesMMOSAL/src \
		$(MY_AIS_ROOT)/CameraQueue/CameraQueueSCQ/src \
		$(MY_AIS_ROOT)/Common/src
else
	LOCAL_C_INCLUDES += \
		$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \

	LOCAL_ADDITIONAL_DEPENDENCIES :=$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

	LOCAL_SRC_DIR :=\
		$(MY_AIS_ROOT)/CameraMulticlient/common/src/hypervisor \
		$(MY_AIS_ROOT)/CameraMulticlient/common/src \
		$(MY_AIS_ROOT)/CameraMulticlient/client/src \
		$(MY_AIS_ROOT)/CameraOSServices/CameraOSServicesMMOSAL/src \
		$(MY_AIS_ROOT)/CameraQueue/CameraQueueSCQ/src \
		$(MY_AIS_ROOT)/Common/src

	LOCAL_SHARED_LIBRARIES := libuhab liblog
	LOCAL_HEADER_LIBRARIES := libmm-hab_headers
	LOCAL_CFLAGS += -DUSE_HYP
endif

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-hab

LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -maxdepth 1 -name '*.c' | sed s:^$(LOCAL_PATH)::g )

LOCAL_LDFLAGS :=

#Android R uses libmmsoal, overwrite header and libs
ifeq ( ,$(filter Q 10, $(PLATFORM_VERSION)))
LOCAL_HEADER_LIBRARIES += libmmosal_headers
LOCAL_SHARED_LIBRARIES += liblog
ifeq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_SHARED_LIBRARIES += libmmosal_vendor
else
LOCAL_SHARED_LIBRARIES += libmmosal
endif #PLATFORM_VERSION U_ABOVE
#LOCAL_COPY_HEADERS_TO := qcarcam
LOCAL_C_INCLUDES += API/inc/qcarcam.h
LOCAL_C_INCLUDES += API/inc/qcarcam_types.h
LOCAL_C_INCLUDES += API/inc/qcarcam_diag_types.h
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
else
#Early partition uses libmmsoal, overwrite header and libs
ifeq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
LOCAL_HEADER_LIBRARIES += libmmosal_headers
LOCAL_SHARED_LIBRARIES += liblog
ifeq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_SHARED_LIBRARIES += libmmosal_vendor
else
LOCAL_SHARED_LIBRARIES += libmmosal
endif #PLATFORM_VERSION U_ABOVE
else
#Android Q uses libmmsoal_proprietary, overwrite header and libs
LOCAL_HEADER_LIBRARIES += libmmosal_proprietary_headers
LOCAL_SHARED_LIBRARIES += libmmosal_proprietary liblog
#LOCAL_COPY_HEADERS_TO := qcarcam
LOCAL_C_INCLUDES += API/inc/qcarcam.h
LOCAL_C_INCLUDES += API/inc/qcarcam_types.h
LOCAL_C_INCLUDES += API/inc/qcarcam_diag_types.h
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
endif
endif

LOCAL_MODULE := libais_client_qcc6

LOCAL_PRELINK_MODULE:= false

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

ifeq ($(AIS_BUILD_STATIC),true)
include $(BUILD_STATIC_LIBRARY)
else
include $(BUILD_SHARED_LIBRARY)
endif


############################################################
####### Build the library for QCarCam header files #########
############################################################
include $(CLEAR_VARS)
LOCAL_MODULE := libais_headers_qcc6
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/./API/inc
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libais_proprietary_headers_qcc6
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/./API/inc
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_HEADER_LIBRARY)
