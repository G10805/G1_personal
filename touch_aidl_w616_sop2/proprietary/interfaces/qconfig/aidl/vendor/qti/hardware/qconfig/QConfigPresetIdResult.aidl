/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.hardware.qconfig;
import vendor.qti.hardware.qconfig.Result;

/*
 * Structure for the return value and user config value
 */
@VintfStability
parcelable QConfigPresetIdResult {
    Result result;
    String value;
}