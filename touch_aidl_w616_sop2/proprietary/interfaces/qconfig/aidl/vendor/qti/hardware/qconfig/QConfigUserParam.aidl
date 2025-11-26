/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.hardware.qconfig;

/*
 * Structure for the key-value pair of user config
 */
@VintfStability
parcelable QConfigUserParam {
    String key;
    String value;
}
