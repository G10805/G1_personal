LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#----------------------------------------------------------------------------
#                 Common definitons
#----------------------------------------------------------------------------

osal-def += -D_ANDROID_

#----------------------------------------------------------------------------
#             Make the Shared library (liblx-osal)
#----------------------------------------------------------------------------

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_CFLAGS := $(osal-def)
LOCAL_SRC_FILES := src/ar_osal_test.c \
                   src/ar_test_file_io.c \
                   src/ar_test_shmem.c \
                   src/ar_test_signal_mutex_thread.c

LOCAL_MODULE := osal_in_test_example3

LOCAL_SHARED_LIBRARIES := liblog \
                          liblx-osal
LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc

include $(BUILD_EXECUTABLE)



