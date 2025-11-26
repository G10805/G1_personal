ifeq ($(CONFIG_ARCH_SA8155), y)
dtbo-y += sa8155-v1-camera.dtbo
endif

ifeq ($(CONFIG_ARCH_SA8155), y)
dtbo-y += sa8155-v2-camera.dtbo
endif

ifeq ($(CONFIG_ARCH_SA8195), y)
dtbo-y += sa8195p-camera.dtbo
endif
