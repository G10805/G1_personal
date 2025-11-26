LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    src/com_qualcomm_listen_ListenSoundModel.cpp \
    src/SoundModel.cpp

LOCAL_SHARED_LIBRARIES := libutils liblog liblistensoundmodel2.qti liblsmclient
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/../../lsm_ipc_client/inc \
    $(TOP)/libnativehelper/include/nativehelper

LOCAL_HEADER_LIBRARIES := jni_headers

LOCAL_MODULE:= liblistenjni.qti
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := both
LOCAL_SYSTEM_EXT_MODULE := true

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_SVA_MULTI_STAGE)),true)
LOCAL_CFLAGS += -DSVA_MULTI_STAGE_ENABLED
endif

include $(BUILD_SHARED_LIBRARY)
