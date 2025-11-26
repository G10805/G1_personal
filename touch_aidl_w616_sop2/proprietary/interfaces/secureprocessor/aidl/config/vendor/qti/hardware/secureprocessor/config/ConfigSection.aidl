/*
  * Copyright (c) 2023 Qualcomm Technologies, Inc.
  * All Rights Reserved.
  * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.secureprocessor.config;

/**
 * Secure processor config sections
 *   Secure processor configs are classified into following three
 *   sections -
 *   1. Image configs: These configs are applicable to associated image
 *                     and can change from image to image like timestamp,
 *                     frame_number, exposure_time etc.
 *   2. Session configs: These configs are applied to requested session.
 *   3. Custom configs: Clients are allows to add custom configs which
 *                      are transparent to secure processor interface.
 *                      The custom config (tag) definitions shall start
 *                      from SECURE_PROCESSOR_CUSTOM_CONFIG_START.
 */
@VintfStability
@Backing(type="int")
enum ConfigSection {
    /**
     * Image specific configuration section.
     */
    SECURE_PROCESSOR_IMAGE_CONFIG,
    /**
     * Session sepecific configuration section.
     */
    SECURE_PROCESSOR_SESSION_CONFIG,
    /**
     * Custom config section - all configurations defined in this section are
     * transparent to interface.
     */
    SECURE_PROCESSOR_CUSTOM_CONFIG,
}
