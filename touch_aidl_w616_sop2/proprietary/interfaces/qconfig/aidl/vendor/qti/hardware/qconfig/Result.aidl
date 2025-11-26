/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.hardware.qconfig;

/*
 * Result type for qconfig API
 *
 */
@VintfStability
@Backing(type="int")
enum Result {
    SUCCESS,
    ERROR,
    INVALID_ARGUMENT,
    NOT_SUPPORTED,
}
