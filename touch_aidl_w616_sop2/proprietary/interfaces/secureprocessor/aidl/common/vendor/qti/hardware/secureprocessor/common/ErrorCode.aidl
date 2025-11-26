/*
  * Copyright (c) 2023 Qualcomm Technologies, Inc.
  * All Rights Reserved.
  * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.secureprocessor.common;

/**
 * Secure Processor error codes.
 */
@VintfStability
@Backing(type="int")
enum ErrorCode {
    SECURE_PROCESSOR_OK = 0,
    SECURE_PROCESSOR_FAIL,
    SECURE_PROCESSOR_BAD_VAL,
    SECURE_PROCESSOR_NEED_CALIBRATE,
    SECURE_PROCESSOR_CUSTOM_STATUS = (1 << 16),
}
