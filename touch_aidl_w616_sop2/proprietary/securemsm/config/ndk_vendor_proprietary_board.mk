#Enable INCLUDE_NDK flag to include system and vendor ndks for HY11 build

# Monaco specific defines
ifeq ($(TARGET_BOARD_PLATFORM),monaco)
INCLUDE_NDK := true
endif
