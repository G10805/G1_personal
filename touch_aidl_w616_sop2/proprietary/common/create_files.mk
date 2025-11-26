possible_cvv_dirs := $(QCPATH)/prebuilt_HY11-HWASAN $(QCPATH)/prebuilt_HY11 $(QCPATH)/prebuilt_HY22 $(QCPATH)/prebuilt_grease
prebuilt_cvv_dirs :=  $(wildcard $(possible_cvv_dirs))
TBP := $(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX)
ifeq ($(PRODUCT_NAME), gen4_gvm_gy)
TBP := gen4_gvm_gy
endif
ifeq ($(PRODUCT_NAME), gen5_gvm_gy)
TBP := gen5_gvm_gy
endif
ifeq ($(PRODUCT_NAME), msmnile_gvmq_s_u)
TBP := msmnile_gvmq_s_u
endif
ifneq ($(prebuilt_cvv_dirs),)
RET := $(shell for i in $(prebuilt_cvv_dirs);do bash vendor/qcom/proprietary/common/create_bp.sh $(TBP) $$i; done)
endif
