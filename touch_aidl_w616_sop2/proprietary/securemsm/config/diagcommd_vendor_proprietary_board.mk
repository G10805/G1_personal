#Strait 2.5 specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),blair)
TARGET_ENABLE_DIAGCOMMD := false
endif

#Lanai specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),pineapple)
TARGET_ENABLE_DIAGCOMMD := false
endif

#Kalpeni specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),pitti)
TARGET_ENABLE_DIAGCOMMD := false
endif

#Milos specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),volcano)
TARGET_ENABLE_DIAGCOMMD := false
endif

#Matrix specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),niobe)
TARGET_ENABLE_DIAGCOMMD := false
endif

#pakala specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),sun)
TARGET_ENABLE_DIAGCOMMD := false
endif

#Hana gen3 specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),msmnile)
TARGET_ENABLE_DIAGCOMMD := false
endif

#Talos AU specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),$(MSMSTEPPE))
TARGET_ENABLE_DIAGCOMMD := false
endif

#Monaco specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),monaco)
TARGET_ENABLE_DIAGCOMMD := false
endif
