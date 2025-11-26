# ---------------------------------------------------------------------------------
#             Make the DRM Sample App client module
# ---------------------------------------------------------------------------------
ifeq ($(ENABLE_DRM_SAMPLE_APP), true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
        src/drm_sample_app.cpp

QC_PROP_ROOT ?= vendor/qcom/proprietary
LOCAL_C_INCLUDES := $(SECUREMSM_SHIP_PATH)/smcinvoke/inc/ \
                    $(SECUREMSM_SHIP_PATH)/mink/inc/interface \
                    $(SECUREMSM_SHIP_PATH)/mink/inc/uid \
                    $(TOP)/vendor/qcom/proprietary/securemsm/drm_sample_app/inc \

LOCAL_HEADER_LIBRARIES := smcinvoke_kernel_headers \
                          libvmmem_headers \
                          vendor_common_inc

LOCAL_SHARED_LIBRARIES := \
  liblog \
  libutils \
  libdmabufheap \
  libvmmem \
  libminkdescriptor \
  libminksocket_vendor

LOCAL_MODULE := drm_sample_app
LOCAL_MODULE_TAGS := tests
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)
endif # ENABLE_DRM_SAMPLE_APP
