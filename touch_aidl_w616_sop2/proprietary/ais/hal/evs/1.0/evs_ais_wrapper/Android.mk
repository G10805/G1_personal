LOCAL_PATH:= $(call my-dir)

##################################
include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin

LOCAL_SRC_FILES := \
    gl_wrapper.cpp \
    video_capture.cpp \
    buffer_copy.cpp \

#compile AIS wrapper
LOCAL_SRC_FILES +=ais_evs_gldisplay.cpp ais_service.cpp ais_evs_enumerator.cpp ais_evs_camera.cpp

LOCAL_SHARED_LIBRARIES := \
    android.hardware.automotive.evs@1.0 \
    libui \
    libEGL \
    libGLESv2 \
    libbase \
    libbinder \
    libcutils \
    libhardware \
    libhidlbase \
    libhidltransport \
    liblog \
    libutils \
    libC2D2 \

LOCAL_SHARED_LIBRARIES += libgui_vendor

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_INIT_RC := android.hardware.automotive.evs@1.0-ais.rc

ifneq ( ,$(filter 10 Q 11 R, $(PLATFORM_VERSION)))
#gralloc_priv.h location moved in Android Q and above
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/opensource/commonsys-intf/display/gralloc/ \
                    $(TOP)/frameworks/native/include
LOCAL_SHARED_LIBRARIES += libbinder
ifneq ( ,$(filter 10 Q, $(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DANDROID_Q_AOSP
else
LOCAL_CFLAGS += -DANDROID_R_AOSP
endif
else
LOCAL_CFLAGS += -DANDROID_P_AOSP
endif

#link AIS libraries
LOCAL_SHARED_LIBRARIES += libais_client libdl
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qcarcam
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/adreno \
		    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
		    $(TARGET_OUT_HEADERS)/qcom/display \

LOCAL_MODULE := android.hardware.automotive.evs@1.0-ais

LOCAL_MODULE_TAGS := optional
LOCAL_STRIP_MODULE := keep_symbols

LOCAL_CFLAGS += -DLOG_TAG=\"EvsHal\"
LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES
LOCAL_CFLAGS += -Wall -Werror -Wunused -Wunreachable-code

#RGBA conversion
LOCAL_CFLAGS += -DENABLE_RGBA_CONVERSION

# NOTE:  It can be helpful, while debugging, to disable optimizations
#LOCAL_CFLAGS += -O0 -g

include $(BUILD_EXECUTABLE)
