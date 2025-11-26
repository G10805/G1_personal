ifeq ($(CAMX_CHICDK_PATH),)
  LOCAL_PATH            := $(abspath $(call my-dir)/../..)
  CAMX_CHICDK_PATH      := $(abspath $(LOCAL_PATH)/../..)
else
  LOCAL_PATH            := $(CAMX_CDK_PATH)/generated
endif
CAMX_CURDIR             := $(call my-dir)

include $(CLEAR_VARS)


# Get definitions common to the CAMX project here
include $(CAMX_CHICDK_PATH)/vendor/common.mk

LOCAL_HEADER_LIBRARIES += mm-camera-headers

ifeq ($(CAMX_EXT_VBUILD),)
# Call the buildbins.py via the LOCAL_POST_INSTALL_CMD hook after copying and removing
# a dummy file in non-VGDB environment.
  LOCAL_MODULE           := camx_buildbins
  LOCAL_MODULE_OWNER     := qti
  LOCAL_MODULE_TAGS      := optional
  LOCAL_MODULE_CLASS     := DATA
  LOCAL_SRC_FILES        := .tmp
  LOCAL_MODULE_PATH      := $(TARGET_OUT_VENDOR)/lib64/camera
  include $(BUILD_PREBUILT)
# End of buildbins.py call
endif
