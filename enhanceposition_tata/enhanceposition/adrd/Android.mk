# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
# MediaTek Inc. (C) 2016. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libdl \
	libm \
	libc \
	adr \
	vehicle_info \
	android.hardware.gnss@1.0 \
    	android.hardware.automotive.vehicle@2.2 \
    	android.hardware.automotive.vehicle@2.1 \
    	android.hardware.automotive.vehicle@2.0 \
    	vendor.visteon.system.vsthostconfiguration@1.0 \
	libhidlbase \
	libhidltransport \
	libutils


LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/inc \
	$(LOCAL_PATH)/inc/GNSS \
	$(LOCAL_PATH)/inc/algo \
	vendor/visteon/hardware/interfaces/automotive/vehicle \
  	vendor/visteon/system/vsthostconfiguration/1.0 \

LOCAL_C_INCLUDES += $(TOP)/system/core/include
LOCAL_C_INCLUDES += $(TOP)/system/core/libcutils/include
LOCAL_HEADER_LIBRARIES +=  libcutils_headers libhardware_headers
LOCAL_EXPORT_HEADER_LIBRARY_HEADERS += libhardware_headers

LOCAL_SRC_FILES += \
        src/main.c \
        src/module.c \
        src/module_sensor.c \
        src/module_sensor_helper.c \
        src/module_gnss.c \
        src/module_gnss_helper.c \
        src/module_odom.c \
        src/module_adr_algo.c \
        src/module_adr_algo_helper.c \
        src/module_debug.c \
        src/adr_data_convert.c \
        src/config-parser.c \
        src/adr_util.c \
        src/log.c \
        src/module_sanity.c \
        src/module_data_store.c \
        src/module_daemon.c \
        src/data_coder.c \
	varconfig.cpp

LOCAL_CFLAGS += -D__ANDROID_OS__ -DV_LIBDIR="\"/vendor/lib64/vehicle_info.so\""

LOCAL_MODULE := adrd
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)

LOCAL_INIT_RC := adrd.rc
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := adr_setup
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := bin/adr_setup.sh
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := adr_alt.conf
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := config/adr_alt.conf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/etc/adr
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := mpe.conf
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := config/mpe.conf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/etc/adr
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := adr_punch.conf
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := config/adr_punch.conf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/etc/adr
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := adr_Tiago.conf
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := config/adr_Tiago.conf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/etc/adr
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := adr_Tigor.conf
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := config/adr_Tigor.conf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/etc/adr
include $(BUILD_PREBUILT)
