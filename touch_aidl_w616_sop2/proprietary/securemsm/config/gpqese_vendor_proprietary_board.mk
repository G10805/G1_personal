#Lahaina specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),lahaina)
TARGET_USES_GPQESE := true
endif

#Waipio specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),taro)
TARGET_USES_GPQESE := true
endif

#Kailua specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),kalama)
TARGET_USES_GPQESE := true
# Enable this flag in case strongbox, weaver
# solutions use OMAPI services for APDU exchange
# Also set secure_element_vintf_enabled as true in
# resource-overlay/kalama/SecureElement/res/values/config.xml
ENABLE_OMAPI_VINTF := true
endif

#Mannar specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),holi)
TARGET_USES_GPQESE := true
endif

#Bengal specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),bengal)
TARGET_USES_GPQESE := true
endif
#Strait 2.5 specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),blair)
TARGET_USES_GPQESE := true
# Enable this flag in case strongbox, weaver
# solutions use OMAPI services for APDU exchange
# Also set secure_element_vintf_enabled as true in
# resource-overlay/blair/SecureElement/res/values/config.xml
ENABLE_OMAPI_VINTF := true
#use GPQTEEC by default
TARGET_USES_GPQTEEC :=true
endif

#Lanai specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),pineapple)
TARGET_USES_GPQESE := true
# Enable this flag in case strongbox, weaver
# solutions use OMAPI services for APDU exchange
# Also set secure_element_vintf_enabled as true in
# resource-overlay/pineapple/SecureElement/res/values/config.xml
ENABLE_OMAPI_VINTF := true
endif

#Kalpeni and Kalpeni GO specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),pitti)
ifneq ($(TARGET_BOARD_SUFFIX),_32go)
TARGET_USES_GPQESE := true
# Enable this flag in case strongbox, weaver
# solutions use OMAPI services for APDU exchange
# Also set secure_element_vintf_enabled as true in
# resource-overlay/pineapple/SecureElement/res/values/config.xml
ENABLE_OMAPI_VINTF := true
endif
endif

#Milos specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),volcano)
TARGET_USES_GPQESE := true
# Enable this flag in case strongbox, weaver
# solutions use OMAPI services for APDU exchange
# Also set secure_element_vintf_enabled as true in
# resource-overlay/pineapple/SecureElement/res/values/config.xml
ENABLE_OMAPI_VINTF := true
endif

#Pakala specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),sun)
TARGET_USES_GPQESE := true
# Enable this flag in case strongbox, weaver
# solutions use OMAPI services for APDU exchange
# Also set secure_element_vintf_enabled as true in
# resource-overlay/pineapple/SecureElement/res/values/config.xml
ENABLE_OMAPI_VINTF := true
ENABLE_ESEHAL_FUZZER := true
endif

#Disable GPQESE for automotive targets
ifeq ($(TARGET_BOARD_AUTO),true)
TARGET_USES_GPQESE := false
endif

#Monaco specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),monaco)
TARGET_USES_GPQESE := true
#During boot init.svc.qteeconnector-hal-1-0 property
#read is denied, till it is resolved, gpqese client will
#use GPQTEEC by default for monaco
TARGET_USES_GPQTEEC :=true
# Enable this flag in case strongbox solutions
# use OMAPI services for APDU exchange
# Also set secure_element_vintf_enabled as true in
# resource-overlay/monaco/SecureElement/res/values/config.xml
ENABLE_OMAPI_VINTF := true
endif
