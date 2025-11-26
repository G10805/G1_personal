PREBUILT_PATH := $(call my-dir)
LOCAL_PATH         := $(PREBUILT_PATH)

include $(CLEAR_VARS)
LOCAL_MODULE        := DynamicDDSService
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_MODULE_CLASS  := APPS
LOCAL_CERTIFICATE   := platform
LOCAL_MODULE_SUFFIX := .apk
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/app/DynamicDDSService/DynamicDDSService.apk
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/app
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := ODLT
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_MODULE_CLASS  := APPS
LOCAL_CERTIFICATE   := platform
LOCAL_MODULE_SUFFIX := .apk
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/app/ODLT/ODLT.apk
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/app
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := QTIDiagServices
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_MODULE_CLASS  := APPS
LOCAL_CERTIFICATE   := platform
LOCAL_MODULE_SUFFIX := .apk
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/app/QTIDiagServices/QTIDiagServices.apk
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/app
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := qsguard
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_MODULE_CLASS  := EXECUTABLES
LOCAL_SHARED_LIBRARIES :=  libbase libcutils liblog libgui libutils libbinder libservices libc++ libc libm libdl
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/bin/qsguard
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/bin
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := qspmsvc
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_MODULE_CLASS  := EXECUTABLES
LOCAL_SHARED_LIBRARIES :=  liblog libutils libcutils libbinder vendor.qti.qspmhal@1.0 android.hardware.thermal@1.0 android.hardware.thermal@2.0 android.hidl.memory@1.0 libhidlmemory libhidlbase libc++ libc libm libdl
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/bin/qspmsvc
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/bin
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := com.qualcomm.qti.dpm.api@1.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/com.qualcomm.qti.dpm.api@1.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := com.quicinc.cne.api@1.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  com.quicinc.cne.constants@1.0 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/com.quicinc.cne.api@1.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := com.quicinc.cne.api@1.1
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  com.quicinc.cne.api@1.0 com.quicinc.cne.constants@2.1 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/com.quicinc.cne.api@1.1.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := com.quicinc.cne.constants@1.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/com.quicinc.cne.constants@1.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := com.quicinc.cne.constants@2.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/com.quicinc.cne.constants@2.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := com.quicinc.cne.constants@2.1
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/com.quicinc.cne.constants@2.1.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := com.quicinc.cne.server@1.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  com.quicinc.cne.constants@1.0 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/com.quicinc.cne.server@1.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := com.quicinc.cne.server@2.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  com.quicinc.cne.constants@2.0 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/com.quicinc.cne.server@2.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := com.quicinc.cne.server@2.1
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  com.quicinc.cne.constants@2.1 com.quicinc.cne.server@2.0 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/com.quicinc.cne.server@2.1.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := com.quicinc.cne.server@2.2
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  com.quicinc.cne.constants@2.0 com.quicinc.cne.constants@2.1 com.quicinc.cne.server@2.0 com.quicinc.cne.server@2.1 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/com.quicinc.cne.server@2.2.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := libcomposerextn.qti
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libcutils liblog libutils libui libsync libhidlbase libdisplayconfig.system.qti libtinyxml2 vendor.qti.hardware.limits@1.0 vendor.qti.hardware.limits@1.1 libbinder libbinder_ndk vendor.qti.hardware.display.config-V7-ndk libgralloc.system.qti libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/libcomposerextn.qti.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := libDiagService
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libandroid_runtime libnativehelper libutils libcutils liblog libdiag_system libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/libDiagService.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := libjnihelpers
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/libjnihelpers.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := liblayerext.qti
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libcutils liblog libutils libtinyxml2 libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/liblayerext.qti.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := liblistensoundmodel2.qti
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libcutils liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/liblistensoundmodel2.qti.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := libmink-sock-native-api
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  liblog libminksocket_system libjnihelpers libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/libmink-sock-native-api.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := libmmparser_lite
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libutils libcutils liblog libmmosal libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/libmmparser_lite.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := libQSEEComAPI_system
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libc libcutils libutils liblog libhidlbase libhidlmemory libdmabufheap libbinder_ndk vendor.qti.hardware.qseecom@1.0 android.hidl.allocator@1.0 android.hidl.memory@1.0 vendor.qti.hardware.qseecom-V1-ndk libc++ libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/libQSEEComAPI_system.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := libqspmsvc
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  liblog libutils libcutils libbinder libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/libqspmsvc.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := libsdm-disp-apis.qti
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libcutils libutils liblog libhidlbase vendor.display.color@1.0 libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/libsdm-disp-apis.qti.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := libsmomoconfig.qti
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libutils libbinder libcomposerextn.qti liblog libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/libsmomoconfig.qti.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := libthermalclient.qti
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libutils liblog libbinder libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/libthermalclient.qti.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := libtrigger-handler
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libcutils libutils liblog vendor.qti.qspmhal@1.0 libhidlbase libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/libtrigger-handler.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := libupdateprof.qti
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.qspmhal@1.0 libhidlbase libhidltransport liblog libutils libcutils android.hidl.allocator@1.0 android.hidl.memory@1.0 libhidlmemory libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/libupdateprof.qti.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := libvr_amb_engine
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/libvr_amb_engine.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := libvr_object_engine
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/libvr_object_engine.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.data.factory@1.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  com.quicinc.cne.api@1.0 com.quicinc.cne.api@1.1 com.quicinc.cne.constants@2.1 com.quicinc.cne.server@2.0 com.quicinc.cne.server@2.1 com.quicinc.cne.server@2.2 vendor.qti.hardware.data.dynamicdds@1.0 vendor.qti.hardware.data.qmi@1.0 vendor.qti.ims.rcsconfig@1.0 vendor.qti.latency@2.0 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.data.factory@1.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.data.factory@2.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.hardware.data.cne.internal.api@1.0 vendor.qti.hardware.data.cne.internal.constants@1.0 vendor.qti.hardware.data.cne.internal.server@1.0 vendor.qti.hardware.data.dynamicdds@1.0 vendor.qti.hardware.data.qmi@1.0 vendor.qti.ims.rcsconfig@1.0 vendor.qti.latency@2.0 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.data.factory@2.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.data.factory@2.1
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.data.factory@2.0 vendor.qti.data.slm@1.0 vendor.qti.hardware.data.cne.internal.api@1.0 vendor.qti.hardware.data.cne.internal.constants@1.0 vendor.qti.hardware.data.cne.internal.server@1.0 vendor.qti.hardware.data.dynamicdds@1.0 vendor.qti.hardware.data.qmi@1.0 vendor.qti.ims.rcsconfig@1.0 vendor.qti.ims.rcsconfig@1.1 vendor.qti.latency@2.0 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.data.factory@2.1.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.data.factory@2.2
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.data.factory@2.0 vendor.qti.data.factory@2.1 vendor.qti.data.mwqem@1.0 vendor.qti.data.slm@1.0 vendor.qti.hardware.data.cne.internal.api@1.0 vendor.qti.hardware.data.cne.internal.constants@1.0 vendor.qti.hardware.data.cne.internal.server@1.0 vendor.qti.hardware.data.dynamicdds@1.0 vendor.qti.hardware.data.qmi@1.0 vendor.qti.ims.rcsconfig@1.0 vendor.qti.ims.rcsconfig@1.1 vendor.qti.latency@2.0 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.data.factory@2.2.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.data.factory@2.3
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.data.factory@2.0 vendor.qti.data.factory@2.1 vendor.qti.data.factory@2.2 vendor.qti.data.mwqem@1.0 vendor.qti.data.slm@1.0 vendor.qti.hardware.data.cne.internal.api@1.0 vendor.qti.hardware.data.cne.internal.constants@1.0 vendor.qti.hardware.data.cne.internal.server@1.0 vendor.qti.hardware.data.dynamicdds@1.0 vendor.qti.hardware.data.lce@1.0 vendor.qti.hardware.data.qmi@1.0 vendor.qti.ims.rcsconfig@1.0 vendor.qti.ims.rcsconfig@1.1 vendor.qti.latency@2.0 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.data.factory@2.3.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.data.factory@2.4
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.data.factory@2.0 vendor.qti.data.factory@2.1 vendor.qti.data.factory@2.2 vendor.qti.data.factory@2.3 vendor.qti.data.mwqem@1.0 vendor.qti.data.slm@1.0 vendor.qti.hardware.data.cne.internal.api@1.0 vendor.qti.hardware.data.cne.internal.constants@1.0 vendor.qti.hardware.data.cne.internal.server@1.0 vendor.qti.hardware.data.cne.internal.server@1.1 vendor.qti.hardware.data.dynamicdds@1.0 vendor.qti.hardware.data.lce@1.0 vendor.qti.hardware.data.qmi@1.0 vendor.qti.ims.rcsconfig@1.0 vendor.qti.ims.rcsconfig@1.1 vendor.qti.latency@2.0 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.data.factory@2.4.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.data.factory@2.5
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.data.factory@2.0 vendor.qti.data.factory@2.1 vendor.qti.data.factory@2.2 vendor.qti.data.factory@2.3 vendor.qti.data.factory@2.4 vendor.qti.data.mwqem@1.0 vendor.qti.data.slm@1.0 vendor.qti.hardware.data.cne.internal.api@1.0 vendor.qti.hardware.data.cne.internal.constants@1.0 vendor.qti.hardware.data.cne.internal.server@1.0 vendor.qti.hardware.data.cne.internal.server@1.1 vendor.qti.hardware.data.cne.internal.server@1.2 vendor.qti.hardware.data.dynamicdds@1.0 vendor.qti.hardware.data.flow@1.0 vendor.qti.hardware.data.lce@1.0 vendor.qti.hardware.data.qmi@1.0 vendor.qti.ims.rcsconfig@1.0 vendor.qti.ims.rcsconfig@1.1 vendor.qti.latency@2.0 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.data.factory@2.5.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.data.factory@2.6
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.data.factory@2.0 vendor.qti.data.factory@2.1 vendor.qti.data.factory@2.2 vendor.qti.data.factory@2.3 vendor.qti.data.factory@2.4 vendor.qti.data.factory@2.5 vendor.qti.data.mwqem@1.0 vendor.qti.data.slm@1.0 vendor.qti.hardware.data.cne.internal.api@1.0 vendor.qti.hardware.data.cne.internal.constants@1.0 vendor.qti.hardware.data.cne.internal.server@1.0 vendor.qti.hardware.data.cne.internal.server@1.1 vendor.qti.hardware.data.cne.internal.server@1.2 vendor.qti.hardware.data.cne.internal.server@1.3 vendor.qti.hardware.data.dynamicdds@1.0 vendor.qti.hardware.data.flow@1.0 vendor.qti.hardware.data.lce@1.0 vendor.qti.hardware.data.qmi@1.0 vendor.qti.ims.rcsconfig@1.0 vendor.qti.ims.rcsconfig@1.1 vendor.qti.latency@2.0 vendor.qti.latency@2.1 vendor.qti.latency@2.2 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.data.factory@2.6.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.data.factory@2.7
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.data.factory@2.0 vendor.qti.data.factory@2.1 vendor.qti.data.factory@2.2 vendor.qti.data.factory@2.3 vendor.qti.data.factory@2.4 vendor.qti.data.factory@2.5 vendor.qti.data.factory@2.6 vendor.qti.data.mwqem@1.0 vendor.qti.data.slm@1.0 vendor.qti.hardware.data.cne.internal.api@1.0 vendor.qti.hardware.data.cne.internal.constants@1.0 vendor.qti.hardware.data.cne.internal.server@1.0 vendor.qti.hardware.data.cne.internal.server@1.1 vendor.qti.hardware.data.cne.internal.server@1.2 vendor.qti.hardware.data.cne.internal.server@1.3 vendor.qti.hardware.data.dynamicdds@1.0 vendor.qti.hardware.data.flow@1.0 vendor.qti.hardware.data.flow@1.1 vendor.qti.hardware.data.lce@1.0 vendor.qti.hardware.data.qmi@1.0 vendor.qti.ims.rcsconfig@1.0 vendor.qti.ims.rcsconfig@1.1 vendor.qti.latency@2.0 vendor.qti.latency@2.1 vendor.qti.latency@2.2 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.data.factory@2.7.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.hardware.data.cne.internal.api@1.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.hardware.data.cne.internal.constants@1.0 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.hardware.data.cne.internal.api@1.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.hardware.data.cne.internal.constants@1.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.hardware.data.cne.internal.constants@1.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.hardware.data.cne.internal.server@1.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.hardware.data.cne.internal.constants@1.0 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.hardware.data.cne.internal.server@1.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.hardware.data.cne.internal.server@1.1
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.hardware.data.cne.internal.constants@1.0 vendor.qti.hardware.data.cne.internal.server@1.0 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.hardware.data.cne.internal.server@1.1.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.hardware.data.cne.internal.server@1.2
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.hardware.data.cne.internal.constants@1.0 vendor.qti.hardware.data.cne.internal.server@1.0 vendor.qti.hardware.data.cne.internal.server@1.1 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.hardware.data.cne.internal.server@1.2.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.hardware.data.cne.internal.server@1.3
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.hardware.data.cne.internal.constants@1.0 vendor.qti.hardware.data.cne.internal.server@1.0 vendor.qti.hardware.data.cne.internal.server@1.1 vendor.qti.hardware.data.cne.internal.server@1.2 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.hardware.data.cne.internal.server@1.3.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.hardware.data.connectionfactory-V1-ndk
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libbinder_ndk vendor.qti.hardware.data.dataactivity-V1-ndk vendor.qti.hardware.data.ka-V1-ndk libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.hardware.data.connectionfactory-V1-ndk.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.hardware.data.dataactivity-V1-ndk
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libbinder_ndk libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.hardware.data.dataactivity-V1-ndk.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.hardware.data.ka-V1-ndk
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libbinder_ndk libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.hardware.data.ka-V1-ndk.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.hardware.data.qmi@1.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.hardware.data.qmi@1.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.hardware.dpmservice@1.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.hardware.dpmservice@1.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.hardware.dpmservice@1.1
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.hardware.dpmservice@1.0 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.hardware.dpmservice@1.1.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.hardware.mwqemadapter@1.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.hardware.mwqemadapter@1.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.hardware.slmadapter@1.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.hardware.slmadapter@1.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.ims.connection@1.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.ims.connection@1.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.ims.factory@1.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.ims.callcapability@1.0 vendor.qti.ims.rcsconfig@2.0 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.ims.factory@1.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.ims.factory@1.1
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.ims.callcapability@1.0 vendor.qti.ims.factory@1.0 vendor.qti.ims.rcsconfig@2.0 vendor.qti.ims.rcsconfig@2.1 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.ims.factory@1.1.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.ims.factory@2.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.ims.connection@1.0 vendor.qti.ims.callcapability@1.0 vendor.qti.ims.rcsuce@1.0 vendor.qti.ims.rcssip@1.0 vendor.qti.ims.configservice@1.0 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.ims.factory@2.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.ims.factory@2.1
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.ims.callcapability@1.0 vendor.qti.ims.configservice@1.0 vendor.qti.ims.configservice@1.1 vendor.qti.ims.connection@1.0 vendor.qti.ims.factory@2.0 vendor.qti.ims.rcssip@1.0 vendor.qti.ims.rcssip@1.1 vendor.qti.ims.rcsuce@1.0 vendor.qti.ims.rcsuce@1.1 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.ims.factory@2.1.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.ims.factory@2.2
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.ims.callcapability@1.0 vendor.qti.ims.configservice@1.0 vendor.qti.ims.configservice@1.1 vendor.qti.ims.connection@1.0 vendor.qti.ims.factory@2.0 vendor.qti.ims.factory@2.1 vendor.qti.ims.rcssip@1.0 vendor.qti.ims.rcssip@1.1 vendor.qti.ims.rcssip@1.2 vendor.qti.ims.rcsuce@1.0 vendor.qti.ims.rcsuce@1.1 vendor.qti.ims.rcsuce@1.2 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.ims.factory@2.2.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.ims.rcssip@1.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.ims.rcssip@1.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.ims.rcssip@1.1
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.ims.rcssip@1.0 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.ims.rcssip@1.1.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.ims.rcssip@1.2
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.ims.rcssip@1.0 vendor.qti.ims.rcssip@1.1 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.ims.rcssip@1.2.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.ims.rcsuce@1.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.ims.rcsuce@1.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.ims.rcsuce@1.1
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.ims.rcsuce@1.0 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.ims.rcsuce@1.1.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.ims.rcsuce@1.2
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.ims.rcsuce@1.0 vendor.qti.ims.rcsuce@1.1 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.ims.rcsuce@1.2.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.imsrtpservice@3.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.imsrtpservice@3.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.imsrtpservice@3.1
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  vendor.qti.imsrtpservice@3.0 libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.imsrtpservice@3.1.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := vendor.qti.mstatservice@1.0
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES :=  libhidlbase liblog libutils libcutils libc++ libc libm libdl
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_OWNER  := qcom
LOCAL_MODULE_TAGS   := optional
LOCAL_SRC_FILES     := ../../.././target/product/qssi_au_64/system_ext/lib64/vendor.qti.mstatservice@1.0.so
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM_EXT)/lib64
include $(BUILD_PREBUILT)
LOCAL_PATH         := $(PREBUILT_PATH)

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.rcsuce-V1.1-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcsuce-V1.1-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.rcsuce-V1.1-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcsuce-V1.1-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.hardware.data.cne.internal.server-V1.3-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.data.cne.internal.server-V1.3-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.rcsuce-V1.2-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcsuce-V1.2-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.rcsuce-V1.2-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcsuce-V1.2-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.factory-V1.1-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.factory-V1.1-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.factory-V1.1-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.factory-V1.1-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.rcsuce-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcsuce-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.rcsuce-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcsuce-V1.0-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.hardware.dpmservice-V1.1-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.dpmservice-V1.1-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.hardware.dpmservice-V1.1-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.dpmservice-V1.1-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.hardware.slmadapter-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.slmadapter-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.hardware.slmadapter-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.slmadapter-V1.0-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.rcssip-V1.1-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcssip-V1.1-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.rcssip-V1.1-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcssip-V1.1-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.hardware.data.cne.internal.server-V1.2-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.data.cne.internal.server-V1.2-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.hardware.data.cne.internal.api-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.data.cne.internal.api-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.factory-V2.1-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.factory-V2.1-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.factory-V2.1-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.factory-V2.1-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.factory-V2.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.factory-V2.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.factory-V2.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.factory-V2.0-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.hardware.mwqemadapter-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.mwqemadapter-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.hardware.mwqemadapter-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.mwqemadapter-V1.0-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.data.factory-V2.7-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.7-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := com.quicinc.cne.constants-V2.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/com.quicinc.cne.constants-V2.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.factory-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.factory-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.factory-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.factory-V1.0-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := com.quicinc.cne.constants-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/com.quicinc.cne.constants-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.hardware.dpmservice-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.dpmservice-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.hardware.dpmservice-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.dpmservice-V1.0-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := com.quicinc.cne.api-V1.1-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/com.quicinc.cne.api-V1.1-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.data.factory-V2.2-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.2-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.factory-V2.2-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.factory-V2.2-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.factory-V2.2-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.factory-V2.2-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := com.quicinc.cne.server-V2.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/com.quicinc.cne.server-V2.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.data.factory-V2.1-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.1-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.data.factory-V2.1-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.1-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := com.quicinc.cne.server-V2.2-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/com.quicinc.cne.server-V2.2-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.hardware.data.cne.internal.server-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.data.cne.internal.server-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.data.factory-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.data.factory-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V1.0-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := com.quicinc.cne.api-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/com.quicinc.cne.api-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.hardware.data.cne.internal.constants-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.data.cne.internal.constants-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.hardware.data.cne.internal.server-V1.1-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.data.cne.internal.server-V1.1-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.connection-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.connection-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.connection-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.connection-V1.0-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.rcssip-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcssip-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.rcssip-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcssip-V1.0-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := com.quicinc.cne.server-V2.1-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/com.quicinc.cne.server-V2.1-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.rcssip-V1.2-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcssip-V1.2-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.ims.rcssip-V1.2-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.ims.rcssip-V1.2-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := com.quicinc.cne.constants-V2.1-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/com.quicinc.cne.constants-V2.1-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.hardware.data.qmi-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.hardware.data.qmi-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.data.factory-V2.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.data.factory-V2.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.0-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.data.factory-V2.6-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.6-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.data.factory-V2.4-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.4-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.data.factory-V2.4-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.4-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.data.factory-V2.5-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.5-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := com.quicinc.cne.server-V1.0-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/com.quicinc.cne.server-V1.0-java_intermediates/classes-header.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE               := vendor.qti.data.factory-V2.3-java
LOCAL_MODULE_OWNER         := qcom
LOCAL_MODULE_TAGS          := optional
LOCAL_MODULE_CLASS         := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX        := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_SRC_FILES            := ../../.././target/common/obj/JAVA_LIBRARIES/vendor.qti.data.factory-V2.3-java_intermediates/classes.jar
LOCAL_UNINSTALLABLE_MODULE := true
ifeq ($(MODULE.TARGET.$(LOCAL_MODULE_CLASS).$(LOCAL_MODULE)),)
include $(BUILD_PREBUILT)
endif
