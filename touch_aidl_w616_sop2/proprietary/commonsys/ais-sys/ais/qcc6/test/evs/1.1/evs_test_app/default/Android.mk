LOCAL_PATH:= $(call my-dir)

##################################
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    evs_app.cpp \
    EvsStateControl.cpp \
    RenderBase.cpp \
    RenderDirectView.cpp \
    RenderTopView.cpp \
    ConfigManager.cpp \
    glError.cpp \
    shader.cpp \
    TexWrapper.cpp \
    VideoTex.cpp \
    StreamHandler.cpp \
    FormatConvert.cpp \
    RenderPixelCopy.cpp

LOCAL_SHARED_LIBRARIES := \
    libbase \
    libbinder \
    libcutils \
    libutils \
    libui \
    libhidlbase \
    libEGL \
    libGLESv2 \
    libhardware \
    libpng \
    libcamera_metadata \
    android.hardware.camera.device@3.2 \
    android.hardware.automotive.evs@1.0 \
    android.hardware.automotive.evs@1.1 \
    android.hardware.automotive.vehicle@2.0 \
    android.hardware.graphics.mapper@4.0 \
    libgralloctypes \
    libjsoncpp

LOCAL_STRIP_MODULE := keep_symbols

LOCAL_INIT_RC := evs_ais_app_qcc6.rc

LOCAL_MODULE:= evs_ais_app_qcc6
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -DLOG_TAG=\"EvsApp\"
LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES -DFPS_PRINT
LOCAL_CFLAGS += -Wall -Werror -Wunused -Wunreachable-code

ifneq ( ,$(filter 11 R ,$(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DANDROID_R_AOSP
LOCAL_SHARED_LIBRARIES += libgralloc.qti
else
LOCAL_SHARED_LIBRARIES += libgralloc.system.qti
endif

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../API/inc \
                    $(LOCAL_PATH)/../../../../../API/vendor

#for gralloc_priv.h
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/display
LOCAL_HEADER_LIBRARIES += display_intf_headers
include $(BUILD_EXECUTABLE)

#AIS camera config file
include $(CLEAR_VARS)
LOCAL_MODULE := config_qcarcam_qcc6.json
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/automotive/evs
LOCAL_SRC_FILES := config.json
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := LabeledChecker_qcar_qcc6.png
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/automotive/evs
LOCAL_SRC_FILES := LabeledChecker.png
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := CarFromTop_qcar_qcc6.png
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/automotive/evs
LOCAL_SRC_FILES := CarFromTop.png
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := evs_ais_app_default_resources_qcc6
LOCAL_REQUIRED_MODULES := \
    CarFromTop_qcar_qcc6.png \
    LabeledChecker_qcar_qcc6.png \
    config_qcarcam_qcc6.json
include $(BUILD_PHONY_PACKAGE)
