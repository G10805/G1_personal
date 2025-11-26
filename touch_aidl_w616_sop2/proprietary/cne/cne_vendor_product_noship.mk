
ifneq ($(TARGET_BOARD_AUTO),true)

#CNE
CNE := cnd

ifeq ($(GENERIC_ODM_IMAGE),true)
CNE += cnd-generic.rc
else
CNE += cnd.rc
CNE += com.qualcomm.qti.cne.xml
endif

CNE += CneApp
CNE += libcne
CNE += libcneapiclient
CNE += libwms
CNE += libxml

#this lib is no more compiled
#CNE += libvendorconn
CNE += libmasc
CNE += libcneqmiutils

#CNE_DBG
#CNE_DBG += test2client

ifneq ($(TARGET_SUPPORTS_WEARABLES),true)
#QMS
CNE_QMS := qms
CNE_QMS += qms.rc
CNE_QMS += libTxPwrJni
CNE_QMS += TxPwrAdmin
CNE_QMS += libqms_client
CNE_QMS += libssm
CNE_QMS += libqsmmw
endif

#MODEMMANAGER
ifeq ($(PRODUCT_ENABLE_QESDK),true)
MODEMMANAGER := modemManager
MODEMMANAGER += modemManager.rc
PRODUCT_PACKAGES += $(MODEMMANAGER)
endif

PRODUCT_PACKAGES_ENG += cneapitest
PRODUCT_PACKAGES += $(CNE)
PRODUCT_PACKAGES += $(CNE_QMS)

else
CNE += cneEthernetService

endif

#CACERT
CACERT := CACertService
CACERT += libcacertclient
CACERT += libjnihelper

PRODUCT_PACKAGES += $(CACERT)

ifeq ($(TARGET_BOARD_AUTO),true)

#AutoMs
AUTOMS := automs
AUTOMS += automs.rc

PRODUCT_PACKAGES += $(AUTOMS)

endif
