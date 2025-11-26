/*
  * Copyright (c) 2023 Qualcomm Technologies, Inc.
  * All Rights Reserved.
  * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
///////////////////////////////////////////////////////////////////////////////
// THIS FILE IS IMMUTABLE. DO NOT EDIT IN ANY CASE.                          //
///////////////////////////////////////////////////////////////////////////////

// This file is a snapshot of an AIDL file. Do not edit it manually. There are
// two cases:
// 1). this is a frozen version file - do not edit this in any case.
// 2). this is a 'current' file. If you make a backwards compatible change to
//     the interface (from the latest frozen version), the build system will
//     prompt you to update this file with `m <name>-update-api`.
//
// You must not make a backward incompatible change to any AIDL file built
// with the aidl_interface module type with versions property set. The module
// type is used to build AIDL files in a way that they can be used across
// independently updatable components of the system. If a device is shipped
// with such a backward incompatible change, it has a high risk of breaking
// later when a module using the interface is updated, e.g., Mainline modules.

package vendor.qti.hardware.secureprocessor.config;
@Backing(type="int") @VintfStability
enum ConfigTag {
  SECURE_PROCESSOR_IMAGE_CONFIG_CAMERA_ID = (vendor.qti.hardware.secureprocessor.config.ConfigSectionStart.SECURE_PROCESSOR_IMAGE_CONFIG_START + 1) /* 1 */,
  SECURE_PROCESSOR_IMAGE_CONFIG_FRAME_NUMBER,
  SECURE_PROCESSOR_IMAGE_CONFIG_TIMESTAMP,
  SECURE_PROCESSOR_IMAGE_CONFIG_FRAME_BUFFER_WIDTH,
  SECURE_PROCESSOR_IMAGE_CONFIG_FRAME_BUFFER_HEIGHT,
  SECURE_PROCESSOR_IMAGE_CONFIG_FRAME_BUFFER_STRIDE,
  SECURE_PROCESSOR_IMAGE_CONFIG_FRAME_BUFFER_FORMAT,
  SECURE_PROCESSOR_IMAGE_CONFIG_END,
  SECURE_PROCESSOR_SESSION_CONFIG_NUM_SENSOR = vendor.qti.hardware.secureprocessor.config.ConfigSectionStart.SECURE_PROCESSOR_SESSION_CONFIG_START /* 65536 */,
  SECURE_PROCESSOR_SESSION_CONFIG_USECASE_IDENTIFIER,
  SECURE_PROCESSOR_SESSION_CONFIG_END,
  SECURE_PROCESSOR_CUSTOM_CONFIG_START = vendor.qti.hardware.secureprocessor.config.ConfigSectionStart.SECURE_PROCESSOR_CUSTOM_CONFIG_START /* 131072 */,
}
