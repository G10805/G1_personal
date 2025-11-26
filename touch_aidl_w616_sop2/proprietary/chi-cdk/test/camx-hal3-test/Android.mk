LOCAL_PATH:= $(call my-dir)
#######################################################
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
     BufferManager.cpp \
     TestLog.cpp \
     QCameraHAL3Base.cpp \
     QCameraHAL3Device.cpp \
     QCameraHAL3TestSnapshot.cpp \
     QCameraHAL3TestPreview.cpp \
     QCameraHALTestMain.cpp \
     QCameraHAL3TestVideo.cpp \
     QCameraTestVideoEncoder.cpp \
     QCameraHAL3TestConfig.cpp \

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    liblog \
    libhardware \
    libcamera_metadata \
    libcamera_client_qti \
    libhidlbase \
    libgralloctypes \
    android.hardware.graphics.mapper@4.0 \
    libgralloc.qti

LOCAL_HEADER_LIBRARIES := display_headers display_intf_headers

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

#gralloc_priv.h location moved in Android Q
ifeq ($(PLATFORM_VERSION), 10)
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/opensource/commonsys-intf/display/gralloc/ \
 $(TOP)/frameworks/native/include \
 $(TARGET_OUT_HEADERS)/common/inc
LOCAL_CFLAGS += -DANDROID_Q_AOSP -DTESTUTIL_VENDOR_LIB
LOCAL_SHARED_LIBRARIES += libbinder
endif

ifneq ($(LIBION_HEADER_PATH_WRAPPER), )
 include $(LIBION_HEADER_PATH_WRAPPER)
 LOCAL_C_INCLUDES += $(LIBION_HEADER_PATHS)
else
 LOCAL_C_INCLUDES += \
 system/core/libion/include \
 system/core/libion/kernel-headers
endif

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_C_INCLUDES += \
    system/media/camera/include \
    system/media/private/camera/include \
    system/core/include/cutils \
    system/core/include/system \
    system/core/libsystem/include \
    frameworks/native/libs/nativebase/include \
    frameworks/native/libs/arect/include \
    frameworks/native/libs/nativewindow/include/ \
    frameworks/native/include/media/hardware \
    system/core/libgrallocusage/include \
    hardware/libhardware/include  \
    hardware/qcom/display/libgralloc1 \
    hardware/qcom/media/mm-core/inc \
    hardware/qcom/media/libstagefrighthw \

LOCAL_MODULE := camx-hal3-test
LOCAL_CFLAGS += -Wall -Wextra -Werr -Wno-unused-parameter -Wno-unused-variable
LOCAL_CFLAGS += -DCAMERA_STORAGE_DIR="\"/data/vendor/camera/\""
LOCAL_CFLAGS += -std=c++17 -std=gnu++0x
LOCAL_CFLAGS += -DDISABLE_META_MODE=1
LOCAL_CFLAGS += -DUSE_GRALLOC1
LOCAL_MODULE_TAGS := tests
LOCAL_32_BIT_ONLY := true
LOCAL_VENDOR_MODULE := true
include $(BUILD_EXECUTABLE)
