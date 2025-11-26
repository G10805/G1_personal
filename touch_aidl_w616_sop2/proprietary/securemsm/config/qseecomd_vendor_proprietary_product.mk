#QSEECOM module specific libraries and binaries

TARGET_ENABLE_TIMER_LISTENER := true
TAGRET_ENABLE_SECURE_PROCESSOR_LISTENER := true
TARGET_ENABLE_INTERRUPT_SERVICE_LISTENER := true

ifeq ($(TARGET_USES_GY), true)
TARGET_ENABLE_TIMER_LISTENER := false
TAGRET_ENABLE_SECURE_PROCESSOR_LISTENER := false
TARGET_ENABLE_INTERRUPT_SERVICE_LISTENER := false
endif #TARGET_USES_GY

PRODUCT_PACKAGES += gpfspath_oem_config.xml
PRODUCT_PACKAGES += gpfspath_le_oem_config.xml
PRODUCT_PACKAGES += libdrmfs
PRODUCT_PACKAGES += libdrmMinimalfs
PRODUCT_PACKAGES += libdrmMinimalfsHelper
PRODUCT_PACKAGES += libdrmtime
PRODUCT_PACKAGES += libGPreqcancel
PRODUCT_PACKAGES += libGPreqcancel_svc
PRODUCT_PACKAGES += libQSEEComAPI
PRODUCT_PACKAGES += libQSEEComAPIStatic
PRODUCT_PACKAGES += libqseecom_compat
PRODUCT_PACKAGES += librpmb
PRODUCT_PACKAGES += librpmbStatic
PRODUCT_PACKAGES += librpmbStaticHelper
PRODUCT_PACKAGES += libgpt
PRODUCT_PACKAGES += libgptStatic
PRODUCT_PACKAGES += libgptStaticHelper
PRODUCT_PACKAGES += libssd
PRODUCT_PACKAGES += libssdStatic
PRODUCT_PACKAGES += libssdStaticHelper
PRODUCT_PACKAGES_DEBUG += qseecom_sample_client
PRODUCT_PACKAGES += rpmbClient
PRODUCT_PACKAGES += smcinvoke_example
PRODUCT_PACKAGES += smcinvoke_skeleton
PRODUCT_PACKAGES += smcinvoke_skeleton_cpp
PRODUCT_PACKAGES += qseecomd
PRODUCT_PACKAGES += qseecomd.rc

ifeq ($(strip $(TARGET_ENABLE_DIAGCOMMD)),true)
PRODUCT_PACKAGES += diagcommd
PRODUCT_PACKAGES += diagcommd.rc
endif

PRODUCT_PACKAGES_DEBUG += acvp_test
PRODUCT_HOST_PACKAGES += ACVP.py

