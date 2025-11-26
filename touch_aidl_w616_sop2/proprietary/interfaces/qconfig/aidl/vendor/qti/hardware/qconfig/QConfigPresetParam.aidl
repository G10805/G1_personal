/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.hardware.qconfig;

/*
 * Structure for the key-value pair of preset parameter
 */
@VintfStability
parcelable QConfigPresetParam {
    String name;
    String value;
}
