AUTOMAKE_OPTIONS=foreign subdir-objects
lib_LTLIBRARIES = libvehicle_info.la

libvehicle_info_la_SOURCES =  vehicle_info.c

DEFS+=-D__LINUX_OS__ -DPACKAGE_ARCH='"$(PACKAGE_ARCH)"' -DTARGET_PLATFORM='"$(TARGET_PLATFORM)"'