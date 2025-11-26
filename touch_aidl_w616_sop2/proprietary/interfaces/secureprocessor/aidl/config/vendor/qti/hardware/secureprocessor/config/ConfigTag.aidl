/*
  * Copyright (c) 2023 Qualcomm Technologies, Inc.
  * All Rights Reserved.
  * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.secureprocessor.config;

/**
 * Secure processor config tag definitions
 *   A config consists of <tag, value> pair.
 *
 */
@VintfStability
@Backing(type="int")
enum ConfigTag {
    /**
     * Image config: Camera identifier
     * Type: int32_t
     * Mandatory image configuration.
     */
    SECURE_PROCESSOR_IMAGE_CONFIG_CAMERA_ID = vendor.qti.hardware.secureprocessor.config.ConfigSectionStart.SECURE_PROCESSOR_IMAGE_CONFIG_START + 1,
    /**
     * Image config: Frame number
     * Type: int64_t
     * Mandatory image configuration.
     */
    SECURE_PROCESSOR_IMAGE_CONFIG_FRAME_NUMBER,
    /**
     * Image config: Frame timestamp
     * Type: int64_t
     * Mandatory image configuration.
     */
    SECURE_PROCESSOR_IMAGE_CONFIG_TIMESTAMP,
    /**
     * Image config: Frame buffer width
     * Type: int32_t
     * Mandatory image configuration.
     */
    SECURE_PROCESSOR_IMAGE_CONFIG_FRAME_BUFFER_WIDTH,
    /**
     * Image config: Frame buffer height
     * Type: int32_t
     * Mandatory image configuration.
     */
    SECURE_PROCESSOR_IMAGE_CONFIG_FRAME_BUFFER_HEIGHT,
    /**
     * Image config: Frame buffer stride
     * Type: int32_t
     * Mandatory image configuration.
     */
    SECURE_PROCESSOR_IMAGE_CONFIG_FRAME_BUFFER_STRIDE,
    /**
     * Image config: Frame buffer format (expects android_pixel_format_t)
     * Type: int32_t
     * Mandatory image configuration.
     */
    SECURE_PROCESSOR_IMAGE_CONFIG_FRAME_BUFFER_FORMAT,
    SECURE_PROCESSOR_IMAGE_CONFIG_END,
    /**
     * Session config: Num sensors required in usecase
     * Type: int32_t
     * Optional session configuration (default: 1)
     * This configuration ensures that number of requested camera sensors are
     * streaming in secure state before allowing any data-processing on secure
     * data processor (aka secure destination).
     */
    SECURE_PROCESSOR_SESSION_CONFIG_NUM_SENSOR = vendor.qti.hardware.secureprocessor.config.ConfigSectionStart.SECURE_PROCESSOR_SESSION_CONFIG_START,
    /**
     * Session config: Usecase identifier
     * Type: int8_t[]
     * Mandatory session configuration.
     * It identifies the entities to be run/interact on secure destination for
     * secure data consumption/processing.
     */
    SECURE_PROCESSOR_SESSION_CONFIG_USECASE_IDENTIFIER,
    SECURE_PROCESSOR_SESSION_CONFIG_END,
    /**
     * Custom config definitions to start from
     * SECURE_PROCESSOR_CUSTOM_CONFIG_START
     */
    SECURE_PROCESSOR_CUSTOM_CONFIG_START = vendor.qti.hardware.secureprocessor.config.ConfigSectionStart.SECURE_PROCESSOR_CUSTOM_CONFIG_START,
}
