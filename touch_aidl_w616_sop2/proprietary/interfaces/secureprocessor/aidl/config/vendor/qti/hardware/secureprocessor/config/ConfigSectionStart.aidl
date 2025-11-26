/*
  * Copyright (c) 2023 Qualcomm Technologies, Inc.
  * All Rights Reserved.
  * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.secureprocessor.config;

@VintfStability
@Backing(type="int")
enum ConfigSectionStart {
    SECURE_PROCESSOR_IMAGE_CONFIG_START = vendor.qti.hardware.secureprocessor.config.ConfigSection.SECURE_PROCESSOR_IMAGE_CONFIG << 16,
    SECURE_PROCESSOR_SESSION_CONFIG_START = vendor.qti.hardware.secureprocessor.config.ConfigSection.SECURE_PROCESSOR_SESSION_CONFIG << 16,
    SECURE_PROCESSOR_CUSTOM_CONFIG_START = vendor.qti.hardware.secureprocessor.config.ConfigSection.SECURE_PROCESSOR_CUSTOM_CONFIG << 16,
}
