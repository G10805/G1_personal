IDENTITY_USE_QTI_SERVICE := true
#Set IDENTITY_USE_QTI_SERVICE false for matrix, monaco and Gen-3 SP's Hana/Poipu/Talos
ifeq ($(filter $(TARGET_BOARD_PLATFORM), sm6150 msmnile monaco niobe),$(TARGET_BOARD_PLATFORM))
IDENTITY_USE_QTI_SERVICE := false
endif
