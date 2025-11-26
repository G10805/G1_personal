/**
 * @file vendor_ext_properties.h
 *
 * Copyright (c) 2020, 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __VENDOR_EXT_PROPERTIES__
#define __VENDOR_EXT_PROPERTIES__

#ifdef __cplusplus
extern "C" {
#endif


/// @brief ISP settings
typedef enum
{
    VENDOR_EXT_PROP_RESET,
    VENDOR_EXT_PROP_CLEAR_OP_STATUS,
    VENDOR_EXT_PROP_PIXEL_FORMAT,
    VENDOR_EXT_PROP_AUTO_HUE,
    VENDOR_EXT_PROP_AUTO_WHITE_BALANCE,
    VENDOR_EXT_PROP_WHITE_BALANCE_TEMP,
    VENDOR_EXT_PROP_EXPOSURE_COMPENSATION,
    VENDOR_EXT_PROP_BACKLIGHT_COMPENSATION,
    VENDOR_EXT_PROP_SHARPNESS,
    VENDOR_EXT_PROP_HDR_ENABLE,
    VENDOR_EXT_PROP_NOISE_REDUCTION,
    VENDOR_EXT_PROP_FRAME_RATE,
    VENDOR_EXT_PROP_FOCAL_LENGTH_H,
    VENDOR_EXT_PROP_FOCAL_LENGTH_v,
    VENDOR_EXT_PROP_NUM_EMBEDDED_LINES_TOP,
    VENDOR_EXT_PROP_NUM_EMBEDDED_LINES_BOTTOM,
    VENDOR_EXT_PROP_ROI_STATISTICS,
    VENDOR_EXT_PROP_METADATA_MODES,
    VENDOR_EXT_PROP_TEMP_SENSOR_0,
    VENDOR_EXT_PROP_TEMP_SENSOR_1,
    VENDOR_EXT_PROP_TEST,
    VENDOR_EXT_PROP_TEST_I2C_BULK,
    VENDOR_EXT_PROP_FRAME_META_DATA,
    VENDOR_EXT_PROP_MAX = 0x7FFFFFFF
} vendor_ext_property_type_t;

/// @brief Union to hold the values of different properties.
typedef union
{
  int             int_val;
  unsigned int    uint_val;
  float           float_val;
} vendor_ext_property_val_t;

/// @brief Structure to hold the property type and its value
typedef struct
{
    vendor_ext_property_type_t   type;
    vendor_ext_property_val_t    value;
} vendor_ext_property_t;


#ifdef __cplusplus
}
#endif

#endif /* __VENDOR_EXT_PROPERTIES__ */
