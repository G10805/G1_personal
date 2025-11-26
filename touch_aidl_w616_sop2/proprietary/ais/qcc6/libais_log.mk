#======================================================================
#makefile for ais_log_proprietary_qcc6 shared library
#======================================================================
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

MY_AIS_ROOT := $(LOCAL_PATH)

LOCAL_CFLAGS := $(ais_log_compile_cflags) -Wno-unused-variable -Wno-unused-parameter -fvisibility=hidden

LOCAL_C_INCLUDES += \
	$(MY_AIS_ROOT)/Common/inc \
	$(TARGET_OUT_HEADERS)/mm-osal/include

LOCAL_SRC_DIR :=\
	$(MY_AIS_ROOT)/Common/src

LOCAL_SHARED_LIBRARIES := liblog

#Android R and above uses libmmsoal, overwrite headers and libs
ifeq ( ,$(filter Q 10, $(PLATFORM_VERSION)))
LOCAL_HEADER_LIBRARIES := libmmosal_headers
ifeq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_SHARED_LIBRARIES += libmmosal_vendor
else
LOCAL_SHARED_LIBRARIES += libmmosal
endif #PLATFORM_VERSION U_ABOVE
else
LOCAL_HEADER_LIBRARIES := libmmosal_proprietary_headers
LOCAL_SHARED_LIBRARIES += libmmosal_proprietary
endif #PLATFORM_VERSION R_ABOVE

LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -maxdepth 1 -name '*.c' | sed s:^$(LOCAL_PATH)::g )
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog

LOCAL_LDFLAGS :=

LOCAL_MODULE := libais_log_proprietary_qcc6

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

ifneq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
include $(BUILD_SHARED_LIBRARY)
endif

#======================================================================
#makefile for ais_log_proprietary_qcc6 static library
#======================================================================
include $(CLEAR_VARS)

MY_AIS_ROOT := $(LOCAL_PATH)

LOCAL_CFLAGS := $(ais_log_compile_cflags) -Wno-unused-variable -Wno-unused-parameter -fvisibility=hidden

LOCAL_C_INCLUDES += \
	$(MY_AIS_ROOT)/Common/inc \
	$(TARGET_OUT_HEADERS)/mm-osal/include

LOCAL_SRC_DIR :=\
	$(MY_AIS_ROOT)/Common/src

LOCAL_SHARED_LIBRARIES := liblog

#Android R and above uses libmmsoal, overwrite headers and libs
ifeq ( ,$(filter Q 10, $(PLATFORM_VERSION)))
LOCAL_HEADER_LIBRARIES := libmmosal_headers
ifeq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_SHARED_LIBRARIES += libmmosal_vendor
else
LOCAL_SHARED_LIBRARIES += libmmosal
endif #PLATFORM_VERSION U_ABOVE
else
LOCAL_HEADER_LIBRARIES := libmmosal_proprietary_headers
LOCAL_SHARED_LIBRARIES += libmmosal_proprietary
endif #PLATFORM_VERSION R_ABOVE

LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -maxdepth 1 -name '*.c' | sed s:^$(LOCAL_PATH)::g )
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog

LOCAL_LDFLAGS :=

LOCAL_MODULE := libais_log_proprietary_qcc6

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

ifneq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
include $(BUILD_STATIC_LIBRARY)
endif
