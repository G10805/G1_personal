/*
  * Copyright (c) 2023 Qualcomm Technologies, Inc.
  * All Rights Reserved.
  * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.secureprocessor.config;

/**
 * The concept for configuration classification is borrowed from AOSP Camera
 * HAL. Reference - /platform/hardware/interfaces/camera/metadata
 *
 *
 *
 * Secure processor config datatypes.
 */
@VintfStability
@Backing(type="int")
enum ConfigType {
    /**
     * Signed 8-bit integer (int8_t)
     */
    BYTE = 0,
    /**
     * Signed 32-bit integer (int32_t)
     */
    INT32 = 1,
    /**
     * Signed 64-bit integer (int64_t)
     */
    INT64 = 2,
}