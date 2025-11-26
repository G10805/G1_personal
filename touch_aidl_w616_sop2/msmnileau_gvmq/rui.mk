# RUI_SRC used in CarPlay stack Android.mk
RUI_SRC := $(TOPDIR)vendor/visteon/proprietary/rui_media
# RUI_MEDIA_SRC used only for S220 or W616
RUI_MEDIA_SRC := $(TOPDIR)vendor/visteon/proprietary/rui_media
CARPLAY_STACK_SRC := $(TOPDIR)vendor/visteon/proprietary/carplay_stack
RUI_MEDIA_SRC_PROJECT_DIR := infocore_cim

ANDROIDAUTO_STACK_SRC := $(TOPDIR)vendor/visteon/proprietary/androidauto_stack

# SELinux non_plat policies
BOARD_SEPOLICY_DIRS += $(RUI_SRC)/project/$(RUI_MEDIA_SRC_PROJECT_DIR)/sepolicy/non_plat
# SELinux public policies
PRODUCT_PUBLIC_SEPOLICY_DIRS += $(RUI_SRC)/project/$(RUI_MEDIA_SRC_PROJECT_DIR)/sepolicy/public
# SELinux private policies
PRODUCT_PRIVATE_SEPOLICY_DIRS += $(RUI_SRC)/project/$(RUI_MEDIA_SRC_PROJECT_DIR)/sepolicy/private


