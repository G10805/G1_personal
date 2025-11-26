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
    WindowSurface.cpp \
    FormatConvert.cpp \

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libutils \
    libui \
    libgui \
    libhidlbase \
    libhidltransport \
    libEGL \
    libGLESv2 \
    libhardware \
    libpng \
    android.hardware.automotive.evs@1.0 \
    android.hardware.automotive.vehicle@2.0 \

ifneq ( ,$(filter 10 Q 11 R, $(PLATFORM_VERSION)))
#gralloc_priv.h location moved in Android Q and above
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/opensource/commonsys-intf/display/gralloc/
LOCAL_SHARED_LIBRARIES += libjsoncpp
ifneq ( ,$(filter 10 Q, $(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DANDROID_Q_AOSP
else
LOCAL_CFLAGS += -DANDROID_R_AOSP
endif
else
LOCAL_CFLAGS += -DANDROID_P_AOSP
#libjsoncppso is qc custom library only created in Android P
LOCAL_SHARED_LIBRARIES += libjsoncppso
endif

LOCAL_STRIP_MODULE := keep_symbols

LOCAL_INIT_RC := evs_app.rc

LOCAL_MODULE:= evs_ais_app
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += -DLOG_TAG=\"EvsApp\"
LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES -DFPS_PRINT
LOCAL_CFLAGS += -Wall -Werror -Wunused -Wunreachable-code

#compile time AIS flag
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qcom/display

include $(BUILD_EXECUTABLE)

#AIS camera config file
include $(CLEAR_VARS)
LOCAL_MODULE := config_qcarcam.json
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/automotive/evs
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := CarFromTop1.png
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/automotive/evs
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := LabeledChecker1.png
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/automotive/evs
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := evs_ais_app_default_resources
LOCAL_REQUIRED_MODULES := \
    CarFromTop1.png \
    LabeledChecker1.png \
    config_qcarcam.json
include $(BUILD_PHONY_PACKAGE)
